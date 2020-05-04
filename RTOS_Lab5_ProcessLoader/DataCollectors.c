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
#include "../inc/ADCSWTrigger.h"
#include "../inc/FIFO.h"

#define FIFOSIZE    1024      // size of the FIFOs (must be power of 2)
#define FIFOSUCCESS 1         // return value on success
#define FIFOFAIL    0         // return value on failure

Sema4Type MicSema;

AddIndexFifo(Mic, FIFOSIZE, int, FIFOSUCCESS, FIFOFAIL)

AddIndexFifo(HRS, FIFOSIZE, int, FIFOSUCCESS, FIFOFAIL)
	
AddIndexFifo(GRS, FIFOSIZE, int, FIFOSUCCESS, FIFOFAIL)

void MicSampler(void){
	ADC0_InitSWTriggerSeq3(0);
	if(MicFifo_Put(ADC0_InSeq3())==FIFOFAIL){
		OS_Signal(&MicSema);
	}
}
	
void HSRSampler(void){
	ADC0_InitSWTriggerSeq3(1);
	HRSFifo_Put(ADC0_InSeq3());
}
	
void GSRSampler(void){
	ADC0_InitSWTriggerSeq3(2);
	GRSFifo_Put(ADC0_InSeq3());
}


void MainThread(){
	OS_Wait(&MicSema);
	// Invoke ITFFT Process and filtering?
	
	while(1){}
}

void ITFFT_Process(){
}
