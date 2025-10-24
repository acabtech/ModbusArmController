#include <Arduino.h>
#include "Config.h"
#include "ServoManager.h"
#include "RelayManager.h"
#include "BME280Reader.h"
#include "ModbusHandler.h"

ServoManager servoMgr;
RelayManager relayMgr;
BME280Reader sensor;
ModbusHandler modbus(&servoMgr, &relayMgr, &sensor);

void setup() {
    Serial.begin(115200);
    while (!Serial) {
        ; // Wait for serial port to connect
    }
    Serial.println("Modbus Arm Controller Starting...");

    if (!servoMgr.init()) {
        Serial.println("ServoManager init failed");
    }
    if (!relayMgr.init()) {
        Serial.println("RelayManager init failed");
    }
    if (!sensor.init()) {
        Serial.println("BME280Reader init failed");
    }
    if (!modbus.init()) {
        Serial.println("ModbusHandler init failed");
    }

    Serial.println("Initialization complete.");
}

void loop() {
    modbus.task();
    // Additional non-blocking tasks can be added here if needed
    delay(10);  // Small delay to prevent overwhelming the loop
}