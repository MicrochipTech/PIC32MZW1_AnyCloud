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
#include "atca_basic.h"
#include "tng/tng_atcacert_client.h"

#define AT_CMD_READCERT_BUFFER_SZ	1500

typedef struct
{
    uint8_t             buffer[AT_CMD_READCERT_BUFFER_SZ];
    size_t            	numBytesBuffered;
} ATCMD_READCERT_STATE;

/*******************************************************************************
* Command interface prototypes
*******************************************************************************/
static ATCMD_STATUS _READCERTExecute(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const int numParams, ATCMD_PARAM *pParamList);

/*******************************************************************************
* Command parameters
*******************************************************************************/

static const ATCMD_HELP_PARAM paramCertType =
    {"CERTTYPE", "Certificate Type", ATCMD_PARAM_TYPE_CLASS_INTEGER,
        .numOpts = 4,
        {
            {"1", "Device Certificate"},
            {"2", "Root Certificate"},
            {"3", "Host Certificate"},
            {"4", "Signer Certificate"}
        }
    };


/*******************************************************************************
* Command examples
*******************************************************************************/

/*******************************************************************************
* Command descriptors
*******************************************************************************/
const AT_CMD_TYPE_DESC atCmdTypeDescREADCERT =
    {
        .pCmdName   = "+READCERT",
        .cmdInit    = NULL,
        .cmdExecute = _READCERTExecute,
        .cmdUpdate  = NULL,
        .pSummary   = "This command uploads the device certificate from the DCE",
        .appVal     = 0,
        .numVars    = 1,
        {
            {
                .numParams   = 1,
                .pParams     =
                {
                    &paramCertType
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
static int ATCMD_Read_DeviceCertPem(uint8_t *deviceCertPem, size_t *pDeviceCertPemSize);
static int ATCMD_Read_RootCert(uint8_t *rootCertPem, size_t *prootCertSize);
static int ATCMD_Read_SignerCert(uint8_t *signerCert, size_t *pSignerCertSize);

/*******************************************************************************
* Local defines and types
*******************************************************************************/
static ATCMD_READCERT_STATE	*readCertState;

/*******************************************************************************
* Local data
*******************************************************************************/

/*******************************************************************************
* Command init functions
*******************************************************************************/

/*******************************************************************************
* Command execute functions
*******************************************************************************/
static ATCMD_STATUS _READCERTExecute(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const int numParams, ATCMD_PARAM *pParamList)
{
    int maxOutputPrintBytes = (AT_CMD_CONF_PRINTF_OUT_BUF_SIZE/2) - 1;

    if (1 != numParams)
    {
        return ATCMD_STATUS_INCORRECT_NUM_PARAMS;
    }

    readCertState = malloc(sizeof(ATCMD_READCERT_STATE));
    if(readCertState == NULL)
    {
        return ATCMD_STATUS_ERROR;
    }
    
    switch (pParamList[0].value.i)
    {
        case 1:
        {
            readCertState->numBytesBuffered = AT_CMD_READCERT_BUFFER_SZ;
            memset(readCertState->buffer, 0, AT_CMD_READCERT_BUFFER_SZ);
            ATCMD_Read_DeviceCertPem(readCertState->buffer, &readCertState->numBytesBuffered);

            ATCMD_Printf("+READCERT:1, %d,", readCertState->numBytesBuffered);
            if(readCertState->numBytesBuffered < maxOutputPrintBytes)
            {
                ATCMD_PrintStringSafe((char*)&readCertState->buffer, readCertState->numBytesBuffered);
            }
            else
            {
                ATCMD_PrintStringSafeWithDelimiterInfo((char*)&readCertState->buffer, maxOutputPrintBytes, true, false);                
                ATCMD_PrintStringSafeWithDelimiterInfo((char*)&readCertState->buffer[maxOutputPrintBytes], readCertState->numBytesBuffered - maxOutputPrintBytes, false, true);                
            }
            ATCMD_Print("\r\n", 2);

            break;
        }

        case 2:
        {
            readCertState->numBytesBuffered = AT_CMD_READCERT_BUFFER_SZ;
            memset(readCertState->buffer, 0, AT_CMD_READCERT_BUFFER_SZ);
            size_t deviceCertSize = 1500;
            uint8_t deviceCert[deviceCertSize];

            ATCMD_Read_RootCert(deviceCert, &deviceCertSize);

            readCertState->numBytesBuffered = wc_DerToPem(deviceCert, deviceCertSize, readCertState->buffer, readCertState->numBytesBuffered, CERT_TYPE);
            if ((readCertState->numBytesBuffered <= 0)) {
                size_t  tmpNumBytesBuffered = readCertState->numBytesBuffered;
                SYS_CONSOLE_PRINT("    Failed converting device Cert to PEM (%d)\r\n", readCertState->numBytesBuffered);
                free(readCertState);
                return tmpNumBytesBuffered;
            }

            ATCMD_Printf("+READCERT:2, %d,", readCertState->numBytesBuffered);
            if(readCertState->numBytesBuffered < maxOutputPrintBytes)
            {
                ATCMD_PrintStringSafe((char*)&readCertState->buffer, readCertState->numBytesBuffered);
            }
            else
            {
                ATCMD_PrintStringSafeWithDelimiterInfo((char*)&readCertState->buffer, maxOutputPrintBytes, true, false);                
                ATCMD_PrintStringSafeWithDelimiterInfo((char*)&readCertState->buffer[maxOutputPrintBytes], readCertState->numBytesBuffered - maxOutputPrintBytes, false, true);                
            }
            ATCMD_Print("\r\n", 2);

            break;
        }

        case 3:
        {
            if(atCmdAppContext.certFileLength)
            {
                ATCMD_Printf("+READCERT:3, %d,", atCmdAppContext.certFileLength);
                if(atCmdAppContext.certFileLength < maxOutputPrintBytes)
                {
                    ATCMD_PrintStringSafe((char*)&atCmdAppContext.certFile, atCmdAppContext.certFileLength);
                }
                else
                {
                    ATCMD_PrintStringSafeWithDelimiterInfo((char*)&atCmdAppContext.certFile, maxOutputPrintBytes, true, false);                
                    ATCMD_PrintStringSafeWithDelimiterInfo((char*)&atCmdAppContext.certFile[maxOutputPrintBytes], atCmdAppContext.certFileLength - maxOutputPrintBytes, false, true);                
                }
                ATCMD_Print("\r\n", 2);
            }
            else
            {
                free(readCertState);
                return ATCMD_STATUS_ERROR;                
            }
            break;
        }

        case 4:
        {
            readCertState->numBytesBuffered = AT_CMD_READCERT_BUFFER_SZ;
            memset(readCertState->buffer, 0, AT_CMD_READCERT_BUFFER_SZ);
            ATCMD_Read_SignerCert(readCertState->buffer, &readCertState->numBytesBuffered);

            ATCMD_Printf("+READCERT:4, %d,", readCertState->numBytesBuffered);
            if(readCertState->numBytesBuffered < maxOutputPrintBytes)
            {
                ATCMD_PrintStringSafe((char*)&readCertState->buffer, readCertState->numBytesBuffered);
            }
            else
            {
                ATCMD_PrintStringSafeWithDelimiterInfo((char*)&readCertState->buffer, maxOutputPrintBytes, true, false);                
                ATCMD_PrintStringSafeWithDelimiterInfo((char*)&readCertState->buffer[maxOutputPrintBytes], readCertState->numBytesBuffered - maxOutputPrintBytes, false, true);                
            }
            ATCMD_Print("\r\n", 2);

            break;
        }

    default:
        {
            free(readCertState);
            return ATCMD_STATUS_ERROR;
        }
    }

    free(readCertState);
    return ATCMD_STATUS_OK;
}

static int ATCMD_Read_RootCert(uint8_t *rootCert, size_t *pRootCertSize) {
    ATCA_STATUS status;
    
    /*Read root Certificate*/
    status = tng_atcacert_root_cert_size(pRootCertSize);
    if (ATCA_SUCCESS != status) {
        SYS_CONSOLE_PRINT("    ATCMD_Read_RootCert: tng_atcacert_root_cert_size Failed \r\n");
        return status;
    }

    status = tng_atcacert_root_cert(rootCert, pRootCertSize);
    if (ATCA_SUCCESS != status) {
        SYS_CONSOLE_PRINT("    ATCMD_Read_RootCert: tng_atcacert_root_cert Failed \r\n");
        return status;
    }
        
    return 0;
}

static int ATCMD_Read_DeviceCert(uint8_t *deviceCert, size_t *pDeviceCertSize) {
    ATCA_STATUS status;
    extern ATCAIfaceCfg atecc608_0_init_data;

    status = atcab_init(&atecc608_0_init_data);
    if (ATCA_SUCCESS != status)
        return status;
    
    /*Read signer cert*/
    size_t signerCertSize = 0;
    status = tng_atcacert_max_signer_cert_size(&signerCertSize);
    if (ATCA_SUCCESS != status) {
        SYS_CONSOLE_PRINT("    ATCMD_Read_DeviceCert: tng_atcacert_max_signer_cert_size Failed \r\n");
        return status;
    }
    uint8_t signerCert[signerCertSize];
    status = tng_atcacert_read_signer_cert((uint8_t*) & signerCert, &signerCertSize);
    if (ATCA_SUCCESS != status) {
        SYS_CONSOLE_PRINT("    ATCMD_Read_DeviceCert: tng_atcacert_read_signer_cert Failed \r\n");
        return status;
    }

    /*Read device cert signer by the signer above*/
    status = tng_atcacert_max_device_cert_size(pDeviceCertSize);
    if (ATCA_SUCCESS != status) {
        SYS_CONSOLE_PRINT("    ATCMD_Read_DeviceCert: tng_atcacert_max_signer_cert_size Failed \r\n");
        return status;
    }

    status = tng_atcacert_read_device_cert(deviceCert, pDeviceCertSize, (uint8_t*) & signerCert);
    if (ATCA_SUCCESS != status) {
        SYS_CONSOLE_PRINT("    ATCMD_Read_DeviceCert: tng_atcacert_read_device_cert Failed (%x) \r\n", status);
        return status;
    }
    
    return 0;
}

static int ATCMD_Read_DeviceCertPem(uint8_t *deviceCertPem, size_t *pDeviceCertPemSize) {
    
    size_t deviceCertSize = 1024;
    uint8_t deviceCert[deviceCertSize];

    ATCMD_Read_DeviceCert(deviceCert, &deviceCertSize);

    *pDeviceCertPemSize = wc_DerToPem(deviceCert, deviceCertSize, deviceCertPem, *pDeviceCertPemSize, CERT_TYPE);
    if ((*pDeviceCertPemSize <= 0)) {
        SYS_CONSOLE_PRINT("    Failed converting device Cert to PEM (%d)\r\n", *pDeviceCertPemSize);
        return *pDeviceCertPemSize;
    }
    
    return 0;
}

static int ATCMD_Read_SignerCert(uint8_t *signerCert, size_t *pSignerCertSize) {
    ATCA_STATUS status;
    extern ATCAIfaceCfg atecc608_0_init_data;

    status = atcab_init(&atecc608_0_init_data);
    if (ATCA_SUCCESS != status)
        return status;

    size_t tmpSignerCertSize = 0;

    /*Read signer cert*/
    status = tng_atcacert_max_signer_cert_size(&tmpSignerCertSize);
    if (ATCA_SUCCESS != status) {
        SYS_CONSOLE_PRINT("    ATCMD_Read_DeviceCert: tng_atcacert_max_signer_cert_size Failed \r\n");
        return status;
    }

    uint8_t tmpSignerCert[tmpSignerCertSize];
    status = tng_atcacert_read_signer_cert(tmpSignerCert, &tmpSignerCertSize);
    if (ATCA_SUCCESS != status) {
        SYS_CONSOLE_PRINT("    ATCMD_Read_DeviceCert: tng_atcacert_read_signer_cert Failed \r\n");
        return status;
    }

    *pSignerCertSize = wc_DerToPem(tmpSignerCert, tmpSignerCertSize, signerCert, *pSignerCertSize, CERT_TYPE);
    if ((*pSignerCertSize <= 0)) {
        SYS_CONSOLE_PRINT("    Failed converting device Cert to PEM (%d)\r\n", *pSignerCertSize);
        return *pSignerCertSize;
    }
    
    return 0;
}

