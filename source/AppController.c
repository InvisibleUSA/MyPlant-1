/*
 * Licensee agrees that the example code provided to Licensee has been developed and released by Bosch solely as an example to be used as a potential reference for Licensee�s application development.
 * Fitness and suitability of the example code for any use within Licensee�s applications need to be verified by Licensee on its own authority by taking appropriate state of the art actions and measures (e.g. by means of quality assurance measures).
 * Licensee shall be responsible for conducting the development of its applications as well as integration of parts of the example code into such applications, taking into account the state of the art of technology and any statutory regulations and provisions applicable for such applications. Compliance with the functional system requirements and testing there of (including validation of information/data security aspects and functional safety) and release shall be solely incumbent upon Licensee.
 * For the avoidance of doubt, Licensee shall be responsible and fully liable for the applications and any distribution of such applications into the market.
 *
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     (1) Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *     (2) Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *
 *     (3)The name of the author may not be used to
 *     endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 *  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *  DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 *  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 *  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 *  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 *  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 *  IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */
/*----------------------------------------------------------------------------*/
/**
 * @ingroup APPS_LIST
 *
 * @defgroup XDK_APPLICATION_TEMPLATE MyPlant
 * @{
 *
 * @brief XDK Application Template
 *
 * @details XDK Application Template without any functionality.
 * Could be used as a starting point to develop new application based on XDK platform.
 *
 * @file
 **/
/* module includes ********************************************************** */

/* own header files */
#include "XdkAppInfo.h"
#undef BCDS_MODULE_ID  /* Module ID define before including Basics package*/
#define BCDS_MODULE_ID XDK_APP_MODULE_ID_APP_CONTROLLER

#include "AppController.h"
#include <stdio.h>
#include "BCDS_CmdProcessor.h"
#include "FreeRTOS.h"
#include "timers.h"

#include "sensors.h"
#include "mqtt.h"
#include "battery.h"

#define TOPIC_LENGTH 25
char topicTemperature[TOPIC_LENGTH] = "temperature/";
char topicHumidity[TOPIC_LENGTH]    = "humidity/";
char topicMoisture[TOPIC_LENGTH]    = "moisture/";
char topicLight[TOPIC_LENGTH]       = "light/";
char topicPressure[TOPIC_LENGTH]    = "pressure/";
char topicBattery[TOPIC_LENGTH]     = "battery/";

static void initTopics(void)
{
	strncat(topicTemperature, (const char*)MACAddressStr, TOPIC_LENGTH);
	strncat(topicHumidity,    (const char*)MACAddressStr, TOPIC_LENGTH);
	strncat(topicMoisture,    (const char*)MACAddressStr, TOPIC_LENGTH);
	strncat(topicLight,       (const char*)MACAddressStr, TOPIC_LENGTH);
	strncat(topicPressure,    (const char*)MACAddressStr, TOPIC_LENGTH);
	strncat(topicBattery,     (const char*)MACAddressStr, TOPIC_LENGTH);
}

static void measureCallback(xTimerHandle th)
{
	BCDS_UNUSED(th);

	bool     success     = true;

	uint8_t  humidity    = UINT8_MAX;
	uint8_t  moisture    = UINT8_MAX;
	int32_t  temperature = INT32_MAX;
	uint32_t illuminance = UINT32_MAX;
	uint32_t pressure    = UINT32_MAX;


	illuminance = measureLight();
	success &= (illuminance != UINT32_MAX);


	if (RETCODE_SUCCESS != measureEnvironment(&humidity, &temperature, &pressure))
	{
		success = false;
		// TODO error reporting
	}

	moisture = measureMoisture();
	assert(moisture <= 100);
	assert(humidity <= 100);

	char buffer[20];

	snprintf(buffer, 20, "%ld", temperature);
	mqtt_publish(topicTemperature, buffer);
	snprintf(buffer, 20, "%lu", illuminance);
	mqtt_publish(topicLight, buffer);
	snprintf(buffer, 20, "%lu", pressure);
	mqtt_publish(topicPressure, buffer);
	snprintf(buffer, 20, "%u", humidity);
	mqtt_publish(topicHumidity, buffer);
	snprintf(buffer, 20, "%u", moisture);
	mqtt_publish(topicMoisture, buffer);
}

static void batteryCallback(xTimerHandle th)
{
	BCDS_UNUSED(th);
	BSP_ChargeState_T chargeState = getBatteryStatus();

	char* buffer;
	switch (chargeState)
	{
	case BSP_XDK_CHARGE_STATUS_ON:         /**< Battery is being charged */
		buffer = "110";
		break;
	case BSP_XDK_CHARGE_STATUS_CRITICAL:   /**< Battery voltage is critical */
		buffer = "10";
		break;
	case BSP_XDK_CHARGE_STATUS_LOW:        /**< Battery voltage is low */
		buffer = "30";
		break;
	case BSP_XDK_CHARGE_STATUS_NORMAL:     /**< Battery voltage is normal */
		buffer = "60";
		break;
	case BSP_XDK_CHARGE_STATUS_FULL:       /**< Battery voltage is full */
		buffer = "100";
		break;
	case BSP_XDK_CHARGE_STATUS_NO_BATTERY: /**< Battery could not be detected, */
	case BSP_XDK_CHARGE_STATUS_UNDEFINED:  /**< Charge status could not been defined */
	case BSP_XDK_CHARGE_STATUS_ERROR:      /**< if charger has not been enabled by calling enable function with correct parameters */
		buffer = "-10";
		break;
	}

	mqtt_publish(topicBattery, buffer);
}

void AppController_Init(void * cmdProcessorHandle, uint32_t param2)
{
	if (cmdProcessorHandle == NULL)
	{
		printf("Command processor handle is null \n\r");
		Retcode_RaiseError(RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_NULL_POINTER));
		assert(false);
		return;
	}
	BCDS_UNUSED(param2);

	Retcode_T ret = initializeSensors();
	if (RETCODE_SUCCESS != ret)
	{
		Retcode_RaiseError(ret);
		assert(false);
		return;
	}

	ret = initBattery();
	if (RETCODE_SUCCESS != ret)
	{
		Retcode_RaiseError(ret);
		assert(false);
		return;
	}

	ret = mqtt_init("EventiLoccioni", "welcometoloccioni", "192.168.33.161", 1883);
	if (RETCODE_SUCCESS != ret)
	{
		Retcode_RaiseError(ret);
		assert(false);
		return;
	}

	initTopics();

	xTimerHandle measuringtimer = xTimerCreate((const char* const) "measure", pdMS_TO_TICKS(60000), pdTRUE, NULL, measureCallback);
	if (NULL == measuringtimer)
	{
		Retcode_RaiseError(RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_NULL_POINTER));
		assert(false);
		return;
	}
	xTimerHandle batterytimer = xTimerCreate((const char* const) "battery", pdMS_TO_TICKS(120000), pdTRUE, NULL, batteryCallback);
	if (NULL == batterytimer)
	{
		Retcode_RaiseError(RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_NULL_POINTER));
		assert(false);
		return;
	}

	const uint32_t TIMERBLOCKTIME = UINT32_C(0xffff);
	xTimerStart(measuringtimer, TIMERBLOCKTIME);
	xTimerStart(batterytimer, TIMERBLOCKTIME);
}
