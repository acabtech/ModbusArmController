# Testing Strategy for Modbus Arm Controller

This document outlines the testing approach for the multifunction system to ensure reliability, correctness, and maintainability. Tests are divided into unit, integration, and system levels, focusing on modular components. Use PlatformIO's test framework or Arduino sketches for execution. Run tests on hardware (Pico with peripherals) or simulation (mock hardware via software).

## Unit Tests
Test individual classes in isolation. Create separate Arduino sketches or use a test runner like Unity/AutoTest for RP2040.

1. **ServoManager Tests** ([`include/ServoManager.h`](include/ServoManager.h)):
   - Init: Verify all servos attach to pins GP6-10; check failure if invalid pin.
   - setPosition: Test bounds (0-180° constrain), read back via getPosition.
   - setAllToDefault: All positions = 90°.
   - Edge: Index out of bounds (0-2), destruct calls default.
   - Mock: Use dummy Servo objects if no hardware.

2. **RelayManager Tests** ([`include/RelayManager.h`](include/RelayManager.h)):
   - Init: Pins GP11-26 set OUTPUT, initial LOW (safe).
   - setRelay: Toggle HIGH/LOW, verify getRelay state.
   - setAllRelays: All ON/OFF.
   - setSafeState: All OFF.
   - Edge: Index out of bounds (0-15), destruct calls safe state.

3. **BME280Reader Tests** ([`include/BME280Reader.h`](include/BME280Reader.h)):
   - Init: I2C begin on GP4/5, sensor detect at 0x76; fail if not found.
   - readSensor: Valid readings (temp/hum/press/alt); check !readFailed().
   - Edge: Init fail (wrong address), read without init returns false.
   - Simulation: Mock Adafruit_BME280 with fixed values (e.g., 25°C, 50%, 1013hPa, 0m).

4. **ModbusHandler Tests** ([`include/ModbusHandler.h`](include/ModbusHandler.h)):
   - Init: Serial1 begin, slave setup with ID 1, add real Hregs (servos 0-2, sensors 10-13) / relay coils.
   - task: Poll returns 0; sync coils to relays, regs to servos.
   - Sensor update: Every 5s, read and set Hregs 10-13.
   - pre/postTransmission: TX pin toggle (if RS485).
   - Mock: Fake Modbus requests via direct slave.Coil/Hreg calls.

**Tools**: Arduino IDE for sketches, or PlatformIO test/ dir with pio test.

## Integration Tests
Test interactions between modules via Modbus commands. Use external Modbus master (e.g., Modbus Poll on PC connected via USB-UART adapter).

1. **Modbus Coils (Relays)**:
   - Write coil 0 ON (0x05/0x15): Verify relay 0 HIGH, getRelay true.
   - Read coils 0-15 (0x01): Matches relayStates.
   - Multiple writes: Set 0-15 ON, read back all 1s.

2. **Modbus Holding Registers (Servos/Sensors)**:
   - Write reg 0 = 45 (0x06/0x10): Servo 0 to 45°, getPosition 45.
   - Read regs 0-2: Current servo positions.
   - Read regs 10-13: Sensor values update every 5s; verify scaling (temp x10, etc.).
   - Write invalid (e.g., 200°): Constrained to 180°.

3. **Full Flow**:
   - Init all: No failures logged.
   - Loop: Continuous polling, no blocking >10ms.
   - Sensor poll: Hregs update without Modbus request.

**Setup**: Connect master to UART (GP0/1), baud 9600. Monitor Serial for logs.

## System Tests
End-to-end on hardware.

1. **Reliability**:
   - Power cycle: Safe states on boot (servos 90°, relays OFF).
   - Invalid Modbus: Error responses (e.g., illegal address), no crash.
   - Overload: Rapid writes (100/sec), no missed updates.
   - Long-run: 24h operation, sensor reads stable, no memory leaks.

2. **Simulation**:
   - No hardware: Mock classes (e.g., fake servo write logs position).
   - Fault injection: Disconnect I2C, verify init fail log, skip reads.

3. **Performance**:
   - Modbus response time <100ms.
   - CPU usage: FreeRTOS if added, or simple loop timing.

## Test Execution
- **Build/Test**: `platformio test` (configure in platformio.ini).
- **Coverage**: Aim 80% on core functions; use gcov if possible.
- **Automation**: CI via GitHub Actions with PlatformIO.
- **Hardware**: Pico + breadboard (servos, relays, BME280); USB for Serial.
- **Debug**: Serial logs at 115200; add more in classes for failures.

Update tests as features evolve. Run before/after changes for regression.
