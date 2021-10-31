/**
 * \file
 *
 * \brief Implementasi RTOS v10.2.1
 *
 */

#include <asf.h>
#include <stdio.h>
#include "FreeRTOS/include/FreeRTOS.h"
#include "FreeRTOS/include/queue.h"
#include "FreeRTOS/include/task.h"
#include "FreeRTOS/include/timers.h"
#include "FreeRTOS/include/semphr.h"


/* Define a task */
static portTASK_FUNCTION_PROTO(vBlinkLed0, p_);
static portTASK_FUNCTION_PROTO(vBlinkLed1, q_);
static portTASK_FUNCTION_PROTO(vCounter, r_);
static portTASK_FUNCTION_PROTO(vPushButton1, s_);

/* Define semaphore */
SemaphoreHandle_t xSemaphore;
uint16_t counter = 0;
int led0_cnt = 0;
int led1_cnt = 0;

static void buzzerEvenOdd() {
	if (counter % 2 == 0) {
		ioport_set_pin_level(J1_PIN0, 1);
		} else {
		ioport_set_pin_level(J1_PIN0, 0);
	}
}

int main (void)
{
	//set J1 Pin 0 to output
	ioport_set_pin_dir(J1_PIN0, IOPORT_DIR_OUTPUT);

	board_init();
	pmic_init();
	gfx_mono_init();
	
	gpio_set_pin_high(LCD_BACKLIGHT_ENABLE_PIN);
	gfx_mono_draw_string("RTOS Steven", 0, 0, &sysfont);
	gfx_mono_draw_string("Counter : 0", 0, 8, &sysfont);

	/* Create the task */
	
	xTaskCreate(vBlinkLed0, "", 1000, NULL, tskIDLE_PRIORITY + 1, NULL);	// higher priority
	xTaskCreate(vBlinkLed1, "", 1000, NULL, tskIDLE_PRIORITY + 2, NULL);	// higher priority
	xTaskCreate(vPushButton1, "", 1000, NULL, tskIDLE_PRIORITY + 3, NULL);	// higher priority
	xTaskCreate(vCounter, "", 1000, NULL, tskIDLE_PRIORITY, NULL);			// low priority
	
	/* Semaphore */
	xSemaphore = xSemaphoreCreateBinary();
	xSemaphoreGive(xSemaphore);
	
	/* Start the task */
	
	vTaskStartScheduler();
}

static portTASK_FUNCTION(vBlinkLed0, p_) {
	char strbuf[128];
	int flagLed0 = 1;
	
	while(1) {
		flagLed0 = !flagLed0;
		led0_cnt++;
		ioport_set_pin_level(LED0_GPIO, flagLed0);
		snprintf(strbuf, sizeof(strbuf), "LED 0 : %d ", led0_cnt);
		gfx_mono_draw_string(strbuf,0, 16, &sysfont);
		vTaskDelay(100/portTICK_PERIOD_MS);
	}
}

static portTASK_FUNCTION(vBlinkLed1, q_) {	
	char strbuf[128];
	int flagLed1 = 0;
	
	while(1) {
		flagLed1 = !flagLed1;
		led1_cnt++;
		ioport_set_pin_level(LED1_GPIO, flagLed1);
		snprintf(strbuf, sizeof(strbuf), "LED 1 : %d ", led1_cnt);
		gfx_mono_draw_string(strbuf,0, 24, &sysfont);
		vTaskDelay(200/portTICK_PERIOD_MS);
	}
}

static portTASK_FUNCTION(vPushButton1, s_) {
	char strbuf[128];
	
	while(1) {
		
		if(ioport_get_pin_level(GPIO_PUSH_BUTTON_1)==0){
			if(xSemaphoreTake(xSemaphore, (TickType_t) 10) == pdTRUE) {
				counter++;
				snprintf(strbuf, sizeof(strbuf), "Counter : %d ", counter);
				gfx_mono_draw_string(strbuf,0, 8, &sysfont);
				buzzerEvenOdd();
				xSemaphoreGive(xSemaphore);
			}
		} else if (ioport_get_pin_level(GPIO_PUSH_BUTTON_2)==0){
			if(xSemaphoreTake(xSemaphore, (TickType_t) 10) == pdTRUE) {
				counter--;
				snprintf(strbuf, sizeof(strbuf), "Counter : %d ", counter);
				gfx_mono_draw_string(strbuf,0, 8, &sysfont);
				buzzerEvenOdd();
				xSemaphoreGive(xSemaphore);
			}
		}
		
		vTaskDelay(10/portTICK_PERIOD_MS);
	}
}

static portTASK_FUNCTION(vCounter, r_) {
	char strbuf[128];
	
	while(1) {
		
		if(xSemaphoreTake(xSemaphore, (TickType_t) 10) == pdTRUE) {
			counter++;
			snprintf(strbuf, sizeof(strbuf), "Counter : %d ", counter);
			gfx_mono_draw_string(strbuf,0, 8, &sysfont);
			
			buzzerEvenOdd();
			
			xSemaphoreGive(xSemaphore);	
		}
		
		vTaskDelay(100/portTICK_PERIOD_MS);
	}	
}