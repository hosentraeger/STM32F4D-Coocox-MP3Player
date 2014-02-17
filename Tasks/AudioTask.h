#ifndef __AUDIO_TASK_H__
#define __AUDIO_TASK_H__

#include "FreeRTOS.h"
#include "queue.h"

extern xQueueHandle xAudioQueue;

struct AAudioCommandMessage
{
	char Command;
	char * sFilename;
};

enum FILEFORMAT
{
	FILEFORMAT_UNKNOWN,
	FILEFORMAT_WAVE,
	FILEFORMAT_MP3,
	FILEFORMAT_FLAC,
	FILEFORMAT_APE
};

void AudioTask ( void * pvParameters );

#endif // __AUDIO_TASK_H__
