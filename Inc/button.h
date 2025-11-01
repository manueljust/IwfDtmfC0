/*
 * button.h
 *
 *  Created on: Jun 11, 2024
 *    Modified: Oct 25, 2025 -> replaced HAL with LL
 *      Author: JUMA9
 */

#ifndef INC_BUTTON_H_
#define INC_BUTTON_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32c0xx.h"
#include "stm32c0xx_ll_gpio.h"

typedef enum {
	BTN_DOWN_UNSTABLE,
	BTN_FALLING,
	BTN_DOWN,
	BTN_UP_UNSTABLE,
	BTN_RISING,
	BTN_UP,
} buttonState_t;

typedef struct {
	GPIO_TypeDef *GPIOx;
	uint32_t GPIO_Pin;
	uint32_t debounceTimeMs;
	uint32_t lastToggleTime;
	uint32_t lastEdgeTime;
	uint32_t lastGpioState;
	buttonState_t state;
} button_t;

void ReadInitialButtonState(button_t * const button);
void ReadButtonState(button_t * const button, uint32_t now);

#ifdef __cplusplus
}
#endif

#endif /* INC_BUTTON_H_ */
