#ifndef __USB_TASK_H__
#define __USB_TASK_H__

#include "FreeRTOS.h"
#include "semphr.h"

extern xSemaphoreHandle xCDCSemaphore;


void USBTask ( void * pvParameters );

#endif // __USB_TASK_H__
