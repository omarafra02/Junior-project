#include "metrics.h"
#include "common.h"

#include <iostream>

using namespace std;

uint32_t CountUniqueAcks()
{
    uint32_t count = 0;

    for (const auto& item : g_ackReceived)
    {
        if (item.second)
            count++;
    }

    return count;
}

void PrintMetrics()
{
    double packetLoss = 0.0;

    if (g_originalSent > 0)
    {
        packetLoss =
            ((double)(g_originalSent - g_acceptedPackets) / g_originalSent) * 100.0;
    }

    uint32_t uniqueAcks = CountUniqueAcks();

    double ackLoss = 0.0;

    if (g_acceptedPackets > 0)
    {
        ackLoss =
            ((double)(g_acceptedPackets - uniqueAcks) / g_acceptedPackets) * 100.0;
    }

    double avgLatency =
        g_acceptedPackets ? g_totalLatencyMs / g_acceptedPackets : 0.0;

    double throughput =
        (g_totalBytes * 8.0) / SIMULATION_TIME;

    cout << "\n========== FINAL REPORT ==========" << endl;
    cout << "Original Sent Packets       : " << g_originalSent << endl;
    cout << "Accepted Unique Packets     : " << g_acceptedPackets << endl;
    cout << "ACK Packets Received        : " << g_ackPackets << endl;
    cout << "Unique ACKs                 : " << uniqueAcks << endl;
    cout << "Packet Loss                 : " << packetLoss << " %" << endl;
    cout << "ACK Loss                    : " << ackLoss << " %" << endl;
    cout << "Duplicates Dropped          : " << g_duplicatesDropped << endl;
    cout << "Replay Attacks Blocked      : " << g_replayBlocked << endl;
    cout << "Authentication Failed       : " << g_authFailed << endl;
    cout << "Average Latency             : " << avgLatency << " ms" << endl;
    cout << "Throughput                  : " << throughput << " bps" << endl;
    cout << "=================================" << endl;
}
