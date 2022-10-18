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
#include "at_cmds/at_cmd_xmodem.h"
#include "at_cmds/at_cmd_pkcs.h"
#include "cert_header.h"

/*******************************************************************************
* Command interface prototypes
*******************************************************************************/
static ATCMD_STATUS _LOADInit(const AT_CMD_TYPE_DESC* pCmdTypeDesc);
static ATCMD_STATUS _LOADExecute(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const int numParams, ATCMD_PARAM *pParamList);
static ATCMD_STATUS _LOADUpdate(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const AT_CMD_TYPE_DESC* pCurrentCmdTypeDesc);

extern bool NET_PRES_LoadVerifyDerBuffer(unsigned char* in, long sz);

/*******************************************************************************
* Command parameters
*******************************************************************************/
#if 0 
static const ATCMD_HELP_PARAM paramTransferProtocol =
    {"TSFRPROT", "Transfer protocol", ATCMD_PARAM_TYPE_CLASS_INTEGER,
        .numOpts = 4,
        {
            {"1", "X Modem + checksum"},
            {"2", "X Modem + CRC16"},
            {"3", "X Modem 1K"},
            {"4", "Y Modem"}
        }
    };

#else

static const ATCMD_HELP_PARAM paramTransferLength =
    {"LENGTH", "The length of the data to send (1 – 1500 bytes)", ATCMD_PARAM_TYPE_CLASS_INTEGER, 0};

static const ATCMD_HELP_PARAM paramCertname =
    {"CERTNAME", "The name of the certificate", ATCMD_PARAM_TYPE_CLASS_STRING, 0};
#endif

#if 0
static const ATCMD_HELP_PARAM paramPrikeyname =
    {"PRIKEYNAME", "The name of the private key", ATCMD_PARAM_TYPE_CLASS_STRING, 0};
#endif

/*******************************************************************************
* Command examples
*******************************************************************************/

/*******************************************************************************
* Command descriptors
*******************************************************************************/
const AT_CMD_TYPE_DESC atCmdTypeDescLOADCERT =
    {
        .pCmdName   = "+LOADCERT",
        .cmdInit    = _LOADInit,
        .cmdExecute = _LOADExecute,
        .cmdUpdate  = _LOADUpdate,
        .pSummary   = "This command downloads a certificate to the DCE",
        // .appVal     = ATAPP_VAL_LOAD_TYPE_CERT,
        .numVars    = 1,
        {
            {
                .numParams   = 2,
                .pParams     =
                {
                    &paramTransferLength, //&paramTransferProtocol,
                    &paramCertname
                },
                .numExamples = 0,
                .pExamples   =
                {
                    NULL
                }
            }
        }
    };

#if 0
const AT_CMD_TYPE_DESC atCmdTypeDescLOADPRIVKEY =
    {
        .pCmdName   = "+LOADPRIVKEY",
        .cmdInit    = NULL,
        .cmdExecute = _LOADExecute,
        .cmdUpdate  = NULL,
        .pSummary   = "This command downloads a private key to the DCE",
        .appVal     = ATAPP_VAL_LOAD_TYPE_PRIKEY,
        .numVars    = 1,
        {
            {
                .numParams   = 2,
                .pParams     =
                {
                    &paramTransferProtocol,
                    &paramPrikeyname
                },
                .numExamples = 0,
                .pExamples   =
                {
                    NULL
                }
            }
        }
    };
#endif

/*******************************************************************************
* External references
*******************************************************************************/
extern ATCMD_APP_CONTEXT atCmdAppContext;

/*******************************************************************************
* Local defines and types
*******************************************************************************/
typedef struct
{
    ATCMD_APP_VAL   type;
    uint_fast8_t    expPktNum;
    uint8_t         *pBuf;
    int             numBytes;
    int             maxNumBytes;
} ATLOAD_TSFR_CTX;

/*******************************************************************************
* Local data
*******************************************************************************/
static bool xmTsfrComplete;
static ATLOAD_TSFR_CTX tsfrCtx;

#if 0 
/*******************************************************************************
* Local functions
*******************************************************************************/
static void _XModemDataHandler(const uint_fast8_t pktNum, const uint8_t *pBuf, size_t numBufBytes)
{
    if (NULL != pBuf)
    {
        if (pktNum == tsfrCtx.expPktNum)
        {
            tsfrCtx.numBytes += numBufBytes;

            if (tsfrCtx.numBytes <= tsfrCtx.maxNumBytes)
            {
                memcpy(tsfrCtx.pBuf, pBuf, numBufBytes);

                tsfrCtx.pBuf += numBufBytes;
            }
            else
            {
                tsfrCtx.numBytes = 0;
                ATCMD_XModemStop();
            }

            tsfrCtx.expPktNum++;
        }
    }
    else
    {
        xmTsfrComplete = true;
    }
}
#else
static void _loadCertWriteBinaryDataHandler(const uint8_t *pBuf, size_t numBufBytes)
{
    memcpy(&atCmdAppContext.certFile[atCmdAppContext.certFileLength], pBuf,  numBufBytes);
    atCmdAppContext.certFileLength += numBufBytes;
    if(atCmdAppContext.certFileLength == tsfrCtx.numBytes)
    {
        ATCMD_LeaveBinaryMode();
        xmTsfrComplete = true;
    }
}

#endif

/*******************************************************************************
* Command init functions
*******************************************************************************/
static ATCMD_STATUS _LOADInit(const AT_CMD_TYPE_DESC* pCmdTypeDesc)
{
    atCmdAppContext.certFileLength = 0;

    return ATCMD_STATUS_OK;
}

/*******************************************************************************
* Command execute functions
*******************************************************************************/
static ATCMD_STATUS _LOADExecute(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const int numParams, ATCMD_PARAM *pParamList)
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

#if 0    
    /* Validate transfer protocol against builtin support */

    switch (pParamList[0].value.i)
    {
        case 1:
        {
#ifndef AT_CMD_CONF_XMODEM_SUPPORT_CSUM
            return ATCMD_APP_STATUS_TSFR_PROTOCOL_NOT_SUPPORTED;
#else
            break;
#endif
        }

        case 2:
        {
#ifndef AT_CMD_CONF_XMODEM_SUPPORT_CRC
            return ATCMD_APP_STATUS_TSFR_PROTOCOL_NOT_SUPPORTED;
#else
            break;
#endif
        }

        case 3:
        {
#if !defined(AT_CMD_CONF_XMODEM_SUPPORT_CRC) || !defined(AT_CMD_CONF_XMODEM_SUPPORT_1K)
            return ATCMD_APP_STATUS_TSFR_PROTOCOL_NOT_SUPPORTED;
#else
            break;
#endif
        }

        case 4:
        {
#if !defined(AT_CMD_CONF_XMODEM_SUPPORT_YMODEM_PROTOCOL)
            return ATCMD_APP_STATUS_TSFR_PROTOCOL_NOT_SUPPORTED;
#else
            break;
#endif
        }

        default:
        {
            break;
        }
    }

    switch (pCmdTypeDesc->appVal)
    {
        case ATAPP_VAL_LOAD_TYPE_CERT:
        {
            tsfrCtx.pBuf = atCmdAppContext.certFile;
            tsfrCtx.maxNumBytes = AT_CMD_CERT_FILE_MAX_SZ;
            break;
        }
#if 0
        case ATAPP_VAL_LOAD_TYPE_PRIKEY:
        {
            tsfrCtx.pBuf = atCmdAppContext.priKeyFile;
            tsfrCtx.maxNumBytes = AT_CMD_PRIKEY_FILE_MAX_SZ;
            break;
        }
#endif
        default:
        {
            return ATCMD_STATUS_ERROR;
        }
    }

    tsfrCtx.type      = pCmdTypeDesc->appVal;
    tsfrCtx.numBytes  = 0;
    tsfrCtx.expPktNum = 1;

    xmTsfrComplete = false;

    if (1 == pParamList[0].value.i)
    {
        ATCMD_XModemStart(false, &_XModemDataHandler);
    }
    else
    {
        ATCMD_XModemStart(true, &_XModemDataHandler);
    }
#endif

    atCmdAppContext.certFileLength = 0;
    tsfrCtx.numBytes = pParamList[0].value.i;
    memset(atCmdAppContext.certFile, 0,  sizeof(atCmdAppContext.certFile));

    xmTsfrComplete = false;
    ATCMD_EnterBinaryMode(&_loadCertWriteBinaryDataHandler);

    return ATCMD_STATUS_OK;
}

/*******************************************************************************
* Command update functions
*******************************************************************************/
static ATCMD_STATUS _LOADUpdate(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const AT_CMD_TYPE_DESC* pCurrentCmdTypeDesc)
{
    int derFileLength = 0;

    if (true == xmTsfrComplete)
    {
        xmTsfrComplete = false;

        switch (tsfrCtx.type)
        {
            case ATAPP_VAL_LOAD_TYPE_CERT:
            {
                if ('-' == atCmdAppContext.certFile[0])
                {
                    derFileLength = PKCS_PEMToDER((char*)atCmdAppContext.certFile, tsfrCtx.numBytes, "CERTIFICATE", atCmdAppContext.certFile);
                }
                else if (0x30 == atCmdAppContext.certFile[0])
                {
                    derFileLength = PKCS_DERLength(atCmdAppContext.certFile, tsfrCtx.numBytes);
                }

                ATCMD_Print("+LOADCERT:", 10);

                if (derFileLength > 0)
                {
                    atCmdAppContext.certFileLength = derFileLength;

                    NET_PRES_LoadVerifyDerBuffer(atCmdAppContext.certFile, derFileLength);
                
                    ATCMD_Print("0\r\n", 3);
                }
                else
                {
                    ATCMD_Print("1\r\n", 3);
                }

                tsfrCtx.numBytes = 0;
                
                break;
            }

#if 0
            case ATAPP_VAL_LOAD_TYPE_PRIKEY:
            {
                PKCS_PEM_TYPE pemType;
                bool derDecoded = false;

                if ('-' == atCmdAppContext.priKeyFile[0])
                {
                    /* File appears to be PEM formatted, check what type it is */

                    pemType = PKCS_PEMType((char*)atCmdAppContext.priKeyFile, tsfrCtx.numBytes);

                    if (PKCS_PEM_TYPE_PRIVATE_KEY == pemType)
                    {
                        /* Appears to be PKCS#8 private key format, convert to DER and decode */

                        derFileLength = PKCS_PEMToDER((char*)atCmdAppContext.priKeyFile, tsfrCtx.numBytes, "PRIVATE KEY", atCmdAppContext.priKeyFile);

                        if (derFileLength > 0)
                        {
                            derDecoded = PKCS8_ParsePrivateKeyDER(atCmdAppContext.priKeyFile, derFileLength, &atCmdAppContext.priKey);
                        }
                    }
                    else if (PKCS_PEM_TYPE_RSA_PRIVATE_KEY == pemType)
                    {
                        /* Appears to be PKCS#1 private key format, convert to DER and decode */

                        derFileLength = PKCS_PEMToDER((char*)atCmdAppContext.priKeyFile, tsfrCtx.numBytes, "RSA PRIVATE KEY", atCmdAppContext.priKeyFile);

                        if (derFileLength > 0)
                        {
                            derDecoded = PKCS1_ParseRSAPrivateKeyDER(atCmdAppContext.priKeyFile, derFileLength, &atCmdAppContext.priKey);
                        }
                    }
                }
                else if (0x30 == atCmdAppContext.priKeyFile[0])
                {
                    /* File appears to be DER formatted, try to decode it */

                    derFileLength = PKCS_DERLength(atCmdAppContext.priKeyFile, tsfrCtx.numBytes);

                    derDecoded = PKCS1_ParseRSAPrivateKeyDER(atCmdAppContext.priKeyFile, derFileLength, &atCmdAppContext.priKey);
                }

                ATCMD_Print("+LOADPRIVKEY:", 13);

                if (true == derDecoded)
                {
                    atCmdAppContext.priKeyFileLength = derFileLength;
                    ATCMD_Print("0\r\n", 3);
                }
                else
                {
                    if (derFileLength > 0)
                    {
                        ATCMD_Print("2,\"DER decode\"\r\n", 16);
                    }
                    else
                    {
                        ATCMD_Print("1,\"PEM decode\"\r\n", 16);
                    }

                    return ATCMD_STATUS_ERROR;
                }

                break;
            }
#endif
            
            default:
            {
                return ATCMD_STATUS_ERROR;
            }
        }
    }
    else
    {
        if ((false == ATCMD_ModeIsBinary()) && (tsfrCtx.numBytes))
        {
            xmTsfrComplete = true;
        }
    }

    return ATCMD_STATUS_OK;
}
