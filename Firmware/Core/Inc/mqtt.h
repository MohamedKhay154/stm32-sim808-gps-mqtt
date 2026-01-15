/*
 * mqtt.h
 *
 *  Created on: Nov 25, 2025
 *      Author: Mohamed
 */

#ifndef SRC_MQTT_H_
#define SRC_MQTT_H_



#endif /* SRC_MQTT_H_ */

#include "main.h"
#include "MQTTPacket.h"
#include "MQTTConnect.h"
#include <stdint.h>
#include <stdbool.h>


int MQTT_BuildConnectPacket(unsigned char *buf, int bufsize);
int MQTT_BuildPublishPacket(unsigned char *buf, int bufsize,
                            const char *topic, const char *message);
int MQTT_BuildSubscribePacket(unsigned char *buf, int bufsize, const char *topic);
