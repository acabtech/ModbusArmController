#ifndef MODBUSHANDLER_H
#define MODBUSHANDLER_H

#include <Arduino.h>
#include <stdint.h>
#include <ModbusRTU.h>
#include "Config.h"
#include "ServoManager.h"
#include "RelayManager.h"
#include "BME280Reader.h"

class ModbusHandler {
private:
    static ModbusHandler* instance;
    ModbusRTU slave;
    ModbusRTU usbSlave;
    ServoManager* servoMgr;
    RelayManager* relayMgr;
    BME280Reader* sensor;
    uint16_t holdingRegs[20];  // Buffer for holding registers (e.g., servos 0-4, sensors 10-13)
    bool coilStates[NUM_RELAYS];       // Buffer for coils (relays)
    unsigned long lastSensorRead;
    bool initialized;

    uint16_t relayToCoil(int relayIndex) const { return relayIndex; }
    uint16_t servoToReg(int servoIndex) const { return servoIndex; }

public:
    ModbusHandler(ServoManager* sm, RelayManager* rm, BME280Reader* sr);
    ~ModbusHandler();

    bool init();
    void task();  // Poll for Modbus requests
    void preTransmission() {}
    void postTransmission() {}
    bool isInitialized() const { return initialized; }
};

#endif // MODBUSHANDLER_H