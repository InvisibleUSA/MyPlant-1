/*
 * sensors.h
 *
 *  Created on: Aug 9, 2018
 *      Author: erik
 */

#ifndef SOURCE_SENSORS_H_
#define SOURCE_SENSORS_H_

#include <stdint.h>
#include "BCDS_Retcode.h"

Retcode_T initializeSensors(void);
uint8_t measureMoisture(void);
uint32_t measureLight(void);
Retcode_T measureEnvironment(uint8_t* humidity, int32_t* temperature, uint32_t* pressure);

#endif /* SOURCE_SENSORS_H_ */
