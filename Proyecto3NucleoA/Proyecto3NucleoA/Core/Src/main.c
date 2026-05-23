/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "SevenSegmentLib.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
// Valores lógicos para PWM
#define TIM_ARR_VAL 100 // Tu período total es 100

// Valores calibrados (Prueba con estos)
#define PWM_HI (68)  // ~0.85 uS en alto (Aumenta el margen del 1 lógico)
#define PWM_LO (30)  // ~0.37 uS en alto (Asegura el 0 lógico)

// Umbral de sensores
#define THRESHOLD 80

// Número de Neopixeles por tira
#define MAX_LED 4

D7S mi_display;
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

I2C_HandleTypeDef hi2c1;

TIM_HandleTypeDef htim2;
DMA_HandleTypeDef hdma_tim2_ch1;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

// VARIABLES PARA I2C
// Variable para representar el estado del parqueo
// 0x00 (0b00000000) -> No hay carros estacionados
// 0x01 (0b00000001) -> Carro detectado en S1
// 0x02 (0b00000010) -> Carro detectado en S2
// 0x04 (0b00000100) -> Carro detectado en S3
// 0x08 (0b00001000) -> Carro detectado en S4
uint8_t state = 0; // Esta variable se transmite al ESP32 por el protocolo I2C.
uint8_t i2c_busy = 0;

// VARIABLES PARA NEOPIXEL
TIM_HandleTypeDef *pwm_timer;
uint32_t pwm_channel;
uint8_t LED_Data[MAX_LED][3];
uint32_t pwmData[(24 * MAX_LED) + 300];

// VARIABLES PARA ADC
uint8_t adc_data[4];


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_I2C1_Init(void);
static void MX_TIM2_Init(void);
static void MX_ADC1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void NeoPixel_Init(TIM_HandleTypeDef *htim, uint32_t channel) {
    pwm_timer = htim;
    pwm_channel = channel;
}

void NeoPixel_SetColor(uint8_t led_index, uint8_t r, uint8_t g, uint8_t b) {
    if(led_index < MAX_LED) {
        LED_Data[led_index][0] = g; // Orden GRB de los WS2812B
        LED_Data[led_index][1] = r;
        LED_Data[led_index][2] = b;
    }
}

void NeoPixel_Show(void) {
    uint32_t indx = 0;
    uint32_t color;

    for (int i = 0; i < MAX_LED; i++) {
        color = ((LED_Data[i][0] << 16) | (LED_Data[i][1] << 8) | (LED_Data[i][2]));

        for (int j = 23; j >= 0; j--) {
            if (color & (1 << j)) {
                pwmData[indx] = PWM_HI;  // ~64% alto (Lógico 1)
            } else {
                pwmData[indx] = PWM_LO;  // ~32% alto (Lógico 0)
            }
            indx++;
        }
    }

    for (int i = 0; i < 300; i++) {
        pwmData[indx] = 0; // Señal de RESET
        indx++;
    }

    HAL_TIM_PWM_Start_DMA(pwm_timer, pwm_channel, (uint32_t *)pwmData, indx);
}


// Función para enviar un número siempre con 3 caracteres (000-255)
void UART_SendInt8_Fixed(uint8_t num) {
    char buffer[4];
    buffer[0] = (num / 100) + '0';       // Centenas
    buffer[1] = ((num / 10) % 10) + '0'; // Decenas
    buffer[2] = (num % 10) + '0';        // Unidades
    buffer[3] = '\0';
    HAL_UART_Transmit(&huart2, (uint8_t *)buffer, 3, 100);
}

void UART_SendString(const char *str) {
    uint16_t len = 0;
    while (str[len] != '\0') len++;
    HAL_UART_Transmit(&huart2, (uint8_t *)str, len, 100);
}

// Función para enviar los 4 bits de estado como "0000"
void UART_SendBinary4Bit(uint8_t state) {
    char bin_str[5]; // 4 bits + terminador
    // Revisamos cada bit (del 3 al 0)
    bin_str[0] = (state & 0x08) ? '1' : '0'; // S4
    bin_str[1] = (state & 0x04) ? '1' : '0'; // S3
    bin_str[2] = (state & 0x02) ? '1' : '0'; // S2
    bin_str[3] = (state & 0x01) ? '1' : '0'; // S1
    bin_str[4] = '\0';
    HAL_UART_Transmit(&huart2, (uint8_t *)bin_str, 4, 100);
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
  MX_DMA_Init();
  MX_USART2_UART_Init();
  MX_I2C1_Init();
  MX_TIM2_Init();
  MX_ADC1_Init();
  /* USER CODE BEGIN 2 */
  HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adc_data, 4);

  // Inicializa los NeoPixels usando el Timer 2, Canal 1
  NeoPixel_Init(&htim2, TIM_CHANNEL_1);

  // =================================================================
    // MAPEO CALIBRADO: Secuencia Física (e - g - a - d - f - c - b)
    // =================================================================
    mi_display.port_a = GPIO_DSS_4_GPIO_Port; mi_display.pin_a = GPIO_DSS_4_Pin; // Paso 3 fue A
    mi_display.port_b = GPIO_DSS_2_GPIO_Port; mi_display.pin_b = GPIO_DSS_2_Pin; // Paso 7 fue B
    mi_display.port_c = GPIO_DSS_6_GPIO_Port; mi_display.pin_c = GPIO_DSS_6_Pin; // Paso 6 fue C
    mi_display.port_d = GPIO_DSS_5_GPIO_Port; mi_display.pin_d = GPIO_DSS_5_Pin; // Paso 4 fue D
    mi_display.port_e = GPIO_DSS_7_GPIO_Port; mi_display.pin_e = GPIO_DSS_7_Pin; // Paso 1 fue E
    mi_display.port_f = GPIO_DSS_3_GPIO_Port; mi_display.pin_f = GPIO_DSS_3_Pin; // Paso 5 fue F
    mi_display.port_g = GPIO_DSS_1_GPIO_Port; mi_display.pin_g = GPIO_DSS_1_Pin; // Paso 2 fue G
    // =================================================================
  //Display_TestSequence(&mi_display);

  // Apagar el display al inicio por precaución
  DisplayOff(&mi_display);

  HAL_I2C_EnableListen_IT(&hi2c1);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	// 1. Actualizar ADC
	HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adc_data, 4);
	HAL_Delay(5); // Tiempo para que el DMA trabaje

	// 2. Lógica de umbrales para I2C y Visualización
	state = 0;
	if(adc_data[0] > THRESHOLD) state |= 0x01; // S1
	if(adc_data[1] > THRESHOLD) state |= 0x02; // S2
	if(adc_data[2] > THRESHOLD) state |= 0x04; // S3
	if(adc_data[3] > THRESHOLD) state |= 0x08; // S4

	// === NUEVA LÓGICA PARA ACTUALIZAR LOS NEOPIXELS ===

	uint8_t espacios_ocupados = 0; // Variable para contar los carros

	for (int i = 0; i < MAX_LED; i++) {
		// Evaluamos si el bit 'i' de 'state' está encendido (1 = Ocupado)
		if (state & (1 << i)) {
			// Parqueo Libre -> Color Rojo (R=255, G=0, B=0)
			NeoPixel_SetColor(i, 0, 255, 0);
		} else {
			// Parqueo Ocupado -> Color Rojo (R=0, G=255, B=0)
			espacios_ocupados += 1;
			NeoPixel_SetColor(i, 255, 0, 0);
		}
	}

	// Detener el PWM antes de actualizar evita que el DMA lea datos corruptos
	HAL_TIM_PWM_Stop_DMA(pwm_timer, pwm_channel);
	NeoPixel_Show();

	// === LÓGICA DEL DISPLAY 7 SEGMENTOS ===
	uint8_t parqueos_disponibles = MAX_LED - espacios_ocupados;
	DisplayNumber(&mi_display, parqueos_disponibles);
	// ==================================================

	// 3. Preparar I2C para el Maestro (Modo Esclavo IT)
//	static uint32_t last_i2c_attempt = 0;
//
//	if (i2c_busy == 1) {
//	    if ((HAL_GetTick() - last_i2c_attempt) > 1000) { // Si pasan más de 1 segundo
//	        // ¡BUS BLOQUEADO! Resetear
//	        HAL_I2C_Abort_IT(&hi2c1);
//	        i2c_busy = 0;
//	    }
//	} else {
//	    // Si no está ocupado, intentamos transmitir
//	    i2c_busy = 1;
//	    last_i2c_attempt = HAL_GetTick();
//	    HAL_I2C_Slave_Transmit_IT(&hi2c1, &state, 1);
//	}

	// 4. Transmisión de la Tabla por UART
	UART_SendString("| CH0: ");
	UART_SendInt8_Fixed(adc_data[0]);
	UART_SendString(" | CH1: ");
	UART_SendInt8_Fixed(adc_data[1]);
	UART_SendString(" | CH2: ");
	UART_SendInt8_Fixed(adc_data[2]);
	UART_SendString(" | CH3: ");
	UART_SendInt8_Fixed(adc_data[3]);

	// Nueva Columna: Sensores (Binario)
	UART_SendString(" | SENSORS: ");
	UART_SendBinary4Bit(state);

	// Columna: Estado (Hexadecimal)
	UART_SendString(" | HEX: 0x");
	char hex_buf[3];
	const char *hex_map = "0123456789ABCDEF";
	hex_buf[0] = hex_map[(state >> 4) & 0x0F];
	hex_buf[1] = hex_map[state & 0x0F];
	hex_buf[2] = '\0';
	UART_SendString(hex_buf);

	UART_SendString(" |\r\n");

	HAL_Delay(200);
    /* USER CODE END WHILE */

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
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 320;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_8B;
  hadc1.Init.ScanConvMode = ENABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 4;
  hadc1.Init.DMAContinuousRequests = ENABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = 2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_4;
  sConfig.Rank = 3;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_8;
  sConfig.Rank = 4;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 64;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 0;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 100-1;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */
  HAL_TIM_MspPostInit(&htim2);

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA2_CLK_ENABLE();
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Stream5_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream5_IRQn);
  /* DMA2_Stream0_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);

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
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_DSS_5_Pin|GPIO_DSS_2_Pin|GPIO_DSS_4_Pin|GPIO_DSS_3_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_DSS_6_Pin|GPIO_DSS_7_Pin|GPIO_DSS_1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : GPIO_DSS_5_Pin GPIO_DSS_2_Pin GPIO_DSS_4_Pin GPIO_DSS_3_Pin */
  GPIO_InitStruct.Pin = GPIO_DSS_5_Pin|GPIO_DSS_2_Pin|GPIO_DSS_4_Pin|GPIO_DSS_3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : GPIO_DSS_6_Pin GPIO_DSS_7_Pin GPIO_DSS_1_Pin */
  GPIO_InitStruct.Pin = GPIO_DSS_6_Pin|GPIO_DSS_7_Pin|GPIO_DSS_1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
// 1. Este Callback se activa cuando el ESP32 (o el escáner) hace Match con la dirección de la STM32
void HAL_I2C_AddrCallback(I2C_HandleTypeDef *hi2c, uint8_t TransferDirection, uint16_t AddrMatchCode)
{
    if (hi2c->Instance == I2C1)
    {
        // Si el Maestro quiere LEER (el escáner o el ESP32 pidiendo datos)
        if (TransferDirection == I2C_DIRECTION_TRANSMIT)
        {
            // Le respondemos enviando de forma secuencial nuestra variable 'state'
            HAL_I2C_Slave_Sequential_Transmit_IT(hi2c, &state, 1, I2C_LAST_FRAME);
        }
        else
        {
            // Si el Maestro quisiera ESCRIBIRLE a la STM32 (no es tu caso actual, pero se deja listo)
            // Aquí se usaría HAL_I2C_Slave_Sequential_Receive_IT
        }
    }
}

// 2. Este Callback se activa cuando la transmisión de datos hacia el maestro termina con éxito
void HAL_I2C_SlaveTxCpltCallback(I2C_HandleTypeDef *hi2c) {
    // Transmisión completada con éxito
}

// 3. ¡EL MÁS IMPORTANTE! Se activa cuando el maestro manda un STOP y termina la comunicación completa
void HAL_I2C_ListenCpltCallback(I2C_HandleTypeDef *hi2c) {
    if (hi2c->Instance == I2C1) {
        // Volvemos a activar la escucha para la siguiente petición o el siguiente escaneo
        HAL_I2C_EnableListen_IT(hi2c);
    }
}

// 4. Este Callback atrapa los errores (como el NACK que provoca el escáner al no leer datos)
void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c) {
    if (hi2c->Instance == I2C1) {
        // Si el error fue simplemente un NACK (Acknowledge Failure), es normal con los escáneres
        if (HAL_I2C_GetError(hi2c) == HAL_I2C_ERROR_AF) {
            // Manejo silencioso del NACK
        } else {
            // Si es un error grave del bus, reiniciamos el periférico por seguridad
            HAL_I2C_DeInit(hi2c);
            HAL_I2C_Init(hi2c);
        }

        // Sin importar el error, volvemos a levantar las orejas del I2C para escuchar
        HAL_I2C_EnableListen_IT(hi2c);
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
#ifdef USE_FULL_ASSERT
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
