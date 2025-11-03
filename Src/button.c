/*
 * button.c
 *
 *  Created on: Jun 11, 2024
 *      Author: JUMA9
 */

#include "button.h"

void ReadInitialButtonState(button_t * const button)
{
    button->lastGpioState = LL_GPIO_IsInputPinSet(button->GPIOx, button->GPIO_Pin);
    button->state = (button->lastGpioState != 0u) ? BTN_UP : BTN_DOWN;
    button->lastToggleTime = 0;
    button->lastEdgeTime   = 0;
}

void ReadButtonState(button_t * const button, uint32_t now)
{
    uint32_t rawState = LL_GPIO_IsInputPinSet(button->GPIOx, button->GPIO_Pin);
    if (button->lastGpioState != rawState)
    {
        button->lastGpioState = rawState;
        button->lastToggleTime = now;
        button->state = 0u == rawState ? BTN_DOWN_UNSTABLE : BTN_UP_UNSTABLE;
    }
    else if (0 != button->lastToggleTime)
    {
        if ((now - button->lastToggleTime) >= button->debounceTimeMs)
        {
            button->lastToggleTime = 0u;
            button->state = 0u == rawState ? BTN_FALLING : BTN_RISING;
            button->lastEdgeTime = now;
        }
        else
        {
            button->state = 0u == rawState ? BTN_DOWN_UNSTABLE : BTN_UP_UNSTABLE;
        }
    }
    else
    {
        button->state = 0u == rawState ? BTN_DOWN : BTN_UP;
    }
}

