#include "app.h"
#include "security.h"
#include "anti_replay.h"
#include "gui.h"

#include "ns3/socket.h"
#include "ns3/udp-socket-factory.h"

#include <arpa/inet.h>
#include <cstring>
#include <iostream>

#include "/home/omar/ns-allinone-3.47/ns-3.47/c_library_v2/common/mavlink.h"

using namespace std;

vector<uint8_t> BuildSecurePacket(uint8_t droneId, const vector<uint8_t>& plaintext)
{
    vector<uint8_t> ciphertext;
    uint8_t nonce[NONCE_SIZE];
    uint8_t tag[TAG_SIZE];

    if (!EncryptPacket(droneId, plaintext, ciphertext, nonce, tag))
        return {};

    vector<uint8_t> packet;

    packet.reserve(NONCE_SIZE + ciphertext.size() + TAG_SIZE);
    packet.insert(packet.end(), nonce, nonce + NONCE_SIZE);
    packet.insert(packet.end(), ciphertext.begin(), ciphertext.end());
    packet.insert(packet.end(), tag, tag + TAG_SIZE);

    return packet;
}

void SaveReplayPacketIfNeeded(uint8_t droneId, Ptr<Packet> packet, Address target, uint32_t seq)
{
    if (droneId == DRONE1_ID && g_replayDrone1.packet == nullptr)
    {
        g_replayDrone1.packet = packet->Copy();
        g_replayDrone1.target = target;
        g_replayDrone1.seq = seq;
    }

    if (droneId == DRONE2_ID && g_replayDrone2.packet == nullptr)
    {
        g_replayDrone2.packet = packet->Copy();
        g_replayDrone2.target = target;
        g_replayDrone2.seq = seq;
    }
}

void SendAck(Ptr<Socket> socket, Address target, uint32_t seq)
{
    uint32_t netSeq = htonl(seq);

    uint8_t ackData[5];
    ackData[0] = 'A';
    memcpy(ackData + 1, &netSeq, sizeof(netSeq));

    Ptr<Packet> ackPacket = Create<Packet>(ackData, sizeof(ackData));
    socket->SendTo(ackPacket, 0, target);
}

void SendPacket(Ptr<Socket> socket, Address target, uint8_t droneId)
{
    uint32_t seq = g_seqCounter++;
    uint32_t netSeq = htonl(seq);

    mavlink_message_t mavMsg;

    mavlink_msg_heartbeat_pack(
        1,
        200,
        &mavMsg,
        MAV_TYPE_QUADROTOR,
        MAV_AUTOPILOT_GENERIC,
        0,
        0,
        0);

    uint8_t mavBuf[MAVLINK_MAX_PACKET_LEN];
    uint16_t mavLen = mavlink_msg_to_send_buffer(mavBuf, &mavMsg);

    vector<uint8_t> plaintext;

    plaintext.insert(plaintext.end(), (uint8_t*)&netSeq, (uint8_t*)&netSeq + sizeof(netSeq));
    plaintext.push_back(droneId);
    plaintext.push_back('M');
    plaintext.push_back('A');
    plaintext.push_back('V');
    plaintext.insert(plaintext.end(), mavBuf, mavBuf + mavLen);

    vector<uint8_t> securePacket = BuildSecurePacket(droneId, plaintext);

    if (securePacket.empty())
    {
        cout << "[GCS] Encryption failed for Drone " << int(droneId) << endl;
        return;
    }

    Ptr<Packet> packet = Create<Packet>(securePacket.data(), securePacket.size());
    socket->SendTo(packet, 0, target);

    SaveReplayPacketIfNeeded(droneId, packet, target, seq);

    g_sendTimes[seq] = Simulator::Now();
    g_ackReceived[seq] = false;
    g_seqToDrone[seq] = droneId;
    g_originalSent++;

    GuiShowGcsSending(droneId, seq);

    cout << "[GCS] Sent Secure MAVLink Packet SEQ="
         << seq << " To Drone " << int(droneId) << endl;
}

void ReplayAttackFromAttacker(Ptr<Socket> attackerSocket,
                              const ReplayPacketStore& replayStore,
                              uint8_t droneId)
{
    if (replayStore.packet == nullptr)
    {
        cout << "[ATTACKER] No saved packet for Drone " << int(droneId) << endl;
        return;
    }

    g_attackerReplaySeqs.insert(replayStore.seq);

    GuiShowAttackerReplay(droneId, replayStore.seq);

    attackerSocket->SendTo(replayStore.packet->Copy(), 0, replayStore.target);

    cout << "[ATTACKER] Replay packet sent to Drone "
         << int(droneId)
         << " SEQ=" << replayStore.seq << endl;
}

void ReceiveAtDrone(uint8_t droneId, Ptr<Socket> socket)
{
    Ptr<Packet> packet;
    Address from;

    while ((packet = socket->RecvFrom(from)))
    {
        uint32_t packetSize = packet->GetSize();

        if (packetSize < MIN_SECURE_PACKET_SIZE)
            continue;
	vector<uint8_t> buffer(packetSize);
        packet->CopyData(buffer.data(), packetSize);

        uint8_t nonce[NONCE_SIZE];
        uint8_t tag[TAG_SIZE];

        memcpy(nonce, buffer.data(), NONCE_SIZE);
        memcpy(tag, buffer.data() + packetSize - TAG_SIZE, TAG_SIZE);

        const uint8_t* ciphertext = buffer.data() + NONCE_SIZE;
        size_t ciphertextLen = packetSize - NONCE_SIZE - TAG_SIZE;

        vector<uint8_t> plaintext;

        if (!DecryptPacket(droneId, ciphertext, ciphertextLen, nonce, tag, plaintext))
        {
            g_authFailed++;
            cout << "[Drone " << int(droneId) << "] Authentication Failed" << endl;
            GuiShowAuthFailed(droneId);
            continue;
        }

        if (plaintext.size() < 8)
            continue;

        uint32_t netSeq;
        memcpy(&netSeq, plaintext.data(), sizeof(netSeq));

        uint32_t seq = ntohl(netSeq);

        int replayStatus = CheckAntiReplay(droneId, seq);

        if (replayStatus == 1)
        {
            if (g_attackerReplaySeqs.count(seq))
            {
                g_replayBlocked++;
                cout << "[Drone " << int(droneId) << "] Replay Attack Blocked SEQ=" << seq << endl;
            }
            else
            {
                g_duplicatesDropped++;
                cout << "[Drone " << int(droneId) << "] Duplicate Dropped SEQ=" << seq << endl;
            }

            GuiShowReplayBlocked(droneId, seq);
            SendAck(socket, g_gcsAckAddress, seq);
            continue;
        }

        if (replayStatus == 2)
        {
            g_replayBlocked++;
            cout << "[Drone " << int(droneId) << "] Old Replay Blocked SEQ=" << seq << endl;

            GuiShowReplayBlocked(droneId, seq);
            continue;
        }

        g_acceptedPackets++;
        g_totalBytes += packetSize;

        if (g_sendTimes.count(seq))
        {
            g_totalLatencyMs +=
                (Simulator::Now() - g_sendTimes[seq]).GetMilliSeconds();
        }

        mavlink_message_t mavMsg;
        mavlink_status_t mavStatus;

        for (size_t i = 8; i < plaintext.size(); i++)
        {
            mavlink_parse_char(MAVLINK_COMM_0, plaintext[i], &mavMsg, &mavStatus);
        }

        GuiMoveDrone(droneId, socket->GetNode());
        GuiShowDroneAccepted(droneId, seq);

        SendAck(socket, g_gcsAckAddress, seq);
    }
}

void ReceiveAtGCS(Ptr<Socket> socket)
{
    Ptr<Packet> packet;
    Address from;

    while ((packet = socket->RecvFrom(from)))
    {
        if (packet->GetSize() != 5)
            continue;

        uint8_t ackData[5];
        packet->CopyData(ackData, sizeof(ackData));

        if (ackData[0] != 'A')
            continue;

        uint32_t netSeq;
        memcpy(&netSeq, ackData + 1, sizeof(netSeq));

        uint32_t seq = ntohl(netSeq);

        g_ackReceived[seq] = true;
        g_ackPackets++;

        uint8_t droneId = 0;

        if (g_seqToDrone.count(seq))
            droneId = g_seqToDrone[seq];

        cout << "[GCS] ACK Received SEQ=" << seq << endl;

        if (droneId != 0)
            GuiShowAckAtGcs(droneId, seq);
    }
}

void CreateDroneSockets(NodeContainer& nodes)
{
    for (uint8_t droneId = DRONE1_ID; droneId <= DRONE2_ID; droneId++)
    {
        Ptr<Socket> socket =
            Socket::CreateSocket(
                nodes.Get(GetNodeIdFromDroneId(droneId)),
                UdpSocketFactory::GetTypeId());

        socket->Bind(InetSocketAddress(Ipv4Address::GetAny(), UDP_PORT));
        socket->SetRecvCallback(MakeBoundCallback(&ReceiveAtDrone, droneId));
    }
}
