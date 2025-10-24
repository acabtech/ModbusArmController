# Modbus Arm Controller

A multifunction system based on Raspberry Pi Pico RP2040, implementing Modbus RTU over UART for controlling up to 5 servos, up to 16 relays, and reading data from a BME280 environmental sensor (temperature, humidity, pressure, altitude). Designed for long-term reliability and maintainability with modular architecture.

## Hardware Overview
- **Microcontroller**: Raspberry Pi Pico (RP2040)
- **Communication**: Modbus RTU over UART (Serial1)
- **Actuators**:
  - 5 Servos (modular control via ServoManager)
  - 16 Relays (modular control via RelayManager, direct GPIO)
- **Sensor**: BME280 (I2C, reads temp, humidity, pressure, altitude)
- **Power**: Ensure stable 5V supply for Pico and peripherals; servos may require external power.

## Pin Assignments
Defined in [`include/Config.h`](include/Config.h:1):
- **UART (Modbus)**: TX = GP0, RX = GP1, Baud = 9600, 8N1
- **I2C (BME280)**: SDA = GP4, SCL = GP5, Address = 0x76
- **Servos**: GP6, GP7, GP8, GP9, GP10 (default position 90°)
- **Relays**: GP11 to GP26 (active HIGH, safe state LOW/OFF)

## Modbus Protocol
- **Slave ID**: 1
- **Function Codes Supported**:
  - 0x01 Read Coils: Relays (addresses 0-15, ON/OFF)
  - 0x05 Write Single Coil: Set relay ON/OFF
  - 0x15 Write Multiple Coils: Set multiple relays
  - 0x03 Read Holding Registers: Servo positions (0-4: 0-180°), Sensor data (10: Temp x10, 11: Humidity x10, 12: Pressure Pa, 13: Altitude x100 m)
  - 0x06 Write Single Register: Set servo position
  - 0x10 Write Multiple Registers: Set multiple servo positions
- Sensor data updated every 5s in loop.
- Responses include error codes for invalid requests.

## Software Architecture
- **Framework**: Arduino for RP2040 (PlatformIO)
- **Libraries**:
  - Modbus-Master-Slave-for-Arduino (Modbus RTU slave)
  - Adafruit BME280 Library & Unified Sensor (BME280)
  - Built-in Servo library (servos)
  - Built-in Wire & Arduino.h (I2C, GPIO)
- **Modular Classes** (in `include/` and `src/`):
  - [`Config.h`](include/Config.h): Central pins, settings, addresses
  - [`ServoManager`](include/ServoManager.h): Initializes/sets servo positions, safe default 90°
  - [`RelayManager`](include/RelayManager.h): Initializes/sets relay states, safe OFF
  - [`BME280Reader`](include/BME280Reader.h): Initializes/reads sensor data
  - [`ModbusHandler`](include/ModbusHandler.h): Manages Modbus polling, syncs coils/registers to/from managers, periodic sensor reads
- **Main Flow** ([`src/main.cpp`](src/main.cpp:1)):
  - `setup()`: Init Serial (debug), managers, Modbus; log failures
  - `loop()`: Poll Modbus (handles commands/responses), 10ms delay
- **Reliability Features**:
  - Input validation: Constrain servo degrees (0-180), bounds checks
  - Safe states: Servos to 90° on destruct/init fail; Relays OFF
  - Logging: Serial output for init status, errors
  - Watchdog: Not implemented (add via `<hardware/watchdog.h>` if needed for production)
  - Non-blocking: Modbus poll in loop, no delays in critical paths
  - Maintainability: Configurable via header, modular classes for easy extension (e.g., more servos via arrays)

## Pinout Diagram
```
RP2040 Pico Pins:
- GP0: UART TX (Modbus)
- GP1: UART RX (Modbus)
- GP4: I2C SDA (BME280)
- GP5: I2C SCL (BME280)
- GP6-10: Servos 0-4 (PWM)
- GP11-26: Relays 0-15 (Digital Out)
- GP? : Serial Debug (USB)
```
(Visual diagram: Use Fritzing or draw.io for wiring; BME280 VCC/GND to 3.3V/GND, Servos/Relays to 5V/GND as needed.)

## Setup & Build
1. Install PlatformIO in VSCode.
2. Open project in VSCode with PlatformIO extension.
3. Run `platformio run` to build.
4. Upload via `platformio run --target upload` (connect Pico in BOOTSEL mode).
5. Monitor debug via `platformio device monitor` (115200 baud).

## Testing
- **Unit**: Test each class init/set/read in isolation (e.g., Serial commands).
- **Integration**: Use Modbus master tool (e.g., Modbus Poll) to read/write coils/registers; verify servo/relay actions, sensor updates.
- **Simulation**: Mock sensor data if no hardware; add tests in `test/` dir.
- **Reliability**: Stress test with invalid Modbus requests, power cycles; check safe states.

## Maintenance
- Update configs in [`Config.h`](include/Config.h) for pins/baud/slave ID.
- Extend: Add more servos/relays by resizing arrays; implement RS485 DE/RE pin in pre/postTransmission.
- Firmware: Use UF2 for OTA-like updates via BOOTSEL.
- Troubleshooting: Check Serial output; ensure libraries installed via PlatformIO.

## License
MIT License.