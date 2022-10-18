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
#include <inttypes.h>
#include <time.h>

#include "at_cmd_app.h"
#include "at_cmd_sys_time.h"
#include "tcpip/src/tcpip_manager_control.h"
#include "tcpip/dns.h"

/*******************************************************************************
* Command interface prototypes
*******************************************************************************/
static ATCMD_STATUS _WSTAInit(const AT_CMD_TYPE_DESC* pCmdTypeDesc);
static ATCMD_STATUS _WSTACExecute(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const int numParams, ATCMD_PARAM *pParamList);
static ATCMD_STATUS _WSTAExecute(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const int numParams, ATCMD_PARAM *pParamList);
static ATCMD_STATUS _WSTAUpdate(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const AT_CMD_TYPE_DESC* pCurrentCmdTypeDesc);

/*******************************************************************************
* Command parameters
*******************************************************************************/
static const ATCMD_HELP_PARAM paramID =
    {"ID", "Parameter ID number", ATCMD_PARAM_TYPE_CLASS_INTEGER, 0};

static const ATCMD_HELP_PARAM paramVAL =
    {"VAL", "Parameter value", ATCMD_PARAM_TYPE_CLASS_ANY, 0};

static const ATCMD_HELP_PARAM paramSTATE =
    {"STATE", "State of the Wi-Fi station feature", ATCMD_PARAM_TYPE_INTEGER,
        .numOpts = 3,
        {
            {"0", "Disable"},
            {"1", "Use configuration from +WSTAC command"},
            {"2", "Attempt re-establishment of last successful connection"}
        }
    };

static const ATCMD_HELP_PARAM paramTIMEOUT =
    {"TIMEOUT", "State change timeout", ATCMD_PARAM_TYPE_CLASS_INTEGER, 0};

/*******************************************************************************
* Command examples
*******************************************************************************/

/*******************************************************************************
* Command descriptors
*******************************************************************************/
const AT_CMD_TYPE_DESC atCmdTypeDescWSTAC =
    {
        .pCmdName   = "+WSTAC",
        .cmdInit    = _WSTAInit,
        .cmdExecute = _WSTACExecute,
        .cmdUpdate  = NULL,
        .pSummary   = "This command is used to read or set the DCE's Wi-Fi station mode configuration",
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

const AT_CMD_TYPE_DESC atCmdTypeDescWSTA =
    {
        .pCmdName   = "+WSTA",
        .cmdInit    = NULL,
        .cmdExecute = _WSTAExecute,
        .cmdUpdate  = _WSTAUpdate,
        .pSummary   = "This command is used to enable the DCE's station mode functionality",
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
                    &paramSTATE
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
                    &paramSTATE,
                    &paramTIMEOUT
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
#define WSTAC_MAP_MAX_PARAMS    15

/*******************************************************************************
* Local data
*******************************************************************************/
static const ATCMD_STORE_MAP_ELEMENT wstaConfMap[] = {
    {1,  offsetof(ATCMD_APP_WSTA_CONF, ssid),               ATCMD_STORE_TYPE_STRING,     AT_CMD_SSID_SZ,    ATCMD_STORE_ACCESS_RW},
    {2,  offsetof(ATCMD_APP_WSTA_CONF, secType),            ATCMD_STORE_TYPE_INT,        sizeof(int),       ATCMD_STORE_ACCESS_RW},
    {3,  offsetof(ATCMD_APP_WSTA_CONF, credentials),        ATCMD_STORE_TYPE_STRING,     AT_CMD_CRED_SZ,    ATCMD_STORE_ACCESS_RW},
    {4,  offsetof(ATCMD_APP_WSTA_CONF, channel),            ATCMD_STORE_TYPE_INT,        sizeof(int),       ATCMD_STORE_ACCESS_RW},
    {5,  offsetof(ATCMD_APP_WSTA_CONF, bssid),              ATCMD_STORE_TYPE_STRING,     6,                 ATCMD_STORE_ACCESS_RW},
    {6,  offsetof(ATCMD_APP_WSTA_CONF, ipAddr),             ATCMD_STORE_TYPE_IPV4ADDR,   sizeof(uint32_t),  ATCMD_STORE_ACCESS_RW},
    {7,  offsetof(ATCMD_APP_WSTA_CONF, netMask),            ATCMD_STORE_TYPE_IPV4ADDR,   sizeof(uint32_t),  ATCMD_STORE_ACCESS_RW},
    {8,  offsetof(ATCMD_APP_WSTA_CONF, dfltRouter),         ATCMD_STORE_TYPE_IPV4ADDR,   sizeof(uint32_t),  ATCMD_STORE_ACCESS_RW},
    {9,  offsetof(ATCMD_APP_WSTA_CONF, dnsSvr1),            ATCMD_STORE_TYPE_IPV4ADDR,   sizeof(uint32_t),  ATCMD_STORE_ACCESS_RW},
    {10, offsetof(ATCMD_APP_WSTA_CONF, dnsSvr2),            ATCMD_STORE_TYPE_IPV4ADDR,   sizeof(uint32_t),  ATCMD_STORE_ACCESS_RW},
    {11, offsetof(ATCMD_APP_WSTA_CONF, dhcpClientEnabled),  ATCMD_STORE_TYPE_BOOL,       1,                 ATCMD_STORE_ACCESS_RW},
    {12, offsetof(ATCMD_APP_WSTA_CONF, ntpSvr),             ATCMD_STORE_TYPE_STRING,     AT_CMD_NTPSRV_SZ,  ATCMD_STORE_ACCESS_RW},
    {13, offsetof(ATCMD_APP_WSTA_CONF, ntpStatic),          ATCMD_STORE_TYPE_BOOL,       1,                 ATCMD_STORE_ACCESS_RW},
    {14, offsetof(ATCMD_APP_WSTA_CONF, ntpClient),          ATCMD_STORE_TYPE_BOOL,       1,                 ATCMD_STORE_ACCESS_RW},
    {15, offsetof(ATCMD_APP_WSTA_CONF, roaming),            ATCMD_STORE_TYPE_BOOL,       1,                 ATCMD_STORE_ACCESS_RW},
    {0,  0,                                                 ATCMD_STORE_TYPE_INVALID,    0,                 ATCMD_STORE_ACCESS_RW}
};

static const ATCMD_STORE_MAP_ELEMENT wstaStateMap[] = {
    {4,  offsetof(ATCMD_APP_WSTA_STATE, channel),           ATCMD_STORE_TYPE_INT,        sizeof(int),       ATCMD_STORE_ACCESS_READ},
    {5,  offsetof(ATCMD_APP_WSTA_STATE, bssid),             ATCMD_STORE_TYPE_STRING,     6,                 ATCMD_STORE_ACCESS_READ},
    {6,  offsetof(ATCMD_APP_WSTA_STATE, ipAddr),            ATCMD_STORE_TYPE_IPV4ADDR,   sizeof(uint32_t),  ATCMD_STORE_ACCESS_READ},
    {7,  offsetof(ATCMD_APP_WSTA_STATE, netMask),           ATCMD_STORE_TYPE_IPV4ADDR,   sizeof(uint32_t),  ATCMD_STORE_ACCESS_READ},
    {8,  offsetof(ATCMD_APP_WSTA_STATE, dfltRouter),        ATCMD_STORE_TYPE_IPV4ADDR,   sizeof(uint32_t),  ATCMD_STORE_ACCESS_READ},
    {9,  offsetof(ATCMD_APP_WSTA_STATE, dnsSvr1),           ATCMD_STORE_TYPE_IPV4ADDR,   sizeof(uint32_t),  ATCMD_STORE_ACCESS_READ},
    {10, offsetof(ATCMD_APP_WSTA_STATE, dnsSvr2),           ATCMD_STORE_TYPE_IPV4ADDR,   sizeof(uint32_t),  ATCMD_STORE_ACCESS_READ},
    {12, offsetof(ATCMD_APP_WSTA_STATE, ntpSvr),            ATCMD_STORE_TYPE_IPV4ADDR,   sizeof(uint32_t),  ATCMD_STORE_ACCESS_READ},
    {0,  0,                                                 ATCMD_STORE_TYPE_INVALID,    0,                 ATCMD_STORE_ACCESS_READ}
};

static WDRV_PIC32MZW_AUTH_CONTEXT authCtx;
static WDRV_PIC32MZW_BSS_CONTEXT  bssCtx;
static bool assocInfoPending;
static TCPIP_DNS_HANDLE dnsResolveHandle;
static bool ntpSrvResolved;
static uint32_t dnsResolveStartMs;

/*******************************************************************************
* Local functions
*******************************************************************************/
static void _DNSResolvResetQuery(void)
{
    if (NULL != dnsResolveHandle)
    {
        TCPIP_DNS_HandlerDeRegister(dnsResolveHandle);
    }

    dnsResolveHandle = NULL;
    ntpSrvResolved = false;
}

static void _DNSResolvEventHandler(TCPIP_NET_HANDLE hNet, TCPIP_DNS_EVENT_TYPE evType, const char* pName, const void* hParam)
{
    if (TCPIP_DNS_EVENT_NAME_RESOLVED == evType)
    {
        if (0 == memcmp(pName, &atCmdAppContext.wstaConf.ntpSvr[1], atCmdAppContext.wstaConf.ntpSvr[0]))
        {
            ntpSrvResolved = true;
        }
    }
}

static void _SNTPSetup(void)
{
    if (false == atCmdAppContext.wstaConf.ntpClient)
    {
        return;
    }

    if (false == atCmdAppContext.wstaConf.ntpStatic)
    {
        TCPIP_DHCP_INFO dhcpInfo;
        char ipAddrStr[20];

        if (false == atCmdAppContext.wstaConf.dhcpClientEnabled)
        {
            return;
        }

        TCPIP_DHCP_InfoGet(atCmdAppContext.netHandle, &dhcpInfo);
		
        if (0 == dhcpInfo.ntpServersNo)
        {
            return;
        }

        TCPIP_Helper_IPAddressToString(&dhcpInfo.ntpServers[0], &ipAddrStr[0], sizeof(ipAddrStr));

        if (SNTP_RES_OK != TCPIP_SNTP_ConnectionParamSet(atCmdAppContext.netHandle, IP_ADDRESS_TYPE_IPV4, ipAddrStr))
        {
            return;
        }

        atCmdAppContext.wstaConnState.ntpSvr = dhcpInfo.ntpServers[0].Val;
    }
    else if (atCmdAppContext.wstaConf.ntpSvr[0] > 0)
    {
        IPV4_ADDR ntpSvrAddr;
		
        if (false == TCPIP_Helper_StringToIPAddress((char*)&atCmdAppContext.wstaConf.ntpSvr[1], &ntpSvrAddr))
        {
            dnsResolveHandle = TCPIP_DNS_HandlerRegister(atCmdAppContext.netHandle, &_DNSResolvEventHandler, &atCmdAppContext.wstaConf.ntpSvr[1]);

            if (NULL != dnsResolveHandle)
            {
                TCPIP_DNS_RESULT dnsResult;

                dnsResult = TCPIP_DNS_Send_Query((char*)&atCmdAppContext.wstaConf.ntpSvr[1], TCPIP_DNS_TYPE_A);

                if (TCPIP_DNS_RES_PENDING == dnsResult)
                {
                    dnsResolveStartMs = ATCMD_PlatformGetSysTimeMs();
                }
                else
                {
                    _DNSResolvResetQuery();
                }
            }

            return;
        }
        else
        {
            if (SNTP_RES_OK != TCPIP_SNTP_ConnectionParamSet(atCmdAppContext.netHandle, IP_ADDRESS_TYPE_IPV4, (char*)&atCmdAppContext.wstaConf.ntpSvr[1]))
            {
                return;
            }

            atCmdAppContext.wstaConnState.ntpSvr = ntpSvrAddr.Val;
        }
    }

    /* Kick SNTP client to attempt a new connection */
    TCPIP_SNTP_ConnectionInitiate();
 }

static void _DHCPCallback(TCPIP_NET_HANDLE hNet, TCPIP_DHCP_EVENT_TYPE evType, const void *param)
{		
    if (false == atCmdAppContext.wstaConf.dhcpClientEnabled)
    {
        return;
    }

    if (DHCP_EVENT_BOUND == evType)
    {
        if (ATCMD_APP_STATE_STA_CONNECTED == atCmdAppContext.appState)
        {
            atCmdAppContext.wstaConnState.ipAddr     = TCPIP_STACK_NetAddress(atCmdAppContext.netHandle);
            atCmdAppContext.wstaConnState.netMask    = TCPIP_STACK_NetMask(atCmdAppContext.netHandle);
            atCmdAppContext.wstaConnState.dfltRouter = TCPIP_STACK_NetAddressGateway(atCmdAppContext.netHandle);
            atCmdAppContext.wstaConnState.dnsSvr1    = TCPIP_STACK_NetAddressDnsPrimary(atCmdAppContext.netHandle);
            atCmdAppContext.wstaConnState.dnsSvr2    = TCPIP_STACK_NetAddressDnsSecond(atCmdAppContext.netHandle);

            ATCMD_Print("+WSTAAIP:", 9);
            ATCMD_PrintIPv4Address(atCmdAppContext.wstaConnState.ipAddr);
            ATCMD_Print("\r\n", 2);

            _SNTPSetup();
        }
    }
}

static void _SNTPCallback(TCPIP_SNTP_EVENT evType, const void *evParam)
{
    if (TCPIP_SNTP_EVENT_TSTAMP_OK == evType)
    {
        ATCMD_SysTimeDisplayTimeAEC(atCmdAppContext.timeFormat, ((TCPIP_SNTP_EVENT_TIME_DATA *)evParam)->tUnixSeconds);
        ATCMD_SysTimeSetNTPSync(true);
    }
    else
    {
        //ATCMD_Printf("Could not get time %d\r\n", evType);
    }
}

static ATCMD_STATUS _BSSDisconnect(bool initiate)
{
    if (true == TCPIP_DHCP_IsEnabled(atCmdAppContext.netHandle))
    {
        TCPIP_DHCP_Disable(atCmdAppContext.netHandle);

        TCPIP_DHCP_HandlerDeRegister(atCmdAppContext.dhcpcHandle);
    }

    if (0 != atCmdAppContext.sntpHandle)
    {
        /* Deregister SNTP client callback handler */
        TCPIP_SNTP_HandlerDeRegister(atCmdAppContext.sntpHandle);

        atCmdAppContext.sntpHandle = 0;
    }

    if (true == initiate)
    {
        if (WDRV_PIC32MZW_STATUS_OK != WDRV_PIC32MZW_BSSDisconnect(atCmdAppContext.wdrvHandle))
        {
            return ATCMD_APP_STATUS_WIFI_API_REQUEST_FAILED;
        }
    }

    atCmdAppContext.wstaStateTimeout = 0;

    return ATCMD_STATUS_OK;
}

void DHCPDebugPrint(const char *func, uint32_t line);

static void _ConnectNotifyCallback(DRV_HANDLE handle, WDRV_PIC32MZW_ASSOC_HANDLE assocHandle, WDRV_PIC32MZW_CONN_STATE currentState)
{
    if (WDRV_PIC32MZW_CONN_STATE_CONNECTED == currentState)
    {
        assocInfoPending            = true;
        atCmdAppContext.assocHandle = assocHandle;

        WDRV_PIC32MZW_PowerSaveBroadcastTrackingSet(atCmdAppContext.wdrvHandle, true);
        WDRV_PIC32MZW_PowerSaveModeSet(atCmdAppContext.wdrvHandle, WDRV_PIC32MZW_POWERSAVE_RUN_MODE,WDRV_PIC32MZW_POWERSAVE_PIC_ASYNC_MODE);

        ATCMD_APPStateMachineEvent(ATCMD_APP_EVENT_STA_CONNECTED, true);
    }
    else if (WDRV_PIC32MZW_CONN_STATE_DISCONNECTED == currentState)
    {
        if (ATCMD_APP_STATE_STA_DISCONNECTING == atCmdAppContext.appState)
        {
            ATCMD_APPStateMachineEvent(ATCMD_APP_EVENT_STA_DISCONNECTED, true);
            ATCMD_Print("+WSTALD\r\n", 9);
        }
        else
        {
            if (ATCMD_APP_STATE_STA_CONNECTED == atCmdAppContext.appState)
            {
                ATCMD_APPStateMachineEvent(ATCMD_APP_EVENT_LINK_LOSS, true);
                ATCMD_Print("+WSTALD\r\n", 9);
            }

            _BSSDisconnect(false);
        }

        memset(&atCmdAppContext.wstaConnState, 0, sizeof(ATCMD_APP_WSTA_STATE));
        atCmdAppContext.assocHandle = WDRV_PIC32MZW_ASSOC_HANDLE_INVALID;
    }
}

static ATCMD_STATUS _BSSConnect(
    const WDRV_PIC32MZW_BSS_CONTEXT *const pBSSCtx,
    const WDRV_PIC32MZW_AUTH_CONTEXT *const pAuthCtx)
{
    TCPIP_NET_IF *pNetIf;

    pNetIf = _TCPIPStackHandleToNet(atCmdAppContext.netHandle);

    if (NULL == pNetIf)
    {
        SYS_CONSOLE_PRINT("_BSSConnect(): Exit 1\r\n");
        return ATCMD_APP_STATUS_NETWORK_ERROR;
    }
    
    if (true == atCmdAppContext.wstaConf.dhcpClientEnabled)
    {
        /* Dynamic IP address allocation, enable DHCP client */
        if (false == TCPIP_DHCP_Enable(atCmdAppContext.netHandle))
        {
            SYS_CONSOLE_PRINT("_BSSConnect(): Exit 2\r\n");
            return ATCMD_APP_STATUS_NETWORK_ERROR;
        }

        /* Select DHCP client service on WiFi interface */
        if (TCPIP_STACK_ADDRESS_SERVICE_NONE == TCPIP_STACK_AddressServiceSelect(pNetIf, TCPIP_NETWORK_CONFIG_DHCP_CLIENT_ON))
        {
            SYS_CONSOLE_PRINT("_BSSConnect(): Exit 3\r\n");
            return ATCMD_APP_STATUS_NETWORK_ERROR;
        }

        /* Register DHCP client callback handler */
        atCmdAppContext.dhcpcHandle = TCPIP_DHCP_HandlerRegister(atCmdAppContext.netHandle, _DHCPCallback, NULL);

        if (0 == atCmdAppContext.dhcpcHandle)
        {
            SYS_CONSOLE_PRINT("_BSSConnect(): Exit 4\r\n");
            return ATCMD_APP_STATUS_NETWORK_ERROR;
        }
    }
    else
    {        
        TCPIP_DHCP_Disable(atCmdAppContext.netHandle);

        /* Set static IP address */
        if (false == TCPIP_STACK_NetAddressSet(atCmdAppContext.netHandle, (IPV4_ADDR*)&atCmdAppContext.wstaConf.ipAddr, (IPV4_ADDR*)&atCmdAppContext.wstaConf.netMask, false))
        {
            SYS_CONSOLE_PRINT("_BSSConnect(): Exit 5\r\n");
            return ATCMD_APP_STATUS_NETWORK_ERROR;
        }

        /* Set static default gateway */
        TCPIP_STACK_GatewayAddressSet(pNetIf, (IPV4_ADDR*)&atCmdAppContext.wstaConf.dfltRouter);

        /* Set primary DNS server address */
        TCPIP_STACK_PrimaryDNSAddressSet(pNetIf, (IPV4_ADDR*)&atCmdAppContext.wstaConf.dnsSvr1);

        /* Set secondary DNS server address */
        TCPIP_STACK_SecondaryDNSAddressSet(pNetIf, (IPV4_ADDR*)&atCmdAppContext.wstaConf.dnsSvr2);
    }

    /* Register SNTP client callback handler */
    if (0 != atCmdAppContext.sntpHandle)
    {
        TCPIP_SNTP_HandlerDeRegister(atCmdAppContext.sntpHandle);

        atCmdAppContext.sntpHandle = 0;
    }

    atCmdAppContext.sntpHandle = TCPIP_SNTP_HandlerRegister(_SNTPCallback);

    if (0 == atCmdAppContext.sntpHandle)
    {
        SYS_CONSOLE_PRINT("_BSSConnect(): Exit 6\r\n");
        return ATCMD_APP_STATUS_NETWORK_ERROR;
    }

    /* Configure NTP server address */
    if (SNTP_RES_OK != TCPIP_SNTP_ConnectionParamSet(atCmdAppContext.netHandle, IP_ADDRESS_TYPE_IPV4, ""))
    {
        SYS_CONSOLE_PRINT("_BSSConnect(): Exit 7\r\n");
        return ATCMD_APP_STATUS_NETWORK_ERROR;
    }

    ntpSrvResolved = false;

    memset(&atCmdAppContext.wstaConnState, 0, sizeof(ATCMD_APP_WSTA_STATE));

    if (WDRV_PIC32MZW_STATUS_OK != WDRV_PIC32MZW_BSSConnect(atCmdAppContext.wdrvHandle, pBSSCtx, pAuthCtx, _ConnectNotifyCallback))
    {
        return ATCMD_APP_STATUS_WIFI_API_REQUEST_FAILED;
    }

    return ATCMD_STATUS_OK;
}

/*******************************************************************************
* Command init functions
*******************************************************************************/
static ATCMD_STATUS _WSTAInit(const AT_CMD_TYPE_DESC* pCmdTypeDesc)
{
    memset(&atCmdAppContext.wstaConf, 0, sizeof(ATCMD_APP_WSTA_CONF));
    memset(&atCmdAppContext.wstaConnState, 0, sizeof(ATCMD_APP_WSTA_STATE));

    atCmdAppContext.wstaConf.channel            = 255;
    atCmdAppContext.wstaConf.dhcpClientEnabled  = true;
    atCmdAppContext.wstaConf.ntpClient          = true;

    WDRV_PIC32MZW_BSSCtxSetDefaults(&bssCtx);
    WDRV_PIC32MZW_AuthCtxSetDefaults(&authCtx);

    assocInfoPending = false;

    dnsResolveHandle = NULL;

    _DNSResolvResetQuery();

    return ATCMD_STATUS_OK;
}

void DHCPDebugPrint(const char *func, uint32_t line);

/*******************************************************************************
* Command execute functions
*******************************************************************************/
static ATCMD_STATUS _WSTACExecute(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const int numParams, ATCMD_PARAM *pParamList)
{
    if (0 == numParams)
    {
#if 0 //TBR        
        size_t deviceCertSize = 1024;
        uint8_t deviceCert[deviceCertSize];
        memset(deviceCert, 0, deviceCertSize);
        MSD_APP_Read_DeviceCertPem((uint8_t*) & deviceCert, &deviceCertSize);
        
        //ATCMD_EnterBinaryMode(NULL);
        ATCMD_Printf("WSTAC:(%d)", deviceCertSize);
        for(id =0; id < deviceCertSize; id++)
            ATCMD_Printf("%c", deviceCert[id]);
        //ATCMD_LeaveBinaryMode();
#endif
        int id;        

        /* Dump all configuration elements */

        for (id=1; id<=WSTAC_MAP_MAX_PARAMS; id++)
        {
            if (0 != atCmdAppContext.wstaConnState.wstaState)
            {
                /* If connected then try and access the ID in the state structure first, only if not found
                   do we fall back to the configuration structure */

                if (true == ATCMD_StructStorePrint("WSTAC", wstaStateMap, &atCmdAppContext.wstaConnState, id))
                {
                    continue;
                }
            }

            /* Read the element from the configuration structure */

            ATCMD_StructStorePrint("WSTAC", wstaConfMap, &atCmdAppContext.wstaConf, id);
        }

		atCmdAppContext.respond_to_app = 2;

        return ATCMD_STATUS_OK;
    }
    else if (1 == numParams)
    {
        /* Check the parameter types are correct */

        if (false == ATCMD_ParamValidateTypes(pCmdTypeDesc, 1, numParams, pParamList))
        {
            return ATCMD_STATUS_INVALID_PARAMETER;
        }

        if (0 != atCmdAppContext.wstaConnState.wstaState)
        {
            /* If connected then try and access the ID in the state structure first, only if not found
                do we fall back to the configuration structure */

            if (true == ATCMD_StructStorePrint(pCmdTypeDesc->pCmdName, wstaStateMap, &atCmdAppContext.wstaConnState, pParamList[0].value.i))
            {
                return ATCMD_STATUS_OK;
            }
        }

        /* Access the element in the configuration structure */

        if (false == ATCMD_StructStorePrint(pCmdTypeDesc->pCmdName, wstaConfMap, &atCmdAppContext.wstaConf, pParamList[0].value.i))
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

        /* If connected then block write access to all elements */

        if (0 != atCmdAppContext.wstaConnState.wstaState)
        {
            return ATCMD_STATUS_STORE_UPDATE_BLOCKED;
        }

        /* Access the element in the configuration structure */

        if (0 == ATCMD_StructStoreWriteParam(wstaConfMap, &atCmdAppContext.wstaConf, pParamList[0].value.i, &pParamList[1]))
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

extern uint8_t allow_task_exec;

static ATCMD_STATUS _WSTAExecute(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const int numParams, ATCMD_PARAM *pParamList)
{
    ATCMD_STATUS retStatus;

    if (0 == numParams)
    {
        ATCMD_Printf("+WSTA:%d", atCmdAppContext.wstaConnState.wstaState);

        if (0 != atCmdAppContext.wstaConnState.wstaState)
        {
            ATCMD_Print(",", 1);
            ATCMD_PrintStringSafe((char*)&atCmdAppContext.wstaConf.ssid[1], atCmdAppContext.wstaConf.ssid[0]);
            ATCMD_Printf(",%d,%d", atCmdAppContext.wstaConnState.rssi, atCmdAppContext.wstaConf.secType);
        }

        ATCMD_Print("\r\n", 2);

        return ATCMD_STATUS_OK;
    }
    else if (numParams < 3)
    {
        if (false == ATCMD_ParamValidateTypes(pCmdTypeDesc, numParams, numParams, pParamList))
        {
            return ATCMD_STATUS_INVALID_PARAMETER;
        }

        atCmdAppContext.wstaStateTimeStartMs = ATCMD_PlatformGetSysTimeMs();
        atCmdAppContext.wstaStateTimeout     = 0;
		
        switch (pParamList[0].value.i)
        {
            case 0:
            {
                if (false == ATCMD_APPStateMachineEvent(ATCMD_APP_EVENT_STA_DISCONNECTING, false))
                {
                    return ATCMD_APP_STATUS_STA_DISCONN_REFUSED;
                }

                retStatus = _BSSDisconnect(true);

                if (ATCMD_STATUS_OK != retStatus)
                {
                    return retStatus;
                }

                if (false == ATCMD_APPStateMachineEvent(ATCMD_APP_EVENT_STA_DISCONNECTING, true))
                {
                    return ATCMD_APP_STATUS_STA_DISCONN_FAILED;
                }

                break;
            }

            case 1:
            {
                WDRV_PIC32MZW_CHANNEL_ID channel;

                if (false == ATCMD_APPStateMachineEvent(ATCMD_APP_EVENT_STA_CONNECTING, false))
                {
                    return ATCMD_APP_STATUS_STA_CONN_REFUSED;
                }

                /* Initialize the BSS context to use default values. */

                if (WDRV_PIC32MZW_STATUS_OK != WDRV_PIC32MZW_BSSCtxSetDefaults(&bssCtx))
                {
                    return ATCMD_APP_STATUS_WIFI_API_REQUEST_FAILED;
                }

                /* Update BSS context with target SSID for connection. */

                if (WDRV_PIC32MZW_STATUS_OK != WDRV_PIC32MZW_BSSCtxSetSSID(&bssCtx, (uint8_t*)&atCmdAppContext.wstaConf.ssid[1], atCmdAppContext.wstaConf.ssid[0]))
                {
                    return ATCMD_STATUS_INVALID_PARAMETER;
                }

                channel = atCmdAppContext.wstaConf.channel;

                if (255 == channel)
                {
                    channel = WDRV_PIC32MZW_CID_ANY;
                }

                if (WDRV_PIC32MZW_STATUS_OK != WDRV_PIC32MZW_BSSCtxSetChannel(&bssCtx, channel))
                {
                    return ATCMD_STATUS_INVALID_PARAMETER;
                }

                switch (atCmdAppContext.wstaConf.secType)
                {
                    case 0:
                    {
                        /* Initialize the authentication context for open mode. */

                        if (WDRV_PIC32MZW_STATUS_OK != WDRV_PIC32MZW_AuthCtxSetOpen(&authCtx))
                        {
                            return ATCMD_APP_STATUS_WIFI_API_REQUEST_FAILED;
                        }
                        break;
                    }

                    case 1:
                    {
                        /* Initialize the authentication context for WEP. */
                        return ATCMD_STATUS_INVALID_PARAMETER;
                        break;
                    }

                    case 2:
                    {
                        /* Initialize the authentication context for WPA/WPA2. */

                        if (WDRV_PIC32MZW_STATUS_OK != WDRV_PIC32MZW_AuthCtxSetPersonal(&authCtx, (uint8_t*)&atCmdAppContext.wstaConf.credentials[1], atCmdAppContext.wstaConf.credentials[0], WDRV_PIC32MZW_AUTH_TYPE_WPAWPA2_PERSONAL))
                        {
                            return ATCMD_STATUS_INVALID_PARAMETER;
                        }
                        break;
                    }

                    case 3:
                    {
                        /* Initialize the authentication context for WPA2. */

                        if (WDRV_PIC32MZW_STATUS_OK != WDRV_PIC32MZW_AuthCtxSetPersonal(&authCtx, (uint8_t*)&atCmdAppContext.wstaConf.credentials[1], atCmdAppContext.wstaConf.credentials[0], WDRV_PIC32MZW_AUTH_TYPE_WPA2_PERSONAL))
                        {
                            return ATCMD_STATUS_INVALID_PARAMETER;
                        }
                        break;
                    }

#ifdef WDRV_PIC32MZW_WPA3_SUPPORT

                    case 4:
                    {
                        /* Initialize the authentication context for WPA2/WPA3. */

                        if (WDRV_PIC32MZW_STATUS_OK != WDRV_PIC32MZW_AuthCtxSetPersonal(&authCtx, (uint8_t*)&atCmdAppContext.wstaConf.credentials[1], atCmdAppContext.wstaConf.credentials[0], WDRV_PIC32MZW_AUTH_TYPE_WPA2WPA3_PERSONAL))
                        {
                            return ATCMD_STATUS_INVALID_PARAMETER;
                        }
                        break;
                    }

                    case 5:
                    {
                        /* Initialize the authentication context for WPA2/WPA3. */

                        if (WDRV_PIC32MZW_STATUS_OK != WDRV_PIC32MZW_AuthCtxSetPersonal(&authCtx, (uint8_t*)&atCmdAppContext.wstaConf.credentials[1], atCmdAppContext.wstaConf.credentials[0], WDRV_PIC32MZW_AUTH_TYPE_WPA3_PERSONAL))
                        {
                            return ATCMD_STATUS_INVALID_PARAMETER;
                        }
                        break;
                    }

#endif

                    default:
                    {
                        return ATCMD_STATUS_INVALID_PARAMETER;
                    }
                }

                retStatus = _BSSConnect(&bssCtx, &authCtx);
				
				if (ATCMD_STATUS_OK != retStatus)
                {
                    return retStatus;
                }

                if (false == ATCMD_APPStateMachineEvent(ATCMD_APP_EVENT_STA_CONNECTING, true))
                {
                    return ATCMD_APP_STATUS_STA_CONN_FAILED;
                }

                break;
            }

            case 2:
            {
                if (false == ATCMD_APPStateMachineEvent(ATCMD_APP_EVENT_STA_CONNECTING, false))
                {
                    return ATCMD_APP_STATUS_STA_CONN_REFUSED;
                }

                retStatus = _BSSConnect(&bssCtx, &authCtx);

                if (ATCMD_STATUS_OK != retStatus)
                {
                    return retStatus;
                }

                if (false == ATCMD_APPStateMachineEvent(ATCMD_APP_EVENT_STA_CONNECTING, true))
                {
                    return ATCMD_APP_STATUS_STA_CONN_FAILED;
                }
                break;
            }

            default:
            {
                return ATCMD_STATUS_INVALID_PARAMETER;
            }
        }

        if (2 == numParams)
        {
            atCmdAppContext.wstaStateTimeout = pParamList[1].value.i;
        }

        atCmdAppContext.wstaConnState.wstaState = pParamList[0].value.i;
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
static ATCMD_STATUS _WSTAUpdate(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const AT_CMD_TYPE_DESC* pCurrentCmdTypeDesc)
{
    if (ATCMD_APP_STATE_STA_CONNECTED == atCmdAppContext.appState)
    {
        if (true == assocInfoPending)
        {
            if (0 == atCmdAppContext.wstaConnState.bssid[0])
            {
                WDRV_PIC32MZW_MAC_ADDR bssAddr;

                if (WDRV_PIC32MZW_STATUS_OK == WDRV_PIC32MZW_AssocPeerAddressGet(atCmdAppContext.assocHandle, &bssAddr))
                {
                    if (true == bssAddr.valid)
                    {
                        memcpy(&atCmdAppContext.wstaConnState.bssid[1], bssAddr.addr, WDRV_PIC32MZW_MAC_ADDR_LEN);
                        atCmdAppContext.wstaConnState.bssid[0] = WDRV_PIC32MZW_MAC_ADDR_LEN;
                    }
                }
            }

            if (0 == atCmdAppContext.wstaConnState.channel)
            {
                WDRV_PIC32MZW_CHANNEL_ID opChannel;

                if (WDRV_PIC32MZW_STATUS_OK == WDRV_PIC32MZW_InfoOpChanGet(atCmdAppContext.wdrvHandle, &opChannel))
                {
                    if (WDRV_PIC32MZW_CID_ANY != opChannel)
                    {
                        atCmdAppContext.wstaConnState.channel = opChannel;
                    }
                }
            }

            if ((0 != atCmdAppContext.wstaConnState.bssid[0]) && (0 != atCmdAppContext.wstaConnState.channel))
            {
                ATCMD_Print("+WSTALU:", 8);
                ATCMD_PrintMACAddress(&atCmdAppContext.wstaConnState.bssid[1]);
                ATCMD_Printf(",%d\r\n", atCmdAppContext.wstaConnState.channel);

                if (false == atCmdAppContext.wstaConf.dhcpClientEnabled)
                {
                    /* If DHCP isn't being used, static configuration becomes the connection state */

                    atCmdAppContext.wstaConnState.dnsSvr1    = atCmdAppContext.wstaConf.dnsSvr1;
                    atCmdAppContext.wstaConnState.dnsSvr2    = atCmdAppContext.wstaConf.dnsSvr2;
                    atCmdAppContext.wstaConnState.dfltRouter = atCmdAppContext.wstaConf.dfltRouter;
                    atCmdAppContext.wstaConnState.ipAddr     = atCmdAppContext.wstaConf.ipAddr;
                    atCmdAppContext.wstaConnState.netMask    = atCmdAppContext.wstaConf.netMask;

                    _SNTPSetup();
                }

                assocInfoPending = false;
            }
        }

        if (true == ntpSrvResolved)
        {
            TCPIP_DNS_RESULT dnsResult;
            IP_MULTI_ADDRESS ipAddress;
            char ipAddrStr[20];

            dnsResult = TCPIP_DNS_IsResolved((char*)&atCmdAppContext.wstaConf.ntpSvr[1], &ipAddress, TCPIP_DNS_TYPE_A);

            if (TCPIP_DNS_RES_PENDING == dnsResult)
            {
                ntpSrvResolved = false;
            }
            else
            {
                if (TCPIP_DNS_RES_OK == dnsResult)
                {
                    int numRes;
                    int resIdx;

                    numRes = TCPIP_DNS_GetIPAddressesNumber((char*)&atCmdAppContext.wstaConf.ntpSvr[1], IP_ADDRESS_TYPE_IPV4);
                    resIdx = 0;

                    while (numRes--)
                    {
                        TCPIP_DNS_GetIPv4Addresses((char*)&atCmdAppContext.wstaConf.ntpSvr[1], resIdx++, &ipAddress.v4Add, 1);

                        TCPIP_Helper_IPAddressToString(&ipAddress.v4Add, ipAddrStr, sizeof(ipAddrStr));

                        if (SNTP_RES_OK == TCPIP_SNTP_ConnectionParamSet(atCmdAppContext.netHandle, IP_ADDRESS_TYPE_IPV4, ipAddrStr))
                        {
                            atCmdAppContext.wstaConnState.ntpSvr = ipAddress.v4Add.Val;

                            /* Kick SNTP client to attempt a new connection */
                            TCPIP_SNTP_ConnectionInitiate();

                            break;
                        }
                    }
                }

                _DNSResolvResetQuery();
            }
        }
        else if (NULL != dnsResolveHandle)
        {
            uint32_t currTimeMs = ATCMD_PlatformGetSysTimeMs();

            if ((currTimeMs - dnsResolveStartMs) > 5000)
            {
                _DNSResolvResetQuery();
            }
        }
    }
    else if (ATCMD_APP_STATE_STA_CONNECTING == atCmdAppContext.appState)
    {
        if (atCmdAppContext.wstaStateTimeout > 0)
        {
            if ((ATCMD_PlatformGetSysTimeMs() - atCmdAppContext.wstaStateTimeStartMs) > atCmdAppContext.wstaStateTimeout)
            {
                _BSSDisconnect(true);

                ATCMD_APPStateMachineEvent(ATCMD_APP_EVENT_CONN_TIMEOUT, true);

                ATCMD_ReportAECStatus(pCmdTypeDesc->pCmdName, ATCMD_APP_STATUS_STA_CONN_FAILED);

                atCmdAppContext.wstaConnState.wstaState = 0;
                atCmdAppContext.wstaStateTimeout        = 0;
           }
        }
    }
    else
    {
        assocInfoPending = false;
    }

    return ATCMD_STATUS_OK;
}
