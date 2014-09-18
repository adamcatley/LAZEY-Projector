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
#include "main.h"
#include "init.h"
#include <stm32f4xx_hal.h>
#include "boot_screen.h"


/** @addtogroup STM32F4xx_HAL_Examples
  * @{
  */

/** @addtogroup Templates
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
	//Buffer related shortcuts to make code simpler
	#define	 Current_Point 									BufferState[Current_Buffer].current_point						//Current point index being sent to DAC
	#define	 Current_Received_Point 				BufferState[Current_Receiving_Buffer].current_point	//Current point index being received from RPI
	#define  Current_Frame_Points 					BufferState[Current_Buffer].length 									//shortcut for current frame length
	#define  Current_Frame_Drawn	 					BufferState[Current_Buffer].drawn 									//Whether current frame has been drawn at least once
	#define  Current_Frame_State	 					BufferState[Current_Buffer].state
	#define  Current_Receiving_Frame_State	BufferState[Current_Receiving_Buffer].state
	#define  Current_Frame									BufferState[Current_Receiving_Buffer]
	#define  Current_Receiving_Frame				BufferState[Current_Receiving_Buffer]
/* Private variables ---------------------------------------------------------*/
	volatile Buffer_TypeDef BufferState[2]; 											//Holds information about both frame buffers
	volatile BUFFER_SELECT Current_Buffer;  						//Current frame being sent to DAC
	volatile BUFFER_SELECT Current_Receiving_Buffer;		//Current frame being received from RPI
	
	volatile uint8_t* UART_Queue;
	volatile uint8_t  UART_Queue_Length = 0;
	volatile uint8_t 	TxBuffer[DAC_BUFFER_SIZE];
	//Point_TypeDef* Buffer1[POINTS_PER_SEC];
	//*Buffer1	= (Point_TypeDef*)0x68000000;
	//volatile Point_TypeDef *Buffer1 = NULL;
	//volatile Point_TypeDef* Buffer2[POINTS_PER_SEC] = 0x6804000;
	
/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
	uint16_t count = 0;
  /* Add your application code here
     */
	Init();
	//TODO: Initialise BufferState[]
	BufferState[BUFFER_0].state = BUFFER_IDLE;
	BufferState[BUFFER_1].state = BUFFER_IDLE;
	BufferState[BUFFER_0].address = FMC_SRAM_ADDR + 0; //0x68000000
	BufferState[BUFFER_1].address = FMC_SRAM_ADDR + FMC_SRAM_BANK_SIZE; //0x68040000
	Current_Receiving_Buffer = BUFFER_0;
	Current_Buffer = BUFFER_1;//Required to make code run at startup
	Current_Frame_Drawn = 1; //Required to make code run at startup
	
	//Boot Screen setup
	BufferState[BUFFER_1].state = BUFFER_DRAWING; //Set as drawing
	Current_Frame_Drawn = 0; //Display at least once at startup
	BufferState[BUFFER_1].length = (uint16_t)BOOT_SCREEN_POINTS; //Set size
	*(__IO uint32_t*)(BufferState[Current_Buffer].address) = (uint16_t)BOOT_SCREEN_POINTS; //Store size in buffer
	for (count = 0; count < BOOT_SCREEN_POINTS * POINT_SIZE_8; count++){ //Copy boot screen to buffer
		*(__IO uint32_t*)(BufferState[Current_Buffer].address + 2 + count) = boot_scren_data[count];
	}
		
	HAL_GPIO_WritePin(STATUS_LED_PORT, STATUS_LED_PIN, GPIO_PIN_RESET); //LED off at end of init
	
  /* Infinite loop */
  while (1)
  {
		__ASM("nop");
  }
}

Point_TypeDef GetPoint(BUFFER_SELECT buff, uint16_t index){
	Point_TypeDef tmpPoint;
	uint32_t addr;
	if(index > POINTS_PER_SEC) return tmpPoint; //Could cause error
	addr = BufferState[buff].address + 2;//Select 0x68000000 or 0x68040000 depending on selected buffer, with 2 byte offset where length is stored
	
	tmpPoint._Reserved = *(__IO uint8_t*)(addr + (POINT_SIZE_8 * index) + 0);
	tmpPoint.Brightness = *(__IO uint8_t*)(addr + (POINT_SIZE_8 * index) + 1);	
	tmpPoint.X = *(__IO uint16_t*)(addr + (POINT_SIZE_8 * index) + 2);
	tmpPoint.Y = *(__IO uint16_t*)(addr + (POINT_SIZE_8 * index) + 4);
	
	return tmpPoint;
}

//Redundant?
uint8_t SetPoint(BUFFER_SELECT buff, uint16_t index, Point_TypeDef point){
	uint32_t addr;
	if(index > POINTS_PER_SEC) return 1;
	addr = BufferState[buff].address + 2;//Select 0x68000000 or 0x68040000 depending on selected buffer
	
	*(__IO uint32_t*)(addr + (POINT_SIZE_8 * index) + 0) = (uint8_t)point._Reserved;
	*(__IO uint32_t*)(addr + (POINT_SIZE_8 * index) + 1) = (uint8_t)point.Brightness;
	*(__IO uint32_t*)(addr + (POINT_SIZE_8 * index) + 2) = (uint16_t)point.X;
	*(__IO uint32_t*)(addr + (POINT_SIZE_8 * index) + 4) = (uint16_t)point.Y;
	
	return 0;
}

void PulsePin(GPIO_TypeDef* port, uint16_t pin, PULSE_DIRECTION direction){	
	if (direction == PULSE_HIGH){
		HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET);
		__ASM("nop");//Longer delay required for slower devices? Currently gives 70ns pulse
		HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET);		
	}
	else { //PULSE_LOW
		HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET);
		__ASM("nop");//Longer delay required for slower devices? Currently gives 70ns pulse
		HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET);			
	}
}


//Note: DAC returns updated register value on next transmission
HAL_StatusTypeDef DAC_Send(SPI_HandleTypeDef *hspi, DAC_WRITE_MODE mode, DAC_ADDRESS address, uint16_t data){
	
	//while(HAL_SPI_GetState(hspi) != HAL_SPI_STATE_READY){} //Wait until transmission completed
	
	TxBuffer[0] = mode | address;
	TxBuffer[1] = (uint8_t)(data >> 8);
	TxBuffer[2] = (uint8_t)data;		
		
	HAL_GPIO_WritePin(DAC_SPI_GPIO_PORT, DAC_SPI_NSS_PIN, GPIO_PIN_RESET); //NSS low to begin transfer
	
	HAL_SPI_Transmit_DMA(hspi, (uint8_t*)TxBuffer, DAC_BUFFER_SIZE);		
	hspi->hdmatx->XferHalfCpltCallback = 0; //Disable half complete callback, saves ~1us
	
	return HAL_OK;
}

HAL_StatusTypeDef DAC_SendReceive(SPI_HandleTypeDef *hspi, DAC_WRITE_MODE mode, DAC_ADDRESS address, uint16_t data, uint8_t *rxbuff){
	static uint8_t TxBuffer[DAC_BUFFER_SIZE];
	
	while(HAL_SPI_GetState(hspi) != HAL_SPI_STATE_READY){} //Wait until transmission completed
	
	TxBuffer[0] = mode | address;
	TxBuffer[1] = (uint8_t)(data >> 8);
	TxBuffer[2] = (uint8_t)data;		
		
	HAL_GPIO_WritePin(DAC_SPI_GPIO_PORT, DAC_SPI_NSS_PIN, GPIO_PIN_RESET); //NSS low to begin transfer
	
	if(HAL_SPI_TransmitReceive_DMA(hspi, (uint8_t*)TxBuffer, (uint8_t*)rxbuff, DAC_BUFFER_SIZE) != HAL_OK)
  {    
    Error_Handler();/* Transfer error in transmission process */
  }	
	return HAL_OK;
}


//Print out serial data over UART to the RPI for debugging
void UART_Print(uint8_t *message, uint8_t length){
	//Wait until transmission completed, could be an issue if long delay
	//while(HAL_UART_GetState(&huart) != HAL_UART_STATE_READY){} //Not confirmed to work
	if (HAL_UART_GetState(&huart) != HAL_UART_STATE_READY) {
		UART_Queue = message;
		UART_Queue_Length = length;
		return;
	}
	//Send message
	if(HAL_UART_Transmit_DMA(&huart, message, length)!= HAL_OK)
  {
    Error_Handler();
  }
}

BUFFER_SELECT Switch_Buffer(){
	//Would be better to use simple operator eg. !Current_Buffer
	if (Current_Buffer == BUFFER_0){
		Current_Buffer = BUFFER_1;
	} else { //Current_Buffer == BUFFER_1
		Current_Buffer = BUFFER_0;
	}
	return Current_Buffer;
}	

BUFFER_SELECT Switch_Receiving_Buffer(){
	//Would be better to use simple operator eg. !Current_Buffer
	if (Current_Receiving_Buffer == BUFFER_0){
		Current_Receiving_Buffer = BUFFER_1;
	} else { //Current_Receiving_Buffer == BUFFER_1
		Current_Receiving_Buffer = BUFFER_0;
	}
	return Current_Buffer;
}	

/*
 *
 *	Callbacks
 *
 */

/**
  * @brief  Timer period elapsed callback.
  * @param  htim: TIM handle.
  * @note   Output points to DAC at 30,000pps
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	Point_TypeDef tmpPoint;
	//PulsePin(DEBUG_IO_PORT, DEBUG_IO_1, PULSE_HIGH);
	
	/* Send next point */
	if (Current_Frame_State != BUFFER_DRAWING) return; //Wait until frame is received
	
	HAL_GPIO_WritePin(DEBUG_IO_PORT, DEBUG_IO_1, GPIO_PIN_SET);
	
	tmpPoint = GetPoint(Current_Buffer, Current_Point);	
	
	DAC_Send(&hDACspi, DAC_WRITE, DAC_X_ADDR, tmpPoint.X); //Each call takes 7.34us
	DAC_Send(&hDACspi, DAC_WRITE, DAC_Y_ADDR, tmpPoint.Y);
	DAC_Send(&hDACspi, DAC_WRITE, DAC_BRI_ADDR, (uint16_t)(tmpPoint.Brightness << 8) + 0xFF); //Better way to convert brightness to 16bit?
	PulsePin(DAC_LDAC_PORT, DAC_LDAC_PIN, PULSE_LOW); //Upate DAC outputs
	
	Current_Point++;
	
	if ((Current_Point >= Current_Frame_Points)){ //End of frame reached
		DEBUG_IO_PORT->ODR ^= DEBUG_IO_2; //frame duration indicator
		Current_Point = 0;
		Current_Frame_Drawn = 1;
	}
	HAL_GPIO_WritePin(DEBUG_IO_PORT, DEBUG_IO_1, GPIO_PIN_RESET);
}

/**
  * @brief  TxRx Transfer completed callback.
  * @param  hspi: SPI handle.
  * @note   Sets NSS back to idle at end of SPI data frame
  * @retval None
  */
void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
	/* Transfer process is complete */
	if (hspi->Instance == DAC_SPI){		
		HAL_GPIO_WritePin(DAC_SPI_GPIO_PORT, DAC_SPI_NSS_PIN, GPIO_PIN_SET); //Take NSS high at end of frame
		//DEBUG_IO_PORT->ODR ^= DEBUG_IO_2; //This happens 100ns after previous function call
	}
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
	/* Transfer process is complete */
	if (hspi->Instance == DAC_SPI){		
		HAL_GPIO_WritePin(DAC_SPI_GPIO_PORT, DAC_SPI_NSS_PIN, GPIO_PIN_SET); //Take NSS high at end of frame
		//DEBUG_IO_PORT->ODR ^= DEBUG_IO_2; //This happens 100ns after previous function call
	}
}

/**
  * @brief  Rx Transfer completed callbacks
  * @param  hspi: pointer to a SPI_HandleTypeDef structure that contains
  *                the configuration information for SPI module.
  * @note   Process new point received from RPI
  * @retval None
  */
void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
{
	//Point_TypeDef tmpPoint;
	if (hspi->Instance == RPI_SPI){
		
	//Frame points received, DMA transfer
	if (Current_Frame_Drawn == 1){
		Current_Receiving_Frame.length = *(__IO uint16_t*)(Current_Frame.address + 0);
		if (Current_Receiving_Frame.length == 0) {
			Current_Receiving_Frame_State = BUFFER_IDLE; //Disable frame if has 0 length
		} else {
			Current_Receiving_Frame_State = BUFFER_DRAWING;	//Set received frame to drawing
		}
		Current_Frame_State = BUFFER_IDLE;
		Current_Frame_Drawn = 0;
		Switch_Buffer(); 
		Switch_Receiving_Buffer(); //Set to receive to other frame
		//modifications
		DAC_Send(&hDACspi, DAC_WRITE, DAC_BRI_ADDR, 0x00); //Blanks laser until next frame starts
		PulsePin(DAC_LDAC_PORT, DAC_LDAC_PIN, PULSE_LOW);
	} 
	#ifdef DEBUG_ONLY
		Print("Frame received");
	#endif
	}	
}

/**
  * @brief  EXTI line detection callbacks.
  * @param  GPIO_Pin: Specifies the pins connected EXTI line
  * @note   New frame pin changed to signify started receiving new frame point data
  * @retval None
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if(GPIO_Pin == RPI_GPIO_FRAME_PIN) //Make sure not aleady
  {
	//If new frame singal received before current ends, terminate current frame and move to new data
	if(hRPIspi.hdmarx->State != HAL_DMA_STATE_READY) HAL_SPI_DMAStop(&hRPIspi); 
		
	//New frame started
	Current_Received_Point = 0;
	Current_Receiving_Frame_State = BUFFER_RECEIVING;
	Current_Receiving_Frame.length = 0;
		
	//Wait for frame size and write directly to buffer length
	//HAL_SPI_Receive(&hRPIspi, (uint8_t*)(&Current_Receiving_Frame.length), 1, 1000);
	
		
	//Set DMA to handle receiving the point data for the whole frame directly to external memeory through FMC
	//HAL_SPI_Receive_DMA(&hRPIspi, (uint8_t*)Current_Receiving_Frame.address, Current_Receiving_Frame.length * POINT_SIZE_16); //Listen for next point
	HAL_SPI_Receive_DMA(&hRPIspi, (uint8_t*)Current_Receiving_Frame.address, (uint16_t)((POINTS_PER_SEC * POINT_SIZE_16) + 1)); //Receive whole frame points and length
	hRPIspi.hdmarx->XferHalfCpltCallback = 0; //Disable half complete callback
	#ifdef DEBUG_ONLY
		Print("New Frame Detected");
	#endif
  }
}

/* Error etc callbacks */

/**
  * @brief  SPI error callbacks.
  * @param  hspi: SPI handle
  * @note   This example shows a simple way to report transfer error, and you can
  *         add your own implementation.
  * @retval None
  */
 void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi)
{  
	if (hspi->Instance == RPI_SPI){
		//Catch overrun error and abort receiving current frame
		__HAL_SPI_CLEAR_OVRFLAG(hspi); 
		HAL_SPI_DMAStop(hspi);
		Current_Receiving_Frame.length = 0;
		return;
	}
  Error_Handler();// Transfer error in reception/transmission process
}

/**
  * @brief  Tx Transfer completed callback
  * @param  UartHandle: UART handle. 
  * @note   This example shows a simple way to report end of IT Tx transfer, and 
  *         you can add your own implementation. 
  * @retval None
  */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *UartHandle)
{
	if (UART_Queue_Length != 0){
		HAL_UART_Transmit_DMA(UartHandle, (uint8_t*)UART_Queue, UART_Queue_Length);
		UART_Queue_Length = 0;
	}
}

/**
  * @brief  UART error callbacks
  * @param  UartHandle: UART handle
  * @note   This example shows a simple way to report transfer error, and you can
  *         add your own implementation.
  * @retval None
  */
 void HAL_UART_ErrorCallback(UART_HandleTypeDef *UartHandle)
{
	Error_Handler();
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void Error_Handler(void)
{
  /* User may add here some code to deal with this error */
	HAL_GPIO_WritePin(STATUS_LED_PORT, STATUS_LED_PIN, GPIO_PIN_SET);
	Print("\x1b[31mError: Unkown error has caused the system to stop!"); //Print error message IN RED!
  while(1)
  {
  }
}

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET);
  while (1)
  {
  }
}
#endif

/**
  * @}
  */ 

/**
  * @}
  */ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
