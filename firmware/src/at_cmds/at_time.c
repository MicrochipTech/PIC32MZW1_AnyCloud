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
#include <time.h>

#include "at_cmd_app.h"
#include "at_cmd_sys_time.h"

/*******************************************************************************
* Command interface prototypes
*******************************************************************************/
static ATCMD_STATUS _TIMEInit(const AT_CMD_TYPE_DESC* pCmdTypeDesc);
static ATCMD_STATUS _TIMEExecute(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const int numParams, ATCMD_PARAM *pParamList);

/*******************************************************************************
* Command parameters
*******************************************************************************/
static const ATCMD_HELP_PARAM paramFormat =
    {"FORMAT", "Format of time", ATCMD_PARAM_TYPE_CLASS_INTEGER,
        .numOpts = 3,
        {
            {"1", "UTC seconds (epoch 01/01/1970 - UNIX timestamp)"},
            {"2", "UTC seconds (epoch 01/01/1900 - NTP time)"},
            {"3", "RFC3339 / ISO-8601 format"}
        }
    };

static const ATCMD_HELP_PARAM paramUTCSeconds =
    {"UTC_SEC", "UTC seconds", ATCMD_PARAM_TYPE_CLASS_INTEGER, 0};

static const ATCMD_HELP_PARAM paramRFC3339 =
    {"DATE_TIME", "Date/time in format YYYY-MM-DDTHH:MM:SS.00Z", ATCMD_PARAM_TYPE_CLASS_STRING, 0};

/*******************************************************************************
* Command examples
*******************************************************************************/

/*******************************************************************************
* Command descriptors
*******************************************************************************/
const AT_CMD_TYPE_DESC atCmdTypeDescTIME =
    {
        .pCmdName   = "+TIME",
        .cmdInit    = _TIMEInit,
        .cmdExecute = _TIMEExecute,
        .cmdUpdate  = NULL,
        .pSummary   = "This command is used to set or query the system time",
        .numVars    = 3,
        {
            {
                .numParams   = 1,
                .pParams     =
                {
                    &paramFormat
                },
                .numExamples = 0,
                .pExamples   =
                {
                    NULL
                }
            },
            {
                .numParams   = 2,
                .pParams     =
                {
                    &paramFormat,
                    &paramUTCSeconds
                },
                .numExamples = 0,
                .pExamples   =
                {
                    NULL
                }
            },
            {
                .numParams   = 2,
                .pParams     =
                {
                    &paramFormat,
                    &paramRFC3339
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
extern ATCMD_APP_CONTEXT atCmdAppContext;

/*******************************************************************************
* Local defines and types
*******************************************************************************/

/*******************************************************************************
* Local data
*******************************************************************************/

/*******************************************************************************
* Local functions
*******************************************************************************/

/*******************************************************************************
* Command init functions
*******************************************************************************/
static ATCMD_STATUS _TIMEInit(const AT_CMD_TYPE_DESC* pCmdTypeDesc)
{
    atCmdAppContext.timeFormat = 2;

    return ATCMD_STATUS_OK;
}

/*******************************************************************************
* Command execute functions
*******************************************************************************/
static ATCMD_STATUS _TIMEExecute(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const int numParams, ATCMD_PARAM *pParamList)
{
    if (1 == numParams)
    {
        uint32_t utcTime;

        /* Check the parameter types are correct */

        if (false == ATCMD_ParamValidateTypes(pCmdTypeDesc, 0, numParams, pParamList))
        {
            return ATCMD_STATUS_INVALID_PARAMETER;
        }

        if ((pParamList[0].value.i < 1) || (pParamList[0].value.i > 3))
        {
            return ATCMD_STATUS_INVALID_PARAMETER;
        }

        utcTime = ATCMD_SysTimeGetUTC();

        if (0 == utcTime)
        {
            return ATCMD_APP_STATUS_TIME_ERROR;
        }

        if (false == ATCMD_SysTimeDisplayTimeAEC(pParamList[0].value.i, utcTime))
        {
            return ATCMD_APP_STATUS_TIME_ERROR;
        }

        atCmdAppContext.timeFormat = pParamList[0].value.i;
    }
    else if (2 == numParams)
    {
        /* Check the parameter types are correct */

        if (ATCMD_PARAM_TYPE_INTEGER == pParamList[1].type)
        {
            if ((pParamList[0].value.i < 1) || (pParamList[0].value.i > 2))
            {
                return ATCMD_STATUS_INVALID_PARAMETER;
            }

            if (false == ATCMD_ParamValidateTypes(pCmdTypeDesc, 1, numParams, pParamList))
            {
                return ATCMD_STATUS_INVALID_PARAMETER;
            }

            if (false == ATCMD_SysTimeSetUTC(pParamList[0].value.i, &pParamList[1].value.i))
            {
                return ATCMD_APP_STATUS_TIME_ERROR;
            }
        }
        else if (ATCMD_PARAM_TYPE_ASCII_STRING == pParamList[1].type)
        {
            if (false == ATCMD_ParamValidateTypes(pCmdTypeDesc, 2, numParams, pParamList))
            {
                return ATCMD_STATUS_INVALID_PARAMETER;
            }

            if (3 != pParamList[0].value.i)
            {
                return ATCMD_STATUS_INVALID_PARAMETER;
            }

            if (false == ATCMD_SysTimeSetUTC(3, pParamList[1].value.p))
            {
                return ATCMD_APP_STATUS_TIME_ERROR;
            }
        }
        else
        {
            return ATCMD_STATUS_INVALID_PARAMETER;
        }
    }
    else
    {
        return ATCMD_STATUS_INCORRECT_NUM_PARAMS;
    }

    return ATCMD_STATUS_OK;
}

/*******************************************************************************
* Command update functions
*******************************************************************************/
