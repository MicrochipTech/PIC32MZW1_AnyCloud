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
#include "tcpip/src/tcpip_manager_control.h"
#include "tcpip/dhcps.h"

/*******************************************************************************
* Command interface prototypes
*******************************************************************************/
static ATCMD_STATUS _WAPInit(const AT_CMD_TYPE_DESC* pCmdTypeDesc);
static ATCMD_STATUS _WAPCExecute(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const int numParams, ATCMD_PARAM *pParamList);
static ATCMD_STATUS _WAPExecute(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const int numParams, ATCMD_PARAM *pParamList);
static ATCMD_STATUS _WAPUpdate(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const AT_CMD_TYPE_DESC* pCurrentCmdTypeDesc);

/*******************************************************************************
* Command parameters
*******************************************************************************/
static const ATCMD_HELP_PARAM paramID =
    {"ID", "Parameter ID number", ATCMD_PARAM_TYPE_CLASS_INTEGER, 0};

static const ATCMD_HELP_PARAM paramVAL =
    {"VAL", "Parameter value", ATCMD_PARAM_TYPE_CLASS_ANY, 0};

static const ATCMD_HELP_PARAM paramSTATE =
    {"STATE", "State of the hotspot feature", ATCMD_PARAM_TYPE_INTEGER,
        .numOpts = 2,
        {
            {"0", "Disable"},
            {"1", "Enable"}
        }
    };

/*******************************************************************************
* Command examples
*******************************************************************************/

/*******************************************************************************
* Command descriptors
*******************************************************************************/
const AT_CMD_TYPE_DESC atCmdTypeDescWAPC =
    {
        .pCmdName   = "+WAPC",
        .cmdInit    = _WAPInit,
        .cmdExecute = _WAPCExecute,
        .cmdUpdate  = NULL,
        .pSummary   = "This command is used to read or set the DCE's hotspot access point configuration",
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

const AT_CMD_TYPE_DESC atCmdTypeDescWAP =
    {
        .pCmdName   = "+WAP",
        .cmdInit    = NULL,
        .cmdExecute = _WAPExecute,
        .cmdUpdate  = _WAPUpdate,
        .pSummary   = "This command is used to enable the DCE's hotspot access point functionality",
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
#define WAPC_MAP_MAX_PARAMS     11

/*******************************************************************************
* Local data
*******************************************************************************/
static const ATCMD_STORE_MAP_ELEMENT wapConfMap[] = {
    {1,  offsetof(ATCMD_APP_WAP_CONF, ssid),                ATCMD_STORE_TYPE_STRING,     AT_CMD_SSID_SZ,    ATCMD_STORE_ACCESS_RW},
    {2,  offsetof(ATCMD_APP_WAP_CONF, secType),             ATCMD_STORE_TYPE_INT,        sizeof(int),       ATCMD_STORE_ACCESS_RW},
    {3,  offsetof(ATCMD_APP_WAP_CONF, credentials),         ATCMD_STORE_TYPE_STRING,     AT_CMD_CRED_SZ,    ATCMD_STORE_ACCESS_RW},
    {4,  offsetof(ATCMD_APP_WAP_CONF, channel),             ATCMD_STORE_TYPE_INT,        sizeof(int),       ATCMD_STORE_ACCESS_RW},
    {5,  offsetof(ATCMD_APP_WAP_CONF, hidden),              ATCMD_STORE_TYPE_BOOL,       1,                 ATCMD_STORE_ACCESS_RW},
    {6,  offsetof(ATCMD_APP_WAP_CONF, ipAddr),              ATCMD_STORE_TYPE_IPV4ADDR,   sizeof(uint32_t),  ATCMD_STORE_ACCESS_RW},
    {7,  offsetof(ATCMD_APP_WAP_CONF, netMask),             ATCMD_STORE_TYPE_IPV4ADDR,   sizeof(uint32_t),  ATCMD_STORE_ACCESS_RW},
    {8,  offsetof(ATCMD_APP_WAP_CONF, dfltRouter),          ATCMD_STORE_TYPE_IPV4ADDR,   sizeof(uint32_t),  ATCMD_STORE_ACCESS_RW},
    {9,  offsetof(ATCMD_APP_WAP_CONF, dnsSvr1),             ATCMD_STORE_TYPE_IPV4ADDR,   sizeof(uint32_t),  ATCMD_STORE_ACCESS_RW},
    {10, offsetof(ATCMD_APP_WAP_CONF, dhcpServerEnabled),   ATCMD_STORE_TYPE_BOOL,       1,                 ATCMD_STORE_ACCESS_RW},
    {11, offsetof(ATCMD_APP_WAP_CONF, dhcpServerPoolStart), ATCMD_STORE_TYPE_IPV4ADDR,   sizeof(uint32_t),  ATCMD_STORE_ACCESS_RW},
    {0,  0,                                                 ATCMD_STORE_TYPE_INVALID,    0,                 ATCMD_STORE_ACCESS_RW}
};

static TCPIP_DHCPS_ADDRESS_CONFIG dhcpsPoolConfig=
{
    .interfaceIndex     = TCPIP_DHCP_SERVER_INTERFACE_INDEX_IDX0,
    .serverIPAddress    = NULL,
    .startIPAddRange    = NULL,
    .ipMaskAddress      = NULL,
//    .gatewayAddress     = NULL,
    .priDNS             = NULL,
    .secondDNS          = NULL,
    .poolEnabled        = TCPIP_DHCP_SERVER_POOL_ENABLED_IDX0,
};

/*******************************************************************************
* Local functions
*******************************************************************************/
static void _APResetSTAState(ATCMD_APP_WAP_STA_STATE *pStaState)
{
    if (NULL == pStaState)
    {
        return;
    }

    pStaState->assocHandle = WDRV_PIC32MZW_ASSOC_HANDLE_INVALID;
    pStaState->connected   = false;
    pStaState->ipAddr      = 0;
    pStaState->bssid.valid = false;
}

static ATCMD_APP_WAP_STA_STATE* _APFindAssociation(WDRV_PIC32MZW_ASSOC_HANDLE assocHandle)
{
    int i;

    for (i=0; i<WDRV_PIC32MZW_NUM_ASSOCS; i++)
    {
        if (atCmdAppContext.wapConnState.sta[i].assocHandle == assocHandle)
        {
            return &atCmdAppContext.wapConnState.sta[i];
        }
    }

    return NULL;
}

static void _APClearAssociation(WDRV_PIC32MZW_ASSOC_HANDLE assocHandle)
{
    int i;

    for (i=0; i<WDRV_PIC32MZW_NUM_ASSOCS; i++)
    {
        if (atCmdAppContext.wapConnState.sta[i].assocHandle == assocHandle)
        {
            _APResetSTAState(&atCmdAppContext.wapConnState.sta[i]);
            return;
        }
    }
}

static void _APNotifyCallback(DRV_HANDLE handle, WDRV_PIC32MZW_ASSOC_HANDLE assocHandle, WDRV_PIC32MZW_CONN_STATE currentState)
{
    ATCMD_APP_WAP_STA_STATE *pStaState;

    pStaState = _APFindAssociation(assocHandle);

    if (WDRV_PIC32MZW_CONN_STATE_CONNECTED == currentState)
    {
        if (NULL == pStaState)
        {
            pStaState = _APFindAssociation(WDRV_PIC32MZW_ASSOC_HANDLE_INVALID);

            if (NULL == pStaState)
            {
                /* Recycle old associations? */
                return;
            }
        }

        if (false == pStaState->connected)
        {
            if (WDRV_PIC32MZW_STATUS_OK == WDRV_PIC32MZW_AssocPeerAddressGet(assocHandle, &pStaState->bssid))
            {
                ATCMD_Printf("+WAPSC:");
                ATCMD_PrintMACAddress(pStaState->bssid.addr);
                ATCMD_Print("\r\n", 2);
            }
            else
            {
                pStaState->bssid.valid = false;
            }

            pStaState->connected   = true;
            pStaState->assocHandle = assocHandle;
            pStaState->ipAddr      = 0;
        }
    }
    else if (WDRV_PIC32MZW_CONN_STATE_DISCONNECTED == currentState)
    {
        if (NULL == pStaState)
        {
            return;
        }

        if (true == pStaState->connected)
        {
            ATCMD_Printf("+WAPSD:");
            ATCMD_PrintMACAddress(pStaState->bssid.addr);
            ATCMD_Print("\r\n", 2);

            if (0 != pStaState->ipAddr)
            {
                TCPIP_DHCPS_LeaseEntryRemove(atCmdAppContext.netHandle, (TCPIP_MAC_ADDR*)pStaState->bssid.addr);
            }

            _APClearAssociation(assocHandle);
        }
    }
}

ATCMD_STATUS ATCMD_WAP_Start(bool activeProvisioning)
{
    WDRV_PIC32MZW_AUTH_CONTEXT authCtx;
    WDRV_PIC32MZW_BSS_CONTEXT  bssCtx;
    TCPIP_NET_IF *pNetIf;

    /* Initialize the BSS context to use default values. */

    if (WDRV_PIC32MZW_STATUS_OK != WDRV_PIC32MZW_BSSCtxSetDefaults(&bssCtx))
    {
        return ATCMD_APP_STATUS_WIFI_API_REQUEST_FAILED;
    }

    /* Update BSS context with target SSID for connection. */

    if (WDRV_PIC32MZW_STATUS_OK != WDRV_PIC32MZW_BSSCtxSetSSID(&bssCtx, (uint8_t*)&atCmdAppContext.wapConf.ssid[1], atCmdAppContext.wapConf.ssid[0]))
    {
        return ATCMD_STATUS_INVALID_PARAMETER;
    }

    if (WDRV_PIC32MZW_STATUS_OK != WDRV_PIC32MZW_BSSCtxSetChannel(&bssCtx, atCmdAppContext.wapConf.channel))
    {
        return ATCMD_STATUS_INVALID_PARAMETER;
    }

    if (WDRV_PIC32MZW_STATUS_OK != WDRV_PIC32MZW_BSSCtxSetSSIDVisibility(&bssCtx, atCmdAppContext.wapConf.hidden ? false : true))
    {
        return ATCMD_STATUS_INVALID_PARAMETER;
    }

    /* Initialize authentication context. */

    if (WDRV_PIC32MZW_STATUS_OK != WDRV_PIC32MZW_AuthCtxSetDefaults(&authCtx))
    {
        return ATCMD_APP_STATUS_WIFI_API_REQUEST_FAILED;
    }

    switch (atCmdAppContext.wapConf.secType)
    {
        case 0:     // Open
        {
            /* Initialize the authentication context for open mode. */

            if (WDRV_PIC32MZW_STATUS_OK != WDRV_PIC32MZW_AuthCtxSetOpen(&authCtx))
            {
                return ATCMD_APP_STATUS_WIFI_API_REQUEST_FAILED;
            }

            break;
        }

        case 1:     // WEP
        {
            char* pWEPIdx;
            char* pWEPKey;
            uint8_t wepIdx;

            pWEPIdx = strtok((char*)&atCmdAppContext.wapConf.credentials[1], "*");

            if (NULL == pWEPIdx)
            {
                return ATCMD_STATUS_INVALID_PARAMETER;
            }

            pWEPKey = strtok(NULL, "\0");

            if (NULL == pWEPKey)
            {
                return ATCMD_STATUS_INVALID_PARAMETER;
            }

            /* Initialize the authentication context for WEP. */

            wepIdx = strtol(pWEPIdx, NULL, 0);

            if (WDRV_PIC32MZW_STATUS_OK != WDRV_PIC32MZW_AuthCtxSetWEP(&authCtx, wepIdx, (uint8_t*)pWEPKey, strlen(pWEPKey)))
            {
                return ATCMD_STATUS_INVALID_PARAMETER;
            }

            break;
        }

        case 2:     // WPA PSK
        case 3:     // WPA2 PSK
        {
            /* Initialize the authentication context for WPA. */

            if (WDRV_PIC32MZW_STATUS_OK != WDRV_PIC32MZW_AuthCtxSetPersonal(&authCtx, (uint8_t*)&atCmdAppContext.wapConf.credentials[1], atCmdAppContext.wapConf.credentials[0], WDRV_PIC32MZW_AUTH_TYPE_WPAWPA2_PERSONAL))
            {
                return ATCMD_STATUS_INVALID_PARAMETER;
            }

            break;
        }

        case 4:     // Enterprise
        {
            /* TODO */
            return ATCMD_APP_STATUS_UNSUPPORTTED_SEC_TYPE;
        }
    }

    if (false == activeProvisioning)
    {
        /* TODO */
    }
    else
    {
        /* TODO */
    }

	TCPIP_DHCP_Disable(atCmdAppContext.netHandle);
	
    if (false == TCPIP_STACK_NetAddressSet(atCmdAppContext.netHandle, (IPV4_ADDR*)&atCmdAppContext.wapConf.ipAddr, (IPV4_ADDR*)&atCmdAppContext.wapConf.netMask, false))
    {
    	SYS_CONSOLE_PRINT("TCPIP_STACK_NetAddressSet failed\n");
        return ATCMD_APP_STATUS_NETWORK_ERROR;
    }

    pNetIf = _TCPIPStackHandleToNet(atCmdAppContext.netHandle);

    if (NULL == pNetIf)
    {
    	SYS_CONSOLE_PRINT("_TCPIPStackHandleToNet failed\n");
		return ATCMD_APP_STATUS_NETWORK_ERROR;
    }

    /* Set static default gateway */
    TCPIP_STACK_GatewayAddressSet(pNetIf, (IPV4_ADDR*)&atCmdAppContext.wapConf.dfltRouter);

    /* Set primary DNS server address */
    TCPIP_STACK_PrimaryDNSAddressSet(pNetIf, (IPV4_ADDR*)&atCmdAppContext.wapConf.dnsSvr1);

    /* Set secondary DNS server address */
    TCPIP_STACK_SecondaryDNSAddressSet(pNetIf, (IPV4_ADDR*)&atCmdAppContext.wapConf.dnsSvr1);

    if (true == TCPIP_DHCP_IsEnabled(atCmdAppContext.netHandle))
    {
        TCPIP_DHCP_Disable(atCmdAppContext.netHandle);
    }

    if (true == atCmdAppContext.wapConf.dhcpServerEnabled)
    {
        char dhcpsServerIPAddressStr[16];
        char dhcpsStartIPAddressStr[16];
        char dhcpsIPMaskAddressStr[16];
        char dhcpsGatewayAddressStr[16];
        char dhcpsDNS1IPAddressStr[16];

        TCPIP_Helper_IPAddressToString((IPV4_ADDR*)&atCmdAppContext.wapConf.ipAddr, (char*)dhcpsServerIPAddressStr, sizeof(dhcpsServerIPAddressStr));
        TCPIP_Helper_IPAddressToString((IPV4_ADDR*)&atCmdAppContext.wapConf.netMask, (char*)dhcpsIPMaskAddressStr, sizeof(dhcpsIPMaskAddressStr));
        TCPIP_Helper_IPAddressToString((IPV4_ADDR*)&atCmdAppContext.wapConf.dfltRouter, (char*)dhcpsGatewayAddressStr, sizeof(dhcpsGatewayAddressStr));
        TCPIP_Helper_IPAddressToString((IPV4_ADDR*)&atCmdAppContext.wapConf.dnsSvr1, (char*)dhcpsDNS1IPAddressStr, sizeof(dhcpsDNS1IPAddressStr));
        TCPIP_Helper_IPAddressToString((IPV4_ADDR*)&atCmdAppContext.wapConf.dhcpServerPoolStart, (char*)dhcpsStartIPAddressStr, sizeof(dhcpsStartIPAddressStr));

        dhcpsPoolConfig.serverIPAddress = dhcpsServerIPAddressStr;
        dhcpsPoolConfig.startIPAddRange = dhcpsStartIPAddressStr;
        dhcpsPoolConfig.ipMaskAddress   = dhcpsIPMaskAddressStr;
// ANY_CLOUD_RN        dhcpsPoolConfig.gatewayAddress  = dhcpsGatewayAddressStr;
        dhcpsPoolConfig.priDNS          = dhcpsDNS1IPAddressStr;
        dhcpsPoolConfig.secondDNS       = dhcpsDNS1IPAddressStr;

        if (false == TCPIP_DHCPS_Enable(atCmdAppContext.netHandle))
        {
            return ATCMD_APP_STATUS_NETWORK_ERROR;
        }

        if (TCPIP_STACK_ADDRESS_SERVICE_NONE == TCPIP_STACK_AddressServiceSelect(pNetIf, TCPIP_NETWORK_CONFIG_DHCP_SERVER_ON))
        {
            return ATCMD_APP_STATUS_NETWORK_ERROR;
        }
    }

    if (WDRV_PIC32MZW_STATUS_OK != WDRV_PIC32MZW_APStart(atCmdAppContext.wdrvHandle, &bssCtx, &authCtx, &_APNotifyCallback))
    {
        return ATCMD_APP_STATUS_WIFI_API_REQUEST_FAILED;
    }

    atCmdAppContext.wapConnState.wapState = 1;

    return ATCMD_STATUS_OK;
}

ATCMD_STATUS ATCMD_WAP_Stop(void)
{
    TCPIP_DHCPS_RemovePoolEntries(atCmdAppContext.netHandle, DHCP_SERVER_POOL_ENTRY_ALL);

    if (true == TCPIP_DHCPS_IsEnabled(atCmdAppContext.netHandle))
    {
        TCPIP_DHCPS_Disable(atCmdAppContext.netHandle);
    }

    if (WDRV_PIC32MZW_STATUS_OK != WDRV_PIC32MZW_APStop(atCmdAppContext.wdrvHandle))
    {
        return ATCMD_APP_STATUS_WIFI_API_REQUEST_FAILED;
    }

    atCmdAppContext.wapConnState.wapState = 0;

    return ATCMD_STATUS_OK;
}

/*******************************************************************************
* Command init functions
*******************************************************************************/
static ATCMD_STATUS _WAPInit(const AT_CMD_TYPE_DESC* pCmdTypeDesc)
{
    int i;

    memset(&atCmdAppContext.wapConf, 0, sizeof(ATCMD_APP_WAP_CONF));
    memset(&atCmdAppContext.wapConnState, 0, sizeof(ATCMD_APP_WAP_STATE));

    atCmdAppContext.wapConf.channel             = 6;
    atCmdAppContext.wapConf.dhcpServerEnabled   = true;
    atCmdAppContext.wapConf.ipAddr              = 0x0100A8C0;
    atCmdAppContext.wapConf.netMask             = 0x00FFFFFF;
    atCmdAppContext.wapConf.dfltRouter          = 0x0100A8C0;
    atCmdAppContext.wapConf.dnsSvr1             = 0x0100A8C0;
    atCmdAppContext.wapConf.dhcpServerPoolStart = 0x1000A8C0;

    for (i=0; i<WDRV_PIC32MZW_NUM_ASSOCS; i++)
    {
        _APResetSTAState(&atCmdAppContext.wapConnState.sta[i]);
    }

    return ATCMD_STATUS_OK;
}

/*******************************************************************************
* Command execute functions
*******************************************************************************/
static ATCMD_STATUS _WAPCExecute(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const int numParams, ATCMD_PARAM *pParamList)
{
    if (0 == numParams)
    {
        int id;

        /* Dump all configuration elements */

        for (id=1; id<=WAPC_MAP_MAX_PARAMS; id++)
        {
            /* Read the element from the configuration structure */

            ATCMD_StructStorePrint(pCmdTypeDesc->pCmdName, wapConfMap, &atCmdAppContext.wapConf, id);
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

        if (false == ATCMD_StructStorePrint(pCmdTypeDesc->pCmdName, wapConfMap, &atCmdAppContext.wapConf, pParamList[0].value.i))
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

        /* Access the element in the configuration structure */

        if (0 == ATCMD_StructStoreWriteParam(wapConfMap, &atCmdAppContext.wapConf, pParamList[0].value.i, &pParamList[1]))
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

static ATCMD_STATUS _WAPExecute(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const int numParams, ATCMD_PARAM *pParamList)
{
    ATCMD_STATUS retStatus;

    if (0 == numParams)
    {
        if (ATCMD_APP_STATE_AP_STARTED == ATCMD_APPStateMachineCurrentState())
        {
            if (false == atCmdAppContext.wapConnState.sta[0].connected)
            {
                ATCMD_Print("+WAP:1\r\n", 8);
            }
            else
            {
                ATCMD_Print("+WAP:1,", 7);
                ATCMD_PrintMACAddress(atCmdAppContext.wapConnState.sta[0].bssid.addr);
                ATCMD_Printf(",%d", atCmdAppContext.wapConnState.sta[0].rssi);
                ATCMD_Print("\r\n", 2);
            }
        }
        else
        {
            ATCMD_Print("+WAP:0\r\n", 8);
        }

        return ATCMD_STATUS_OK;
    }
    else if (1 == numParams)
    {
        if (false == ATCMD_ParamValidateTypes(pCmdTypeDesc, 1, numParams, pParamList))
        {
            return ATCMD_STATUS_INVALID_PARAMETER;
        }

        switch (pParamList[0].value.i)
        {
            case 0:
            {
                if (false == ATCMD_APPStateMachineEvent(ATCMD_APP_EVENT_AP_STOPPING, false))
                {
                    return ATCMD_APP_STATUS_WAP_STOP_REFUSED;
                }

                retStatus = ATCMD_WAP_Stop();

                if (ATCMD_STATUS_OK != retStatus)
                {
                    return retStatus;
                }

                if (false == ATCMD_APPStateMachineEvent(ATCMD_APP_EVENT_AP_STOPPING, true))
                {
                    return ATCMD_APP_STATUS_WAP_STOP_FAILED;
                }

                break;
            }

            case 1:
            {
                if (false == ATCMD_APPStateMachineEvent(ATCMD_APP_EVENT_AP_STARTING, false))
                {
                    return ATCMD_APP_STATUS_WAP_START_REFUSED;
                }

                retStatus = ATCMD_WAP_Start(false);

                if (ATCMD_STATUS_OK != retStatus)
                {
                    return retStatus;
                }

                if (false == ATCMD_APPStateMachineEvent(ATCMD_APP_EVENT_AP_STARTING, true))
                {
                    return ATCMD_APP_STATUS_WAP_START_FAILED;
                }

                break;
            }

            default:
            {
                return ATCMD_STATUS_INVALID_PARAMETER;
            }
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
static ATCMD_STATUS _WAPUpdate(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const AT_CMD_TYPE_DESC* pCurrentCmdTypeDesc)
{
    int i;

    if (ATCMD_APP_STATE_AP_STARTED != atCmdAppContext.appState)
    {
        return ATCMD_STATUS_OK;
    }

    if (true == atCmdAppContext.wapConf.dhcpServerEnabled)
    {
        for (i=0; i<WDRV_PIC32MZW_NUM_ASSOCS; i++)
        {
            ATCMD_APP_WAP_STA_STATE *pStaState;

            pStaState =&atCmdAppContext.wapConnState.sta[i];

            if (true == pStaState->connected)
            {
                if ((0 == pStaState->ipAddr) && (true == pStaState->bssid.valid))
                {
                    TCPIP_DHCPS_LEASE_HANDLE dhcpsLease = 0;
                    TCPIP_DHCPS_LEASE_ENTRY dhcpsLeaseEntry;

                    do
                    {
                        dhcpsLease = TCPIP_DHCPS_LeaseEntryGet(atCmdAppContext.netHandle, &dhcpsLeaseEntry, dhcpsLease);

                        if ((0 != dhcpsLease) && (0 == memcmp(&dhcpsLeaseEntry.hwAdd, pStaState->bssid.addr, WDRV_PIC32MZW_MAC_ADDR_LEN)))
                        {
                            pStaState->ipAddr = dhcpsLeaseEntry.ipAddress.Val;

                            ATCMD_Print("+WAPAIP:", 8);
                            ATCMD_PrintIPv4Address(dhcpsLeaseEntry.ipAddress.Val);
                            ATCMD_Print("\r\n", 2);
                            break;
                        }
                    }
                    while (0 != dhcpsLease);
                }
            }
        }
    }

    return ATCMD_STATUS_OK;
}
