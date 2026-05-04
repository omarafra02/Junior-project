#include "gui.h"

void GuiSetGcsIdle()
{
    if (!g_anim) return;

    g_anim->UpdateNodeColor(GCS_NODE_ID, 0, 255, 0);
    g_anim->UpdateNodeDescription(GCS_NODE_ID, "GCS");
}

void GuiSetDroneIdle(uint8_t droneId)
{
    if (!g_anim) return;

    uint32_t nodeId = GetNodeIdFromDroneId(droneId);
    g_anim->UpdateNodeColor(nodeId, 0, 0, 255);
    g_anim->UpdateNodeDescription(nodeId, DroneName(droneId));
}

void GuiSetAttackerIdle()
{
    if (!g_anim) return;

    g_anim->UpdateNodeColor(ATTACKER_NODE_ID, 255, 0, 0);
    g_anim->UpdateNodeDescription(ATTACKER_NODE_ID, "ATTACKER");
}

void GuiShowGcsSending(uint8_t droneId, uint32_t seq)
{
    if (!g_anim) return;

    uint32_t droneNodeId = GetNodeIdFromDroneId(droneId);

    g_anim->UpdateNodeColor(GCS_NODE_ID, 160, 32, 240);
    g_anim->UpdateNodeDescription(
        GCS_NODE_ID,
        "GCS -> D" + to_string(droneId) + " | SEQ=" + to_string(seq));

    g_anim->UpdateLinkDescription(
        GCS_NODE_ID,
        droneNodeId,
        "SECURE SEND | SEQ=" + to_string(seq));

    Simulator::Schedule(Seconds(0.35), &GuiSetGcsIdle);
}

void GuiShowDroneAccepted(uint8_t droneId, uint32_t seq)
{
    if (!g_anim) return;

    uint32_t nodeId = GetNodeIdFromDroneId(droneId);

    g_anim->UpdateNodeColor(nodeId, 255, 255, 0);
    g_anim->UpdateNodeDescription(
        nodeId,
        "D" + to_string(droneId) + " | OK | SEQ=" + to_string(seq));

    g_anim->UpdateLinkDescription(
        GCS_NODE_ID,
        nodeId,
        "ACCEPTED | AUTH OK | SEQ=" + to_string(seq));

    Simulator::Schedule(Seconds(0.45), &GuiSetDroneIdle, droneId);
}

void GuiShowAckAtGcs(uint8_t droneId, uint32_t seq)
{
    if (!g_anim) return;

    g_anim->UpdateNodeColor(GCS_NODE_ID, 0, 255, 255);
    g_anim->UpdateNodeDescription(
        GCS_NODE_ID,
        "GCS | ACK D" + to_string(droneId) + " | SEQ=" + to_string(seq));

    g_anim->UpdateLinkDescription(
        GetNodeIdFromDroneId(droneId),
        GCS_NODE_ID,
        "ACK <- D" + to_string(droneId) + " | SEQ=" + to_string(seq));

    Simulator::Schedule(Seconds(0.45), &GuiSetGcsIdle);
}

void GuiShowReplayBlocked(uint8_t droneId, uint32_t seq)
{
    if (!g_anim) return;

    uint32_t nodeId = GetNodeIdFromDroneId(droneId);

    g_anim->UpdateNodeColor(nodeId, 255, 0, 0);
    g_anim->UpdateNodeDescription(
        nodeId,
        "D" + to_string(droneId) + " | REPLAY BLOCKED | SEQ=" + to_string(seq));

    g_anim->UpdateLinkDescription(
        ATTACKER_NODE_ID,
        nodeId,
        "REPLAY BLOCKED | SEQ=" + to_string(seq));

    Simulator::Schedule(Seconds(1.0), &GuiSetDroneIdle, droneId);
}

void GuiShowAuthFailed(uint8_t droneId)
{
    if (!g_anim) return;

    uint32_t nodeId = GetNodeIdFromDroneId(droneId);

    g_anim->UpdateNodeColor(nodeId, 255, 0, 0);
    g_anim->UpdateNodeDescription(
        nodeId,
        "D" + to_string(droneId) + " | AUTH FAILED");

    Simulator::Schedule(Seconds(1.0), &GuiSetDroneIdle, droneId);
}

void GuiShowAttackerReplay(uint8_t droneId, uint32_t seq)
{
    if (!g_anim) return;

    uint32_t droneNodeId = GetNodeIdFromDroneId(droneId);

    g_anim->UpdateNodeColor(ATTACKER_NODE_ID, 255, 0, 0);
    g_anim->UpdateNodeDescription(
        ATTACKER_NODE_ID,
        "ATTACKER -> D" + to_string(droneId) + " | SEQ=" + to_string(seq));

    g_anim->UpdateLinkDescription(
        ATTACKER_NODE_ID,
        droneNodeId,
        "REPLAY ATTACK | SEQ=" + to_string(seq));

    Simulator::Schedule(Seconds(1.0), &GuiSetAttackerIdle);
}

void GuiMoveDrone(uint8_t droneId, Ptr<Node> node)
{
    Ptr<MobilityModel> mobility = node->GetObject<MobilityModel>();
    Vector pos = mobility->GetPosition();

    if (droneId == DRONE1_ID)
        pos.y += 2.0;
    else if (droneId == DRONE2_ID)
        pos.y -= 2.0;

    mobility->SetPosition(pos);
}

void ConfigureAnimation(NodeContainer& nodes)
{
    static AnimationInterface anim("secure-uav-split.xml");
    g_anim = &anim;

    anim.EnablePacketMetadata(true);
    anim.SetConstantPosition(nodes.Get(GCS_NODE_ID), 0, 0);
    anim.SetConstantPosition(nodes.Get(DRONE1_NODE_ID), 70, 35);
    anim.SetConstantPosition(nodes.Get(DRONE2_NODE_ID), 70, -35);
    anim.SetConstantPosition(nodes.Get(ATTACKER_NODE_ID), -35, 45);

    anim.UpdateNodeDescription(nodes.Get(GCS_NODE_ID), "GCS");
    anim.UpdateNodeDescription(nodes.Get(DRONE1_NODE_ID), "Drone1");
    anim.UpdateNodeDescription(nodes.Get(DRONE2_NODE_ID), "Drone2");
    anim.UpdateNodeDescription(nodes.Get(ATTACKER_NODE_ID), "ATTACKER");

    anim.UpdateNodeColor(nodes.Get(GCS_NODE_ID), 0, 255, 0);
    anim.UpdateNodeColor(nodes.Get(DRONE1_NODE_ID), 0, 0, 255);
    anim.UpdateNodeColor(nodes.Get(DRONE2_NODE_ID), 0, 0, 255);
    anim.UpdateNodeColor(nodes.Get(ATTACKER_NODE_ID), 255, 0, 0);

    anim.UpdateNodeSize(GCS_NODE_ID, 12.0, 12.0);
    anim.UpdateNodeSize(DRONE1_NODE_ID, 9.0, 9.0);
    anim.UpdateNodeSize(DRONE2_NODE_ID, 9.0, 9.0);
    anim.UpdateNodeSize(ATTACKER_NODE_ID, 14.0, 14.0);

    anim.UpdateLinkDescription(GCS_NODE_ID, DRONE1_NODE_ID, "SECURE CHANNEL");
    anim.UpdateLinkDescription(GCS_NODE_ID, DRONE2_NODE_ID, "SECURE CHANNEL");
    anim.UpdateLinkDescription(ATTACKER_NODE_ID, DRONE1_NODE_ID, "REPLAY ATTACK PATH");
    anim.UpdateLinkDescription(ATTACKER_NODE_ID, DRONE2_NODE_ID, "REPLAY ATTACK PATH");
}
