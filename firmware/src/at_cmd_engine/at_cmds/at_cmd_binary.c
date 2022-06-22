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

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "include/at_cmds.h"
#include "platform/platform.h"

static bool binaryMode;
static int escapeCharCnt;
static uint32_t escapeCharLastTimeMs;
static tpfATCMDBinaryDataHandler pfBinaryDataHandler;
static uint8_t escapeSeq[AT_CMD_CONF_BIN_ESCAPE_SEQ_LENGTH];

/*****************************************************************************
 * Initialise binary mode
 *****************************************************************************/
void ATCMD_BinaryInit(void)
{
    pfBinaryDataHandler = NULL;
    binaryMode          = false;

    memset(escapeSeq, AT_CMD_CONF_BIN_ESCAPE_CHAR, AT_CMD_CONF_BIN_ESCAPE_SEQ_LENGTH);
}

/*****************************************************************************
 * Change input mode to binary mode
 *****************************************************************************/
void ATCMD_EnterBinaryMode(tpfATCMDBinaryDataHandler pBinDataHandler)
{
#ifdef AT_CMD_CONF_BIN_MODE_USE_PROMPT
    if (false == binaryMode)
        ATCMD_PlatformUARTWritePutByte(AT_CMD_CONF_BIN_MODE_PROMPT_CHAR);
#endif

    binaryMode          = true;
    escapeCharCnt       = 0;
    pfBinaryDataHandler = pBinDataHandler;
}

/*****************************************************************************
 * Change the input mode back to binary mode
 *****************************************************************************/
void ATCMD_LeaveBinaryMode(void)
{
    binaryMode          = false;
    pfBinaryDataHandler = NULL;
}

/*****************************************************************************
 * Return if application is currently in binary mode
 *****************************************************************************/
bool ATCMD_ModeIsBinary(void)
{
    return binaryMode;
}

/*****************************************************************************
 * Process UART data in binary mode
 *****************************************************************************/
bool ATCMD_BinaryProcess(void)
{
    uint32_t curTimeMs;
    size_t numBytes;

    curTimeMs = ATCMD_PlatformGetSysTimeMs();

    numBytes = ATCMD_PlatformUARTReadGetCount();

#if AT_CMD_CONF_BIN_FAST_BUFFER_SIZE > 0
    if (numBytes > AT_CMD_CONF_BIN_ESCAPE_SEQ_LENGTH)
    {
        /* Deal with all data, except the last 'n' characters
            which might be the escape sequence. */
        numBytes -= AT_CMD_CONF_BIN_ESCAPE_SEQ_LENGTH;

        while (numBytes)
        {
            /* Process data in blocks, ignore escape sequence processing
                as there is obvious data following any sequence which is present. */

            uint8_t binBuffer[AT_CMD_CONF_BIN_FAST_BUFFER_SIZE];
            size_t numBufBytes;

            numBufBytes = sizeof(binBuffer);
            if (numBufBytes > numBytes)
                numBufBytes = numBytes;

            /* Read data from UART buffer and pass to binary data handler. */

            numBufBytes = ATCMD_PlatformUARTReadGetBuffer(binBuffer, numBufBytes);

            if (NULL != pfBinaryDataHandler)
                pfBinaryDataHandler(binBuffer, numBufBytes);

            numBytes -= numBufBytes;
        }

        /* Still need to cope with possible escape sequence. */

        numBytes += AT_CMD_CONF_BIN_ESCAPE_SEQ_LENGTH;
    }
#endif

    while (numBytes--)
    {
        uint8_t newByte;

        newByte = ATCMD_PlatformUARTReadGetByte();

        /* Read each character from UART, look for up to 'n' characters, reject
            any sequence longer than 'n'. */

        if ((AT_CMD_CONF_BIN_ESCAPE_CHAR == newByte) && (escapeCharCnt < AT_CMD_CONF_BIN_ESCAPE_SEQ_LENGTH))
        {
            escapeCharCnt++;
            escapeCharLastTimeMs = curTimeMs;
        }
        else
        {
            escapeCharCnt = 0;
        }

        /* If not in the escape sequence pass the character straight on to
            binary data handler. */

        if ((0 == escapeCharCnt) && (NULL != pfBinaryDataHandler))
            pfBinaryDataHandler(&newByte, 1);
    }

    if ((escapeCharCnt > 0) && (escapeCharCnt < AT_CMD_CONF_BIN_ESCAPE_SEQ_LENGTH))
    {
        /* For a partial sequence check the short timeout, reject sequence if not
            quick enough and pass escape characters on to binary data handler. */

        if ((curTimeMs - escapeCharLastTimeMs) > AT_CMD_CONF_BIN_ESCAPE_INTR_TIMEOUT)
        {
            if (NULL != pfBinaryDataHandler)
                pfBinaryDataHandler(escapeSeq, escapeCharCnt);

            escapeCharCnt = 0;
        }
    }
    else if (escapeCharCnt == AT_CMD_CONF_BIN_ESCAPE_SEQ_LENGTH)
    {
        /* If complete sequence has been seen, check long timeout. Leave binary mode
            if timeout triggers. */

        if ((curTimeMs - escapeCharLastTimeMs) > AT_CMD_CONF_BIN_ESCAPE_FINAL_TIMEOUT)
        {
            ATCMD_PlatformUARTWritePutBuffer("\r\n", 2);

            ATCMD_LeaveBinaryMode();

            ATCMD_ReportStatus(ATCMD_STATUS_OK);

            return false;
        }
    }

    return true;
}
