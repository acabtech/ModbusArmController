#include <Arduino.h>
#include "ServoManager.h"

ServoManager::ServoManager() : initialized(false) {
}

ServoManager::~ServoManager() {
    setAllToDefault();
}

bool ServoManager::init() {
    for (int i = 0; i < NUM_SERVOS; i++) {
        // attach() returns the channel number (0 for the first servo) on
        // success and INVALID_SERVO (255) on failure, so it must not be
        // used as a boolean -- otherwise the first servo aborts init().
        if (servos[i].attach(SERVO_PINS[i]) == INVALID_SERVO) {
            // Log error if needed
            // Roll back any servos already attached in this attempt.
            for (int j = 0; j < i; j++) {
                servos[j].detach();
            }
            return false;
        }
        servos[i].write(SERVO_DEFAULT_POSITION);
    }
    initialized = true;
    return true;
}

void ServoManager::setPosition(int index, int degrees) {
    if (index >= 0 && index < NUM_SERVOS && initialized) {
        degrees = constrain(degrees, 0, 180);
        servos[index].write(degrees);
    }
}

int ServoManager::getPosition(int index) {
    if (index >= 0 && index < NUM_SERVOS && initialized) {
        return servos[index].read();
    }
    return -1;  // Error indicator
}

void ServoManager::setAllToDefault() {
    for (int i = 0; i < NUM_SERVOS; i++) {
        setPosition(i, SERVO_DEFAULT_POSITION);
    }
}