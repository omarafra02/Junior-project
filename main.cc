#include "common.h"
#include "gui.h"
#include "app.h"
#include "metrics.h"

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/socket.h"
#include "ns3/udp-socket-factory.h"

using namespace ns3;

void ConfigureWifi(NodeContainer& nodes, NetDeviceContainer& devices)
{
    WifiHelper wifi;
    wifi.SetStandard(WIFI_STANDARD_80211g);

    YansWifiPhyHelper phy;

    phy.Set("TxPowerStart", DoubleValue(20.0));
    phy.Set("TxPowerEnd", DoubleValue(20.0));

    YansWifiChannelHelper channel;

    channel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");

    channel.AddPropagationLoss(
        "ns3::LogDistancePropagationLossModel",
        "Exponent", DoubleValue(2.4),
        "ReferenceLoss", DoubleValue(40.0));

    phy.SetChannel(channel.Create());

    WifiMacHelper mac;
    mac.SetType("ns3::AdhocWifiMac");

    devices = wifi.Install(phy, mac, nodes);

    phy.EnablePcapAll("secure-uav-split");
}

void ConfigureMobility(NodeContainer& nodes)
{
    MobilityHelper mobility;
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(nodes);

    nodes.Get(GCS_NODE_ID)->GetObject<MobilityModel>()->SetPosition(Vector(0, 0, 0));
    nodes.Get(DRONE1_NODE_ID)->GetObject<MobilityModel>()->SetPosition(Vector(70, 35, 0));
    nodes.Get(DRONE2_NODE_ID)->GetObject<MobilityModel>()->SetPosition(Vector(70, -35, 0));
    nodes.Get(ATTACKER_NODE_ID)->GetObject<MobilityModel>()->SetPosition(Vector(-35, 45, 0));
}

int main()
{
    NodeContainer nodes;
    nodes.Create(4);

    NetDeviceContainer devices;

    ConfigureWifi(nodes, devices);

    InternetStackHelper stack;
    stack.Install(nodes);

    ConfigureMobility(nodes);
    ConfigureAnimation(nodes);

    Ipv4AddressHelper address;
    address.SetBase("192.168.1.0", "255.255.255.0");

    Ipv4InterfaceContainer interfaces =
        address.Assign(devices);

    CreateDroneSockets(nodes);

    Ptr<Socket> gcsSocket =
        Socket::CreateSocket(nodes.Get(GCS_NODE_ID), UdpSocketFactory::GetTypeId());

    gcsSocket->Bind(InetSocketAddress(Ipv4Address::GetAny(), UDP_PORT));
    gcsSocket->SetRecvCallback(MakeCallback(&ReceiveAtGCS));

    Ptr<Socket> attackerSocket =
        Socket::CreateSocket(nodes.Get(ATTACKER_NODE_ID), UdpSocketFactory::GetTypeId());

    Address drone1Addr =
        InetSocketAddress(interfaces.GetAddress(DRONE1_NODE_ID), UDP_PORT);

    Address drone2Addr =
        InetSocketAddress(interfaces.GetAddress(DRONE2_NODE_ID), UDP_PORT);

    g_gcsAckAddress =
        InetSocketAddress(interfaces.GetAddress(GCS_NODE_ID), UDP_PORT);

    for (uint32_t i = 1; i <= TOTAL_PACKETS_PER_DRONE; i++)
    {
        Simulator::Schedule(
            Seconds(i * SEND_INTERVAL),
            &SendPacket,
            gcsSocket,
            drone1Addr,
            DRONE1_ID);

        Simulator::Schedule(
            Seconds(i * SEND_INTERVAL + DRONE2_SEND_OFFSET),
            &SendPacket,
            gcsSocket,
            drone2Addr,
            DRONE2_ID);
    }

    Simulator::Schedule(
        Seconds(ATTACK_DRONE1_TIME),
        &ReplayAttackFromAttacker,
        attackerSocket,
        cref(g_replayDrone1),
        DRONE1_ID);

    Simulator::Schedule(
        Seconds(ATTACK_DRONE2_TIME),
        &ReplayAttackFromAttacker,
        attackerSocket,
        cref(g_replayDrone2),
        DRONE2_ID);

    Simulator::Schedule(Seconds(METRICS_TIME), &PrintMetrics);

    Simulator::Stop(Seconds(SIMULATION_TIME));
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}
