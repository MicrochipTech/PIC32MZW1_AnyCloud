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
#include "at_cmds/at_cmd_inet.h"
#include "wolfssl/ssl.h"
#include "wolfssl/wolfcrypt/logging.h"
#include "wolfssl/wolfcrypt/random.h"

/*******************************************************************************
* Command interface prototypes
*******************************************************************************/
static ATCMD_STATUS _SOCKInit(const AT_CMD_TYPE_DESC* pCmdTypeDesc);
static ATCMD_STATUS _SOCKOExecute(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const int numParams, ATCMD_PARAM *pParamList);
static ATCMD_STATUS _SOCKBLExecute(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const int numParams, ATCMD_PARAM *pParamList);
static ATCMD_STATUS _SOCKBRExecute(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const int numParams, ATCMD_PARAM *pParamList);
static ATCMD_STATUS _SOCKTLSExecute(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const int numParams, ATCMD_PARAM *pParamList);
static ATCMD_STATUS _SOCKWRExecute(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const int numParams, ATCMD_PARAM *pParamList);
static ATCMD_STATUS _SOCKWRTOExecute(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const int numParams, ATCMD_PARAM *pParamList);
static ATCMD_STATUS _SOCKRDExecute(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const int numParams, ATCMD_PARAM *pParamList);
static ATCMD_STATUS _SOCKCLExecute(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const int numParams, ATCMD_PARAM *pParamList);
static ATCMD_STATUS _SOCKLSTExecute(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const int numParams, ATCMD_PARAM *pParamList);
static ATCMD_STATUS _SOCKBMExecute(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const int numParams, ATCMD_PARAM *pParamList);
static ATCMD_STATUS _SOCKUpdate(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const AT_CMD_TYPE_DESC* pCurrentCmdTypeDesc);

/*******************************************************************************
* Command parameters
*******************************************************************************/
static const ATCMD_HELP_PARAM paramPROTOCOL =
    {"PROTOCOL", "The protocol to use", ATCMD_PARAM_TYPE_CLASS_INTEGER,
        .numOpts = 2,
        {
            {"1", "UDP"},
            {"2", "TCP"}
        }
    };

static const ATCMD_HELP_PARAM paramLCL_PORT =
    {"LCL_PORT", "The local port number to use", ATCMD_PARAM_TYPE_CLASS_INTEGER, 0};

static const ATCMD_HELP_PARAM paramRMT_ADDR =
    {"RMT_ADDR", "The address of the remote device", ATCMD_PARAM_TYPE_CLASS_STRING, 0};

static const ATCMD_HELP_PARAM paramRMT_PORT =
    {"RMT_PORT", "The port number on the remote device", ATCMD_PARAM_TYPE_CLASS_INTEGER, 0};

static const ATCMD_HELP_PARAM paramSOCK_ID =
    {"SOCK_ID", "The socket ID", ATCMD_PARAM_TYPE_CLASS_INTEGER, 0};

static const ATCMD_HELP_PARAM paramLENGTH_WR =
    {"LENGTH", "The length of the data to send (1 – 1500 bytes)", ATCMD_PARAM_TYPE_CLASS_INTEGER, 0};

static const ATCMD_HELP_PARAM paramDATA =
    {"DATA", "The data to send in either ASCII or hexadecimal string format. If omitted the DCE will enter raw binary mode and will remain in that mode until the specified length of binary data has been received from the DTE", ATCMD_PARAM_TYPE_CLASS_STRING, 0};

static const ATCMD_HELP_PARAM paramOUTPUT_MODE =
    {"OUTPUT_MODE", "The format the DTE wishes to receive the data", ATCMD_PARAM_TYPE_CLASS_INTEGER,
        .numOpts = 2,
        {
            {"1", "ASCII or hex string"},
            {"2", "Binary"}
        }
    };

static const ATCMD_HELP_PARAM paramLENGTH_RD =
    {"LENGTH", "The number of bytes the DTE wishes to read", ATCMD_PARAM_TYPE_CLASS_INTEGER, 0};

static const ATCMD_HELP_PARAM paramMCAST_ADDR =
    {"MCAST_ADDR", "The address of the multicast group", ATCMD_PARAM_TYPE_CLASS_STRING, 0};

static const ATCMD_HELP_PARAM paramMCAST_PORT =
    {"MCAST_PORT", "The port number of the multicast group", ATCMD_PARAM_TYPE_CLASS_INTEGER, 0};

static const ATCMD_HELP_PARAM paramTLS_CONF =
    {"TLS_CONF", "TLS certificate configuration", ATCMD_PARAM_TYPE_CLASS_INTEGER, 0};

/*******************************************************************************
* Command examples
*******************************************************************************/

/*******************************************************************************
* Command descriptors
*******************************************************************************/
const AT_CMD_TYPE_DESC atCmdTypeDescSOCKO =
    {
        .pCmdName   = "+SOCKO",
        .cmdInit    = _SOCKInit,
        .cmdExecute = _SOCKOExecute,
        .cmdUpdate  = NULL,
        .pSummary   = "This command is used to open a new socket",
        .numVars    = 1,
        {
            {
                .numParams   = 1,
                .pParams     =
                {
                    &paramPROTOCOL
                },
                .numExamples = 0,
                .pExamples   =
                {
                    NULL
                }
            }
        }
    };

const AT_CMD_TYPE_DESC atCmdTypeDescSOCKBL =
    {
        .pCmdName   = "+SOCKBL",
        .cmdInit    = NULL,
        .cmdExecute = _SOCKBLExecute,
        .cmdUpdate  = NULL,
        .pSummary   = "This command is used to bind a socket to a local port",
        .numVars    = 1,
        {
            {
                .numParams   = 2,
                .pParams     =
                {
                    &paramSOCK_ID,
                    &paramLCL_PORT
                },
                .numExamples = 0,
                .pExamples   =
                {
                    NULL
                }
            }
        }
    };

const AT_CMD_TYPE_DESC atCmdTypeDescSOCKBR =
    {
        .pCmdName   = "+SOCKBR",
        .cmdInit    = NULL,
        .cmdExecute = _SOCKBRExecute,
        .cmdUpdate  = NULL,
        .pSummary   = "This command is used to bind a socket to a remote address",
        .numVars    = 1,
        {
            {
                .numParams   = 3,
                .pParams     =
                {
                    &paramSOCK_ID,
                    &paramRMT_ADDR,
                    &paramRMT_PORT
                },
                .numExamples = 0,
                .pExamples   =
                {
                    NULL
                }
            }
        }
    };

const AT_CMD_TYPE_DESC atCmdTypeDescSOCKBM =
    {
        .pCmdName   = "+SOCKBM",
        .cmdInit    = NULL,
        .cmdExecute = _SOCKBMExecute,
        .cmdUpdate  = NULL,
        .pSummary   = "This command is used to bind a socket to a multicast group",
        .numVars    = 1,
        {
            {
                .numParams   = 3,
                .pParams     =
                {
                    &paramSOCK_ID,
                    &paramMCAST_ADDR,
                    &paramMCAST_PORT
                },
                .numExamples = 0,
                .pExamples   =
                {
                    NULL
                }
            }
        }
    };

const AT_CMD_TYPE_DESC atCmdTypeDescSOCKTLS =
    {
        .pCmdName   = "+SOCKTLS",
        .cmdInit    = NULL,
        .cmdExecute = _SOCKTLSExecute,
        .cmdUpdate  = NULL,
        .pSummary   = "This command is used to enable TLS on a socket",
        .numVars    = 1,
        {
            {
                .numParams   = 2,
                .pParams     =
                {
                    &paramSOCK_ID,
                    &paramTLS_CONF
                },
                .numExamples = 0,
                .pExamples   =
                {
                    NULL
                }
            }
        }
    };

const AT_CMD_TYPE_DESC atCmdTypeDescSOCKWR =
    {
        .pCmdName   = "+SOCKWR",
        .cmdInit    = NULL,
        .cmdExecute = _SOCKWRExecute,
        .cmdUpdate  = NULL,
        .pSummary   = "This command is used to send data over a socket that is bound to a remote address and port number",
        .numVars    = 2,
        {
            {
                .numParams   = 3,
                .pParams     =
                {
                    &paramSOCK_ID,
                    &paramLENGTH_WR,
                    &paramDATA
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
                    &paramSOCK_ID,
                    &paramLENGTH_WR
                },
                .numExamples = 0,
                .pExamples   =
                {
                    NULL
                }
            }
        }
    };

const AT_CMD_TYPE_DESC atCmdTypeDescSOCKWRTO =
    {
        .pCmdName   = "+SOCKWRTO",
        .cmdInit    = NULL,
        .cmdExecute = _SOCKWRTOExecute,
        .cmdUpdate  = NULL,
        .pSummary   = "This command is used to send data to an arbitrary destination using the connectionless UDP protocol",
        .numVars    = 2,
        {
            {
                .numParams   = 5,
                .pParams     =
                {
                    &paramSOCK_ID,
                    &paramRMT_ADDR,
                    &paramRMT_PORT,
                    &paramLENGTH_WR,
                    &paramDATA
                },
                .numExamples = 0,
                .pExamples   =
                {
                    NULL
                }
            },
            {
                .numParams   = 4,
                .pParams     =
                {
                    &paramSOCK_ID,
                    &paramRMT_ADDR,
                    &paramRMT_PORT,
                    &paramLENGTH_WR
                },
                .numExamples = 0,
                .pExamples   =
                {
                    NULL
                }
            }
        }
    };

const AT_CMD_TYPE_DESC atCmdTypeDescSOCKRD =
    {
        .pCmdName   = "+SOCKRD",
        .cmdInit    = NULL,
        .cmdExecute = _SOCKRDExecute,
        .cmdUpdate  = _SOCKUpdate,
        .pSummary   = "This command is used to read data from a socket",
        .numVars    = 1,
        {
            {
                .numParams   = 3,
                .pParams     =
                {
                    &paramSOCK_ID,
                    &paramOUTPUT_MODE,
                    &paramLENGTH_RD
                },
                .numExamples = 0,
                .pExamples   =
                {
                    NULL
                }
            }
        }
    };

const AT_CMD_TYPE_DESC atCmdTypeDescSOCKCL =
    {
        .pCmdName   = "+SOCKCL",
        .cmdInit    = NULL,
        .cmdExecute = _SOCKCLExecute,
        .cmdUpdate  = NULL,
        .pSummary   = "This command is used to close a socket",
        .numVars    = 1,
        {
            {
                .numParams   = 1,
                .pParams     =
                {
                    &paramSOCK_ID
                },
                .numExamples = 0,
                .pExamples   =
                {
                    NULL
                }
            }
        }
    };

const AT_CMD_TYPE_DESC atCmdTypeDescSOCKLST =
    {
        .pCmdName   = "+SOCKLST",
        .cmdInit    = NULL,
        .cmdExecute = _SOCKLSTExecute,
        .cmdUpdate  = NULL,
        .pSummary   = "This command is used to present a list of the DCE's open sockets/connections",
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
                    &paramSOCK_ID
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
extern int CheckAvailableSize(WOLFSSL *ssl, int size);
uint32_t g_binModeNumBytes = 0;

/*******************************************************************************
* Local defines and types
*******************************************************************************/
#define ATCMD_SOCK_BIND_TIMEOUT_MS  10000

typedef enum
{
    ATCMD_SOCK_PROTO_UDP = 1,
    ATCMD_SOCK_PROTO_TCP = 2
} ATCMD_SOCK_PROTO;

typedef enum
{
    ATCMD_SOCK_ENCRYPT_STATE_NONE = 0,
    ATCMD_SOCK_ENCRYPT_STATE_STARTING,
    ATCMD_SOCK_ENCRYPT_STATE_NEGOTIATING,
    ATCMD_SOCK_ENCRYPT_STATE_DONE,
    ATCMD_SOCK_ENCRYPT_STATE_FAILED,
    ATCMD_SOCK_ENCRYPT_STATE_CLOSED,
} ATCMD_SOCK_ENCRYPT_STATE;

typedef struct _ATCMD_SOCK_STATE
{
    bool                        inUse;
    bool                        needsEncryption;
    bool                        isConnected;
    struct _ATCMD_SOCK_STATE    *pParent;
    int                         handle;
    uint8_t                     provOpen;
    ATCMD_SOCK_ENCRYPT_STATE    encryptState;
    WOLFSSL                     *pWolfSSLSession;
    int                         tlsConfIdx;
    int16_t                     transHandle;
    const void*                 sigHandler;
    int                         protocol;
    uint16_t                    localPort;
    IPV4_ADDR                   remoteIPv4Addr;
    uint16_t                    remotePort;
    uint16_t                    pendingDataLength;
    uint32_t                    lastTimeMs;
    int                       childTransHandle[AT_CMD_SOCK_MAX_CLIENTS];
} ATCMD_SOCK_STATE;

typedef struct
{
    ATCMD_SOCK_STATE    *pSockState;
    uint8_t             buffer[AT_CMD_SOCK_RX_BUFFER_SZ];
    bool                isBinary;
    uint16_t            numBytesRequested;
    uint16_t            numBytesBuffered;
    uint8_t*            pBufPtr;
} ATCMD_SOCKRD_STATE;

/*******************************************************************************
* Local data
*******************************************************************************/
static ATCMD_SOCK_STATE socketState[AT_CMD_SOCK_MAX_NUM];
static ATCMD_SOCK_STATE *pSOCKWRBinarySock;
static int nextSocketHandle;
static ATCMD_SOCKRD_STATE sockRdState;

/*******************************************************************************
* Local functions
*******************************************************************************/
static uint16_t _sockTCPReadReady(ATCMD_SOCK_STATE *pSockState)
{
    uint16_t numBytes = 0;

    if (ATCMD_SOCK_ENCRYPT_STATE_NONE == pSockState->encryptState)
    {
        numBytes = TCPIP_TCP_GetIsReady(pSockState->transHandle);
    }
    else if (ATCMD_SOCK_ENCRYPT_STATE_DONE == pSockState->encryptState)
    {
        numBytes = wolfSSL_pending(pSockState->pWolfSSLSession);

        if (0 == numBytes)
        {
            char buffer;

            if (0 != wolfSSL_peek(pSockState->pWolfSSLSession, &buffer, 1))
            {
                numBytes = wolfSSL_pending(pSockState->pWolfSSLSession);
            }
        }
    }

    return numBytes;
}

static bool _sockTCPWriteReady(ATCMD_SOCK_STATE *pSockState, uint16_t numBufBytes)
{
    int sockPutReadyBytes;

    sockPutReadyBytes = TCPIP_TCP_PutIsReady(pSockState->transHandle);

    if (ATCMD_SOCK_ENCRYPT_STATE_NONE == pSockState->encryptState)
    {
        if (sockPutReadyBytes < numBufBytes)
        {
            return false;
        }
    }
    else if (ATCMD_SOCK_ENCRYPT_STATE_DONE == pSockState->encryptState)
    {
        char buffer;

        if (wolfSSL_write(pSockState->pWolfSSLSession, &buffer, 0) >= 0)
        {
            if (0 == CheckAvailableSize(pSockState->pWolfSSLSession, numBufBytes))
            {
                if (wolfSSL_GetOutputSize(pSockState->pWolfSSLSession, numBufBytes) > sockPutReadyBytes)
                {
                    return false;
                }
            }
        }
    }
    else
    {
        return false;
    }

    return true;
}

static void _sockErrorAEC(int handle, ATCMD_APP_STATUS statusCode)
{
    const char *pStatusMsg = ATCMD_APPTranslateStatusCode(statusCode);

    ATCMD_Printf("+SOCKERR:%d,%d", handle, statusCode);

    if (NULL != pStatusMsg)
    {
        ATCMD_Printf(",%s", pStatusMsg);
    }

    ATCMD_Printf("\r\n");
}

static int _getSocketHandle(void)
{
    if (0 == nextSocketHandle)
    {
        nextSocketHandle = ATCMD_PlatformGetSysTimeMs();
    }
    else
    {
        nextSocketHandle++;
    }

    if (0 == nextSocketHandle)
    {
        nextSocketHandle++;
    }

    return nextSocketHandle;
}

static ATCMD_SOCK_STATE* _findSocketByHandle(int handle)
{
    int i;

    for (i=0; i<AT_CMD_SOCK_MAX_NUM; i++)
    {
        if (false == socketState[i].inUse)
        {
            if (-1 == handle)
            {
                /* Asked for handle -1 (any) and found an empty structure */

                memset(&socketState[i], 0, sizeof(ATCMD_SOCK_STATE));

                return &socketState[i];
            }
        }
        else if (handle == socketState[i].handle)
        {
            /* Asked for a specific handle and found it */

            return &socketState[i];
        }
    }

    return NULL;
}

static ATCMD_SOCK_STATE* _findSocketByTransHandle(int transHandle)
{
    int i;

    for (i=0; i<AT_CMD_SOCK_MAX_NUM; i++)
    {
        if (transHandle == socketState[i].transHandle)
        {
            /* Asked for a specific handle and found it */

            return &socketState[i];
        }
    }

    return NULL;
}

static ATCMD_SOCK_STATE* _findUDPSocketByTransHandle(UDP_SOCKET transHandle)
{
    int i;

    for (i=0; i<AT_CMD_SOCK_MAX_NUM; i++)
    {
        if ((true == socketState[i].inUse) && (ATCMD_SOCK_PROTO_UDP == socketState[i].protocol) && (transHandle == socketState[i].transHandle))
        {
            /* Asked for a specific handle and found it */

            return &socketState[i];
        }
    }

    return NULL;
}

static void _closeSocket(ATCMD_SOCK_STATE *pSockState)
{
    if (NULL == pSockState)
    {
        return;
    }

    if (false == pSockState->inUse)
    {
        return;
    }

    if (-1 == pSockState->transHandle)
    {
        pSockState->inUse = false;
        return;
    }

    if (NULL != pSockState->pParent)
    {
        /* For server listening socket spawned sockets, just disconnect but
           leave the socket present for reuse. */
        if (true == TCPIP_TCP_Disconnect(pSockState->transHandle))
        {
            if (NULL != pSockState->pWolfSSLSession)
            {
                ATCMD_TLS_FreeSession(pSockState->tlsConfIdx, pSockState->pWolfSSLSession);
            }

            pSockState->encryptState = ATCMD_SOCK_ENCRYPT_STATE_NONE;
        }

        pSockState->remoteIPv4Addr.Val = 0;
        pSockState->remotePort         = 0;
        pSockState->pendingDataLength  = 0;
    }
    else
    {
        if (ATCMD_SOCK_PROTO_UDP == pSockState->protocol)
        {
            TCPIP_UDP_SignalHandlerDeregister(pSockState->transHandle, pSockState->sigHandler);

            TCPIP_UDP_Close(pSockState->transHandle);
        }
        else
        {
            TCPIP_TCP_SignalHandlerDeregister(pSockState->transHandle, pSockState->sigHandler);

            if (NULL != pSockState->pWolfSSLSession)
            {
                ATCMD_TLS_FreeSession(pSockState->tlsConfIdx, pSockState->pWolfSSLSession);
            }

            pSockState->encryptState = ATCMD_SOCK_ENCRYPT_STATE_NONE;

            TCPIP_TCP_Close(pSockState->transHandle);
        }

        pSockState->transHandle    = -1;
        pSockState->inUse          = false;
    }
}

static void _socketWriteBinaryDataHandler(const uint8_t *pBuf, size_t numBufBytes)
{
    if (NULL == pSOCKWRBinarySock)
    {
        return;
    }

    if (ATCMD_SOCK_PROTO_UDP == pSOCKWRBinarySock->protocol)
    {
        /* UDP socket, ensure remote address/port is set before trying to sendto */

        if ((0 == pSOCKWRBinarySock->remoteIPv4Addr.Val) || (0 == pSOCKWRBinarySock->remotePort))
        {
            return;
        }

        if (TCPIP_UDP_PutIsReady(pSOCKWRBinarySock->transHandle) < numBufBytes)
        {
            return;
        }

        if (0 == TCPIP_UDP_ArrayPut(pSOCKWRBinarySock->transHandle, pBuf, numBufBytes))
        {
            return;
        }

        TCPIP_UDP_Flush(pSOCKWRBinarySock->transHandle);
    }
    else if (ATCMD_SOCK_PROTO_TCP == pSOCKWRBinarySock->protocol)
    {
        if (false == _sockTCPWriteReady(pSOCKWRBinarySock, numBufBytes))
        {
            return;
        }

        if (ATCMD_SOCK_ENCRYPT_STATE_NONE == pSOCKWRBinarySock->encryptState)
        {
            if (0 == TCPIP_TCP_ArrayPut(pSOCKWRBinarySock->transHandle, pBuf, numBufBytes))
            {
                return;
            }
        }
        else
        {
            if (wolfSSL_write(pSOCKWRBinarySock->pWolfSSLSession, pBuf, numBufBytes) < 0)
            {
                return;
            }
        }
    }
    else
    {
        return;
    }
}

static void _tcpSocketSignalHandler(TCP_SOCKET hTCP, TCPIP_NET_HANDLE hNet, TCPIP_TCP_SIGNAL_TYPE sigType, const void* param)
{
    ATCMD_SOCK_STATE *const pSockState = (ATCMD_SOCK_STATE *const)param;

//    ATCMD_Printf("TSH(%d): (0x%08x) 0x%04x\r\n", hTCP, param, sigType);

    if (true == ATCMD_ModeIsBinary())
    {
        return;        
    }
    
    if (NULL == pSockState)
    {
        return;
    }

    if (0 != (sigType & TCPIP_TCP_SIGNAL_ESTABLISHED))
    {
        TCP_SOCKET_INFO tcpSockInfo;

        if (NULL != pSockState->pParent)
        {
            pSockState->needsEncryption = pSockState->pParent->needsEncryption;
        }

        if (true == pSockState->needsEncryption)
        {
            if (ATCMD_SOCK_ENCRYPT_STATE_NONE == pSockState->encryptState)
            {
                pSockState->encryptState = ATCMD_SOCK_ENCRYPT_STATE_STARTING;
                pSockState->needsEncryption = false;
            }
            else
            {
                SYS_CONSOLE_PRINT("\n_tcpSocketSignalHandler():ATCMD_APP_STATUS_SOCKET_TLS_FAILED\n");
                _sockErrorAEC(pSockState->handle, ATCMD_APP_STATUS_SOCKET_TLS_FAILED);
                return;
            }
        }

        memset(&tcpSockInfo, 0, sizeof(tcpSockInfo));
        if(false == TCPIP_TCP_SocketInfoGet(pSockState->transHandle, &tcpSockInfo))
        {
            ATCMD_Printf("TCPIP_TCP_SocketInfoGet() returned false; pSockState->transHandle = 0x%x ; hTCP = 0x%x", pSockState->transHandle, hTCP);            
        }

        if (NULL != pSockState->pParent)
        {
            pSockState->handle             = _getSocketHandle();
            pSockState->remoteIPv4Addr.Val = tcpSockInfo.remoteIPaddress.v4Add.Val == 0x01000000u ? 0: tcpSockInfo.remoteIPaddress.v4Add.Val;
            pSockState->remotePort         = tcpSockInfo.remotePort;
        }
        else
        {
            pSockState->localPort = tcpSockInfo.localPort;
        }

        ATCMD_Printf("+SOCKIND:%d,", pSockState->handle);
        ATCMD_PrintIPv4Address(tcpSockInfo.localIPaddress.v4Add.Val);
        ATCMD_Printf(",%d,", pSockState->localPort);
        ATCMD_PrintIPv4Address(pSockState->remoteIPv4Addr.Val);
        ATCMD_Printf(",%d\r\n", pSockState->remotePort);

        if (ATCMD_SOCK_ENCRYPT_STATE_NONE == pSockState->encryptState)
        {
            pSockState->isConnected = true;
        }
    }

    if (0 != (sigType & TCPIP_TCP_SIGNAL_RX_DATA))
    {
        uint16_t numBytes;

        numBytes = _sockTCPReadReady(pSockState);

        if ((numBytes > 0) && (numBytes > pSockState->pendingDataLength))
        {
            ATCMD_Printf("+SOCKRXT:%d,%d\r\n", pSockState->handle, numBytes);
        }

        pSockState->pendingDataLength = numBytes;
    }

    if (0 != (sigType & (TCPIP_TCP_SIGNAL_TX_RST|TCPIP_TCP_SIGNAL_RX_FIN|TCPIP_TCP_SIGNAL_RX_RST|TCPIP_TCP_SIGNAL_KEEP_ALIVE_TMO)))
    {
        ATCMD_Printf("+SOCKCL:%d\r\n", pSockState->handle);

        if (true == TCPIP_TCP_Disconnect(pSockState->transHandle))
        {
            if (NULL != pSockState->pWolfSSLSession)
            {
                ATCMD_TLS_FreeSession(pSockState->tlsConfIdx, pSockState->pWolfSSLSession);
            }

            pSockState->encryptState = ATCMD_SOCK_ENCRYPT_STATE_NONE;
        }

        pSockState->remoteIPv4Addr.Val = 0;
        pSockState->remotePort         = 0;
        pSockState->pendingDataLength  = 0;
    }
}

static void _udpSocketSignalHandler(UDP_SOCKET hUDP, TCPIP_NET_HANDLE hNet, TCPIP_UDP_SIGNAL_TYPE sigType, const void* param)
{
    ATCMD_SOCK_STATE *const pSockState = _findUDPSocketByTransHandle(hUDP);

//    ATCMD_Printf("USH(%d): (0x%08x) 0x%04x\r\n", hUDP, param, sigType);

    if (NULL == pSockState)
    {
        return;
    }

    if ((0 != (sigType & TCPIP_UDP_SIGNAL_RX_DATA)) && (0 == pSockState->pendingDataLength))
    {
        UDP_SOCKET_INFO udpSockInfo;

        pSockState->pendingDataLength = TCPIP_UDP_GetIsReady(pSockState->transHandle);

        TCPIP_UDP_SocketInfoGet(pSockState->transHandle, &udpSockInfo);

        ATCMD_Printf("+SOCKRXU:%d,", pSockState->handle);
        ATCMD_PrintIPv4Address(udpSockInfo.sourceIPaddress.v4Add.Val);
        ATCMD_Printf(",%d,%d\r\n", udpSockInfo.remotePort, pSockState->pendingDataLength);
    }
}

/*******************************************************************************
* Command init functions
*******************************************************************************/
static ATCMD_STATUS _SOCKInit(const AT_CMD_TYPE_DESC* pCmdTypeDesc)
{
    memset(socketState, 0, sizeof(socketState));
    memset(&sockRdState, 0, sizeof(sockRdState));

    pSOCKWRBinarySock = NULL;
    nextSocketHandle  = 0;

    wolfSSL_Init();

    return ATCMD_STATUS_OK;
}

/*******************************************************************************
* Command execute functions
*******************************************************************************/
static ATCMD_STATUS _SOCKOExecute(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const int numParams, ATCMD_PARAM *pParamList)
{
    ATCMD_SOCK_STATE *pSockState = NULL;

    /* Validate the parameters against the defined descriptors to ensure types match */

    if (1 == numParams)
    {
        if (false == ATCMD_ParamValidateTypes(pCmdTypeDesc, 0, numParams, pParamList))
        {
            return ATCMD_STATUS_INVALID_PARAMETER;
        }
    }
    else
    {
        return ATCMD_STATUS_INCORRECT_NUM_PARAMS;
    }

    /* Find empty socket structure to use for new socket */

    pSockState = _findSocketByHandle(-1);

    if (NULL == pSockState)
    {
        return ATCMD_APP_STATUS_NO_FREE_SOCKETS;
    }

    if (ATCMD_SOCK_PROTO_UDP == pParamList[0].value.i)
    {
        /* UDP */
    }
    else if (ATCMD_SOCK_PROTO_TCP == pParamList[0].value.i)
    {
        /* TCP */
    }
    else
    {
        return ATCMD_APP_STATUS_INVALID_SOCKET_PROTOCOL;
    }

    pSockState->isConnected     = false;
    pSockState->handle          = _getSocketHandle();
    pSockState->protocol        = pParamList[0].value.i;
    pSockState->pParent         = NULL;
    pSockState->inUse           = true;
    pSockState->needsEncryption = false;
    pSockState->transHandle     = -1;

    ATCMD_Printf("+SOCKO:%d\r\n", pSockState->handle);

    return ATCMD_STATUS_OK;
}

static ATCMD_STATUS _SOCKBLExecute(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const int numParams, ATCMD_PARAM *pParamList)
{
    ATCMD_SOCK_STATE *pSockState = NULL;

    /* Validate the parameters against the defined descriptors to ensure types match */

    if (2 == numParams)
    {
        if (false == ATCMD_ParamValidateTypes(pCmdTypeDesc, 0, numParams, pParamList))
        {
            return ATCMD_STATUS_INVALID_PARAMETER;
        }
    }
    else
    {
        return ATCMD_STATUS_INCORRECT_NUM_PARAMS;
    }

    /* Find the socket structure associated with this socket ID */

    pSockState = _findSocketByHandle(pParamList[0].value.i);

    if (NULL == pSockState)
    {
        return ATCMD_APP_STATUS_SOCKET_ID_NOT_FOUND;
    }

    if (ATCMD_SOCK_PROTO_UDP == pSockState->protocol)
    {
        /* UDP socket */

        pSockState->localPort  = pParamList[1].value.i;

        pSockState->transHandle = TCPIP_UDP_ServerOpen(IP_ADDRESS_TYPE_IPV4, pSockState->localPort, NULL);

        if (-1 == pSockState->transHandle)
        {
            _closeSocket(pSockState);

            return ATCMD_APP_STATUS_SOCKET_BIND_FAILED;
        }

        pSockState->sigHandler = TCPIP_UDP_SignalHandlerRegister(pSockState->transHandle, 0xffff /*TCPIP_UDP_SIGNAL_RX_DATA*/, _udpSocketSignalHandler, pSockState);
    }
    else if (ATCMD_SOCK_PROTO_TCP == pSockState->protocol)
    {
        int i;

        /* TCP socket */

        pSockState->localPort  = pParamList[1].value.i;

        for (i=0; i<AT_CMD_SOCK_MAX_CLIENTS; i++)
        {
            ATCMD_SOCK_STATE *pSrvSockState;

            pSrvSockState = _findSocketByHandle(-1);

            if (NULL != pSrvSockState)
            {
                pSrvSockState->transHandle = TCPIP_TCP_ServerOpen(IP_ADDRESS_TYPE_IPV4, pSockState->localPort, NULL);

                if (-1 == pSrvSockState->transHandle)
                {
                    _closeSocket(pSrvSockState);

                    return ATCMD_APP_STATUS_SOCKET_BIND_FAILED;
                }
                
                pSockState->childTransHandle[i] = pSrvSockState->transHandle;
                
                pSrvSockState->sigHandler       = TCPIP_TCP_SignalHandlerRegister(pSrvSockState->transHandle, 0xffff /*TCPIP_TCP_SIGNAL_ESTABLISHED | TCPIP_TCP_SIGNAL_RX_RST | TCPIP_TCP_SIGNAL_RX_DATA*/, _tcpSocketSignalHandler, pSrvSockState);

                pSrvSockState->localPort        = pSockState->localPort;
                pSrvSockState->protocol         = ATCMD_SOCK_PROTO_TCP;
                pSrvSockState->inUse            = true;
                pSrvSockState->pParent          = pSockState;
                pSrvSockState->needsEncryption  = pSockState->needsEncryption;
            }
            else
            {
                // TODO
            }
        }
    }
    else
    {
        return ATCMD_APP_STATUS_INVALID_SOCKET_PROTOCOL;
    }

    return ATCMD_STATUS_OK;
}

static ATCMD_STATUS _SOCKBRExecute(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const int numParams, ATCMD_PARAM *pParamList)
{
    ATCMD_SOCK_STATE *pSockState = NULL;

    /* Validate the parameters against the defined descriptors to ensure types match */

    if (3 == numParams)
    {
        if (false == ATCMD_ParamValidateTypes(pCmdTypeDesc, 0, numParams, pParamList))
        {
            return ATCMD_STATUS_INVALID_PARAMETER;
        }
    }
    else
    {
        return ATCMD_STATUS_INCORRECT_NUM_PARAMS;
    }

    /* Find the socket structure associated with this socket ID */

    pSockState = _findSocketByHandle(pParamList[0].value.i);

    if (NULL == pSockState)
    {
        return ATCMD_APP_STATUS_SOCKET_ID_NOT_FOUND;
    }

    TCPIP_Helper_StringToIPAddress((char*)pParamList[1].value.p, &pSockState->remoteIPv4Addr);

    pSockState->localPort  = 0;
    pSockState->remotePort = pParamList[2].value.i;

    if (ATCMD_SOCK_PROTO_UDP == pSockState->protocol)
    {
        /* UDP socket, bind to remote address/port, this shouldn't be necessary if using sendto */

        pSockState->transHandle = TCPIP_UDP_ClientOpen(IP_ADDRESS_TYPE_IPV4, pSockState->remotePort, (IP_MULTI_ADDRESS*)&pSockState->remoteIPv4Addr);

        if (-1 == pSockState->transHandle)
        {
            _closeSocket(pSockState);

            return ATCMD_APP_STATUS_SOCKET_BIND_FAILED;
        }

        pSockState->sigHandler = TCPIP_UDP_SignalHandlerRegister(pSockState->transHandle, 0xffff /*TCPIP_UDP_SIGNAL_RX_DATA*/, _udpSocketSignalHandler, pSockState);
    }
    else if (ATCMD_SOCK_PROTO_TCP == pSockState->protocol)
    {
        /* TCP socket, connect to remote address/port */

        if (true == pSockState->needsEncryption)
        {
            pSockState->encryptState = ATCMD_SOCK_ENCRYPT_STATE_STARTING;
        }

        pSockState->transHandle = TCPIP_TCP_ClientOpen(IP_ADDRESS_TYPE_IPV4, pSockState->remotePort, (IP_MULTI_ADDRESS*)&pSockState->remoteIPv4Addr);

        if (-1 == pSockState->transHandle)
        {
            _closeSocket(pSockState);

            return ATCMD_APP_STATUS_SOCKET_BIND_FAILED;
        }

        pSockState->sigHandler = TCPIP_TCP_SignalHandlerRegister(pSockState->transHandle, 0xffff /*TCPIP_TCP_SIGNAL_ESTABLISHED | TCPIP_TCP_SIGNAL_RX_RST*/, _tcpSocketSignalHandler, pSockState);

        pSockState->needsEncryption = false;
    }
    else
    {
        return ATCMD_APP_STATUS_INVALID_SOCKET_PROTOCOL;
    }

    pSockState->lastTimeMs = ATCMD_PlatformGetSysTimeMs();

    return ATCMD_STATUS_OK;
}

static ATCMD_STATUS _SOCKBMExecute(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const int numParams, ATCMD_PARAM *pParamList)
{
    ATCMD_SOCK_STATE *pSockState = NULL;

    if (3 == numParams)
    {
        if (false == ATCMD_ParamValidateTypes(pCmdTypeDesc, 0, numParams, pParamList))
        {
            return ATCMD_STATUS_INVALID_PARAMETER;
        }
    }
    else
    {
        return ATCMD_STATUS_INCORRECT_NUM_PARAMS;
    }

    /* Find the socket structure associated with this socket ID */

    pSockState = _findSocketByHandle(pParamList[0].value.i);

    if (NULL == pSockState)
    {
        return ATCMD_APP_STATUS_SOCKET_ID_NOT_FOUND;
    }

    TCPIP_Helper_StringToIPAddress((char*)pParamList[1].value.p, &pSockState->remoteIPv4Addr);

    pSockState->localPort  = pParamList[2].value.i;
    pSockState->remotePort = pParamList[2].value.i;

    if (ATCMD_SOCK_PROTO_UDP == pSockState->protocol)
    {
        UDP_OPTION_MULTICAST_DATA sockOpt;

        pSockState->transHandle = TCPIP_UDP_ClientOpen(IP_ADDRESS_TYPE_IPV4, pSockState->remotePort, (IP_MULTI_ADDRESS*)&pSockState->remoteIPv4Addr);

        if (-1 == pSockState->transHandle)
        {
            _closeSocket(pSockState);

            return ATCMD_APP_STATUS_SOCKET_BIND_FAILED;
        }

        pSockState->sigHandler = TCPIP_UDP_SignalHandlerRegister(pSockState->transHandle, 0xffff /*TCPIP_UDP_SIGNAL_RX_DATA*/, _udpSocketSignalHandler, pSockState);

        sockOpt.flagsMask = 0xFF;
        sockOpt.flagsValue = UDP_MCAST_FLAG_DEFAULT; 

        if (false == TCPIP_UDP_Bind(pSockState->transHandle, IP_ADDRESS_TYPE_IPV4, pSockState->localPort, NULL))
        {
            return ATCMD_APP_STATUS_SOCKET_BIND_FAILED;
        }

        if (false == TCPIP_UDP_OptionsSet((UDP_SOCKET)pSockState->transHandle, UDP_OPTION_MULTICAST, &sockOpt))
        {
            return ATCMD_APP_STATUS_MULTICAST_ERROR;
        }

        if (TCPIP_IGMP_OK != TCPIP_IGMP_Subscribe((UDP_SOCKET)pSockState->transHandle, atCmdAppContext.netHandle, pSockState->remoteIPv4Addr, TCPIP_IGMP_FILTER_EXCLUDE, NULL, NULL))
        {
            return ATCMD_APP_STATUS_MULTICAST_ERROR;
        }
    }
    else
    {
        return ATCMD_APP_STATUS_INVALID_SOCKET_PROTOCOL;
    }

    return ATCMD_STATUS_OK;
}

static ATCMD_STATUS _SOCKTLSExecute(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const int numParams, ATCMD_PARAM *pParamList)
{
    ATCMD_SOCK_STATE *pSockState = NULL;

    /* Validate the parameters against the defined descriptors to ensure types match */

    if (2 == numParams)
    {
        if (false == ATCMD_ParamValidateTypes(pCmdTypeDesc, 0, numParams, pParamList))
        {
            return ATCMD_STATUS_INVALID_PARAMETER;
        }
    }
    else
    {
        return ATCMD_STATUS_INCORRECT_NUM_PARAMS;
    }

    if ((pParamList[1].value.i < 1) || (pParamList[1].value.i > AT_CMD_TLS_NUM_CONFS))
    {
        return ATCMD_STATUS_INVALID_PARAMETER;
    }

    /* Find the socket structure associated with this socket ID */

    pSockState = _findSocketByHandle(pParamList[0].value.i);

    if (NULL == pSockState)
    {
        return ATCMD_APP_STATUS_SOCKET_ID_NOT_FOUND;
    }

    if (ATCMD_SOCK_PROTO_TCP != pSockState->protocol)
    {
        return ATCMD_APP_STATUS_INVALID_SOCKET_PROTOCOL;
    }

    pSockState->tlsConfIdx = pParamList[1].value.i;

    if (ATCMD_SOCK_ENCRYPT_STATE_NONE == pSockState->encryptState)
    {
        if (true == pSockState->isConnected)
        {
            pSockState->encryptState = ATCMD_SOCK_ENCRYPT_STATE_STARTING;

            pSockState->isConnected = false;
        }
        else
        {
            pSockState->needsEncryption = true;
        }
    }
    else if (ATCMD_SOCK_ENCRYPT_STATE_DONE == pSockState->encryptState)
    {
        ATCMD_Printf("+SOCKTLS:%d\r\n", pSockState->handle);
    }

    return ATCMD_STATUS_OK;
}

static ATCMD_STATUS _SOCKWRExecute(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const int numParams, ATCMD_PARAM *pParamList)
{
    ATCMD_SOCK_STATE *pSockState = NULL;

    /* Validate the parameters against the defined descriptors to ensure types match */

    if (3 == numParams)
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

        if (0 != pParamList[1].value.i)
        {
            g_binModeNumBytes = pParamList[1].value.i;
        }
    }
    else
    {
        return ATCMD_STATUS_INCORRECT_NUM_PARAMS;
    }

    /* Find the socket structure associated with this socket ID */

    pSockState = _findSocketByHandle(pParamList[0].value.i);

    if (NULL == pSockState)
    {
        return ATCMD_APP_STATUS_SOCKET_ID_NOT_FOUND;
    }

    if ((0 == pSockState->remoteIPv4Addr.Val) || (0 == pSockState->remotePort))
    {
        return ATCMD_APP_STATUS_SOCKET_ID_NOT_FOUND;        
    }
    
    if (2 == numParams)
    {
        /* Binary mode */

        ATCMD_Print("\r\n", 2);
        ATCMD_EnterBinaryMode(&_socketWriteBinaryDataHandler);
        pSOCKWRBinarySock = pSockState;
    }
    else if (pParamList[1].value.i == pParamList[2].length)
    {
        /* ASCII or hex string mode */

        if (ATCMD_SOCK_PROTO_UDP == pSockState->protocol)
        {
            /* UDP socket, ensure remote address/port is set before trying to sendto */

            if ((0 == pSockState->remoteIPv4Addr.Val) || (0 == pSockState->remotePort))
            {
                return ATCMD_APP_STATUS_SOCKET_REMOTE_NOT_SET;
            }

            if (TCPIP_UDP_PutIsReady(pSockState->transHandle) < pParamList[2].length)
            {
                return ATCMD_APP_STATUS_SOCKET_SEND_FAILED;
            }

            if (0 == TCPIP_UDP_ArrayPut(pSockState->transHandle, pParamList[2].value.p, pParamList[2].length))
            {
                return ATCMD_APP_STATUS_SOCKET_SEND_FAILED;
            }

            TCPIP_UDP_Flush(pSockState->transHandle);
        }
        else if (ATCMD_SOCK_PROTO_TCP == pSockState->protocol)
        {
            /* TCP socket */

            if (false == _sockTCPWriteReady(pSockState, pParamList[2].length))
            {
                return ATCMD_APP_STATUS_SOCKET_SEND_FAILED;
            }

            if (ATCMD_SOCK_ENCRYPT_STATE_NONE == pSockState->encryptState)
            {
                if (0 == TCPIP_TCP_ArrayPut(pSockState->transHandle, pParamList[2].value.p, pParamList[2].length))
                {
                    return ATCMD_APP_STATUS_SOCKET_SEND_FAILED;
                }
            }
            else if (ATCMD_SOCK_ENCRYPT_STATE_DONE == pSockState->encryptState)
            {
                if (wolfSSL_write(pSockState->pWolfSSLSession, pParamList[2].value.p, pParamList[2].length) < 0)
                {
                    return ATCMD_APP_STATUS_SOCKET_SEND_FAILED;
                }
            }
            else
            {
                return ATCMD_APP_STATUS_SOCKET_SEND_FAILED;
            }
        }
        else
        {
            return ATCMD_APP_STATUS_INVALID_SOCKET_PROTOCOL;
        }
    }
    else
    {
        return ATCMD_APP_STATUS_LENGTH_MISMATCH;
    }

    return ATCMD_STATUS_OK;
}

static ATCMD_STATUS _SOCKWRTOExecute(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const int numParams, ATCMD_PARAM *pParamList)
{
    ATCMD_SOCK_STATE *pSockState = NULL;

    /* Validate the parameters against the defined descriptors to ensure types match */

    if (5 == numParams)
    {
        if (false == ATCMD_ParamValidateTypes(pCmdTypeDesc, 0, numParams, pParamList))
        {
            return ATCMD_STATUS_INVALID_PARAMETER;
        }
    }
    else if (4 == numParams)
    {
        if (false == ATCMD_ParamValidateTypes(pCmdTypeDesc, 1, numParams, pParamList))
        {
            return ATCMD_STATUS_INVALID_PARAMETER;
        }

        if (0 != pParamList[3].value.i)
        {
            g_binModeNumBytes = pParamList[3].value.i;
        }
    }
    else
    {
        return ATCMD_STATUS_INCORRECT_NUM_PARAMS;
    }

    /* Find the socket structure associated with this socket ID */

    pSockState = _findSocketByHandle(pParamList[0].value.i);

    if (NULL == pSockState)
    {
        return ATCMD_APP_STATUS_SOCKET_ID_NOT_FOUND;
    }

    if (4 == numParams)
    {
        /* Binary mode */

        ATCMD_Print("\r\n", 2);
        ATCMD_EnterBinaryMode(&_socketWriteBinaryDataHandler);
        pSOCKWRBinarySock = pSockState;
        if ((0 == pSockState->remoteIPv4Addr.Val) || (0 == pSockState->remotePort))
        {
            IPV4_ADDR remoteAddr;
            TCPIP_Helper_StringToIPAddress((char*)pParamList[1].value.p, &remoteAddr);
            TCPIP_UDP_DestinationIPAddressSet(pSockState->transHandle, IP_ADDRESS_TYPE_IPV4, (IP_MULTI_ADDRESS*)&remoteAddr);
            TCPIP_UDP_DestinationPortSet(pSockState->transHandle, pParamList[2].value.i);
            pSockState->remoteIPv4Addr.Val = remoteAddr.Val;
            pSockState->remotePort         = pParamList[2].value.i;
        }
    }
    else if (pParamList[3].value.i == pParamList[4].length)
    {
        /* ASCII or hex string mode */

        if (ATCMD_SOCK_PROTO_UDP == pSockState->protocol)
        {
            /* UDP socket, use supplied remote address/port instead of stored one */

            UDP_SOCKET_INFO udpSockInfo;
            IPV4_ADDR remoteAddr;

            TCPIP_UDP_SocketInfoGet(pSockState->transHandle, &udpSockInfo);

            TCPIP_Helper_StringToIPAddress((char*)pParamList[1].value.p, &remoteAddr);
            TCPIP_UDP_DestinationIPAddressSet(pSockState->transHandle, IP_ADDRESS_TYPE_IPV4, (IP_MULTI_ADDRESS*)&remoteAddr);
            TCPIP_UDP_DestinationPortSet(pSockState->transHandle, pParamList[2].value.i);

            if (TCPIP_UDP_PutIsReady(pSockState->transHandle) < pParamList[4].length)
            {
                SYS_CONSOLE_PRINT("\nLength Expected = %d; Available = %d\n", pParamList[4].length, TCPIP_UDP_PutIsReady(pSockState->transHandle));
                return ATCMD_APP_STATUS_SOCKET_SEND_FAILED;
            }

            if (0 == TCPIP_UDP_ArrayPut(pSockState->transHandle, pParamList[4].value.p, pParamList[4].length))
            {
                SYS_CONSOLE_PRINT("\nTCPIP_UDP_ArrayPut() : Failed\n");
                return ATCMD_APP_STATUS_SOCKET_SEND_FAILED;
            }

            TCPIP_UDP_Flush(pSockState->transHandle);

            if ((0 == pSockState->remoteIPv4Addr.Val) || (0 == pSockState->remotePort))
            {
                pSockState->remoteIPv4Addr.Val = remoteAddr.Val;
                pSockState->remotePort         = pParamList[2].value.i;
            }
        }
        else
        {
            return ATCMD_APP_STATUS_INVALID_SOCKET_PROTOCOL;
        }
    }
    else
    {
        return ATCMD_APP_STATUS_LENGTH_MISMATCH;
    }

    return ATCMD_STATUS_OK;
}

static ATCMD_STATUS _SOCKRDExecute(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const int numParams, ATCMD_PARAM *pParamList)
{
    ATCMD_SOCK_STATE *pSockState = NULL;

    if (3 == numParams)
    {
        if (false == ATCMD_ParamValidateTypes(pCmdTypeDesc, 0, numParams, pParamList))
        {
            return ATCMD_STATUS_INVALID_PARAMETER;
        }
    }
    else
    {
        return ATCMD_STATUS_INCORRECT_NUM_PARAMS;
    }

    /* Find the socket structure associated with this socket ID */

    pSockState = _findSocketByHandle(pParamList[0].value.i);

    if (NULL == pSockState)
    {
        return ATCMD_APP_STATUS_SOCKET_ID_NOT_FOUND;
    }

    if (pParamList[2].value.i > AT_CMD_SOCK_RX_BUFFER_SZ)
    {
        return ATCMD_STATUS_INVALID_PARAMETER;
    }

    if (pParamList[2].value.i > 0)
    {
        sockRdState.numBytesBuffered = 0;
        sockRdState.pBufPtr = sockRdState.buffer;

        if (1 == pParamList[1].value.i)
        {
            /* ASCII or hex string */

            sockRdState.pSockState          = pSockState;
            sockRdState.isBinary            = false;
            sockRdState.numBytesRequested   = pParamList[2].value.i;

            return ATCMD_STATUS_PENDING;
        }
        else if (2 == pParamList[1].value.i)
        {
            /* Binary */

            sockRdState.pSockState          = pSockState;
            sockRdState.isBinary            = true;
            sockRdState.numBytesRequested   = pParamList[2].value.i;

            return ATCMD_STATUS_PENDING;
        }
        else
        {
            return ATCMD_APP_STATUS_INVALID_SOCKET_PROTOCOL;
        }
    }

    return ATCMD_STATUS_OK;
}

static ATCMD_STATUS _SOCKCLExecute(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const int numParams, ATCMD_PARAM *pParamList)
{
    ATCMD_SOCK_STATE *pSockState = NULL;

    if (1 == numParams)
    {
        if (false == ATCMD_ParamValidateTypes(pCmdTypeDesc, 0, numParams, pParamList))
        {
            return ATCMD_STATUS_INVALID_PARAMETER;
        }
    }
    else
    {
        return ATCMD_STATUS_INCORRECT_NUM_PARAMS;
    }

    /* Find the socket structure associated with this socket ID */

    pSockState = _findSocketByHandle(pParamList[0].value.i);

    if (NULL == pSockState)
    {
        return ATCMD_APP_STATUS_SOCKET_ID_NOT_FOUND;
    }

    if(pSockState->transHandle == -1)
    {
        int i = 0; 
        ATCMD_SOCK_STATE *pTmpSockState = NULL;
        
        for(i = 0; i < AT_CMD_SOCK_MAX_CLIENTS; i++)
        {
            pTmpSockState = _findSocketByTransHandle(pSockState->childTransHandle[i]);

            if (NULL == pTmpSockState)
            {
                continue;
            }

            if (ATCMD_SOCK_PROTO_UDP == pTmpSockState->protocol)
            {
                continue;
            }

            if (NULL != pTmpSockState->pWolfSSLSession)
            {
                ATCMD_TLS_FreeSession(pTmpSockState->tlsConfIdx, pTmpSockState->pWolfSSLSession);
            }

            pTmpSockState->encryptState = ATCMD_SOCK_ENCRYPT_STATE_NONE;
            TCPIP_TCP_Close(pTmpSockState->transHandle);

            pTmpSockState->transHandle    = -1;
            pTmpSockState->inUse          = false;
            pTmpSockState->remoteIPv4Addr.Val = 0;
            pTmpSockState->remotePort         = 0;
            pTmpSockState->pendingDataLength  = 0;
        }
    }
    
    _closeSocket(pSockState);

    return ATCMD_STATUS_OK;
}

static ATCMD_STATUS _SOCKLSTExecute(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const int numParams, ATCMD_PARAM *pParamList)
{
    int i;

    if (1 == numParams)
    {
        if (false == ATCMD_ParamValidateTypes(pCmdTypeDesc, 1, numParams, pParamList))
        {
            return ATCMD_STATUS_INVALID_PARAMETER;
        }
    }
    else if (numParams > 1)
    {
        return ATCMD_STATUS_INCORRECT_NUM_PARAMS;
    }

    for (i=0; i<AT_CMD_SOCK_MAX_NUM; i++)
    {
        if ((true == socketState[i].inUse) && (0 != socketState[i].handle))
        {
            if ((0 == numParams) || (socketState[i].handle == pParamList[0].value.i))
            {
                if (0 != socketState[i].remoteIPv4Addr.Val)
                {
                    ATCMD_Printf("+SOCKLST:%d,%d,", socketState[i].handle, socketState[i].protocol);

                    ATCMD_PrintIPv4Address(socketState[i].remoteIPv4Addr.Val);
                }
                else
                {
                    if(socketState[i].pParent)
                        continue;
                    
                    ATCMD_Printf("+SOCKLST:%d,%d,", socketState[i].handle, socketState[i].protocol);

                    ATCMD_Print("\"\"", 2);
                }

                ATCMD_Printf(",%d,%d\r\n", socketState[i].remotePort, socketState[i].localPort);
            }
        }
    }

    return ATCMD_STATUS_OK;
}

/*******************************************************************************
* Command update functions
*******************************************************************************/
static ATCMD_STATUS _SOCKUpdate(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const AT_CMD_TYPE_DESC* pCurrentCmdTypeDesc)
{
    int i;
    ATCMD_STATUS retStatus = ATCMD_STATUS_OK;
    uint32_t currentTimeMs = ATCMD_PlatformGetSysTimeMs();

    if (pCurrentCmdTypeDesc == pCmdTypeDesc)
    {
        if ((pCmdTypeDesc == &atCmdTypeDescSOCKRD) && (NULL != sockRdState.pSockState))
        {
            uint16_t numBytes;

            if ((0 == sockRdState.numBytesBuffered) && (sockRdState.numBytesRequested > 0))
            {
                if (ATCMD_SOCK_PROTO_UDP == sockRdState.pSockState->protocol)
                {
                    numBytes = TCPIP_UDP_GetIsReady(sockRdState.pSockState->transHandle);
                }
                else
                {
                    numBytes = _sockTCPReadReady(sockRdState.pSockState);
                }

                if (numBytes > sockRdState.numBytesRequested)
                {
                    numBytes = sockRdState.numBytesRequested;
                }

                if (ATCMD_SOCK_ENCRYPT_STATE_NONE == sockRdState.pSockState->encryptState)
                {
                    if (ATCMD_SOCK_PROTO_UDP == sockRdState.pSockState->protocol)
                    {
                        sockRdState.numBytesBuffered = TCPIP_UDP_ArrayGet(sockRdState.pSockState->transHandle, sockRdState.pBufPtr, numBytes);
                    }
                    else
                    {
                        sockRdState.numBytesBuffered = TCPIP_TCP_ArrayGet(sockRdState.pSockState->transHandle, sockRdState.pBufPtr, numBytes);
                    }
                }
                else if (ATCMD_SOCK_ENCRYPT_STATE_DONE == sockRdState.pSockState->encryptState)
                {
                    int readRes = wolfSSL_read(sockRdState.pSockState->pWolfSSLSession, sockRdState.pBufPtr, numBytes);

                    if (readRes > 0)
                    {
                        sockRdState.numBytesBuffered = readRes;
                    }
                }

                sockRdState.pSockState->pendingDataLength -= sockRdState.numBytesBuffered;
            }

            if (sockRdState.numBytesBuffered > 0)
            {
                numBytes = sockRdState.numBytesBuffered;

                if (numBytes > 128)
                {
                    numBytes = 128;
                }

                if (true == sockRdState.isBinary)
                {
                    if (true != ATCMD_ModeIsBinary())
                    {
                        ATCMD_Printf("+SOCKRD:%d,%d,", sockRdState.pSockState->handle, sockRdState.numBytesBuffered);
                        ATCMD_Print("\r\n", 2);
                        ATCMD_EnterBinaryMode(NULL);
                    }

                    ATCMD_Print((char*)sockRdState.pBufPtr, numBytes);
                }
                else
                {
                    ATCMD_Printf("+SOCKRD:%d,%d,", sockRdState.pSockState->handle, numBytes);
                    ATCMD_PrintStringSafe((char*)sockRdState.pBufPtr, numBytes);
                    ATCMD_Print("\r\n", 2);
                }

                sockRdState.numBytesBuffered -= numBytes;
                sockRdState.pBufPtr          += numBytes;

                if (0 == sockRdState.numBytesBuffered)
                {
                    if (true == ATCMD_ModeIsBinary())
                    {
                        ATCMD_LeaveBinaryMode();
                    }

                    if (ATCMD_SOCK_PROTO_UDP == sockRdState.pSockState->protocol)
                    {
                        UDP_SOCKET_INFO udpSockInfo;

                        sockRdState.pSockState->pendingDataLength = TCPIP_UDP_GetIsReady(sockRdState.pSockState->transHandle);

                        if (sockRdState.pSockState->pendingDataLength > 0)
                        {
                            TCPIP_UDP_SocketInfoGet(sockRdState.pSockState->transHandle, &udpSockInfo);

                            ATCMD_Printf("+SOCKRXU:%d,", sockRdState.pSockState->handle);
                            ATCMD_PrintIPv4Address(udpSockInfo.sourceIPaddress.v4Add.Val);
                            ATCMD_Printf(",%d,%d\r\n", udpSockInfo.remotePort, sockRdState.pSockState->pendingDataLength);
                        }
                    }

                    sockRdState.pSockState = NULL;
                }
                else
                {
                    retStatus = ATCMD_STATUS_PENDING;
                }
            }
        }
    }

    for (i=0; i<AT_CMD_SOCK_MAX_NUM; i++)
    {
        ATCMD_SOCK_STATE *pSockState = &socketState[i];

        if (true == pSockState->inUse)
        {
            if (true == pSockState->isConnected)
            {
                continue;
            }

            if ((ATCMD_SOCK_PROTO_TCP == pSockState->protocol) && (0 != pSockState->remotePort))
            {
                switch (pSockState->encryptState)
                {
                    case ATCMD_SOCK_ENCRYPT_STATE_NONE:
                    {
                        if (-1 != pSockState->transHandle)
                        {
                            if ((currentTimeMs - pSockState->lastTimeMs) > ATCMD_SOCK_BIND_TIMEOUT_MS)
                            {
                                _sockErrorAEC(pSockState->handle, ATCMD_APP_STATUS_SOCKET_BIND_FAILED);
                                _closeSocket(pSockState);
                            }
                        }

                        break;
                    }

                    case ATCMD_SOCK_ENCRYPT_STATE_STARTING:
                    {
                        /* Wait for TCP connection to be established. */

                        if (!TCPIP_TCP_IsConnected(pSockState->transHandle))
                        {
                            break;
                        }

                        if (NULL == pSockState->pParent)
                        {
                            pSockState->pWolfSSLSession = ATCMD_TLS_AllocSession(pSockState->tlsConfIdx, &atCmdAppContext.tlsConf[0], true, pSockState->transHandle);
                        }
                        else
                        {
                            pSockState->pWolfSSLSession = ATCMD_TLS_AllocSession(pSockState->tlsConfIdx, &atCmdAppContext.tlsConf[1], false, pSockState->transHandle);
                        }

                        if (NULL == pSockState->pWolfSSLSession)
                        {
                            SYS_CONSOLE_PRINT("\n_SOCKUpdate():ATCMD_SOCK_ENCRYPT_STATE_STARTING case\n");
                            pSockState->encryptState = ATCMD_SOCK_ENCRYPT_STATE_FAILED;
                        }

                        /* Intentional fall through to the next state */
                    }

                    case ATCMD_SOCK_ENCRYPT_STATE_NEGOTIATING:
                    {
                        int result;

                        if (NULL == pSockState->pWolfSSLSession)
                        {
                            SYS_CONSOLE_PRINT("\n_SOCKUpdate():ATCMD_SOCK_ENCRYPT_STATE_NEGOTIATING case first\n");
                            pSockState->encryptState = ATCMD_SOCK_ENCRYPT_STATE_FAILED;
                            break;
                        }

                        if (0 == wolfSSL_is_server(pSockState->pWolfSSLSession))
                        {
                            result = wolfSSL_connect(pSockState->pWolfSSLSession);
                        }
                        else
                        {
                            result = wolfSSL_accept(pSockState->pWolfSSLSession);
                        }

                        if (SSL_SUCCESS == result)
                        {
                            pSockState->encryptState = ATCMD_SOCK_ENCRYPT_STATE_DONE;
                        }
                        else
                        {
                            int error = wolfSSL_get_error(pSockState->pWolfSSLSession, result);

                            if ((SSL_ERROR_WANT_READ == error) || (SSL_ERROR_WANT_WRITE == error))
                            {
                                pSockState->encryptState = ATCMD_SOCK_ENCRYPT_STATE_NEGOTIATING;
                            }
                            else
                            {
                                SYS_CONSOLE_PRINT("\n_SOCKUpdate():ATCMD_SOCK_ENCRYPT_STATE_NEGOTIATING case second - error = (%d) ; result = (%d)\n", error, result);
                                pSockState->encryptState = ATCMD_SOCK_ENCRYPT_STATE_FAILED;
                            }
                        }

                        break;
                    }

                    case ATCMD_SOCK_ENCRYPT_STATE_DONE:
                    {
                        pSockState->isConnected = true;

                        ATCMD_Printf("+SOCKTLS:%d\r\n", pSockState->handle);
                        break;
                    }

                    case ATCMD_SOCK_ENCRYPT_STATE_FAILED:
                    default:
                    {
                        SYS_CONSOLE_PRINT("\n_SOCKUpdate():ATCMD_APP_STATUS_SOCKET_TLS_FAILED\n");
                        _sockErrorAEC(pSockState->handle, ATCMD_APP_STATUS_SOCKET_TLS_FAILED);
                        _closeSocket(pSockState);
                        break;
                    }
                }
            }
        }
    }

    return retStatus;
}
