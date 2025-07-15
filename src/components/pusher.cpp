#include "pusher.h"
#include "./../util.h"

// instantiate a pusher object
Rune::PusherGeneric::PusherGeneric(wheelUpdateCallback_t callback, firemode_t *firemode_curr) {
  updatewheelState = callback;
  firemode = firemode_curr;
  shotsFired = 0;
}

// returns true if the pusher module was successfully initialized
bool Rune::PusherGeneric::init() {
  return false;
}

// handler code for when the trigger is pressed
void Rune::PusherGeneric::triggerRisingEdge() {  
}

// handler code for when the trigger is released
void Rune::PusherGeneric::triggerFallingEdge() {
}

void Rune::PusherGeneric::pusherTick() {
}

void Rune::PusherGeneric::updatePusherState() {
}