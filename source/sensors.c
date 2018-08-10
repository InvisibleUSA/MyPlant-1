/*
 * sensors.c
 *
 *  Created on: Aug 9, 2018
 *      Author: erik
 */

#include "sensors.h"

// ADC
#include "em_gpio.h"
#include "BSP_BoardShared.h"
#include "em_adc.h"

// Sensors
#include "XdkSensorHandle.h"

uint32_t measureLight(void)
{
	uint32_t value = 0;
	Retcode_T ret = LightSensor_readLuxData(xdkLightSensor_MAX44009_Handle, &value);
	if (RETCODE_SUCCESS != ret)
	{
		return UINT32_MAX;
	}
	return value / 1000;
}

Retcode_T measureEnvironment(uint8_t* humidity, int32_t* temperature, uint32_t* pressure)
{
	Environmental_Data_T value = {0,0,0};

	Retcode_T ret = Environmental_readCompensatedData(xdkEnvironmental_BME280_Handle, &value);

	*humidity = value.humidity;
	*temperature = value.temperature;
	*pressure = value.pressure / 100;

	return ret;
}

#include "BCDS_BSP_Charger_BQ2407X.h"
uint8_t measureMoisture(void)
{
	// ADC for external sensor
	GPIO_PinModeSet(gpioPortD, 6, gpioModeInputPull, 0);
	GPIO_PinOutClear(gpioPortD, 6);

	ADC_InitSingle_TypeDef channelInit = ADC_INITSINGLE_DEFAULT;
	channelInit.reference = adcRef2V5;
	channelInit.resolution = adcRes12Bit;
	channelInit.input = adcSingleInpCh6;
    ADC_InitSingle(ADC0, &channelInit);


	uint32_t AdcSample = 0;

	while ((ADC0->STATUS & (ADC_STATUS_SINGLEACT)) && (BSP_UNLOCKED == ADCLock));

	__disable_irq();
	ADCLock = BSP_LOCKED;
	__enable_irq();

	ADC_Start(ADC0, adcStartSingle);

	// Wait while conversion is in process
	while (ADC0->STATUS & (ADC_STATUS_SINGLEACT));

	AdcSample = 0xFFF & ADC_DataSingleGet(ADC0);

	//printf("Measured value = %lu/4096 ; %.2f/2.5V; %lu%%/100%%\n\r", AdcSample, (float) AdcSample * 2.5 / 4096, AdcSample * 100 / 2300);

	__disable_irq();
	ADCLock = BSP_UNLOCKED;
	__enable_irq();

	AdcSample = AdcSample * 100 / 2300;
	return (AdcSample > 100)? 100: AdcSample;
}

Retcode_T initializeSensors(void)
{
	Retcode_T ret = RETCODE_UNINITIALIZED;

	// Light Sensor
	ret = LightSensor_init(xdkLightSensor_MAX44009_Handle);
	if (RETCODE_SUCCESS != ret)
		return ret;

	// Temperature, atmospheric humidity
	ret = Environmental_init(xdkEnvironmental_BME280_Handle);
	if (RETCODE_SUCCESS != ret)
			return ret;

	return ret;
}

