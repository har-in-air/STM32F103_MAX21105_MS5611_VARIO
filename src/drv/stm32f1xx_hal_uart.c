#include "common.h"
#include "config.h"
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_uart.h"


#ifdef UART_MTK3329_BINARY
extern void gps_UartRcvByte(uint8_t b);
#endif

#ifdef UART_NMEA
volatile int gbNMEAFlag = 0;
#endif
   
static void UART_SetConfig (UART_HandleTypeDef *huart);

static HAL_StatusTypeDef UART_WaitOnFlagUntilTimeout(UART_HandleTypeDef *huart, uint32_t Flag, FlagStatus Status, uint32_t Timeout);


HAL_StatusTypeDef HAL_UART_Init(UART_HandleTypeDef *huart){
  if(huart == NULL)  {
    return HAL_ERROR;
  }

  if(huart->Init.HwFlowCtl != UART_HWCONTROL_NONE)  {
    /* The hardware flow control is available only for USART1, USART2, USART3 */
    assert_param(IS_UART_HWFLOW_INSTANCE(huart->Instance));
    assert_param(IS_UART_HARDWARE_FLOW_CONTROL(huart->Init.HwFlowCtl));
  }
  else  {
    assert_param(IS_UART_INSTANCE(huart->Instance));
  }
  assert_param(IS_UART_WORD_LENGTH(huart->Init.WordLength));
  assert_param(IS_UART_OVERSAMPLING(huart->Init.OverSampling));
  
  if(huart->State == HAL_UART_STATE_RESET)  {  
    huart->Lock = HAL_UNLOCKED;
        /* Init the low level hardware */
    HAL_UART_MspInit(huart);
  }

  huart->State = HAL_UART_STATE_BUSY;
  __HAL_UART_DISABLE(huart);
  UART_SetConfig(huart);
  
  /* In asynchronous mode, the following bits must be kept cleared: 
     - LINEN and CLKEN bits in the USART_CR2 register,
     - SCEN, HDSEL and IREN  bits in the USART_CR3 register.*/
  CLEAR_BIT(huart->Instance->CR2, (USART_CR2_LINEN | USART_CR2_CLKEN));
  CLEAR_BIT(huart->Instance->CR3, (USART_CR3_SCEN | USART_CR3_HDSEL | USART_CR3_IREN));
  
  __HAL_UART_ENABLE(huart);
    /* Enable the UART Data Register not empty Interrupt */
  __HAL_UART_ENABLE_IT(huart, UART_IT_RXNE);
  
  huart->ErrorCode = HAL_UART_ERROR_NONE;
  huart->State= HAL_UART_STATE_READY;
  
  return HAL_OK;
}



HAL_StatusTypeDef HAL_UART_DeInit(UART_HandleTypeDef *huart){
  if(huart == NULL){
    return HAL_ERROR;
  }
  
  assert_param(IS_UART_INSTANCE(huart->Instance));
  huart->State = HAL_UART_STATE_BUSY;
  __HAL_UART_DISABLE(huart);
  
  huart->Instance->CR1 = 0x0;
  huart->Instance->CR2 = 0x0;
  huart->Instance->CR3 = 0x0;
  
  /* DeInit the low level hardware */
  HAL_UART_MspDeInit(huart);
  __HAL_UART_DISABLE_IT(huart, UART_IT_RXNE);
  huart->ErrorCode = HAL_UART_ERROR_NONE;
  huart->State = HAL_UART_STATE_RESET;

  __HAL_UNLOCK(huart);

  return HAL_OK;
}

int UART_SendData(UART_HandleTypeDef *huart, uint16_t c){
  uint32_t timeout = 5;
  huart->Instance->DR = c & 0x00FF;
  if(UART_WaitOnFlagUntilTimeout(huart, UART_FLAG_TC, RESET, timeout) != HAL_OK){ 
      return HAL_TIMEOUT;
      }
  return HAL_OK;
}



uint16_t UART_ReceiveData(UART_HandleTypeDef* huart) {
  return (uint16_t)(huart->Instance->DR & 0x00FF);
}




/**
  * @brief  This function handles UART interrupt request.
  * @param  huart: Pointer to a UART_HandleTypeDef structure that contains
  *                the configuration information for the specified UART module.
  * @retval None
  */
void HAL_UART_IRQHandler(UART_HandleTypeDef *huart)
{
  uint32_t tmp_flag = 0, tmp_it_source = 0;

  tmp_flag = __HAL_UART_GET_FLAG(huart, UART_FLAG_PE);
  tmp_it_source = __HAL_UART_GET_IT_SOURCE(huart, UART_IT_PE);  
  /* UART parity error interrupt occurred ------------------------------------*/
  if((tmp_flag != RESET) && (tmp_it_source != RESET))
  { 
    __HAL_UART_CLEAR_PEFLAG(huart);
    
    huart->ErrorCode |= HAL_UART_ERROR_PE;
  }
  
  tmp_flag = __HAL_UART_GET_FLAG(huart, UART_FLAG_FE);
  tmp_it_source = __HAL_UART_GET_IT_SOURCE(huart, UART_IT_ERR);
  /* UART frame error interrupt occurred -------------------------------------*/
  if((tmp_flag != RESET) && (tmp_it_source != RESET))
  { 
    __HAL_UART_CLEAR_FEFLAG(huart);
    
    huart->ErrorCode |= HAL_UART_ERROR_FE;
  }
  
  tmp_flag = __HAL_UART_GET_FLAG(huart, UART_FLAG_NE);
  /* UART noise error interrupt occurred -------------------------------------*/
  if((tmp_flag != RESET) && (tmp_it_source != RESET))
  { 
    __HAL_UART_CLEAR_NEFLAG(huart);
    
    huart->ErrorCode |= HAL_UART_ERROR_NE;
  }
  
  tmp_flag = __HAL_UART_GET_FLAG(huart, UART_FLAG_ORE);
  /* UART Over-Run interrupt occurred ----------------------------------------*/
  if((tmp_flag != RESET) && (tmp_it_source != RESET))
  { 
    __HAL_UART_CLEAR_OREFLAG(huart);
    
    huart->ErrorCode |= HAL_UART_ERROR_ORE;
  }
  
  tmp_flag = __HAL_UART_GET_FLAG(huart, UART_FLAG_RXNE);
  tmp_it_source = __HAL_UART_GET_IT_SOURCE(huart, UART_IT_RXNE);
  /* UART in mode Receiver ---------------------------------------------------*/
  if((tmp_flag != RESET) && (tmp_it_source != RESET))  { 
        uint8_t b = (uint8_t)(huart->Instance->DR & 0x00FF); 
        if (huart->Instance == USART1) {
            huart->rxFifoHead++;     // increment head pointer
            if (huart->rxFifoHead == UART_RX_FIFO_SIZE)   huart->rxFifoHead = 0; //wrap if needed
            if (huart->rxFifoHead == huart->rxFifoTail) {
                //uart_ReportError("FIFO overrun");
                }
            huart->rxFifo[huart->rxFifoHead] = b;   // place in buffer
            }
        else if (huart->Instance == USART2) { // mtk3329 connected to uart2
#ifdef UART_MTK3329_BINARY		
            gps_UartRcvByte(b);
#endif			
#ifdef UART_NMEA
                if (b == 36) { // $
                    huart->rxFifoHead = 0;
                    huart->rxFifoTail = 0;
                    }
                else
				if (b == 10) { // /n
                    b = 0; // end of string
					gbNMEAFlag = 1;
                    }
				huart->rxFifo[huart->rxFifoHead] = b;
                huart->rxFifoHead++;     // increment head pointer
                if (huart->rxFifoHead == UART_RX_FIFO_SIZE)   huart->rxFifoHead = 0; //wrap if needed
#endif			
#ifdef UART_STANDARD
            huart->rxFifoHead++;     // increment head pointer
            if (huart->rxFifoHead == UART_RX_FIFO_SIZE)   huart->rxFifoHead = 0; //wrap if needed
            if (huart->rxFifoHead == huart->rxFifoTail) {
                //uart_ReportError("FIFO overrun");
                }
            huart->rxFifo[huart->rxFifoHead] = b;   // place in buffer
#endif
            }
        //led_Toggle();
        }
  
  tmp_flag = __HAL_UART_GET_FLAG(huart, UART_FLAG_TXE);
  tmp_it_source = __HAL_UART_GET_IT_SOURCE(huart, UART_IT_TXE);

  /* UART in mode Transmitter ------------------------------------------------*/
  if((tmp_flag != RESET) && (tmp_it_source != RESET))
  {
    //UART_Transmit_IT(huart);
  }

  tmp_flag = __HAL_UART_GET_FLAG(huart, UART_FLAG_TC);
  tmp_it_source = __HAL_UART_GET_IT_SOURCE(huart, UART_IT_TC);
  /* UART in mode Transmitter end --------------------------------------------*/
  if((tmp_flag != RESET) && (tmp_it_source != RESET))
  {
    //UART_EndTransmit_IT(huart);
  }  

  if(huart->ErrorCode != HAL_UART_ERROR_NONE)
  {
    /* Set the UART state ready to be able to start again the process */
    huart->State = HAL_UART_STATE_READY;
    
    HAL_UART_ErrorCallback(huart);
  }  
}



/**
  * @brief  Tx Transfer completed callbacks.
  * @param  huart: Pointer to a UART_HandleTypeDef structure that contains
  *                the configuration information for the specified UART module.
  * @retval None
  */
 __weak void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
  /* NOTE: This function should not be modified, when the callback is needed,
           the HAL_UART_TxCpltCallback can be implemented in the user file
   */ 
}

/**
  * @brief  Tx Half Transfer completed callbacks.
  * @param  huart: Pointer to a UART_HandleTypeDef structure that contains
  *                the configuration information for the specified UART module.
  * @retval None
  */
 __weak void HAL_UART_TxHalfCpltCallback(UART_HandleTypeDef *huart)
{
  /* NOTE: This function should not be modified, when the callback is needed,
           the HAL_UART_TxHalfCpltCallback can be implemented in the user file
   */ 
}

/**
  * @brief  Rx Transfer completed callbacks.
  * @param  huart: Pointer to a UART_HandleTypeDef structure that contains
  *                the configuration information for the specified UART module.
  * @retval None
  */
__weak void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  /* NOTE: This function should not be modified, when the callback is needed,
           the HAL_UART_RxCpltCallback can be implemented in the user file
   */
}

/**
  * @brief  Rx Half Transfer completed callbacks.
  * @param  huart: Pointer to a UART_HandleTypeDef structure that contains
  *                the configuration information for the specified UART module.
  * @retval None
  */
__weak void HAL_UART_RxHalfCpltCallback(UART_HandleTypeDef *huart)
{
  /* NOTE: This function should not be modified, when the callback is needed,
           the HAL_UART_RxHalfCpltCallback can be implemented in the user file
   */
}

/**
  * @brief  UART error callbacks.
  * @param  huart: Pointer to a UART_HandleTypeDef structure that contains
  *                the configuration information for the specified UART module.
  * @retval None
  */
 __weak void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
  /* NOTE: This function should not be modified, when the callback is needed,
           the HAL_UART_ErrorCallback can be implemented in the user file
   */ 
}




/** @defgroup UART_Exported_Functions_Group4 Peripheral State and Errors functions 
  *  @brief   UART State and Errors functions 
  *
@verbatim   
  ==============================================================================
                 ##### Peripheral State and Errors functions #####
  ==============================================================================  
 [..]
   This subsection provides a set of functions allowing to return the State of 
   UART communication process, return Peripheral Errors occurred during communication 
   process
   (+) HAL_UART_GetState() API can be helpful to check in run-time the state of the UART peripheral.
   (+) HAL_UART_GetError() check in run-time errors that could be occurred during communication. 

@endverbatim
  * @{
  */
  
/**
  * @brief  Returns the UART state.
  * @param  huart: Pointer to a UART_HandleTypeDef structure that contains
  *                the configuration information for the specified UART module.
  * @retval HAL state
  */
HAL_UART_StateTypeDef HAL_UART_GetState(UART_HandleTypeDef *huart)
{
  return huart->State;
}

/**
* @brief  Return the UART error code
* @param  huart: Pointer to a UART_HandleTypeDef structure that contains
  *              the configuration information for the specified UART.
* @retval UART Error Code
*/
uint32_t HAL_UART_GetError(UART_HandleTypeDef *huart)
{
  return huart->ErrorCode;
}



/**
  * @brief  This function handles UART Communication Timeout.
  * @param  huart: Pointer to a UART_HandleTypeDef structure that contains
  *                the configuration information for the specified UART module.
  * @param  Flag: specifies the UART flag to check.
  * @param  Status: The new Flag status (SET or RESET).
  * @param  Timeout: Timeout duration
  * @retval HAL status
  */
static HAL_StatusTypeDef UART_WaitOnFlagUntilTimeout(UART_HandleTypeDef *huart, uint32_t Flag, FlagStatus Status, uint32_t Timeout)
{
  uint32_t tickstart = 0;

  /* Get tick */ 
  tickstart = HAL_GetTick();

  /* Wait until flag is set */
  if(Status == RESET)
  {
    while(__HAL_UART_GET_FLAG(huart, Flag) == RESET)
    {
      /* Check for the Timeout */
      if(Timeout != HAL_MAX_DELAY)
      {
        if((Timeout == 0)||((HAL_GetTick() - tickstart ) > Timeout))
        {
          /* Disable TXE, RXNE, PE and ERR (Frame error, noise error, overrun error) interrupts for the interrupt process */
          //__HAL_UART_DISABLE_IT(huart, UART_IT_TXE);
          //__HAL_UART_DISABLE_IT(huart, UART_IT_RXNE);
          //__HAL_UART_DISABLE_IT(huart, UART_IT_PE);
          //__HAL_UART_DISABLE_IT(huart, UART_IT_ERR);

          huart->State= HAL_UART_STATE_READY;

          /* Process Unlocked */
          __HAL_UNLOCK(huart);

          return HAL_TIMEOUT;
        }
      }
    }
  }
  else
  {
    while(__HAL_UART_GET_FLAG(huart, Flag) != RESET)
    {
      /* Check for the Timeout */
      if(Timeout != HAL_MAX_DELAY)
      {
        if((Timeout == 0)||((HAL_GetTick() - tickstart ) > Timeout))
        {
          /* Disable TXE, RXNE, PE and ERR (Frame error, noise error, overrun error) interrupts for the interrupt process */
          //__HAL_UART_DISABLE_IT(huart, UART_IT_TXE);
          //__HAL_UART_DISABLE_IT(huart, UART_IT_RXNE);
          //__HAL_UART_DISABLE_IT(huart, UART_IT_PE);
          //__HAL_UART_DISABLE_IT(huart, UART_IT_ERR);

          huart->State= HAL_UART_STATE_READY;

          /* Process Unlocked */
          __HAL_UNLOCK(huart);

          return HAL_TIMEOUT;
        }
      }
    }
  }
  return HAL_OK;
}



/**
  * @brief  Configures the UART peripheral. 
  * @param  huart: Pointer to a UART_HandleTypeDef structure that contains
  *                the configuration information for the specified UART module.
  * @retval None
  */
static void UART_SetConfig(UART_HandleTypeDef *huart)
{
  uint32_t tmpreg = 0x00;
  
  /* Check the parameters */
  assert_param(IS_UART_BAUDRATE(huart->Init.BaudRate));  
  assert_param(IS_UART_STOPBITS(huart->Init.StopBits));
  assert_param(IS_UART_PARITY(huart->Init.Parity));
  assert_param(IS_UART_MODE(huart->Init.Mode));

  /*------- UART-associated USART registers setting : CR2 Configuration ------*/
  /* Configure the UART Stop Bits: Set STOP[13:12] bits according 
   * to huart->Init.StopBits value */
  MODIFY_REG(huart->Instance->CR2, USART_CR2_STOP, huart->Init.StopBits);

  /*------- UART-associated USART registers setting : CR1 Configuration ------*/
  /* Configure the UART Word Length, Parity and mode: 
     Set the M bits according to huart->Init.WordLength value 
     Set PCE and PS bits according to huart->Init.Parity value
     Set TE and RE bits according to huart->Init.Mode value */
  tmpreg = (uint32_t)huart->Init.WordLength | huart->Init.Parity | huart->Init.Mode ;
  MODIFY_REG(huart->Instance->CR1, 
             (uint32_t)(USART_CR1_M | USART_CR1_PCE | USART_CR1_PS | USART_CR1_TE | USART_CR1_RE), 
             tmpreg);
  
  /*------- UART-associated USART registers setting : CR3 Configuration ------*/
  /* Configure the UART HFC: Set CTSE and RTSE bits according to huart->Init.HwFlowCtl value */
  MODIFY_REG(huart->Instance->CR3, (USART_CR3_RTSE | USART_CR3_CTSE), huart->Init.HwFlowCtl);
  
  /*------- UART-associated USART registers setting : BRR Configuration ------*/
  if((huart->Instance == USART1))
  {
    huart->Instance->BRR = UART_BRR_SAMPLING16(HAL_RCC_GetPCLK2Freq(), huart->Init.BaudRate);
  }
  else
  {
    huart->Instance->BRR = UART_BRR_SAMPLING16(HAL_RCC_GetPCLK1Freq(), huart->Init.BaudRate);
  }
}



