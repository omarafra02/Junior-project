#include "common.h"

uint32_t g_seqCounter = 1;
Address g_gcsAckAddress;
AnimationInterface* g_anim = nullptr;

map<uint32_t, uint8_t> g_seqToDrone;
map<uint8_t, AntiReplayWindow> g_antiReplay;
map<uint32_t, bool> g_ackReceived;
set<uint32_t> g_attackerReplaySeqs;

ReplayPacketStore g_replayDrone1;
ReplayPacketStore g_replayDrone2;

uint32_t g_originalSent = 0;
uint32_t g_acceptedPackets = 0;
uint32_t g_ackPackets = 0;
uint32_t g_duplicatesDropped = 0;
uint32_t g_replayBlocked = 0;
uint32_t g_authFailed = 0;

uint32_t g_totalBytes = 0;
double g_totalLatencyMs = 0.0;
map<uint32_t, Time> g_sendTimes;

uint32_t GetNodeIdFromDroneId(uint8_t droneId)
{
    return static_cast<uint32_t>(droneId);
}

string DroneName(uint8_t droneId)
{
    return "Drone" + to_string(droneId);
}
