#pragma once

#include "common.h"
#include <vector>

bool EncryptPacket(uint8_t droneId,
                   const vector<uint8_t>& plaintext,
                   vector<uint8_t>& ciphertext,
                   uint8_t nonce[NONCE_SIZE],
                   uint8_t tag[TAG_SIZE]);

bool DecryptPacket(uint8_t droneId,
                   const uint8_t* ciphertext,
                   size_t ciphertextLen,
                   const uint8_t nonce[NONCE_SIZE],
                   const uint8_t tag[TAG_SIZE],
                   vector<uint8_t>& plaintext);
