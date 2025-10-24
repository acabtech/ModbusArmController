#include "BME280Reader.h"
#include <math.h>

BME280Reader::BME280Reader() : initialized(false) {
}

BME280Reader::~BME280Reader() {
}

bool BME280Reader::init() {
    Wire.begin();
    if (!bme.begin(BME280_I2C_ADDRESS)) {
        // Log error if needed
        return false;
    }
    initialized = true;
    return true;
}

bool BME280Reader::readSensor(float& temperature, float& humidity, float& pressure, float& altitude) {
    if (!initialized) return false;
    temperature = bme.readTemperature();
    humidity = bme.readHumidity();
    pressure = bme.readPressure();
    altitude = bme.readAltitude(1013.25);  // Standard sea level pressure in hPa
    return !isnan(temperature) && !isnan(humidity) && !isnan(pressure) && !isnan(altitude);
}