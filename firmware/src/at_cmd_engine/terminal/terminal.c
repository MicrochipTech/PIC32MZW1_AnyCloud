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
#include <stddef.h>
#include <string.h>

#include "platform/platform.h"

#include "terminal.h"
#include "ansi_decoder.h"
#include "system/console/sys_console.h"

#ifdef TP_HISTORY_ENABLED
#define TP_HISTORY_NUM_INDEXS                   AT_CMD_CONF_TERM_CMD_HISTORY_DEPTH
#define TP_HISTORY_BUFFER_SIZE                  (AT_CMD_CONF_MAX_COMMAND_LENGTH * (AT_CMD_CONF_TERM_CMD_HISTORY_DEPTH/2))

typedef struct
{
    char    *pCmdStart;
    int     cmdLength1;
    int     cmdLength2;
} TP_LINE_BUF;

typedef struct
{
    int             firstLineIdx;
    int             lastLineIdx;
    int             readLineIdx;

    TP_LINE_BUF     index[TP_HISTORY_NUM_INDEXS];

    int             lineBufInOffset;
    int             lineBufRemain;
    char            lineBuf[TP_HISTORY_BUFFER_SIZE];
} TP_HISTORY;

static TP_HISTORY tpLineHistory;
#endif

static ANSI_STREAM_DECODER_STATE ansiStreamState;
static char *pCmdBuffer;
static size_t cmdBufferLength;
static size_t cmdBufferMaxLength;
static size_t cmdBufferInsertPos;
static bool serialEcho = false;
#if AT_CMD_CONF_TP_ESC_TIMEOUT_MS > 0
static uint32_t escTime;
#endif

#if defined(TP_HISTORY_ENABLED) || (AT_CMD_CONF_TP_ESC_TIMEOUT_MS > 0)
#ifdef AT_CMD_CONF_CMD_MODE_USE_PROMPT
static const char clearLine[] = {0x1b, '[', '2', 'K', '\r', '\0', AT_CMD_CONF_CMD_MODE_PROMPT_CHAR};
#define TP_CLEAR_LINE_SEQ_LEN   (sizeof(clearLine)-1)
#else
static const char clearLine[] = {0x1b, '[', '2', 'K', '\r', '\0'};
#define TP_CLEAR_LINE_SEQ_LEN   sizeof(clearLine)
#endif
#endif /* defined(TP_HISTORY_ENABLED) || (AT_CMD_CONF_TP_ESC_TIMEOUT_MS > 0) */

#ifdef TP_HISTORY_ENABLED
/*****************************************************************************
 * Replace command line buffer with one from the history
 *****************************************************************************/
static void _ReplaceCommandLine(int histIdx)
{
    const TP_LINE_BUF *const pLineBuf = &tpLineHistory.index[histIdx];

    /* Send the sequence to clear the terminal line ready for a new command */

    ATCMD_PlatformUARTWritePutBuffer(clearLine, sizeof(clearLine));

    /* Copy the stored line buffer to the current command buffer, if it wraps copy the
       second part as well. */

    memcpy(pCmdBuffer, pLineBuf->pCmdStart, pLineBuf->cmdLength1);
    cmdBufferLength = pLineBuf->cmdLength1;

    if (pLineBuf->cmdLength2 > 0)
    {
        memcpy(&pCmdBuffer[pLineBuf->cmdLength1], tpLineHistory.lineBuf, pLineBuf->cmdLength2);
        cmdBufferLength += pLineBuf->cmdLength2;
    }

    /* Ensure the insertion point is set up to match */

    cmdBufferInsertPos = cmdBufferLength;

    /* Out the stored line buffer to the serial */

    ATCMD_PlatformUARTWritePutBuffer(pCmdBuffer, cmdBufferLength);
}
#endif

/*****************************************************************************
 * Initialised terminal parser
 *****************************************************************************/
void TP_CommandDecoderInit(void)
{
    pCmdBuffer = NULL;
    serialEcho = false;

#ifdef TP_HISTORY_ENABLED
    memset(&tpLineHistory, 0, sizeof(TP_HISTORY));
    tpLineHistory.lineBufRemain = TP_HISTORY_BUFFER_SIZE;
    tpLineHistory.lastLineIdx   = -1;
    tpLineHistory.firstLineIdx  = -1;
    tpLineHistory.readLineIdx   = -1;
#endif
}

/*****************************************************************************
 * Set the serial echo flag
 *****************************************************************************/
bool TP_EchoSet(bool newEcho)
{
    bool oldEcho;

    oldEcho = serialEcho;

    serialEcho = newEcho;

    return oldEcho;
}

/*****************************************************************************
 * Get the serial echo flag
 *****************************************************************************/
bool TP_EchoGet(void)
{
    return serialEcho;
}

/*****************************************************************************
 * Reset parser to start of new line
 *****************************************************************************/
void TP_CommandDecoderStartNewLine(char *pConsoleCmdBuffer, size_t cmdBufferSize)
{
	return;
	
    TP_ANSIStreamDecoderInit(&ansiStreamState);

    pCmdBuffer          = pConsoleCmdBuffer;
    cmdBufferLength     = 0;
    cmdBufferMaxLength  = cmdBufferSize;
    cmdBufferInsertPos  = 0;
#if AT_CMD_CONF_TP_ESC_TIMEOUT_MS > 0
    escTime             = 0;
#endif

    pCmdBuffer[0]       = '\0';

#ifdef TP_HISTORY_ENABLED
    tpLineHistory.readLineIdx = -1;
#endif

#ifdef AT_CMD_CONF_CMD_MODE_USE_PROMPT
    if (true == serialEcho)
    {
        ATCMD_PlatformUARTWritePutByte(AT_CMD_CONF_CMD_MODE_PROMPT_CHAR);
    }
#endif
}

/*****************************************************************************
 * Process incoming serial stream
 *****************************************************************************/
TERM_DECODE_EVENT TP_CommandDecoderProcess(void)
{
    int decodeLength;
    char newC;
    size_t numUartBytes;

    if (NULL == pCmdBuffer)
    {
        return TERM_DECODE_EVENT_ERROR;
    }

#if AT_CMD_CONF_TP_ESC_TIMEOUT_MS > 0
    if ((true == serialEcho) && (0 != escTime))
    {
        if ((ATCMD_PlatformGetSysTimeMs() - escTime) > AT_CMD_CONF_TP_ESC_TIMEOUT_MS)
        {
            ATCMD_PlatformUARTWritePutBuffer(clearLine, TP_CLEAR_LINE_SEQ_LEN);

            TP_CommandDecoderStartNewLine(pCmdBuffer, cmdBufferMaxLength);
        }
    }
#endif

    numUartBytes = ATCMD_PlatformUARTReadGetCount();

    if (0 == numUartBytes)
    {
        return TERM_DECODE_EVENT_NONE;
    }

    while (numUartBytes--)
    {
#if AT_CMD_CONF_TP_ESC_TIMEOUT_MS > 0
        escTime = 0;
#endif
        newC = ATCMD_PlatformUARTReadGetByte();

        /* Pass new character through stream decoder. */

        decodeLength = TP_ANSIStreamDecoder(&ansiStreamState, newC, &pCmdBuffer[cmdBufferInsertPos], cmdBufferMaxLength - cmdBufferInsertPos);

        /* Stream decoder returns the number of characters placed into
            the command buffer which maybe more than one. */
		
        if (decodeLength > 0)
        {
            if (true == serialEcho)
            {
                ATCMD_PlatformUARTWritePutBuffer(&pCmdBuffer[cmdBufferInsertPos], decodeLength);
            }

            /* Push the insert position up and increase the length of
                the command buffer contents if the position goes past the end. */

            cmdBufferInsertPos += decodeLength;

            if (cmdBufferInsertPos >= cmdBufferLength)
            {
                cmdBufferLength = cmdBufferInsertPos;
            }
        }
        else if (decodeLength < 0)
        {
            return TERM_DECODE_EVENT_ERROR;
        }
        else
        {
            ANSI_STREAM_DECISION streamDecision;

            streamDecision = TP_ANSIStreamDecoderDecision(&ansiStreamState);

            switch(streamDecision)
            {
                case ANSI_STREAM_DECISION_CONTROL_BS:
                {
                    static const char backSpace[6] = {0x1b, '[', 'D', 0x1b, '[', 'K'};

                    if (cmdBufferInsertPos > 0)
                    {
                        if (true == serialEcho)
                        {
                            ATCMD_PlatformUARTWritePutBuffer(backSpace, 6);
                        }

                        if (cmdBufferLength == cmdBufferInsertPos)
                        {
                            cmdBufferLength--;
                        }

                        cmdBufferInsertPos--;
                    }

                    break;
                }

                case ANSI_STREAM_DECISION_CONTROL_LF:
                {
                    if (true == serialEcho)
                    {
                        ATCMD_PlatformUARTWritePutByte('\n');
                    }

                    pCmdBuffer[cmdBufferLength] = '\0';

                    if (cmdBufferLength > 0)
                    {
#ifdef TP_HISTORY_ENABLED
                        int newCmdIdx;
                        int firstCopyLen;
                        TP_LINE_BUF *pNewLineBuf;

                        /* Find new line buffer in history context */

                        newCmdIdx = tpLineHistory.lastLineIdx + 1;
                        if (newCmdIdx == TP_HISTORY_NUM_INDEXS)
                        {
                            newCmdIdx = 0;
                        }

                        pNewLineBuf = &tpLineHistory.index[newCmdIdx];

                        while ((cmdBufferLength > (size_t)tpLineHistory.lineBufRemain) || (newCmdIdx == tpLineHistory.firstLineIdx))
                        {
                            /* Free space in history context. If remaining space in the buffer is not sufficient
                               then we free the oldest entry and try again. If all the line buffers are in use
                               we free the oldest one. */

                            TP_LINE_BUF *const pFirstLineBuf = &tpLineHistory.index[tpLineHistory.firstLineIdx];

                            /* Return the used space to the pool */

                            tpLineHistory.lineBufRemain += pFirstLineBuf->cmdLength1;
                            tpLineHistory.lineBufRemain += pFirstLineBuf->cmdLength2;

                            /* Clear the old line buffer structure */

                            pFirstLineBuf->pCmdStart  = NULL;
                            pFirstLineBuf->cmdLength1 = 0;
                            pFirstLineBuf->cmdLength2 = 0;

                            /* Move first line index forward, but only until we reach the last line index */

                            if (tpLineHistory.firstLineIdx != tpLineHistory.lastLineIdx)
                            {
                                tpLineHistory.firstLineIdx++;
                                if (tpLineHistory.firstLineIdx == TP_HISTORY_NUM_INDEXS)
                                {
                                    tpLineHistory.firstLineIdx = 0;
                                }
                            }
                            else
                            {
                                tpLineHistory.firstLineIdx = -1;
                            }
                        }

                        /* Record the start of the new line buffer within the pool buffer */

                        pNewLineBuf->pCmdStart = &tpLineHistory.lineBuf[tpLineHistory.lineBufInOffset];

                        /* Copy current command buffer into pool buffer taking care to wrap */

                        firstCopyLen = TP_HISTORY_BUFFER_SIZE - tpLineHistory.lineBufInOffset;

                        if (firstCopyLen >= (int)cmdBufferLength)
                        {
                            memcpy(pNewLineBuf->pCmdStart, pCmdBuffer, cmdBufferLength);
                            pNewLineBuf->cmdLength1 = cmdBufferLength;
                            pNewLineBuf->cmdLength2 = 0;
                        }
                        else
                        {
                            memcpy(pNewLineBuf->pCmdStart, pCmdBuffer, firstCopyLen);
                            memcpy(tpLineHistory.lineBuf, &pCmdBuffer[firstCopyLen], cmdBufferLength-firstCopyLen);
                            pNewLineBuf->cmdLength1 = firstCopyLen;
                            pNewLineBuf->cmdLength2 = cmdBufferLength-firstCopyLen;
                        }

                        /* Move up insertion point, wrapping if needed */

                        tpLineHistory.lineBufInOffset += cmdBufferLength;
                        if (tpLineHistory.lineBufInOffset >= TP_HISTORY_BUFFER_SIZE)
                        {
                            tpLineHistory.lineBufInOffset -= TP_HISTORY_BUFFER_SIZE;
                        }

                        tpLineHistory.lineBufRemain -= cmdBufferLength;

                        /* New line buffer becomes the pools last and we clear the read index
                           for any recall later */

                        tpLineHistory.lastLineIdx = newCmdIdx;
                        tpLineHistory.readLineIdx = -1;

                        /* If the first line index became unset above (or first time through) set
                           it to the current index as there is only one line buffer in use now */

                        if (-1 == tpLineHistory.firstLineIdx)
                        {
                            tpLineHistory.firstLineIdx = newCmdIdx;
                        }
#endif
                        return TERM_DECODE_EVENT_END_OF_LINE;
                    }

#ifdef AT_CMD_CONF_CMD_MODE_USE_PROMPT
                    if (true == serialEcho)
                    {
                        ATCMD_PlatformUARTWritePutByte(AT_CMD_CONF_CMD_MODE_PROMPT_CHAR);
                    }
#endif
                    break;
                }

                case ANSI_STREAM_DECISION_CONTROL_CR:
                {
                    if (true == serialEcho)
                    {
                        ATCMD_PlatformUARTWritePutByte('\r');
                    }

                    cmdBufferInsertPos = 0;
                    break;
                }

                case ANSI_STREAM_DECISION_CONTROL_ESC:
                {
#if AT_CMD_CONF_TP_ESC_TIMEOUT_MS > 0
                    escTime = ATCMD_PlatformGetSysTimeMs();
                    if (0 == escTime)
                    {
                        escTime--;
                    }
#endif
                    break;
                }

#ifdef TP_HISTORY_ENABLED
                case ANSI_STREAM_DECISION_ESCAPE_CSI_KEYUP:
                {
                    if (false == serialEcho)
                    {
                        break;
                    }

                    /* If read index is unset we have a new recall request, so set the read index
                       to the last line index used. If the read index isn't all the way back at the
                       first index allow it to move back by one entry. */

                    if (tpLineHistory.readLineIdx < 0)
                    {
                        tpLineHistory.readLineIdx = tpLineHistory.lastLineIdx;
                    }
                    else
                    {
                        if (tpLineHistory.readLineIdx != tpLineHistory.firstLineIdx)
                        {
                            tpLineHistory.readLineIdx--;
                            if (tpLineHistory.readLineIdx < 0)
                            {
                                tpLineHistory.readLineIdx += TP_HISTORY_NUM_INDEXS;
                            }
                        }
                    }

                    if (tpLineHistory.readLineIdx >= 0)
                    {
                        /* Replace the current command line with the one from the history */
                        _ReplaceCommandLine(tpLineHistory.readLineIdx);
                    }

                    break;
                }

                case ANSI_STREAM_DECISION_ESCAPE_CSI_KEYDOWN:
                {
                    if (false == serialEcho)
                    {
                        break;
                    }

                    /* Down is only allowed when up has previously been used, so read index
                       must be set to do anything. Move the read index forward until it
                       reaches the last line index. */

                    if (tpLineHistory.readLineIdx >= 0)
                    {
                        if (tpLineHistory.readLineIdx != tpLineHistory.lastLineIdx)
                        {
                            tpLineHistory.readLineIdx++;
                            if (tpLineHistory.readLineIdx >= TP_HISTORY_NUM_INDEXS)
                            {
                                tpLineHistory.readLineIdx -= TP_HISTORY_NUM_INDEXS;
                            }
                        }

                        /* Replace the current command line with the one from the history */
                        _ReplaceCommandLine(tpLineHistory.readLineIdx);
                    }

                    break;
                }
#endif

                default:
                {
                    break;
                }
            }
        }
    }

    return TERM_DECODE_EVENT_NONE;
}
