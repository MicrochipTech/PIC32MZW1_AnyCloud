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

#ifndef _AT_CMD_XMODEM_H
#define _AT_CMD_XMODEM_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "conf_at_cmd.h"
#include "platform/platform.h"

#ifdef __cplusplus  // Provide C++ Compatibility
extern "C" {
#endif

typedef void (*tpfATCMDXModemDataHandler)(const uint_fast8_t pktNum, const uint8_t *pBuf, size_t numBufBytes);

void ATCMD_XModemInit(void);
bool ATCMD_XModemIsStarted(void);
void ATCMD_XModemStart(bool requireCRCs, tpfATCMDXModemDataHandler pXMDataHandler);
void ATCMD_XModemStop(void);
void ATCMD_XModemProcess(void);

#ifdef __cplusplus
}
#endif

#endif /* _AT_CMD_XMODEM_H */
