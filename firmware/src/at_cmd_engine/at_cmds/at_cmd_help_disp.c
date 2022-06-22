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
#include <string.h>

#include "include/at_cmds.h"

/*****************************************************************************
 * Format a display of help information about a command
 *****************************************************************************/
void ATCMD_ShowHelp(const AT_CMD_TYPE_DESC *pCmd)
{
    uint8_t v, i, j;
    uint8_t maxParamNameLen;
    uint8_t maxOptNameLen;
    uint8_t maxExpNameLen;
    uint8_t nameLen;

    if (NULL == pCmd)
    {
        return;
    }

    if (NULL == pCmd->pCmdName)
    {
        return;
    }

    ATCMD_Printf("Description\r\n");

    maxParamNameLen = 0;

    if (NULL != pCmd->pSummary)
    {
        ATCMD_Printf("  %s.\r\n\r\n", pCmd->pSummary);
    }

    for (v=0; v<pCmd->numVars; v++)
    {
        const ATCMD_HELP_CMD_VAR *pVar = &pCmd->vars[v];

        ATCMD_Printf("Command Syntax:\r\n  " AT_CMD_CONF_AT_BASE_STRING "%s", pCmd->pCmdName);

        if (pVar->numParams > 0)
        {
            ATCMD_Printf("=", pVar->numParams);

            for (i=0; i<pVar->numParams; i++)
            {
                const ATCMD_HELP_PARAM *pParam = pVar->pParams[i];

                if (0 != i)
                {
                    ATCMD_Printf(",");
                }

                if (NULL == pParam->pDescription)
                {
                    ATCMD_Printf("%s", pParam->pName);
                }
                else
                {
                    ATCMD_Printf("<%s>", pParam->pName);
                }

                nameLen = (uint8_t)strlen(pParam->pName);

                if (nameLen > maxParamNameLen)
                {
                    maxParamNameLen = nameLen;
                }
            }
        }

        ATCMD_Printf("\r\n\r\n");

        if (pVar->numParams > 0)
        {
            bool paramDisplayed = false;

            if (maxParamNameLen < 15)
            {
                maxParamNameLen = 15;
            }

            ATCMD_Printf("  Parameter Name%-*cType     Description\r\n", (maxParamNameLen-12), ' ');

            for (i=0; i<pVar->numParams; i++)
            {
                const ATCMD_HELP_PARAM *pParam = pVar->pParams[i];

                if (NULL == pParam->pDescription)
                {
                    continue;
                }

                paramDisplayed = true;

                ATCMD_Printf("  <%s>%-*c", pParam->pName, (maxParamNameLen-strlen(pParam->pName)), ' ');

                switch(pParam->typeClass)
                {
                    case ATCMD_PARAM_TYPE_CLASS_ANY:
                    {
                        ATCMD_Printf("Any      ");
                        break;
                    }

                    case ATCMD_PARAM_TYPE_CLASS_INTEGER:
                    {
                        ATCMD_Printf("Integer  ");
                        break;
                    }

                    case ATCMD_PARAM_TYPE_CLASS_STRING:
                    {
                        ATCMD_Printf("String   ");
                        break;
                    }
                }

                ATCMD_Printf("%s.\r\n", pParam->pDescription);

                maxOptNameLen = 0;

                if (pParam->numOpts > 0)
                {
                    for (j=0; j<pParam->numOpts; j++)
                    {
                        const ATCMD_HELP_PARAM_OPT *pOpt = &pParam->opts[j];

                        nameLen = (uint8_t)strlen(pOpt->pName);

                        if (nameLen > maxOptNameLen)
                        {
                            maxOptNameLen = nameLen;
                        }
                    }

                    for (j=0; j<pParam->numOpts; j++)
                    {
                        const ATCMD_HELP_PARAM_OPT *pOpt = &pParam->opts[j];

                        ATCMD_Printf("%*s%-*s: %s.\r\n", maxParamNameLen+16, " ", maxOptNameLen+1, pOpt->pName, pOpt->pDescription);
                    }
                }
            }

            if (true == paramDisplayed)
            {
                ATCMD_Printf("\r\n");
            }
        }

        if (pVar->numExamples > 0)
        {
            maxExpNameLen = 0;

            for (i=0; i<pVar->numExamples; i++)
            {
                const ATCMD_HELP_EXAMPLE *pExample = pVar->pExamples[i];

                if (NULL == pExample->pText)
                {
                    nameLen = 0;
                }
                else
                {
                    nameLen = (uint8_t)strlen(pExample->pText);
                }

                if (nameLen > maxExpNameLen)
                {
                    maxExpNameLen = nameLen;
                }
            }

            for (i=0; i<pVar->numExamples; i++)
            {
                const ATCMD_HELP_EXAMPLE *pExample = pVar->pExamples[i];

                if (0 == i)
                {
                    ATCMD_Printf("  Example: ");
                }
                else
                {
                    ATCMD_Printf("         : ");
                }

                if (NULL == pExample->pText)
                {
                    ATCMD_Printf(AT_CMD_CONF_AT_BASE_STRING "%s%-*s", pCmd->pCmdName, maxExpNameLen, "");
                }
                else
                {
                    ATCMD_Printf(AT_CMD_CONF_AT_BASE_STRING "%s=%-*s", pCmd->pCmdName, maxExpNameLen, pExample->pText);
                }

                if (NULL != pExample->pDescription)
                {
                    ATCMD_Printf(" -> %s.", pExample->pDescription);
                }

                ATCMD_Printf("\r\n");
            }

            ATCMD_Printf("\r\n");
        }
    }
}
