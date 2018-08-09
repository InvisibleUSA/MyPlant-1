/*
 * battery.c
 *
 *  Created on: Aug 9, 2018
 *      Author: erik
 */

#include "battery.h"

BSP_ChargeState_T getBatteryStatus(void)
{
	BSP_ChargeState_T state = BSP_XDK_CHARGE_STATUS_UNDEFINED;
	uint32_t voltage = 0;

	Retcode_T ret = BSP_Charger_BQ2407X_MeasureSignal(BSP_XDK_EXT_SENSE_VBAT_ADC, &voltage);
	if (RETCODE_SUCCESS != ret)
		return state;

	//printf("Voltage: %ld\r\n", output);

	ret = BSP_Charger_BQ2407X_CheckStatus(&state, voltage);
	if (RETCODE_SUCCESS != ret)
		return state;

	return state;
}

Retcode_T initBattery(void)
{
	Retcode_T ret = BSP_Charger_BQ2407X_Connect();
	if (RETCODE_SUCCESS != ret)
		return ret;

	ret = BSP_Charger_BQ2407X_Enable(BSP_XDK_CHARGING_SPEED_1);
	if (RETCODE_SUCCESS != ret)
		return ret;

	return ret;
}
