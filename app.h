#pragma once

#include "common.h"

void SendPacket(Ptr<Socket> socket, Address target, uint8_t droneId);
void ReceiveAtDrone(uint8_t droneId, Ptr<Socket> socket);
void ReceiveAtGCS(Ptr<Socket> socket);

void ReplayAttackFromAttacker(Ptr<Socket> attackerSocket,
                              const ReplayPacketStore& replayStore,
                              uint8_t droneId);

void CreateDroneSockets(NodeContainer& nodes);
