#include "DataCollectors.h"
#include <stdio.h> 
#include "../inc/tm4c123gh6pm.h"
#include "../inc/CortexM.h"
#include "../inc/LaunchPad.h"
#include "../inc/PLL.h"
#include "../inc/LPF.h"
#include "../RTOS_Labs_common/UART0int.h"
#include "../RTOS_Labs_common/ADC.h"
#include "../RTOS_Labs_common/OS.h"
#include "../RTOS_Labs_common/heap.h"
#include "../RTOS_Labs_common/Interpreter.h"
#include "../RTOS_Labs_common/ST7735.h"
#include "../RTOS_Labs_common/eDisk.h"
#include "../RTOS_Labs_common/eFile.h"
#include "esp8266.h"
#include "../inc/ADCSWTrigger.h"
#include "../inc/FIFO.h"
#include "Blynk.h"

#define FIFOSIZE    32      // size of the FIFOs (must be power of 2)
#define FIFOSUCCESS 1         // return value on success
#define FIFOFAIL    0         // return value on failure

Sema4Type MicSema;
Sema4Type HSRSema;
Sema4Type GSRSema;

AddIndexFifo(Mic, 50, int, FIFOSUCCESS, FIFOFAIL)   // These 3 FIFOs are for the original samples to be filtered

AddIndexFifo(HRS, FIFOSIZE, int, FIFOSUCCESS, FIFOFAIL)
	
AddIndexFifo(GRS, FIFOSIZE, int, FIFOSUCCESS, FIFOFAIL)
	
AddIndexFifo(Mic_Proxy, 50, int, FIFOSUCCESS, FIFOFAIL)  // These 3 FIFOs are the filtered samples to be sent over Blynk/ITFFT

AddIndexFifo(HRS_Proxy, FIFOSIZE, int, FIFOSUCCESS, FIFOFAIL)
	
AddIndexFifo(GRS_Proxy, FIFOSIZE, int, FIFOSUCCESS, FIFOFAIL)

void DataCollector_Init(void){
	OS_InitSemaphore(&MicSema, 0);
	OS_InitSemaphore(&HSRSema, 0);
	OS_InitSemaphore(&GSRSema, 0);
}

void MicSampler(void){
	ADC0_InitSWTriggerSeq3(0);
	if(MicFifo_Put(ADC0_InSeq3())==FIFOFAIL){
		OS_Signal(&MicSema);
	}
}
	
void HSRSampler(void){
	ADC0_InitSWTriggerSeq3(1);
	if(HRSFifo_Put(ADC0_InSeq3())==FIFOFAIL){
		OS_Signal(&HSRSema);
	}
}
	
void GSRSampler(void){
	ADC0_InitSWTriggerSeq3(2);
	if(GRSFifo_Put(ADC0_InSeq3())==FIFOFAIL){
		OS_Signal(&GSRSema);
	}
}


void MicThread(){
	
	while(1){
	OS_Wait(&MicSema);
	LPF_Init(0, 50);
	for(int i=0; i<50; i++){
		  int data;
			LPF_Calc(MicFifo_Get(&data));
		  Mic_ProxyFifo_Put(data);
			SendInformation(data);
	}
	}
	// Invoke ITFFT Process and filtering?
	
	
}

int16_t x = 0;
int16_t y = ST7735_TFTHEIGHT / 2;
int16_t color = ST7735_BLUE;
int divisor = 34;

void GraphThread(){
	//determine ypoint here
	int data;
	int prev_y = 25;
	
	ST7735_InitR(INITR_REDTAB);
	while(1){
		while(HRS_ProxyFifo_Get(&data) != FIFOFAIL){
			//y = (data / divisor) + 20;
			y = (data > 1800) ? 105 : 25;
			ST7735_DrawFastVLine(x, 0,  ST7735_TFTHEIGHT,  ST7735_WHITE);
			
			if(y != prev_y){
				ST7735_DrawFastVLine(x, 25,  80,  color);
			}else{
				//y = (data / divisor);
				ST7735_DrawPixel(x, y, color);
			}
			
			prev_y = y;
			x = (x + 1) % ST7735_TFTWIDTH;
		}
		OS_Suspend();
	}
}

/*void HSRThread(){
	while(1){
	OS_Wait(&HSRSema);
	// Invoke ITFFT Process and filtering?
	LPF_Init2(0, FIFOSIZE);
	for(int i=0; i<FIFOSIZE; i++){
		  int data;
			//LPF_Calc2(HRSFifo_Get(&data));
			HRSFifo_Get(&data);
		  HRS_ProxyFifo_Put(data);
			//SendInformation(data);
			UART_OutUDec(data);
			UART_OutString("\n\r");
}
}
}*/


void HSRThread(){
	while(1){
	OS_Wait(&HSRSema);
	DisableInterrupts();
	// Invoke ITFFT Process and filtering?
	//LPF_Init2(0, FIFOSIZE);
	int beat = 0;
	int prev_data;
	(HRSFifo_Get(&prev_data));
	for(int i=1; i<FIFOSIZE; i++){
		int data;
		(HRSFifo_Get(&data));
		if((data-prev_data)>500 || ((prev_data-data)>500))
			beat++;
		prev_data = data;
		HRS_ProxyFifo_Put(data);
 
		//SendInformation(data);
		//UART_OutUDec(data);
		//UART_OutString("\n\r");
	}
	EnableInterrupts();
	UART_OutUDec(beat*20);
	//ST7735_DrawString(0,0,"HR",ST7735_BLUE);
	//ST7735_SetCursor(20,0);
	//ST7735_OutUDec(beat*20);
	UART_OutString("\n\r");
	}
}

void GSRThread(){
	while(1){
	OS_Wait(&GSRSema);
	LPF_Init3(0, FIFOSIZE);
	for(int i=0; i<FIFOSIZE; i++){
		  int data;
			LPF_Calc3(GRSFifo_Get(&data));
		  GRS_ProxyFifo_Put(data);
		  SendInformation(data);
	}
}
}

void ITFFT_Process(){
	ST7735_InitB();
	ESP8266_Init();       // Enable ESP8266 Serial Port
  ESP8266_Reset();      // Reset the WiFi module
  ESP8266_SetupWiFi();  // Setup communications to Blynk Server  

}

