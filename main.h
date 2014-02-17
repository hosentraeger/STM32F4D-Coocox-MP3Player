#ifndef __MAIN_H__
#define __MAIN_H__

// cmsis libs
#include "stm32f4xx.h"

// discovery specific
#include "stm32f4_discovery.h"

// number of LEDs to drive
#define NUM_LEDS 4

enum LED_STATE
{
	LED_STATE_OFF,
	LED_STATE_BLINK_SLOW,
	LED_STATE_BLINK_FAST,
	LED_STATE_FLASH,
	LED_STATE_ON
};

// each LED's state
extern enum LED_STATE led_state[NUM_LEDS];


#endif // __MAIN_H__
