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

#include <stddef.h>

#include "include/at_cmds.h"
#include "at_cmd_app.h"

/*******************************************************************************
* Command interface prototypes
*******************************************************************************/
static ATCMD_STATUS _InternalCmdInit(const AT_CMD_TYPE_DESC* pCmdTypeDesc);
static ATCMD_STATUS _InternalCmdExecute(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const int numParams, ATCMD_PARAM *pParamList);
static ATCMD_STATUS _InternalCmdUpdate(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const AT_CMD_TYPE_DESC* pCurrentCmdTypeDesc);

/*******************************************************************************
* Command parameters
*******************************************************************************/
static const ATCMD_HELP_PARAM paramBAUD_RATE =
    {"BAUD_RATE", "Baud rate", ATCMD_PARAM_TYPE_CLASS_INTEGER, 0};

/*******************************************************************************
* Command examples
*******************************************************************************/

/*******************************************************************************
* Command descriptors
*******************************************************************************/
#ifdef AT_CMD_CONF_MANUFACTURER_ID
const AT_CMD_TYPE_DESC atCmdTypeDescGMI =
    {
        .pCmdName   = "+GMI",
        .cmdInit    = NULL,
        .cmdExecute = _InternalCmdExecute,
        .cmdUpdate  = NULL,
        .pSummary   = "This command requests manufacturer identification",
        .appVal     = ATCMD_INT_APP_VAL_GMI,
        .numVars    = 1,
        {
            {
                .numParams   = 0,
                .pParams     =
                {
                    NULL
                },
                .numExamples = 0,
                .pExamples   =
                {
                    NULL
                }
            }
        }
    };
#endif

#ifdef AT_CMD_CONF_MODEL_ID
const AT_CMD_TYPE_DESC atCmdTypeDescGMM =
    {
        .pCmdName   = "+GMM",
        .cmdInit    = NULL,
        .cmdExecute = _InternalCmdExecute,
        .cmdUpdate  = NULL,
        .pSummary   = "This command requests model identification",
        .appVal     = ATCMD_INT_APP_VAL_GMM,
        .numVars    = 1,
        {
            {
                .numParams   = 0,
                .pParams     =
                {
                    NULL
                },
                .numExamples = 0,
                .pExamples   =
                {
                    NULL
                }
            }
        }
    };
#endif

#ifdef AT_CMD_CONF_REVISION_ID
const AT_CMD_TYPE_DESC atCmdTypeDescGMR =
    {
        .pCmdName   = "+GMR",
        .cmdInit    = NULL,
        .cmdExecute = _InternalCmdExecute,
        .cmdUpdate  = NULL,
        .pSummary   = "This command requests revision identification",
        .appVal     = ATCMD_INT_APP_VAL_GMR,
        .numVars    = 1,
        {
            {
                .numParams   = 0,
                .pParams     =
                {
                    NULL
                },
                .numExamples = 0,
                .pExamples   =
                {
                    NULL
                }
            }
        }
    };
#endif

const AT_CMD_TYPE_DESC atCmdTypeDescIPR =
    {
        .pCmdName   = "+IPR",
        .cmdInit    = _InternalCmdInit,
        .cmdExecute = _InternalCmdExecute,
        .cmdUpdate  = _InternalCmdUpdate,
        .pSummary   = "This command sets the DTE serial port baud rate",
        .appVal     = ATCMD_INT_APP_VAL_IPR,
        .numVars    = 2,
        {
            {
                .numParams   = 0,
                .pParams     =
                {
                    NULL
                },
                .numExamples = 0,
                .pExamples   =
                {
                    NULL
                }
            },
            {
                .numParams   = 1,
                .pParams     =
                {
                    &paramBAUD_RATE
                },
                .numExamples = 0,
                .pExamples   =
                {
                    NULL
                }
            }
        }
    };

/*******************************************************************************
* External references
*******************************************************************************/

/*******************************************************************************
* Local defines and types
*******************************************************************************/

/*******************************************************************************
* Local data
*******************************************************************************/
uint32_t newBaudRate;

/*******************************************************************************
* Local functions
*******************************************************************************/

/*******************************************************************************
* Command init functions
*******************************************************************************/
static ATCMD_STATUS _InternalCmdInit(const AT_CMD_TYPE_DESC* pCmdTypeDesc)
{
    newBaudRate = 0;

    return ATCMD_STATUS_OK;
}

/*******************************************************************************
* Command execute functions
*******************************************************************************/
static ATCMD_STATUS _InternalCmdExecute(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const int numParams, ATCMD_PARAM *pParamList)
{
    if (1 == pCmdTypeDesc->numVars)
    {
        /* Check the parameter types are correct */

        if (false == ATCMD_ParamValidateTypes(pCmdTypeDesc, 0, numParams, pParamList))
        {
            return ATCMD_STATUS_INVALID_PARAMETER;
        }
    }

    switch (pCmdTypeDesc->appVal)
    {
#ifdef AT_CMD_CONF_MANUFACTURER_ID
        case ATCMD_INT_APP_VAL_GMI:
        {
            const char *pGMIStr;

            ATCMD_Print("+GMI=", 5);

            pGMIStr = AT_CMD_CONF_MANUFACTURER_ID;

            if (NULL != pGMIStr)
            {
                ATCMD_Printf("%s", pGMIStr);
            }

            ATCMD_Print("\r\n", 2);
            break;
        }
#endif
#ifdef AT_CMD_CONF_MODEL_ID
        case ATCMD_INT_APP_VAL_GMM:
        {
            const char *pGMMStr;

            ATCMD_Print("+GMM=", 5);

            pGMMStr = AT_CMD_CONF_MODEL_ID;

            if (NULL != pGMMStr)
            {
                ATCMD_Printf("%s", pGMMStr);
            }

            ATCMD_Print("\r\n", 2);
            break;
        }
#endif
#ifdef AT_CMD_CONF_REVISION_ID
        case ATCMD_INT_APP_VAL_GMR:
        {
            const char *pGMRStr;

            ATCMD_Print("+GMR=", 5);

            pGMRStr = AT_CMD_CONF_REVISION_ID;

            if (NULL != pGMRStr)
            {
                ATCMD_Printf("%s", pGMRStr);
            }

            ATCMD_Print("\r\n", 2);
            break;
        }
#endif
        case ATCMD_INT_APP_VAL_IPR:
        {
            if (0 == numParams)
            {
                ATCMD_Printf("+IPR:%d\r\n", ATCMD_PlatformUARTGetBaudRate());
            }
            else if (1 == numParams)
            {
                /* Check the parameter types are correct */

                if (false == ATCMD_ParamValidateTypes(pCmdTypeDesc, 1, numParams, pParamList))
                {
                    return ATCMD_STATUS_INVALID_PARAMETER;
                }

                newBaudRate = pParamList[0].value.i;
            }
            else
            {
                return ATCMD_STATUS_INCORRECT_NUM_PARAMS;
            }

            break;
        }

        default:
        {
            return ATCMD_STATUS_INVALID_CMD;
        }
    }

    return ATCMD_STATUS_OK;
}

/*******************************************************************************
* Command update functions
*******************************************************************************/
static ATCMD_STATUS _InternalCmdUpdate(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const AT_CMD_TYPE_DESC* pCurrentCmdTypeDesc)
{
    switch (pCmdTypeDesc->appVal)
    {
        case ATCMD_INT_APP_VAL_IPR:
        {
            if (0 != newBaudRate)
            {
                ATCMD_PlatformUARTSetBaudRate(newBaudRate);

                newBaudRate = 0;
            }

            break;
        }
    }

    return ATCMD_STATUS_OK;
}
