#include "FreeRTOS.h"
#include "task.h"
#include "stm32f4_discovery.h"
#include "USBTask.h"
#include "stm32_ub_usb_cdc.h"
#include "stdio.h"
#include "string.h"

void USBTask ( void * pvParameters )
{
	char buf[APP_TX_BUF_SIZE]; // puffer fuer Datenempfang
	USB_CDC_RXSTATUS_t check;

	// Init vom USB-OTG-Port als CDC-Device
	// (Virtueller-ComPort)
	UB_USB_CDC_Init ( );

	while ( 1 )
	{
		// Test ob USB-Verbindung zum PC besteht
		if(UB_USB_CDC_GetStatus()==USB_CDC_CONNECTED) {
			// Check ob Daten per USB empfangen wurden
			check=UB_USB_CDC_ReceiveString ( buf );
			if ( check == RX_READY )
			{
				// TODO: string received
			};
		};
	};
};
