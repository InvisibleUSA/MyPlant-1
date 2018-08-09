/*
 * mqtt.h
 *
 *  Created on: Aug 9, 2018
 *      Author: erik
 */

#ifndef SOURCE_MQTT_H_
#define SOURCE_MQTT_H_

#include <stdint.h>
#include <stdbool.h>
#include "BCDS_Retcode.h"


extern uint8_t MACAddress[6];
extern char MACAddressStr[2*6+1];

// publish data on a topic
// returns true on success
bool mqtt_publish(const char* topic, const char* msg);
Retcode_T mqtt_init(const char* SSID, const char* password, const char* mqtturl, uint16_t port);
#endif /* SOURCE_MQTT_H_ */
