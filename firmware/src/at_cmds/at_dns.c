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
#include "tcpip/dns.h"

/*******************************************************************************
* Command interface prototypes
*******************************************************************************/
static ATCMD_STATUS _DNSRESOLVInit(const AT_CMD_TYPE_DESC* pCmdTypeDesc);
static ATCMD_STATUS _DNSRESOLVExecute(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const int numParams, ATCMD_PARAM *pParamList);
static ATCMD_STATUS _DNSRESOLVUpdate(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const AT_CMD_TYPE_DESC* pCurrentCmdTypeDesc);

/*******************************************************************************
* Command parameters
*******************************************************************************/
static const ATCMD_HELP_PARAM paramResvoleType =
    {"TYPE", "Type of record", ATCMD_PARAM_TYPE_CLASS_INTEGER,
        .numOpts = 1,
        {
            {"1", "A"}
        }
    };

static const ATCMD_HELP_PARAM paramDomainName =
    {"DOMAIN_NAME", "Domain name to resolve", ATCMD_PARAM_TYPE_CLASS_STRING, 0};

/*******************************************************************************
* Command examples
*******************************************************************************/

/*******************************************************************************
* Command descriptors
*******************************************************************************/
const AT_CMD_TYPE_DESC atCmdTypeDescDNSRESOLV =
    {
        .pCmdName   = "+DNSRESOLV",
        .cmdInit    = _DNSRESOLVInit,
        .cmdExecute = _DNSRESOLVExecute,
        .cmdUpdate  = _DNSRESOLVUpdate,
        .pSummary   = "This command is used to resolve domain names via DNS",
        .numVars    = 1,
        {
            {
                .numParams   = 2,
                .pParams     =
                {
                    &paramResvoleType,
                    &paramDomainName
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

static TCPIP_DNS_HANDLE dnsResolveHandle;
static bool nameResolved;
static uint32_t dnsResolveStartMs;

/*******************************************************************************
* Local functions
*******************************************************************************/
static void _DNSResolvResetQuery(void)
{
    memset(nameToResolve, 0, TCPIP_DNS_CLIENT_MAX_HOSTNAME_LEN);

    dnsResolveHandle = NULL;
    nameResolved     = false;
}

static void _DNSResolvEventHandler(TCPIP_NET_HANDLE hNet, TCPIP_DNS_EVENT_TYPE evType, const char* pName, const void* hParam)
{
    if (TCPIP_DNS_EVENT_NAME_RESOLVED == evType)
    {
        if (0 == memcmp(pName, nameToResolve, strlen(nameToResolve)))
        {
            nameResolved = true;
        }
    }
}

/*******************************************************************************
* Command init functions
*******************************************************************************/
static ATCMD_STATUS _DNSRESOLVInit(const AT_CMD_TYPE_DESC* pCmdTypeDesc)
{
    dnsResolveHandle = NULL;

    _DNSResolvResetQuery();

    return ATCMD_STATUS_OK;
}

/*******************************************************************************
* Command execute functions
*******************************************************************************/
static ATCMD_STATUS _DNSRESOLVExecute(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const int numParams, ATCMD_PARAM *pParamList)
{
    if (2 == numParams)
    {
        /* Check the parameter types are correct */

        if (false == ATCMD_ParamValidateTypes(pCmdTypeDesc, 0, numParams, pParamList))
        {
            return ATCMD_STATUS_INVALID_PARAMETER;
        }
    }
    else
    {
        return ATCMD_STATUS_INCORRECT_NUM_PARAMS;
    }

    if (ATCMD_APP_STATE_STA_CONNECTED != atCmdAppContext.appState)
    {
        return ATCMD_APP_STATUS_STA_NOT_CONNECTED;
    }

    _DNSResolvResetQuery();

    if (1 == pParamList[0].value.i)
    {
        TCPIP_DNS_RESULT dnsResult;

        /* A record lookup */

        if (strlen((char*)pParamList[1].value.p) >= TCPIP_DNS_CLIENT_MAX_HOSTNAME_LEN)
        {
            return ATCMD_STATUS_INVALID_PARAMETER;
        }

        strcpy(nameToResolve, (char*)pParamList[1].value.p);

        dnsResolveHandle = TCPIP_DNS_HandlerRegister(atCmdAppContext.netHandle, &_DNSResolvEventHandler, nameToResolve);
        
        if (NULL != dnsResolveHandle)
        {
            dnsResult = TCPIP_DNS_Resolve(nameToResolve, TCPIP_DNS_TYPE_A);

            if (dnsResult < 0)
            {
                _DNSResolvResetQuery();

                return ATCMD_APP_STATUS_DNS_TYPE_NOT_SUPPORTED;
            }
            else if (TCPIP_DNS_RES_NAME_IS_IPADDRESS == dnsResult)
            {
                ATCMD_Printf("+DNSRESOLV:0,\"%s\",\"%s\"\r\n", nameToResolve, nameToResolve);

                _DNSResolvResetQuery();
            }
            else
            {
                dnsResolveStartMs = ATCMD_PlatformGetSysTimeMs();
            }
        }
    }
    else
    {
        return ATCMD_APP_STATUS_DNS_TYPE_NOT_SUPPORTED;
    }

    return ATCMD_STATUS_OK;
}

/*******************************************************************************
* Command update functions
*******************************************************************************/
static ATCMD_STATUS _DNSRESOLVUpdate(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const AT_CMD_TYPE_DESC* pCurrentCmdTypeDesc)
{
    TCPIP_DNS_RESULT dnsResult;
    IP_MULTI_ADDRESS ipAddress;
    char ipAddrStr[20];

    if (true == nameResolved)
    {
        dnsResult = TCPIP_DNS_IsResolved(nameToResolve, &ipAddress, TCPIP_DNS_TYPE_A);

        if (TCPIP_DNS_RES_PENDING == dnsResult)
        {
            nameResolved = false;
        }
        else
        {
            if (TCPIP_DNS_RES_OK == dnsResult)
            {
                int numRes;
                int resIdx;

                numRes = TCPIP_DNS_GetIPAddressesNumber(nameToResolve, IP_ADDRESS_TYPE_IPV4);
                resIdx = 0;

                while (numRes--)
                {
                    TCPIP_DNS_GetIPv4Addresses(nameToResolve, resIdx++, &ipAddress.v4Add, 1);

                    TCPIP_Helper_IPAddressToString(&ipAddress.v4Add, ipAddrStr, sizeof(ipAddrStr));

                    ATCMD_Printf("+DNSRESOLV:0,\"%s\",\"%s\"\r\n", nameToResolve, ipAddrStr);
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
            ATCMD_ReportAECStatus("+DNSRESOLV", ATCMD_APP_STATUS_DNS_TIMEOUT);

            _DNSResolvResetQuery();
        }
    }

    return ATCMD_STATUS_OK;
}
