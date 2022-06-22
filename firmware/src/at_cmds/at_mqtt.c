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
// ANY_CLOUD_RN #include "third_party/wolfmqtt/wolfmqtt/mqtt_client.h"
#include "wolfssl/ssl.h"
#include "wolfssl/wolfcrypt/logging.h"
#include "wolfssl/wolfcrypt/random.h"
#include "mqtt_app.h"

extern MQTT_APP_DATA mqtt_appData;

SYS_MODULE_OBJ g_sSysMqttHandle = SYS_MODULE_OBJ_INVALID;

/*******************************************************************************
* Command interface prototypes
*******************************************************************************/
static ATCMD_STATUS _MQTTInit(const AT_CMD_TYPE_DESC* pCmdTypeDesc);
static ATCMD_STATUS _MQTTCExecute(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const int numParams, ATCMD_PARAM *pParamList);
static ATCMD_STATUS _MQTTCONNExecute(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const int numParams, ATCMD_PARAM *pParamList);
static ATCMD_STATUS _MQTTSUBExecute(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const int numParams, ATCMD_PARAM *pParamList);
static ATCMD_STATUS _MQTTUNSUBExecute(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const int numParams, ATCMD_PARAM *pParamList);
static ATCMD_STATUS _MQTTPUBExecute(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const int numParams, ATCMD_PARAM *pParamList);
static ATCMD_STATUS _MQTTLWTExecute(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const int numParams, ATCMD_PARAM *pParamList);
static ATCMD_STATUS _MQTTDISCONNExecute(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const int numParams, ATCMD_PARAM *pParamList);
#ifdef WOLFMQTT_V5
static ATCMD_STATUS _MQTTPROPTXExecute(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const int numParams, ATCMD_PARAM *pParamList);
static ATCMD_STATUS _MQTTPROPRXExecute(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const int numParams, ATCMD_PARAM *pParamList);
static ATCMD_STATUS _MQTTPROPTXSExecute(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const int numParams, ATCMD_PARAM *pParamList);
#endif
static ATCMD_STATUS _MQTTUpdate(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const AT_CMD_TYPE_DESC* pCurrentCmdTypeDesc);

/*******************************************************************************
* Command parameters
*******************************************************************************/
static const ATCMD_HELP_PARAM paramID =
    {"ID", "Parameter ID number", ATCMD_PARAM_TYPE_CLASS_INTEGER, 0};

#if 0
static const ATCMD_HELP_PARAM paramKEY =
    {"KEY", "Parameter key", ATCMD_PARAM_TYPE_CLASS_STRING, 0};
#endif

static const ATCMD_HELP_PARAM paramVAL =
    {"VAL", "Parameter value", ATCMD_PARAM_TYPE_CLASS_ANY, 0};

#if 0
static const ATCMD_HELP_PARAM paramVALS =
    {"VAL", "Parameter value", ATCMD_PARAM_TYPE_CLASS_STRING, 0};
#endif

static const ATCMD_HELP_PARAM paramCLEAN =
    {"CLEAN", "Clean Session", ATCMD_PARAM_TYPE_CLASS_INTEGER, 0,
        .numOpts = 2,
        {
            {"0", "Use existing session, if available"},
            {"1", "Use new session"}
        }
};

static const ATCMD_HELP_PARAM paramTOPIC_NAME =
    {"TOPIC_NAME", "Topic Name", ATCMD_PARAM_TYPE_CLASS_STRING, 0};

static const ATCMD_HELP_PARAM paramMAX_QOS =
    {"MAX_QOS", "Maximum QoS", ATCMD_PARAM_TYPE_CLASS_INTEGER, 0,
        .numOpts = 3,
        {
            {"0", "QoS 0"},
            {"1", "QoS 1"},
            {"2", "QoS 2"}
        }
};

static const ATCMD_HELP_PARAM paramDUP =
    {"DUP", "Duplicate Message", ATCMD_PARAM_TYPE_CLASS_INTEGER, 0,
        .numOpts = 2,
        {
            {"0", "New message"},
            {"1", "Duplicate message"}
        }
};

static const ATCMD_HELP_PARAM paramRETAIN =
    {"RETAIN", "Retain Message", ATCMD_PARAM_TYPE_CLASS_INTEGER, 0,
        .numOpts = 2,
        {
            {"0", "Not retained on the server"},
            {"1", "Retained on the server"}
        }
};

static const ATCMD_HELP_PARAM paramQOS =
    {"QOS", "QoS", ATCMD_PARAM_TYPE_CLASS_INTEGER, 0,
        .numOpts = 3,
        {
            {"0", "QoS 0"},
            {"1", "QoS 1"},
            {"2", "QoS 2"}
        }
};

static const ATCMD_HELP_PARAM paramTOPIC_PAYLOAD =
    {"TOPIC_PAYLOAD", "Topic Payload", ATCMD_PARAM_TYPE_CLASS_STRING, 0};

#ifdef WOLFMQTT_V5
static const ATCMD_HELP_PARAM paramREASON_CODE =
    {"REASON_CODE", "Reason Code", ATCMD_PARAM_TYPE_CLASS_INTEGER, 0};

static const ATCMD_HELP_PARAM paramPROP_ID =
    {"PROP_ID", "Property Identifier", ATCMD_PARAM_TYPE_CLASS_INTEGER, 0};

static const ATCMD_HELP_PARAM paramPROP_SEL =
    {"PROP_SEL", "Property Selected", ATCMD_PARAM_TYPE_CLASS_INTEGER, 0,
        .numOpts = 2,
        {
            {"0", "Property not selected"},
            {"1", "Property is selected"}
        }
};
#endif

/*******************************************************************************
* Command examples
*******************************************************************************/

/*******************************************************************************
* Command descriptors
*******************************************************************************/
const AT_CMD_TYPE_DESC atCmdTypeDescMQTTC =
    {
        .pCmdName   = "+MQTTC",
        .cmdInit    = _MQTTInit,
        .cmdExecute = _MQTTCExecute,
        .cmdUpdate  = NULL,
        .pSummary   = "This command is used to read or set the MQTT configuration",
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

const AT_CMD_TYPE_DESC atCmdTypeDescMQTTCONN =
    {
        .pCmdName   = "+MQTTCONN",
        .cmdInit    = _MQTTInit,
        .cmdExecute = _MQTTCONNExecute,
        .cmdUpdate  = _MQTTUpdate,
        .pSummary   = "This is used to connect to an MQTT broker",
        .numVars    = 1,
        {
            {
                .numParams   = 1,
                .pParams     =
                {
                    &paramCLEAN
                },
                .numExamples = 0,
                .pExamples   =
                {
                    NULL
                }
            }
        }
    };

const AT_CMD_TYPE_DESC atCmdTypeDescMQTTSUB =
    {
        .pCmdName   = "+MQTTSUB",
        .cmdInit    = NULL,
        .cmdExecute = _MQTTSUBExecute,
        .cmdUpdate  = NULL,
        .pSummary   = "This is used to subscribe to an MQTT topic",
        .numVars    = 1,
        {
            {
                .numParams   = 2,
                .pParams     =
                {
                    &paramTOPIC_NAME,
                    &paramMAX_QOS
                },
                .numExamples = 0,
                .pExamples   =
                {
                    NULL
                }
            }
        }
    };

const AT_CMD_TYPE_DESC atCmdTypeDescMQTTUNSUB =
    {
        .pCmdName   = "+MQTTUNSUB",
        .cmdInit    = NULL,
        .cmdExecute = _MQTTUNSUBExecute,
        .cmdUpdate  = NULL,
        .pSummary   = "This is used to unsubscribe from an MQTT topic",
        .numVars    = 1,
        {
            {
                .numParams   = 1,
                .pParams     =
                {
                    &paramTOPIC_NAME
                },
                .numExamples = 0,
                .pExamples   =
                {
                    NULL
                }
            }
        }
    };

const AT_CMD_TYPE_DESC atCmdTypeDescMQTTPUB =
    {
        .pCmdName   = "+MQTTPUB",
        .cmdInit    = NULL,
        .cmdExecute = _MQTTPUBExecute,
        .cmdUpdate  = NULL,
        .pSummary   = "This is used to publish a message",
        .numVars    = 1,
        {
            {
                .numParams   = 5,
                .pParams     =
                {
                    &paramDUP,
                    &paramQOS,
                    &paramRETAIN,
                    &paramTOPIC_NAME,
                    &paramTOPIC_PAYLOAD
                },
                .numExamples = 0,
                .pExamples   =
                {
                    NULL
                }
            }
        }
    };

const AT_CMD_TYPE_DESC atCmdTypeDescMQTTLWT =
    {
        .pCmdName   = "+MQTTLWT",
        .cmdInit    = NULL,
        .cmdExecute = _MQTTLWTExecute,
        .cmdUpdate  = NULL,
        .pSummary   = "This is used to define a last will message",
        .numVars    = 1,
        {
            {
                .numParams   = 4,
                .pParams     =
                {
                    &paramQOS,
                    &paramRETAIN,
                    &paramTOPIC_NAME,
                    &paramTOPIC_PAYLOAD
                },
                .numExamples = 0,
                .pExamples   =
                {
                    NULL
                }
            }
        }
    };

const AT_CMD_TYPE_DESC atCmdTypeDescMQTTDISCONN =
    {
        .pCmdName   = "+MQTTDISCONN",
        .cmdInit    = NULL,
        .cmdExecute = _MQTTDISCONNExecute,
        .cmdUpdate  = NULL,
        .pSummary   = "This is used to disconnect from a broker",
#ifdef WOLFMQTT_V5
        .numVars    = 2,
#else
        .numVars    = 1,
#endif
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
#ifdef WOLFMQTT_V5
            },
            {
                .numParams   = 1,
                .pParams     =
                {
                    &paramREASON_CODE
                },
                .numExamples = 0,
                .pExamples   =
                {
                    NULL
                }
#endif
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
#ifdef DEBUG_MQTT
#define MQTT_DEBUG_PRINTF(str, ...)     ATCMD_Printf("#mqtt: " str "\r\n", ##__VA_ARGS__)
#else
#define MQTT_DEBUG_PRINTF(str, ...)
#endif

#define MQTTC_MAP_MAX_PARAMS        7
#ifdef WOLFMQTT_V5
#define MQTTPROPTX_MAP_MAX_PARAMS   42
#define MQTTPROPRX_MAP_MAX_PARAMS   42
#endif

#define MQTT_PROP_01_TYPE   ((MQTT_DATA_TYPE_BYTE    << 16) | (1 << MQTT_PACKET_TYPE_PUBLISH))
#define MQTT_PROP_02_TYPE   ((MQTT_DATA_TYPE_INT     << 16) | (1 << MQTT_PACKET_TYPE_PUBLISH))
#define MQTT_PROP_03_TYPE   ((MQTT_DATA_TYPE_STRING  << 16) | (1 << MQTT_PACKET_TYPE_PUBLISH))
#define MQTT_PROP_08_TYPE   ((MQTT_DATA_TYPE_STRING  << 16) | (1 << MQTT_PACKET_TYPE_PUBLISH))
#define MQTT_PROP_09_TYPE   ((MQTT_DATA_TYPE_BINARY  << 16) | (1 << MQTT_PACKET_TYPE_PUBLISH))
#define MQTT_PROP_11_TYPE   ((MQTT_DATA_TYPE_VAR_INT << 16) | (1 << MQTT_PACKET_TYPE_PUBLISH) | (1 << MQTT_PACKET_TYPE_SUBSCRIBE))
#define MQTT_PROP_17_TYPE   ((MQTT_DATA_TYPE_INT     << 16) | (1 << MQTT_PACKET_TYPE_CONNECT) | (1 << MQTT_PACKET_TYPE_CONNECT_ACK) | (1 << MQTT_PACKET_TYPE_DISCONNECT))
#define MQTT_PROP_18_TYPE   ((MQTT_DATA_TYPE_STRING  << 16) | (1 << MQTT_PACKET_TYPE_CONNECT_ACK))
#define MQTT_PROP_19_TYPE   ((MQTT_DATA_TYPE_SHORT   << 16) | (1 << MQTT_PACKET_TYPE_CONNECT_ACK))
#define MQTT_PROP_21_TYPE   ((MQTT_DATA_TYPE_STRING  << 16) | (1 << MQTT_PACKET_TYPE_CONNECT) | (1 << MQTT_PACKET_TYPE_CONNECT_ACK) | (1 << MQTT_PACKET_TYPE_AUTH))
#define MQTT_PROP_22_TYPE   ((MQTT_DATA_TYPE_BINARY  << 16) | (1 << MQTT_PACKET_TYPE_CONNECT) | (1 << MQTT_PACKET_TYPE_CONNECT_ACK) | (1 << MQTT_PACKET_TYPE_AUTH))
#define MQTT_PROP_23_TYPE   ((MQTT_DATA_TYPE_BYTE    << 16) | (1 << MQTT_PACKET_TYPE_CONNECT))
#define MQTT_PROP_24_TYPE   ((MQTT_DATA_TYPE_INT     << 16) | (1 << MQTT_PACKET_TYPE_PUBLISH))
#define MQTT_PROP_25_TYPE   ((MQTT_DATA_TYPE_BYTE    << 16) | (1 << MQTT_PACKET_TYPE_CONNECT))
#define MQTT_PROP_26_TYPE   ((MQTT_DATA_TYPE_STRING  << 16) | (1 << MQTT_PACKET_TYPE_CONNECT_ACK))
#define MQTT_PROP_28_TYPE   ((MQTT_DATA_TYPE_STRING  << 16) | (1 << MQTT_PACKET_TYPE_CONNECT_ACK) | (1 << MQTT_PACKET_TYPE_DISCONNECT))
#define MQTT_PROP_31_TYPE   ((MQTT_DATA_TYPE_STRING  << 16) | (1 << MQTT_PACKET_TYPE_CONNECT_ACK) | (1 << MQTT_PACKET_TYPE_PUBLISH_ACK) | (1 << MQTT_PACKET_TYPE_PUBLISH_REC) |\
                                                              (1 << MQTT_PACKET_TYPE_PUBLISH_REL) | (1 << MQTT_PACKET_TYPE_PUBLISH_COMP) | (1 << MQTT_PACKET_TYPE_SUBSCRIBE_ACK) |\
                                                              (1 << MQTT_PACKET_TYPE_UNSUBSCRIBE_ACK) | (1 << MQTT_PACKET_TYPE_DISCONNECT) | (1 << MQTT_PACKET_TYPE_AUTH))
#define MQTT_PROP_33_TYPE   ((MQTT_DATA_TYPE_SHORT   << 16) | (1 << MQTT_PACKET_TYPE_CONNECT) | (1 << MQTT_PACKET_TYPE_CONNECT_ACK))
#define MQTT_PROP_34_TYPE   ((MQTT_DATA_TYPE_SHORT   << 16) | (1 << MQTT_PACKET_TYPE_CONNECT) | (1 << MQTT_PACKET_TYPE_CONNECT_ACK))
#define MQTT_PROP_35_TYPE   ((MQTT_DATA_TYPE_SHORT   << 16) | (1 << MQTT_PACKET_TYPE_PUBLISH))
#define MQTT_PROP_36_TYPE   ((MQTT_DATA_TYPE_BYTE    << 16) | (1 << MQTT_PACKET_TYPE_CONNECT_ACK))
#define MQTT_PROP_37_TYPE   ((MQTT_DATA_TYPE_BYTE    << 16) | (1 << MQTT_PACKET_TYPE_CONNECT_ACK))
#define MQTT_PROP_38_TYPE   ((MQTT_DATA_TYPE_STRING_PAIR << 16) | 0xFFFF)
#define MQTT_PROP_39_TYPE   ((MQTT_DATA_TYPE_INT     << 16) | (1 << MQTT_PACKET_TYPE_CONNECT) | (1 << MQTT_PACKET_TYPE_CONNECT_ACK))
#define MQTT_PROP_40_TYPE   ((MQTT_DATA_TYPE_BYTE    << 16) | (1 << MQTT_PACKET_TYPE_CONNECT_ACK))
#define MQTT_PROP_41_TYPE   ((MQTT_DATA_TYPE_BYTE    << 16) | (1 << MQTT_PACKET_TYPE_CONNECT_ACK))
#define MQTT_PROP_42_TYPE   ((MQTT_DATA_TYPE_BYTE    << 16) | (1 << MQTT_PACKET_TYPE_CONNECT_ACK))

typedef enum
{
    ATCMD_MQTT_EVENT_NET_REQ,
    ATCMD_MQTT_EVENT_NET_CONN,
    ATCMD_MQTT_EVENT_NET_REQ_TLS,
    ATCMD_MQTT_EVENT_NET_TLS_CONN,
    ATCMD_MQTT_EVENT_CONNECT_SENT,
    ATCMD_MQTT_EVENT_CONNACK_RCV,
    ATCMD_MQTT_EVENT_SUBSCRIBE_SENT,
    ATCMD_MQTT_EVENT_SUBSCRIBE_RCV,
    ATCMD_MQTT_EVENT_UNSUBSCRIBE_SENT,
    ATCMD_MQTT_EVENT_UNSUBSCRIBE_RCV,
    ATCMD_MQTT_EVENT_PUBLISH_SENT,
    ATCMD_MQTT_EVENT_PUBLISH_COMP,
    ATCMD_MQTT_EVENT_DISCONNECT_SENT,
    ATCMD_MQTT_EVENT_DISCONNECT,
    ATCMD_MQTT_EVENT_ERROR,
    ATCMD_MQTT_EVENT_RESET,
    ATCMD_MQTT_EVENT_TIMEOUT
} ATCMD_MQTT_EVENT;

typedef enum
{
    ATCMD_MQTT_NET_STATE_UNCONNECTED,
    ATCMD_MQTT_NET_STATE_DNS_RESOLV,
    ATCMD_MQTT_NET_STATE_SKT_CONNECTING,
    ATCMD_MQTT_NET_STATE_WAIT_TLS,
    ATCMD_MQTT_NET_STATE_CONNECTED,
    ATCMD_MQTT_NET_STATE_ERROR
} ATCMD_MQTT_NET_STATE;

typedef struct
{
    
} MqttConnect;

typedef struct
{
    MqttConnect     msg;
#ifdef WOLFMQTT_V5
    MqttProp        props[];
#endif
} ATCMD_MQTT_CONNMSG_CTX;

typedef struct
{
    
} MqttPublish;

typedef struct
{
    MqttPublish     msg;
    char            topicName[AT_CMD_MQTT_TOPIC_SZ+1];
    uint8_t         topicPayload[];
} ATCMD_MQTT_PUBMSG_CTX;

typedef struct
{
    
} MqttSubscribe;

typedef struct
{
    
} MqttTopic;

typedef struct
{
    MqttSubscribe   msg;
    MqttTopic       topic;
    char            topicName[AT_CMD_MQTT_TOPIC_SZ+1];
#ifdef WOLFMQTT_V5
    MqttProp        props[];
#endif
} ATCMD_MQTT_SUBMSG_CTX;

typedef struct
{
    
} MqttUnsubscribe;

typedef struct
{
    MqttUnsubscribe msg;
    MqttTopic       topic;
    char            topicName[AT_CMD_MQTT_TOPIC_SZ+1];
#ifdef WOLFMQTT_V5
    MqttProp        props[];
#endif
} ATCMD_MQTT_UNSUBMSG_CTX;

typedef struct
{
    
} MqttDisconnect;

typedef struct
{
    MqttDisconnect  msg;
#ifdef WOLFMQTT_V5
    MqttProp        props[];
#endif
} ATCMD_MQTT_DISCONNMSG_CTX;

#ifdef WOLFMQTT_V5
typedef struct
{
    uint32_t        enabled[2];
} ATCMD_MQTT_PROP_MASK;
#endif

typedef struct
{
    ATCMD_MQTT_NET_STATE        state;
    bool                        autoConnect;
    TCPIP_DNS_HANDLE            dnsResolveHandle;
    IP_MULTI_ADDRESS            ipAddress;
    int16_t                     transHandle;
    const void*                 sigHandler;
    int                         wrOffset;
    int                         rdOffset;
    uint16_t                    pktID;
    WOLFSSL                     *pWolfSSLSession;
    ATCMD_MQTT_CONNMSG_CTX      *pMQTTConnMsg;
    ATCMD_MQTT_SUBMSG_CTX       *pMQTTSubMsg;
    ATCMD_MQTT_UNSUBMSG_CTX     *pMQTTUnsubMsg;
    ATCMD_MQTT_PUBMSG_CTX       *pMQTTPubMsg;
    ATCMD_MQTT_PUBMSG_CTX       *pMQTTLWTMsg;
    ATCMD_MQTT_DISCONNMSG_CTX   *pMQTTDisconnMsg;
#ifdef WOLFMQTT_V5
    ATCMD_MQTT_PROP_MASK        propTXEnabled;
#endif
} ATCMD_MQTT_CONTEXT;

/*******************************************************************************
* Local data
*******************************************************************************/
static const ATCMD_STORE_MAP_ELEMENT mqttConfMap[] = {
    {.id=1,  .offset=offsetof(ATCMD_APP_MQTT_CONF, broker),     .type=ATCMD_STORE_TYPE_STRING,     .maxSize=AT_CMD_MQTT_BROKER_SZ,     .access=ATCMD_STORE_ACCESS_RW},
    {.id=2,  .offset=offsetof(ATCMD_APP_MQTT_CONF, port),       .type=ATCMD_STORE_TYPE_INT,        .maxSize=sizeof(int),               .access=ATCMD_STORE_ACCESS_RW},
    {.id=3,  .offset=offsetof(ATCMD_APP_MQTT_CONF, clientID),   .type=ATCMD_STORE_TYPE_STRING,     .maxSize=AT_CMD_MQTT_CLIENTID_SZ,   .access=ATCMD_STORE_ACCESS_RW},
    {.id=4,  .offset=offsetof(ATCMD_APP_MQTT_CONF, username),   .type=ATCMD_STORE_TYPE_STRING,     .maxSize=AT_CMD_MQTT_USERNAME_SZ,   .access=ATCMD_STORE_ACCESS_RW},
    {.id=5,  .offset=offsetof(ATCMD_APP_MQTT_CONF, password),   .type=ATCMD_STORE_TYPE_STRING,     .maxSize=AT_CMD_MQTT_PASSWORD_SZ,   .access=ATCMD_STORE_ACCESS_RW},
    {.id=6,  .offset=offsetof(ATCMD_APP_MQTT_CONF, keepAlive),  .type=ATCMD_STORE_TYPE_INT,        .maxSize=sizeof(int),               .access=ATCMD_STORE_ACCESS_RW},
    {.id=7,  .offset=offsetof(ATCMD_APP_MQTT_CONF, tlsConfIdx), .type=ATCMD_STORE_TYPE_INT,        .maxSize=sizeof(int),               .access=ATCMD_STORE_ACCESS_RW},

    {.id=0, .type=ATCMD_STORE_TYPE_INVALID}
};

#ifdef WOLFMQTT_V5
static const ATCMD_STORE_MAP_ELEMENT mqttPropTxMap[] = {
    {.id=17, .offset=offsetof(ATCMD_APP_MQTT_PROP_TX_CONF, sessionExpiryInt),   .type=ATCMD_STORE_TYPE_INT,     .maxSize=sizeof(int),   .access=ATCMD_STORE_ACCESS_RW,  .userType=MQTT_PROP_17_TYPE},

    {.id=0, .type=ATCMD_STORE_TYPE_INVALID}
};

static const ATCMD_STORE_MAP_ELEMENT mqttPropRxMap[] = {
    {.id=17, .offset=offsetof(ATCMD_APP_MQTT_PROP_RX_CONF, sessionExpiryInt),   .type=ATCMD_STORE_TYPE_INT,     .maxSize=sizeof(int),   .access=ATCMD_STORE_ACCESS_RW,  .userType=MQTT_PROP_17_TYPE},
    {.id=34, .offset=offsetof(ATCMD_APP_MQTT_PROP_RX_CONF, topicAliasMax),      .type=ATCMD_STORE_TYPE_INT,     .maxSize=sizeof(int),   .access=ATCMD_STORE_ACCESS_RW,  .userType=MQTT_PROP_34_TYPE},

    {.id=0, .type=ATCMD_STORE_TYPE_INVALID}
};
#endif

typedef struct
{
	SYS_MODULE_OBJ mqtt_handle;
} MqttClient;

typedef struct
{
    
} MqttNet;

static MqttClient mqttClient;
//static uint32_t lastKeepAliveTimeMs;
static uint32_t lastStateTransitionMs;
static uint32_t currentStateTimeoutMs;

/*******************************************************************************
* Local functions
*******************************************************************************/
static MqttClient* _mqttGetClientState(void)
{
	return &mqttClient;
}
	
int32_t APP_MQTT_GetStatus(void *p) 
{
	MqttClient *pMQTTClient = NULL;
	
	pMQTTClient = _mqttGetClientState();
	
	if (NULL == pMQTTClient)
	{
		return ATCMD_APP_STATUS_MQTT_ERROR;
	}
		
	return SYS_MQTT_GetStatus(pMQTTClient->mqtt_handle);
}

//static bool _mqttPublish(MqttClient *pMQTTClient);
#ifdef WOLFMQTT_V5
static ATCMD_STATUS _mqttDisconnectStart(MqttClient *pMQTTClient, int reasonCode);
#else
static ATCMD_STATUS _mqttDisconnectStart(MqttClient *pMQTTClient);
#endif

#ifdef WOLFMQTT_V5
static void _mqttPropMaskSet(ATCMD_MQTT_PROP_MASK *pMask, unsigned int id, bool sel)
{
    if ((NULL == pMask) || (0 == id) || (id > 42))
    {
        return;
    }

    id--;

    if (true == sel)
    {
        pMask->enabled[id >> 5] |= (1 << (id % 32));
    }
    else
    {
        pMask->enabled[id >> 5] &= ~(1 << (id % 32));
    }
}

static bool _mqttPropMaskGet(ATCMD_MQTT_PROP_MASK *pMask, unsigned int id)
{
    if ((NULL == pMask) || (0 == id) || (id > 42))
    {
        return false;
    }

    id--;

    if (0 != (pMask->enabled[id >> 5] & (1 << (id % 32))))
    {
        return true;
    }
    else
    {
        return false;
    }
}

static void _mqttUserPropStoreClear(ATCMD_APP_MQTT_USER_PROP_STORE *pUserProp)
{
    if (NULL == pUserProp)
    {
        return;
    }

    memset(pUserProp, 0, sizeof(ATCMD_APP_MQTT_USER_PROP_STORE));
}

static bool _mqttUserPropStoreRemovePair(ATCMD_APP_MQTT_USER_PROP_STORE *pUserProp, const char *pK, int kLen)
{
    int i;
    char *pS;
    char *pD = NULL;
    bool key;

    if ((NULL == pUserProp) || (NULL == pK) || (0 == kLen))
    {
        return false;
    }

    if (pUserProp->length > 0)
    {
        i = pUserProp->length;
        pS = pUserProp->pairs;

        key = true;

        while (i)
        {
            int l;

            if ('\0' == *pS)
            {
                pS++;
                i--;
                continue;
            }

            l = strlen(pS);

            if (0 != l)
            {
                if (true == key)
                {
                    if (NULL != pD)
                    {
                        memmove(pD, pS, i);

                        pUserProp->length -= (pS - pD);
                        pUserProp->num--;

                        memset(&pUserProp->pairs[pUserProp->length], 0, AT_CMD_MQTT_USER_PROP_STORE_SZ - pUserProp->length);

                        return true;
                    }

                    if ((kLen == l) && (0 == memcmp(pS, pK, l)))
                    {
                        pD = pS;
                    }

                    key = false;
                }
                else
                {
                    key = true;
                }

                pS += l;
                i -= l;
            }
        }

        if (NULL != pD)
        {
            pUserProp->length -= (pS - pD);
            pUserProp->num--;

            memset(&pUserProp->pairs[pUserProp->length], 0, AT_CMD_MQTT_USER_PROP_STORE_SZ - pUserProp->length);
            return true;
        }
    }

    return false;
}

static bool _mqttUserPropStoreAddPair(ATCMD_APP_MQTT_USER_PROP_STORE *pUserProp, const char *pK, int kLen, const char *pV, int vLen)
{
    char *pD;

    if ((NULL == pUserProp) || (NULL == pK) || (0 == kLen) || (NULL == pV) || (0 == vLen))
    {
        return false;
    }

    if ((pUserProp->length + kLen + vLen + 2) > AT_CMD_MQTT_USER_PROP_STORE_SZ)
    {
        return false;
    }

    pD = &pUserProp->pairs[pUserProp->length];

    if (pUserProp->length > 0)
    {
        *pD++ = '\0';
    }
    memcpy(pD, pK, kLen);
    pD += kLen;

    *pD++ = '\0';
    memcpy(pD, pV, vLen);

    pUserProp->length += (kLen + vLen + 2);
    pUserProp->num++;

    return true;
}

static void _mqttUserPropDisplay(const char *pATCmd, ATCMD_APP_MQTT_USER_PROP_STORE *pUserProp)
{
    int i;
    char *pS;
    bool key;

    if ((NULL == pATCmd) || (NULL == pUserProp))
    {
        return;
    }

    if (pUserProp->length > 0)
    {
        i = pUserProp->length;
        pS = pUserProp->pairs;

        key = true;

        while (i)
        {
            int l;

            if ('\0' == *pS)
            {
                pS++;
                i--;
                continue;
            }

            l = strlen(pS);

            if (0 != l)
            {
                if (true == key)
                {
                    ATCMD_Printf("%s:%d,", pATCmd, MQTT_PROP_USER_PROP);
                    ATCMD_PrintStringSafe(pS, l);
                    ATCMD_Print(",", 1);
                    key = false;
                }
                else
                {
                    ATCMD_PrintStringSafe(pS, l);
                    ATCMD_Print("\r\n", 2);
                    key = true;
                }

                pS += l;
                i -= l;
            }
        }
    }
}

static bool _mqttAddProp(MqttPacketType msgType, MqttProp *pProp, ATCMD_APP_MQTT_USER_PROP_STORE *pUserProp, bool isLWT, ATCMD_MQTT_PROP_MASK *pEnabledProps)
{
    const ATCMD_STORE_MAP_ELEMENT *pmqttConfMapElem;
    uint8_t *pElemPtr;
    MqttDataType propDataType;
    int propMsgType;
    bool propAdded = false;

    if (NULL == pProp)
    {
        return false;
    }

    pmqttConfMapElem = mqttPropTxMap;

    while (NULL != pmqttConfMapElem)
    {
        /* Extract the property data type and allowed message bitmask. */
        propDataType = pmqttConfMapElem->userType >> 16;
        propMsgType  = pmqttConfMapElem->userType & 0xffff;

        if ((NULL != pEnabledProps) && (false == _mqttPropMaskGet(pEnabledProps, pmqttConfMapElem->id)))
        {
            propMsgType = 0;
        }
        else if (true == isLWT)
        {
            /* LWT/Publish messages cannot contain Topic Alias property. */
            if (MQTT_PROP_TOPIC_ALIAS == pmqttConfMapElem->id)
            {
                propMsgType = 0;
            }
        }
        else
        {
            /* Normal publish messages cannot contain Will Delay Interval property. */
            if (MQTT_PROP_WILL_DELAY_INTERVAL == pmqttConfMapElem->id)
            {
                propMsgType = 0;
            }
        }

        /* Check if property can be included in this message type. */
        if (0 != (propMsgType & (1 << msgType)))
        {
            pElemPtr = &((uint8_t*)&atCmdAppContext.mqttPropTxConf)[pmqttConfMapElem->offset];

            pProp->type = pmqttConfMapElem->id;

            switch (propDataType)
            {
                case MQTT_DATA_TYPE_BYTE:
                {
                    break;
                }

                case MQTT_DATA_TYPE_SHORT:
                {
                    break;
                }

                case MQTT_DATA_TYPE_INT:
                {
                    pProp->data_int = *((int*)pElemPtr);
                    break;
                }

                case MQTT_DATA_TYPE_STRING:
                {
                    break;
                }

                case MQTT_DATA_TYPE_VAR_INT:
                {
                    break;
                }

                case MQTT_DATA_TYPE_BINARY:
                {
                    break;
                }

                case MQTT_DATA_TYPE_STRING_PAIR:
                {
                    break;
                }

                default:
                {
                    return false;
                }
            }

            pProp->next = &pProp[1];
            pProp++;
            propAdded = true;
        }

        pmqttConfMapElem = ATCMD_StructStoreFindNext(pmqttConfMapElem);
    }

    if (pUserProp->num > 0)
    {
        if ((NULL == pEnabledProps) || (true == _mqttPropMaskGet(pEnabledProps, MQTT_PROP_USER_PROP)))
        {
            int i;
            char *pS;

            pS = pUserProp->pairs;

            for (i=0; i<pUserProp->num; i++)
            {
                pProp->type = MQTT_PROP_USER_PROP;

                while ('\0' == *pS)
                {
                    pS++;
                }

                pProp->data_str.str = pS;
                pProp->data_str.len = strlen(pS);
                pS += pProp->data_str.len;

                while ('\0' == *pS)
                {
                    pS++;
                }

                pProp->data_str2.str = pS;
                pProp->data_str2.len = strlen(pS);
                pS += pProp->data_str2.len;

                pProp->next = &pProp[1];
                pProp++;
                propAdded = true;
            }
        }
    }

    if (true == propAdded)
    {
        pProp--;
        pProp->next = NULL;
    }

    return propAdded;
}
#endif


#ifdef WOLFMQTT_V5
static int _mqttPropertyCallback(struct _MqttClient* pClient, MqttProp* pProp, void* pCtx)
{
    const ATCMD_STORE_MAP_ELEMENT *pmqttConfMapElem;
//    ATCMD_MQTT_CONTEXT *pMQTTCtx = (ATCMD_MQTT_CONTEXT*)pCtx;
    uint8_t *pElemPtr;
    bool aecStarted = false;
    bool userPropCleared = false;

    if ((NULL == pClient) || (NULL == pProp) || (NULL == pCtx))
    {
        return MQTT_CODE_ERROR_CALLBACK;
    }

    while (NULL != pProp)
    {
        pmqttConfMapElem = ATCMD_StructStoreFindElementByID(mqttPropRxMap, pProp->type);

        if (NULL != pmqttConfMapElem)
        {
            MqttDataType propDataType;

            if (false == aecStarted)
            {
                ATCMD_Printf("+MQTTPROPRX:%d", pProp->type);
                aecStarted = true;
            }
            else
            {
                ATCMD_Printf(",%d", pProp->type);
            }

            propDataType = pmqttConfMapElem->userType >> 16;

            pElemPtr = &((uint8_t*)&atCmdAppContext.mqttPropRxConf)[pmqttConfMapElem->offset];

            switch (propDataType)
            {
                case MQTT_DATA_TYPE_BYTE:
                {
                    *((int*)pElemPtr) = pProp->data_byte;
                    break;
                }

                case MQTT_DATA_TYPE_SHORT:
                {
                    *((int*)pElemPtr) = pProp->data_short;
                    break;
                }

                case MQTT_DATA_TYPE_INT:
                {
                    *((int*)pElemPtr) = pProp->data_int;
                    break;
                }

                case MQTT_DATA_TYPE_STRING:
                {
                    break;
                }

                case MQTT_DATA_TYPE_VAR_INT:
                {
                    break;
                }

                case MQTT_DATA_TYPE_BINARY:
                {
                    break;
                }

                case MQTT_DATA_TYPE_STRING_PAIR:
                {
                    break;
                }

                default:
                {
                    return MQTT_CODE_ERROR_CALLBACK;
                }
            }
        }
        else if (MQTT_PROP_USER_PROP == pProp->type)
        {
            if (false == userPropCleared)
            {
                _mqttUserPropStoreClear(&atCmdAppContext.mqttPropRxConf.user);
                userPropCleared = true;
            }

            if (false == _mqttUserPropStoreAddPair(&atCmdAppContext.mqttPropRxConf.user, pProp->data_str.str, pProp->data_str.len, pProp->data_str2.str, pProp->data_str2.len))
            {
                return MQTT_CODE_ERROR_CALLBACK;
            }

            if (false == aecStarted)
            {
                ATCMD_Printf("+MQTTPROPRX:%d", pProp->type);
                aecStarted = true;
            }
            else
            {
                ATCMD_Printf(",%d", pProp->type);
            }
        }

        pProp = pProp->next;
    }

    if (true == aecStarted)
    {
        ATCMD_Print("\r\n", 2);
    }

    return MQTT_CODE_SUCCESS;
}

#endif

int32_t _MQTTCallback(SYS_MQTT_EVENT_TYPE eEventType, void *data, uint16_t len, void* cookie);

static ATCMD_STATUS _mqttConnectStart(MqttClient *pMQTTClient, int cleanSession)
{
    SYS_MQTT_Config cloudConfig;

    if (NULL == pMQTTClient)
    {
        return ATCMD_APP_STATUS_MQTT_ERROR;
    }
	
    cloudConfig = g_sSysMqttConfig; /*take a copy of the global config and modify just what is required*/
	
    strncpy(cloudConfig.sBrokerConfig.brokerName, (char *)&atCmdAppContext.mqttConf.broker[1], SYS_MQTT_MAX_BROKER_NAME_LEN);
    strncpy(cloudConfig.sBrokerConfig.username, (char *)&atCmdAppContext.mqttConf.username[1], SYS_MQTT_USER_NAME_MAX_LEN);
    strncpy(cloudConfig.sBrokerConfig.clientId, (char *)&atCmdAppContext.mqttConf.clientID[1], SYS_MQTT_CLIENT_ID_MAX_LEN);
    cloudConfig.sBrokerConfig.serverPort = atCmdAppContext.mqttConf.port;
	
	pMQTTClient->mqtt_handle = SYS_MQTT_Connect(&cloudConfig, _MQTTCallback, NULL);

	mqtt_appData.SysMqttHandle = pMQTTClient->mqtt_handle;

	return ATCMD_STATUS_OK;
}


#ifdef WOLFMQTT_V5
static ATCMD_STATUS _mqttDisconnectStart(MqttClient *pMQTTClient, int reasonCode)
#else
static ATCMD_STATUS _mqttDisconnectStart(MqttClient *pMQTTClient)
#endif
{
    if (0 == pMQTTClient || 0 == pMQTTClient->mqtt_handle)
    {
        return ATCMD_APP_STATUS_MQTT_ERROR;
    }

	SYS_MQTT_Disconnect(pMQTTClient->mqtt_handle);

    return ATCMD_STATUS_OK;
}


/*******************************************************************************
* Command init functions
*******************************************************************************/
static ATCMD_STATUS _MQTTInit(const AT_CMD_TYPE_DESC* pCmdTypeDesc)
{
    int s;

    memset(&atCmdAppContext.mqttConf, 0, sizeof(ATCMD_APP_MQTT_CONF));

    atCmdAppContext.mqttConf.port = 8883;

    s = atCmdAppContext.sysConf.devName[0];

    if (s > (AT_CMD_MQTT_CLIENTID_SZ-1))
    {
        s = (AT_CMD_MQTT_CLIENTID_SZ-1);
    }

    memcpy(atCmdAppContext.mqttConf.clientID, atCmdAppContext.sysConf.devName, s+1);

    atCmdAppContext.mqttState.state = ATCMD_MQTT_SESSION_STATE_NOT_CONNECTED;

	mqttClient.mqtt_handle = SYS_MODULE_OBJ_INVALID;

    return ATCMD_STATUS_OK;
}

static char topicName[AT_CMD_MQTT_TOPIC_SZ+1];

/*******************************************************************************
* Command execute functions
*******************************************************************************/
static ATCMD_STATUS _MQTTCExecute(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const int numParams, ATCMD_PARAM *pParamList)
{
    if (0 == numParams)
    {
        int id;

        /* Dump all configuration elements */

        for (id=1; id<=MQTTC_MAP_MAX_PARAMS; id++)
        {
            /* Read the element from the configuration structure */

            ATCMD_StructStorePrint(pCmdTypeDesc->pCmdName, mqttConfMap, &atCmdAppContext.mqttConf, id);
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

        if (false == ATCMD_StructStorePrint(pCmdTypeDesc->pCmdName, mqttConfMap, &atCmdAppContext.mqttConf, pParamList[0].value.i))
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

        if (0 == ATCMD_StructStoreWriteParam(mqttConfMap, &atCmdAppContext.mqttConf, pParamList[0].value.i, &pParamList[1]))
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

static ATCMD_STATUS _MQTTCONNExecute(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const int numParams, ATCMD_PARAM *pParamList)
{
    if (0 == numParams)
    {
       	int32_t status = APP_MQTT_GetStatus(NULL);
        if ((SYS_MQTT_STATUS_MQTT_CONNECTED == status) ||
			(SYS_MQTT_STATUS_WAIT_FOR_MQTT_SUBACK == status) ||
			(SYS_MQTT_STATUS_WAIT_FOR_MQTT_PUBACK == status) ||
			(SYS_MQTT_STATUS_WAIT_FOR_MQTT_UNSUBACK == status))
        {
            ATCMD_Printf("+MQTTCONN:1\r\n");
        }
        else
        {
            ATCMD_Printf("+MQTTCONN:0\r\n");
        }
    }
    else if (1 == numParams)
    {
        MqttClient *pMQTTClient;

        if (false == ATCMD_ParamValidateTypes(pCmdTypeDesc, 0, numParams, pParamList))
        {
            return ATCMD_STATUS_INVALID_PARAMETER;
        }

        if (pParamList[0].value.i > 1)
        {
            return ATCMD_STATUS_INVALID_PARAMETER;
        }

        pMQTTClient = _mqttGetClientState();

        return _mqttConnectStart(pMQTTClient, pParamList[0].value.i);
    }
    else
    {
        return ATCMD_STATUS_INCORRECT_NUM_PARAMS;
    }

    return ATCMD_STATUS_OK;
}

static ATCMD_STATUS _MQTTSUBExecute(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const int numParams, ATCMD_PARAM *pParamList)
{
    MqttClient *pMQTTClient = NULL;
	SYS_MQTT_SubscribeConfig subConfig;

    if (2 != numParams)
    {
        return ATCMD_STATUS_INCORRECT_NUM_PARAMS;
    }

    if (pParamList[0].length > AT_CMD_MQTT_TOPIC_SZ)
    {
        return ATCMD_STATUS_INVALID_PARAMETER;
    }

    if (SYS_MQTT_STATUS_MQTT_CONNECTED != APP_MQTT_GetStatus(NULL))
    {
        return ATCMD_APP_STATUS_MQTT_ERROR;
    }

    pMQTTClient = _mqttGetClientState();

    if (NULL == pMQTTClient)
    {
        return ATCMD_APP_STATUS_MQTT_ERROR;
    }

	memset(&subConfig, 0, sizeof(subConfig));

	memcpy(subConfig.topicName, pParamList[0].value.p, pParamList[0].length);
	subConfig.qos = pParamList[1].value.i;
	
	SYS_MQTT_Subscribe(pMQTTClient->mqtt_handle, &subConfig);

    return ATCMD_STATUS_OK;
}

static ATCMD_STATUS _MQTTUNSUBExecute(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const int numParams, ATCMD_PARAM *pParamList)
{
    MqttClient *pMQTTClient = NULL;

    if (1 != numParams)
    {
        return ATCMD_STATUS_INCORRECT_NUM_PARAMS;
    }

    if (pParamList[0].length > AT_CMD_MQTT_TOPIC_SZ)
    {
        return ATCMD_STATUS_INVALID_PARAMETER;
    }

    if (SYS_MQTT_STATUS_MQTT_CONNECTED != APP_MQTT_GetStatus(NULL))
    {
        return ATCMD_APP_STATUS_MQTT_ERROR;
    }

    pMQTTClient = _mqttGetClientState();

    if (NULL == pMQTTClient)
    {
        return ATCMD_APP_STATUS_MQTT_ERROR;
    }

	memset(topicName, 0, sizeof(topicName));

	memcpy(topicName, pParamList[0].value.p, pParamList[0].length);

	SYS_MQTT_Unsubscribe(pMQTTClient->mqtt_handle, topicName);

    return ATCMD_STATUS_OK;
}

static ATCMD_STATUS _MQTTPUBExecute(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const int numParams, ATCMD_PARAM *pParamList)
{
    MqttClient *pMQTTClient = NULL;
	SYS_MQTT_PublishTopicCfg pubConfig;
	
    if (5 != numParams)
    {
        return ATCMD_STATUS_INCORRECT_NUM_PARAMS;
    }

    pMQTTClient = _mqttGetClientState();

    if (NULL == pMQTTClient)
    {
        return ATCMD_APP_STATUS_MQTT_ERROR;
    }

    if (SYS_MQTT_STATUS_MQTT_CONNECTED != APP_MQTT_GetStatus(NULL))
    {
        return ATCMD_APP_STATUS_MQTT_ERROR;
    }
	
	memset(&pubConfig, 0, sizeof(pubConfig));
	memcpy(pubConfig.topicName, pParamList[3].value.p, pParamList[3].length);

	pubConfig.qos = pParamList[1].value.i;
	pubConfig.retain = pParamList[2].value.i;

	SYS_MQTT_Publish(pMQTTClient->mqtt_handle, &pubConfig, 
            (char *)pParamList[4].value.p, (uint16_t)pParamList[4].length);

    return ATCMD_STATUS_OK;
}

static ATCMD_STATUS _MQTTLWTExecute(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const int numParams, ATCMD_PARAM *pParamList)
{
//    ATCMD_STATUS comStatus;
    MqttClient *pMQTTClient = NULL;
    ATCMD_MQTT_CONTEXT *pMQTTCtx = NULL;

    pMQTTClient = _mqttGetClientState();

    if (NULL == pMQTTClient)
    {
        return ATCMD_APP_STATUS_MQTT_ERROR;
    }

    //pMQTTCtx = (ATCMD_MQTT_CONTEXT*)pMQTTClient->net->context;

    if (NULL == pMQTTCtx)
    {
        return ATCMD_APP_STATUS_MQTT_ERROR;
    }

    return false;
}

static ATCMD_STATUS _MQTTDISCONNExecute(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const int numParams, ATCMD_PARAM *pParamList)
{
    MqttClient *pMQTTClient = NULL;

#ifdef WOLFMQTT_V5
    if (numParams > 1)
    {
        return ATCMD_STATUS_INCORRECT_NUM_PARAMS;
    }
#else
    if (numParams > 0)
    {
        return ATCMD_STATUS_INCORRECT_NUM_PARAMS;
    }
#endif

    if (SYS_MQTT_STATUS_MQTT_CONNECTED != APP_MQTT_GetStatus(NULL))
    {
        return ATCMD_APP_STATUS_MQTT_ERROR;
    }

    pMQTTClient = _mqttGetClientState();

    if (NULL == pMQTTClient)
    {
        return ATCMD_APP_STATUS_MQTT_ERROR;
    }

#ifdef WOLFMQTT_V5
    if (1 == numParams)
    {
        return _mqttDisconnectStart(pMQTTClient, pParamList[0].value.i);
    }
    else
    {
        return _mqttDisconnectStart(pMQTTClient, MQTT_REASON_NORMAL_DISCONNECTION);
    }
#else
    return _mqttDisconnectStart(pMQTTClient);
#endif
}

#ifdef WOLFMQTT_V5
static ATCMD_STATUS _MQTTPROPTXExecute(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const int numParams, ATCMD_PARAM *pParamList)
{
    if (0 == numParams)
    {
        int id;

        /* Dump all configuration elements */

        for (id=1; id<=MQTTPROPTX_MAP_MAX_PARAMS; id++)
        {
            /* Read the element from the configuration structure */

            if (id == MQTT_PROP_USER_PROP)
            {
                _mqttUserPropDisplay(pCmdTypeDesc->pCmdName, &atCmdAppContext.mqttPropTxConf.user);
            }
            else
            {
                ATCMD_StructStorePrint(pCmdTypeDesc->pCmdName, mqttPropTxMap, &atCmdAppContext.mqttPropTxConf, id);
            }
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

        if (pParamList[0].value.i == MQTT_PROP_USER_PROP)
        {
            _mqttUserPropDisplay(pCmdTypeDesc->pCmdName, &atCmdAppContext.mqttPropTxConf.user);
        }
        else
        {
            /* Access the element in the configuration structure */

            if (false == ATCMD_StructStorePrint(pCmdTypeDesc->pCmdName, mqttPropTxMap, &atCmdAppContext.mqttPropTxConf, pParamList[0].value.i))
            {
                return ATCMD_STATUS_STORE_ACCESS_FAILED;
            }
        }
    }
    else if (2 == numParams)
    {
        /* Check the parameter types are correct */

        if (false == ATCMD_ParamValidateTypes(pCmdTypeDesc, 2, numParams, pParamList))
        {
            return ATCMD_STATUS_INVALID_PARAMETER;
        }

        if (MQTT_PROP_USER_PROP == pParamList[0].value.i)
        {
            if (false == _mqttUserPropStoreRemovePair(&atCmdAppContext.mqttPropTxConf.user, (char*)pParamList[1].value.p, pParamList[1].length))
            {
                return ATCMD_STATUS_INVALID_PARAMETER;
            }
        }
        else
        {
            /* Access the element in the configuration structure */

            if (0 == ATCMD_StructStoreWriteParam(mqttPropTxMap, &atCmdAppContext.mqttPropTxConf, pParamList[0].value.i, &pParamList[1]))
            {
                return ATCMD_STATUS_STORE_ACCESS_FAILED;
            }
        }
    }
    else if (3 == numParams)
    {
        /* Check the parameter types are correct */

        if (false == ATCMD_ParamValidateTypes(pCmdTypeDesc, 3, numParams, pParamList))
        {
            return ATCMD_STATUS_INVALID_PARAMETER;
        }

        if (MQTT_PROP_USER_PROP != pParamList[0].value.i)
        {
            return ATCMD_STATUS_INVALID_PARAMETER;
        }

        if (false == _mqttUserPropStoreAddPair(&atCmdAppContext.mqttPropTxConf.user, (char*)pParamList[1].value.p, pParamList[1].length, (char*)pParamList[2].value.p, pParamList[2].length))
        {
            return ATCMD_STATUS_INVALID_PARAMETER;
        }
    }
    else
    {
        return ATCMD_STATUS_INCORRECT_NUM_PARAMS;
    }

    return ATCMD_STATUS_OK;
}

static ATCMD_STATUS _MQTTPROPRXExecute(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const int numParams, ATCMD_PARAM *pParamList)
{
    if (0 == numParams)
    {
        int id;

        /* Dump all configuration elements */

        for (id=1; id<=MQTTPROPRX_MAP_MAX_PARAMS; id++)
        {
            /* Read the element from the configuration structure */

            if (id == MQTT_PROP_USER_PROP)
            {
                _mqttUserPropDisplay(pCmdTypeDesc->pCmdName, &atCmdAppContext.mqttPropRxConf.user);
            }
            else
            {
                ATCMD_StructStorePrint(pCmdTypeDesc->pCmdName, mqttPropRxMap, &atCmdAppContext.mqttPropRxConf, id);
            }
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

        if (MQTT_PROP_USER_PROP == pParamList[0].value.i)
        {
            _mqttUserPropDisplay(pCmdTypeDesc->pCmdName, &atCmdAppContext.mqttPropRxConf.user);
        }
        else
        {
            /* Access the element in the configuration structure */

            if (false == ATCMD_StructStorePrint(pCmdTypeDesc->pCmdName, mqttPropRxMap, &atCmdAppContext.mqttPropRxConf, pParamList[0].value.i))
            {
                return ATCMD_STATUS_STORE_ACCESS_FAILED;
            }
        }
    }
    else
    {
        return ATCMD_STATUS_INCORRECT_NUM_PARAMS;
    }

    return ATCMD_STATUS_OK;
}

static ATCMD_STATUS _MQTTPROPTXSExecute(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const int numParams, ATCMD_PARAM *pParamList)
{
    MqttClient *pMQTTClient;
    ATCMD_MQTT_CONTEXT* pMQTTCtx;

    if (0 == numParams)
    {
        return ATCMD_STATUS_OK;
    }
    else if (numParams > 2)
    {
        return ATCMD_STATUS_INCORRECT_NUM_PARAMS;
    }

    pMQTTClient = _mqttGetClientState();

    if (NULL == pMQTTClient)
    {
        return ATCMD_APP_STATUS_MQTT_ERROR;
    }

    pMQTTCtx = (ATCMD_MQTT_CONTEXT*)pMQTTClient->net->context;

    if (NULL == pMQTTCtx)
    {
        return ATCMD_APP_STATUS_MQTT_ERROR;
    }

    if (1 == numParams)
    {
        /* Check the parameter types are correct */

        if (false == ATCMD_ParamValidateTypes(pCmdTypeDesc, 1, numParams, pParamList))
        {
            return ATCMD_STATUS_INVALID_PARAMETER;
        }
    }
    else
    {
        /* Check the parameter types are correct */

        if (false == ATCMD_ParamValidateTypes(pCmdTypeDesc, 2, numParams, pParamList))
        {
            return ATCMD_STATUS_INVALID_PARAMETER;
        }
    }

    if ((0 == pParamList[0].value.u) || (pParamList[0].value.u > 42))
    {
        return ATCMD_STATUS_INVALID_PARAMETER;
    }

    if (1 == numParams)
    {
        int sel;

        if (true == _mqttPropMaskGet(&pMQTTCtx->propTXEnabled, pParamList[0].value.u))
        {
            sel = 1;
        }
        else
        {
            sel = 0;
        }

        ATCMD_Printf("+MQTTPROPTXS:%d,%d\r\n", pParamList[0].value.u, sel);
    }
    else
    {
        if (pParamList[1].value.u > 1)
        {
            return ATCMD_STATUS_INVALID_PARAMETER;
        }

        _mqttPropMaskSet(&pMQTTCtx->propTXEnabled, pParamList[0].value.u, (pParamList[1].value.i == 1) ? true : false);
    }

    return ATCMD_STATUS_OK;
}
#endif

/*******************************************************************************
* Command update functions
*******************************************************************************/
static ATCMD_STATUS _MQTTUpdate(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const AT_CMD_TYPE_DESC* pCurrentCmdTypeDesc)
{
    MqttClient *pMQTTClient;

    if (ATCMD_MQTT_SESSION_STATE_NOT_CONNECTED == atCmdAppContext.mqttState.state)
    {
        return ATCMD_STATUS_OK;
    }

    pMQTTClient = _mqttGetClientState();

    if (NULL == pMQTTClient)
    {
        return ATCMD_APP_STATUS_MQTT_ERROR;
    }

    if (currentStateTimeoutMs > 0)
    {
        if ((ATCMD_PlatformGetSysTimeMs() - lastStateTransitionMs) > currentStateTimeoutMs)
        {
//            _mqttStateMachineEvent(ATCMD_MQTT_EVENT_TIMEOUT);
        }
    }

    return ATCMD_STATUS_OK;
}

int32_t _MQTTCallback(SYS_MQTT_EVENT_TYPE eEventType, void *data, uint16_t len, void* cookie)
{
		switch (eEventType) {
			case SYS_MQTT_EVENT_MSG_RCVD:
			{
				/* Message received on Subscribed Topic */
				SYS_MQTT_PublishConfig	*psMsg = (SYS_MQTT_PublishConfig	*)data;
//				psMsg->message[psMsg->messageLength] = 0;
//				psMsg->topicName[psMsg->topicLength] = 0;
				ATCMD_Printf("+MQTTPUB:%d,", psMsg->topicLength);
				ATCMD_PrintStringSafe(psMsg->topicName, psMsg->topicLength);
				ATCMD_Printf(",%d,", psMsg->messageLength);
				ATCMD_PrintStringSafe((char*)psMsg->message, psMsg->messageLength);
				ATCMD_Printf("\r\n");
//				SYS_CONSOLE_PRINT("%s: %s\r", psMsg->topicName, psMsg->message);
			}
				break;
	
			case SYS_MQTT_EVENT_MSG_DISCONNECTED:
			{
				ATCMD_Printf("+MQTTCONN:0\r\n");
			}
				break;
	
			case SYS_MQTT_EVENT_MSG_CONNECTED:
			{
	//			SYS_CONSOLE_PRINT("\nMqttCallback(): Connected\r\n");
                                ATCMD_Printf("+MQTTCONNACK:0,0\r\n");
				ATCMD_Printf("+MQTTCONN:1\r\n");
			}
				break;
	
			case SYS_MQTT_EVENT_MSG_SUBSCRIBED:
			{
//				SYS_MQTT_SubscribeConfig	*psMqttSubCfg = (SYS_MQTT_SubscribeConfig	*)data;
				ATCMD_Printf("+MQTTSUB:0\r\n");
			}
				break;
	
			case SYS_MQTT_EVENT_MSG_UNSUBSCRIBED:
			{
				/* MQTT Topic Unsubscribed; Now the Client will not receive any messages for this Topic */
				ATCMD_Printf("+MQTTUNSUB:1\r\n");
			}
				break;
	
			case SYS_MQTT_EVENT_MSG_PUBLISHED:
			{
				/* MQTT Client Msg Published */
				ATCMD_Printf("+MQTTPUBACC\r\n");
			}
				break;
	
			case SYS_MQTT_EVENT_MSG_CONNACK_TO:
			{
				/* MQTT Client ConnAck TimeOut; User will need to reconnect again */
                                ATCMD_Printf("+MQTTCONNACK:1,1\r\n");
				ATCMD_Printf("+MQTTCONN:0\r\n");
			}
				break;
	
			case SYS_MQTT_EVENT_MSG_SUBACK_TO:
			{
				/* MQTT Client SubAck TimeOut; User will need to subscribe again */
				ATCMD_Printf("+MQTTSUB:1\r\n");
			}
				break;
	
			case SYS_MQTT_EVENT_MSG_PUBACK_TO:
			{
				/* MQTT Client PubAck TimeOut; User will need to publish again */
				ATCMD_Printf("+MQTTPUBERR\r\n");
			}
				break;
	
			case SYS_MQTT_EVENT_MSG_UNSUBACK_TO:
			{
				/* MQTT Client UnSubAck TimeOut; User will need to Unsubscribe again */
				ATCMD_Printf("+MQTTUNSUB:0\r\n");
			}
				break;
	
		}
		return 0;
}


