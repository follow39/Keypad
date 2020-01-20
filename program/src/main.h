#include "stm32f429xx.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "initialization.h"

#define F_CPU 16000000UL
#define ROW_COUNT 4
#define ROW_START 0
#define ROW_MASK 0xF
#define CLS_COUNT 3
#define CLS_START 4
#define CLS_MASK 0x70
#define BUTTONS_COUNT 12
#define SHORT_CLICK 100
#define SHORT_CLICK_ERROR (SHORT_CLICK/2)
#define LONG_CLICK 1000
#define LONG_CLICK_ERROR (LONG_CLICK/10)
#define BUTTON_DROP_INIT (SHORT_CLICK-30)
#define DELAY_SCAN 10
#define DECREASE_DROPPED 10
#define NONE_SYMBOL '\0'


 /*
	* PA0-PA3 - ROW1-ROW4
	* PA4-PA6 - CLM1-CLM3
	*/


const char symbols[] = {'1', '2', '3',
												'4', '5', '6',
												'7', '8', '9',
												'*', '0', '#'};


int main(void);
inline uint16_t readKeys(void);
void vButtonCheck(void *pvParameters);
void vSingleButtonShortPressed(void *pvParameters);
void vPairButtonShortPressed(void *pvParameters);
void vSingleButtonLongPressed(void *pvParameters);
void vPairButtonLongPressed(void *pvParameters);

//*******************************************************
void single_key_pressed(char key){}
void double_key_pressed(char key1, char key2){}
void single_key_long_pressed(char key){}
void double_key_long_pressed(char key1, char key2){}
