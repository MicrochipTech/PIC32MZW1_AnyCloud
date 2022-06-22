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
#include <string.h>

#include "at_cmd_app.h"

/*******************************************************************************
* Command interface prototypes
*******************************************************************************/
static ATCMD_STATUS _CFGInit(const AT_CMD_TYPE_DESC* pCmdTypeDesc);
static ATCMD_STATUS _CFGExecute(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const int numParams, ATCMD_PARAM *pParamList);

/*******************************************************************************
* Command parameters
*******************************************************************************/
static const ATCMD_HELP_PARAM paramID =
    {"ID", "Parameter ID number", ATCMD_PARAM_TYPE_CLASS_INTEGER, 0};

static const ATCMD_HELP_PARAM paramVAL =
    {"VAL", "Parameter value", ATCMD_PARAM_TYPE_CLASS_ANY, 0};

/*******************************************************************************
* Command examples
*******************************************************************************/

/*******************************************************************************
* Command descriptors
*******************************************************************************/
const AT_CMD_TYPE_DESC atCmdTypeDescCFG =
    {
        .pCmdName   = "+CFG",
        .cmdInit    = _CFGInit,
        .cmdExecute = _CFGExecute,
        .cmdUpdate  = NULL,
        .pSummary   = "This command is used to read or set the system configuration",
        .numVars    = 3,
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
                    &paramID
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
                    &paramID,
                    &paramVAL
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
#define SYSCFG_MAP_MAX_PARAMS    2

/*******************************************************************************
* Local data
*******************************************************************************/
static const ATCMD_STORE_MAP_ELEMENT sysConfMap[] = {
    {1,  offsetof(ATCMD_APP_SYS_CONF, macAddr),             ATCMD_STORE_TYPE_MACADDR,    6,     ATCMD_STORE_ACCESS_READ},
    {2,  offsetof(ATCMD_APP_SYS_CONF, devName),             ATCMD_STORE_TYPE_STRING,     32,    ATCMD_STORE_ACCESS_RW},
    {0,  0,                                                 ATCMD_STORE_TYPE_INVALID,    0,     ATCMD_STORE_ACCESS_RW}
};

/*******************************************************************************
* Local functions
*******************************************************************************/

/*******************************************************************************
* Command init functions
*******************************************************************************/
static ATCMD_STATUS _CFGInit(const AT_CMD_TYPE_DESC* pCmdTypeDesc)
{
    memset(&atCmdAppContext.sysConf, 0, sizeof(ATCMD_APP_SYS_CONF));

    if (WDRV_PIC32MZW_STATUS_OK != WDRV_PIC32MZW_InfoDeviceMACAddressGet(atCmdAppContext.wdrvHandle, atCmdAppContext.sysConf.macAddr))
    {
        return ATCMD_APP_STATUS_WIFI_API_REQUEST_FAILED;
    }

    atCmdAppContext.sysConf.devName[0] = snprintf((char*)&atCmdAppContext.sysConf.devName[1], 32, "PIC32MZW1-%02x-%02x", atCmdAppContext.sysConf.macAddr[4], atCmdAppContext.sysConf.macAddr[5]);

    return ATCMD_STATUS_OK;
}

/*******************************************************************************
* Command execute functions
*******************************************************************************/
static ATCMD_STATUS _CFGExecute(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const int numParams, ATCMD_PARAM *pParamList)
{
    if (0 == numParams)
    {
        int id;

        /* Dump all configuration elements */

        for (id=1; id<=SYSCFG_MAP_MAX_PARAMS; id++)
        {
            /* Read the element from the configuration structure */

            ATCMD_StructStorePrint(pCmdTypeDesc->pCmdName, sysConfMap, &atCmdAppContext.sysConf, id);
        }

        return ATCMD_STATUS_OK;
    }
    else if (1 == numParams)
    {
        /* Check the parameter types are correct */

        if (false == ATCMD_ParamValidateTypes(pCmdTypeDesc, 1, numParams, pParamList))
        {
            return ATCMD_STATUS_INVALID_PARAMETER;
        }

        /* Access the element in the configuration structure */

        if (false == ATCMD_StructStorePrint(pCmdTypeDesc->pCmdName, sysConfMap, &atCmdAppContext.sysConf, pParamList[0].value.i))
        {
            return ATCMD_STATUS_STORE_ACCESS_FAILED;
        }
    }
    else if (2 == numParams)
    {
        /* Check the parameter types are correct */

        if (false == ATCMD_ParamValidateTypes(pCmdTypeDesc, 2, numParams, pParamList))
        {
            return ATCMD_STATUS_INVALID_PARAMETER;
        }

        /* Access the element in the configuration structure */

        if (0 == ATCMD_StructStoreWriteParam(sysConfMap, &atCmdAppContext.sysConf, pParamList[0].value.i, &pParamList[1]))
        {
            return ATCMD_STATUS_STORE_ACCESS_FAILED;
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
