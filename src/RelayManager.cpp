#include "RelayManager.h"
#include <Arduino.h>

RelayManager::RelayManager() : initialized(false) {
    for (int i = 0; i < NUM_RELAYS; i++) {
        relayStates[i] = false;
        pinMode(RELAY_PINS[i], OUTPUT);
        digitalWrite(RELAY_PINS[i], RELAY_SAFE_STATE);
    }
}

RelayManager::~RelayManager() {
    setSafeState();
}

bool RelayManager::init() {
    // Pins already set in constructor
    initialized = true;
    return true;
}

void RelayManager::setRelay(int index, bool state) {
    if (index >= 0 && index < NUM_RELAYS && initialized) {
        relayStates[index] = state;
        digitalWrite(RELAY_PINS[index], state ? HIGH : RELAY_SAFE_STATE);
    }
}

bool RelayManager::getRelay(int index) const {
    if (index >= 0 && index < NUM_RELAYS) {
        return relayStates[index];
    }
    return false;
}

void RelayManager::setAllRelays(bool state) {
    for (int i = 0; i < NUM_RELAYS; i++) {
        setRelay(i, state);
    }
}

void RelayManager::setSafeState() {
    setAllRelays(false);
}