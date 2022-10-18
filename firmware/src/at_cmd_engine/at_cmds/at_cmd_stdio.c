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
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "include/at_cmds.h"
#include "platform/platform.h"
#include "terminal/terminal.h"
#include "at_cmds/at_cmd_inet.h"
#include "at_cmd_app.h"

static bool isAECOutput;
static bool isAECLineClean;
static int statusVebosityLevel;

static const char* statusCodeStr[] = {
    "OK",                               // ATCMD_STATUS_OK
    "General Error",                    // ATCMD_STATUS_ERROR
    "Invalid AT Command",               // ATCMD_STATUS_INVALID_CMD
    "Unknown AT Command",               // ATCMD_STATUS_UNKNOWN_CMD
    "Invalid Parameter",                // ATCMD_STATUS_INVALID_PARAMETER
    "Incorrect Number of Parameters",   // ATCMD_STATUS_INCORRECT_NUM_PARAMS
    "Configuration Update Blocked",     // ATCMD_STATUS_STORE_UPDATE_BLOCKED,
    "Configuration Access Failed"       // ATCMD_STATUS_STORE_ACCESS_FAILED,
};

static void ByteToStr(uint8_t byte, char *pStr)
{
    char nibble;

    nibble = (byte >> 4) & 0x0f;

    if (nibble <= 9)
    {
        *pStr++ = '0' + nibble;
    }
    else
    {
        *pStr++ = 'A' + nibble - 10;
    }

    nibble = byte & 0x0f;

    if (nibble <= 9)
    {
        *pStr = '0' + nibble;
    }
    else
    {
        *pStr = 'A' + nibble - 10;
    }
}

/*****************************************************************************
 * Enters AEC output mode
 *****************************************************************************/
void ATCMD_EnterAECMode(void)
{
    isAECOutput = true;
    isAECLineClean = true;
}

/*****************************************************************************
 * Leaves AEC output mode
 *****************************************************************************/
void ATCMD_LeaveAECMode(void)
{
    isAECOutput = false;
}

extern ATCMD_APP_CONTEXT atCmdAppContext;
extern int8_t response_buffer[1024];
extern int32_t response_buffer_length;

/*****************************************************************************
 * Print to output channel
 *****************************************************************************/
void ATCMD_Print(const char *pMsg, size_t msgLength)
{
    if (0 == msgLength)
    {
        return;
    }

    if ((true == isAECOutput) && (true == isAECLineClean))
    {
        /* If sending AEC, if no other AEC output has been sent on this line
           then send a CR first to wipe out the prompt */

        ATCMD_PlatformUARTWritePutByte('\r');
    }

    ATCMD_PlatformUARTWritePutBuffer(pMsg, msgLength);

    if(atCmdAppContext.respond_to_app == 1)
    {
        /*
         * Do not write into the response buffer if response_buffer_length
         * is bigger than response buffer.
         */
        if(response_buffer_length + msgLength <= sizeof(response_buffer))
        {
            memcpy(response_buffer + response_buffer_length, pMsg, msgLength);
            response_buffer_length += msgLength;
        }
    }

    if (true == isAECOutput)
    {
        /* If this AEC output would complete a line (terminates with LF) then
           generate a new prompt for user input. If no LF then make the line
           as incomplete so next time doesn't try and wipe out the prompt above */

        if ('\n' == pMsg[msgLength-1])
        {
#ifdef AT_CMD_CONF_CMD_MODE_USE_PROMPT
            if (true == TP_EchoGet())
            {
                ATCMD_PlatformUARTWritePutByte(AT_CMD_CONF_CMD_MODE_PROMPT_CHAR);
            }
#endif
            isAECLineClean = true;
        }
        else
        {
            isAECLineClean = false;
        }
    }
}

/*****************************************************************************
 * Print to output channel with formatting support
 *****************************************************************************/
void ATCMD_Printf(const char *format, ...)
{
    char tmpBuf[AT_CMD_CONF_PRINTF_OUT_BUF_SIZE];
    int len = 0;
    va_list args;
    va_start( args, format );

    len = vsnprintf(tmpBuf, AT_CMD_CONF_PRINTF_OUT_BUF_SIZE, format, args);

    va_end( args );

    ATCMD_Print(tmpBuf, len);
}

/*****************************************************************************
 * Print to output channel at formatted MAC address
 *****************************************************************************/
void ATCMD_PrintMACAddress(const uint8_t *pMACAddr)
{
    char tmpBuf[20];
    int len = 0;
    int i;

    tmpBuf[len++] = '"';

    /* Produce format "aa:bb:cc:dd:ee:ff" */

    for (i=0; i<6; i++)
    {
        ByteToStr(*pMACAddr++, &tmpBuf[len]);
        len += 2;
        if (i < 5)
        {
            tmpBuf[len++] = ':';
        }
    }

    tmpBuf[len++] = '"';

    ATCMD_Print(tmpBuf, len);
}

/*****************************************************************************
 * Print to output channel at formatted IPv4 address
 *****************************************************************************/
void ATCMD_PrintIPv4Address(const uint32_t ipv4Addr)
{
    char tmpBuf[20];
    int len = 0;

    tmpBuf[0] = '"';

    /* Produce format "a.b.c.d" */

    if (NULL == at_cmd_inet_ntop(0, &ipv4Addr, &tmpBuf[1], sizeof(tmpBuf)-1))
    {
        return;
    }

    len = strlen(tmpBuf);

    tmpBuf[len++] = '"';

    ATCMD_Print(tmpBuf, len);
}

/*****************************************************************************
 * Print to output channel a string with escaped control characters
 *****************************************************************************/
void ATCMD_PrintStringASCIIEsc(const char *pStr, size_t strLength)
{
    char tmpBuf[AT_CMD_CONF_PRINTF_OUT_BUF_SIZE];
    int len = 0;

    tmpBuf[len++] = '"';

    /* Produce format "..." with displayable characters and known escape sequences */

    while (strLength--)
    {
        if ((*pStr >= 0x20) && (*pStr <= 0x7e))
        {
            /* Displayable characters go straight through, except \ and " which
               must be escaped with \ */

            if (('\\' == *pStr) || ('"' == *pStr))
            {
                tmpBuf[len++] = '\\';
            }

            tmpBuf[len++] = *pStr++;
        }
        else
        {
            tmpBuf[len++] = '\\';

            if (0x00 == *pStr)
            {
                /* Just in case we've been asked to display a string not using
                   null termination, display \0 */

                tmpBuf[len++] = '0';
            }
            else if ((*pStr >= 0x07) && (*pStr <= 0x0d))
            {
                /* These escape codes are grouped nicely together, so use a
                   lookup table to translate */

                tmpBuf[len++] = "abtnvfr"[*pStr - 0x07];
            }
            else if (0x1b == *pStr)
            {
                /* Escape is off on it's own, produce \e */

                tmpBuf[len++] = 'e';
            }
            else
            {
                /* Just in case we get an invalid character, backtrack
                   to remove the '\' */
                len--;
            }

            pStr++;
        }
    }

    tmpBuf[len++] = '"';

    ATCMD_Print(tmpBuf, len);
}


/*****************************************************************************
 * Print to output channel a string as a series of hex bytes
 *****************************************************************************/
void ATCMD_PrintStringHex(const uint8_t *pBytes, size_t strLength)
{
    char tmpBuf[AT_CMD_CONF_PRINTF_OUT_BUF_SIZE];
    int len = 0;

    tmpBuf[len++] = '[';

    /* Produce the format [....] containing pairs of characters, each representing
       a single byte */

    while (strLength--)
    {
        ByteToStr(*pBytes++, &tmpBuf[len]);
        len += 2;
    }

    tmpBuf[len++] = ']';

    ATCMD_Print(tmpBuf, len);
}

/*****************************************************************************
 * Print to the output channel a string, in the most appropriate form possible
 *****************************************************************************/
void ATCMD_PrintStringSafe(const char *pStr, size_t strLength)
{
    size_t i;
    bool containsEscapableASCII = false;

    if (0 == strLength)
    {
        ATCMD_Print("[]", 2);
        return;
    }

    for (i=0; i<strLength; i++)
    {
        if ((pStr[i] >= 0x20) && (pStr[i] <= 0x7e))
        {
            /* ASCII, only displayable characters */
        }
        else if (pStr[i] >= 128)
        {
            /* UTF-8 and other unknown encodings as well as plain binary data.
               No other output format supported so go straight to hex display */

            ATCMD_PrintStringHex((uint8_t*)pStr, strLength);
            return;
        }
        else if ((0x00 == pStr[i]) || ((pStr[i] >= 0x07) && (pStr[i] <= 0x0d)) || (0x1b == pStr[i]))
        {
            /* Contains escaped ASCII */

            containsEscapableASCII = true;
        }
        else
        {
            /* Non displayable ASCII or non escaped ASCII characters, go
               straight to hex display */

            ATCMD_PrintStringHex((uint8_t*)pStr, strLength);
            return;
        }
    }

    if (true == containsEscapableASCII)
    {
        ATCMD_PrintStringASCIIEsc(pStr, strLength);
        return;
    }

    ATCMD_Print("\"", 1);
    ATCMD_Print(pStr, strLength);
    ATCMD_Print("\"", 1);
}

/*****************************************************************************
 * Print to output channel a string with escaped control characters
 *****************************************************************************/
void ATCMD_PrintStringASCIIEscWithDelimiterInfo(const char *pStr, size_t strLength, bool startDelimiter, bool endDelimiter)
{
    char tmpBuf[AT_CMD_CONF_PRINTF_OUT_BUF_SIZE];
    int len = 0;

    if(startDelimiter == true)
        tmpBuf[len++] = '"';

    /* Produce format "..." with displayable characters and known escape sequences */

    while (strLength--)
    {
        if ((*pStr >= 0x20) && (*pStr <= 0x7e))
        {
            /* Displayable characters go straight through, except \ and " which
               must be escaped with \ */

            if (('\\' == *pStr) || ('"' == *pStr))
            {
                tmpBuf[len++] = '\\';
            }

            tmpBuf[len++] = *pStr++;
        }
        else
        {
            tmpBuf[len++] = '\\';

            if (0x00 == *pStr)
            {
                /* Just in case we've been asked to display a string not using
                   null termination, display \0 */

                tmpBuf[len++] = '0';
            }
            else if ((*pStr >= 0x07) && (*pStr <= 0x0d))
            {
                /* These escape codes are grouped nicely together, so use a
                   lookup table to translate */

                tmpBuf[len++] = "abtnvfr"[*pStr - 0x07];
            }
            else if (0x1b == *pStr)
            {
                /* Escape is off on it's own, produce \e */

                tmpBuf[len++] = 'e';
            }
            else
            {
                /* Just in case we get an invalid character, backtrack
                   to remove the '\' */
                len--;
            }

            pStr++;
        }
    }

    if(endDelimiter == true)
        tmpBuf[len++] = '"';

    ATCMD_Print(tmpBuf, len);
}


/*****************************************************************************
 * Print to output channel a string as a series of hex bytes
 *****************************************************************************/
void ATCMD_PrintStringHexWithDelimiterInfo(const uint8_t *pBytes, size_t strLength, bool startDelimiter, bool endDelimiter)
{
    char tmpBuf[AT_CMD_CONF_PRINTF_OUT_BUF_SIZE];
    int len = 0;

    if(startDelimiter == true)
        tmpBuf[len++] = '[';

    /* Produce the format [....] containing pairs of characters, each representing
       a single byte */

    while (strLength--)
    {
        ByteToStr(*pBytes++, &tmpBuf[len]);
        len += 2;
    }

    if(endDelimiter == true)
        tmpBuf[len++] = ']';

    ATCMD_Print(tmpBuf, len);
}


/*****************************************************************************
 * Print to the output channel a string, in the most appropriate form possible
 *****************************************************************************/
void ATCMD_PrintStringSafeWithDelimiterInfo(const char *pStr, size_t strLength, bool startDelimiter, bool endDelimiter)
{
    size_t i;
    bool containsEscapableASCII = false;

    if (0 == strLength)
    {
        ATCMD_Print("[]", 2);
        return;
    }

    for (i=0; i<strLength; i++)
    {
        if ((pStr[i] >= 0x20) && (pStr[i] <= 0x7e))
        {
            /* ASCII, only displayable characters */
        }
        else if (pStr[i] >= 128)
        {
            /* UTF-8 and other unknown encodings as well as plain binary data.
               No other output format supported so go straight to hex display */

            ATCMD_PrintStringHexWithDelimiterInfo((uint8_t*)pStr, strLength, startDelimiter, endDelimiter);
            return;
        }
        else if ((0x00 == pStr[i]) || ((pStr[i] >= 0x07) && (pStr[i] <= 0x0d)) || (0x1b == pStr[i]))
        {
            /* Contains escaped ASCII */

            containsEscapableASCII = true;
        }
        else
        {
            /* Non displayable ASCII or non escaped ASCII characters, go
               straight to hex display */

            ATCMD_PrintStringHexWithDelimiterInfo((uint8_t*)pStr, strLength, startDelimiter, endDelimiter);
            return;
        }
    }

    if (true == containsEscapableASCII)
    {
        ATCMD_PrintStringASCIIEscWithDelimiterInfo(pStr, strLength, startDelimiter, endDelimiter);
        return;
    }

    if(startDelimiter == true)
        ATCMD_Print("\"", 1);
    
    ATCMD_Print(pStr, strLength);
    
    if(endDelimiter == true)
        ATCMD_Print("\"", 1);
}


/*****************************************************************************
 * Sets the level of verbosity for status output
 *****************************************************************************/
void ATCMD_SetStatusVerbosityLevel(int newLevel)
{
    statusVebosityLevel = newLevel;
}

/*****************************************************************************
 * Display a status report
 *****************************************************************************/
void ATCMD_ReportStatus(const ATCMD_STATUS statusCode)
{
    const char *pStatusMsg = NULL;

    /*
        Level 0:        0           1
        Level 1:        0           1:statusCode
        Level 2:        OK          ERROR
        Level 3:        OK          ERROR:statusCode
        Level 4:        OK          ERROR:statusMsg (or ERROR:statusCode if no message)
        Level 5:        OK          ERROR:statusCode, statusMsg (or ERROR:statusCode if no message)
    */

    if (statusVebosityLevel < 2)
    {
        if (ATCMD_STATUS_OK == statusCode)
        {
            ATCMD_Printf("0\r\n");
        }
        else
        {
            if (0 == statusVebosityLevel)
            {
                ATCMD_Printf("1\r\n");
            }
            else
            {
                ATCMD_Printf("1:%d\r\n", statusCode);
            }
        }
    }
    else
    {
        if (ATCMD_STATUS_OK == statusCode)
        {
            ATCMD_Printf(AT_CMD_CONF_AT_OK_STRING "\r\n");
        }
        else
        {
            if (2 == statusVebosityLevel)
            {
                ATCMD_Printf(AT_CMD_CONF_AT_ERROR_BASE_STRING "\r\n");
            }
            else if (3 == statusVebosityLevel)
            {
                ATCMD_Printf(AT_CMD_CONF_AT_ERROR_BASE_STRING ":%d\r\n", statusCode);
            }
            else
            {
                if (statusCode < ATCMD_STATUS_CUSTOM_MSG_BASE)
                {
                    pStatusMsg = statusCodeStr[statusCode];
                }
                else
                {
                    pStatusMsg = ATCMD_APPTranslateStatusCode(statusCode);
                }

                if (NULL != pStatusMsg)
                {
                    if (4 == statusVebosityLevel)
                    {
                        ATCMD_Printf(AT_CMD_CONF_AT_ERROR_BASE_STRING ":\"%s\"\r\n", pStatusMsg);
                    }
                    else
                    {
                        ATCMD_Printf(AT_CMD_CONF_AT_ERROR_BASE_STRING ":%d,\"%s\"\r\n", statusCode, pStatusMsg);
                    }
                }
                else
                {
                    ATCMD_Printf(AT_CMD_CONF_AT_ERROR_BASE_STRING ":%d\r\n", statusCode);
                }
            }
        }
    }
}

/*****************************************************************************
 * Display an AEC status report
 *****************************************************************************/
void ATCMD_ReportAECStatus(const char *pCmdName, const ATCMD_STATUS statusCode)
{
    if (NULL != pCmdName)
    {
        ATCMD_Printf("\r%s:", pCmdName);
    }

    ATCMD_ReportStatus(statusCode);
}
