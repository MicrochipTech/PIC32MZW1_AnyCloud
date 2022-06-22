/**
 *
 * Copyright (c) 2019 Microchip Technology Inc. and its subsidiaries.
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

#ifndef _PLATFORM_H
#define _PLATFORM_H

#include "conf_at_cmd.h"

#ifdef __cplusplus  // Provide C++ Compatibility
extern "C" {
#endif

void ATCMD_PlatformInit(void);
void ATCMD_PlatformUARTSetBaudRate(uint32_t baud);
uint32_t ATCMD_PlatformUARTGetBaudRate(void);
size_t ATCMD_PlatformUARTReadGetCount(void);
uint8_t ATCMD_PlatformUARTReadGetByte(void);
size_t ATCMD_PlatformUARTReadGetBuffer(void *pBuf, size_t numBytes);
size_t ATCMD_PlatformUARTWriteGetSpace(void);
bool ATCMD_PlatformUARTWritePutByte(uint8_t b);
bool ATCMD_PlatformUARTWritePutBuffer(const void *pBuf, size_t numBytes);
size_t ATCMD_PlatformDebugUARTWriteGetSpace(void);
bool ATCMD_PlatformDebugUARTWritePutByte(uint8_t b);
bool ATCMD_PlatformDebugUARTWritePutBuffer(const void *pBuf, size_t numBytes);
bool ATCMD_PlatformDebugUARTWriteBufferFlush(bool inCriticalSection);
uint32_t ATCMD_PlatformGetSysTimeMs(void);

#ifdef __cplusplus
}
#endif

#endif /* _PLATFORM_H */
