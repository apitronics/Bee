/*
 * Apitronics - DeviceHub.cpp
 * library for creating devices (sensors & actuators) for read/write commands from remote server
 *
 * Created: 9/15/2014
 *  Author: Colin Dignam
 *
 */


#include "DeviceHub.h"

/******************************************************************************
* DeviceHub
******************************************************************************/
DeviceHub::DeviceHub(uint8_t numSensors, Sensor** sensorPointers, const uint8_t numActuators, Actuator** actuatorPointers)
{
	_numsensors = numSensors;
	_sensors = sensorPointers;
	_numactuators = numActuators;
	_actuators = actuatorPointers;

	vector<int> UUIDrow;
	for (int i = 0; i < _numsensors; ++i)
	{
		UUIDrow.push_back(_sensors[i]->getUUID());
	}
}



void DeviceHub::sampleDevices()
{
	for (int i = 0; i < _numsensors; ++i)
	{
		_sensors[i]->getData();
	}
	for (int i = 0; i < _numactuators; ++i)
	{
		_actuators[i]->getData();
	}
}






















//testing DEBUG
int main(int argc, char const *argv[])
{
	
	return 0;
}
//end DEBUG



