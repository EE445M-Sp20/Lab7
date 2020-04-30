// *************Interpreter.c**************
// Students implement this as part of EE445M/EE380L.12 Lab 1,2,3,4 
// High-level OS user interface
// 
// Runs on LM4F120/TM4C123
// Jonathan W. Valvano 1/18/20, valvano@mail.utexas.edu
#include <stdint.h>
#include <string.h> 
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h> 
#include "../RTOS_Labs_common/OS.h"
#include "../RTOS_Labs_common/ST7735.h"
#include "../RTOS_Lab5_ProcessLoader/loader.h"
#include "../inc/ADCT0ATrigger.h"
#include "../inc/ADCSWTrigger.h"
#include "../RTOS_Labs_common/UART0int.h"
#include "../RTOS_Labs_common/eDisk.h"
#include "../RTOS_Labs_common/eFile.h"
#include "../RTOS_Labs_common/ST7735.h"
#include "../RTOS_Labs_common/OS.h"
#include "../inc/tm4c123gh6pm.h"
#include "../inc/CortexM.h"
#include "../RTOS_Labs_common/esp8266.h"
#include "../inc/LaunchPad.h"


// Command protocols (will be extended through)
#define PROTO "lcd_out"
#define ADCIN "adcin"
#define CLEAR_SCREEN "cls"
#define START_TIMER "start_timer"
#define GET_TIME_ELAPSED "get_time"
#define RESET_TIME "reset_time"
#define ADC_INIT   "adc_init"
#define JITTER_HIST "jitter_hist"
#define EN_DN_STATS "enable_frac"
#define CPU_UTIL    "cpu_util"
#define FORMAT_FILESYSTEM "format_filesys"
#define PRINT_DIRECTORY "ls"
#define CREATE_FILE     "touch"
#define WRITE_TO_FILE   "fwrite"
#define PRINT_FILE      "fread"
#define DELETE_FILE     "rm"
#define LOAD_ELF        "load"


#define RPC_ADC_IN "0"
#define RPC_OS_Time     "1"
#define RPC_ST7735_Message "2"
#define RPC_eFile_format   "3"
#define RPC_eFile_create   "4"
#define RPC_eFile_read   "5"
#define RPC_eFile_write   "6"
#define RPC_exec_elf     "7"
#define RPC_Toggle_led       "8"

#define RPC_CLIENT_RECEIVE "rpc_client_receive"


//Bounds and Errors (will be extended through)
#define MAXIMUM_COMMAND_WORD_LENGTH 20
#define MAXIMUM_NUM_ARGUMENTS       10

extern int32_t MaxJitter;             // largest time jitter between interrupts in usec
extern uint32_t const JitterSize;
extern uint32_t JitterHistogram[];
extern uint32_t FilterWork;
extern uint32_t Disabled;
extern uint32_t Enabled;
extern uint32_t Disabled_usecs;
extern uint32_t Disabled_percentage;
extern uint32_t CPUUtil;
void decode_and_transmit(char* buffer);
void number_to_string(uint32_t, char*);
void setup_args(char*, char**);

// Print jitter histogram
void Jitter(int32_t MaxJitter, uint32_t const JitterSize, uint32_t JitterHistogram[]){
  // write this for Lab 3 (the latest)
	UART_OutString("Max Jitter = ");
	UART_OutUDec(MaxJitter);
	UART_OutChar('\n');
	UART_OutChar('\r');
	
	UART_OutString("Jitter Size = ");
	UART_OutUDec(JitterSize);
	UART_OutChar('\n');
	UART_OutChar('\r');
	
	UART_OutString("The jitter histogram is \n\r");
	for(int i=0; i<JitterSize; i++){
	//UART_OutString("Max Jitter = ");
	UART_OutUDec(i);
	UART_OutChar(' ');
	UART_OutUDec(JitterHistogram[i]);
	UART_OutChar('\n');
	UART_OutChar('\r');
	}
	
	UART_OutUDec(FilterWork);
	UART_OutChar('\n');
	UART_OutChar('\r');
	
	
	
}

// *********** Command line interpreter (shell written from scratch) ************


// ******** split ************
// parses and splits the input command into desired form
// If command or argument length exceeds size limit, it becomes an invalid command
// and is just skipped (TODO: Send an error message in this case)
// input:  a pointer to arrays of characters which is where the individual tokens are to be stored
//         pointer to characters (array) which is the input line from user           
// output: none
void split(char (*tokens)[MAXIMUM_COMMAND_WORD_LENGTH], char* input){
		int i=0;
		int arg_ctr=0;
		int tmp_ctr=0;
		i=0;
		while(input[i]!='\0' && arg_ctr<=9){
		while(input[i]!=' ' && i<=99){
		tokens[arg_ctr][tmp_ctr]=input[i];
		i++;
		tmp_ctr++;

		}
		tokens[arg_ctr][tmp_ctr]='\0';             
		arg_ctr++;
		i++;
		tmp_ctr=0;
	}

}

int strCmp(char string1[], char string2[] )
{
	for (int i = 0; ; i++){
	 if (string1[i] != string2[i]){
				return string1[i] < string2[i] ? -1 : 1;
		}

		if (string1[i] == '\0'){
				return 0;
		}
	}
}
char* my_itoa(int number, char str[]) {
    //create an empty string to store number
   sprintf(str, "%d", number); //make the number into string using sprintf function
   return str;
}

int string_to_int(char* inp){
return atoi(inp);  // Just returns 0 if no number or invalid number is present
}

// ******** split ************
// Matches the command with the protocol and invokes the appropriate command with the arguments
// Does nothing if command is invalid (TODO: Sens an error message over UART)
// input:  a pointer to arrays of characters which is where the individual tokens are be stored        
// output: none
void decode_exec(char (*tokens)[MAXIMUM_COMMAND_WORD_LENGTH]){
	if(strCmp(tokens[0], PROTO )==0){
	int arg = string_to_int(tokens[1]);
	int arg2 = string_to_int(tokens[2]);
	ST7735_Message(arg, arg2, tokens[3], string_to_int(tokens[4]));

}

if(strCmp(tokens[0], "cls")==0){
}

if(strCmp(tokens[0], START_TIMER)==0){
	OS_ClearMsTime();
}

if(strCmp(tokens[0], GET_TIME_ELAPSED)==0){
	ST7735_Message(0, 0, "Time elapsed =", OS_MsTime());
}

if(strCmp(tokens[0], RESET_TIME)==0){
	OS_ClearMsTime();
}

if(strCmp(tokens[0], ADC_INIT)==0){
	ADC0_InitSWTriggerSeq3_Ch9();
}

if(strCmp(tokens[0], ADCIN)==0){
	ST7735_Message(0, 0, "adc_IN =", ADC0_InSeq3());
}

if(strCmp(tokens[0], JITTER_HIST)==0){
	Jitter(MaxJitter, JitterSize, JitterHistogram);
}

if(strCmp(tokens[0], LOAD_ELF)==0){
	int mount = eFile_Mount();
	int open = eFile_ROpen(tokens[1]);
static const ELFSymbol_t symtab[] = {
{ "ST7735_Message", ST7735_Message }
};
ELFEnv_t env = { symtab, 1 };
	int result =  exec_elf(tokens[1], &env);

}

if(strCmp(tokens[0], EN_DN_STATS)==0){
	UART_OutString("Disabled millisecs: ");
	UART_OutUDec(Disabled/80000);
	UART_OutString("\n\r");
	
	UART_OutString("Disabled percentage: ");
	UART_OutUDec(Disabled_percentage);
	UART_OutString("\n\r");
	
}

if(strCmp(tokens[0], CPU_UTIL)==0){
	UART_OutString("\n\r");
}


if(strCmp(tokens[0], FORMAT_FILESYSTEM)==0){
	eFile_Mount();
	eFile_Init();
	eFile_Format();
	UART_OutString("\n\r");
}

if(strCmp(tokens[0], PRINT_DIRECTORY)==0){
	eFile_DOpen("");
	char *name;
	unsigned long size;
	while(eFile_DirNext(&name, &size)!=1){
		int i=0;
		while(name[i]!=0){
		UART_OutChar(name[i]);
		i++;
		}
		UART_OutString("\n\r");
		UART_OutUDec(size);
		
		UART_OutString("\n\r");
		UART_OutString("\n\r");
		
	};
	UART_OutString("\n\r");
}

if(strCmp(tokens[0], CREATE_FILE)==0){
	int i=0;
	while(tokens[1][i] !=0){
		i++;
	}
	if(i<=7){
	int res = eFile_Create(tokens[1]);
	if(res){
		UART_OutString("Creation failure \n\r");
	}
	else{
		UART_OutString("Creation success \n\r");
	}
	}
	UART_OutString("\n\r");
}

if(strCmp(tokens[0], WRITE_TO_FILE)==0){
		int i=0;
	while(tokens[1][i] !=0){
		i++;
	}
	if(i<=7){
	int res = eFile_WOpen(tokens[1]);
	if(res){
		UART_OutString("Open failure \n\r");
	}
	else{
		UART_OutString("Open success \n\r");
		res = eFile_Write(tokens[2][0]);
			if(res){
				UART_OutString("Write failure \n\r");
			}
			else{
				UART_OutString("Write success \n\r");
				res = eFile_WClose();
			}
	}
	}
	UART_OutString("\n\r");
}

if(strCmp(tokens[0], PRINT_FILE)==0){
			int i=0;
	while(tokens[1][i] !=0){
		i++;
	}
	if(i<=7){
	int res = eFile_ROpen(tokens[1]);
	if(res){
		UART_OutString("Open failure \n\r");
	}
	else{
		UART_OutString("Open success \n\r");
		char ch;
		while( eFile_ReadNext(&ch)!=1){
			UART_OutChar(ch);
		}
		eFile_RClose();

	}
	}
	UART_OutString("\n\r");
}

if(strCmp(tokens[0], DELETE_FILE)==0){
	int res = eFile_Delete(tokens[1]);
	if(res){
	 UART_OutString("Delete failed\n\r");
	}
	else{
		UART_OutString("Delete successful\n\r");
	}
	UART_OutString("\n\r");
}

if(strCmp(tokens[0], RPC_ADC_IN)==0){
	const char* adc = "adcin";
	
	ESP8266_Send("0 \r\n");
	char Rec[10];
	ESP8266_Receive(Rec, 10);
	UART_OutString("Received ADC ");
	UART_OutString(Rec);
	
}

if(strCmp(tokens[0], RPC_OS_Time)==0){
	const char* ostime = "ostime";
	
	ESP8266_Send("1 \r\n");
	char Rec[10];
	ESP8266_Receive(Rec, 10);
	UART_OutString("Received Time ");
	UART_OutString(Rec);
	

}

if(strCmp(tokens[0], RPC_ST7735_Message)==0){

	const char* st7735 = "st7735";
	char transmit_message[100];
	transmit_message[0]='2';
	transmit_message[1]=' ';
	int ctr=2;
	int i=0;
	while(tokens[1][i] != 0){
		transmit_message[ctr] = tokens[1][i];
		ctr++;
		i++;
	}
	transmit_message[ctr]=' ';
	ctr++;
	i=0;
	while(tokens[2][i] != 0){
		transmit_message[ctr] = tokens[2][i];
		ctr++;
		i++;
	}
	transmit_message[ctr]=' ';
	ctr++;
	i=0;
	while(tokens[3][i] != 0){
		transmit_message[ctr] = tokens[3][i];
		ctr++;
		i++;
	}
	transmit_message[ctr]=' ';
	ctr++;
	i=0;
	while(tokens[4][i] != 0){
		transmit_message[ctr] = tokens[4][i];
		ctr++;
		i++;
	}
	transmit_message[ctr++]=' ';
	transmit_message[ctr++]='\r';
	transmit_message[ctr++]='\n';
	transmit_message[ctr++]=0;
	ctr++;
	ESP8266_Send(transmit_message);	
	
}

if(strCmp(tokens[0], RPC_eFile_format)==0){
ESP8266_Send("3 \r\n");	
}



if(strCmp(tokens[0], RPC_eFile_create)==0){
	char transmit_message[100];
	transmit_message[0]='4';
	transmit_message[1]=' ';
	int ctr=2;
	int i=0;
	while(tokens[1][i] != 0){
		transmit_message[ctr] = tokens[1][i];
		ctr++;
		i++;
	}
	transmit_message[ctr++]=' ';
	transmit_message[ctr++]='\r';
	transmit_message[ctr++]='\n';
	transmit_message[ctr++]=0;
	ESP8266_Send(transmit_message);	
	
}

if(strCmp(tokens[0], RPC_eFile_write)==0){
	char transmit_message[100];
	transmit_message[0]='6';
	transmit_message[1]=' ';
	int ctr=2;
	int i=0;
	while(tokens[1][i] != 0){
		transmit_message[ctr] = tokens[1][i];
		ctr++;
		i++;
	}
	transmit_message[ctr]=' ';
	ctr++;
	i=0;
	while(tokens[2][i] != 0){
		transmit_message[ctr] = tokens[2][i];
		ctr++;
		i++;
	}
	transmit_message[ctr++]=' ';
	transmit_message[ctr++]='\r';
	transmit_message[ctr++]='\n';
	transmit_message[ctr++]=0;
	ESP8266_Send(transmit_message);
}



if(strCmp(tokens[0], RPC_exec_elf)==0){
	char transmit_message[100];
	transmit_message[0]='7';
	transmit_message[1]=' ';
	int ctr=2;
	int i=0;
	while(tokens[1][i] != 0){
		transmit_message[ctr] = tokens[1][i];
		ctr++;
		i++;
	}
	transmit_message[ctr++]=' ';
	transmit_message[ctr++]='\r';
	transmit_message[ctr++]='\n';
	transmit_message[ctr++]=0;
	ESP8266_Send(transmit_message);	
}

if(strCmp(tokens[0], RPC_eFile_read)==0){
	char transmit_message[100];
	transmit_message[0]='5';
	transmit_message[1]=' ';
	int ctr=2;
	int i=0;
	while(tokens[1][i] != 0){
		transmit_message[ctr] = tokens[1][i];
		ctr++;
		i++;
	}
	transmit_message[ctr++]=' ';
	transmit_message[ctr++]='\r';
	transmit_message[ctr++]='\n';
	transmit_message[ctr++]=0;
	ESP8266_Send(transmit_message);	


}

if(strCmp(tokens[0], RPC_Toggle_led)==0){
	ESP8266_Send("8 \r\n");
}

}





void decode_and_transmit(char* buffer){
	
	int i =0;
	char first_command[50];
	char** args;
	//setup_args(buffer, args);
	while(buffer[i]!=0 && buffer[i]!=' '){
		first_command[i]=buffer[i];
	}
	first_command[i]=0;
	if((strcmp(first_command, ADCIN)==0)){
		char arg[100];
		number_to_string(ADC0_InSeq3(), arg);
		ESP8266_Send(arg);
	}
	
	if((strcmp(first_command, CLEAR_SCREEN)==0)){
	//	Output_Clear();
	}
	
	if((strcmp(first_command, PROTO)==0)){
		
		ST7735_Message(string_to_int(args[1]), string_to_int(args[2]), args[3], string_to_int(args[4]));
	}
	
	if((strcmp(first_command, PRINT_FILE)==0)){
		
	}
	
}

void number_to_string(uint32_t number, char* buffer){
}

//format, print directory, create file, write to file, print file,
//delete file
// ******** Interpreter ************
// Reads in command from UART, tokenizes the arguments and decodes and executes them by matching
// the commands to protocols
// input:  None         
// output: none
void Interpreter(){
	while(1){
	char string[MAXIMUM_COMMAND_WORD_LENGTH];
	UART_InString(string, 99);
	char args[MAXIMUM_NUM_ARGUMENTS][MAXIMUM_COMMAND_WORD_LENGTH];
	split(args, string);
	decode_exec(args);
	PF3 ^= 0x08;
	OS_Suspend();
	}
}



// CLIENT SIDE RPC


void RPCConnectWifi(){
  // Initialize and bring up Wifi adapter  
	ST7735_InitR(INITR_REDTAB);
  if(!ESP8266_Init(true,false)) {  // verbose rx echo on UART for debugging
    ST7735_DrawString(0,1,"No Wifi adapter",ST7735_YELLOW); 
    OS_Kill();
  }
  // Get Wifi adapter version (for debugging)
  ESP8266_GetVersionNumber(); 
  // Connect to access point
  if(!ESP8266_Connect(true)) {  
    ST7735_DrawString(0,1,"No Wifi network",ST7735_YELLOW); 
    OS_Kill();
  }
  ST7735_DrawString(0,1,"Wifi connected",ST7735_GREEN);
   if(!ESP8266_MakeTCPConnection("10.0.0.20", 1025, 0)){ // open socket to web server on port 80
    ST7735_DrawString(0,2,"Connection failed",ST7735_YELLOW); 
    //Running = 0;
    OS_Kill();
  }    
  // Launch thread to fetch weather  
 // if(OS_AddThread(&SendRPC_Request,128,1)){}// NumCreated++;
  // Kill thread (should really loop to check and reconnect if necessary
	OS_Kill();
		while(1){
			OS_Suspend();
		
		}
  //OS_Kill(); 
}  


// SERVER SIDE RPC



void decode_exec_rpc_server(char (*tokens)[MAXIMUM_COMMAND_WORD_LENGTH]){


if(strCmp(tokens[0], RPC_ADC_IN)==0){
	//const char* adc = "adcin";
	char buffer[20];
	ADC0_InitSWTriggerSeq3_Ch9();
	uint32_t adc = ADC0_InSeq3();
	buffer[3]='\r';
	buffer[4]='\n';
	ESP8266_Send(my_itoa(adc,buffer));   // here 10 means decimal
	
}

if(strCmp(tokens[0], RPC_OS_Time)==0){
//	const char* ostime = "ostime";
	char buffer[20];
	uint32_t time = OS_Time();
	buffer[7]='\r';
	buffer[8]='\n';
	ESP8266_Send(my_itoa(time,buffer));   // here 10 means decimal
}

if(strCmp(tokens[0], RPC_ST7735_Message)==0){

//	const char* st7735 = "st7735";

	int arg = string_to_int(tokens[1]);
	int arg2 = string_to_int(tokens[2]);
	ST7735_Message(arg, arg2, tokens[3], string_to_int(tokens[4]));
	
}

if(strCmp(tokens[0], RPC_eFile_format)==0){
	eFile_Mount();
	eFile_Init();
	eFile_Format();
}

if(strCmp(tokens[0], RPC_eFile_create)==0){
	int i=0;
	while(tokens[1][i] !=0){
		i++;
	}
	if(i<=7){
	int res = eFile_Create(tokens[1]);
	if(res){
		UART_OutString("Creation failure \n\r");
	}
	else{
		UART_OutString("Creation success \n\r");
	}
	}
	UART_OutString("\n\r");
}

if(strCmp(tokens[0], RPC_eFile_read)==0){
			int i=0;
	while(tokens[1][i] !=0){
		i++;
	}
	if(i<=7){
	int res = eFile_ROpen(tokens[1]);
	if(res){
		UART_OutString("Open failure \n\r");
	}
	else{
		UART_OutString("Open success \n\r");
		char ch[100];
		int i =0;
		while( eFile_ReadNext(&ch[i])!=1 && i<100){
			UART_OutChar(ch[i]);
			i++;
		}
		ch[i++]='\r';
		ch[i++]='\n';
		ch[i++]=0;
		
		ESP8266_Send(ch);
		eFile_RClose();

	}
	}
	UART_OutString("\n\r");
}

if(strCmp(tokens[0], RPC_eFile_write)==0){
		int i=0;
	while(tokens[1][i] !=0){
		i++;
	}
	if(i<=7){
	int res = eFile_WOpen(tokens[1]);
	if(res){
		UART_OutString("Open failure \n\r");
	}
	else{
		UART_OutString("Open success \n\r");
		res = eFile_Write(tokens[2][0]);
			if(res){
				UART_OutString("Write failure \n\r");
			}
			else{
				UART_OutString("Write success \n\r");
				res = eFile_WClose();
			}
	}
	}
	UART_OutString("\n\r");
}

//if(strCmp(tokens[0], RPC_exec_elf)==0){
//	const char* adc = "exec_elf ";
//	const char* arg = tokens[1];
//	int i = 0;
//	char transmit[100];
//	while(tokens[i]!=0){
//		transmit[9+i]=tokens[1][i];
//		i++;
//	}
//	ESP8266_Send(adc);	
//}

if(strCmp(tokens[0], RPC_exec_elf)==0){
	int mount = eFile_Mount();
	int open = eFile_ROpen(tokens[1]);
static const ELFSymbol_t symtab[] = {
{ "ST7735_Message", ST7735_Message }
};
ELFEnv_t env = { symtab, 1 };
	int result =  exec_elf(tokens[1], &env);

}

if(strCmp(tokens[0], RPC_CLIENT_RECEIVE)==0){
	char received_buffer[100];
	ESP8266_Receive(received_buffer, 100);
	decode_and_transmit(received_buffer);
}

if(strCmp(tokens[0], RPC_Toggle_led)==0){
	PF2 ^= 0x04;
}
}



Sema4Type WebServerRPCSema;

static void RPC_Server(){
	char string[MAXIMUM_COMMAND_WORD_LENGTH];
	ESP8266_Receive(string, 99);
	char args[MAXIMUM_NUM_ARGUMENTS][MAXIMUM_COMMAND_WORD_LENGTH];
	split(args, string);
	decode_exec_rpc_server(args);
	PF3 ^= 0x08;
	OS_Signal(&WebServerRPCSema);
	OS_Kill();	
}

void ServerRPC(void){
  // Initialize and bring up Wifi adapter 
		OS_InitSemaphore(&WebServerRPCSema,0);
  if(!ESP8266_Init(true,false)) {  // verbose rx echo on UART for debugging
    ST7735_DrawString(0,1,"No Wifi adapter",ST7735_YELLOW); 
    OS_Kill();
  }
  // Get Wifi adapter version (for debugging)
  ESP8266_GetVersionNumber(); 
  // Connect to access point
  if(!ESP8266_Connect(true)) {  
    ST7735_DrawString(0,1,"No Wifi network",ST7735_YELLOW); 
    OS_Kill();
  }
  ST7735_DrawString(0,1,"Wifi connected",ST7735_GREEN);
  if(!ESP8266_StartServer(80,600)) {  // port 80, 5min timeout
    ST7735_DrawString(0,2,"Server failure",ST7735_YELLOW); 
    OS_Kill();
  }  
  ST7735_DrawString(0,2,"Server started",ST7735_GREEN);
   ESP8266_WaitForConnection();
  while(1) {
    // Wait for connection
   
    
    // Launch thread with higher priority to serve request
    if(OS_AddThread(&RPC_Server,128,1)) {}//NumCreated++;
    
    // The ESP driver only supports one connection, wait for the thread to complete
    OS_Wait(&WebServerRPCSema);
  }
}  

