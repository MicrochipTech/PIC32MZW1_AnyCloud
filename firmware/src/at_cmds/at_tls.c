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
#include "at_cmd_tls.h"

/*******************************************************************************
* Command interface prototypes
*******************************************************************************/
static ATCMD_STATUS _TLSInit(const AT_CMD_TYPE_DESC* pCmdTypeDesc);
static ATCMD_STATUS _TLSCExecute(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const int numParams, ATCMD_PARAM *pParamList);

/*******************************************************************************
* Command parameters
*******************************************************************************/
static const ATCMD_HELP_PARAM paramCONF =
    {"CONF", "Configuration number", ATCMD_PARAM_TYPE_CLASS_INTEGER, 0};

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
const AT_CMD_TYPE_DESC atCmdTypeDescTLSC =
    {
        .pCmdName   = "+TLSC",
        .cmdInit    = _TLSInit,
        .cmdExecute = _TLSCExecute,
        .cmdUpdate  = NULL,
        .pSummary   = "This command is used to read or set the TLS configuration",
        .numVars    = 3,
        {
            {
                .numParams   = 1,
                .pParams     =
                {
                    &paramCONF
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
                    &paramCONF,
                    &paramID
                },
                .numExamples = 0,
                .pExamples   =
                {
                    NULL
                }
            },
            {
                .numParams   = 3,
                .pParams     =
                {
                    &paramCONF,
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
#define TLSC_MAP_MAX_PARAMS    5

/*******************************************************************************
* Local data
*******************************************************************************/
static const ATCMD_STORE_MAP_ELEMENT tlsConfMap[] = {
    {1,  offsetof(ATCMD_APP_TLS_CONF, caCertName),      ATCMD_STORE_TYPE_STRING,    AT_CMD_TLS_CERT_NAME_SZ,    ATCMD_STORE_ACCESS_RW},
    {2,  offsetof(ATCMD_APP_TLS_CONF, certName),        ATCMD_STORE_TYPE_STRING,    AT_CMD_TLS_CERT_NAME_SZ,    ATCMD_STORE_ACCESS_RW},
    {3,  offsetof(ATCMD_APP_TLS_CONF, priKeyName),      ATCMD_STORE_TYPE_STRING,    AT_CMD_TLS_PRIKEY_NAME_SZ,  ATCMD_STORE_ACCESS_RW},
#ifdef WOLFSSL_ENCRYPTED_KEYS    
    {4,  offsetof(ATCMD_APP_TLS_CONF, priKeyPassword),  ATCMD_STORE_TYPE_STRING,    AT_CMD_TLS_PRIKEY_PW_SZ,    ATCMD_STORE_ACCESS_WRITE},
#endif    
    {5,  offsetof(ATCMD_APP_TLS_CONF, serverName),      ATCMD_STORE_TYPE_STRING,    AT_CMD_TLS_SERVER_NAME_SZ,  ATCMD_STORE_ACCESS_RW},
    {0,  0,                                             ATCMD_STORE_TYPE_INVALID,   0,                          ATCMD_STORE_ACCESS_RW}
};

/*******************************************************************************
* Local functions
*******************************************************************************/

/*******************************************************************************
* Command init functions
*******************************************************************************/
static ATCMD_STATUS _TLSInit(const AT_CMD_TYPE_DESC* pCmdTypeDesc)
{
    memset(&atCmdAppContext.tlsConf, 0, sizeof(atCmdAppContext.tlsConf));
    memset(&atCmdAppContext.tlsState, 0, sizeof(atCmdAppContext.tlsState));

    return ATCMD_STATUS_OK;
}

/*******************************************************************************
* Command execute functions
*******************************************************************************/
static ATCMD_STATUS _TLSCExecute(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const int numParams, ATCMD_PARAM *pParamList)
{
    ATCMD_APP_TLS_CONF *ptlsConf = NULL;

    /* Check the parameter types are correct */

    if (false == ATCMD_ParamValidateTypes(pCmdTypeDesc, numParams-1, numParams, pParamList))
    {
        return ATCMD_STATUS_INVALID_PARAMETER;
    }

    if (numParams >= 1)
    {
        if ((pParamList[0].value.i < 1) || (pParamList[0].value.i > AT_CMD_TLS_NUM_CONFS))
        {
            return ATCMD_STATUS_INVALID_PARAMETER;
        }

        ptlsConf = &atCmdAppContext.tlsConf[pParamList[0].value.i-1];
    }

    if (1 == numParams)
    {
        int id;

        /* Dump all configuration elements */

        for (id=1; id<=TLSC_MAP_MAX_PARAMS; id++)
        {
            /* Read the element from the configuration structure */

            ATCMD_StructStorePrint(pCmdTypeDesc->pCmdName, tlsConfMap, ptlsConf, id);
        }

        return ATCMD_STATUS_OK;
    }
    else if (2 == numParams)
    {
        /* Access the element in the configuration structure */

        if (false == ATCMD_StructStorePrint(pCmdTypeDesc->pCmdName, tlsConfMap, ptlsConf, pParamList[1].value.i))
        {
            return ATCMD_STATUS_STORE_ACCESS_FAILED;
        }
    }
    else if (3 == numParams)
    {
        if (0 != atCmdAppContext.tlsConf[pParamList[0].value.i-1].numSessions)
        {
            return ATCMD_STATUS_STORE_UPDATE_BLOCKED;
        }

        /* Access the element in the configuration structure */

        if (0 == ATCMD_StructStoreWriteParam(tlsConfMap, ptlsConf, pParamList[1].value.i, &pParamList[2]))
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
