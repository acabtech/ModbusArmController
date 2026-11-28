#include "ModbusHandler.h"
#include <Arduino.h>

ModbusHandler* ModbusHandler::instance = nullptr;

namespace {
uint16_t toRegI16(float value) {
    // Route signed values through int16_t so two's-complement encoding is
    // defined for sub-zero temperatures and below-sea-level altitudes.
    return (uint16_t)(int16_t)(value);
}
}

ModbusHandler::ModbusHandler(ServoManager* sm, RelayManager* rm, BME280Reader* sr)
    : servoMgr(sm), relayMgr(rm), sensor(sr), lastSensorRead(0), initialized(false) {
    instance = this;
    // Initialize buffers
    for (int i = 0; i < 20; i++) {
        holdingRegs[i] = 0;
    }
    for (int i = 0; i < 16; i++) {
        coilStates[i] = false;
    }
}

ModbusHandler::~ModbusHandler() {
}

bool ModbusHandler::init() {
    Serial.begin(MODBUS_BAUD_RATE);
    usbSlave.begin(&Serial, -1);
    usbSlave.slave(MODBUS_SLAVE_ID);
    // Only expose implemented coils/registers so writes to unmapped addresses
    // return a Modbus illegal-address exception instead of being silently ACKed.
    usbSlave.addReg(COIL(0), (uint16_t)0, (uint16_t)NUM_RELAYS);
    usbSlave.addReg(HREG(0), (uint16_t)0, (uint16_t)NUM_SERVOS);
    usbSlave.addReg(HREG(TEMP_REG_ADDR), (uint16_t)0, (uint16_t)4);

    // Seed servo holding registers to the default position so a Modbus master
    // reads a state that matches the hardware (servos are commanded to
    // SERVO_DEFAULT_POSITION in ServoManager::init()).
    for (int i = 0; i < NUM_SERVOS; i++) {
        usbSlave.Reg(HREG(i), SERVO_DEFAULT_POSITION);
    }

    // Callbacks for writes to relays and servos. Mirror accepted writes into
    // the other slave bank so both interfaces expose the same state. Reg()
    // writes the bank directly and does not trigger onSet, so no recursion.
    usbSlave.onSet(COIL(0), [](TRegister* reg, uint16_t val) -> uint16_t {
        uint16_t index = reg->address.address;
        bool state = val != 0;
        instance->relayMgr->setRelay(index, state);
        instance->slave.Reg(COIL(index), state ? (uint16_t)0xFF00 : (uint16_t)0x0000);
        return state ? (uint16_t)0xFF00 : (uint16_t)0x0000;
    }, NUM_RELAYS);

    usbSlave.onSet(HREG(0), [](TRegister* reg, uint16_t val) -> uint16_t {
        uint16_t index = reg->address.address;
        if (index < NUM_SERVOS) {
            uint16_t clamped = (uint16_t)constrain(val, 0, 180);
            instance->servoMgr->setPosition(index, clamped);
            instance->slave.Reg(HREG(index), clamped);
            return clamped;
        }
        return val;
    }, NUM_SERVOS);

    // Ignore writes to sensor registers
    auto ignoreWrite = [](TRegister* reg, uint16_t val) -> uint16_t {
        (void)val;
        return instance->usbSlave.Reg(reg->address);
    };
    usbSlave.onSet(HREG(TEMP_REG_ADDR), ignoreWrite, 1);
    usbSlave.onSet(HREG(HUMIDITY_REG_ADDR), ignoreWrite, 1);
    usbSlave.onSet(HREG(PRESSURE_REG_ADDR), ignoreWrite, 1);
    usbSlave.onSet(HREG(ALTITUDE_REG_ADDR), ignoreWrite, 1);

    Serial1.setTX(MODBUS_UART_TX_PIN);
    Serial1.setRX(MODBUS_UART_RX_PIN);
    Serial1.begin(MODBUS_BAUD_RATE);
    slave.begin(&Serial1, MODBUS_DE_PIN, true);
    slave.slave(MODBUS_SLAVE_ID);
    slave.addReg(COIL(0), (uint16_t)0, (uint16_t)NUM_RELAYS);
    slave.addReg(HREG(0), (uint16_t)0, (uint16_t)NUM_SERVOS);
    slave.addReg(HREG(TEMP_REG_ADDR), (uint16_t)0, (uint16_t)4);

    // Seed servo holding registers to the default position so a Modbus master
    // reads a state that matches the hardware (servos are commanded to
    // SERVO_DEFAULT_POSITION in ServoManager::init()).
    for (int i = 0; i < NUM_SERVOS; i++) {
        slave.Reg(HREG(i), SERVO_DEFAULT_POSITION);
    }

    // Callbacks for writes to relays and servos. Mirror accepted writes into
    // the other slave bank so both interfaces expose the same state. Reg()
    // writes the bank directly and does not trigger onSet, so no recursion.
    slave.onSet(COIL(0), [](TRegister* reg, uint16_t val) -> uint16_t {
        uint16_t index = reg->address.address;
        bool state = val != 0;
        instance->relayMgr->setRelay(index, state);
        instance->usbSlave.Reg(COIL(index), state ? (uint16_t)0xFF00 : (uint16_t)0x0000);
        return state ? (uint16_t)0xFF00 : (uint16_t)0x0000;
    }, NUM_RELAYS);

    slave.onSet(HREG(0), [](TRegister* reg, uint16_t val) -> uint16_t {
        uint16_t index = reg->address.address;
        if (index < NUM_SERVOS) {
            uint16_t clamped = (uint16_t)constrain(val, 0, 180);
            instance->servoMgr->setPosition(index, clamped);
            instance->usbSlave.Reg(HREG(index), clamped);
            return clamped;
        }
        return val;
    }, NUM_SERVOS);

    // Ignore writes to sensor registers
    auto ignoreWriteRS485 = [](TRegister* reg, uint16_t val) -> uint16_t {
        (void)val;
        return instance->slave.Reg(reg->address);
    };
    slave.onSet(HREG(TEMP_REG_ADDR), ignoreWriteRS485, 1);
    slave.onSet(HREG(HUMIDITY_REG_ADDR), ignoreWriteRS485, 1);
    slave.onSet(HREG(PRESSURE_REG_ADDR), ignoreWriteRS485, 1);
    slave.onSet(HREG(ALTITUDE_REG_ADDR), ignoreWriteRS485, 1);

    initialized = true;
    return true;
}

void ModbusHandler::task() {
    if (!initialized) return;

    usbSlave.task();
    slave.task();

    // Update sensor data periodically
    if (millis() - lastSensorRead > SENSOR_READ_INTERVAL_MS) {
        float temp, hum, press, alt;
        if (sensor->readSensor(temp, hum, press, alt)) {
            uint16_t tempReg = toRegI16(temp * 10);
            uint16_t humReg = (uint16_t)(hum * 10);
            // Adafruit BME280 readPressure() returns hPa; publish directly as an
            // integer hPa value (fits uint16_t). Altitude is published as
            // decimetres (m*10) to keep sub-metre resolution in uint16_t.
            uint16_t pressReg = (uint16_t)(press + 0.5f);
            uint16_t altReg = toRegI16(alt * 10.0f);

            usbSlave.Reg(HREG(TEMP_REG_ADDR), tempReg);
            usbSlave.Reg(HREG(HUMIDITY_REG_ADDR), humReg);
            usbSlave.Reg(HREG(PRESSURE_REG_ADDR), pressReg);
            usbSlave.Reg(HREG(ALTITUDE_REG_ADDR), altReg);

            slave.Reg(HREG(TEMP_REG_ADDR), tempReg);
            slave.Reg(HREG(HUMIDITY_REG_ADDR), humReg);
            slave.Reg(HREG(PRESSURE_REG_ADDR), pressReg);
            slave.Reg(HREG(ALTITUDE_REG_ADDR), altReg);
        }
        lastSensorRead = millis();
    }

#ifdef DEBUG
    while (Serial.available()) {
        Serial.write(Serial.read());
    }
    while (Serial1.available()) {
        Serial1.write(Serial1.read());
    }
#endif
}
