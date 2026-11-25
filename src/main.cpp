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
    // Debug logging over the USB CDC 'Serial' port is intentionally disabled:
    // Serial is the USB Modbus RTU transport (initialized in
    // ModbusHandler::init()). Human-readable output on that UART would corrupt
    // Modbus frames, so init failures are not reported here. Startup also
    // never blocks on the USB host being connected, so headless RS485
    // deployments boot and run Modbus without a serial monitor.

    servoMgr.init();
    relayMgr.init();
    sensor.init();
    modbus.init();

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