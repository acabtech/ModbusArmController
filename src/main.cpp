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

unsigned long lastBlinkTime = 0;
bool ledState = false;
void setup() {
    Serial.begin(MODBUS_BAUD_RATE);
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
    pinMode(25, OUTPUT);
}

void loop() {
    modbus.task();

    if (millis() - lastBlinkTime >= 100) {
        lastBlinkTime = millis();
        ledState = !ledState;
        digitalWrite(25, ledState);
    }


    delay(10);  //100Hz loop rate
}