/*
 * sim808.h
 *
 *  Created on: Nov 6, 2025
 *      Author: Mohamed
 */

#ifndef INC_SIM808_H_
#define INC_SIM808_H_



#include "main.h"
#include "mqtt.h"
#include <stdint.h>
#include <stdbool.h>



#define RX_BUFFER_SIZE 512
#define TIMEOUT 10000


typedef struct {
    char latitude[16];
    char longitude[16];
} GPSData;


extern unsigned char pkt[300];
extern bool sim_connection_lost;  // true if any command fails



/**
 * @brief UART receive complete callback
 *
 * This callback is called by the HAL UART driver when one character
 * has been received from the SIM module into the receive buffer.
 * It is typically used to store incoming data and restart reception.
 *
 * @param huart Pointer to the UART handle instance
 * @retval None
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);

/**
 * @brief Print a log message to the terminal
 *
 * Works similarly to printf, sending a null-terminated string
 * to the debug terminal or UART2 output.
 *
 * @param msg Null-terminated message string
 * @retval None
 */
void Log(const char *msg);

/**
 * @brief Send a raw AT command
 *
 * Sends an AT command to the SIM module without waiting
 * for any response.
 *
 * @param cmd Null-terminated AT command string
 * @retval None
 */
void SendAT( char *cmd);

/**
 * @brief Wait for a specific response from the SIM module
 *
 * Blocks until the expected response string is found in the
 * receive buffer or the timeout expires.
 *
 * @param expected Expected response string
 * @param timeout_ms Timeout in milliseconds
 * @retval true  Response received
 * @retval false Timeout occurred
 */
bool waitForResponse( char *expected, uint32_t timeout_ms);

/**
 * @brief Send an AT command and wait for a response
 *
 * Sends an AT command to the SIM module and waits until
 * the expected response is received.
 *
 * @param cmd AT command to send
 * @param expected Expected response string
 * @retval true  Command succeeded
 * @retval false Command failed or timed out
 */
bool SendCommand( char *cmd ,  char *expected);

/**
 * @brief Check if a buffer contains a substring
 *
 * Searches for the occurrence of a substring inside a buffer.
 *
 * @param haystack Buffer to search in
 * @param needle Substring to search for
 * @retval true  Substring found
 * @retval false Substring not found
 */
bool bufferContains( char *haystack,const char *needle);

/*
* @brief Wait for a specific prompt character
*
* Blocks until a specific character is received from the SIM module
* or the timeout expires.
*
* @param prompt Expected prompt character
* @param timeout_ms Timeout in milliseconds
* @retval true  Prompt received
* @retval false Timeout occurred
*/
bool waitForPrompt(char prompt, uint32_t timeout_ms);

/**
 * @brief Parse GPS data from a buffer
 *
 * Extracts latitude, longitude and other GPS information
 * from the raw response buffer.
 *
 * @param buffer Buffer containing GPS data
 * @retval Parsed GPS data structure
 */
GPSData parseGPS (char *buffer);

/**
 * @brief Format a GPS link from GPS data
 *
 * Generates a formatted GPS link (e.g. Google Maps)
 * using the provided GPS coordinates.
 *
 * @param gps Pointer to GPS data structure
 * @param linkBuffer Output buffer for the formatted link
 * @param bufferSize Size of the output buffer
 * @retval None
 */
void getGPSLink(const GPSData *gps, char *linkBuffer, size_t bufferSize);

/**
 * @brief Extract SMS text from a receive buffer
 *
 * Parses the received buffer and extracts the SMS message
 * content into the provided output buffer.
 *
 * @param rxBuffer Buffer containing the raw SMS data
 * @param smsText Output buffer for the extracted SMS text
 * @param smsTextSize Size of the output buffer
 * @retval None
 */

void extractSMSText(const char *rxBuffer, char *smsText, size_t smsTextSize);


void getGPS();

/**
 * @brief Power on the GPS module
 *
 * Enables GPS functionality on the SIM module.
 *
 * @retval None
 */
void powerOnGPS();

/**
 * @brief Power off the GPS module
 *
 * Disables GPS functionality to save power.
 *
 * @retval None
 */
void powerOffGPS();

/**
 * @brief Check for incoming SMS commands
 *
 * Checks the received text for a valid SMS command.
 *
 * @param text Received SMS text
 * @retval true  Valid SMS detected
 * @retval false No valid SMS found
 */
bool checkSMS(const char *text);

/**
 * @brief Send GPS position via SMS
 *
 * Retrieves the current GPS position and sends it
 * as an SMS message.
 *
 * @param out Output buffer
 * @param out_size Size of the output buffer
 * @retval None
 */
void sendGPS(char *out ,int out_size);

/**
 * @brief Send an SMS message
 *
 * Sends a text message using the SIM module.
 *
 * @param sms Null-terminated SMS message string
 * @retval None
 */
void sendSMS(const char *sms);

/**
 * @brief Initialize GPRS connection
 *
 * Configures and activates the GPRS connection
 * on the SIM module.
 *
 * @retval true  GPRS initialized successfully
 * @retval false Initialization failed
 */
bool initGPRS();

/**
 * @brief Start a TCP connection
 *
 * Opens a TCP connection to the MQTT broker.
 *
 * @retval true  Connection successful
 * @retval false Connection failed
 */
bool startTCP();

//just a test
void sendPosTest(const char *message);

/**
 * @brief Build MQTT payload in hexadecimal format
 *
 * Combines two MQTT packets into a single hexadecimal
 * payload suitable for transmission.
 *
 * @param out Output buffer
 * @param outSize Size of output buffer
 * @param pkt1 First packet buffer
 * @param pkt1_len Length of first packet
 * @param pkt2 Second packet buffer
 * @param pkt2_len Length of second packet
 * @retval Length of generated payload
 */
int buildHexMQTTPayload(char *out, int outSize,
                        uint8_t *pkt1, int pkt1_len,
                        uint8_t *pkt2, int pkt2_len);


/**
 * @brief Initialize receive mode and subscribe to broker
 *
 * Establishes the MQTT receive connection and subscribes
 * to the configured topic.
 *
 * @retval true  Initialization successful
 * @retval false Initialization failed
 */
bool initReceive();

/**
 * @brief Check for incoming MQTT commands
 *
 * Parses received MQTT messages and checks for
 * valid control commands.
 *
 * @param text Received message text
 * @retval true  Valid command received
 * @retval false No valid command
 */
bool checkCommand( char *text);

/**
 * @brief Get pointer to received data buffer
 *
 * Returns a pointer to the internal receive buffer
 * containing the latest data.
 *
 * @retval Pointer to receive buffer
 */
char* CheckReceivedBuffer(void);

/**
 * @brief Send GPS position via MQTT
 *
 * Publishes the current GPS position to the MQTT broker.
 *
 * @retval None
 */
void sendPos( );

/**
 * @brief Check network and MQTT connection status
 *
 * Verifies that the device is still connected to
 * the network and MQTT broker.
 *
 * @retval true  Connection active
 * @retval false Connection lost
 */
bool checkConnection();


#endif /* INC_SIM808_H_ */
