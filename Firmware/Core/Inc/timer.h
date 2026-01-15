/*
 * timer.h
 *
 *  Created on: Dec 22, 2025
 *      Author: Mohamed
 */

#ifndef INC_TIMER_H_
#define INC_TIMER_H_

#include "timer.h"
#include "main.h"

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Start the tracking timer with a given period (seconds)
 * @param period_sec: period in seconds
 */
void TrackTimer_Start(uint32_t period_sec);

/**
 * @brief Stop the tracking timer
 */
void TrackTimer_Stop(void);

/**
 * @brief Check if the timer period has elapsed (1-second tick)
 * @return true if timer event pending, false otherwise
 */
bool TrackTimer_EventPending(void);


/**
 * @brief Start the tracking timer with a given period (seconds)
 * @param period_sec: period in seconds
 */
void WatchdogTimer_Start();


/**
 * @brief Stop the tracking timer
 */
void WatchdogTimer_Stop(void);


/**
 * @brief Check if the timer period has elapsed (1-second tick)
 * @return true if timer event pending, false otherwise
 */
bool WatchdogTimer_EventPending(void);


#endif /* INC_TIMER_H_ */
