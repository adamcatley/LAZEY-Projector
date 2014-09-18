/**
  ******************************************************************************
  * @file    Templates/Src/main.c 
  * @author  MCD Application Team
  * @version V1.1.0
  * @date    26-June-2014
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2014 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "init.h"

/** @addtogroup Templates
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
	//GPIO
	GPIO_InitTypeDef GPIO_InitStructure;
	//FMC SRAM
	SRAM_HandleTypeDef  hsram;
	FMC_NORSRAM_InitTypeDef SRAM_Init;
	FMC_NORSRAM_TimingTypeDef SRAM_Timing;
	uint32_t SRAM_Counter = 0;
	//Timer
	TIM_HandleTypeDef    htim;
	//DAC
	SPI_HandleTypeDef  hDACspi;
	uint8_t DAC_SPI_Buffer[DAC_BUFFER_SIZE];
	//RPI
	SPI_HandleTypeDef  hRPIspi;
	UART_HandleTypeDef huart;
	uint8_t RPI_SPI_Buffer[RPI_SPI_BUFFER_SIZE];
	
	//Testing
	Point_TypeDef Watch1;
/* Private function prototypes -----------------------------------------------*/
static void SystemClock_Config(void);
static void GPIO_Init(void);
static void FMC_SRAM_Init(void);
static void TIM_Init(void);
static void DAC_Init(void);
static void RPI_Init(void);
static void RPI_UART_Init(void);
	


/* Private functions ---------------------------------------------------------*/
int Init(void){
	//System critical
	
	/* 
		STM32F4xx HAL library initialization:
       - Configure the Flash prefetch, Flash preread and Buffer caches
       - Systick timer is configured by default as source of time base, but user 
             can eventually implement his proper time base source (a general purpose 
             timer for example or other time source), keeping in mind that Time base 
             duration should be kept 1ms since PPP_TIMEOUT_VALUEs are defined and 
             handled in milliseconds basis.
       - Low Level Initialization
  */
  HAL_Init();

  /* Configure the System clock to have a frequency of 180 MHz */
  SystemClock_Config();
	
	//Application
	GPIO_Init(); //Do first for debugging
	HAL_GPIO_WritePin(STATUS_LED_PORT, STATUS_LED_PIN, GPIO_PIN_SET); //LED on during initialisation
	
	//Laser safety disable
	
	//The pin must be toggle high and then stay low before the system will start and enable the laser.
	//This ensures a human is present and has understood the laser will become dangerous by toggling the pin
	//This is to ensure the laser isn't active without knowledge of an operator
	while (HAL_GPIO_ReadPin(LASER_ENABLE_PORT, LASER_ENABLE_PIN) == GPIO_PIN_RESET){ //Wait for pint to go high (internal pullup)
		__ASM("nop");
	}
		while (HAL_GPIO_ReadPin(LASER_ENABLE_PORT, LASER_ENABLE_PIN) == GPIO_PIN_SET){ //Wait for pint to go low to begin
		__ASM("nop");
	}
	
	RPI_UART_Init();
	FMC_SRAM_Init();
	DAC_Init();
	RPI_Init(); //Can tirgger EXTI interupt once completed
	TIM_Init(); //Activate last (as triggers interrupts once enabled)	
	
	//TODO: Split Init functions into sub-functions
	Print("Controller Initialised, running...");
	return 0;
}

static void GPIO_Init(void){
	//Status LED
	STATUS_LED_CLK_ENABLE();
	GPIO_InitStructure.Pin = STATUS_LED_PIN; //Status LED on PB7
	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Speed = GPIO_SPEED_HIGH;
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(STATUS_LED_PORT, &GPIO_InitStructure);
	
	HAL_GPIO_WritePin(STATUS_LED_PORT,STATUS_LED_PIN, GPIO_PIN_RESET);
	
	//RPI GPIO here?
	
	//Debugging Pins
	DEBUG_IO_CLK_ENABLE();
	GPIO_InitStructure.Pin = DEBUG_IO_2 | DEBUG_IO_3;
	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Speed = GPIO_SPEED_HIGH;
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(DEBUG_IO_PORT, &GPIO_InitStructure);	
	
	
	GPIO_InitStructure.Pin = LASER_ENABLE_PIN;
	GPIO_InitStructure.Mode = GPIO_MODE_INPUT;
	GPIO_InitStructure.Speed = GPIO_SPEED_HIGH;
	GPIO_InitStructure.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(LASER_ENABLE_PORT, &GPIO_InitStructure);	
	
}

static void FMC_SRAM_Init(void){
	//FMC Pins Setup (39 Pins)
	
	//Common pin configuration
	GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStructure.Alternate = GPIO_AF12_FMC;
	GPIO_InitStructure.Speed = GPIO_SPEED_HIGH;//GPIO_Speed_100MHz
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	//Port D	
	__GPIOD_CLK_ENABLE();
	GPIO_InitStructure.Pin = GPIO_PIN_0  | GPIO_PIN_1  | GPIO_PIN_4 |
                           GPIO_PIN_5  | GPIO_PIN_8  | GPIO_PIN_9 |
                           GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 |
                           GPIO_PIN_14 | GPIO_PIN_15;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStructure);
	//Port E	
	__GPIOE_CLK_ENABLE();
	GPIO_InitStructure.Pin = GPIO_PIN_0  | GPIO_PIN_1  | GPIO_PIN_7 |
                           GPIO_PIN_8  | GPIO_PIN_9  | GPIO_PIN_10 |
                           GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 |
                           GPIO_PIN_14 | GPIO_PIN_15;
	HAL_GPIO_Init(GPIOE, &GPIO_InitStructure);
	//Port F	
	__GPIOF_CLK_ENABLE();
	GPIO_InitStructure.Pin = GPIO_PIN_0  | GPIO_PIN_1  | GPIO_PIN_2 |
                           GPIO_PIN_3  | GPIO_PIN_4  | GPIO_PIN_5 |
                           GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 |
                           GPIO_PIN_15;
	HAL_GPIO_Init(GPIOF, &GPIO_InitStructure);
	//Port G	
	__GPIOG_CLK_ENABLE();
	GPIO_InitStructure.Pin = GPIO_PIN_0  | GPIO_PIN_1 | GPIO_PIN_2 |
                           GPIO_PIN_3  | GPIO_PIN_4  | GPIO_PIN_5 |
                           GPIO_PIN_10;
	HAL_GPIO_Init(GPIOG, &GPIO_InitStructure);
	
	
	//FMC Setup
	
	__FMC_CLK_ENABLE();
	
	SRAM_Init.NSBank = FMC_NORSRAM_BANK3; //0x68000000
	SRAM_Init.DataAddressMux = FMC_DATA_ADDRESS_MUX_DISABLE;
	SRAM_Init.MemoryType = FMC_MEMORY_TYPE_SRAM;
	SRAM_Init.MemoryDataWidth = FMC_NORSRAM_MEM_BUS_WIDTH_16;
	SRAM_Init.BurstAccessMode = FMC_BURST_ACCESS_MODE_DISABLE;
	SRAM_Init.WaitSignalPolarity = FMC_WAIT_SIGNAL_POLARITY_LOW; //N/A
	SRAM_Init.WrapMode = FMC_WRAP_MODE_DISABLE; //N/A
	SRAM_Init.WaitSignalActive = FMC_WAIT_TIMING_BEFORE_WS; //N/A
	SRAM_Init.WriteOperation = FMC_WRITE_OPERATION_ENABLE;
	SRAM_Init.WaitSignal = FMC_WAIT_SIGNAL_DISABLE; //N/A
	SRAM_Init.ExtendedMode = FMC_EXTENDED_MODE_DISABLE; //N/A
	SRAM_Init.AsynchronousWait = FMC_ASYNCHRONOUS_WAIT_DISABLE; //N/A
	SRAM_Init.WriteBurst = FMC_WRITE_BURST_DISABLE;
	SRAM_Init.ContinuousClock = FMC_CONTINUOUS_CLOCK_SYNC_ONLY; //N/A
	hsram.Init = SRAM_Init;
	hsram.Instance = FMC_NORSRAM_DEVICE;
	
	SRAM_Timing.AddressSetupTime = 1; //0-15
	SRAM_Timing.AddressHoldTime = 0; //1-15
	SRAM_Timing.DataSetupTime = 2;//3; //1-255
	SRAM_Timing.BusTurnAroundDuration = 0; //N/A
	SRAM_Timing.CLKDivision = 0; //N/A
	SRAM_Timing.DataLatency = 0; //N/A
	SRAM_Timing.AccessMode = FMC_ACCESS_MODE_A; //Page 1598 ref man
	
	if(HAL_SRAM_Init(&hsram, &SRAM_Timing, &SRAM_Timing) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler(); 
  }
	HAL_SRAM_WriteOperation_Enable(&hsram);
	
	//FMC SRAM Memory Test
	
	/* Write data value to all SRAM memory */
	for (SRAM_Counter = 0; SRAM_Counter < FMC_SRAM_SIZE; SRAM_Counter++)
	{
		*(__IO uint16_t*) (FMC_SRAM_ADDR + 2*SRAM_Counter) = (uint16_t)(0x1E5A + SRAM_Counter);
	}
	
	/* Read back SRAM memory and check content correctness*/
	SRAM_Counter = 0;
	while ((SRAM_Counter < FMC_SRAM_SIZE)) //Takes ~25ms
	{
		if ( *(__IO uint16_t*)(FMC_SRAM_ADDR + 2*SRAM_Counter) != (uint16_t)(0x1E5A + SRAM_Counter))
		{
			Error_Handler();
		}
		SRAM_Counter++;
	}
	
	//Erase SRAM memory
	for (SRAM_Counter = 0; SRAM_Counter < FMC_SRAM_SIZE; SRAM_Counter++) //Takes ~17.5ms CPU time, could use DMA2 memory-memory
	{
		*(__IO uint16_t*) (FMC_SRAM_ADDR + 2*SRAM_Counter) = (uint16_t)0x00;
	}
}

static void TIM_Init(void){
	//Timer 3 used for updating next point
	htim.Instance = TIMx;
	htim.Init.Period = 1;
  htim.Init.Prescaler = (uint32_t) ((SystemCoreClock / 4) / (POINTS_PER_SEC)) - 1; //1499 for 30k
  htim.Init.ClockDivision = 0;
  htim.Init.CounterMode = TIM_COUNTERMODE_UP;
  if(HAL_TIM_Base_Init(&htim) != HAL_OK) //Also sets clock for TIM
  {
    /* Initialization Error */
    Error_Handler();
  }
	/* Start Channel1 */
  if(HAL_TIM_Base_Start_IT(&htim) != HAL_OK)
  {
    /* Starting Error */
    Error_Handler();
  }
}

static void DAC_Init(void){
	uint16_t DACSetupData = 0x0300 | DAC_DAC3_GAIN_2 | DAC_DAC2_GAIN_2 | DAC_DAC1_GAIN_4 | DAC_DAC0_GAIN_4;// 0b0000001100001100, 0x030C //Only changes DAC2,3 gain to x2
	
	//Control pins setup
	DAC_CONTROL_CLK_ENABLE();
	GPIO_InitStructure.Pin = DAC_LDAC_PIN | DAC_RST_PIN;
	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Speed = GPIO_SPEED_HIGH;
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(DAC_CONTROL_PORT, &GPIO_InitStructure);
	//WARNING - Enabling DAC brings the laser online - WARNING
	HAL_GPIO_WritePin(DAC_CONTROL_PORT, DAC_RST_PIN | DAC_LDAC_PIN, GPIO_PIN_SET); //De-assert both control lines to enable DAC
	//WARNING - Enabling DAC brings the laser online - WARNING
	
	/* Set the SPI parameters */
  hDACspi.Instance               = DAC_SPI;
  hDACspi.Init.Mode							 = SPI_MODE_MASTER;
  hDACspi.Init.Direction         = SPI_DIRECTION_2LINES;
  hDACspi.Init.DataSize          = SPI_DATASIZE_8BIT; //DAC has 24bit shift register so transmit 3x8bit commands
  hDACspi.Init.CLKPolarity       = SPI_POLARITY_HIGH; //CPOL=1
  hDACspi.Init.CLKPhase          = SPI_PHASE_1EDGE; //CPHA = 0
  hDACspi.Init.NSS               = SPI_NSS_SOFT;
  hDACspi.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4; //90MHz / prescaler. use 4 = 22.5MHz? Overrun error without DMA
  hDACspi.Init.FirstBit          = SPI_FIRSTBIT_MSB;
  hDACspi.Init.TIMode            = SPI_TIMODE_DISABLED;  
  hDACspi.Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLED;
  hDACspi.Init.CRCPolynomial     = 7;
	
	if(HAL_SPI_Init(&hDACspi) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler();
  }
	
	//Test communication with DAC (register is received on next transmission)
	while(((DAC_SPI_Buffer[1] << 8) | DAC_SPI_Buffer[2]) != DAC_DEFAULT_DATA){
		DAC_SendReceive(&hDACspi, DAC_READ, DAC_COMMAND_ADDR, 0x0000, (uint8_t *)DAC_SPI_Buffer);
	}
	
	//Configure DAC for oepration
	DAC_SendReceive(&hDACspi, DAC_WRITE, DAC_COMMAND_ADDR, DACSetupData, (uint8_t *)DAC_SPI_Buffer);
	
	//Read back control register to check configuration worked (DAC returns updated register on next transmission)
	do {
		DAC_SendReceive(&hDACspi, DAC_READ, DAC_COMMAND_ADDR, 0x0000, (uint8_t *)DAC_SPI_Buffer);
	} while(((DAC_SPI_Buffer[1] << 8) | DAC_SPI_Buffer[2]) != DACSetupData);
	
	//Set zero and gain registers here:
	//TODO: Calibrate with accurate measurment device
}

static void RPI_Init(void){
	//GPIO pins setup
	RPI_GPIO_CLK_ENABLE();
	
	//Outputs	
	GPIO_InitStructure.Pin = RPI_GPIO2_PIN | RPI_GPIO3_PIN | RPI_GPIO4_PIN | RPI_GPIO5_PIN;
	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Speed = GPIO_SPEED_HIGH;
	HAL_GPIO_Init(RPI_GPIO_PORT, &GPIO_InitStructure);
		
	//Inputs
	GPIO_InitStructure.Pin = RPI_GPIO_FRAME_PIN;
	GPIO_InitStructure.Mode = GPIO_MODE_IT_RISING_FALLING;
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(RPI_GPIO_PORT, &GPIO_InitStructure);
	
	//New Frame pin interrupt
  /* Enable and set EXTI Line0 Interrupt */
  HAL_NVIC_SetPriority(RPI_GPIO_FRAME_IRQn, RPI_FRAME_NVIC_PRIORITY, 0);
  HAL_NVIC_EnableIRQ(RPI_GPIO_FRAME_IRQn);
	
	/* Set the SPI parameters */
  hRPIspi.Instance               = RPI_SPI;
  hRPIspi.Init.Mode							 = SPI_MODE_SLAVE ;
  hRPIspi.Init.Direction         = SPI_DIRECTION_2LINES;
  hRPIspi.Init.DataSize          = SPI_DATASIZE_16BIT; //RPI has 8 bit shift register, but 16bit (DMA) operations on controller should be faster
  hRPIspi.Init.CLKPolarity       = SPI_POLARITY_LOW; //CPOL=0
  hRPIspi.Init.CLKPhase          = SPI_PHASE_1EDGE; //CPHA=0
  hRPIspi.Init.NSS               = SPI_NSS_HARD_INPUT;
  hRPIspi.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16; //N/A as slave. 31MHz from RPI
  hRPIspi.Init.FirstBit          = SPI_FIRSTBIT_MSB;
  hRPIspi.Init.TIMode            = SPI_TIMODE_DISABLED;  
  hRPIspi.Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLED;
  hRPIspi.Init.CRCPolynomial     = 7;
	
	if(HAL_SPI_Init(&hRPIspi) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler();
  }		
	
}

void RPI_UART_Init(void){
	//UART to RPI
	/* Put the USART peripheral in the Asynchronous mode (UART Mode) */
  /* UART1 configured as follow:
      - Word Length = 8 Bits
      - Stop Bit = One Stop bit
      - Parity = None
      - BaudRate = 115200 baud
      - Hardware flow control disabled (RTS and CTS signals) */
  huart.Instance        = USART1;
  huart.Init.BaudRate   = RPI_UART_BAUD_RATE;
  huart.Init.WordLength = UART_WORDLENGTH_8B;
  huart.Init.StopBits   = UART_STOPBITS_1;
  huart.Init.Parity     = UART_PARITY_NONE;
  huart.Init.HwFlowCtl  = UART_HWCONTROL_NONE;
  huart.Init.Mode       = UART_MODE_TX;
  
  if(HAL_UART_Init(&huart) != HAL_OK)
  {
    Error_Handler();
  }
	Print("\x1b[2J\x1b[H\x1b[32mInitialising..."); //Clear terminal, set colour to laser green
}


/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow : 
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 180000000
  *            HCLK(Hz)                       = 180000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 4
  *            APB2 Prescaler                 = 2
  *            HSE Frequency(Hz)              = 8000000
  *            PLL_M                          = 8
  *            PLL_N                          = 360
  *            PLL_P                          = 2
  *            PLL_Q                          = 7
  *            VDD(V)                         = 3.3
  *            Main regulator output voltage  = Scale1 mode
  *            Flash Latency(WS)              = 5
	*						 (8MHz/8 *360 /2 = 180Mhz
  * @param  None
  * @retval None
  */
static void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;

  /* Enable Power Control clock */
  __PWR_CLK_ENABLE();
  
  /* The voltage scaling allows optimizing the power consumption when the device is 
     clocked below the maximum system frequency, to update the voltage scaling value 
     regarding system frequency refer to product datasheet.  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  
  /* Enable HSE Oscillator and activate PLL with HSE as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 360;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler();
  }
  
  if(HAL_PWREx_ActivateOverDrive() != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler();
  }
  
  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 
     clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;  
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;  
  if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler();
  }
}


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
