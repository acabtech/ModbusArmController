#ifndef MODBUSHANDLER_H
#define MODBUSHANDLER_H

#include <ModbusRtu.h>
#include "Config.h"
#include "ServoManager.h"
#include "RelayManager.h"
#include "BME280Reader.h"

class ModbusHandler {
private:
    Modbus slave;
    ServoManager* servoMgr;
    RelayManager* relayMgr;
    BME280Reader* sensor;
    uint16_t holdingRegs[20];  // Buffer for holding registers (e.g., servos 0-4, sensors 10-13)
    bool coilStates[16];       // Buffer for coils (relays 0-15)
    unsigned long lastSensorRead;
    bool initialized;

    void updateSensorData();
    uint8_t relayToCoil(int relayIndex) const { return relayIndex; }
    uint8_t servoToReg(int servoIndex) const { return servoIndex; }

public:
    ModbusHandler(ServoManager* sm, RelayManager* rm, BME280Reader* sr);
    ~ModbusHandler();

    bool init();
    void task();  // Poll for Modbus requests
    void preTransmission();
    void postTransmission();
    bool isInitialized() const { return initialized; }
};

#endif // MODBUSHANDLER_H