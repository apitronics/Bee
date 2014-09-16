
#include "BeeDevices.h"
#include <iostream>
using namespace std;
//this is for testing


int main(int argc, char const *argv[])
{
	cout << "begin";

	uint8_t sleep = 0;
	uint16_t sampleper = 10;
	uint16_t logper = 20;
	uint8_t rparams = 0;
	uint8_t wparams = 0;
	BeeDevice myBee(sleep, sampleper, logper, rparams, wparams);

	cout << "end";
	return 0;
}
