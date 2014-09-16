/*
 * Apitronics - DeviceHub.h
 * library for creating devices (sensors & actuators) for read/write commands from remote server
 *
 * Created: 9/15/2014
 *  Author: Colin Dignam
 *
 */

#ifndef DeviceHub_H
#define DeviceHub_H 

#include <inttypes.h>
#include <vector>

// #include <iostream>
using namespace std;


/******************************************************************************
* DeviceHub - Definitions
******************************************************************************/
class DeviceHub
{
public:
	DeviceHub(uint8_t numSensors, Sensor** sensorPointers, uint8_t numActuators, Actuator** actuatorPointers);
	~DeviceHub();



	//reads sensor values and/or actuator values and stores array
	void sampleDevices();


	bool sendIDpacket();
	bool sendDatapacket();

private:

	Sensor** _sensors;
	Actuator** _actuators;
	uint8_t _numsensors;
	uint8_t _numactuators;

	//two dimensional dynamic array for storing sample data
	//  	   	0        	1       	2        
	//	0	| 	UUID 	| 	UUID 	| 	UUID 	|  ...
	//	1	| datapoint | datapoint | datapoint |  ...
	//	2	| datapoint | datapoint | datapoint |  ...
	vector<vector<uint16_t> > sampleData; //two dimensional dynamic array

};















/*END DeviceHub_H */
#endif