/**
  ******************************************************************************
  * @file    Templates/Inc/main.h 
  * @author  MCD Application Team
  * @version V1.1.0
  * @date    26-June-2014
  * @brief   Header for main.c module
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
  
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Exported types ------------------------------------------------------------*/
/** 
  * @brief  Point Structure definition
	*/
typedef struct __Point_TypeDef
{
  uint16_t               X;
	uint16_t               Y;
	uint8_t                Brightness;
	uint8_t                _Reserved;

}Point_TypeDef;

typedef enum
{
  BUFFER_0 = 0x00,
  BUFFER_1 = 0x01
    
}BUFFER_SELECT;

typedef enum
{
	BUFFER_IDLE, //Initial value
  BUFFER_RECEIVING,
  BUFFER_RECEIVED,
  BUFFER_DRAWING,
  BUFFER_DRAWN //Used to mark frame as finished with and can be overwritten
    
}BUFFER_STATE;

typedef struct __Buffer_TypeDef
{
  uint16_t               length; //Number of points in frame
	uint16_t               current_point; //Current point index being sent/receved
	BUFFER_STATE           state;	//Whether frame is being sent or received
	uint8_t                drawn; //Whether current frame has been drawn at least once
	uint32_t							 address;

}Buffer_TypeDef;

typedef enum
{
  PULSE_LOW,
  PULSE_HIGH 
    
}PULSE_DIRECTION;

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/

	//System
//	#define DEBUG_ONLY 							//Used to disable code that expects external devices present which isn't during development (receiving/writing points)
	#define POINTS_PER_SEC 					30000 //30000
	#define POINT_SIZE_16						3 //16 bit
	#define POINT_SIZE_8						POINT_SIZE_16 * 2 //8 bit
	#define COMMAND_RESET						0xAA //Possible command implementation using reserved byte in point data

	//Interrupt pirorities (Grouping 4 used, 4 bit pre-empt, 0 sub priority)
	#define TIMx_NVIC_PRIORITY 			1
	#define RPI_FRAME_NVIC_PRIORITY 3
	#define DAC_SPI_NVIC_PRIORITY 	0
	#define RPI_SPI_NVIC_PRIORITY 	2
	#define RPI_UART_NVIC_PRIORITY 	4
	
	//GPIO & debugging pins (PH13,14,15, SPI2)
	#define STATUS_LED_CLK_ENABLE() __GPIOB_CLK_ENABLE()
	#define STATUS_LED_PIN 					GPIO_PIN_7
	#define STATUS_LED_PORT 				GPIOB
	
	#define LASER_ENABLE_PIN				DEBUG_IO_1
	#define LASER_ENABLE_PORT				DEBUG_IO_PORT

	#define DEBUG_IO_CLK_ENABLE() 	__GPIOH_CLK_ENABLE()
	#define DEBUG_IO_1 							GPIO_PIN_13
	#define DEBUG_IO_2 							GPIO_PIN_14
	#define DEBUG_IO_3 							GPIO_PIN_15
	#define DEBUG_IO_PORT 					GPIOH

	//FMC SRAM
	#define FMC_SRAM_ADDR						((uint32_t)0x68000000) //FMC Bank 1, SRAM Bank 3 (NE3)
	#define FMC_SRAM_SIZE						((uint32_t)0x00080000) //Each address 8 bit. 0x68000000-0x68080000, wrap around each FMC_SRAM_SIZE. 70% used with 30k points
	#define FMC_SRAM_BANK_SIZE 			FMC_SRAM_SIZE / 2 //0x00040000 Not actual bank, only used in software for buffer memory

	
	//Timer
	/* User can use this section to tailor TIMx instance used and associated 
		 resources */
	/* Definition for TIMx clock resources */
	#define TIMx                           TIM3
	#define TIMx_CLK_ENABLE                __TIM3_CLK_ENABLE

	/* Definition for TIMx's NVIC */
	#define TIMx_IRQn                      TIM3_IRQn
	#define TIMx_IRQHandler                TIM3_IRQHandler
	
	//DAC
	/* Definition for DAC_SPI clock resources */
	#define DAC_SPI                             SPI4
	#define DAC_SPI_CLK_ENABLE()                __SPI4_CLK_ENABLE()
	#define DAC_SPI_DMA_CLK_ENABLE()            __DMA2_CLK_ENABLE()
	#define DAC_SPI_GPIO_CLK_ENABLE()      			__GPIOE_CLK_ENABLE()
	#define DAC_SPI_FORCE_RESET()               __SPI4_FORCE_RESET()
	#define DAC_SPI_RELEASE_RESET()             __SPI4_RELEASE_RESET()

	/* Definition for DAC_SPI Pins */
	#define DAC_SPI_SCK_PIN                     GPIO_PIN_2
	#define DAC_SPI_NSS_PIN                     GPIO_PIN_4
	#define DAC_SPI_MISO_PIN                    GPIO_PIN_5
	#define DAC_SPI_MOSI_PIN                    GPIO_PIN_6
	#define DAC_SPI_GPIO_PORT        			      GPIOE
	#define DAC_SPI_GPIO_AF                     GPIO_AF5_SPI4
	
	/* Definition for DAC control Pins */	
	#define DAC_CONTROL_CLK_ENABLE()      			__GPIOA_CLK_ENABLE()
	#define DAC_RST_PIN 												GPIO_PIN_11
	#define DAC_LDAC_PIN 												GPIO_PIN_12
	#define DAC_LDAC_PORT 											DAC_CONTROL_PORT
	#define DAC_RST_PORT 												DAC_CONTROL_PORT
	#define DAC_CONTROL_PORT 										GPIOA
	
	/* Definition for DAC_SPI's NVIC */
	#define DAC_SPI_DMA_TX_IRQn                 DMA2_Stream1_IRQn
	#define DAC_SPI_DMA_RX_IRQn                 DMA2_Stream0_IRQn
	#define DAC_SPI_DMA_TX_IRQHandler           DMA2_Stream1_IRQHandler
	#define DAC_SPI_DMA_RX_IRQHandler           DMA2_Stream0_IRQHandler
	
	/* Definition for DAC_SPIx's DMA */
	#define DAC_SPI_TX_DMA_CHANNEL              DMA_CHANNEL_4
	#define DAC_SPI_TX_DMA_STREAM               DMA2_Stream1
	#define DAC_SPI_RX_DMA_CHANNEL              DMA_CHANNEL_4
	#define DAC_SPI_RX_DMA_STREAM               DMA2_Stream0
	
typedef enum
{
  DAC_WRITE     	 = 0x00,
  DAC_READ	    	 = 0x80
    
}DAC_WRITE_MODE;

typedef enum
{
	DAC_COMMAND_ADDR	= 0x0,
	DAC_DAC0_ADDR			= 0x4,
	DAC_DAC1_ADDR			= 0x5,
	DAC_DAC2_ADDR			= 0x6,
  DAC_DAC3_ADDR			= 0x7, 
  DAC_X_ADDR				= DAC_DAC0_ADDR,
	DAC_Y_ADDR				= DAC_DAC1_ADDR,  
	DAC_BRI_ADDR			= DAC_DAC2_ADDR  
	
}DAC_ADDRESS;
	
	#define DAC_DAC0_GAIN_2											0x00 //0b000
	#define DAC_DAC0_GAIN_4											0x04 //0b100
	#define DAC_DAC1_GAIN_2											0x00 //0b0000
	#define DAC_DAC1_GAIN_4											0x08 //0b1000
	#define DAC_DAC2_GAIN_2											0x00 //0b00000
	#define DAC_DAC2_GAIN_4											0x10 //0b10000
	#define DAC_DAC3_GAIN_2											0x00 //0b000000
	#define DAC_DAC3_GAIN_4											0x20 //0b100000
										
	#define DAC_DEFAULT_DATA										0x033C
	#define DAC_BUFFER_SIZE											3 //8 bit
										
	#define DAC_UNIPOLAR_MAX										0xFFFF
	#define DAC_UNIPOLAR_MIN										0x0000
	#define DAC_BIPOLAR_MAX											0x7FFF
	#define DAC_BIPOLAR_MIN											0x8000
	
	//RPI
	/* Definition for RPI_SPI clock resources */
	#define RPI_SPI                             SPI1
	#define RPI_SPI_CLK_ENABLE()                __SPI1_CLK_ENABLE()
	#define RPI_SPI_GPIO_CLK_ENABLE()      			__GPIOA_CLK_ENABLE()
	#define RPI_SPI_DMA_CLK_ENABLE()            __DMA2_CLK_ENABLE()
	#define RPI_SPI_FORCE_RESET()               __SPI1_FORCE_RESET()
	#define RPI_SPI_RELEASE_RESET()             __SPI1_RELEASE_RESET()

	/* Definition for RPI_SPI Pins */
	#define RPI_SPI_NSS_PIN                     GPIO_PIN_4
	#define RPI_SPI_SCK_PIN                     GPIO_PIN_5
	#define RPI_SPI_MISO_PIN                    GPIO_PIN_6
	#define RPI_SPI_MOSI_PIN                    GPIO_PIN_7
	#define RPI_SPI_GPIO_PORT        			      GPIOA
	#define RPI_SPI_GPIO_AF                     GPIO_AF5_SPI1
	
	/* Definition for DAC_SPIx's DMA */
	#define RPI_SPI_RX_DMA_CHANNEL              DMA_CHANNEL_3
	#define RPI_SPI_RX_DMA_STREAM               DMA2_Stream2
	
	/* Definition for DAC_SPI's DMA NVIC */
	#define RPI_SPI_DMA_RX_IRQn                 DMA2_Stream2_IRQn
	#define RPI_SPI_DMA_RX_IRQHandler           DMA2_Stream2_IRQHandler
	
	/* Definition for RPI_SPI's IT NVIC */
	#define RPI_SPI_IRQn                        SPI1_IRQn
	#define RPI_SPI_IRQHandler                  SPI1_IRQHandler
	
	/* Definition for RPI GPIO Pins */	
	#define RPI_GPIO_CLK_ENABLE()      					__GPIOA_CLK_ENABLE()
	#define RPI_GPIO1_PIN 											GPIO_PIN_0
	#define RPI_GPIO2_PIN 											GPIO_PIN_1
	#define RPI_GPIO3_PIN 											GPIO_PIN_2
	#define RPI_GPIO4_PIN 											GPIO_PIN_3
	#define RPI_GPIO5_PIN 											GPIO_PIN_8
	#define RPI_GPIO_PORT 											GPIOA
	#define RPI_GPIO_FRAME_PIN 									RPI_GPIO1_PIN
	#define RPI_GPIO_FRAME_IRQn									EXTI0_IRQn //EXTIx, where x is pin number
	#define RPI_GPIO_FRAME_IRQHandler						EXTI0_IRQHandler //EXTIx, where x is pin number
	
	/* Definition for RPI UART Pins */	
	#define RPI_UART					      						USART1
	#define RPI_UART_BAUD_RATE      						115200 //0.02% error - page 973 Reference Manual
	#define RPI_UART_CLK_ENABLE()      					__USART1_CLK_ENABLE()
	#define RPI_UART_GPIO_CLK_ENABLE()      		__GPIOA_CLK_ENABLE()
	#define RPI_UART_DMA_CLK_ENABLE()           __DMA2_CLK_ENABLE()

	#define RPI_UART_TX_PIN 										GPIO_PIN_9
	#define RPI_UART_RX_PIN 										GPIO_PIN_10
	#define RPI_UART_AF	 												GPIO_AF7_USART1
	#define RPI_UART_PORT 											GPIOA
	#define RPI_UART_FORCE_RESET()         	    __USART1_FORCE_RESET()
	#define RPI_UART_RELEASE_RESET()        	  __USART1_RELEASE_RESET()
	
	/* Definition for RPI_UART's DMA */
	#define RPI_UART_TX_DMA_CHANNEL             DMA_CHANNEL_4
	#define RPI_UART_TX_DMA_STREAM              DMA2_Stream7

	/* Definition for RPI_UART's NVIC */
	#define RPI_UART_DMA_TX_IRQn                DMA2_Stream7_IRQn
	#define RPI_UART_DMA_TX_IRQHandler          DMA2_Stream7_IRQHandler
	
	#define RPI_SPI_BUFFER_SIZE									6 //Not used anymore?
	#define RPI_UART_BUFFER_SIZE								50
	
	#define Print(__DATA__)											UART_Print( (uint8_t*)(__DATA__ "\r\n"), (sizeof(__DATA__) / sizeof(*(__DATA__))) + 1 ) //Shortcut for printing string over serial
/* Exported functions ------------------------------------------------------- */
	//ADD user functions so can be called from other files
	//void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);
	void Error_Handler(void);
	void PulsePin(GPIO_TypeDef* port, uint16_t pin, PULSE_DIRECTION direction);
	Point_TypeDef GetPoint(BUFFER_SELECT buff, uint16_t index);
	uint8_t SetPoint(BUFFER_SELECT buff, uint16_t index, Point_TypeDef point);
	HAL_StatusTypeDef DAC_Send(SPI_HandleTypeDef *hspi, DAC_WRITE_MODE mode, DAC_ADDRESS address, uint16_t data);
	HAL_StatusTypeDef DAC_SendReceive(SPI_HandleTypeDef *hspi, DAC_WRITE_MODE mode, DAC_ADDRESS address, uint16_t data, uint8_t *rxbuff);
	void UART_Print(uint8_t* message, uint8_t length);
	
/* Public variables ------------------------------------------------------- */	
	//DAC
	extern SPI_HandleTypeDef  hDACspi; //Declared in init.c
	extern SPI_HandleTypeDef  hRPIspi; //Declared in init.c
	extern UART_HandleTypeDef huart;; //Declared in init.c
	extern uint8_t DAC_SPI_Buffer[DAC_BUFFER_SIZE]; //Declared in init.c
	//extern uint8_t RPI_SPI_Buffer[RPI_SPI_BUFFER_SIZE]; //Declared in init.c

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
