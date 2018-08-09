/*
 * battery.h
 *
 *  Created on: Aug 9, 2018
 *      Author: erik
 */

#ifndef SOURCE_BATTERY_H_
#define SOURCE_BATTERY_H_

// Battery
#include "BCDS_BSP_Charger_BQ2407X.h"

Retcode_T initBattery(void);
BSP_ChargeState_T getBatteryStatus(void);

#endif /* SOURCE_BATTERY_H_ */
