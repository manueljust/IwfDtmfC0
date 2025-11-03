/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "button.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

static button_t nsi_s = { .GPIOx = NSI_GPIO_Port, .GPIO_Pin = NSI_Pin, .debounceTimeMs = 5u };
static button_t gnd_s = { .GPIOx = BUTTON_GND_GPIO_Port, .GPIO_Pin = BUTTON_GND_Pin, .debounceTimeMs = 5u };
static button_t * const nsi = &nsi_s;
static button_t * const gnd = &gnd_s;

static volatile uint8_t pulseCount, earthPressed = 0;
static volatile uint32_t ph1, ph2, dph1, dph2, samplesLeft, nowMs = 0;

static const uint8_t sine[256] = {
  0,  0,  0,  0,  0,  1,  1,  1,  1,  2,  2,  3,  3,  4,  4,  5,
  6,  6,  7,  8,  9,  9, 10, 11, 12, 13, 14, 15, 17, 18, 19, 20,
 21, 23, 24, 25, 27, 28, 30, 31, 32, 34, 35, 37, 39, 40, 42, 43,
 45, 47, 48, 50, 52, 54, 55, 57, 59, 61, 62, 64, 66, 68, 69, 71,
 73, 75, 77, 78, 80, 82, 84, 85, 87, 89, 91, 92, 94, 96, 98, 99,
101,103,104,106,107,109,111,112,114,115,116,118,119,121,122,123,
125,126,127,128,129,131,132,133,134,135,136,137,137,138,139,140,
140,141,142,142,143,143,144,144,145,145,145,145,146,146,146,146,
146,146,146,146,146,145,145,145,145,144,144,143,143,142,142,141,
140,140,139,138,137,137,136,135,134,133,132,131,129,128,127,126,
125,123,122,121,119,118,116,115,114,112,111,109,107,106,104,103,
101, 99, 98, 96, 94, 92, 91, 89, 87, 85, 84, 82, 80, 78, 77, 75,
 73, 71, 69, 68, 66, 64, 62, 61, 59, 57, 55, 54, 52, 50, 48, 47,
 45, 43, 42, 40, 39, 37, 35, 34, 32, 31, 30, 28, 27, 25, 24, 23,
 21, 20, 19, 18, 17, 15, 14, 13, 12, 11, 10,  9,  9,  8,  7,  6,
  6,  5,  4,  4,  3,  3,  2,  2,  1,  1,  1,  1,  0,  0,  0,  0,
};

static const uint32_t rowIncrements[4] = {
    INCREMENT(697),
    INCREMENT(770),
    INCREMENT(862),
    INCREMENT(941),
};
static const uint32_t columnIncrements[4] = {
    INCREMENT(1209),
    INCREMENT(1336),
    INCREMENT(1477),
    INCREMENT(1633),
};

static const uint8_t numbers[10] = { DIGIT_1, DIGIT_2, DIGIT_3, DIGIT_4, DIGIT_5, DIGIT_6, DIGIT_7, DIGIT_8, DIGIT_9, DIGIT_0 };/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_NVIC_Init(void);
/* USER CODE BEGIN PFP */
void PlaySines(uint8_t digit, uint32_t time);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SYSCFG);
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);

  /* SysTick_IRQn interrupt configuration */
  NVIC_SetPriority(SysTick_IRQn, 3);

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM1_Init();
  MX_TIM3_Init();

  /* Initialize interrupts */
  MX_NVIC_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
      ReadButtonState(nsi, nowMs);
      ReadButtonState(gnd, nowMs);

      switch (nsi->state)
      {
          case BTN_RISING: // rising is when nsi contact is being opened
              if (DIAL_IDLE == pulseCount)
              {
                  pulseCount = 0;
              }
              break;
          case BTN_FALLING: // falling is when nsi contact is being closed
              pulseCount++;
              // should not happen... but guard anyways
              if (10 > pulseCount)
              {
                  pulseCount = 10;
              }
              break;
          case BTN_DOWN: // down means nsi is closed
              if (DIAL_IDLE != pulseCount && (nowMs - nsi->lastEdgeTime) > INTER_DIGIT_PAUSE)
              {
                  PlaySines(numbers[pulseCount], TONE_TIME);
                  pulseCount = DIAL_IDLE;
              }
              break;
          default:
              // nothing to do
              break;
      }

      switch (gnd->state)
      {
          case BTN_DOWN:
              // long press -> dial *
              if (EARTH_PAUSE < (nowMs - gnd->lastEdgeTime) && 0 == earthPressed)
              {
                  earthPressed = 1;
                  PlaySines(DIGIT_HASH, TONE_TIME);
              }
              break;
          case BTN_RISING:
              // short press -> dial #
              if (EARTH_PAUSE > (nowMs - gnd->lastEdgeTime) && 0 == earthPressed)
              {
                    PlaySines(DIGIT_STAR, TONE_TIME);
              }
              earthPressed = 0;
          default:
              // nothing to do
              break;
      }    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{

  LL_FLASH_SetLatency(LL_FLASH_LATENCY_1);

  /* HSI configuration and activation */
  LL_RCC_HSI_Enable();
  while(LL_RCC_HSI_IsReady() != 1)
  {
  }

  LL_RCC_HSI_SetCalibTrimming(64);
  LL_RCC_SetHSIDiv(LL_RCC_HSI_DIV_1);
  /* Set AHB prescaler*/
  LL_RCC_SetAHBPrescaler(LL_RCC_HCLK_DIV_1);

  /* Sysclk activation on the HSI */
  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_HSI);
  while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_HSI)
  {
  }

  /* Set APB1 prescaler*/
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
  LL_Init1msTick(CORE_FREQ);
  /* Update CMSIS variable (which can be updated also through SystemCoreClockUpdate function) */
  LL_SetSystemCoreClock(CORE_FREQ);
}

/**
  * @brief NVIC Configuration.
  * @retval None
  */
static void MX_NVIC_Init(void)
{
  /* TIM3_IRQn interrupt configuration */
  NVIC_SetPriority(TIM3_IRQn, 0);
  NVIC_EnableIRQ(TIM3_IRQn);
}

/* USER CODE BEGIN 4 */
void OnSystick(void)
{
    nowMs++;
}

void OnTim3(void)
{
    if (samplesLeft == 0u)
    {
        LL_TIM_DisableCounter(TIM3);
        LL_TIM_DisableCounter(TIM1);
    }
    else
    {
        samplesLeft--;

        // accumulate phases
        ph1 += dph1;
        ph2 += dph2;

        uint8_t a1 = sine[ph1 >> 24];
        uint8_t a2 = sine[ph2 >> 24];
        uint8_t s = a1 + (a2 - ((a2 + 2) >> 2)); // +2 ensures we never exceed 255

        LL_TIM_OC_SetCompareCH4(TIM1, s);
    }
}

void PlaySines(uint8_t digit, uint32_t duration)
{
    if (0 == samplesLeft)
    {
        samplesLeft = CORE_FREQ * duration / 1000;

        ph1 = ph2 = 0;
        dph1 = rowIncrements[digit >> 4];
        dph2 = columnIncrements[digit & 0x0f];

        // enable pwm out (tim3 and tim1)
        LL_TIM_EnableCounter(TIM3);
        LL_TIM_EnableCounter(TIM1);
    }
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
