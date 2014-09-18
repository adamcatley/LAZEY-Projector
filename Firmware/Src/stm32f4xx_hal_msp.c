/**
  ******************************************************************************
  * @file    stm32f4xx_hal_msp_template.c
  * @author  MCD Application Team
  * @version V1.1.0
  * @date    19-June-2014
  * @brief   HAL MSP module.
  *          This file template is located in the HAL folder and should be copied 
  *          to the user folder.
  *         
  @verbatim
 ===============================================================================
                     ##### How to use this driver #####
 ===============================================================================
    [..]
    This file is generated automatically by MicroXplorer and eventually modified 
    by the user

  @endverbatim
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
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_msp.h"

/** @addtogroup STM32F4xx_HAL_Driver
  * @{
  */

/** @defgroup HAL_MSP
  * @brief HAL MSP module.
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/** @defgroup HAL_MSP_Private_Functions
  * @{
  */

  /**
  * @brief TIM MSP Initialization 
  *        This function configures the hardware resources used in the application: 
  *           - Peripheral's clock enable
  *           - Peripheral's GPIO Configuration  
  *           - NVIC configuration for TIM interrupt enable
  * @note User defined HAL function called from HAL_TIM_Base_Init()
  * @param htim: TIM handle pointer
  * @retval None
  */
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim)
{
  /*##-1- Enable peripherals and GPIO Clocks #################################*/
  /* TIMx Peripheral clock enable */
  TIMx_CLK_ENABLE();

  /*##-2- Configure the NVIC for TIMx #########################################*/
  /* Set the TIMx priority */
  HAL_NVIC_SetPriority(TIMx_IRQn, TIMx_NVIC_PRIORITY, 1);
  
  /* Enable the TIMx global Interrupt */
  HAL_NVIC_EnableIRQ(TIMx_IRQn);
}  

/**
  * @brief SPI MSP Initialization 
  *        This function configures the hardware resources used in this example: 
  *           - Peripheral's clock enable
  *           - Peripheral's GPIO Configuration  
  *           - NVIC configuration for SPI interrupt request enable
  * @param hspi: SPI handle pointer
  * @retval None
  */
void HAL_SPI_MspInit(SPI_HandleTypeDef *hspi)
{
	GPIO_InitTypeDef  GPIO_InitStruct;
	static DMA_HandleTypeDef hdma_tx;
	static DMA_HandleTypeDef hDACdma_rx;
	static DMA_HandleTypeDef hRPIdma_rx;
	
	if (hspi->Instance == DAC_SPI){						
		
		/* Enable GPIO, DMA2 and SPI clock */
		DAC_SPI_GPIO_CLK_ENABLE();
		DAC_SPI_CLK_ENABLE(); 
		DAC_SPI_DMA_CLK_ENABLE();
		
		/* SPI GPIO pin configuration  */
		GPIO_InitStruct.Pin       = DAC_SPI_SCK_PIN | DAC_SPI_MISO_PIN | DAC_SPI_MOSI_PIN; 
		GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull      = GPIO_PULLUP;
		GPIO_InitStruct.Speed     = GPIO_SPEED_FAST;
		GPIO_InitStruct.Alternate = DAC_SPI_GPIO_AF;
		
		HAL_GPIO_Init(DAC_SPI_GPIO_PORT, &GPIO_InitStruct);
				
		/* SPI NSS GPIO pin configuration  */
		GPIO_InitStruct.Pin = DAC_SPI_NSS_PIN;
		GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP; //Set as GPIO as NSS must be controlled by software
			
		HAL_GPIO_Init(DAC_SPI_GPIO_PORT, &GPIO_InitStruct);
		HAL_GPIO_WritePin(DAC_SPI_GPIO_PORT, DAC_SPI_NSS_PIN, GPIO_PIN_SET);
		
		/* Configure the DMA handler for Transmission process */
		hdma_tx.Instance                 = DAC_SPI_TX_DMA_STREAM;		
		hdma_tx.Init.Channel             = DAC_SPI_TX_DMA_CHANNEL;
		hdma_tx.Init.Direction           = DMA_MEMORY_TO_PERIPH;
		hdma_tx.Init.PeriphInc           = DMA_PINC_DISABLE;
		hdma_tx.Init.MemInc              = DMA_MINC_ENABLE;
		hdma_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
		hdma_tx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
		hdma_tx.Init.Mode                = DMA_NORMAL;
		hdma_tx.Init.Priority            = DMA_PRIORITY_HIGH;
		hdma_tx.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;         
		hdma_tx.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
		hdma_tx.Init.MemBurst            = DMA_MBURST_SINGLE;//Potential speed up/down by batching?
		hdma_tx.Init.PeriphBurst         = DMA_PBURST_SINGLE;		
		HAL_DMA_Init(&hdma_tx);   
		
		/* Associate the initialized DMA handle to the the SPI handle */
		__HAL_LINKDMA(hspi, hdmatx, hdma_tx);
			
		/* Configure the DMA handler for Transmission process */
		hDACdma_rx.Instance                 = DAC_SPI_RX_DMA_STREAM;		
		hDACdma_rx.Init.Channel             = DAC_SPI_RX_DMA_CHANNEL;
		hDACdma_rx.Init.Direction           = DMA_PERIPH_TO_MEMORY;
		hDACdma_rx.Init.PeriphInc           = DMA_PINC_DISABLE;
		hDACdma_rx.Init.MemInc              = DMA_MINC_ENABLE;
		hDACdma_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
		hDACdma_rx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
		hDACdma_rx.Init.Mode                = DMA_NORMAL;
		hDACdma_rx.Init.Priority            = DMA_PRIORITY_MEDIUM;
		hDACdma_rx.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;         
		hDACdma_rx.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
		hDACdma_rx.Init.MemBurst            = DMA_MBURST_SINGLE;
		hDACdma_rx.Init.PeriphBurst         = DMA_PBURST_SINGLE; 
		HAL_DMA_Init(&hDACdma_rx);
			
		/* Associate the initialized DMA handle to the the SPI handle */
		__HAL_LINKDMA(hspi, hdmarx, hDACdma_rx);
			
		/*##-4- Configure the NVIC for DMA #########################################*/ 
		/* NVIC configuration for DMA transfer complete interrupt (SPI1_TX) */
		HAL_NVIC_SetPriority(DAC_SPI_DMA_TX_IRQn, DAC_SPI_NVIC_PRIORITY, 1);
		HAL_NVIC_EnableIRQ(DAC_SPI_DMA_TX_IRQn);
			
		/* NVIC configuration for DMA transfer complete interrupt (SPI1_RX) */
		HAL_NVIC_SetPriority(DAC_SPI_DMA_RX_IRQn, DAC_SPI_NVIC_PRIORITY, 0);   
		HAL_NVIC_EnableIRQ(DAC_SPI_DMA_RX_IRQn);
		
	}	else if (hspi->Instance == RPI_SPI){
	
		/* Enable GPIO and SPI clock */
		RPI_SPI_GPIO_CLK_ENABLE();
		RPI_SPI_CLK_ENABLE(); 
		
		/* SPI GPIO pin configuration  */
		GPIO_InitStruct.Pin       = RPI_SPI_SCK_PIN | RPI_SPI_MOSI_PIN | RPI_SPI_MISO_PIN | RPI_SPI_NSS_PIN; 
		GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull      = GPIO_PULLUP;
		GPIO_InitStruct.Speed     = GPIO_SPEED_FAST;
		GPIO_InitStruct.Alternate = RPI_SPI_GPIO_AF;
		
		HAL_GPIO_Init(RPI_SPI_GPIO_PORT, &GPIO_InitStruct);

		/*##-3- Configure the NVIC for SPI #########################################*/
		/* NVIC for SPI */
		HAL_NVIC_SetPriority(RPI_SPI_IRQn, RPI_SPI_NVIC_PRIORITY, 1);
		HAL_NVIC_EnableIRQ(RPI_SPI_IRQn);		
		
		/* Configure the DMA handler for Transmission process */
		hRPIdma_rx.Instance                 = RPI_SPI_RX_DMA_STREAM;		
		hRPIdma_rx.Init.Channel             = RPI_SPI_RX_DMA_CHANNEL;
		hRPIdma_rx.Init.Direction           = DMA_PERIPH_TO_MEMORY;
		hRPIdma_rx.Init.PeriphInc           = DMA_PINC_DISABLE;
		hRPIdma_rx.Init.MemInc              = DMA_MINC_ENABLE;
		hRPIdma_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
		hRPIdma_rx.Init.MemDataAlignment    = DMA_MDATAALIGN_HALFWORD;
		hRPIdma_rx.Init.Mode                = DMA_NORMAL;
		hRPIdma_rx.Init.Priority            = DMA_PRIORITY_HIGH;
		hRPIdma_rx.Init.FIFOMode            = DMA_FIFOMODE_ENABLE; //16bytes FIFO buffer        
		hRPIdma_rx.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_HALFFULL; //With 16 bits only 1/2 (INC4) or full (INC4, INC8) allowed
		hRPIdma_rx.Init.MemBurst            = DMA_MBURST_INC4;
		hRPIdma_rx.Init.PeriphBurst         = DMA_PBURST_SINGLE; 
		HAL_DMA_Init(&hRPIdma_rx);
			
		/* Associate the initialized DMA handle to the the SPI handle */
		__HAL_LINKDMA(hspi, hdmarx, hRPIdma_rx);
			
		/*##-4- Configure the NVIC for DMA #########################################*/ 			
		/* NVIC configuration for DMA transfer complete interrupt (SPI4_RX) */
		HAL_NVIC_SetPriority(RPI_SPI_DMA_RX_IRQn, RPI_SPI_NVIC_PRIORITY, 0);   
		HAL_NVIC_EnableIRQ(RPI_SPI_DMA_RX_IRQn);
	}
}

/**
  * @brief SPI MSP De-Initialization 
  *        This function frees the hardware resources used in this example:
  *          - Disable the Peripheral's clock
  *          - Revert GPIO and NVIC configuration to their default state
  * @param hspi: SPI handle pointer
  * @retval None
  */
void HAL_SPI_MspDeInit(SPI_HandleTypeDef *hspi)
{	
	static DMA_HandleTypeDef hdma_tx;
	static DMA_HandleTypeDef hdma_rx;

	if (hspi->Instance == DAC_SPI){		
		
		/*##-1- Reset peripherals ##################################################*/
		DAC_SPI_FORCE_RESET();
		DAC_SPI_RELEASE_RESET();

		/*##-2- Disable peripherals and GPIO Clocks ################################*/
		/* Configure SPI SCK as alternate function  */
		HAL_GPIO_DeInit(DAC_SPI_GPIO_PORT, DAC_SPI_SCK_PIN);
		/* Configure SPI MISO as alternate function  */
		HAL_GPIO_DeInit(DAC_SPI_GPIO_PORT, DAC_SPI_MISO_PIN);
		/* Configure SPI MOSI as alternate function  */
		HAL_GPIO_DeInit(DAC_SPI_GPIO_PORT, DAC_SPI_MOSI_PIN);
		
		/*##-3- Disable the DMA Streams ############################################*/
		/* De-Initialize the DMA Stream associate to transmission process */
		HAL_DMA_DeInit(&hdma_tx); 
		/* De-Initialize the DMA Stream associate to reception process */
		HAL_DMA_DeInit(&hdma_rx);
		
		/*##-4- Disable the NVIC for DMA ###########################################*/
		HAL_NVIC_DisableIRQ(DAC_SPI_DMA_TX_IRQn);
		HAL_NVIC_DisableIRQ(DAC_SPI_DMA_RX_IRQn);
		
	}	else if (hspi->Instance == RPI_SPI){
		/*##-1- Reset peripherals ##################################################*/
		RPI_SPI_FORCE_RESET();
		RPI_SPI_RELEASE_RESET();

		/*##-2- Disable peripherals and GPIO Clocks ################################*/
		/* Configure SPI SCK as alternate function  */
		HAL_GPIO_DeInit(RPI_SPI_GPIO_PORT, RPI_SPI_SCK_PIN);
		/* Configure SPI MISO as alternate function  */
		HAL_GPIO_DeInit(RPI_SPI_GPIO_PORT, RPI_SPI_MISO_PIN);
		/* Configure SPI MOSI as alternate function  */
		HAL_GPIO_DeInit(RPI_SPI_GPIO_PORT, RPI_SPI_MOSI_PIN);
		
		/*##-3- Disable the NVIC for SPI ###########################################*/
		HAL_NVIC_DisableIRQ(RPI_SPI_IRQn);		
	}
}

/**
  * @brief UART MSP Initialization 
  *        This function configures the hardware resources used in this example: 
  *           - Peripheral's clock enable
  *           - Peripheral's GPIO Configuration  
  *           - NVIC configuration for UART interrupt request enable
  * @param huart: UART handle pointer
  * @retval None
  */
void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{  
  GPIO_InitTypeDef  GPIO_InitStruct;
  static DMA_HandleTypeDef hdma_tx;
  
  /*##-1- Enable peripherals and GPIO Clocks #################################*/
  /* Enable GPIO TX/RX * USART1 clock */
  RPI_UART_GPIO_CLK_ENABLE();
  RPI_UART_CLK_ENABLE(); 
	RPI_UART_DMA_CLK_ENABLE();
  
  /*##-2- Configure peripheral GPIO ##########################################*/  
  /* UART TX GPIO pin configuration  */
  GPIO_InitStruct.Pin       = RPI_UART_TX_PIN | RPI_UART_RX_PIN;
  GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull      = GPIO_NOPULL;
  GPIO_InitStruct.Speed     = GPIO_SPEED_FAST;
  GPIO_InitStruct.Alternate = RPI_UART_AF;  
  HAL_GPIO_Init(RPI_UART_PORT, &GPIO_InitStruct);
    
	/*##-3- Configure the DMA streams ##########################################*/
  /* Configure the DMA handler for Transmission process */
  hdma_tx.Instance                 = RPI_UART_TX_DMA_STREAM;  
  hdma_tx.Init.Channel             = RPI_UART_TX_DMA_CHANNEL;
  hdma_tx.Init.Direction           = DMA_MEMORY_TO_PERIPH;
  hdma_tx.Init.PeriphInc           = DMA_PINC_DISABLE;
  hdma_tx.Init.MemInc              = DMA_MINC_ENABLE;
  hdma_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
  hdma_tx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
  hdma_tx.Init.Mode                = DMA_NORMAL;
  hdma_tx.Init.Priority            = DMA_PRIORITY_LOW;
  hdma_tx.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
  hdma_tx.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
  hdma_tx.Init.MemBurst            = DMA_MBURST_INC4;
  hdma_tx.Init.PeriphBurst         = DMA_PBURST_INC4;  
  HAL_DMA_Init(&hdma_tx);   
  
  /* Associate the initialized DMA handle to the the UART handle */
  __HAL_LINKDMA(huart, hdmatx, hdma_tx);
    
    
  /*##-4- Configure the NVIC for DMA #########################################*/
  /* NVIC configuration for DMA transfer complete interrupt (USART1_TX) */
  HAL_NVIC_SetPriority(RPI_UART_DMA_TX_IRQn, 0, 1);
  HAL_NVIC_EnableIRQ(RPI_UART_DMA_TX_IRQn);
}

/**
  * @brief UART MSP De-Initialization 
  *        This function frees the hardware resources used in this example:
  *          - Disable the Peripheral's clock
  *          - Revert GPIO and NVIC configuration to their default state
  * @param huart: UART handle pointer
  * @retval None
  */
void HAL_UART_MspDeInit(UART_HandleTypeDef *huart)
{
  static DMA_HandleTypeDef hdma_tx;
	
  /*##-1- Reset peripherals ##################################################*/
  RPI_UART_FORCE_RESET();
  RPI_UART_RELEASE_RESET();

  /*##-2- Disable peripherals and GPIO Clocks #################################*/
  /* Configure UART Tx as alternate function  */
  HAL_GPIO_DeInit(RPI_UART_PORT, RPI_UART_TX_PIN);
  /* Configure UART Rx as alternate function  */
  HAL_GPIO_DeInit(RPI_UART_PORT, RPI_UART_RX_PIN);
	
	/*##-3- Disable the DMA Streams ############################################*/
  /* De-Initialize the DMA Stream associate to transmission process */
  HAL_DMA_DeInit(&hdma_tx); 
  
 /*##-4- Disable the NVIC for DMA ###########################################*/
  HAL_NVIC_DisableIRQ(RPI_UART_DMA_TX_IRQn);
}

/**
  * @brief  Initializes the Global MSP.
  * @param  None
  * @retval None
  */
void HAL_MspInit(void)
{
  /* NOTE : This function is generated automatically by MicroXplorer and eventually  
            modified by the user
   */ 
}

/**
  * @brief  DeInitializes the Global MSP.
  * @param  None  
  * @retval None
  */
void HAL_MspDeInit(void)
{
  /* NOTE : This function is generated automatically by MicroXplorer and eventually  
            modified by the user
   */
}

/**
  * @brief  Initializes the PPP MSP.
  * @param  None
  * @retval None
  */
void HAL_PPP_MspInit(void)
{
  /* NOTE : This function is generated automatically by MicroXplorer and eventually  
            modified by the user
   */ 
}

/**
  * @brief  DeInitializes the PPP MSP.
  * @param  None  
  * @retval None
  */
void HAL_PPP_MspDeInit(void)
{
  /* NOTE : This function is generated automatically by MicroXplorer and eventually  
            modified by the user
   */
}

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
