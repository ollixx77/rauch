#include "app.h"
#include "tm1637.h"
#include <string.h>

extern TIM_HandleTypeDef htim3;

volatile uint8_t  alarm  = ALARM_OFF;
volatile uint8_t  status = NonSILENT;
volatile uint8_t  displayFlag = 1;

uint8_t  activeList[NUM_INPUTS];
uint8_t  activeCount = 0;
uint8_t  displayIndex = 0;
uint8_t  last_alarm_input[NUM_INPUTS];
uint8_t  alarm_input[NUM_INPUTS] = {0};	// Aktuell anliegende Alarme
uint8_t  alarm_latch[NUM_INPUTS] = {0}; // Speichert Alarme, wird nie zurückgesetzt

tm1637_t seg = {
	.gpio_clk = TM1637_CLK.port,  .pin_clk = TM1637_CLK.pin,	// TM1637 CLK
	.gpio_dat = TM1637_DIO.port,  .pin_dat = TM1637_DIO.pin,	// TM1637 DIO
	.seg_cnt  = 4,
};


uint8_t HasChanged(const uint8_t *a, const uint8_t *b)
{
    for (uint8_t i = 0; i < NUM_INPUTS; i++) {
        if (a[i] != b[i]) return 1;
    }
    return 0;
}


void RebuildList(const uint8_t *alarm_list)
{
    activeCount = 0;
    displayIndex = 0;

    for (uint8_t i = 0; i < NUM_INPUTS; i++) {
        if (alarm_list[i]) {
            activeList[activeCount++] = i;
        }
    }
}


void int_to_str(uint8_t x, char *buf) {
    buf[0] = (status == SILENT ? 'S' : 'E');
    buf[1] = ' ';
    buf[2] = ' ';
    buf[3] = '0' + (x % 10);	// x = 0..9
    buf[4] = '\0';
}


void ShowActiveList(void)
{
    if (activeCount == 0) {
    	tm1637_str(&seg, "----");
        return;
    }

    char str[5];
    int_to_str(activeList[displayIndex], str);
    tm1637_str(&seg, str);

    displayIndex++;

    if (displayIndex >= activeCount) {
        displayIndex = 0;
    }
}


uint8_t ButtonPressed(void)
{
    static uint8_t  lastState     = 1;
    static uint32_t debounceStart = 0;
    static uint8_t  debouncing    = 0;

    uint8_t currentState = HAL_GPIO_ReadPin(BUTTON.port, BUTTON.pin);

    if (lastState == 1 && currentState == 0) {
        debouncing    = 1;
        debounceStart = HAL_GetTick();
    }

    if (debouncing && (HAL_GetTick() - debounceStart > 50)) {
        debouncing = 0;
        if (HAL_GPIO_ReadPin(BUTTON.port, BUTTON.pin) == 0) {
            lastState = currentState;
            return 1;
        }
    }

    lastState = currentState;
    return 0;
}



void app_init(void)
{
    HAL_TIM_Base_Start_IT(&htim3);
    tm1637_init(&seg);
    tm1637_brightness(&seg, 3);
	tm1637_str(&seg, "8888");
	HAL_Delay(1000);
    const uint16_t demo = 50;
/*  for (uint16_t i = 0; i< 10; i++) {
        tm1637_str(&seg, "^   "); HAL_Delay(demo);
        tm1637_str(&seg, " ^  "); HAL_Delay(demo);
        tm1637_str(&seg, "  ^ "); HAL_Delay(demo);
        tm1637_str(&seg, "   ^"); HAL_Delay(demo);
        tm1637_str(&seg, "   ]"); HAL_Delay(demo);
        tm1637_str(&seg, "   _"); HAL_Delay(demo);
        tm1637_str(&seg, "  _ "); HAL_Delay(demo);
        tm1637_str(&seg, " _  "); HAL_Delay(demo);
        tm1637_str(&seg, "_   "); HAL_Delay(demo);
        tm1637_str(&seg, "[   "); HAL_Delay(demo);
    } */
    for (uint16_t i = 0; i< 3; i++) {
        tm1637_str(&seg, "-   "); HAL_Delay(demo);
        tm1637_str(&seg, " -  "); HAL_Delay(demo);
        tm1637_str(&seg, "  - "); HAL_Delay(demo);
        tm1637_str(&seg, "   -"); HAL_Delay(demo);
        tm1637_str(&seg, "  - "); HAL_Delay(demo);
        tm1637_str(&seg, " -  "); HAL_Delay(demo);
    }
    	tm1637_str(&seg, "-   "); HAL_Delay(demo);
    	tm1637_str(&seg, "--  "); HAL_Delay(demo);
    	tm1637_str(&seg, "--- "); HAL_Delay(demo);
}


void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM3)
    {
        HAL_GPIO_TogglePin(LD3_GPIO_Port, LD3_Pin);
        displayFlag = 1;
    }
}


void app_run(void)
{
	// Checking Melder-Inputs 0..9
	for (int i = 0; i < NUM_INPUTS; i++) {
	    alarm_input[i] = (HAL_GPIO_ReadPin(inputs[i].port, inputs[i].pin) == GPIO_PIN_RESET);
	    if (alarm_input[i]) { alarm = ALARM_ON; }
	    	else 			{ /* nothing: ALARM has to stay ON */ }
	}

	if (alarm == ALARM_ON) {
	    HAL_GPIO_WritePin(LED_R.port, LED_R.pin, GPIO_PIN_SET);	}	// red LED

	if (alarm == ALARM_ON && status == NonSILENT) {
		HAL_GPIO_WritePin(RELAY.port, RELAY.pin, GPIO_PIN_SET); }	// Relais ON
	else {
		HAL_GPIO_WritePin(RELAY.port, RELAY.pin, GPIO_PIN_RESET); }	// Relais OFF

	if (status == NonSILENT) {
		HAL_GPIO_WritePin(LED_Y.port, LED_Y.pin, GPIO_PIN_SET); }	// yellow LED ON
	else {
		HAL_GPIO_WritePin(LED_Y.port, LED_Y.pin, GPIO_PIN_RESET); }	// yellow LED OFF


	if (HasChanged(alarm_input, last_alarm_input)) {
	    uint8_t changed = 0;

	    for (uint8_t i = 0; i < NUM_INPUTS; i++) {
	        if (alarm_input[i] && !last_alarm_input[i]) {
	            alarm_latch[i] = 1;
	            changed = 1;
	        }
	    }
	    if (changed) { RebuildList(alarm_latch); }
	    memcpy(last_alarm_input, alarm_input, NUM_INPUTS);
	}

	if (displayFlag) { displayFlag = 0; ShowActiveList(); }

	if (ButtonPressed()) { status = (status == SILENT) ? NonSILENT : SILENT; }

}
