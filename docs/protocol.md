# Tic Tac Toe protocol

Tic tac toe uses the LoRa physical protocol to transmit data, ignoring LoRaWAN and other higher-level protocols. While it's untested,
this protocol is designed to allow multiple games to run in range of each other without stepping on each other's toes. The basic flow
of a game goes like this:

1. Devices A and B send out discovery packets. They also listen for discovery packets.
1. When device B, say, receives a discovery packet from device A, it will send a request to play to device A.
1. When device A receives a request to play from device B, it'll send it's acceptance to device B. The game can now begin.
1. The device that sent the request to play will go first.
1. When the user places an X or O, the device will send the coords to the other device.

There is no communication after that. When the game ends, each device will return to discovery, or stop participating.

## Packet anatomy

Each device has a unique 16 bit ID. Each packet starts with that ID, followed by an 8 bit instruction. If the instruction
takes any parameters, they'll follow the instruction bits. If any piece of data takes more than one byte, the highest byte
will be transmitted first. When a device receives a packet with a recipient ID for a different device, it MUST ignore that
packet.

### SEEKING_OPPONENT (discovery)

1. Sender ID: 16 bits
1. Instruction: 8 bits: 0000 0001

### MATCH_REQUEST

1. Sender ID: 16 bits
1. Instruction: 8 bits: 0000 00010
1. Recipient ID: 16 bits

### ACCEPT_MATCH

1. Sender ID: 16 bits
1. Instruction: 8 bits: 0000 0011
1. Recipient ID: 16 bits

### PLACE_SYMBOL

1. Sender ID: 16 bits
1. Instruction: 8 bits: 0000 0100
1. Recipient ID: 16 bits
1. Coordinates: 8 bits

The coordinates can be constructed with coords = 3x + y, and deconstructed with x = coords / 3, y = coords % 3. (0, 0)
represents the lower left cell, (2, 0) the lower right, (0, 2) the upper left and (2, 2) the upper right.