#include "security.h"

#include <openssl/evp.h>
#include <openssl/rand.h>

static map<uint8_t, array<uint8_t, 32>> droneKeys = {
    {DRONE1_ID, {0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,
                 0x99,0xaa,0xbb,0xcc,0xdd,0xee,0xff,0x10,
                 0x21,0x32,0x43,0x54,0x65,0x76,0x87,0x98,
                 0xa9,0xba,0xcb,0xdc,0xed,0xfe,0x0f,0x1f}},

    {DRONE2_ID, {0xaa,0xbb,0xcc,0xdd,0xee,0xff,0x10,0x11,
                 0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,
                 0x10,0x21,0x32,0x43,0x54,0x65,0x76,0x87,
                 0x98,0xa9,0xba,0xcb,0xdc,0xed,0xfe,0x0f}}
};

bool EncryptPacket(uint8_t droneId,
                   const vector<uint8_t>& plaintext,
                   vector<uint8_t>& ciphertext,
                   uint8_t nonce[NONCE_SIZE],
                   uint8_t tag[TAG_SIZE])
{
    if (RAND_bytes(nonce, NONCE_SIZE) != 1)
        return false;

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx)
        return false;

    bool ok = true;
    int len = 0;
    int ciphertextLen = 0;

    ciphertext.resize(plaintext.size());

    ok &= EVP_EncryptInit_ex(ctx, EVP_chacha20_poly1305(), nullptr, nullptr, nullptr) == 1;
    ok &= EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_AEAD_SET_IVLEN, NONCE_SIZE, nullptr) == 1;
    ok &= EVP_EncryptInit_ex(ctx, nullptr, nullptr, droneKeys[droneId].data(), nonce) == 1;
    ok &= EVP_EncryptUpdate(ctx, ciphertext.data(), &len, plaintext.data(), plaintext.size()) == 1;

    ciphertextLen = len;

    ok &= EVP_EncryptFinal_ex(ctx, ciphertext.data() + len, &len) == 1;
    ciphertextLen += len;

    ok &= EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_AEAD_GET_TAG, TAG_SIZE, tag) == 1;

    EVP_CIPHER_CTX_free(ctx);

    if (!ok)
        return false;

    ciphertext.resize(ciphertextLen);
    return true;
}

bool DecryptPacket(uint8_t droneId,
                   const uint8_t* ciphertext,
                   size_t ciphertextLen,
                   const uint8_t nonce[NONCE_SIZE],
                   const uint8_t tag[TAG_SIZE],
                   vector<uint8_t>& plaintext)
{
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx)
        return false;

    bool ok = true;
    int len = 0;
    int plaintextLen = 0;

    plaintext.resize(ciphertextLen);

    ok &= EVP_DecryptInit_ex(ctx, EVP_chacha20_poly1305(), nullptr, nullptr, nullptr) == 1;
    ok &= EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_AEAD_SET_IVLEN, NONCE_SIZE, nullptr) == 1;
    ok &= EVP_DecryptInit_ex(ctx, nullptr, nullptr, droneKeys[droneId].data(), nonce) == 1;
    ok &= EVP_DecryptUpdate(ctx, plaintext.data(), &len, ciphertext, ciphertextLen) == 1;

    plaintextLen = len;

    ok &= EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_AEAD_SET_TAG, TAG_SIZE, (void*)tag) == 1;

    int finalResult = EVP_DecryptFinal_ex(ctx, plaintext.data() + len, &len);
    EVP_CIPHER_CTX_free(ctx);

    if (!ok || finalResult <= 0)
        return false;

    plaintextLen += len;
    plaintext.resize(plaintextLen);

    return true;
}
