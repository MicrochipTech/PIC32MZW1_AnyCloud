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

// DOM-IGNORE-BEGIN
/*******************************************************************************
Copyright (c) 2022 released Microchip Technology Inc.  All rights reserved.

Microchip licenses to you the right to use, modify, copy and distribute
Software only when embedded on a Microchip microcontroller or digital signal
controller that is integrated into your product or third party product
(pursuant to the sublicense terms in the accompanying license agreement).

You should refer to the license agreement accompanying this Software for
additional information regarding your rights and obligations.

SOFTWARE AND DOCUMENTATION ARE PROVIDED AS IS WITHOUT WARRANTY OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF
MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE.
IN NO EVENT SHALL MICROCHIP OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER
CONTRACT, NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR
OTHER LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE OR
CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT OF
SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
(INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.
 *******************************************************************************/
// DOM-IGNORE-END

#ifndef _AT_CMD_APP_H
#define _AT_CMD_APP_H

#include "include/at_cmds.h"
#include "at_cmds/at_cmd_parser.h"
#include "at_cmds/at_cmd_pkcs.h"
#include "wdrv_pic32mzw_client_api.h"
#include "wolfssl/ssl.h"

#define AT_CMD_SSID_SZ                          32
#define AT_CMD_CRED_SZ                          128
#define AT_CMD_NTPSRV_SZ                        128
#define AT_CMD_WAP_DFLT_IPV4_ADDR               ((192 << 24) | (168 << 16) | (0 << 8) | (1))
#define AT_CMD_WAP_DFLT_IPV4_NETMASK            ((255 << 24) | (255 << 16) | (255 << 8) | (0))
#define AT_CMD_WAP_DFLT_IPV4_GATEWAY            ((192 << 24) | (168 << 16) | (0 << 8) | (1))
#define AT_CMD_WAP_DFLT_IPV4_DNS_SRV1           ((192 << 24) | (168 << 16) | (0 << 8) | (1))
#define AT_CMD_SOCK_MAX_NUM                     20 /* TODO */
#define AT_CMD_SOCK_RX_BUFFER_SZ                2048
#define AT_CMD_CERT_FILE_MAX_SZ                 1500
#define AT_CMD_PRIKEY_FILE_MAX_SZ               2000
#define AT_CMD_MQTT_BROKER_SZ                   64
#define AT_CMD_MQTT_CLIENTID_SZ                 23
#define AT_CMD_MQTT_USERNAME_SZ                 128
#define AT_CMD_MQTT_PASSWORD_SZ                 256
#define AT_CMD_MQTT_TOPIC_SZ                    128
#define AT_CMD_MQTT_NET_CONNECT_TIMEOUT_MS      5000
#define AT_CMD_MQTT_MSG_TIMEOUT_MS              1000
#define AT_CMD_MQTT_MAX_NUM_CONNECT_PROPS       8
#define AT_CMD_MQTT_MAX_NUM_PUBLISH_PROPS       7
#define AT_CMD_MQTT_MAX_NUM_SUBSCRIBE_PROPS     1
#define AT_CMD_MQTT_MAX_NUM_UNSUBSCRIBE_PROPS   0
#define AT_CMD_MQTT_MAX_NUM_DISCONNECT_PROPS    3
#define AT_CMD_MQTT_USER_PROP_STORE_SZ          128
#define AT_CMD_TLS_NUM_STATES                   2
#define AT_CMD_TLS_NUM_CONFS                    2
#define AT_CMD_TLS_CERT_NAME_SZ                 64
#define AT_CMD_TLS_PRIKEY_NAME_SZ               32
#define AT_CMD_TLS_PRIKEY_PW_SZ                 32
#define AT_CMD_TLS_SERVER_NAME_SZ               32

typedef enum
{
    ATCMD_APP_STATUS_WIFI_API_REQUEST_FAILED = ATCMD_STATUS_CUSTOM_MSG_BASE,
    ATCMD_APP_STATUS_STA_NOT_CONNECTED,
    ATCMD_APP_STATUS_SOCKET_ID_NOT_FOUND,
    ATCMD_APP_STATUS_NO_FREE_SOCKETS,
    ATCMD_APP_STATUS_INVALID_SOCKET_PROTOCOL,
    ATCMD_APP_STATUS_SOCKET_CLOSE_FAILED,
    ATCMD_APP_STATUS_SOCKET_BIND_FAILED,
    ATCMD_APP_STATUS_SOCKET_TLS_FAILED,
    ATCMD_APP_STATUS_SOCKET_CONNECT_FAILED,
    ATCMD_APP_STATUS_SOCKET_SEND_FAILED,
    ATCMD_APP_STATUS_SOCKET_SET_OPT_FAILED,
    ATCMD_APP_STATUS_SOCKET_REMOTE_NOT_SET,
    ATCMD_APP_STATUS_LENGTH_MISMATCH,
    ATCMD_APP_STATUS_STA_DISCONN_REFUSED,
    ATCMD_APP_STATUS_STA_DISCONN_FAILED,
    ATCMD_APP_STATUS_STA_CONN_REFUSED,
    ATCMD_APP_STATUS_STA_CONN_FAILED,
    ATCMD_APP_STATUS_WAP_STOP_REFUSED,
    ATCMD_APP_STATUS_WAP_STOP_FAILED,
    ATCMD_APP_STATUS_WAP_START_REFUSED,
    ATCMD_APP_STATUS_WAP_START_FAILED,
    ATCMD_APP_STATUS_PROV_AP_STOP_REFUSED,
    ATCMD_APP_STATUS_PROV_AP_STOP_FAILED,
    ATCMD_APP_STATUS_PROV_AP_START_REFUSED,
    ATCMD_APP_STATUS_PROV_AP_START_FAILED,
    ATCMD_APP_STATUS_PROV_WPS_STOP_REFUSED,
    ATCMD_APP_STATUS_PROV_WPS_STOP_FAILED,
    ATCMD_APP_STATUS_PROV_WPS_START_REFUSED,
    ATCMD_APP_STATUS_PROV_WPS_START_FAILED,
    ATCMD_APP_STATUS_UNSUPPORTTED_SEC_TYPE,
    ATCMD_APP_STATUS_INVALID_SEC_PARAMS,
    ATCMD_APP_STATUS_OTA_IN_PROGRESS,
    ATCMD_APP_STATUS_TSFR_PROTOCOL_NOT_SUPPORTED,
    ATCMD_APP_STATUS_DNS_TYPE_NOT_SUPPORTED,
    ATCMD_APP_STATUS_DNS_TIMEOUT,
    ATCMD_APP_STATUS_PING_FAILED,
    ATCMD_APP_STATUS_NETWORK_ERROR,
    ATCMD_APP_STATUS_MULTICAST_ERROR,
    ATCMD_APP_STATUS_TIME_ERROR,
    ATCMD_APP_STATUS_MQTT_ERROR,
    MAX_ATCMD_APP_STATUS
} ATCMD_APP_STATUS;

typedef enum
{
    ATCMD_APP_EVENT_INVALID,
    ATCMD_APP_EVENT_STA_CONNECTING,
    ATCMD_APP_EVENT_STA_CONNECTED,
    ATCMD_APP_EVENT_STA_DISCONNECTING,
    ATCMD_APP_EVENT_STA_DISCONNECTED,
    ATCMD_APP_EVENT_AP_STARTING,
    ATCMD_APP_EVENT_AP_STOPPING,
    ATCMD_APP_EVENT_PROV_AP_STARTING,
    ATCMD_APP_EVENT_PROV_AP_STOPPING,
    ATCMD_APP_EVENT_START_WPS,
    ATCMD_APP_EVENT_STOP_WPS,
    ATCMD_APP_EVENT_CONN_TIMEOUT,
    ATCMD_APP_EVENT_LINK_LOSS,
} ATCMD_APP_EVENT;

typedef enum
{
    ATCMD_APP_STATE_ANY,
    ATCMD_APP_STATE_IDLE,
    ATCMD_APP_STATE_STA_CONNECTING,
    ATCMD_APP_STATE_STA_CONNECTED,
    ATCMD_APP_STATE_STA_DISCONNECTING,
    ATCMD_APP_STATE_AP_STARTED,
    ATCMD_APP_STATE_PROV_AP_STARTED,
    ATCMD_APP_STATE_WPS_STARTED
} ATCMD_APP_STATE;

typedef enum
{
    ATAPP_VAL_LOAD_TYPE_CERT,
    ATAPP_VAL_LOAD_TYPE_PRIKEY,
} ATCMD_APP_VAL;

typedef enum
{
    ATCMD_MQTT_SESSION_STATE_NOT_CONNECTED,
    ATCMD_MQTT_SESSION_STATE_NET_CONNECTING,
    ATCMD_MQTT_SESSION_STATE_START_TLS,
    ATCMD_MQTT_SESSION_STATE_NET_CONNECTED,
    ATCMD_MQTT_SESSION_STATE_CONNECTING,
    ATCMD_MQTT_SESSION_STATE_CONNECTED,
    ATCMD_MQTT_SESSION_STATE_SUBSCRIBING,
    ATCMD_MQTT_SESSION_STATE_UNSUBSCRIBING,
    ATCMD_MQTT_SESSION_STATE_PUBLISHING,
    ATCMD_MQTT_SESSION_STATE_DISCONNECTING,
    ATCMD_MQTT_SESSION_STATE_DISCONNECTED
} ATCMD_MQTT_SESSION_STATE;

typedef struct
{
    ATCMD_APP_EVENT InEvent;
    ATCMD_APP_STATE curState;
    ATCMD_APP_STATE newState;
} ATCMD_APP_STATE_TRANSITION;

typedef struct
{
    int                 format;
    const char          *pFN;
    const unsigned char *pCertStart;
    const unsigned char *pCertEnd;
    int        size;
} AT_CMD_CERT_ENTRY;

typedef struct
{
    int                 format;
    const char          *pFN;
    const unsigned char *pPriKeyStart;
    const unsigned char *pPriKeyEnd;
    int                 size;
} AT_CMD_PRIKEY_ENTRY;

typedef struct
{
    int         activeScanTime;
    int         passiveListenTime;
} ATCMD_APP_WSCN_CONF;

typedef struct
{
    uint8_t     ssid[AT_CMD_SSID_SZ+1];
    int         secType;
    uint8_t     credentials[AT_CMD_CRED_SZ+1];
    int         channel;
    uint8_t     bssid[6+1];
    uint32_t    ipAddr;
    uint32_t    netMask;
    uint32_t    dfltRouter;
    uint32_t    dnsSvr1;
    uint32_t    dnsSvr2;
    bool        dhcpClientEnabled;
    uint8_t     ntpSvr[AT_CMD_NTPSRV_SZ+1];
    bool        ntpStatic;
    bool        ntpClient;
    bool        roaming;
} ATCMD_APP_WSTA_CONF;

typedef struct
{
    int         wstaState;
    int         channel;
    uint8_t     bssid[6+1];
    uint32_t    ipAddr;
    uint32_t    netMask;
    uint32_t    dfltRouter;
    uint32_t    dnsSvr1;
    uint32_t    dnsSvr2;
    uint32_t    ntpSvr;
    int         rssi;
} ATCMD_APP_WSTA_STATE;

typedef struct
{
    uint8_t     ssid[AT_CMD_SSID_SZ+1];
    int         secType;
    uint8_t     credentials[AT_CMD_CRED_SZ+1];
    int         channel;
    bool        hidden;
    uint32_t    ipAddr;
    uint32_t    netMask;
    uint32_t    dfltRouter;
    uint32_t    dnsSvr1;
    bool        dhcpServerEnabled;
    uint32_t    dhcpServerPoolStart;
} ATCMD_APP_WAP_CONF;

typedef struct
{
    bool                        connected;
    WDRV_PIC32MZW_ASSOC_HANDLE  assocHandle;
    uint32_t                    ipAddr;
    WDRV_PIC32MZW_MAC_ADDR      bssid;
    int                         rssi;
} ATCMD_APP_WAP_STA_STATE;

typedef struct
{
    int                     wapState;
    ATCMD_APP_WAP_STA_STATE sta[WDRV_PIC32MZW_NUM_ASSOCS];
} ATCMD_APP_WAP_STATE;

typedef struct
{
    int         mode;
    uint8_t     pin[8+1];
} ATCMD_APP_WPROV_CONF;

typedef struct
{
    int         wprovState;
} ATCMD_APP_WPROV_STATE;

typedef struct
{
    char                        caCertName[AT_CMD_TLS_CERT_NAME_SZ+1+1];
    char                        certName[AT_CMD_TLS_CERT_NAME_SZ+1+1];
    char                        priKeyName[AT_CMD_TLS_PRIKEY_NAME_SZ+1+1];
    char                        serverName[AT_CMD_TLS_SERVER_NAME_SZ+1+1];
#ifdef WOLFSSL_ENCRYPTED_KEYS
    char                        priKeyPassword[AT_CMD_TLS_PRIKEY_PW_SZ+1+1];
#endif
    int                         numSessions;
} ATCMD_APP_TLS_CONF;

typedef struct
{
    bool                isClient;
    ATCMD_APP_TLS_CONF  *pTlsConf;
    WOLFSSL_CTX         *pTlsCtx;
    int                 numCtxSessions;
} ATCMD_APP_TLS_STATE;

typedef struct
{
    uint8_t     macAddr[6];
    uint8_t     devName[32+1];
} ATCMD_APP_SYS_CONF;

typedef struct
{
    uint8_t     broker[AT_CMD_MQTT_BROKER_SZ+1];
    int         port;
    uint8_t     clientID[AT_CMD_MQTT_CLIENTID_SZ+1+1];
    uint8_t     username[AT_CMD_MQTT_USERNAME_SZ+1];
    uint8_t     password[AT_CMD_MQTT_PASSWORD_SZ+1];
    int         keepAlive;
    int         tlsConfIdx;
} ATCMD_APP_MQTT_CONF;

typedef struct
{
    ATCMD_MQTT_SESSION_STATE    state;
} ATCMD_APP_MQTT_STATE;

#ifdef WOLFMQTT_V5
typedef struct
{
    char        pairs[AT_CMD_MQTT_USER_PROP_STORE_SZ];
    int         num;
    int         length;
} ATCMD_APP_MQTT_USER_PROP_STORE;

typedef struct
{
    int                             sessionExpiryInt;
    ATCMD_APP_MQTT_USER_PROP_STORE  user;
} ATCMD_APP_MQTT_PROP_TX_CONF;

typedef struct
{
    int                             sessionExpiryInt;
    int                             topicAliasMax;
    ATCMD_APP_MQTT_USER_PROP_STORE  user;
} ATCMD_APP_MQTT_PROP_RX_CONF;
#endif

typedef struct
{
    DRV_HANDLE                  wdrvHandle;
    ATCMD_APP_STATE             appState;
    bool                        wscnScanInProgress;
    int                         wscnNumOfBSSs;
    int                         wscnScanReqIdx;
    int                         wscnChannel;
    int                         wscnSSIDFiltLen;
    uint8_t                     wscnSSIDFilt[AT_CMD_SSID_SZ];
    bool                        otaFwInProgress;
    ATCMD_APP_WSCN_CONF         wscnConf;
    ATCMD_APP_WSTA_CONF         wstaConf;
    ATCMD_APP_WSTA_STATE        wstaConnState;
    ATCMD_APP_WAP_CONF          wapConf;
    ATCMD_APP_WAP_STATE         wapConnState;
    ATCMD_APP_WPROV_CONF        wprovConf;
    ATCMD_APP_WPROV_STATE       wprovState;
    uint8_t                     certFile[AT_CMD_CERT_FILE_MAX_SZ];
    int                         certFileLength;
    uint8_t                     priKeyFile[AT_CMD_PRIKEY_FILE_MAX_SZ];
    int                         priKeyFileLength;
    PKCS1_RSA_PRIVATE_KEY       priKey;
    int                         timeFormat;
    ATCMD_APP_SYS_CONF          sysConf;
    WDRV_PIC32MZW_ASSOC_HANDLE  assocHandle;
    TCPIP_NET_HANDLE            netHandle;
    TCPIP_DHCP_HANDLE           dhcpcHandle;
    TCPIP_SNTP_HANDLE           sntpHandle;
    int                         wstaStateTimeout;
    uint32_t                    wstaStateTimeStartMs;
    ATCMD_APP_TLS_CONF          tlsConf[AT_CMD_TLS_NUM_CONFS];
    ATCMD_APP_TLS_STATE         tlsState[AT_CMD_TLS_NUM_STATES];
    ATCMD_APP_MQTT_CONF         mqttConf;
    ATCMD_APP_MQTT_STATE        mqttState;
#ifdef WOLFMQTT_V5
    ATCMD_APP_MQTT_PROP_TX_CONF mqttPropTxConf;
    ATCMD_APP_MQTT_PROP_RX_CONF mqttPropRxConf;
#endif
} ATCMD_APP_CONTEXT;

const char* ATCMD_APPExecuteGMR(void);
const char* ATCMD_APPTranslateStatusCode(ATCMD_APP_STATUS statusCode);
const AT_CMD_CERT_ENTRY* ATCMD_APPCertFind(const char *pName);
const AT_CMD_PRIKEY_ENTRY* ATCMD_APPPriKeyFind(const char *pName);
void ATCMD_APPInit(void);
void ATCMD_APPUpdate(void);
bool ATCMD_APPStateMachineEvent(ATCMD_APP_EVENT event, bool commit);
ATCMD_APP_STATE ATCMD_APPStateMachineCurrentState(void);

ATCMD_STATUS ATCMD_WAP_Start(bool activeProvisioning);
ATCMD_STATUS ATCMD_WAP_Stop(void);

void ATCMD_PING_Callback(uint32_t ipAddress, uint32_t rtt, uint8_t errorCode);

#endif /* _AT_CMD_APP_H */
