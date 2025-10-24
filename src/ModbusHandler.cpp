#include "ModbusHandler.h"
#include <Arduino.h>

ModbusHandler::ModbusHandler(ServoManager* sm, RelayManager* rm, BME280Reader* sr) 
    : servoMgr(sm), relayMgr(rm), sensor(sr), lastSensorRead(0), initialized(false) {
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
    Serial1.begin(MODBUS_BAUD_RATE, SERIAL_8N1, MODBUS_UART_RX_PIN, MODBUS_UART_TX_PIN);
    slave.begin(MODBUS_SLAVE_ID, Serial1);
    slave.addHreg(0, 20);  // Add 20 holding registers
    slave.addCoil(0, 16);  // Add 16 coils
    initialized = true;
    return true;
}

void ModbusHandler::task() {
    if (!initialized) return;

    uint8_t result = slave.poll();
    if (result == 0) {
        // Handle successful poll, update from/to buffers
        // Sync coils to relays
        for (int i = 0; i < NUM_RELAYS; i++) {
            bool newState = slave.Coil(0 + i);
            if (newState != coilStates[i]) {
                coilStates[i] = newState;
                relayMgr->setRelay(i, newState);
            }
        }

        // Sync holding regs to servos
        for (int i = 0; i < NUM_SERVOS; i++) {
            uint16_t regValue = slave.Hreg(SERVO_REG_START_ADDR + i);
            if (regValue != holdingRegs[SERVO_REG_START_ADDR + i]) {
                holdingRegs[SERVO_REG_START_ADDR + i] = regValue;
                servoMgr->setPosition(i, regValue);
            }
        }

        // Update sensor data periodically
        if (millis() - lastSensorRead > SENSOR_READ_INTERVAL_MS) {
            updateSensorData();
            lastSensorRead = millis();
        }

        // Sync sensor data to holding regs
        float temp, hum, press, alt;
        if (sensor->readSensor(temp, hum, press, alt)) {
            holdingRegs[TEMP_REG_ADDR] = (uint16_t)(temp * 10);  // x10 for precision
            holdingRegs[HUMIDITY_REG_ADDR] = (uint16_t)(hum * 10);
            holdingRegs[PRESSURE_REG_ADDR] = (uint16_t)(press);
            holdingRegs[ALTITUDE_REG_ADDR] = (uint16_t)(alt * 100);  // x100 for precision
            slave.Hreg(TEMP_REG_ADDR, holdingRegs[TEMP_REG_ADDR]);
            slave.Hreg(HUMIDITY_REG_ADDR, holdingRegs[HUMIDITY_REG_ADDR]);
            slave.Hreg(PRESSURE_REG_ADDR, holdingRegs[PRESSURE_REG_ADDR]);
            slave.Hreg(ALTITUDE_REG_ADDR, holdingRegs[ALTITUDE_REG_ADDR]);
        }
    }
}

void ModbusHandler::updateSensorData() {
    // This is called in task, but can be expanded if needed
}

void ModbusHandler::preTransmission() {
    digitalWrite(MODBUS_UART_TX_PIN, HIGH);  // Enable RS485 DE/RE if using
}

void ModbusHandler::postTransmission() {
    digitalWrite(MODBUS_UART_TX_PIN, LOW);   // Disable RS485 DE/RE
}