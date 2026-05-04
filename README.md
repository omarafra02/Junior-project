# UAV Secure Communication Simulation (ns-3)

## Overview
This project simulates secure communication between a Ground Control Station (GCS) and UAV drones using ns-3.

## Features
- MAVLink protocol
- ChaCha20-Poly1305 encryption
- Anti-replay protection
- Replay attack simulation
- NetAnim GUI visualization
- Performance metrics (latency, packet loss, throughput)

## Network Topology
- Node 0: Ground Control Station (GCS)
- Node 1: Drone 1
- Node 2: Drone 2
- Node 3: Attacker

## Security Mechanism
- Each drone has a unique symmetric key
- Packets are encrypted using ChaCha20-Poly1305
- Replay protection implemented using sliding window
- Duplicate and replayed packets are detected and blocked

## Attack Scenario
The attacker node captures packets and replays them.
The drones detect and block replayed packets.

## Simulation Output
- NetAnim visualization (GUI)
- Wireshark PCAP files
- Metrics:
  - Latency
  - Packet Loss
  - Throughput

## How to Run

`bash
./ns3 run scratch/uav_secure/uav_secure


## Report
The full project report (including results and screenshots) will be added to this repository
