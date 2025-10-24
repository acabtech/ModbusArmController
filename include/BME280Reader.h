#ifndef BME280READER_H
#define BME280READER_H

#include <Adafruit_BME280.h>
#include <Wire.h>
#include "Config.h"

class BME280Reader {
private:
    Adafruit_BME280 bme;
    bool initialized;

public:
    BME280Reader();
    ~BME280Reader();

    bool init();
    bool readSensor(float& temperature, float& humidity, float& pressure, float& altitude);
    bool isInitialized() const { return initialized; }
};

#endif // BME280READER_H