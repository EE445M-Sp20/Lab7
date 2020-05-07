// Lab5.c
// Runs on LM4F120/TM4C123
// Real Time Operating System for Lab 5

// Jonathan W. Valvano 3/29/17, valvano@mail.utexas.edu
// Andreas Gerstlauer 3/1/16, gerstl@ece.utexas.edu
// EE445M/EE380L.6 
// You may use, edit, run or distribute this file 
// You are free to change the syntax/organization of this file

// LED outputs to logic analyzer for use by OS profile 
// PF1 is preemptive thread switch
// PF2 is first periodic background task (if any)
// PF3 is second periodic background task (if any)
// PC4 is PF4 button touch (SW1 task)

// Outputs for task profiling
// PD0 is idle task
// PD1 is button task

// Button inputs
// PF0 is SW2 task
// PF4 is SW1 button input

// Analog inputs
// PE3 Ain0 sampled at 2kHz, sequencer 3, by Interpreter, using software start

#include <stdint.h>
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
#include "DataCollectors.h"


uint32_t NumCreated;   // number of foreground threads created
uint32_t IdleCount;    // CPU idle counter
uint32_t Count4;
uint32_t FilterWork;

//---------------------User debugging-----------------------
extern int32_t MaxJitter;             // largest time jitter between interrupts in usec

#define PD0  (*((volatile uint32_t *)0x40007004))
#define PD1  (*((volatile uint32_t *)0x40007008))
#define PD2  (*((volatile uint32_t *)0x40007010))
#define PD3  (*((volatile uint32_t *)0x40007020))

void PortD_Init(void){ 
  SYSCTL_RCGCGPIO_R |= 0x08;       // activate port D
  while((SYSCTL_RCGCGPIO_R&0x08)==0){};      
  GPIO_PORTD_DIR_R |= 0x0F;        // make PD3-0 output heartbeats
  GPIO_PORTD_AFSEL_R &= ~0x0F;     // disable alt funct on PD3-0
  GPIO_PORTD_DEN_R |= 0x0F;        // enable digital I/O on PD3-0
  GPIO_PORTD_PCTL_R = ~0x0000FFFF;
  GPIO_PORTD_AMSEL_R &= ~0x0F;;    // disable analog functionality on PD
}


//------------------Task 1--------------------------------
// background thread executes with SW1 button
// one foreground task created with button push

// ***********ButtonWork*************
void ButtonWork(void){  heap_stats_t heap;
  uint32_t myId = OS_Id(); 
  PD1 ^= 0x02;
  if(Heap_Stats(&heap)) OS_Kill();
  PD1 ^= 0x02;
  ST7735_Message(1,0,"Heap size  =",heap.size); 
  ST7735_Message(1,1,"Heap used  =",heap.used);  
  ST7735_Message(1,2,"Heap free  =",heap.free);
  ST7735_Message(1,3,"Heap waste =",heap.size - heap.used - heap.free);
  PD1 ^= 0x02;
  OS_Kill();  // done, OS does not return from a Kill
} 

//************SW1Push*************
// Called when SW1 Button pushed
// Adds another foreground task
// background threads execute once and return
void SW1Push(void){
  if(OS_MsTime() > 20){ // debounce
    if(OS_AddThread(&ButtonWork,100,2)){
      NumCreated++; 
    }
    OS_ClearMsTime();  // at least 20ms between touches
  }
}

//************SW2Push*************
// Called when SW2 Button pushed
// Adds another foreground task
// background threads execute once and return
void SW2Push(void){
  if(OS_MsTime() > 20){ // debounce
    if(OS_AddThread(&ButtonWork,100,2)){
      NumCreated++; 
    }
    OS_ClearMsTime();  // at least 20ms between touches
  }
}

//--------------end of Task 1-----------------------------

//------------------Idle Task--------------------------------
// foreground thread, runs when nothing else does
// never blocks, never sleeps, never dies
// inputs:  none
// outputs: none
void Idle(void){
  IdleCount = 0; 
  PortD_Init();	
  while(1) {
    IdleCount++;
    PD0 ^= 0x01;
    WaitForInterrupt();
  }
}

//--------------end of Idle Task-----------------------------

//*******************final user main DEMONTRATE THIS TO TA**********
int realmain(void){ // realmain
  OS_Init();        // initialize, disable interrupts
  PortD_Init();     // debugging profile
  MaxJitter = 0;    // in 1us units
	PLL_Init(Bus80MHz);
  // hardware init
  ADC_Init(0);  // sequencer 3, channel 0, PE3, sampling in Interpreter
  LaunchPad_Init();
	
  Heap_Init();  // initialize heap
	DataCollector_Init();
	ITFFT_Process();
  
  // attach background tasks
  //OS_AddPeriodicThread(&disk_timerproc,TIME_1MS,0);   // time out routines for disk  
	OS_AddPeriodicThread(&MicSampler,TIME_1MS/40,0);   // time out routines for disk 
	OS_AddPeriodicThread(&HSRSampler,TIME_1MS*100,0);   // time out routines for disk 
	OS_AddPeriodicThread(&GSRSampler,TIME_1MS*333,0);   // time out routines for disk 
  OS_AddSW1Task(&SW1Push,2);
  OS_AddSW2Task(&SW2Push,2);  

  // create initial foreground threads
  NumCreated = 0;
  //NumCreated += OS_AddThread(&Interpreter,128,2); 
  NumCreated += OS_AddThread(&Idle,128,5);  // at lowest priority 
	NumCreated += OS_AddThread(&MicThread, 128, 1);
	NumCreated += OS_AddThread(&HSRThread, 128, 1);
	NumCreated += OS_AddThread(&GSRThread, 128, 1);
  //NumCreated += OS_AddThread(&ITFFT_Process, 128, 1);
  OS_Launch(TIME_2MS); // doesn't return, interrupts enabled in here
  return 0;            // this never executes
}

int main(){
	realmain();
}