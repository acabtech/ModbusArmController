#ifndef SERVOMANAGER_H
#define SERVOMANAGER_H

#include <Servo.h>
#include "Config.h"

class ServoManager {
private:
    Servo servos[NUM_SERVOS];
    bool initialized;

public:
    ServoManager();
    ~ServoManager();

    bool init();
    void setPosition(int index, int degrees);
    int getPosition(int index);
    void setAllToDefault();
    bool isInitialized() const { return initialized; }
};

#endif // SERVOMANAGER_H