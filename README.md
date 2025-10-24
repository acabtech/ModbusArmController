# Modbus Arm Controller

A multifunction system based on Raspberry Pi Pico RP2040, implementing Modbus RTU over UART for controlling up to 5 servos, up to 16 relays, and reading data from a BME280 environmental sensor (temperature, humidity, pressure, altitude). Designed for long-term reliability and maintainability with modular architecture.

## Hardware Overview
- **Microcontroller**: Raspberry Pi Pico (RP2040)
- **Communication**: Modbus RTU over UART (Serial1, UART0 pins)
- **Actuators**:
  - 5 Servos (modular control via ServoManager)
  - 16 Relays (modular control via RelayManager, direct GPIO)
- **Sensor**: BME280 (I2C, reads temp, humidity, pressure, altitude)
- **Power**: Ensure stable 5V supply for Pico and peripherals; servos may require external power. Relays and BME280 can use 3.3V/5V as per specs. Use common ground.

## Pin Assignments
Defined in [`include/Config.h`](include/Config.h:1):
- **UART (Modbus RTU)**: TX = GP0, RX = GP1, Baud = 9600, 8N1
- **I2C (BME280)**: SDA = GP4, SCL = GP5, Address = 0x76 (default; check your module)
- **Servos**: GP6, GP7, GP8, GP9, GP10 (default position 90°)
- **Relays**: GP11 to GP26 (active HIGH, safe state LOW/OFF)
- **USB Serial (Debug)**: Default USB VCP at 9600 baud for logging and monitoring.

Note: GP2/GP3 are used for onboard LED if needed, but not assigned here. Avoid conflicts with other peripherals.

## Dependencies
This project uses PlatformIO for building and dependency management. The required libraries are specified in [`platformio.ini`](platformio.ini) and automatically installed during build.

### Core Libraries
- **Modbus-Master-Slave-for-Arduino**: For Modbus RTU slave implementation. Version ~2.0.1 (handles function codes 0x01, 0x03, 0x05, 0x06, 0x0F, 0x10).
- **Adafruit BME280 Library**: For BME280 sensor readings (temperature, humidity, pressure). Requires `Adafruit_BME280_Library` and `Adafruit_Unified_Sensor` (version ~1.1.4).
- **Servo Library**: Built-in Arduino library for PWM servo control.
- **Wire Library**: Built-in for I2C communication.
- **Arduino.h**: Core framework for RP2040 (Pico board support via PlatformIO).

### Installation
1. Ensure PlatformIO is installed in VSCode.
2. Open the project folder in VSCode.
3. PlatformIO will prompt to install libraries on first build (`platformio run`). Alternatively, run `platformio lib install` manually.
4. No additional pip/npm dependencies; all managed via PlatformIO.

If using Arduino IDE instead, manually install libraries via Library Manager and configure board as "Raspberry Pi Pico".

## Configuration
Configurations are centralized in [`include/Config.h`](include/Config.h) for easy customization without recompiling core logic.

### Key Configurable Parameters
- **Modbus Settings**:
  - Slave ID: Default 1 (change `MODBUS_SLAVE_ID`).
  - Baud Rate: Default 9600 (change `MODBUS_BAUD_RATE`).
  - UART Pins: TX/RX on GP0/GP1 (modify `MODBUS_TX_PIN`, `MODBUS_RX_PIN`).
- **Hardware Pins**:
  - Servos: Array of pins `SERVO_PINS[] = {6,7,8,9,10}`.
  - Relays: Array of pins `RELAY_PINS[] = {11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26}`.
  - I2C: SDA/SCL on GP4/GP5 (modify `I2C_SDA_PIN`, `I2C_SCL_PIN`).
  - BME280 Address: Default 0x76 (change `BME280_ADDRESS` if 0x77).
- **Sensor Update Interval**: Default 5 seconds (change `SENSOR_UPDATE_INTERVAL_MS` in main loop).
- **Servo Defaults**: Initial position 90° (midpoint); range constrained to 0-180°.

To configure:
1. Edit [`include/Config.h`](include/Config.h).
2. Rebuild and upload (`platformio run --target upload`).
3. For production, consider RS485 support by adding DE/RE pin control in ModbusHandler (e.g., GP27 for transceiver enable).

## Software Architecture
- **Framework**: Arduino for RP2040 (PlatformIO).
- **Libraries**: See Dependencies section.
- **Modular Classes** (in `include/` and `src/`):
  - [`Config.h`](include/Config.h): Central pins, settings, addresses.
  - [`ServoManager`](include/ServoManager.h): Initializes/sets servo positions, safe default 90° on init/fail.
  - [`RelayManager`](include/RelayManager.h): Initializes/sets relay states, safe OFF on init/fail.
  - [`BME280Reader`](include/BME280Reader.h): Initializes/reads sensor data (with error handling for init failures).
  - [`ModbusHandler`](include/ModbusHandler.h): Manages Modbus polling, syncs coils/registers to/from managers, periodic sensor reads.
- **Main Flow** ([`src/main.cpp`](src/main.cpp:1)):
  - `setup()`: Init Serial (debug at 9600 baud), managers (Servo, Relay, BME280), Modbus; log init status/errors.
  - `loop()`: Poll Modbus (handles incoming requests/responses), update sensor every 5s, 10ms non-blocking delay.
- **Reliability Features**:
  - Input validation: Servo angles clamped 0-180°, address bounds checks.
  - Safe states: Servos to 90° , relays OFF on constructor/destructor/init fail.
  - Logging: Serial output for init, errors, Modbus activity.
  - Watchdog: Not implemented (add via `#include <hardware/watchdog.h>` and `watchdog_enable()` for production).
  - Non-blocking: All operations in loop; no `delay()` in critical paths.
  - Maintainability: Modular classes for extension (e.g., add more servos by resizing arrays in Config.h).

## Modbus Protocol
- **Slave ID**: 1 (configurable).
- **Baud Rate/Format**: 9600, 8 data bits, No parity, 1 stop bit (8N1).
- **Function Codes Supported**:
  - **Coils (Relays, addresses 0-15)**:
    - 0x01 Read Coils: Read relay states (ON=1/OFF=0).
    - 0x05 Write Single Coil: Set single relay (0xFF00=ON, 0x0000=OFF).
    - 0x0F Write Multiple Coils: Set multiple relays (bitmask).
  - **Holding Registers**:
    - Addresses 0-4: Servo positions (0-180°).
    - Address 10: Temperature (°C x10, e.g., 232 for 23.2°C).
    - Address 11: Humidity (% x10, e.g., 452 for 45.2%).
    - Address 12: Pressure (Pa, integer).
    - Address 13: Altitude (m x100, e.g., 1050 for 10.50m).
    - 0x03 Read Holding Registers: Read servo positions or sensor data.
    - 0x06 Write Single Register: Set single servo position (0-180).
    - 0x10 Write Multiple Registers: Set multiple servo positions.
- Sensor data updated every 5s; reads are non-blocking.
- Error Handling: Standard Modbus exceptions (e.g., 0x02 Illegal Address, 0x03 Invalid Value) for out-of-bounds requests.

### Accessing Modbus Functions
Modbus communication occurs over UART0 (GP0 TX, GP1 RX). Connect a Modbus RTU master (e.g., PC with USB-to-UART adapter like FTDI) to these pins (TX master to RX Pico, RX master to TX Pico, common GND, 3.3V logic).

#### From Modbus to UART0 (Primary Interface)
1. **Hardware Setup**: Use a USB-to-TTL serial adapter (3.3V) connected to GP0/GP1. Set adapter to 9600 baud, 8N1.
2. **Software Tools**:
   - **Modbus Poll (Windows)** or **Modbus Slave Simulator**: Configure as master, slave ID=1, COM port (adapter), 9600 baud.
   - **qModMaster (Linux/Mac)**: Similar setup, select RTU mode.
   - **Python Script**: Use `pymodbus` library:
     ```python
     from pymodbus.client import ModbusSerialClient

     client = ModbusSerialClient(method='rtu', port='COM3', baudrate=9600, timeout=1)
     client.connect()

     # Read coils (relays 0-15)
     coils = client.read_coils(0, 16, slave=1)
     print(coils.bits)

     # Write single coil (turn relay 0 ON)
     client.write_coil(0, True, slave=1)

     # Read holding registers (servo 0 position)
     regs = client.read_holding_registers(0, 1, slave=1)
     print(regs.registers[0])  # 0-180

     # Write register (set servo 0 to 90°)
     client.write_register(0, 90, slave=1)

     # Read sensor (temp at addr 10)
     temp = client.read_holding_registers(10, 1, slave=1)
     print(f"Temperature: {temp.registers[0] / 10}°C")

     client.close()
     ```
3. **Verification**: Monitor Pico's USB Serial (9600 baud) for debug logs of Modbus activity/errors.

#### From USB VCP (Debug/Monitoring Interface)
USB VCP provides serial debug output at 9600 baud (via `Serial` in code). It does not directly handle Modbus commands; use it for:
- **Logging**: View init status, errors, sensor readings (if enabled), Modbus request summaries.
- **Manual Testing**: Currently, no command parser over USB. To add: Implement a simple serial command interface in `loop()` (e.g., parse "RELAY 0 ON" via `Serial.readString()` and call managers directly). Example extension:
  ```cpp
  if (Serial.available()) {
    String cmd = Serial.readString();
    if (cmd.startsWith("SERVO ")) { /* parse and set */ }
  }
  ```
- **Access**: Use `platformio device monitor` or Arduino Serial Monitor. Connect Pico via USB (no special hardware).
- **Limitations**: For full Modbus over USB, consider bridging UART to USB in hardware or adding a USB-Modbus converter. Debug serial is read-only for now.

For both interfaces, ensure no baud mismatch or wiring errors (use oscilloscope if issues).

## Pinout Diagram
```
RP2040 Pico Pins:
- GP0: UART TX (Modbus to external master)
- GP1: UART RX (Modbus from external master)
- GP4: I2C SDA (BME280)
- GP5: I2C SCL (BME280)
- GP6-10: Servos 0-4 (PWM, signal wire)
- GP11-26: Relays 0-15 (Digital Out, active HIGH)
- USB: Debug Serial (9600 baud, logging)
```
(Visual diagram: Use Fritzing or draw.io for wiring; BME280 VCC/GND to 3.3V/GND, Servos signal to pins + external 5V/GND, Relays to pins + external power if high-current.)

## Setup & Build
1. **Prerequisites**: VSCode with PlatformIO extension. Raspberry Pi Pico (RP2040) board.
2. Open project in VSCode (folder: ModbusArmController).
3. **Build**: Run `platformio run` in terminal (installs deps automatically).
4. **Upload**:
   - Hold BOOTSEL on Pico, connect USB, release.
   - Run `platformio run --target upload` (selects correct port).
   - For wireless/debug: Use Picoprobe or built-in USB.
5. **Monitor Debug**: `platformio device monitor` (9600 baud) to view Serial output.
6. **Clean**: `platformio run --target clean` if issues.

Troubleshoot: Check `platformio.ini` for board/framework; ensure Pico drivers (Zadig for Windows if needed).

## Usage
1. **Flash Firmware**: Follow Setup & Build.
2. **Connect Hardware**:
   - Wire BME280 to I2C pins.
   - Connect servos to PWM pins (signal only; power external).
   - Wire relays to GPIO pins.
   - For Modbus: Connect USB-to-UART to GP0/GP1.
3. **Power On**: Pico starts in safe state (servos 90°, relays OFF). Sensor reads begin.
4. **Interact**:
   - Use Modbus master tool/script to send requests (see Accessing Modbus Functions).
   - Monitor USB Serial for logs (e.g., "BME280 init OK", "Modbus request: FC 0x01").
5. **Runtime**: System loops indefinitely; sensor updates every 5s. No manual stop.

Example Workflow:
- Read all relays: Modbus 0x01, addr 0, qty 16.
- Set servo 1 to 45°: Modbus 0x06, addr 1, value 45.
- Query temp: Modbus 0x03, addr 10, qty 1 → divide by 10.

## Testing
### Unit Tests
- **ServoManager**: In `setup()`, test `servoManager.setPosition(0, 90)`; verify PWM output with multimeter/oscilloscope.
- **RelayManager**: Test `relayManager.setRelay(0, true)`; check voltage on pin.
- **BME280Reader**: Call `bmeReader.readAll()`; print to Serial (e.g., add temp print in loop for verification).
- **Isolation**: Temporarily comment out other managers in `main.cpp`; use Serial commands if added.

### Integration Tests
- **Modbus**: Use Modbus Poll:
  - Read coils 0-15: Should return current relay states.
  - Write coil 5 ON: Verify relay activates.
  - Read registers 0-4: Default 90.
  - Write register 2 to 180: Verify servo moves.
  - Read register 10: Valid temp after 5s (room ~23°C → 230).
- **Sensor**: Wait 5s, read via Modbus; compare with multimeter or known values.
- **Full System**: Power cycle; check safe states. Send invalid request (e.g., addr 20 coils) → error response.

### Simulation & Automated Tests
- **No Hardware**: Mock BME280 with fixed values in `BME280Reader.cpp` (override `readAll()`).
- **Add Tests Dir**: Create `test/main.cpp` with Unity framework (add to platformio.ini); unit test classes (e.g., `TEST_ASSERT_EQUAL(90, servo.getPosition(0))`).
- **Stress**: Flood with Modbus requests; check no crashes. Cycle power 10x; verify safe init.
- **Tools**: Oscilloscope for UART timing; logic analyzer for I2C/PWM.

Run: `platformio test` (if tests added).

## Maintenance
- **Update Config**: Edit [`include/Config.h`](include/Config.h); rebuild/upload.
- **Extend Functionality**:
  - More Servos/Relays: Resize arrays in Config.h; update Modbus address mappings in ModbusHandler.
  - RS485: Add DE/RE pin (e.g., GP27); implement in `ModbusHandler::preTransmission()`/`postTransmission()`.
  - Watchdog: Include `<hardware/watchdog.h>`; enable in setup for reset on hangs.
- **Firmware Updates**: Drag UF2 to Pico in BOOTSEL mode for OTA-like updates.
- **Troubleshooting**:
  - No Modbus Response: Check wiring/baud; monitor Serial for "Modbus init fail".
  - Sensor Errors: Verify I2C address/pullups; log `bmeReader.initStatus()`.
  - Build Issues: Update PlatformIO (`platformio update`); check lib versions.
  - Libraries: Reinstall via `platformio lib remove/install`.

## License
MIT License.