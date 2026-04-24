#ifndef APP_H
#define APP_H

#define SILENT			0
#define NonSILENT       1
#define ALARM_OFF       0
#define ALARM_ON        1
#define NUM_INPUTS 		10	// Alarm[0..9]

#include "main.h"

typedef struct {	GPIO_TypeDef* port;
    				uint16_t      pin;
} GPIO_Pin_t;

static const GPIO_Pin_t RELAY      	= {GPIOA, GPIO_PIN_4};
static const GPIO_Pin_t LED_Y      	= {GPIOA, GPIO_PIN_7};
static const GPIO_Pin_t LED_R      	= {GPIOB, GPIO_PIN_3};
static const GPIO_Pin_t BUTTON     	= {GPIOA, GPIO_PIN_6};
static const GPIO_Pin_t TM1637_CLK  = {GPIOA, GPIO_PIN_0};
static const GPIO_Pin_t TM1637_DIO  = {GPIOA, GPIO_PIN_1};

static const GPIO_Pin_t inputs[10] = {
    {GPIOB, GPIO_PIN_6},    // Alarm[0]
    {GPIOB, GPIO_PIN_7},    // Alarm[1]
    {GPIOA, GPIO_PIN_15},   // Alarm[2]
    {GPIOB, GPIO_PIN_1},    // Alarm[3]
    {GPIOA, GPIO_PIN_10},   // Alarm[4]
    {GPIOA, GPIO_PIN_9},    // Alarm[5]
    {GPIOB, GPIO_PIN_0},    // Alarm[6]
    {GPIOB, GPIO_PIN_2},    // Alarm[7]
    {GPIOB, GPIO_PIN_8},    // Alarm[8]
    {GPIOA, GPIO_PIN_8}     // Alarm[9] +1 = NUM_INPUTS
};

void app_init(void);
void app_run(void);

#endif /* APP_H */
