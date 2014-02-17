/**
 * MP3Player
 */

#include "main.h"
// cmsis libs
#include "stm32f4xx.h"

// discovery specific
#include "stm32f4_discovery.h"

// FreeRTOS includes
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"

// Tasks to run
#include "AudioTask.h"
#include "USBTask.h"

// number of FreeRTOS timers to create
#define NUM_TIMER 4

// each LED's state
enum LED_STATE led_state[NUM_LEDS];

// handles to FreeRTOS timers
xTimerHandle xTimers[NUM_TIMER];

// periods of the FreeRTOS timers
portTickType nTimerPeriods[] = { 100, 250, 500, 10 };

// declared in FatFS
void disk_timerproc ( void );

// do board specific init
void DiscoveryInit ( )
{
	uint8_t i = 0;
	for ( i = 0; i < NUM_LEDS; i++ )
	{
		STM_EVAL_LEDInit ( i );
	};
};

// called if one of the FreeRTOS timer has elapsed
// controls the LEDs and the FatFS timing
void vTimerCallback ( xTimerHandle pxTimer )
{
	static uint8_t PulseCounter = 0;
	uint8_t i;

	configASSERT( pxTimer );

	// Which timer has expired?
	long lTimerID = ( long ) pvTimerGetTimerID ( pxTimer );

	for ( i = 0; i < NUM_LEDS; i++ ) if ( led_state[i] == LED_STATE_OFF ) STM_EVAL_LEDOff ( i );
	for ( i = 0; i < NUM_LEDS; i++ ) if ( led_state[i] == LED_STATE_ON ) STM_EVAL_LEDOn ( i );

	switch ( lTimerID )
	{
	case 0: // fast blink timer
		for ( i = 0; i < NUM_LEDS; i++ )
		{
			if ( ( led_state[i] == LED_STATE_FLASH ) && ( ( PulseCounter % 10 ) == 0 ) ) STM_EVAL_LEDOn ( i );
			if ( ( led_state[i] == LED_STATE_FLASH ) && ( ( PulseCounter % 10 ) == 1 ) ) STM_EVAL_LEDOff ( i );
			PulseCounter++;
		};
		break;
	case 1: // medium blink timer
		for ( i = 0; i < NUM_LEDS; i++ ) if ( led_state[i] == LED_STATE_BLINK_FAST ) STM_EVAL_LEDToggle ( i );
		break;
	case 2: // slow blink timer
		for ( i = 0; i < NUM_LEDS; i++ ) if ( led_state[i] == LED_STATE_BLINK_SLOW ) STM_EVAL_LEDToggle ( i );
		break;
	case 3: // FatFS timer
		disk_timerproc ( );
		break;
	}
};

// create and start the FreeRTOS timers
void SetupTimer ( )
{
	// Create then start some timers.  Starting the timers before the scheduler
	// has been started means the timers will start running immediately that
	// the scheduler starts.
	uint8_t x;
	for ( x = 0; x < NUM_TIMER; x++ )
	{
		xTimers[x] = xTimerCreate (
				( const signed char * ) "Timer",    // Just a text name, not used by the kernel.
				nTimerPeriods[x],     				// The timer period in ticks.
				pdTRUE,         					// The timers will auto-reload themselves when they expire.
				( void * ) x,     					// Assign each timer a unique id equal to its array index.
				vTimerCallback     					// Each timer calls the same callback when it expires.
		);

		if ( xTimers[x] == NULL )
		{
			// The timer was not created.
		}
		else
		{
			// Start the timer.  No block time is specified, and even if one was
			// it would be ignored because the scheduler has not yet been
			// started.
			if ( xTimerStart( xTimers[x], 0 ) != pdPASS )
			{
				// The timer could not be set into the Active state.
			}
		}
	}
};

int main ( void )
{
	SystemInit ( );

	DiscoveryInit ( );

	SetupTimer ( );

	led_state[0] = LED_STATE_BLINK_SLOW;

	xTaskCreate (
			USBTask,
			( signed char * ) "USB",
			configMINIMAL_STACK_SIZE,
			NULL,
			1,
			NULL );

	xTaskCreate (
			AudioTask,
			( signed char * ) "Audio",
			configMINIMAL_STACK_SIZE,
			NULL,
			1,
			NULL );

	vTaskStartScheduler ( );

	while ( 1 )
	{
	};
};
