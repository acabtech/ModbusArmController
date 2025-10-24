#ifndef RELAYMANAGER_H
#define RELAYMANAGER_H

#include "Config.h"

class RelayManager {
private:
    bool relayStates[NUM_RELAYS];
    bool initialized;

public:
    RelayManager();
    ~RelayManager();

    bool init();
    void setRelay(int index, bool state);
    bool getRelay(int index) const;
    void setAllRelays(bool state);
    void setSafeState();
    bool isInitialized() const { return initialized; }
};

#endif // RELAYMANAGER_H