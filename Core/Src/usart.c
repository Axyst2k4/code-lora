/* Includes ------------------------------------------------------------------*/
#include "usart.h"
#include "dma.h" // Giả sử file này có tồn tại

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

// Khai báo tất cả các handle ở đầu file
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
DMA_HandleTypeDef hdma_usart1_rx;
DMA_HandleTypeDef hdma_usart2_tx;

/* USART1 init function */
void MX_USART1_UART_Init(void)
{
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX; // Chế độ cả truyền và nhận
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USART2 init function */
void MX_USART2_UART_Init(void)
{
  huart2.Instance        = USART2;
  huart2.Init.BaudRate   = USART_BAUDRATE; // Giữ nguyên tên biến
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits   = UART_STOPBITS_1;
  huart2.Init.Parity     = UART_PARITY_NONE;
  huart2.Init.Mode       = UART_MODE_TX; // Giữ nguyên chế độ chỉ truyền
  huart2.Init.HwFlowCtl  = UART_HWCONTROL_NONE;

  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief Hàm khởi tạo phần cứng cấp thấp cho TẤT CẢ các UART.
  * Hàm này sẽ được HAL_UART_Init() tự động gọi.
  */
void HAL_UART_MspInit(UART_HandleTypeDef* huart)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  // --- CẤU HÌNH CHO USART1 ---
  if(huart->Instance==USART1)
  {
    /* 1. Bật Clock */
    __HAL_RCC_USART1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_DMA1_CLK_ENABLE();

    /** Cấu hình chân GPIO: PA9 (TX), PA10 (RX) */
    GPIO_InitStruct.Pin = GPIO_PIN_9|GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF4_USART1; // Chú ý: AF có thể khác cho F1
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* 2. Cấu hình DMA cho kênh nhận (RX) */
    hdma_usart1_rx.Instance = DMA1_Channel3; // Kênh có thể khác cho F1
    // hdma_usart1_rx.Init.Request = DMA_REQUEST_4; // Request không dùng cho F1
    hdma_usart1_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_usart1_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart1_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart1_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart1_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart1_rx.Init.Mode = DMA_NORMAL;
    hdma_usart1_rx.Init.Priority = DMA_PRIORITY_LOW;
    if (HAL_DMA_Init(&hdma_usart1_rx) != HAL_OK)
    {
      Error_Handler();
    }
    __HAL_LINKDMA(huart, hdmarx, hdma_usart1_rx);

    /* 3. Cấu hình Ngắt (NVIC) */
    HAL_NVIC_SetPriority(DMA1_Channel2_3_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA1_Channel2_3_IRQn);    // Kênh có thể khác cho F1

    HAL_NVIC_SetPriority(USART1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);
  }
  // --- CẤU HÌNH CHO USART2 ---
  else if (huart->Instance == USART2)
  {
    /* 1. Bật Clock */
    __HAL_RCC_USART2_CLK_ENABLE();
    USARTx_TX_GPIO_CLK_ENABLE(); // Giữ nguyên macro
    USARTx_RX_GPIO_CLK_ENABLE(); // Giữ nguyên macro
    DMAx_CLK_ENABLE();           // Giữ nguyên macro

    /* 2. Cấu hình Clock nguồn cho USART2 */
    // Phần này thường không cần cho F1, nhưng giữ lại cũng không sao
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USART2;
    PeriphClkInitStruct.Usart2ClockSelection = RCC_USART2CLKSOURCE_SYSCLK;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      Error_Handler();
    }

    /** Cấu hình chân GPIO: PA2 (TX), PA3 (RX) */
    GPIO_InitStruct.Pin = USARTx_TX_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    // GPIO_InitStruct.Alternate = USARTx_TX_AF; // F1 không dùng Alternate
    HAL_GPIO_Init(USARTx_TX_GPIO_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = USARTx_RX_Pin;
    // GPIO_InitStruct.Alternate = USARTx_RX_AF; // F1 không dùng Alternate
    HAL_GPIO_Init(USARTx_RX_GPIO_Port, &GPIO_InitStruct);

    /* 3. Cấu hình DMA cho kênh truyền (TX) */
    hdma_usart2_tx.Instance                 = USARTx_TX_DMA_CHANNEL;
    // hdma_usart2_tx.Init.Request             = USARTx_TX_DMA_REQUEST; // F1 không dùng
    hdma_usart2_tx.Init.Direction           = DMA_MEMORY_TO_PERIPH;
    hdma_usart2_tx.Init.PeriphInc           = DMA_PINC_DISABLE;
    hdma_usart2_tx.Init.MemInc              = DMA_MINC_ENABLE;
    hdma_usart2_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart2_tx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
    hdma_usart2_tx.Init.Mode                = DMA_NORMAL;
    hdma_usart2_tx.Init.Priority            = DMA_PRIORITY_LOW;
    if (HAL_DMA_Init(&hdma_usart2_tx) != HAL_OK)
    {
      Error_Handler();
    }
    __HAL_LINKDMA(huart, hdmatx, hdma_usart2_tx);

    /* 4. Cấu hình Ngắt (NVIC) */
    HAL_NVIC_SetPriority(USARTx_DMA_TX_IRQn, USARTx_Priority, 1);
    HAL_NVIC_EnableIRQ(USARTx_DMA_TX_IRQn);

    HAL_NVIC_SetPriority(USARTx_IRQn, USARTx_DMA_Priority, 1);
    HAL_NVIC_EnableIRQ(USARTx_IRQn);
  }
}
/**
  * @brief Hàm dọn dẹp phần cứng cấp thấp cho TẤT CẢ các UART.
  */
void HAL_UART_MspDeInit(UART_HandleTypeDef *uartHandle)
{
  // --- DỌN DẸP CHO USART1 ---
  if (uartHandle->Instance == USART1)
  {
    __HAL_RCC_USART1_CLK_DISABLE();
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9 | GPIO_PIN_10);
    HAL_DMA_DeInit(uartHandle->hdmarx);
    HAL_NVIC_DisableIRQ(DMA1_Channel2_3_IRQn); // Kênh có thể khác cho F1
    HAL_NVIC_DisableIRQ(USART1_IRQn);
  }
  // --- DỌN DẸP CHO USART2 ---
  else if (uartHandle->Instance == USART2)
  {
    __HAL_RCC_USART2_CLK_DISABLE();
    HAL_GPIO_DeInit(GPIOA, USARTx_RX_Pin | USARTx_TX_Pin);
    HAL_DMA_DeInit(uartHandle->hdmatx);
    HAL_NVIC_DisableIRQ(USART2_IRQn);
    HAL_NVIC_DisableIRQ(USARTx_DMA_TX_IRQn); // Bổ sung
  }
}
