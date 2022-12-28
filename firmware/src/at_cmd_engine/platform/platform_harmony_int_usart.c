/**
 *
 * Copyright (c) 2021 Microchip Technology Inc. and its subsidiaries.
 *
 * Subject to your compliance with these terms, you may use Microchip
 * software and any derivatives exclusively with Microchip products.
 * It is your responsibility to comply with third party license terms applicable
 * to your use of third party software (including open source software) that
 * may accompany Microchip software.
 *
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES,
 * WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE,
 * INCLUDING ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY,
 * AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT WILL MICROCHIP BE
 * LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, INCIDENTAL OR CONSEQUENTIAL
 * LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND WHATSOEVER RELATED TO THE
 * SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS BEEN ADVISED OF THE
 * POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE FULLEST EXTENT
 * ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN ANY WAY
 * RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
 * THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 */
/*
 * Support and FAQ: visit <a href="https://www.microchip.com/support/">Microchip Support</a>
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "platform/platform.h"

#include "definitions.h"

#include "osal/osal.h"
#include "peripheral/uart/plib_uart_common.h"
#include "peripheral/uart/plib_uart1.h"

static uint32_t volatile timerMS;
#define ATCMD_UART_BAUD_RATE_DEF    230400
uint32_t currBaudRate = ATCMD_UART_BAUD_RATE_DEF;

/* Access semaphore for printing OK and asynch events */
OSAL_SEM_HANDLE_TYPE printEventSemaphore;

static void _timerCallback(uintptr_t context)
{
    timerMS++;
}

void ATCMD_PlatformInit()
{
	SYS_TIME_CallbackRegisterMS(_timerCallback, 0, 1, SYS_TIME_PERIODIC);
}

void ATCMD_PlatformUARTSetBaudRate(uint32_t baud)
{
#if 0
    ATCMD_PLATFORM_USART_STATE *pUsart = _convertIdx(idx);

    if ((NULL == pUsart) || (false == isInit) || (pUsart->setup.baudRate == baud) || (NULL == pUsart->plib))
    {
        return;
    }

    if (OSAL_RESULT_TRUE == OSAL_MUTEX_Lock(&pUsart->writeMutex, OSAL_WAIT_FOREVER))
    {
        while (-1)
        {
            if (false == pUsart->plib->writeIsBusy())
            {
                if (0 == pUsart->transmit.length)
                {
                    break;
                }

                pUsart->plib->write(&pUsart->transmit.pBuffer[pUsart->transmit.outOffset], 1);
            }
        }

        OSAL_MUTEX_Unlock(&pUsart->writeMutex);
    }

    pUsart->plib->initialize();

    pUsart->setup.baudRate = baud;

    pUsart->plib->serialSetup(&pUsart->setup, 0);

    if (pUsart->receive.maxBufSz > 0)
    {
        pUsart->plib->read(&pUsart->receive.pBuffer[pUsart->receive.inOffset], 1);
    }
#else
    UART_SERIAL_SETUP setup;

    memset(&setup, 0, sizeof(UART_SERIAL_SETUP));
    setup.baudRate = baud; 
    setup.dataWidth = UART_DATA_8_BIT;
    setup.parity = UART_PARITY_NONE;
    setup.stopBits = UART_STOP_1_BIT;

    if(UART2_SerialSetup( &setup, 0) == true)
    {
        currBaudRate = baud;        
    }
    
#endif
}

uint32_t ATCMD_PlatformUARTGetBaudRate(void)
{
#if 0
    ATCMD_PLATFORM_USART_STATE *pUsart = _convertIdx(idx);

    if ((NULL == pUsart) || (false == isInit))
    {
        return 0;
    }

    return pUsart->setup.baudRate;
#else
    return currBaudRate;
#endif    
}

size_t ATCMD_PlatformUARTReadGetCount(void)
{
    return UART2_ReadCountGet();
}

uint8_t ATCMD_PlatformUARTReadGetByte(void)
{
    uint8_t byte = 0;

    if (0 == ATCMD_PlatformUARTReadGetBuffer(&byte, 1))
    {
        return 0;
    }

    return byte;
}

size_t ATCMD_PlatformUARTReadGetBuffer(void *pBuf, size_t numBytes)
{
    return UART2_Read(pBuf, numBytes);
}

size_t ATCMD_PlatformUARTWriteGetSpace(void)
{
    return UART2_WriteFreeBufferCountGet();
}

bool ATCMD_PlatformUARTWritePutByte(uint8_t b)
{
    return ATCMD_PlatformUARTWritePutBuffer((void*)&b, 1);
}

bool ATCMD_PlatformUARTWritePutBuffer(const void *pBuf, const size_t numBytes)
{
    if (OSAL_SEM_Pend(&printEventSemaphore, OSAL_WAIT_FOREVER) != OSAL_RESULT_TRUE)
    {
        return false;
    }

//	uint8_t b = '\0';
    UART2_Write((uint8_t *)pBuf, numBytes);
//	UART2_Write(&b, 1);
    

    if (OSAL_SEM_Post(&printEventSemaphore) != OSAL_RESULT_TRUE)
    {
        return false;
    }
    return true;
}

uint32_t ATCMD_PlatformGetSysTimeMs(void)
{
    return timerMS;
}
