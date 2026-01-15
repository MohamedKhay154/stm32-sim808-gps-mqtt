/*
 * sim808.c
 *
 *  Created on: Nov 6, 2025
 *      Author: Mohamed
 */


#include "sim808.h"
#include <string.h>
#include <stdio.h>



extern UART_HandleTypeDef huart3;
extern UART_HandleTypeDef huart2;



uint8_t rx_data;
char sim_buffer[RX_BUFFER_SIZE];
uint16_t idx = 0;
uint8_t data_received = 0;
bool sim_connection_lost = false;




void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART3)
    {
    	 if (idx < RX_BUFFER_SIZE - 1)
    	        {
    	            sim_buffer[idx++] = rx_data;
    	            sim_buffer[idx] = '\0';   // Correct
    	        }
		HAL_UART_Receive_IT(&huart3, &rx_data, 1);
    }
}

void Log(const char *msg)
{
    HAL_UART_Transmit(&huart2, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
}
bool waitForResponse( char *expected, uint32_t timeout_ms)
{
    uint32_t start = HAL_GetTick();
    while (1)
    {
		// Check if expected text is inside received buffer
    	if (bufferContains(sim_buffer,expected))
			return true;

		// Timeout
		if (HAL_GetTick() - start > timeout_ms)
			return false;
    }

}

bool bufferContains( char *haystack,const char *needle)
{

	int hay_len = idx;                 // number of real bytes received
	int needle_len = strlen(needle);

	if (needle_len == 0 || hay_len < needle_len)
		return false;

	// Raw byte search (no NULL needed)
	for (int i = 0; i <= hay_len - needle_len; i++) {
		if (memcmp(&haystack[i], needle, needle_len) == 0) {
			return true;}
	}

	return false;

}
/* --- Send Command to SIM808 --- */
void SendAT( char *cmd)
{
	memset(sim_buffer, 0, RX_BUFFER_SIZE);
	idx = 0;
	data_received = 0;
	//Log("MCU >> ");
	//Log(cmd);
	//Log("\r\n");
	HAL_UART_Receive_IT(&huart3, &rx_data, 1);
    HAL_UART_Transmit(&huart3, (uint8_t *)cmd, strlen(cmd), HAL_MAX_DELAY);
}

bool SendCommand( char *cmd ,  char *expected){
	SendAT(cmd);
	if (waitForResponse(expected, TIMEOUT)) {
		//Log("SIM808 >> ");
		//HAL_UART_Transmit(&huart2, sim_buffer, RX_BUFFER_SIZE, HAL_MAX_DELAY);
		//Log("\r\n");
		return true;
	} else{
		Log("SIM808: Timeout\r\n");
		sim_connection_lost = true;
		return false;
	}
}

void powerOnGPS(){

	SendCommand("AT+CGNSPWR=1\r\n","OK");

}

void powerOffGPS(){

	SendCommand("AT+CGNSPWR=0\r\n","OK");

}

GPSData parseGPS (char *buffer){
	 	GPSData gps;
		char *token = strtok(buffer, ",");
		int i = 0;

		gps.latitude[0] = '\0';
		gps.longitude[0] = '\0';
		while (token != NULL) {
			if (i == 3) {
				strncpy(gps.latitude, token, sizeof(gps.latitude) - 1);
				gps.latitude[sizeof(gps.latitude) - 1] = '\0';
			}
			if (i == 4) {
				strncpy(gps.longitude, token, sizeof(gps.longitude) - 1);
				gps.longitude[sizeof(gps.longitude) - 1] = '\0';
			}
			token = strtok(NULL, ",");
			i++;
		}

		return gps;
}

void getGPSLink(const GPSData *gps, char *linkBuffer, size_t bufferSize)
{
    // Format: Google Maps link
    // Example: https://www.google.com/maps?q=50.776432,6.083887
    snprintf(linkBuffer, bufferSize,
             "https://www.google.com/maps?q=%s,%s",
             gps->latitude,
             gps->longitude);
}
/*
void extractSMSText(const char *rxBuffer, char *smsText, size_t smsTextSize)
{
    // Start by finding the first "+CMGL:" line
    const char *header = strstr(rxBuffer, "+CMGL:");
    if (!header)
    {
        smsText[0] = '\0'; // no message found
        return;
    }

    // Move pointer to the end of the header line
    const char *newlineAfterHeader = strchr(header, '\n');
    if (!newlineAfterHeader)
    {
        smsText[0] = '\0';
        return;
    }

    // Start of message text is after this newline
    const char *msgStart = newlineAfterHeader + 1;

    // Find the next line (usually "OK" or next message)
    const char *msgEnd = strchr(msgStart, '\n');
    if (!msgEnd)
        msgEnd = msgStart + strlen(msgStart);

    // Remove possible carriage return at the end
    if (*(msgEnd-1) == '\r')
        msgEnd--;

    // Copy text safely to smsText
    size_t len = msgEnd - msgStart;
    if (len >= smsTextSize)
        len = smsTextSize - 1;

    strncpy(smsText, msgStart, len);
    smsText[len] = '\0';
}

bool checkSMS(const char *text){

	SendCommand("AT+CMGF=1\r\n");

	SendCommand("AT+CMGL=\"REC UNREAD\"\r\n");

	char smsText[160] = {0};
	extractSMSText(sim_buffer, smsText, sizeof(smsText));
	//Log(smsText);
	if (strstr(smsText, text) != NULL) {
		//Log("SMS text was successfully received : ");
		//Log(smsText);
		return true;
	}
	else {
		//Log("SMS text was not received : ");
		//Log(smsText);
		return false;
	}
}
void sendSMS(const char *sms){
	SendCommand("AT+CMGF=1\r\n");

	SendAT("AT+CMGS=\"+4917647327703\"\r\n");
	if (waitForPrompt('>', TIMEOUT)) {
		// Send SMS text
		SendAT(sms);

		// End with Ctrl+Z
		uint8_t ctrlZ = 0x1A;
		HAL_UART_Transmit(&huart3, &ctrlZ, 1, HAL_MAX_DELAY);

		// Wait for message confirmation (+CMGS: ... OK)
		if (waitForResponse(TIMEOUT)) {
			Log("Done\r\n");
			Log("SIM808 >> ");
			Log(sim_buffer);
			Log("\r\n");
		}

	} else
		Log("Error: no prompt received from SIM808.\r\n");
}

void sendGPS(){
	powerOnGPS();
	SendCommand("AT+CGNSINF\r\n");
	GPSData gps = parseGPS(sim_buffer);
	char buffer[64];

	snprintf(buffer, sizeof(buffer), "Latitude: %s\r\n", gps.latitude);
	Log(buffer);

	snprintf(buffer, sizeof(buffer), "Longitude: %s\r\n", gps.longitude);
	Log(buffer);

	char gpsLink[80]; // buffer to hold the link
	getGPSLink(&gps, gpsLink, sizeof(gpsLink));
	Log(gpsLink);

	sendSMS(gpsLink);


}*/

void sendGPS(char *out ,int out_size) {
    powerOnGPS();
    SendCommand("AT+CGNSSEQ=\"RMC\"\r\n","OK");

    SendCommand("AT+CGNSINF\r\n","OK");

    GPSData gps = parseGPS(sim_buffer);

    // format: "<lat>,<lon>"
    snprintf(out, out_size, "%s,%s", gps.latitude, gps.longitude);
    //Log("Gps msggg\r\n");
    //Log(out);   // single combined output
    //Log("-----------------\r\n");
}
int buildMQTTPacket(uint8_t *out, int outSize,
                       const uint8_t *pkt1, int pkt1_len,
                       const uint8_t *pkt2, int pkt2_len)
{
    int pos = 0;

    // Copy CONNECT packet
    for (int i = 0; i < pkt1_len; i++) {
        if (pos >= outSize) return -1;
        out[pos++] = pkt1[i];
    }

    // Copy PUBLISH packet
    for (int i = 0; i < pkt2_len; i++) {
        if (pos >= outSize) return -1;
        out[pos++] = pkt2[i];
    }

    // Append 0x1A → SIM808 end-of-data
    if (pos >= outSize) return -1;
    out[pos++] = 0x1A;

    return pos;   // total raw bytes length
}

int MQTT_Append1A(unsigned char *buf, int bufsize, int packet_len)
{
    // Make sure we have room for one more byte
    if (packet_len + 1 > bufsize) {
        return -1;   // Not enough space
    }

    buf[packet_len] = 0x1A;  // append byte
    return packet_len + 1;   // new length
}
bool initGPRS(){
	if (!SendCommand("AT+CIPSHUT\r\n", "OK")) return false;
	    if (!SendCommand("AT+CGATT=1\r\n", "OK")) return false;
	    if (!SendCommand("AT+CIPMUX=0\r\n", "OK")) return false;
	    if (!SendCommand("AT+CIPRXGET=1\r\n", "OK")) return false;
	    if (!SendCommand("AT+CSTT=\"iotsim.melita.io\"\r\n", "OK")) return false;
	    if (!SendCommand("AT+CIICR\r\n", "OK")) return false;
	    if (!SendCommand("AT+CIFSR\r\n", "10.0.12.209")) return false;

	    return true;
}

bool startTCP(){

	return SendCommand("AT+CIPSTART=\"TCP\",\"broker.emqx.io\",\"1883\"\r\n","CONNECT OK");
}

bool initReceive(){
	if(!initGPRS()){
		return false;
	}
	if(startTCP()){
		Log("Connected to the BROKER\r\n");
	}else{
		return false;
	}
	SendCommand("AT+CIPSEND\r\n", ">");
	uint8_t pkt1[300];
	uint8_t pkt2[300];
	uint8_t buffer[1024];

	int connect_len = MQTT_BuildConnectPacket(pkt1, sizeof(pkt1));
	int sub_len = MQTT_BuildSubscribePacket(pkt2, sizeof(pkt2), "test/node-red");

	int total_len = buildMQTTPacket(buffer, sizeof(buffer), pkt1, connect_len,
			pkt2, sub_len);
	char hexbuf[1024] = {0};
	int pos = 0;
	for (int i = 0; i < total_len; i++) pos += sprintf(&hexbuf[pos], "%02X ", pkt1[i]);
	//Log("Conn + Sub\r\n");
	//Log(hexbuf);
	//Log("\r\n");
	memset(sim_buffer, 0, RX_BUFFER_SIZE);
	idx = 0;
	HAL_UART_Transmit(&huart3, buffer, total_len,
			HAL_MAX_DELAY);


	if (waitForResponse("OK", TIMEOUT)) {
		//Log("SIM808 >> ");
		//HAL_UART_Transmit(&huart2, sim_buffer, RX_BUFFER_SIZE, HAL_MAX_DELAY);
		//Log("\r\n");
		Log("SIM808 Subscribed\r\n");
		return true;
	} else{
		Log("SIM808: Timeout\r\n");
		sim_connection_lost = true;
		return false;
	}
}

bool checkCommand( char *text){
	if (bufferContains(sim_buffer,"+CIPRXGET: 1")) {
		SendCommand("AT+CIPRXGET=2,1024\r\n", "OK");
		if (bufferContains( sim_buffer, text)){
			Log("Where Received \r\n");
			return true;
		}else
			return false;
	}
	else {
		return false;
	}

}
char* CheckReceivedBuffer(void) {
    if (bufferContains(sim_buffer, "+CIPRXGET: 1")) {
        SendCommand("AT+CIPRXGET=2,1024\r\n", "OK");  // fetch incoming data
        return sim_buffer;  // return pointer to buffer for later processing
    }
    return NULL;  // nothing new
}
void sendPos(){
	//-----gps data -----------
	char gps_str[100];

	sendGPS(gps_str,sizeof(gps_str));

	Log(gps_str);
	Log("\r\n");
	//---------------------------

	//P------MQTT Packets----------
	uint8_t pkt1[300];
	int publish_len = MQTT_BuildPublishPacket(pkt1, sizeof(pkt1),
			"gps/location", gps_str);

	int total_len = MQTT_Append1A(pkt1, sizeof(pkt1), publish_len);
	//--------------------------------------

	SendCommand("AT+CIPSEND\r\n", ">");
	//Log("MCU >> sent packet");
	//Log("\r\n");
	memset(sim_buffer, 0, RX_BUFFER_SIZE);
	idx = 0;
	HAL_UART_Transmit(&huart3, pkt1, total_len, HAL_MAX_DELAY);

	if (waitForResponse("SEND OK", TIMEOUT)) {
		//Log("SIM808 >> ");
		//HAL_UART_Transmit(&huart2, sim_buffer, RX_BUFFER_SIZE, HAL_MAX_DELAY);
		//Log("\r\n");
	} else{
		sim_connection_lost = true;
		Log("SIM808: Timeout\r\n");
	}
}
bool checkConnection(){
	if(SendCommand("AT+CIPSTATUS\r\n", "CONNECT OK")){
		return true;
	}else {
		return false;
	}
}

