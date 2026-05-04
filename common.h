#pragma once

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/netanim-module.h"

#include <array>
#include <map>
#include <set>

using namespace ns3;
using namespace std;

static constexpr uint32_t GCS_NODE_ID = 0;
static constexpr uint32_t DRONE1_NODE_ID = 1;
static constexpr uint32_t DRONE2_NODE_ID = 2;
static constexpr uint32_t ATTACKER_NODE_ID = 3;

static constexpr uint8_t DRONE1_ID = 1;
static constexpr uint8_t DRONE2_ID = 2;

static constexpr uint16_t UDP_PORT = 8080;
static constexpr double SIMULATION_TIME = 12.0;
static constexpr double METRICS_TIME = 11.5;

static constexpr uint32_t TOTAL_PACKETS_PER_DRONE = 50;
static constexpr double SEND_INTERVAL = 0.05;
static constexpr double DRONE2_SEND_OFFSET = 0.025;

static constexpr double ATTACK_DRONE1_TIME = 5.0;
static constexpr double ATTACK_DRONE2_TIME = 6.0;

static constexpr uint32_t NONCE_SIZE = 12;
static constexpr uint32_t TAG_SIZE = 16;
static constexpr uint32_t MIN_SECURE_PACKET_SIZE = NONCE_SIZE + TAG_SIZE;

struct AntiReplayWindow
{
    uint32_t maxSeq = 0;
    uint64_t bitmap = 0;
};

struct ReplayPacketStore
{
    Ptr<Packet> packet;
    Address target;
    uint32_t seq = 0;
};

extern uint32_t g_seqCounter;
extern Address g_gcsAckAddress;
extern AnimationInterface* g_anim;

extern map<uint32_t, uint8_t> g_seqToDrone;
extern map<uint8_t, AntiReplayWindow> g_antiReplay;
extern map<uint32_t, bool> g_ackReceived;
extern set<uint32_t> g_attackerReplaySeqs;

extern ReplayPacketStore g_replayDrone1;
extern ReplayPacketStore g_replayDrone2;

extern uint32_t g_originalSent;
extern uint32_t g_acceptedPackets;
extern uint32_t g_ackPackets;
extern uint32_t g_duplicatesDropped;
extern uint32_t g_replayBlocked;
extern uint32_t g_authFailed;

extern uint32_t g_totalBytes;
extern double g_totalLatencyMs;
extern map<uint32_t, Time> g_sendTimes;

uint32_t GetNodeIdFromDroneId(uint8_t droneId);
string DroneName(uint8_t droneId);
