/*
 * timer.c
 *
 *  Created on: Dec 22, 2025
 *      Author: Mohamed
 */


#include "timer.h"
#include "stm32l4xx_hal.h"

extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3; // CubeMX generated timer


static volatile bool track_timer_flag = false;
static volatile bool watchdog_timer_flag = false;

/**
 * @brief Start the timer
 * @param period_sec Tracking period in seconds
 */
void TrackTimer_Start(uint32_t period_sec)
{
    uint32_t base_freq = 10000; // 10 kHz timer base (PSC = 8399)
    uint32_t arr = period_sec * base_freq - 1;

    __HAL_TIM_DISABLE(&htim2);
    __HAL_TIM_SET_AUTORELOAD(&htim2, arr);
    __HAL_TIM_SET_COUNTER(&htim2, 0);
    HAL_TIM_Base_Start_IT(&htim2);

    track_timer_flag = false;
}

/**
 * @brief Stop the timer
 */
void TrackTimer_Stop(void)
{
    HAL_TIM_Base_Stop_IT(&htim2);
    track_timer_flag = false;
}

/**
 * @brief Check and consume pending timer event
 * @return true if event pending, false otherwise
 */
bool TrackTimer_EventPending(void)
{
    if (track_timer_flag) {
        track_timer_flag = false;
        return true;
    }
    return false;
}

/**
 * @brief Start the timer
 * @param period_sec Tracking period in seconds
 */
void WatchdogTimer_Start()
{
    /*uint32_t base_freq = 10000; // 10 kHz timer base (PSC = 8399)
    uint32_t arr = period_sec * base_freq - 1;

    __HAL_TIM_DISABLE(&htim3);
    __HAL_TIM_SET_AUTORELOAD(&htim3, arr);
    __HAL_TIM_SET_COUNTER(&htim3, 0);
    */
    HAL_TIM_Base_Start_IT(&htim3);

    watchdog_timer_flag = false;
}

/**
 * @brief Stop the timer
 */
void WatchdogTimer_Stop(void)
{
    HAL_TIM_Base_Stop_IT(&htim3);
    watchdog_timer_flag = false;
}

/**
 * @brief Check and consume pending timer event
 * @return true if event pending, false otherwise
 */
bool WatchdogTimer_EventPending(void)
{
    if (watchdog_timer_flag) {
    	watchdog_timer_flag = false;
        return true;
    }
    return false;
}

/**
 * @brief To be called from HAL_TIM_PeriodElapsedCallback
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2) {
    	track_timer_flag = true; // signal event to FSM
    }
    if (htim->Instance == TIM3) {
    	watchdog_timer_flag = true; // signal event to FSM
        }
}
