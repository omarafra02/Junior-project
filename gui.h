#pragma once

#include "common.h"

void ConfigureAnimation(NodeContainer& nodes);

void GuiSetGcsIdle();
void GuiSetDroneIdle(uint8_t droneId);
void GuiSetAttackerIdle();

void GuiShowGcsSending(uint8_t droneId, uint32_t seq);
void GuiShowDroneAccepted(uint8_t droneId, uint32_t seq);
void GuiShowAckAtGcs(uint8_t droneId, uint32_t seq);
void GuiShowReplayBlocked(uint8_t droneId, uint32_t seq);
void GuiShowAuthFailed(uint8_t droneId);
void GuiShowAttackerReplay(uint8_t droneId, uint32_t seq);

void GuiMoveDrone(uint8_t droneId, Ptr<Node> node);
