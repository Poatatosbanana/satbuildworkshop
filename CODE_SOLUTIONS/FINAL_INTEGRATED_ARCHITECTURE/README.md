# Final Integrated Architecture (GS + OBC + ADCS)

This folder provides a complete workshop architecture assembled from the existing exercise code blocks:

- `GS_COMMAND_CONSOLE/GS_COMMAND_CONSOLE.ino`
- `OBC_MISSION_CONTROLLER/OBC_MISSION_CONTROLLER.ino`
- `ADCS_POINTING_CONTROLLER/ADCS_POINTING_CONTROLLER.ino`

## System Architecture

1. **Ground Station (GS)**  
   Sends mission commands over LoRa and prints responses.

2. **On-Board Computer (OBC)**  
   Receives LoRa commands, runs telemetry functions (TIME, TEMPOBC, TEMPEPS, VBAT), and forwards ADCS commands.

3. **ADCS Subsystem**  
   Receives pointing commands from OBC over UART, controls panel servo, and returns ADCS status.

## Interface Protocol

- GS sends: `CMD:<COMMAND>`
- OBC responds: `DATA:<RESPONSE>`

### Supported Commands

- `TIMEOBC`
- `RTIMEOBC`
- `TEMPOBC`
- `TEMPEPS`
- `TEMPCOMPARISON`
- `VBAT`
- `SAT_STATUS`
- `ADCS_STATUS`
- `ADCS_POINT_SUN`
- `ADCS_POINT_NADIR`
- `ADCS_SET_ANGLE:<0-180>`

## Wiring Assumptions

### LoRa
- GS and OBC both use LoRa at `434E6`.
- Wire each LoRa module according to your workshop board schematic.

### OBC Sensor Inputs
- `A1`: OBC temperature sensor
- `A2`: EPS temperature sensor
- `A0`: battery voltage monitor

### OBC ↔ ADCS UART Bus
- OBC sketch uses `SoftwareSerial`:
  - OBC pin `9` (TX) -> ADCS board RX
  - OBC pin `8` (RX) <- ADCS board TX
- Common GND required between OBC and ADCS boards.

### ADCS Actuation and Sensors
- `D5`: servo output for panel orientation
- `A3`: left sun sensor
- `A4`: right sun sensor

## Bring-up Order

1. Upload ADCS sketch to ADCS board.
2. Upload OBC sketch to OBC board.
3. Upload GS sketch to ground station board.
4. Open GS Serial Monitor at `9600 baud` and send commands listed above.
