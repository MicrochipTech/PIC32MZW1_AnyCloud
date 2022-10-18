/**
 *
 * Copyright (c) 2022 Microchip Technology Inc. and its subsidiaries.
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
#include <string.h>

#include "include/at_cmds.h"
#include "at_cmd_app.h"
#include "at_cmd_tls.h"
#include "wolfssl/ssl.h"

#define INSERT_CERT_DER_DATA(fileName, fileSize) {.format = SSL_FILETYPE_ASN1, .pFN = #fileName, .pCertStart = fileName, .pCertEnd = NULL, .size = (int)#fileSize},
#define INSERT_CERT_PEM_DATA(fileName, fileSize) {.format = SSL_FILETYPE_PEM, .pFN = #fileName, .pCertStart = fileName, .pCertEnd = NULL, .size = (int)#fileSize},
#define INSERT_PRIKEY_DER_DATA(fileName, fileSize) {.format = SSL_FILETYPE_ASN1, .pFN = #fileName, .pPriKeyStart = fileName, .pCertEnd = NULL, .size = (int)#fileSize},
#define INSERT_PRIKEY_PEM_DATA(fileName, fileSize) {.format = SSL_FILETYPE_PEM, .pFN = #fileName, .pPriKeyStart = fileName, .pCertEnd = NULL, .size = (int)#fileSize},

#include "cert_header.h"

AT_CMD_CERT_ENTRY certsTable[] = {
#include "certs.h"
    {.pFN = NULL}
};

#define PRIKEY_EXTERN
#include "private.h"
#undef PRIKEY_EXTERN

AT_CMD_PRIKEY_ENTRY priKeysTable[] = {
#include "private.h"
    {.pFN = NULL}
};

extern OSAL_SEM_HANDLE_TYPE printEventSemaphore;

extern const AT_CMD_TYPE_DESC atCmdTypeDescRST;
extern const AT_CMD_TYPE_DESC atCmdTypeDescOTAFW;
extern const AT_CMD_TYPE_DESC atCmdTypeDescSWFW;
extern const AT_CMD_TYPE_DESC atCmdTypeDescWSCNC;
extern const AT_CMD_TYPE_DESC atCmdTypeDescWSCNA;
extern const AT_CMD_TYPE_DESC atCmdTypeDescWSCNP;
extern const AT_CMD_TYPE_DESC atCmdTypeDescWSTAC;
extern const AT_CMD_TYPE_DESC atCmdTypeDescWSTA;
extern const AT_CMD_TYPE_DESC atCmdTypeDescWAPC;
extern const AT_CMD_TYPE_DESC atCmdTypeDescWAP;
extern const AT_CMD_TYPE_DESC atCmdTypeDescWPROVC;
extern const AT_CMD_TYPE_DESC atCmdTypeDescWPROV;
extern const AT_CMD_TYPE_DESC atCmdTypeDescSOCKO;
extern const AT_CMD_TYPE_DESC atCmdTypeDescSOCKBL;
extern const AT_CMD_TYPE_DESC atCmdTypeDescSOCKBR;
extern const AT_CMD_TYPE_DESC atCmdTypeDescSOCKBM;
extern const AT_CMD_TYPE_DESC atCmdTypeDescSOCKTLS;
extern const AT_CMD_TYPE_DESC atCmdTypeDescSOCKWR;
extern const AT_CMD_TYPE_DESC atCmdTypeDescSOCKWRTO;
extern const AT_CMD_TYPE_DESC atCmdTypeDescSOCKRD;
extern const AT_CMD_TYPE_DESC atCmdTypeDescSOCKCL;
extern const AT_CMD_TYPE_DESC atCmdTypeDescSOCKLST;
extern const AT_CMD_TYPE_DESC atCmdTypeDescDNSRESOLV;
extern const AT_CMD_TYPE_DESC atCmdTypeDescTIME;
extern const AT_CMD_TYPE_DESC atCmdTypeDescCFG;
extern const AT_CMD_TYPE_DESC atCmdTypeDescPING;
extern const AT_CMD_TYPE_DESC atCmdTypeDescMQTTC;
extern const AT_CMD_TYPE_DESC atCmdTypeDescMQTTCONN;
extern const AT_CMD_TYPE_DESC atCmdTypeDescMQTTSUB;
extern const AT_CMD_TYPE_DESC atCmdTypeDescMQTTUNSUB;
extern const AT_CMD_TYPE_DESC atCmdTypeDescMQTTPUB;
#ifndef WOLFMQTT_V5
extern const AT_CMD_TYPE_DESC atCmdTypeDescMQTTLWT;
#endif
extern const AT_CMD_TYPE_DESC atCmdTypeDescMQTTDISCONN;
#ifdef WOLFMQTT_V5
extern const AT_CMD_TYPE_DESC atCmdTypeDescMQTTPROPTX;
extern const AT_CMD_TYPE_DESC atCmdTypeDescMQTTPROPRX;
extern const AT_CMD_TYPE_DESC atCmdTypeDescMQTTPROPTXS;
#endif
extern const AT_CMD_TYPE_DESC atCmdTypeDescTLSC;
extern const AT_CMD_TYPE_DESC atCmdTypeDescINFO;
extern const AT_CMD_TYPE_DESC atCmdTypeDescLOADCERT;
extern const AT_CMD_TYPE_DESC atCmdTypeDescREADCERT;
extern const AT_CMD_TYPE_DESC atCmdTypeDescLowPower;

const AT_CMD_TYPE_DESC* atCmdTypeDescTable[] =
{
    &atCmdTypeDescRST,
    &atCmdTypeDescOTAFW,
    &atCmdTypeDescSWFW,
    &atCmdTypeDescWSCNC,
    &atCmdTypeDescWSCNA,
    &atCmdTypeDescWSCNP,
    &atCmdTypeDescWSTAC,
    &atCmdTypeDescWSTA,
    &atCmdTypeDescWAPC,
    &atCmdTypeDescWAP,
	&atCmdTypeDescWPROVC,
	&atCmdTypeDescWPROV,
    &atCmdTypeDescSOCKO,
    &atCmdTypeDescSOCKBL,
    &atCmdTypeDescSOCKBR,
    &atCmdTypeDescSOCKBM,
    &atCmdTypeDescSOCKTLS,
    &atCmdTypeDescSOCKWR,
    &atCmdTypeDescSOCKWRTO,
    &atCmdTypeDescSOCKRD,
    &atCmdTypeDescSOCKCL,
    &atCmdTypeDescSOCKLST,
    &atCmdTypeDescDNSRESOLV,
    &atCmdTypeDescTIME,
    &atCmdTypeDescCFG,
    &atCmdTypeDescPING,
    &atCmdTypeDescMQTTC,
    &atCmdTypeDescMQTTCONN,
    &atCmdTypeDescMQTTSUB,
    &atCmdTypeDescMQTTUNSUB,
    &atCmdTypeDescMQTTPUB,
    &atCmdTypeDescMQTTDISCONN,
    &atCmdTypeDescTLSC,
    &atCmdTypeDescINFO,
    &atCmdTypeDescLOADCERT,
    &atCmdTypeDescREADCERT,
    &atCmdTypeDescLowPower,
    NULL,
};

static const char* statusAppCodeStr[] = {
    "Wi-Fi Request Failed",                     // ATCMD_APP_STATUS_WIFI_API_REQUEST_FAILED
    "STA Not Connected",                        // ATCMD_APP_STATUS_STA_NOT_CONNECTED
    "Socket ID Not Found",                      // ATCMD_APP_STATUS_SOCKET_ID_NOT_FOUND
    "No Free Sockets",                          // ATCMD_APP_STATUS_NO_FREE_SOCKETS
    "Invalid Socket Protocol",                  // ATCMD_APP_STATUS_INVALID_SOCKET_PROTOCOL
    "Socket Close Failed",                      // ATCMD_APP_STATUS_SOCKET_CLOSE_FAILED
    "Socket Bind Failed",                       // ATCMD_APP_STATUS_SOCKET_BIND_FAILED
    "Socket TLS Failed",                        // ATCMD_APP_STATUS_SOCKET_TLS_FAILED
    "Socket Connect Failed",                    // ATCMD_APP_STATUS_SOCKET_CONNECT_FAILED
    "Socket Send Failed",                       // ATCMD_APP_STATUS_SOCKET_SEND_FAILED
    "Socket Set Option Failed",                 // ATCMD_APP_STATUS_SOCKET_SET_OPT_FAILED
    "Socket Destination Not Set",               // ATCMD_APP_STATUS_SOCKET_REMOTE_NOT_SET
    "Length Mismatch",                          // ATCMD_APP_STATUS_LENGTH_MISMATCH
    "STA Disconnect Not Permitted",             // ATCMD_APP_STATUS_STA_DISCONN_REFUSED
    "STA Disconnect Failed",                    // ATCMD_APP_STATUS_STA_DISCONN_FAILED
    "STA Connection Not Permitted",             // ATCMD_APP_STATUS_STA_CONN_REFUSED
    "STA Connection Failed",                    // ATCMD_APP_STATUS_STA_CONN_FAILED
    "Soft AP Stop Not Permitted",               // ATCMD_APP_STATUS_WAP_STOP_REFUSED
    "Soft AP Stop Failed",                      // ATCMD_APP_STATUS_WAP_STOP_FAILED
    "Soft AP STArt Not Permitted",              // ATCMD_APP_STATUS_WAP_START_REFUSED
    "Soft AP STArt Failed",                     // ATCMD_APP_STATUS_WAP_START_FAILED
    "Provisioning Soft AP Stop Not Permitted",  // ATCMD_APP_STATUS_PROV_AP_STOP_REFUSED
    "Provisioning Soft AP Stop Failed",         // ATCMD_APP_STATUS_PROV_AP_STOP_FAILED
    "Provisioning Soft AP STArt Not Permitted", // ATCMD_APP_STATUS_PROV_AP_START_REFUSED
    "Provisioning Soft AP STArt Failed",        // ATCMD_APP_STATUS_PROV_AP_START_FAILED
    "WPS Stop Not Permitted",                   // ATCMD_APP_STATUS_PROV_WPS_STOP_REFUSED
    "WPS Stop Failed",                          // ATCMD_APP_STATUS_PROV_WPS_STOP_FAILED
    "WPS Start Not Permitted",                  // ATCMD_APP_STATUS_PROV_WPS_START_REFUSED
    "WPS Start Failed",                         // ATCMD_APP_STATUS_PROV_WPS_START_FAILED
    "Unsupported Security Type",                // ATCMD_APP_STATUS_UNSUPPORTTED_SEC_TYPE
    "Invalid Security Parameters",              // ATCMD_APP_STATUS_INVALID_SEC_PARAMS
    "OTA In Progress",                          // ATCMD_APP_STATUS_OTA_IN_PROGRESS
    "Unsupported File Transfer Protocol",       // ATCMD_APP_STATUS_TSFR_PROTOCOL_NOT_SUPPORTED
    "DNS Type Not Supported",                   // ATCMD_APP_STATUS_DNS_TYPE_NOT_SUPPORTED
    "DNS Query Timeout",                        // ATCMD_APP_STATUS_DNS_TIMEOUT
    "Ping Failed",                              // ATCMD_APP_STATUS_PING_FAILED
    "Network Error",                            // ATCMD_APP_STATUS_NETWORK_ERROR
    "Multicast Error",                          // ATCMD_APP_STATUS_MULTICAST_ERROR
    "Time Error",                               // ATCMD_APP_STATUS_TIME_ERROR
    "MQTT Error",                               // ATCMD_APP_STATUS_MQTT_ERROR
};

ATCMD_APP_CONTEXT atCmdAppContext;

static const ATCMD_APP_STATE_TRANSITION atCmdAppStateTable[] = {
    {ATCMD_APP_EVENT_STA_CONNECTING,        ATCMD_APP_STATE_IDLE,               ATCMD_APP_STATE_STA_CONNECTING},
    {ATCMD_APP_EVENT_STA_CONNECTED,         ATCMD_APP_STATE_STA_CONNECTING,     ATCMD_APP_STATE_STA_CONNECTED},
    {ATCMD_APP_EVENT_STA_DISCONNECTING,     ATCMD_APP_STATE_STA_CONNECTED,      ATCMD_APP_STATE_STA_DISCONNECTING},
    {ATCMD_APP_EVENT_STA_DISCONNECTED,      ATCMD_APP_STATE_STA_DISCONNECTING,  ATCMD_APP_STATE_IDLE},
    {ATCMD_APP_EVENT_CONN_TIMEOUT,          ATCMD_APP_STATE_STA_CONNECTING,     ATCMD_APP_STATE_IDLE},
    {ATCMD_APP_EVENT_LINK_LOSS,             ATCMD_APP_STATE_STA_CONNECTED,      ATCMD_APP_STATE_IDLE},
    {ATCMD_APP_EVENT_AP_STARTING,           ATCMD_APP_STATE_IDLE,               ATCMD_APP_STATE_AP_STARTED},
    {ATCMD_APP_EVENT_AP_STOPPING,           ATCMD_APP_STATE_AP_STARTED,         ATCMD_APP_STATE_IDLE},
    {ATCMD_APP_EVENT_PROV_AP_STARTING,      ATCMD_APP_STATE_IDLE,               ATCMD_APP_STATE_PROV_AP_STARTED},
    {ATCMD_APP_EVENT_PROV_AP_STOPPING,      ATCMD_APP_STATE_PROV_AP_STARTED,    ATCMD_APP_STATE_IDLE},
    {ATCMD_APP_EVENT_START_WPS,             ATCMD_APP_STATE_IDLE,               ATCMD_APP_STATE_WPS_STARTED},
    {ATCMD_APP_EVENT_STOP_WPS,              ATCMD_APP_STATE_WPS_STARTED,        ATCMD_APP_STATE_IDLE},

    {ATCMD_APP_EVENT_INVALID,               ATCMD_APP_STATE_ANY,                ATCMD_APP_STATE_ANY}        /* Last state */
};

bool ATCMD_APPStateMachineEvent(ATCMD_APP_EVENT event, bool commit)
{
    const ATCMD_APP_STATE_TRANSITION *pState = atCmdAppStateTable;

    while (ATCMD_APP_EVENT_INVALID != pState->InEvent)
    {
        if (event == pState->InEvent)
        {
            if ((ATCMD_APP_STATE_ANY == pState->curState) || (atCmdAppContext.appState == pState->curState) || (atCmdAppContext.appState == pState->newState))
            {
                break;
            }
        }

        pState++;
    }

    if (ATCMD_APP_EVENT_INVALID == pState->InEvent)
    {
        return false;
    }

    if (true == commit)
    {
        //ATCMD_Printf("App State (+%d): %d -> %d\r\n", event, atCmdAppContext.appState, pState->newState);
        atCmdAppContext.appState = pState->newState;
    }

    return true;
}

ATCMD_APP_STATE ATCMD_APPStateMachineCurrentState(void)
{
    return atCmdAppContext.appState;
}

const char* ATCMD_APPExecuteGMR(void)
{
    return NULL;
}

const char* ATCMD_APPTranslateStatusCode(ATCMD_APP_STATUS statusCode)
{
    if ((statusCode < (ATCMD_APP_STATUS)ATCMD_STATUS_CUSTOM_MSG_BASE) || (statusCode >= MAX_ATCMD_APP_STATUS))
    {
        return NULL;
    }

    return statusAppCodeStr[statusCode - ATCMD_STATUS_CUSTOM_MSG_BASE];
}

const AT_CMD_CERT_ENTRY* ATCMD_APPCertFind(const char *pName)
{
    const AT_CMD_CERT_ENTRY *pCertsTableEntry;

    pCertsTableEntry = certsTable;

    while (NULL != pCertsTableEntry->pFN)
    {
        if (0 == strcmp(pName, pCertsTableEntry->pFN))
        {
            return pCertsTableEntry;
        }

        pCertsTableEntry++;
    }

    return NULL;
}

const AT_CMD_PRIKEY_ENTRY* ATCMD_APPPriKeyFind(const char *pName)
{
    const AT_CMD_PRIKEY_ENTRY *pPriKeyTableEntry;

    pPriKeyTableEntry = priKeysTable;

    while (NULL != pPriKeyTableEntry->pFN)
    {
        if (0 == strcmp(pName, pPriKeyTableEntry->pFN))
        {
            return pPriKeyTableEntry;
        }

        pPriKeyTableEntry++;
    }

    return NULL;
}

void ATCMD_APPInit(void)
{
    atCmdAppContext.appState    = ATCMD_APP_STATE_ANY;
    atCmdAppContext.assocHandle = WDRV_PIC32MZW_ASSOC_HANDLE_INVALID;
    atCmdAppContext.netHandle   = TCPIP_STACK_NetHandleGet(TCPIP_NETWORK_DEFAULT_INTERFACE_NAME_IDX0);
    atCmdAppContext.dhcpcHandle = 0;
    atCmdAppContext.sntpHandle  = 0;

    atCmdAppContext.wdrvHandle = WDRV_PIC32MZW_Open(0, 0);

    if (DRV_HANDLE_INVALID != atCmdAppContext.wdrvHandle)
    {
		atCmdAppContext.appState = ATCMD_APP_STATE_IDLE;

//        ATCMD_Printf("\r\nAT Command Application (c) %s Microchip Technology Inc", &__DATE__[7]);
    }

    TCPIP_STACK_NetMulticastSet(atCmdAppContext.netHandle);
    OSAL_SEM_Create(&printEventSemaphore, OSAL_SEM_TYPE_BINARY, 1, 1);
}

void ATCMD_APPUpdate(void)
{
}
