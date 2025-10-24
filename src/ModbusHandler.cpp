#include "ModbusHandler.h"
#include <Arduino.h>

ModbusHandler* ModbusHandler::instance = nullptr;

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
    usbSlave.addReg(COIL(0), (uint16_t)0, (uint16_t)16);
    usbSlave.addReg(HREG(0), (uint16_t)0, (uint16_t)20);

    // Callbacks for writes to relays and servos
    usbSlave.onSet(COIL(0), [](TRegister* reg, uint16_t val) -> uint16_t {
        bool state = val != 0;
        instance->relayMgr->setRelay(reg->address.address, state);
        return val;
    }, 16);

    usbSlave.onSet(HREG(0), [](TRegister* reg, uint16_t val) -> uint16_t {
        int index = reg->address.address;
        if (index < NUM_SERVOS) {
            instance->servoMgr->setPosition(index, val);
        }
        return val;
    }, NUM_SERVOS);

    // Ignore writes to sensor registers
    auto ignoreWrite = [](TRegister* reg, uint16_t val) -> uint16_t {
        return instance->usbSlave.Reg(reg->address);
    };
    usbSlave.onSet(HREG(TEMP_REG_ADDR), ignoreWrite, 1);
    usbSlave.onSet(HREG(HUMIDITY_REG_ADDR), ignoreWrite, 1);
    usbSlave.onSet(HREG(PRESSURE_REG_ADDR), ignoreWrite, 1);
    usbSlave.onSet(HREG(ALTITUDE_REG_ADDR), ignoreWrite, 1);

    Serial1.begin(MODBUS_BAUD_RATE);
    slave.begin(&Serial1, MODBUS_DE_PIN, true);
    slave.slave(MODBUS_SLAVE_ID);
    slave.addReg(COIL(0), (uint16_t)0, (uint16_t)16);
    slave.addReg(HREG(0), (uint16_t)0, (uint16_t)20);

    // Callbacks for writes to relays and servos
    slave.onSet(COIL(0), [](TRegister* reg, uint16_t val) -> uint16_t {
        bool state = val != 0;
        instance->relayMgr->setRelay(reg->address.address, state);
        return val;
    }, 16);

    slave.onSet(HREG(0), [](TRegister* reg, uint16_t val) -> uint16_t {
        int index = reg->address.address;
        if (index < NUM_SERVOS) {
            instance->servoMgr->setPosition(index, val);
        }
        return val;
    }, NUM_SERVOS);

    // Ignore writes to sensor registers
    auto ignoreWriteRS485 = [](TRegister* reg, uint16_t val) -> uint16_t {
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
            usbSlave.Reg(HREG(TEMP_REG_ADDR), (uint16_t)(temp * 10));
            usbSlave.Reg(HREG(HUMIDITY_REG_ADDR), (uint16_t)(hum * 10));
            usbSlave.Reg(HREG(PRESSURE_REG_ADDR), (uint16_t)(press));
            usbSlave.Reg(HREG(ALTITUDE_REG_ADDR), (uint16_t)(alt * 100));

            slave.Reg(HREG(TEMP_REG_ADDR), (uint16_t)(temp * 10));
            slave.Reg(HREG(HUMIDITY_REG_ADDR), (uint16_t)(hum * 10));
            slave.Reg(HREG(PRESSURE_REG_ADDR), (uint16_t)(press));
            slave.Reg(HREG(ALTITUDE_REG_ADDR), (uint16_t)(alt * 100));
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


