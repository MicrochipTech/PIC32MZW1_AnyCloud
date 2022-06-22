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
#include "at_cmds/at_cmd_inet.h"
#include "tcpip/dns.h"

/*******************************************************************************
* Command interface prototypes
*******************************************************************************/
static ATCMD_STATUS _PINGInit(const AT_CMD_TYPE_DESC* pCmdTypeDesc);
static ATCMD_STATUS _PINGExecute(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const int numParams, ATCMD_PARAM *pParamList);
static ATCMD_STATUS _PINGUpdate(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const AT_CMD_TYPE_DESC* pCurrentCmdTypeDesc);

/*******************************************************************************
* Command parameters
*******************************************************************************/
static const ATCMD_HELP_PARAM paramTARGET_ADDR =
    {"TARGET_ADDR", "IP address or host name of target", ATCMD_PARAM_TYPE_CLASS_STRING, 0};

/*******************************************************************************
* Command examples
*******************************************************************************/

/*******************************************************************************
* Command descriptors
*******************************************************************************/
const AT_CMD_TYPE_DESC atCmdTypeDescPING =
    {
        .pCmdName   = "+PING",
        .cmdInit    = _PINGInit,
        .cmdExecute = _PINGExecute,
        .cmdUpdate  = _PINGUpdate,
        .pSummary   = "This command sends a ping (ICMP Echo Request) to the target address",
        .numVars    = 1,
        {
            {
                .numParams   = 1,
                .pParams     =
                {
                    &paramTARGET_ADDR
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
static char nameToResolve[TCPIP_DNS_CLIENT_MAX_HOSTNAME_LEN];

static TCPIP_ICMP_ECHO_REQUEST pingRequest;
static TCPIP_ICMP_REQUEST_HANDLE pingHandle;
static TCPIP_DNS_HANDLE dnsResolveHandle;
static bool nameResolved;
static uint32_t pingRequestTimeMs;

/*******************************************************************************
* Local functions
*******************************************************************************/
static void _PINGResolvResetQuery(void)
{
    if (NULL != dnsResolveHandle)
    {
        TCPIP_DNS_HandlerDeRegister(dnsResolveHandle);
    }

    memset(nameToResolve, 0, TCPIP_DNS_CLIENT_MAX_HOSTNAME_LEN);

    dnsResolveHandle = NULL;
    nameResolved     = false;
}

static void _PINGResolvEventHandler(TCPIP_NET_HANDLE hNet, TCPIP_DNS_EVENT_TYPE evType, const char* pName, const void* hParam)
{
    if (TCPIP_DNS_EVENT_NAME_RESOLVED == evType)
    {
        nameResolved = true;
    }
}

static void _PingCallback(const TCPIP_ICMP_ECHO_REQUEST* pReqData, TCPIP_ICMP_REQUEST_HANDLE icmpHandle, TCPIP_ICMP_ECHO_REQUEST_RESULT result)
{
    char s[20];

    if (TCPIP_ICMP_ECHO_REQUEST_RES_OK == result)
    {
        uint32_t pingRTT;

        pingRTT = ATCMD_PlatformGetSysTimeMs() - pingRequestTimeMs;

        TCPIP_Helper_IPAddressToString(&pReqData->targetAddr, s, sizeof(s));
        ATCMD_Printf("+PING:\"%s\",%d\r\n", s, pingRTT);
    }
    else
    {
        /* TODO: Timeout */
    }
}

static ATCMD_STATUS _PingTargetAddress(const char* pTargetIPAddrStr)
{
    ICMP_ECHO_RESULT res;

    _PINGResolvResetQuery();

    if (false == TCPIP_Helper_StringToIPAddress(pTargetIPAddrStr, &pingRequest.targetAddr))
    {
        return ATCMD_STATUS_INVALID_PARAMETER;
    }

    pingRequest.sequenceNumber = 1;

    res = TCPIP_ICMP_EchoRequest(&pingRequest, &pingHandle);

    if ((NULL == pingHandle) || (ICMP_ECHO_OK != res))
    {
        return ATCMD_APP_STATUS_PING_FAILED;
    }

    pingRequestTimeMs = ATCMD_PlatformGetSysTimeMs();

    return ATCMD_STATUS_OK;
}

/*******************************************************************************
* Command init functions
*******************************************************************************/
static ATCMD_STATUS _PINGInit(const AT_CMD_TYPE_DESC* pCmdTypeDesc)
{
    dnsResolveHandle = NULL;

    pingRequest.netH        = atCmdAppContext.netHandle;
    pingRequest.identifier  = 0xCD78;
    pingRequest.pData       = NULL;
    pingRequest.dataSize    = 0;
    pingRequest.callback    = _PingCallback;

    _PINGResolvResetQuery();

    return ATCMD_STATUS_OK;
}

/*******************************************************************************
* Command execute functions
*******************************************************************************/
static ATCMD_STATUS _PINGExecute(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const int numParams, ATCMD_PARAM *pParamList)
{
    if (1 == numParams)
    {
        /* Check the parameter types are correct */

        if (false == ATCMD_ParamValidateTypes(pCmdTypeDesc, 0, numParams, pParamList))
        {
            return ATCMD_STATUS_INVALID_PARAMETER;
        }

        TCPIP_DNS_RESULT dnsResult;

        /* A record lookup */

        strncpy(nameToResolve, (char*)pParamList[0].value.p, TCPIP_DNS_CLIENT_MAX_HOSTNAME_LEN);

        dnsResolveHandle = TCPIP_DNS_HandlerRegister(atCmdAppContext.netHandle, &_PINGResolvEventHandler, nameToResolve);

        if (NULL != dnsResolveHandle)
        {
            dnsResult = TCPIP_DNS_Resolve(nameToResolve, TCPIP_DNS_TYPE_A);

            if (dnsResult < 0)
            {
                _PINGResolvResetQuery();

                return ATCMD_APP_STATUS_PING_FAILED;
            }
            else if (TCPIP_DNS_RES_NAME_IS_IPADDRESS == dnsResult)
            {
                _PINGResolvResetQuery();

                return _PingTargetAddress((char*)pParamList[0].value.p);
            }
            else
            {
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
static ATCMD_STATUS _PINGUpdate(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const AT_CMD_TYPE_DESC* pCurrentCmdTypeDesc)
{
    TCPIP_DNS_RESULT dnsResult;
    IP_MULTI_ADDRESS ipAddress;
    char ipAddrStr[20];

    if (true == nameResolved)
    {
        dnsResult = TCPIP_DNS_IsResolved(nameToResolve, &ipAddress, TCPIP_DNS_TYPE_A);

        if (TCPIP_DNS_RES_OK == dnsResult)
        {
            TCPIP_Helper_IPAddressToString(&ipAddress.v4Add, ipAddrStr, sizeof(ipAddrStr));

            if (ATCMD_STATUS_OK != _PingTargetAddress(ipAddrStr))
            {
                /* TODO */
            }
        }
        else
        {
            /* TODO */
        }

        _PINGResolvResetQuery();
    }

    return ATCMD_STATUS_OK;
}
