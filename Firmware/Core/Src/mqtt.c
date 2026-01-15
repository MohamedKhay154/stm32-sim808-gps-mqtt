/*
 * mqtt.c
 *
 *  Created on: Nov 25, 2025
 *      Author: Mohamed
 */


#include "mqtt.h"
#include <string.h>
#include <stdio.h>

int MQTT_BuildConnectPacket(unsigned char *buf, int bufsize){

	MQTTPacket_connectData data = MQTTPacket_connectData_initializer;

	data.clientID.cstring = "stm32client";
	data.keepAliveInterval = 60;
	data.cleansession = 1;

	return MQTTSerialize_connect(buf, bufsize, &data);

}
int MQTT_BuildPublishPacket(unsigned char *buf, int bufsize,
                            const char *topic, const char *message){

	 MQTTString topicString = MQTTString_initializer;
	 topicString.cstring = (char*)topic;

	 return MQTTSerialize_publish(
	                buf,
	                bufsize,
	                0,                  // dup
	                0,                  // QoS 0
	                0,                  // retained
	                0,                  // message ID ignored for QoS0
	                topicString,
	                (unsigned char*)message,
	                strlen(message)
	       	   	   	 );

}
int MQTT_BuildSubscribePacket(unsigned char *buf, int bufsize, const char *topic){

	MQTTString topicString = MQTTString_initializer;
	topicString.cstring = (char*)topic;

	int qos = 0;
	int packetId = 1;      // any non-zero ID

	return MQTTSerialize_subscribe(
	            buf,
	            bufsize,
	            0,              // dup
	            packetId,       // Packet ID
	            1,              // # topics
	            &topicString,   // topic list
	            &qos            // qos array
	           );


}
