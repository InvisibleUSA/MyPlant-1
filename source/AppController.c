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
#include "timer.h"
#include "XdkSensorHandle.h"
#include "netcfg.h"

static _u8 MACAddress[6];

static uint32_t measureLight(void)
{
	uint32_t value = 0;
	Retcode_T ret = LightSensor_readLuxData(xdkLightSensor_MAX44009_Handle, &value);
	if (RETCODE_SUCCESS != ret)
	{
		return UINT32_MAX;
	}
	return value / 1000;
}

static Retcode_T processEnvironment(uint32_t* humidity, uint32_t* temperature, uint32_t* pressure)
{
	Environmental_Data_T value = {0,0,0};

	Retcode_T ret = Environmental_readCompensatedData(xdkEnvironmental_BME280_Handle, &value);

	*humidity = value.humidity;
	*temperature = value.temperature;
	*pressure = value.pressure;

	return ret;
}

static void timerCallback(xTimerHandle th)
{
    BCDS_UNUSED(th);


    uint32_t humidity    = UINT32_MAX;
    uint32_t temperature = UINT32_MAX;
    uint32_t illuminance = UINT32_MAX;
    uint32_t pressure    = UINT32_MAX;

    bool success = true;


    illuminance = measureLight();
    success &= (illuminance != UINT32_MAX);

    if (RETCODE_SUCCESS != processEnvironment(&humidity, &temperature, &pressure))
    {
    	//TODO report error
    }
}

static Retcode_T initializeSensors()
{
	Retcode_T ret = RETCODE_UNINITIALIZED;

	// Light Sensor
	ret = LightSensor_init(xdkLightSensor_MAX44009_Handle);
	if (RETCODE_SUCCESS != ret)
		return ret;

	// Temperature, atmospheric humidity
	ret = Environmental_init(xdkEnvironmental_BME280_Handle);
	return ret;
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

	_u8 len = SL_MAC_ADDR_LEN;
	sl_NetCfgGet(SL_MAC_ADDRESS_GET, NULL, &len, (_u8*)&MACAddress);

    xTimerHandle th = xTimerCreate((const char* const) "measure", pdMS_TO_TICKS(5000), pdTRUE, NULL, timerCallback);
    if (NULL == th)
    {
    	Retcode_RaiseError(RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_NULL_POINTER));
    	assert(false);
    	return;
    }

    const uint32_t TIMERBLOCKTIME = UINT32_C(0xffff);
    xTimerStart(th, TIMERBLOCKTIME);
}
