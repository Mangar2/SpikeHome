

#include "SpikeHome.h"
#include "SpikeSensors.h"
#include "yahamaster.h"
#include "Device.h"

const int SOFTWARE_VERSION      = 10;


void setup()
{
    uint16_t sensorType = Device::readSensorType();
    YahaMaster::setupSensors(SOFTWARE_VERSION); 
}

void loop()
{
    Schedule::nextTick();
}
