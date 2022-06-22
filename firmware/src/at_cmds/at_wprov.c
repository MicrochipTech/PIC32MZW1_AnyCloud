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

#include "at_cmd_app.h"

/*******************************************************************************
* Command interface prototypes
*******************************************************************************/
static ATCMD_STATUS _WPROVInit(const AT_CMD_TYPE_DESC* pCmdTypeDesc);
static ATCMD_STATUS _WPROVCExecute(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const int numParams, ATCMD_PARAM *pParamList);
static ATCMD_STATUS _WPROVExecute(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const int numParams, ATCMD_PARAM *pParamList);

/*******************************************************************************
* Command parameters
*******************************************************************************/
static const ATCMD_HELP_PARAM paramID =
    {"ID", "Parameter ID number", ATCMD_PARAM_TYPE_CLASS_INTEGER, 0};

static const ATCMD_HELP_PARAM paramVAL =
    {"VAL", "Parameter value", ATCMD_PARAM_TYPE_CLASS_ANY, 0};

static const ATCMD_HELP_PARAM paramSTATE =
    {"STATE", "State of the provisioning feature", ATCMD_PARAM_TYPE_INTEGER,
        .numOpts = 2,
        {
            {"0", "Stop"},
            {"1", "Start"}
        }
    };

/*******************************************************************************
* Command examples
*******************************************************************************/

/*******************************************************************************
* Command descriptors
*******************************************************************************/
const AT_CMD_TYPE_DESC atCmdTypeDescWPROVC =
    {
        .pCmdName   = "+WPROVC",
        .cmdInit    = _WPROVInit,
        .cmdExecute = _WPROVCExecute,
        .cmdUpdate  = NULL,
        .pSummary   = "This command is used to configure the operation of the DCEs provisioning feature",
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

const AT_CMD_TYPE_DESC atCmdTypeDescWPROV =
    {
        .pCmdName   = "+WPROV",
        .cmdInit    = NULL,
        .cmdExecute = _WPROVExecute,
        .cmdUpdate  = NULL,
        .pSummary   = "This command is used to control the operation of the DCEs provisioning feature",
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
                    &paramSTATE
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
#define WPROVC_MAP_MAX_PARAMS     2

/*******************************************************************************
* Local data
*******************************************************************************/
static const ATCMD_STORE_MAP_ELEMENT wprovConfMap[] = {
    {1,  offsetof(ATCMD_APP_WPROV_CONF, mode),              ATCMD_STORE_TYPE_INT,        sizeof(int),       ATCMD_STORE_ACCESS_RW},
    {2,  offsetof(ATCMD_APP_WPROV_CONF, pin),               ATCMD_STORE_TYPE_STRING,     8,                 ATCMD_STORE_ACCESS_RW},
    {0,  0,                                                 ATCMD_STORE_TYPE_INVALID,    0,                 ATCMD_STORE_ACCESS_RW}
};

/*******************************************************************************
* Local functions
*******************************************************************************/
static bool _ValidateChecksum(uint8_t *pPin)
{
    uint32_t accum = 0;

    accum += 3 * (*pPin++ - '0');
    accum += 1 * (*pPin++ - '0');
    accum += 3 * (*pPin++ - '0');
    accum += 1 * (*pPin++ - '0');
    accum += 3 * (*pPin++ - '0');
    accum += 1 * (*pPin++ - '0');
    accum += 3 * (*pPin++ - '0');
    accum += 1 * (*pPin++ - '0');

    return (0 == (accum % 10));
}

/*******************************************************************************
* Command init functions
*******************************************************************************/
static ATCMD_STATUS _WPROVInit(const AT_CMD_TYPE_DESC* pCmdTypeDesc)
{
    memset(&atCmdAppContext.wprovConf, 0, sizeof(ATCMD_APP_WPROV_CONF));
    memset(&atCmdAppContext.wprovState, 0, sizeof(ATCMD_APP_WPROV_STATE));

    return ATCMD_STATUS_OK;
}

/*******************************************************************************
* Command execute functions
*******************************************************************************/
static ATCMD_STATUS _WPROVCExecute(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const int numParams, ATCMD_PARAM *pParamList)
{
    if (0 == numParams)
    {
        int id;

        /* Dump all configuration elements */

        for (id=1; id<=WPROVC_MAP_MAX_PARAMS; id++)
        {
            /* Read the element from the configuration structure */

            ATCMD_StructStorePrint(pCmdTypeDesc->pCmdName, wprovConfMap, &atCmdAppContext.wprovConf, id);
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

        if (false == ATCMD_StructStorePrint(pCmdTypeDesc->pCmdName, wprovConfMap, &atCmdAppContext.wprovConf, pParamList[0].value.i))
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

        /* If started then block write access to all elements */

        if (0 != atCmdAppContext.wapConnState.wapState)
        {
            return ATCMD_STATUS_STORE_UPDATE_BLOCKED;
        }

        if (2 == pParamList[0].value.i)
        {
            if (8 == pParamList[1].length)
            {
                if (false == _ValidateChecksum(pParamList[1].value.p))
                {
                    return ATCMD_STATUS_INVALID_PARAMETER;
                }
            }
            else if (4 != pParamList[1].length)
            {
                return ATCMD_STATUS_INVALID_PARAMETER;
            }
        }

        /* Access the element in the configuration structure */

        if (0 == ATCMD_StructStoreWriteParam(wprovConfMap, &atCmdAppContext.wprovConf, pParamList[0].value.i, &pParamList[1]))
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

static ATCMD_STATUS _WPROVExecute(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const int numParams, ATCMD_PARAM *pParamList)
{
    ATCMD_STATUS retStatus;

    if (0 == numParams)
    {
        if ((ATCMD_APP_STATE_WPS_STARTED == ATCMD_APPStateMachineCurrentState()) || (ATCMD_APP_STATE_PROV_AP_STARTED == ATCMD_APPStateMachineCurrentState()))
        {
            ATCMD_Print("+WPROV:1\r\n", 10);
        }
        else
        {
            ATCMD_Print("+WPROV:0\r\n", 10);
        }

        return ATCMD_STATUS_OK;
    }
    else if (1 == numParams)
    {
        if (false == ATCMD_ParamValidateTypes(pCmdTypeDesc, 1, numParams, pParamList))
        {
            return ATCMD_STATUS_INVALID_PARAMETER;
        }

        if ((0 == atCmdAppContext.wprovConf.mode) || (1 == atCmdAppContext.wprovConf.mode))
        {
            /* WPS */

            if (0 == pParamList[0].value.i)
            {
                /* WPS Stop */

                if (false == ATCMD_APPStateMachineEvent(ATCMD_APP_EVENT_STOP_WPS, false))
                {
                    return ATCMD_APP_STATUS_PROV_WPS_STOP_REFUSED;
                }

                /* TODO */

                if (false == ATCMD_APPStateMachineEvent(ATCMD_APP_EVENT_STOP_WPS, true))
                {
                    return ATCMD_APP_STATUS_PROV_WPS_STOP_FAILED;
                }
            }
            else if (1 == pParamList[0].value.i)
            {
                /* WPS Start */

                if (false == ATCMD_APPStateMachineEvent(ATCMD_APP_EVENT_START_WPS, false))
                {
                    return ATCMD_APP_STATUS_PROV_WPS_START_REFUSED;
                }

                /* TODO */

                if (false == ATCMD_APPStateMachineEvent(ATCMD_APP_EVENT_START_WPS, true))
                {
                    return ATCMD_APP_STATUS_PROV_WPS_START_FAILED;
                }
            }
            else
            {
                return ATCMD_STATUS_INVALID_PARAMETER;
            }
        }
        else if (2 == atCmdAppContext.wprovConf.mode)
        {
            /* Web page provisioning */

            if (0 == pParamList[0].value.i)
            {
                /* Stop */

                if (false == ATCMD_APPStateMachineEvent(ATCMD_APP_EVENT_PROV_AP_STOPPING, false))
                {
                    return ATCMD_APP_STATUS_PROV_AP_STOP_REFUSED;
                }

                retStatus = ATCMD_WAP_Stop();

                if (ATCMD_STATUS_OK != retStatus)
                {
                    return retStatus;
                }

                if (false == ATCMD_APPStateMachineEvent(ATCMD_APP_EVENT_PROV_AP_STOPPING, true))
                {
                    return ATCMD_APP_STATUS_PROV_AP_STOP_FAILED;
                }
            }
            else if (1 == pParamList[0].value.i)
            {
                /* Start */

                if (false == ATCMD_APPStateMachineEvent(ATCMD_APP_EVENT_PROV_AP_STARTING, false))
                {
                    return ATCMD_APP_STATUS_PROV_AP_START_REFUSED;
                }

                retStatus = ATCMD_WAP_Start(true);

                if (ATCMD_STATUS_OK != retStatus)
                {
                    return retStatus;
                }

                if (false == ATCMD_APPStateMachineEvent(ATCMD_APP_EVENT_PROV_AP_STARTING, true))
                {
                    return ATCMD_APP_STATUS_PROV_AP_START_FAILED;
                }
            }
            else
            {
                return ATCMD_STATUS_INVALID_PARAMETER;
            }
        }

        atCmdAppContext.wprovState.wprovState = pParamList[0].value.i;
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
