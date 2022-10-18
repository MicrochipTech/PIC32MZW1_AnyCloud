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
static ATCMD_STATUS _WSCNInit(const AT_CMD_TYPE_DESC* pCmdTypeDesc);
static ATCMD_STATUS _WSCNCExecute(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const int numParams, ATCMD_PARAM *pParamList);
static ATCMD_STATUS _WSCNAExecute(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const int numParams, ATCMD_PARAM *pParamList);
static ATCMD_STATUS _WSCNPExecute(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const int numParams, ATCMD_PARAM *pParamList);
static ATCMD_STATUS _WSCNUpdate(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const AT_CMD_TYPE_DESC* pCurrentCmdTypeDesc);

/*******************************************************************************
* Command parameters
*******************************************************************************/
static const ATCMD_HELP_PARAM paramID =
    {"ID", "Parameter ID number", ATCMD_PARAM_TYPE_CLASS_INTEGER, 0};

static const ATCMD_HELP_PARAM paramVAL =
    {"VAL", "Parameter value", ATCMD_PARAM_TYPE_CLASS_ANY, 0};

static const ATCMD_HELP_PARAM paramCHANNEL =
    {"CHANNEL", "The channel to scan, a value of 255 scans all available channels", ATCMD_PARAM_TYPE_CLASS_INTEGER, 0};

static const ATCMD_HELP_PARAM paramSSID =
    {"SSID", "Scan for a specific SSID, confirms presence of a cloaked network", ATCMD_PARAM_TYPE_CLASS_STRING, 0};

/*******************************************************************************
* Command examples
*******************************************************************************/

/*******************************************************************************
* Command descriptors
*******************************************************************************/
const AT_CMD_TYPE_DESC atCmdTypeDescWSCNC =
    {
        .pCmdName   = "+WSCNC",
        .cmdInit    = _WSCNInit,
        .cmdExecute = _WSCNCExecute,
        .cmdUpdate  = _WSCNUpdate,
        .pSummary   = "This command is used to modify or query the behavior of the active scanning function",
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

const AT_CMD_TYPE_DESC atCmdTypeDescWSCNA =
    {
        .pCmdName   = "+WSCNA",
        .cmdInit    = NULL,
        .cmdExecute = _WSCNAExecute,
        .cmdUpdate  = NULL,
        .pSummary   = "This command is used to actively scan for infrastructure networks in range of the DCE",
        .numVars    = 2,
        {
            {
                .numParams   = 1,
                .pParams     =
                {
                    &paramCHANNEL
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
                    &paramCHANNEL,
                    &paramSSID
                },
                .numExamples = 0,
                .pExamples   =
                {
                    NULL
                }
            }
        }
    };

const AT_CMD_TYPE_DESC atCmdTypeDescWSCNP =
    {
        .pCmdName   = "+WSCNP",
        .cmdInit    = NULL,
        .cmdExecute = _WSCNPExecute,
        .cmdUpdate  = NULL,
        .pSummary   = "This command is used to passively scan for infrastructure networks in range of the DCE",
        .numVars    = 2,
        {
            {
                .numParams   = 1,
                .pParams     =
                {
                    &paramCHANNEL
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
                    &paramCHANNEL,
                    &paramSSID
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
#define WSCNC_MAP_MAX_PARAMS    2

/*******************************************************************************
* Local data
*******************************************************************************/
static const ATCMD_STORE_MAP_ELEMENT wscnConfMap[] = {
    {1,  offsetof(ATCMD_APP_WSCN_CONF, activeScanTime),         ATCMD_STORE_TYPE_INT,        sizeof(int),       ATCMD_STORE_ACCESS_RW},
    {2,  offsetof(ATCMD_APP_WSCN_CONF, passiveListenTime),      ATCMD_STORE_TYPE_INT,        sizeof(int),       ATCMD_STORE_ACCESS_RW},
    {0,  0,                                                     ATCMD_STORE_TYPE_INVALID,    0,                 ATCMD_STORE_ACCESS_RW}
};

static OSAL_SEM_HANDLE_TYPE bssFindResult;
static uint8_t bssFindIdx;

/*******************************************************************************
* Local functions
*******************************************************************************/
static bool _BSSFindNotifyCallback(DRV_HANDLE handle, uint8_t index, uint8_t ofTotal, WDRV_PIC32MZW_BSS_INFO *pBSSInfo)
{
    if (ofTotal > 0)
    {
        if (0 == index)
        {
            bssFindIdx = 0;
        }

        if (atCmdAppContext.wscnSSIDFiltLen > 0)
        {
            if ((atCmdAppContext.wscnSSIDFiltLen != pBSSInfo->ctx.ssid.length) || (0 != memcmp(pBSSInfo->ctx.ssid.name, atCmdAppContext.wscnSSIDFilt, atCmdAppContext.wscnSSIDFiltLen)))
            {
                pBSSInfo = NULL;
            }
        }

        if (NULL != pBSSInfo)
        {
            OSAL_SEM_Post(&bssFindResult);
        }
    }

    return false;
}

/*******************************************************************************
* Command init functions
*******************************************************************************/
static ATCMD_STATUS _WSCNInit(const AT_CMD_TYPE_DESC* pCmdTypeDesc)
{
    atCmdAppContext.wscnConf.activeScanTime         = DRV_PIC32MZW_DEFAULT_ACTIVE_SCAN_TIME;
    atCmdAppContext.wscnConf.passiveListenTime      = DRV_PIC32MZW_DEFAULT_PASSIVE_SCAN_TIME;

    atCmdAppContext.wscnScanInProgress  = false;
    atCmdAppContext.wscnNumOfBSSs       = 0;

    OSAL_SEM_Create(&bssFindResult, OSAL_SEM_TYPE_COUNTING, 255, 0);

    return ATCMD_STATUS_OK;
}

/*******************************************************************************
* Command execute functions
*******************************************************************************/
static ATCMD_STATUS _WSCNCExecute(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const int numParams, ATCMD_PARAM *pParamList)
{
    if (0 == numParams)
    {
        int id;

        /* Dump all configuration elements */

        for (id=1; id<=WSCNC_MAP_MAX_PARAMS; id++)
        {
            /* Read the element from the configuration structure */

            ATCMD_StructStorePrint(pCmdTypeDesc->pCmdName, wscnConfMap, &atCmdAppContext.wscnConf, id);
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

        if (false == ATCMD_StructStorePrint(pCmdTypeDesc->pCmdName, wscnConfMap, &atCmdAppContext.wscnConf, pParamList[0].value.i))
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

        if (0 == ATCMD_StructStoreWriteParam(wscnConfMap, &atCmdAppContext.wscnConf, pParamList[0].value.i, &pParamList[1]))
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

static ATCMD_STATUS _WSCNCommonExecute(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const int numParams, ATCMD_PARAM *pParamList)
{
    atCmdAppContext.wscnSSIDFiltLen = 0;

    if (1 == numParams)
    {
        if (false == ATCMD_ParamValidateTypes(pCmdTypeDesc, 0, numParams, pParamList))
        {
            return ATCMD_STATUS_INVALID_PARAMETER;
        }
    }
    else if (2 == numParams)
    {
        if (false == ATCMD_ParamValidateTypes(pCmdTypeDesc, 1, numParams, pParamList))
        {
            return ATCMD_STATUS_INVALID_PARAMETER;
        }

        if (pParamList[1].length > AT_CMD_SSID_SZ)
        {
            return ATCMD_STATUS_INVALID_PARAMETER;
        }

        atCmdAppContext.wscnSSIDFiltLen = pParamList[1].length;
        memset(atCmdAppContext.wscnSSIDFilt, 0, AT_CMD_SSID_SZ);
        memcpy(atCmdAppContext.wscnSSIDFilt, pParamList[1].value.p, pParamList[1].length);
    }
    else
    {
        return ATCMD_STATUS_INCORRECT_NUM_PARAMS;
    }

    if (255 == pParamList[0].value.i)
    {
        atCmdAppContext.wscnChannel = WDRV_PIC32MZW_CID_ANY;
    }
    else
    {
        atCmdAppContext.wscnChannel = pParamList[0].value.i;
    }

    return ATCMD_STATUS_OK;
}

static ATCMD_STATUS _WSCNAExecute(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const int numParams, ATCMD_PARAM *pParamList)
{
    ATCMD_STATUS retStatus;

    retStatus = _WSCNCommonExecute(pCmdTypeDesc, numParams, pParamList);

    if (ATCMD_STATUS_OK != retStatus)
    {
        return retStatus;
    }

//#ifdef ANY_CLOUD_RN
    if (WDRV_PIC32MZW_STATUS_OK != WDRV_PIC32MZW_BSSFindSetScanParameters(atCmdAppContext.wdrvHandle, 
							2, (uint16_t)atCmdAppContext.wscnConf.activeScanTime, 
							(uint16_t)atCmdAppContext.wscnConf.passiveListenTime, DRV_PIC32MZW_SCAN_MAX_NUM_PROBE))
    {
        return ATCMD_APP_STATUS_WIFI_API_REQUEST_FAILED;
    }

    if (WDRV_PIC32MZW_STATUS_OK != WDRV_PIC32MZW_BSSFindFirst(atCmdAppContext.wdrvHandle, atCmdAppContext.wscnChannel, true, NULL, _BSSFindNotifyCallback))
    {
        return ATCMD_APP_STATUS_WIFI_API_REQUEST_FAILED;
    }
//#endif
    
    atCmdAppContext.wscnScanInProgress = true;

    return ATCMD_STATUS_OK;
}

static ATCMD_STATUS _WSCNPExecute(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const int numParams, ATCMD_PARAM *pParamList)
{
    ATCMD_STATUS retStatus;

    retStatus = _WSCNCommonExecute(pCmdTypeDesc, numParams, pParamList);

    if (ATCMD_STATUS_OK != retStatus)
    {
        return retStatus;
    }

//#ifdef ANY_CLOUD_RN
    if (WDRV_PIC32MZW_STATUS_OK != WDRV_PIC32MZW_BSSFindSetScanParameters(atCmdAppContext.wdrvHandle, 
							2, (uint16_t)atCmdAppContext.wscnConf.activeScanTime, 
							(uint16_t)atCmdAppContext.wscnConf.passiveListenTime, DRV_PIC32MZW_SCAN_MAX_NUM_PROBE))
    {
        return ATCMD_APP_STATUS_WIFI_API_REQUEST_FAILED;
    }

    if (WDRV_PIC32MZW_STATUS_OK != WDRV_PIC32MZW_BSSFindFirst(atCmdAppContext.wdrvHandle, atCmdAppContext.wscnChannel, false, NULL, _BSSFindNotifyCallback))
    {
        return ATCMD_APP_STATUS_WIFI_API_REQUEST_FAILED;
    }
//#endif

    
    atCmdAppContext.wscnScanInProgress = true;

    return ATCMD_STATUS_OK;
}

/*******************************************************************************
* Command update functions
*******************************************************************************/
static ATCMD_STATUS _WSCNUpdate(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const AT_CMD_TYPE_DESC* pCurrentCmdTypeDesc)
{
    WDRV_PIC32MZW_BSS_INFO bssInfo;

    if (OSAL_SEM_GetCount(&bssFindResult) > 0)
    {
        if (ATCMD_PlatformUARTWriteGetSpace() > 128)
        {
            if (OSAL_RESULT_TRUE == OSAL_SEM_Pend(&bssFindResult, 0))
            {
                if (WDRV_PIC32MZW_STATUS_OK == WDRV_PIC32MZW_BSSFindGetInfo(atCmdAppContext.wdrvHandle, &bssInfo))
                {
                    int authType;

                    authType = -1;

                    switch (bssInfo.authTypeRecommended)
                    {
                        case WDRV_PIC32MZW_AUTH_TYPE_OPEN:
                        {
                            authType = 0;
                            break;
                        }

                        case WDRV_PIC32MZW_AUTH_TYPE_WEP:
                        {
                            authType = 1;
                            break;
                        }

                        case WDRV_PIC32MZW_AUTH_TYPE_WPAWPA2_PERSONAL:
                        {
                            authType = 2;
                            break;
                        }

                        case WDRV_PIC32MZW_AUTH_TYPE_WPA2_PERSONAL:
                        {
                            authType = 3;
                            break;
                        }

                        default:
                        {
                            break;
                        }
                    }

                    if (authType >= 0)
                    {
                        ATCMD_Printf("+WSCNIND:%d,%d,%d,", bssInfo.rssi, authType, bssInfo.ctx.channel);
                        ATCMD_PrintMACAddress(bssInfo.ctx.bssid.addr);
                        ATCMD_Print(",", 1);
                        ATCMD_PrintStringSafe((char*)bssInfo.ctx.ssid.name, bssInfo.ctx.ssid.length);
                        ATCMD_Printf("\r\n");
                    }

                    if (WDRV_PIC32MZW_STATUS_BSS_FIND_END == WDRV_PIC32MZW_BSSFindNext(atCmdAppContext.wdrvHandle, _BSSFindNotifyCallback))
                    {
                        atCmdAppContext.wscnScanInProgress = false;
						ATCMD_Print("+WSCNDONE\r\n", 11);
						atCmdAppContext.respond_to_app = 2;
                    }
                }
            }
        }
    }

    return ATCMD_STATUS_OK;
}
