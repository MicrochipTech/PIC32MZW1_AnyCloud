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

#include "include/at_cmds.h"
#include "at_cmd_parser.h"
#ifdef AT_CMD_INCLUDE_XMODEM_SUPPORT
#include "at_cmd_xmodem.h"
#endif
#include "platform/platform.h"
#include "terminal/terminal.h"
#include "at_cmd_app.h"

extern const AT_CMD_TYPE_DESC* atCmdTypeDescTable[];
extern const AT_CMD_TYPE_DESC* atCmdTypeDescTableInt[];

static char consoleCmdBuffer[AT_CMD_CONF_MAX_COMMAND_LENGTH];
static uint32_t lastTermPollTimeMs;

const AT_CMD_TYPE_DESC* pCurrentCmdTypeDesc;

static void _ResetCommands(void)
{
    const AT_CMD_TYPE_DESC **pCmdTableEntry;

    ATCMD_SetStatusVerbosityLevel(AT_CMD_CONF_DEFAULT_VERBOSE_LEVEL);
//    TP_EchoSet(true);

    pCmdTableEntry = atCmdTypeDescTableInt;

    while (NULL != *pCmdTableEntry)
    {
        if (NULL != (*pCmdTableEntry)->cmdInit)
        {
            (*pCmdTableEntry)->cmdInit(*pCmdTableEntry);
        }

        pCmdTableEntry++;
    }

    pCmdTableEntry = atCmdTypeDescTable;

    while (NULL != *pCmdTableEntry)
    {
        if (NULL != (*pCmdTableEntry)->cmdInit)
        {
            (*pCmdTableEntry)->cmdInit(*pCmdTableEntry);
        }

        pCmdTableEntry++;
    }
}

/*****************************************************************************
 * Complete an executed command
 *****************************************************************************/
bool ATCMD_CompleteCommand(ATCMD_STATUS status)
{
    pCurrentCmdTypeDesc = NULL;

    if (ATCMD_STATUS_OK != status)
    {
        ATCMD_ReportStatus(status);
        return false;
    }

    if (false == ATCMD_ModeIsBinary())
    {
        ATCMD_ReportStatus(ATCMD_STATUS_OK);
    }

    return true;
}

static bool atcmd_init_done = 0;

/*****************************************************************************
 * Initialise the AT command engine
 *****************************************************************************/
void ATCMD_Init(void)
{
	if (atcmd_init_done == 1)
		return;

	atcmd_init_done = 1;
	
    ATCMD_BinaryInit();

#ifdef AT_CMD_INCLUDE_XMODEM_SUPPORT
    ATCMD_XModemInit();
#endif

    ATCMD_LeaveAECMode();

    ATCMD_APPInit();

    TP_CommandDecoderInit();

	ATCMD_PlatformInit();

    ATCMD_PlatformUARTWritePutBuffer("\r\nOK\r\n\0", 6);
    TP_CommandDecoderStartNewLine(consoleCmdBuffer, AT_CMD_CONF_MAX_COMMAND_LENGTH);

    pCurrentCmdTypeDesc = NULL;

    ATCMD_EnterAECMode();

    _ResetCommands();

    lastTermPollTimeMs = ATCMD_PlatformGetSysTimeMs();
}

/*****************************************************************************
 * Update the AT command state machine
 *****************************************************************************/
void ATCMD_Update(int termPollRateMs)
{
    const AT_CMD_TYPE_DESC **pCmdTableEntry;
    bool inBinaryMode = false;
    ATCMD_STATUS status;

    if ((ATCMD_PlatformGetSysTimeMs() - lastTermPollTimeMs) > termPollRateMs)
    {
        if (true == ATCMD_ModeIsBinary())
        {
            inBinaryMode = true;
            ATCMD_BinaryProcess();
        }
#ifdef AT_CMD_INCLUDE_XMODEM_SUPPORT
        else if (true == ATCMD_XModemIsStarted())
        {
            ATCMD_XModemProcess();
        }
#endif
#if 1
		else
        {
            TERM_DECODE_EVENT termEvent;

            termEvent = TP_CommandDecoderProcess();

            switch(termEvent)
            {
                case TERM_DECODE_EVENT_ERROR:
                {
                    TP_CommandDecoderStartNewLine(consoleCmdBuffer, AT_CMD_CONF_MAX_COMMAND_LENGTH);
                    break;
                }

                case TERM_DECODE_EVENT_END_OF_LINE:
                    ATCMD_LeaveAECMode();
                    ATCMD_ParseCommandLine(consoleCmdBuffer);

                    if (NULL == pCurrentCmdTypeDesc)
                    {
                        if (false == ATCMD_ModeIsBinary())
                        {
                            ATCMD_EnterAECMode();
                            TP_CommandDecoderStartNewLine(consoleCmdBuffer, AT_CMD_CONF_MAX_COMMAND_LENGTH);
                        }
                    }

                    break;

                case TERM_DECODE_EVENT_NONE:
                default:
                    break;
            }
        }
#endif
        lastTermPollTimeMs = ATCMD_PlatformGetSysTimeMs();
    }

    ATCMD_APPUpdate();

    pCmdTableEntry = atCmdTypeDescTableInt;

    while (NULL != *pCmdTableEntry)
    {
        if (NULL != (*pCmdTableEntry)->cmdUpdate)
        {
            status = (*pCmdTableEntry)->cmdUpdate(*pCmdTableEntry, pCurrentCmdTypeDesc);

            if (*pCmdTableEntry == pCurrentCmdTypeDesc)
            {
                if (ATCMD_STATUS_PENDING != status)
                {
                    ATCMD_CompleteCommand(status);
                    if (false == ATCMD_ModeIsBinary())
                    {
                        ATCMD_EnterAECMode();
                        TP_CommandDecoderStartNewLine(consoleCmdBuffer, AT_CMD_CONF_MAX_COMMAND_LENGTH);
                    }
                }
            }
        }

        pCmdTableEntry++;
    }

    pCmdTableEntry = atCmdTypeDescTable;

    while (NULL != *pCmdTableEntry)
    {
        if (NULL != (*pCmdTableEntry)->cmdUpdate)
        {
            status = (*pCmdTableEntry)->cmdUpdate(*pCmdTableEntry, pCurrentCmdTypeDesc);

            if (*pCmdTableEntry == pCurrentCmdTypeDesc)
            {
                if (ATCMD_STATUS_PENDING != status)
                {
                    ATCMD_CompleteCommand(status);
                    if (false == ATCMD_ModeIsBinary())
                    {
                        ATCMD_EnterAECMode();
                        TP_CommandDecoderStartNewLine(consoleCmdBuffer, AT_CMD_CONF_MAX_COMMAND_LENGTH);
                    }
                }
            }
        }

        pCmdTableEntry++;
    }

    /* If we transitioned back to command mode from binary mode then reset the AEC state
       and command line input state */

    if ((true == inBinaryMode) && (false == ATCMD_ModeIsBinary()))
    {
        ATCMD_EnterAECMode();
        TP_CommandDecoderStartNewLine(consoleCmdBuffer, AT_CMD_CONF_MAX_COMMAND_LENGTH);
    }
}
