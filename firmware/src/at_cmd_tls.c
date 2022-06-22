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

#include "at_cmd_app.h"
#include "at_cmd_tls.h"
#include "wolfssl/ssl.h"
#include "wolfssl/wolfcrypt/logging.h"
#include "wolfssl/wolfcrypt/random.h"

extern ATCMD_APP_CONTEXT atCmdAppContext;

static int _sockWolfSSLRecvCallback(WOLFSSL *pSSLCtx, char *pBuf, int size, void *pAppCtx)
{
    TCP_SOCKET hTCP = *(TCP_SOCKET*)pAppCtx;
    
    if (false == TCPIP_TCP_IsConnected(hTCP))
    {
        return WOLFSSL_CBIO_ERR_CONN_CLOSE;
    }

    if (0 == TCPIP_TCP_GetIsReady(hTCP))
    {
        return WOLFSSL_CBIO_ERR_WANT_READ;
    }

    return TCPIP_TCP_ArrayGet(hTCP, (uint8_t*)pBuf, size);
}

static int _sockWolfSSLSendCallback(WOLFSSL *pSSLCtx, char *pBuf, int size, void *pAppCtx)
{
    TCP_SOCKET hTCP = *(TCP_SOCKET*)pAppCtx;

    if (false == TCPIP_TCP_IsConnected(hTCP))
    {
        return WOLFSSL_CBIO_ERR_CONN_CLOSE;
    }

    if (0 == TCPIP_TCP_PutIsReady(hTCP))
    {
        return WOLFSSL_CBIO_ERR_WANT_WRITE;
    }

    return TCPIP_TCP_ArrayPut(hTCP, (uint8_t*)pBuf, (uint16_t)size);
}

#ifdef WOLFSSL_ENCRYPTED_KEYS
static int _wolfsslPemPasswordCallback(char* passwd, int sz, int rw, void* userdata)
{
    const ATCMD_APP_TLS_CONF *pTlsConf = userdata;

    if (NULL == pTlsConf)
    {
        return 0;
    }

    if (0 == pTlsConf->priKeyPassword[0])
    {
        return 0;
    }

    if (pTlsConf->priKeyPassword[0] > sz)
    {
        return 0;
    }

    strcpy(passwd, &pTlsConf->priKeyPassword[1]);

    return pTlsConf->priKeyPassword[0];
}
#endif

static WOLFSSL_CTX* _tlsCreateTlsCtx(bool isClient)
{
    WOLFSSL_CTX *pTlsCtx;
    ATCMD_APP_TLS_CONF *pTlsConf;
    const AT_CMD_CERT_ENTRY     *pCACertEntry = NULL;
    const AT_CMD_CERT_ENTRY     *pCertEntry   = NULL;
    const AT_CMD_PRIKEY_ENTRY   *pPriKeyEntry = NULL;
    
   if (true == isClient)
    {
        pTlsCtx = wolfSSL_CTX_new(wolfSSLv23_client_method());
        pTlsConf = &atCmdAppContext.tlsConf[0];
    }
    else
    {
        pTlsCtx = wolfSSL_CTX_new(wolfSSLv23_server_method());
        pTlsConf = &atCmdAppContext.tlsConf[1];
    }

    if (NULL == pTlsCtx)
    {
        return NULL;
    }

    if (pTlsConf->caCertName[0] > 0)
    {
        pCACertEntry = ATCMD_APPCertFind(&pTlsConf->caCertName[1]);
    }
    if (pTlsConf->certName[0] > 0)
    {
        pCertEntry = ATCMD_APPCertFind(&pTlsConf->certName[1]);
    }
    if (pTlsConf->priKeyName[0] > 0)
    {
        pPriKeyEntry = ATCMD_APPPriKeyFind(&pTlsConf->priKeyName[1]);
    }

    wolfSSL_SetIORecv(pTlsCtx, &_sockWolfSSLRecvCallback);
    wolfSSL_SetIOSend(pTlsCtx, &_sockWolfSSLSendCallback);

    wolfSSL_CTX_set_verify(pTlsCtx, WOLFSSL_VERIFY_NONE, 0);

#ifdef HAVE_SNI
    if ('\0' != pTlsConf->serverName[0])
    {
        if (SSL_SUCCESS != wolfSSL_CTX_UseSNI(pTlsCtx, 0, pTlsConf->serverName, strlen(pTlsConf->serverName)))
        {
           wolfSSL_CTX_free(pTlsCtx);
            return NULL;
        }
    }
#endif

    if (NULL != pCACertEntry)
    {
        if (SSL_SUCCESS != wolfSSL_CTX_load_verify_buffer(pTlsCtx, pCACertEntry->pCertStart, pCACertEntry->size, pCACertEntry->format))
        {
            wolfSSL_CTX_free(pTlsCtx);
            return NULL;
        }

        wolfSSL_CTX_set_verify(pTlsCtx, WOLFSSL_VERIFY_PEER, 0);
    }

    if (NULL != pCertEntry)
    {
        if (SSL_SUCCESS != wolfSSL_CTX_use_certificate_buffer(pTlsCtx, pCertEntry->pCertStart, pCertEntry->size, pCertEntry->format))
        {
            wolfSSL_CTX_free(pTlsCtx);
            return NULL;
        }

#ifdef WOLFSSL_ENCRYPTED_KEYS
        wolfSSL_CTX_set_default_passwd_cb_userdata(pTlsCtx, (void*)pTlsConf);
        wolfSSL_CTX_set_default_passwd_cb(pTlsCtx, _wolfsslPemPasswordCallback);
#endif
    }
    if (NULL != pPriKeyEntry)
    {    
        if (SSL_SUCCESS != wolfSSL_CTX_use_PrivateKey_buffer(pTlsCtx, pPriKeyEntry->pPriKeyStart, pPriKeyEntry->pPriKeyEnd - pPriKeyEntry->pPriKeyStart, pPriKeyEntry->format))
        {
            wolfSSL_CTX_free(pTlsCtx);
            return NULL;
        }
    }

    if (WOLFSSL_SUCCESS != wolfSSL_CTX_UseSupportedCurve(pTlsCtx, WOLFSSL_ECC_SECP256R1)) {
        return false;
    }
    return pTlsCtx;
}

WOLFSSL* ATCMD_TLS_AllocSession(int stateIdx, ATCMD_APP_TLS_CONF *pTlsConf, bool isClient, int fd)
{
    ATCMD_APP_TLS_STATE *pTlsState;
    WOLFSSL *pTlsSess;

    if ((stateIdx < 1) || (stateIdx > AT_CMD_TLS_NUM_STATES))
    {
        return NULL;
    }

    if (NULL == pTlsConf)
    {
        return NULL;
    }

    pTlsState = &atCmdAppContext.tlsState[stateIdx-1];

    if (NULL == pTlsState->pTlsCtx)
    {
        pTlsState->pTlsCtx = _tlsCreateTlsCtx(isClient);

        if (NULL == pTlsState->pTlsCtx)
        {
            SYS_CONSOLE_PRINT("ATCMD_TLS_AllocSession(): (NULL == pTlsState->pTlsCtx)\r\n");
            return NULL;
        }

        pTlsState->numCtxSessions = 0;
    }
    else if (pTlsState->isClient != isClient)
    {
        return NULL;
    }

    pTlsSess = wolfSSL_new(pTlsState->pTlsCtx);

    if (NULL == pTlsSess)
    {
        return NULL;
    }

    if (SSL_SUCCESS != wolfSSL_set_fd(pTlsSess, fd))
    {
        wolfSSL_free(pTlsSess);
        return NULL;
    }

    pTlsState->pTlsConf = pTlsConf;
    pTlsState->isClient = isClient;
    pTlsState->numCtxSessions++;

    pTlsConf->numSessions++;

    return pTlsSess;
}

bool ATCMD_TLS_FreeSession(int stateIdx, WOLFSSL *pTlsSess)
{
    ATCMD_APP_TLS_STATE *pTlsState;

    if ((stateIdx < 1) || (stateIdx > AT_CMD_TLS_NUM_STATES))
    {
        return false;
    }

    pTlsState = &atCmdAppContext.tlsState[stateIdx-1];

    wolfSSL_free(pTlsSess);

    pTlsState->numCtxSessions--;

    if (0 == pTlsState->numCtxSessions)
    {
        wolfSSL_CTX_free(pTlsState->pTlsCtx);
        pTlsState->pTlsCtx  = NULL;

        if (NULL != pTlsState->pTlsConf)
        {
            pTlsState->pTlsConf->numSessions--;
            pTlsState->pTlsConf = NULL;
        }
    }

    return true;
}
