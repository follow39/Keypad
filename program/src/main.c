#include "main.h"

	
QueueHandle_t xQueueSingleButton, xQueuePairButtons;
xSemaphoreHandle xSingleButtonShortPressed, 
									xPairButtonShortPressed, 
									xSingleButtonLongPressed, 
									xPairButtonLongPressed;

int main(void)
{
	//SystemInit();
	init();
	
	xQueueSingleButton =	xQueueCreate(8, sizeof(char));
	xQueueSingleButton =	xQueueCreate(8, sizeof(char));
	xSingleButtonShortPressed = xSemaphoreCreateBinary();
	xPairButtonShortPressed = xSemaphoreCreateBinary();
	xSingleButtonLongPressed = xSemaphoreCreateBinary();
	xPairButtonLongPressed = xSemaphoreCreateBinary();

	xTaskCreate(&vButtonCheck,
	            (char *)"ButtonCheck",
	            configMINIMAL_STACK_SIZE, 
	            NULL,
	            1,
	            NULL);
	
	xTaskCreate(&vSingleButtonShortPressed,
	            (char *)"SingleButtonShortPressed",
	            configMINIMAL_STACK_SIZE, 
	            NULL,
	            1,
	            NULL);
	
	xTaskCreate(&vPairButtonShortPressed,
	            (char *)"PairButtonShortPressed",
	            configMINIMAL_STACK_SIZE, 
	            NULL,
	            1,
	            NULL);

	xTaskCreate(&vSingleButtonLongPressed,
	            (char *)"SingleButtonLongPressed",
	            configMINIMAL_STACK_SIZE, 
	            NULL,
	            1,
	            NULL);

	xTaskCreate(&vPairButtonLongPressed,
	            (char *)"PairButtonLongPressed",
	            configMINIMAL_STACK_SIZE, 
	            NULL,
	            1,
	            NULL);

							
	vTaskStartScheduler();
}


void vButtonCheck(void *pvParameters)
{	
	uint16_t key_code = 0;
	
	const uint8_t delay = DELAY_SCAN;
	char temp_symbol = NONE_SYMBOL;	
	
	uint8_t button_dropped_counter = 0;
	uint16_t key_code_dropped = 0;
	
	uint8_t button_long_counter = 0;
	uint16_t key_code_long = 0;
	
	uint16_t buttons_time[BUTTONS_COUNT];
	uint8_t buttons_pressed[BUTTONS_COUNT];
	
	while (1)
	{
		button_dropped_counter = 0;
		key_code_dropped = 0;
		button_long_counter = 0;
		key_code_long = 0;
		
		key_code = readKeys();

		//loop for integrating ports press time and
		//detecting button drops
		for (int i = 0; i < BUTTONS_COUNT; ++i)
		{
			//if key pressed now
			if (key_code & (1 << i))
			{
				buttons_time[i] += delay;
				
				//condition for setting the button pressed flag
				if ((buttons_time[i] > SHORT_CLICK) && (buttons_time[i] < LONG_CLICK))
				{
					buttons_pressed[i] = 1;
				}
				
				//condition for setting the button long pressed flag				
				if (buttons_pressed[i] && (buttons_time[i] > LONG_CLICK))
				{
					++button_long_counter;
					key_code_long |= 1 << i;
				}
			}
			else
			{
				//if button wasn't press
				if (buttons_time[i] == 0)
					continue;
				
				//if button long press was process before
				if (buttons_time[i] > LONG_CLICK)
				{
					if (buttons_pressed[i] == 0)
					{
						buttons_time[i] = 0;
					}
				}
				
				//if button was dropped with short press
				if (buttons_time[i] > SHORT_CLICK)
				{
					buttons_time[i] = BUTTON_DROP_INIT;
				}
				
				if (buttons_time[i] > DECREASE_DROPPED)
				{
					buttons_time[i] -= DECREASE_DROPPED;
				}
				else
				{
					buttons_time[i] = 0;
					
					//if button was dropped with recorded short press
					if (buttons_pressed[i] != 0)
					{
						++button_dropped_counter;
						key_code_dropped |= 1 << i;
					}
				}
			}
		}
		
		if (button_dropped_counter != 0)
		{
			if (button_dropped_counter == 1)
			{
				for (int i = 0; i < BUTTONS_COUNT; ++i)
				{
					if (key_code_dropped & (1 << i))
					{
						temp_symbol = symbols[i];
					}
					
					if ((buttons_pressed[i] == 1) && (buttons_time[i] < SHORT_CLICK_ERROR))
					{
						++button_dropped_counter;
					}
				}
				
				if (button_dropped_counter == 1)
				{
					xQueueSend(xQueueSingleButton, (char *)&temp_symbol, (TickType_t )0);
					xSemaphoreGive(xSingleButtonShortPressed);
				}
				else
				{
					for (int i = 0; i < BUTTONS_COUNT; ++i)
					{
						if ((buttons_pressed[i] == 1) && (buttons_time[i] < SHORT_CLICK_ERROR))
						{
							xQueueSend(xQueuePairButtons, (char *)&temp_symbol, (TickType_t )0);
							xQueueSend(xQueuePairButtons, (char *)&symbols[i], (TickType_t )0);
							xSemaphoreGive(xPairButtonShortPressed);
							
							temp_symbol = NONE_SYMBOL;
							break;								
						}
					}
				}
			}
			else
			{
				for (int i = 0; i < BUTTONS_COUNT; ++i)
				{
					if (key_code_dropped & (1 << i))
					{
						if (temp_symbol == NONE_SYMBOL)
						{
							temp_symbol = symbols[i];
						}
						else
						{
							xQueueSend(xQueuePairButtons, (char *)&temp_symbol, (TickType_t )0);
							xQueueSend(xQueuePairButtons, (char *)&symbols[i], (TickType_t )0);
							xSemaphoreGive(xPairButtonShortPressed);
							
							temp_symbol = NONE_SYMBOL;
							break;
						}
					}
				}
			}
		}
		
		if (button_long_counter != 0)
		{
			if (button_long_counter == 1)
			{
				for (int i = 0; i < BUTTONS_COUNT; ++i)
				{
					if (key_code_long & (1 << i))
					{
						temp_symbol = symbols[i];
					}
					if ((buttons_pressed[i] == 1) && (buttons_time[i] > (LONG_CLICK) - LONG_CLICK_ERROR))
					{
						++button_long_counter;
					}
				}
				
				//if long pressed more than 1 button with LONG_CLICK_ERROR delay between them
				if (button_long_counter > 1)
				{
					for (int i = 0; i < BUTTONS_COUNT; ++i)
					{
						if ((buttons_pressed[i] == 1) && (buttons_time[i] > (LONG_CLICK) - LONG_CLICK_ERROR) && (!key_code_long & (1 << i)))
						{
							xQueueSend(xQueuePairButtons, (char *)&temp_symbol, (TickType_t )0);
							xQueueSend(xQueuePairButtons, (char *)&symbols[i], (TickType_t )0);
							xSemaphoreGive(xPairButtonLongPressed);
							
							temp_symbol = NONE_SYMBOL;
							break;								
						}
					}
				}
				else
				{
					//long pressed single button
					xQueueSend(xQueueSingleButton, (char *)&temp_symbol, (TickType_t )0);
					xSemaphoreGive(xSingleButtonLongPressed);
					
					temp_symbol = NONE_SYMBOL;
				}
			}
			else
			{
				//if long pressed more than 1 button
				for (int i = 0; i < BUTTONS_COUNT; ++i)
				{
					if (key_code_long & (1 << i))
					{
						if (temp_symbol == NONE_SYMBOL)
						{
							temp_symbol = symbols[i];
						}
						else
						{
							xQueueSend(xQueuePairButtons, (char *)&temp_symbol, (TickType_t )0);
							xQueueSend(xQueuePairButtons, (char *)&symbols[i], (TickType_t )0);
							xSemaphoreGive(xPairButtonLongPressed);
							
							temp_symbol = NONE_SYMBOL;
							break;
						}
					}
				}
			}
			
			for (int i = 0; i < BUTTONS_COUNT; ++i)
			{
				if (buttons_pressed[i] && (buttons_time[i] > (LONG_CLICK) - LONG_CLICK_ERROR))
				{
					buttons_time[i] = LONG_CLICK + 1;
					buttons_pressed[i] = 0;
				}				
			}			
		}
		
		
		vTaskDelay(delay);
	}	
}

//read the ports and convert to a 16-bit value, where 
//each bit represents a button with the corresponding number
uint16_t readKeys(void)
{
	uint16_t result = 0;
	
	char array[CLS_COUNT];
	
	uint16_t shift = 1 << CLS_START;
	
	//read rows state for each cls
	for (uint8_t i = 0; i < CLS_COUNT; ++i)
	{
		shift <<= i;
		
		GPIOA->ODR = shift^CLS_MASK;
		asm("nop");
		asm("nop");
		array[i] = GPIOA->IDR;
	}
	GPIOA->ODR |= CLS_MASK;
	
	
	//processing pressed buttons
	for (uint8_t i = 0; i < CLS_COUNT; ++i)
	{
		for (uint8_t j = 0; j < ROW_COUNT; ++j)
		{
			if (array[i] & (1 << j))
			{
				result |= (1 << (j*CLS_COUNT + i));
			}			
		}
	}
	
	return result;
}


void vSingleButtonShortPressed(void *pvParameters)
{
	char symbol;
	
	while (1)
	{
		xSemaphoreTake(xSingleButtonShortPressed, portMAX_DELAY);
		xQueueReceive(xQueueSingleButton, &(symbol), (TickType_t)10);
		single_key_pressed(symbol);
	}
}

void vPairButtonShortPressed(void *pvParameters)
{
	char symbol1;
	char symbol2;
	
	while (1)
	{
		xSemaphoreTake(xSingleButtonShortPressed, portMAX_DELAY);
		xQueueReceive(xQueueSingleButton, &(symbol1), (TickType_t)10);
		xQueueReceive(xQueueSingleButton, &(symbol2), (TickType_t)10);
		double_key_pressed(symbol1, symbol2);
	}
}

void vSingleButtonLongPressed(void *pvParameters)
{
	char symbol;
	
	while (1)
	{
		xSemaphoreTake(xSingleButtonShortPressed, portMAX_DELAY);
		xQueueReceive(xQueueSingleButton, &(symbol), (TickType_t)10);
		single_key_long_pressed(symbol);
	}
}

void vPairButtonLongPressed(void *pvParameters)
{
	char symbol1;
	char symbol2;
	
	while (1)
	{
		xSemaphoreTake(xSingleButtonShortPressed, portMAX_DELAY);
		xQueueReceive(xQueueSingleButton, &(symbol1), (TickType_t)10);
		xQueueReceive(xQueueSingleButton, &(symbol2), (TickType_t)10);
		double_key_long_pressed(symbol1, symbol2);
	}
}
