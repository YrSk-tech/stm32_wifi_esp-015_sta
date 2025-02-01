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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define WIFI_SSID ""    // Your Wi-Fi SSID
#define WIFI_PSWD ""   // Your Wi-Fi Password
#define STATIC_IP ""   // Static IP Address
#define GATEWAY ""       // Router Gateway
#define SUBNET "255.255.255.0"       // Subnet Mask
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
uint8_t rxBuffer[512] = {0};
uint8_t ATisOK;
int channel;
int onoff;
int led = 1;
char ATcommand[128];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
// Функція для надсилання AT-команд
// Функція для надсилання AT-команд
void sendATCommand(const char *command) {
    HAL_UART_Transmit(&huart1, (uint8_t *)command, strlen(command), 1000);
    HAL_UART_Transmit(&huart1, (uint8_t *)"\r\n", 2, 100);
    HAL_UART_Receive(&huart1, rxBuffer, 512, 2000);
}

// Ініціалізація Wi-Fi
void wifi_init() {
    ATisOK = 0;
    int retries = 0;

    // 1. Скидання модуля ESP8266
    sendATCommand("AT+RST");
    HAL_Delay(5000);

    // 2. Встановлення режиму STA (Station Mode)
    while(!ATisOK && retries < 5) {
        sendATCommand("AT+CWMODE_CUR=1");  // STA mode
        if(strstr((char *)rxBuffer, "OK")) ATisOK = 1;
        retries++;
        HAL_Delay(500);
    }

    // 3. Вимкнення DHCP для статичної IP-адреси
    sendATCommand("AT+CWDHCP=1,0");

    // 4. Встановлення статичної IP-адреси
    sprintf(ATcommand, "AT+CIPSTA_CUR=\"%s\",\"%s\",\"%s\"", STATIC_IP, GATEWAY, SUBNET);
    sendATCommand(ATcommand);

    // 5. Підключення до Wi-Fi мережі
    ATisOK = 0;
    retries = 0;
    while(!ATisOK && retries < 5) {
        sprintf(ATcommand, "AT+CWJAP_CUR=\"%s\",\"%s\"", WIFI_SSID, WIFI_PSWD);
        sendATCommand(ATcommand);
        if(strstr((char *)rxBuffer, "WIFI CONNECTED") || strstr((char *)rxBuffer, "OK")) ATisOK = 1;
        retries++;
        HAL_Delay(2000);
    }

    // 6. Отримання IP-адреси
    sendATCommand("AT+CIFSR");

    // 7. Налаштування TCP-сервера
    sendATCommand("AT+CIPMUX=1");       // Дозволити кілька підключень
    sendATCommand("AT+CIPSERVER=1,80"); // Запуск веб-сервера на порту 80
}
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
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */

  wifi_init();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
      memset(rxBuffer,0,sizeof(rxBuffer));
      HAL_UART_Receive(&huart1, rxBuffer, 512, 1000);

      if(strstr((char *)rxBuffer,"+IPD,0")) channel = 0;
      else if(strstr((char *)rxBuffer,"+IPD,1")) channel = 1;
      else if(strstr((char *)rxBuffer,"+IPD,2")) channel = 2;
      else if(strstr((char *)rxBuffer,"+IPD,3")) channel = 3;
      else channel = 100;

      if(strstr((char *)rxBuffer,"GET /lighton")) onoff = 0;
      else if(strstr((char *)rxBuffer,"GET /lightoff")) onoff = 1;
      else onoff = led;

      if(channel < 8 && onoff == 1) {
          HAL_GPIO_WritePin(GPIO_LED_GPIO_Port, GPIO_LED_Pin, GPIO_PIN_SET);
          led = 1;
          sprintf(ATcommand,"AT+CIPSEND=%d,50\r\n",channel);
          sendATCommand(ATcommand);
          sendATCommand("<html><body>Light is ON</body></html>");
          sendATCommand("AT+CIPCLOSE=0");
      }
      else if(channel < 8 && onoff == 0) {
          HAL_GPIO_WritePin(GPIO_LED_GPIO_Port, GPIO_LED_Pin, GPIO_PIN_RESET);
          led = 0;
          sprintf(ATcommand,"AT+CIPSEND=%d,50\r\n",channel);
          sendATCommand(ATcommand);
          sendATCommand("<html><body>Light is OFF</body></html>");
          sendATCommand("AT+CIPCLOSE=0");
      }
	/* USER CODE END WHILE */
	}
    /* USER CODE BEGIN 3 */

  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIO_LED_GPIO_Port, GPIO_LED_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin : GPIO_LED_Pin */
  GPIO_InitStruct.Pin = GPIO_LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIO_LED_GPIO_Port, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

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
