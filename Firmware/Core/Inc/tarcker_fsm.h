/*
 * tarcker_fsm.h
 *
 *  Created on: Dec 22, 2025
 *      Author: Mohamed
 */

#ifndef INC_TARCKER_FSM_H_
#define INC_TARCKER_FSM_H_

#include "main.h"
#include "sim808.h"
#include "timer.h"
#include <stdint.h>

typedef enum {
    STATE_IDLE,
    STATE_TRACKING,
    STATE_PARK,
	STATE_RECONNECT,
} State_t;

typedef enum {
    EVENT_NONE,

    /* MQTT commands */
    EVENT_CMD_WHERE,
    EVENT_CMD_TRACK_ON,
    EVENT_CMD_PARK_ON,
    EVENT_CMD_STOP,
	EVENT_CONNECTION_LOST,
	EVENT_CONNECTION_CLOSE,

    /* Buttons */
    EVENT_BTN_TRACK,
    EVENT_BTN_PARK,

    /* Timers */
    EVENT_TRACK_TIMER,

    /* Sensors */
    EVENT_GEOFENCE_EXIT
} Event_t;

extern State_t current_state;
extern State_t previous_state;
extern Event_t prev_ev;
/*
static double last_lat = 0.0;
static double last_lon = 0.0;

static uint32_t tracking_period_sec = 5;
static double park_radius_m = 10.0;
*/

//Handlers
 void State_Idle_Enter(void);
 void State_Idle_Exit(void);
 void State_Tracking_Enter(void);
 void State_Tracking_Exit(void);
 void State_Park_Enter(void);
 void State_Park_Exit(void);
 void State_Reconnect_Enter();
 void State_Reconnect_Exit();
 void State_Idle_HandleEvent(Event_t ev);
 void State_Tracking_HandleEvent(Event_t ev);
 void State_Park_HandleEvent(Event_t ev);
 void State_Reconnect_HandleEvent(Event_t ev);
 void Transition_To(State_t new_state);

/* ===== Events ===== */
Event_t Event_Wait(void);

#endif /* INC_TARCKER_FSM_H_ */
