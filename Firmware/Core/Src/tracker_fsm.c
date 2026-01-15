/*
 * tracker_fsm.c
 *
 *  Created on: Dec 22, 2025
 *      Author: Mohamed
 */

#include "tarcker_fsm.h"
#include <string.h>
#include <stdio.h>
State_t current_state = STATE_IDLE;
State_t previous_state = STATE_IDLE;
Event_t prev_ev = EVENT_NONE;
/* =============== IDLE state Entry / Exit =============== */
 void State_Idle_Enter(void)
{
    TrackTimer_Stop();
    //GPS_Sleep();
    //Power_EnterSleep();
	Log("Enter Idle State\r\n");
}

 void State_Idle_Exit(void)
{
    //Power_ExitSleep();
    //GPS_Wakeup();
	Log("EXIT Idle State\r\n");
}
/* =============== TRACK state Entry / Exit =============== */
 void State_Tracking_Enter(void)
{
    //GPS_Wakeup();
    //MQTT_Connect();
    //MQTT_SubscribeCommands();
    TrackTimer_Start(1);
	 Log("Enter Tracking State\r\n");
}

 void State_Tracking_Exit(void)
{
    TrackTimer_Stop();
	Log("EXIT Tracking State\r\n");

}
/* =============== Park state Entry / Exit =============== */
 void State_Park_Enter(void)
{
    //GPS_Wakeup();

    /*if (GPS_GetPosition(&last_lat, &last_lon)) {
        Geofence_SetCenter(last_lat, last_lon, park_radius_m);
    }*/
	 Log("Enter Park State\r\n");
	 sendPos();
}

 void State_Park_Exit(void)
{
    /* Nothing special for now */
	 Log("EXIT Park State\r\n");
}
 void State_Reconnect_Enter(){
	 Log("Enter RECONNECT State\r\n");
	 WatchdogTimer_Stop();
 }
 void State_Reconnect_Exit(){
	 Log("EXIT RECONNECT State\r\n");
	 WatchdogTimer_Start();

 }
/* ================ Transition function =================== */
 void Transition_To(State_t new_state)
{
    if (current_state == new_state) {
        return;   // prevent re-entering same state
    }
	// If we are moving into RECONNECT, save previous state
	if (new_state == STATE_RECONNECT) {
		previous_state = current_state;
	}
    /* ---- Exit old state ---- */
    switch (current_state) {

    case STATE_IDLE:
        State_Idle_Exit();
        break;

    case STATE_TRACKING:
        State_Tracking_Exit();
        break;

    case STATE_PARK:
        State_Park_Exit();
        break;
    case STATE_RECONNECT:
    	State_Reconnect_Exit();
    	break;

    default:
        break;
    }

    current_state = new_state;

    /* ---- Enter new state ---- */
    switch (new_state) {

    case STATE_IDLE:
        State_Idle_Enter();
        break;

    case STATE_TRACKING:
        State_Tracking_Enter();
        break;

    case STATE_PARK:
        State_Park_Enter();
        break;
    case STATE_RECONNECT:
    	State_Reconnect_Enter();
    	break;
    default:
        break;
    }
}
/* ================ IDLE Handler =================== */
 void State_Idle_HandleEvent(Event_t ev)
{
    switch (ev) {

    case EVENT_CMD_WHERE:
        Log("Received WHERE command\r\n");
        sendPos();
        Log("Sent pos, back to idle\r\n");
        break;
    case EVENT_CONNECTION_LOST:
		Log("The Connections is Lost :( d\r\n");
		SendCommand("AT+CIPCLOSE\r\n", "OK");
		Log("Closed connection, going to RECONNECT\r\n");
        Transition_To(STATE_RECONNECT);
         break;

    case EVENT_CMD_TRACK_ON:
    	Log("Received Track on command\r\n");
        Transition_To(STATE_TRACKING);
        break;

    case EVENT_CMD_PARK_ON:
		Log("Received Park on command\r\n");
		Transition_To(STATE_PARK);
		break;


    default:
        break;
    }
}
/* ================ Track Handler =================== */
 void State_Tracking_HandleEvent(Event_t ev)
{
    switch (ev) {

    case EVENT_TRACK_TIMER:
		Log("Tracking ...\r\n");
		sendPos();
        break;

    case EVENT_CMD_WHERE:
        //SendPosition();
        break;

    case EVENT_CMD_STOP:
        Transition_To(STATE_IDLE);
		break;
	case EVENT_CONNECTION_LOST:
		Log("The Connections is Lost :( d\r\n");
		SendCommand("AT+CIPCLOSE\r\n", "OK");
		Log("Closed connection, going to RECONNECT\r\n");
		Transition_To(STATE_RECONNECT);
		break;

	case EVENT_CMD_PARK_ON:
		Log("Received Park on command\r\n");
		Transition_To(STATE_PARK);
		break;


    default:
        break;
    }
}
/* ================ PARK Handler =================== */
 void State_Park_HandleEvent(Event_t ev)
{
    switch (ev) {

    case EVENT_CMD_WHERE:
        //SendPosition();
        break;

    case EVENT_GEOFENCE_EXIT:
        //MQTT_PublishAlert("GEOFENCE BREACH");
        Transition_To(STATE_TRACKING);
        break;

	case EVENT_CONNECTION_LOST:
		Log("The Connections is Lost :( d\r\n");
		//SendCommand("AT+CIPCLOSE\r\n", "OK");
		//Log("Closed connection, going to RECONNECT\r\n");
		Transition_To(STATE_RECONNECT);
		break;

	 case EVENT_CMD_TRACK_ON:
		Log("Received Track on command\r\n");
		Transition_To(STATE_TRACKING);
		break;

    case EVENT_CMD_STOP:
        Transition_To(STATE_IDLE);
        break;

    default:
        break;
    }
}
 /* ================ RECONNECT Handler =================== */
 void State_Reconnect_HandleEvent(Event_t ev)
 {
	switch (ev) {
	//case EVENT_TRACK_TIMER:
	case EVENT_CONNECTION_LOST:
		Log("Retrying to connect...\r\n");
		if (initReceive()) {
			Log("Reconnection was successful !\r\n");
			sim_connection_lost = false;
			Transition_To(previous_state);
			//MQTT_PublishAlert("Connection Restored");
		}
		break;

	default:
		break;  // ignore other events
	}
}
/* ================ Methods =================== */

Event_t Event_Wait(void){

	/* Check connection first */
	if (sim_connection_lost) {

		return EVENT_CONNECTION_LOST;
	}

	if (TrackTimer_EventPending()) {
	        return EVENT_TRACK_TIMER;
	    }
	if (WatchdogTimer_EventPending()) {
		if(!checkConnection()){
			return EVENT_CONNECTION_LOST;
		}
	}
	/* --- SIM commands --- */
	char* msg = CheckReceivedBuffer();

	if (msg != NULL) {
	    if (bufferContains(msg, "Where")) {
	        return EVENT_CMD_WHERE;
	    }
	    if (bufferContains(msg, "close")) {
	    	sim_connection_lost = true;
	        return EVENT_NONE;
	    }
		if (bufferContains(msg, "track")) {
			return EVENT_CMD_TRACK_ON;
		}
		if (bufferContains(msg, "stop")) {
			return EVENT_CMD_STOP;
		}
		if (bufferContains(msg, "park")) {
			return EVENT_CMD_PARK_ON;
		}
	}

	/* --- Buttons (optional, polling example) --- */
	/*
	 if (ButtonTrack_Pressed()) {
	 return EVENT_BTN_TRACK;
	 }

	 if (ButtonPark_Pressed()) {
	 return EVENT_BTN_PARK;
	 }
	 */

	return EVENT_NONE;
}
















