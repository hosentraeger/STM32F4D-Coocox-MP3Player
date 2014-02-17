/**
 * this task listens for commands
 * and plays audio files
 */
#include "stm32f4_discovery.h"
#include "stm32f4_discovery_audio_codec.h"
#include "AudioTask.h"

/* FreeRTOS includes */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"

/* FatFS */
#include "ff.h"

/* printf()... */
#include <stdio.h>
/* strrchr, strncmp */
#include <string.h>

#include "main.h"

xQueueHandle xAudioQueue = NULL;
FATFS FatFs;

struct t_AudioContext
{
	FIL fil;
	uint8_t fileIsOpen;
	uint8_t lastFrame;
	enum FILEFORMAT format;
	uint16_t dmaBuffer[1152];
	uint8_t sampleBuffer[2*1152];
	uint16_t sampleBufferSize;
	uint8_t encodedDataBuffer[1940];
	uint16_t encodedDataBufferSize;
} audioContext;


void EVAL_AUDIO_TransferComplete_CallBack(uint32_t pBuffer, uint32_t Size)
{
	struct AAudioCommandMessage CmdMsg;
	long lHigherPriorityTaskWoken = pdFALSE;
	/* Test on DMA Stream Transfer Complete interrupt */
	if ( DMA_GetITStatus ( DMA1_Stream7, DMA_IT_TCIF7 ) )
	{
		/* Clear DMA Stream Transfer Complete interrupt pending bit */
		DMA_ClearITPendingBit ( DMA1_Stream7, DMA_IT_TCIF7 );

		lHigherPriorityTaskWoken = pdFALSE;
		// Unblock the task by releasing the semaphore.
		CmdMsg.Command = 't';
		xQueueSendToBackFromISR ( xAudioQueue, &CmdMsg, &lHigherPriorityTaskWoken );
		portEND_SWITCHING_ISR ( lHigherPriorityTaskWoken );
	};
};

void EVAL_AUDIO_HalfTransfer_CallBack(uint32_t pBuffer, uint32_t Size)
{
	struct AAudioCommandMessage CmdMsg;
	long lHigherPriorityTaskWoken = pdFALSE;

	/* Test on DMA Stream Transfer Complete interrupt */
	if ( DMA_GetITStatus ( DMA1_Stream7, DMA_IT_HTIF7 ) )
	{
		/* Clear DMA Stream Transfer Complete interrupt pending bit */
		DMA_ClearITPendingBit ( DMA1_Stream7, DMA_IT_HTIF7 );

		lHigherPriorityTaskWoken = pdFALSE;
		// Unblock the task by releasing the semaphore.
		CmdMsg.Command = 'h';
		xQueueSendToBackFromISR ( xAudioQueue, &CmdMsg, &lHigherPriorityTaskWoken );
		portEND_SWITCHING_ISR ( lHigherPriorityTaskWoken );
	};
};

uint16_t EVAL_AUDIO_GetSampleCallBack(void)
{
	return ( 0 );
};

uint32_t Codec_TIMEOUT_UserCallback ( void )
{
	return ( 0 );
};

uint8_t AudioProvideSamplesFromWaveFile ( )
{
	FRESULT res;

	// read data into buffer
	res = f_read (
			&audioContext.fil,
			audioContext.sampleBuffer,
			sizeof ( audioContext.sampleBuffer ),
			( UINT * ) & audioContext.sampleBufferSize );

	if ( res )
	{
		f_close ( &audioContext.fil );
		audioContext.fileIsOpen = 0;
		return ( 3 );
	};

	if ( 0 == audioContext.sampleBufferSize )
	{
		audioContext.fileIsOpen = 0;
		f_close ( &audioContext.fil );
		return ( 4 );
	};

	return ( 0 );
};

uint8_t AudioStartPlayFile ( char * sFilename )
{
	FRESULT res;
	uint8_t rc = 0;
	// find the extension of the filename
	char * extension = strrchr ( sFilename, '.' );

	// no or wrong extension -> exit
	if ( NULL == extension ) return ( 1 );

	audioContext.format = FILEFORMAT_UNKNOWN;

	if (      !strnicmp ( extension, ".wav", 4 ) ) audioContext.format = FILEFORMAT_WAVE;
	else if ( !strnicmp ( extension, ".wav", 4 ) ) audioContext.format = FILEFORMAT_MP3;
	else if ( !strnicmp ( extension, ".flac", 4 ) ) audioContext.format = FILEFORMAT_FLAC;
	else if ( !strnicmp ( extension, ".fla", 4 ) ) audioContext.format = FILEFORMAT_FLAC;
	else if ( !strnicmp ( extension, ".ape", 4 ) ) audioContext.format = FILEFORMAT_APE;

	if ( FILEFORMAT_UNKNOWN == audioContext.format ) return ( 2 );

	// open file, on error exit
	res = f_open (
			&audioContext.fil,
			sFilename,
			FA_OPEN_EXISTING | FA_READ ); // open existing file in read mode

	if ( res ) return ( 2 );
	audioContext.fileIsOpen = 1;

	switch ( audioContext.format )
	{
	case FILEFORMAT_WAVE:
		rc = AudioProvideSamplesFromWaveFile ( );
		break;

	default:
		audioContext.fileIsOpen = 0;
		f_close ( &audioContext.fil );
		return ( 3 );
		break;
	};

	return ( rc );
};

void AudioTask ( void * pvParameters )
{
	struct AAudioCommandMessage CmdMsg;
	uint8_t rc = 0;
	FRESULT res;

	xAudioQueue = xQueueCreate ( 8, sizeof ( char ) );

	res = f_mount ( &FatFs, "", 1 ); // mount the drive
	if ( res )
	{
		led_state[0] = LED_STATE_BLINK_FAST;
		while ( 1 );
	}

	while ( 1 )
	{
		if ( xQueueReceive ( xAudioQueue, &CmdMsg, portMAX_DELAY ) )
		{
			switch ( CmdMsg.Command )
			{
			case 'h': // half transfer complete
				break;
			case 't': // transfer complete
				break;
			case 'p': // play file
				rc = AudioStartPlayFile ( CmdMsg.sFilename );
				switch ( rc )
				{
					case 0: EVAL_AUDIO_Play ( audioContext.dmaBuffer, audioContext.sampleBufferSize ); break;
					case 1: led_state[1] = LED_STATE_BLINK_FAST; break;
					case 2: led_state[2] = LED_STATE_BLINK_FAST; break;
				}
				break;
			case 'r': // pause/resume
				break;
			case 's': // stop playback
				EVAL_AUDIO_Stop ( CODEC_PDWN_SW );
				break;
			case 'v': // set volume
				break;
			case '+': // volume up
				break;
			case '-': // volume down
				break;
			case 'n': // next file
				break;
			case 'l': // last file
				break;
			};
		};
	};
};
