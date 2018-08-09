/*
 * mqtt.c
 *
 *  Created on: Aug 9, 2018
 *      Author: erik
 */

#include "mqtt.h"

#include "Serval_Mqtt.h"
#include "BCDS_WlanConnect.h"
#include "BCDS_NetworkConfig.h"
#include "PAL_initialize_ih.h"
#include "PAL_socketMonitor_ih.h"
#include "Serval_Network.h"

uint8_t MACAddress[6] = { 0 };
char MACAddressStr[2*6+1] = "";

static MqttSession_T session;
static MqttSession_T *session_ptr = &session;

static Retcode_T connectWifi(const char* SSID, const char* pw)
{
	WlanConnect_SSID_T connectSSID = (WlanConnect_SSID_T) SSID;
	WlanConnect_PassPhrase_T connectPassPhrase = (WlanConnect_PassPhrase_T) pw;
	return WlanConnect_WPA(connectSSID, connectPassPhrase, NULL);
}

/*static void handleConnection(MqttConnectionEstablishedEvent_T connectionData){
  int rc_connect = (int) connectionData.connectReturnCode;
  printf("Connection Event:\n\r"
          "\tServer Return Code: %d (0 for success)\n\r",
          (int) rc_connect);
}*/

/*static void subscribe(void)
{
  static const char *sub_topic = "summerschool/temp";
  static StringDescr_T subscription_topics[1];
  static Mqtt_qos_t qos[1] = {MQTT_QOS_AT_MOST_ONE};
  StringDescr_wrap(&(subscription_topics[0]), sub_topic);
  Mqtt_subscribe(session_ptr, 1, subscription_topics, qos);
}*/

/*static void handleIncomingPublish(MqttPublishData_T publishData)
{
	int topic_length = publishData.topic.length + 1;
	int data_length = publishData.length + 1;
	char published_topic_buffer[topic_length];
	char published_data_buffer[data_length];
	snprintf(published_topic_buffer, topic_length, publishData.topic.start);
	snprintf(published_data_buffer, data_length, (char *) publishData.payload);
	printf("Incoming Published Message:\n\r"
			"\tTopic: %s\n\r"
			"\tPayload: %s\n\r", published_topic_buffer, published_data_buffer);
}*/

bool mqtt_publish(const char* topic, const char* msg)
{
	static StringDescr_T pub_topic_descr;
	StringDescr_wrap(&pub_topic_descr, topic);

	retcode_t retc = Mqtt_publish(session_ptr,
			pub_topic_descr,
			msg,
			strlen(msg),
			MQTT_QOS_AT_MOST_ONE,
			false);
	return RC_OK == retc;
}

static retcode_t event_handler(MqttSession_T* session, MqttEvent_t event,
              const MqttEventData_t* eventData)
{
	BCDS_UNUSED(session);
	printf("Event handler called\n\r");
	switch(event)
	{
	case MQTT_CONNECTION_ESTABLISHED:
		//handleConnection(eventData->connect);
		// subscribing and publishing can now be done
		//subscribe();
		break;
	case MQTT_CONNECTION_ERROR:
		//handleConnection(eventData->connect);
		break;
	case MQTT_INCOMING_PUBLISH:
		//handleIncomingPublish(eventData->publish);
		break;
	case MQTT_SUBSCRIPTION_ACKNOWLEDGED:
		printf("Subscription Successful\n\r");
		//publish();
		break;
	case MQTT_PUBLISHED_DATA:
		printf("Publish Successful\n\r");
		break;
	default:
		printf("Unhandled MQTT Event: %d\n\r", event);
		break;
	}
	return RC_OK;
}

enum UrlOrIp { URL, IP };

static enum UrlOrIp isUrlOrIp(const char* str)
{
	// IPs in dotted decimal are a maximum of 15 characters long: 255.255.255.255
	if (strlen(str) > 15)
		return URL;
	for (uint16_t i = 0; i < strlen(str); ++i)
	{
		if ((      str[i] < '0'
				|| str[i] > '9')
				&& (str[i] != '.'))
			return URL;
	}

	return IP;
}

static void mqttSetTarget(const char* url, uint16_t port)
{
	Ip_Address_T ip;
    //const char* ipstr = "192.168.33.161";
	char server_ip_buffer[16];
	static char	mqtt_broker[64];
	const char* mqtt_broker_format = "mqtt://%s:%d";

	if (isUrlOrIp(url) == URL)
	{
		PAL_getIpaddress((uint8_t *) url, &ip);
     	Ip_convertAddrToString(&ip, server_ip_buffer);
	}
	else
	{
		strcpy(server_ip_buffer, url);
	}

	sprintf(mqtt_broker, mqtt_broker_format, server_ip_buffer, port);
	SupportedUrl_fromString(mqtt_broker,(uint16_t) strlen(mqtt_broker), &session_ptr->target);
}

static void mqttSetConnectData(void)
{
	session_ptr->MQTTVersion = 3;
	session_ptr->keepAliveInterval = 100;
	session_ptr->cleanSession = true;
	session_ptr->will.haveWill = false;

	StringDescr_T device_name_descr;
	StringDescr_wrap(&device_name_descr, MACAddressStr);
	session_ptr->clientID = device_name_descr;
}

Retcode_T mqtt_init(const char* SSID, const char* password, const char* mqtturl, uint16_t port)
{
	Retcode_T ret = RETCODE_UNINITIALIZED;

	// initialize wifi and dhcp
	ret = WlanConnect_Init();
	if (RETCODE_SUCCESS != ret)
		return ret;
	ret = NetworkConfig_SetIpDhcp(NULL);
	if (RETCODE_SUCCESS != ret)
			return ret;



	// connect to wifi
	ret = connectWifi(SSID, password);
	if (RETCODE_SUCCESS != ret)
		return ret;



	// initialize dns services
	ret = PAL_initialize();
	if (RETCODE_SUCCESS != ret)
		return ret;
	PAL_socketMonitorInit();



	// get mac address
	_u8 len = SL_MAC_ADDR_LEN;
	ret = sl_NetCfgGet(SL_MAC_ADDRESS_GET, NULL, &len, (_u8*)&MACAddress);
	if (SL_RET_CODE_OK != ret)
		return ret;
	sprintf(MACAddressStr, "%02x%02x%02x%02x%02x%02x",
				(uint16_t)MACAddress[0], (uint16_t)MACAddress[1],
				(uint16_t)MACAddress[2], (uint16_t)MACAddress[3],
				(uint16_t)MACAddress[4], (uint16_t)MACAddress[5]);



	// initialize mqtt and connect
	retcode_t retc = Mqtt_initialize();
	if (RC_OK != retc)
		return retc;
	session_ptr = &session;
	retc = Mqtt_initializeInternalSession(session_ptr);
	if (RC_OK != retc)
		return retc;
	mqttSetTarget(mqtturl, port);
	mqttSetConnectData();
	session_ptr->onMqttEvent = event_handler;
	retc = Mqtt_connect(session_ptr);
	if (RC_OK != retc)
		return retc;

	return ret;
}
