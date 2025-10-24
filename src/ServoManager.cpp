#include "ServoManager.h"

ServoManager::ServoManager() : initialized(false) {
}

ServoManager::~ServoManager() {
    setAllToDefault();
}

bool ServoManager::init() {
    for (int i = 0; i < NUM_SERVOS; i++) {
        if (!servos[i].attach(SERVO_PINS[i])) {
            // Log error if needed
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