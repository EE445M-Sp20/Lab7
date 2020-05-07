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

AddIndexFifo(Mic, 1024, int, FIFOSUCCESS, FIFOFAIL)   // These 3 FIFOs are for the original samples to be filtered

AddIndexFifo(HRS, FIFOSIZE, int, FIFOSUCCESS, FIFOFAIL)
	
AddIndexFifo(GRS, FIFOSIZE, int, FIFOSUCCESS, FIFOFAIL)
	
AddIndexFifo(Mic_Proxy, 1024, int, FIFOSUCCESS, FIFOFAIL)  // These 3 FIFOs are the filtered samples to be sent over Blynk/ITFFT

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
	LPF_Init(0, 1024);
	for(int i=0; i<1024; i++){
		  int data;
			LPF_Calc(MicFifo_Get(&data));
		  Mic_ProxyFifo_Put(data);
	}
	}
	// Invoke ITFFT Process and filtering?
	
	
}

void HSRThread(){
	while(1){
	OS_Wait(&HSRSema);
	// Invoke ITFFT Process and filtering?
	LPF_Init2(0, FIFOSIZE);
	for(int i=0; i<FIFOSIZE; i++){
		  int data;
			LPF_Calc2(MicFifo_Get(&data));
		  HRS_ProxyFifo_Put(data);
	}
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
	}
}
}

void ITFFT_Process(){
	ESP8266_Init();       // Enable ESP8266 Serial Port
  ESP8266_Reset();      // Reset the WiFi module
  ESP8266_SetupWiFi();  // Setup communications to Blynk Server  

}
