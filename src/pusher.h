#pragma once

#include "config.h"

// pusher logic for when the trigger is pulled
void pusherTriggerRisingEdgeHook();

// pusher logic for when the trigger is released
void pusherTriggerFallingEdgeHook();

// called every tick of the system control loop
void pusherTick();

#ifdef PUSHER_SCOTCH_YOKE
// check to make sure the pusher isnt stuck on for too long after the trigger is released (dead switch/cannot travel/etc)
bool pusherSafetyCallback(repeating_timer_t *rt);
#endif