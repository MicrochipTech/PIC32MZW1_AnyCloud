 /*******************************************************************************
 Source file for the Net Pres Encryption glue functions to work with Harmony


  Summary:


  Description:

*******************************************************************************/

/*****************************************************************************
 Copyright (C) 2013-2022 Microchip Technology Inc. and its subsidiaries.

Microchip Technology Inc. and its subsidiaries.

Subject to your compliance with these terms, you may use Microchip software 
and any derivatives exclusively with Microchip products. It is your 
responsibility to comply with third party license terms applicable to your 
use of third party software (including open source software) that may 
accompany Microchip software.

THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER 
EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED 
WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A PARTICULAR 
PURPOSE.

IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND 
WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS 
BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE 
FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN 
ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY, 
THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
*****************************************************************************/


#include "net_pres_enc_glue.h"
#include "net_pres/pres/net_pres_transportapi.h"
#include "net_pres/pres/net_pres_certstore.h"

#include "config.h"
#include "wolfssl/ssl.h"
#include "wolfssl/wolfcrypt/logging.h"
#include "wolfssl/wolfcrypt/random.h"
#include "cert_header.h"
#include "system/console/sys_console.h"

extern  int CheckAvailableSize(WOLFSSL *ssl, int size);
#include "wolfssl/wolfcrypt/port/atmel/atmel.h"

unsigned char *g_NewCertFile;
long g_NewCertSz = 0;

typedef struct 
{
    WOLFSSL_CTX* context;
    NET_PRES_TransportObject * transObject;
    bool isInited;
}net_pres_wolfsslInfo;

// Temporary fix till crypto library is upgraded to recent wolfssl versions.
int  InitRng(RNG* rng)
{
    return wc_InitRng(rng);
}

NET_PRES_EncProviderObject net_pres_EncProviderStreamClient0 = 
{
    .fpInit =    NET_PRES_EncProviderStreamClientInit0,
    .fpDeinit =  NET_PRES_EncProviderStreamClientDeinit0,
    .fpOpen =    NET_PRES_EncProviderStreamClientOpen0,
    .fpConnect = NET_PRES_EncProviderClientConnect0,
    .fpClose =   NET_PRES_EncProviderConnectionClose0,
    .fpWrite =   NET_PRES_EncProviderWrite0,
    .fpWriteReady =   NET_PRES_EncProviderWriteReady0,
    .fpRead =    NET_PRES_EncProviderRead0,
    .fpReadReady = NET_PRES_EncProviderReadReady0,
    .fpPeek =    NET_PRES_EncProviderPeek0,
    .fpIsInited = NET_PRES_EncProviderStreamClientIsInited0,
    .fpOutputSize = NET_PRES_EncProviderOutputSize0,
    .fpMaxOutputSize = NET_PRES_EncProviderMaxOutputSize0,
};
	
net_pres_wolfsslInfo net_pres_wolfSSLInfoStreamClient0;

int NET_PRES_EncGlue_StreamClientReceiveCb0(void *sslin, char *buf, int sz, void *ctx)
{
    int fd = *(int *)ctx;
    uint16_t bufferSize;
    bufferSize = (*net_pres_wolfSSLInfoStreamClient0.transObject->fpReadyToRead)((uintptr_t)fd);
    if (bufferSize == 0)
    {
        return WOLFSSL_CBIO_ERR_WANT_READ;
    }
    bufferSize = (*net_pres_wolfSSLInfoStreamClient0.transObject->fpRead)((uintptr_t)fd, (uint8_t*)buf, sz);
    return bufferSize;
}
int NET_PRES_EncGlue_StreamClientSendCb0(void *sslin, char *buf, int sz, void *ctx)
{
    int fd = *(int *)ctx;
    uint16_t bufferSize;
    bufferSize = (*net_pres_wolfSSLInfoStreamClient0.transObject->fpReadyToWrite)((uintptr_t)fd);
    if (bufferSize == 0)
    {
        return WOLFSSL_CBIO_ERR_WANT_WRITE;
    }

    bufferSize =  (*net_pres_wolfSSLInfoStreamClient0.transObject->fpWrite)((uintptr_t)fd, (uint8_t*)buf, (uint16_t)sz);
    return bufferSize;
}
	
static uint8_t _net_pres_wolfsslUsers = 0;

		
bool NET_PRES_EncProviderStreamClientInit0(NET_PRES_TransportObject * transObject)
{
    const uint8_t * caCertsPtr;
    int32_t caCertsLen;
    if (!NET_PRES_CertStoreGetCACerts(&caCertsPtr, &caCertsLen, 0))
    {
        return false;
    }
    if (_net_pres_wolfsslUsers == 0)
    {
        wolfSSL_Init();
        _net_pres_wolfsslUsers++;
    }
    net_pres_wolfSSLInfoStreamClient0.transObject = transObject;
	net_pres_wolfSSLInfoStreamClient0.context = wolfSSL_CTX_new(wolfSSLv23_client_method());
    if (net_pres_wolfSSLInfoStreamClient0.context == 0)
    {
        return false;
    }
    wolfSSL_SetIORecv(net_pres_wolfSSLInfoStreamClient0.context, (CallbackIORecv)&NET_PRES_EncGlue_StreamClientReceiveCb0);
    wolfSSL_SetIOSend(net_pres_wolfSSLInfoStreamClient0.context, (CallbackIOSend)&NET_PRES_EncGlue_StreamClientSendCb0);
#ifndef RN_MODE_DISABLED
#if 1 // Azure
		if (wolfSSL_CTX_load_verify_buffer(net_pres_wolfSSLInfoStreamClient0.context, app_client_cert_der_2048_azure, sizeof_app_client_cert_der_2048_azure, SSL_FILETYPE_ASN1) != SSL_SUCCESS)
		{
			// Couldn't load the CA certificates
			SYS_CONSOLE_PRINT("Something went wrong loading the CA certificates\r\n");
			wolfSSL_CTX_free(net_pres_wolfSSLInfoStreamClient0.context);
			return false;
		}
#endif
#if 1 // AWS
		if (wolfSSL_CTX_load_verify_buffer(net_pres_wolfSSLInfoStreamClient0.context, app_client_cert_der_starfield_base, sizeof_app_client_cert_der_starfield_base, SSL_FILETYPE_ASN1) != SSL_SUCCESS)
		{
			// Couldn't load the CA certificates
			SYS_CONSOLE_PRINT("Something went wrong loading the CA certificates\r\n");
			wolfSSL_CTX_free(net_pres_wolfSSLInfoStreamClient0.context);
			return false;
		}
		if (wolfSSL_CTX_load_verify_buffer(net_pres_wolfSSLInfoStreamClient0.context, caCertsPtr, caCertsLen, SSL_FILETYPE_ASN1) != SSL_SUCCESS)
		{
			// Couldn't load the CA certificates
			SYS_CONSOLE_PRINT("Something went wrong loading the CA certificates\r\n");
			wolfSSL_CTX_free(net_pres_wolfSSLInfoStreamClient0.context);
			return false;
		}
#endif	
#if 1 // MOSQUITTO_ORG
		if (wolfSSL_CTX_load_verify_buffer(net_pres_wolfSSLInfoStreamClient0.context, app_client_cert_der_mosquitto_org, sizeof_app_client_cert_der_mosquitto_org, SSL_FILETYPE_ASN1) != SSL_SUCCESS)
		{
			// Couldn't load the CA certificates
			SYS_CONSOLE_PRINT("Something went wrong loading the CA certificates\r\n");
			wolfSSL_CTX_free(net_pres_wolfSSLInfoStreamClient0.context);
			return false;
		}
#endif
#if 1// Local Mosquitto Server
		if (wolfSSL_CTX_load_verify_buffer(net_pres_wolfSSLInfoStreamClient0.context, app_client_cert_der_2048_local_mosquitto_server, sizeof_app_client_cert_der_2048_local_mosquitto_server, SSL_FILETYPE_ASN1) != SSL_SUCCESS)
		{
			// Couldn't load the CA certificates
			SYS_CONSOLE_PRINT("Something went wrong loading the CA certificates\r\n");
			wolfSSL_CTX_free(net_pres_wolfSSLInfoStreamClient0.context);
			return false;
		}
#endif
#if 1 //Certificate from Host
    if(g_NewCertSz)
    {
        if (wolfSSL_CTX_load_verify_buffer(net_pres_wolfSSLInfoStreamClient0.context, g_NewCertFile, g_NewCertSz, SSL_FILETYPE_ASN1) != SSL_SUCCESS)
        {
                // Couldn't load the CA certificates
                SYS_CONSOLE_PRINT("Something went wrong loading the CA certificates\r\n");
                wolfSSL_CTX_free(net_pres_wolfSSLInfoStreamClient0.context);
                return false;
        }
        g_NewCertSz = 0;
    }
#endif
#else
	if (wolfSSL_CTX_load_verify_buffer(net_pres_wolfSSLInfoStreamClient0.context, caCertsPtr, caCertsLen, SSL_FILETYPE_ASN1) != SSL_SUCCESS)
    {
        // Couldn't load the CA certificates
        //SYS_CONSOLE_MESSAGE("Something went wrong loading the CA certificates\r\n");
        wolfSSL_CTX_free(net_pres_wolfSSLInfoStreamClient0.context);
        return false;
    }
#endif	
    // Turn off verification, because SNTP is usually blocked by a firewall
    wolfSSL_CTX_set_verify(net_pres_wolfSSLInfoStreamClient0.context, SSL_VERIFY_PEER, 0);
//    wolfSSL_CTX_set_verify(net_pres_wolfSSLInfoStreamClient0.context, SSL_VERIFY_NONE, 0);
    /*initialize Trust*Go and load device certificate into the context*/
    atcatls_set_callbacks(net_pres_wolfSSLInfoStreamClient0.context);
    /*Use TLS extension since we support only P256R1 with ECC608 Trust&Go*/
    if (WOLFSSL_SUCCESS != wolfSSL_CTX_UseSupportedCurve(net_pres_wolfSSLInfoStreamClient0.context, WOLFSSL_ECC_SECP256R1)) {
        return false;
    }
    
    net_pres_wolfSSLInfoStreamClient0.isInited = true;
    return true;
}
bool NET_PRES_EncProviderStreamClientDeinit0(void)
{
    atmel_finish();
    wolfSSL_CTX_free(net_pres_wolfSSLInfoStreamClient0.context);
    net_pres_wolfSSLInfoStreamClient0.isInited = false;
    _net_pres_wolfsslUsers--;
    if (_net_pres_wolfsslUsers == 0)
    {
        wolfSSL_Cleanup();
    }
    return true;
}

char *sni_host_name = NULL;

bool NET_PRES_EncProviderStreamClientOpen0(uintptr_t transHandle, void * providerData)
{
        WOLFSSL* ssl = wolfSSL_new(net_pres_wolfSSLInfoStreamClient0.context);
        if (ssl == NULL)
        {
            return false;
        }
        if (wolfSSL_set_fd(ssl, transHandle) != SSL_SUCCESS)
        {
            wolfSSL_free(ssl);
            return false;
        }

		if(strlen(sni_host_name) != 0)
		{
//        	if (wolfSSL_UseSNI(ssl, WOLFSSL_SNI_HOST_NAME, NET_PRES_SNI_HOST_NAME, strlen(NET_PRES_SNI_HOST_NAME)) != WOLFSSL_SUCCESS)
			if (wolfSSL_UseSNI(ssl, WOLFSSL_SNI_HOST_NAME, sni_host_name, strlen(sni_host_name)) != WOLFSSL_SUCCESS)
	        {
    	        return false;
        	}
		}
        if (wolfSSL_UseALPN(ssl, NET_PRES_ALPN_PROTOCOL_NAME_LIST, sizeof(NET_PRES_ALPN_PROTOCOL_NAME_LIST),WOLFSSL_ALPN_FAILED_ON_MISMATCH) != WOLFSSL_SUCCESS)
        {
            return false;
        }
        memcpy(providerData, &ssl, sizeof(WOLFSSL*));
        return true;
}
bool NET_PRES_EncProviderStreamClientIsInited0(void)
{
    return net_pres_wolfSSLInfoStreamClient0.isInited;
}
NET_PRES_EncSessionStatus NET_PRES_EncProviderClientConnect0(void * providerData)
{
    WOLFSSL* ssl;
    memcpy(&ssl, providerData, sizeof(WOLFSSL*));
    int result = wolfSSL_connect(ssl);
    switch (result)
    {
        case SSL_SUCCESS:
            return NET_PRES_ENC_SS_OPEN;
        default:
        {
            int error = wolfSSL_get_error(ssl, result);
            switch (error)
            {
                case SSL_ERROR_WANT_READ:
                case SSL_ERROR_WANT_WRITE:
                    return NET_PRES_ENC_SS_CLIENT_NEGOTIATING;
                default:
                    return NET_PRES_ENC_SS_FAILED;
            }
        }
    }
}
NET_PRES_EncSessionStatus NET_PRES_EncProviderConnectionClose0(void * providerData)
{
    WOLFSSL* ssl;
    memcpy(&ssl, providerData, sizeof(WOLFSSL*));
    wolfSSL_free(ssl);
    return NET_PRES_ENC_SS_CLOSED;
}
int32_t NET_PRES_EncProviderWrite0(void * providerData, const uint8_t * buffer, uint16_t size)
{
    WOLFSSL* ssl;
    memcpy(&ssl, providerData, sizeof(WOLFSSL*));
    int ret = wolfSSL_write(ssl, buffer, size);
    if (ret < 0)
    {
        return 0;
    }    
    return ret;
}
uint16_t NET_PRES_EncProviderWriteReady0(void * providerData, uint16_t reqSize, uint16_t minSize)
{
    
    char buffer;
    WOLFSSL* ssl;
    memcpy(&ssl, providerData, sizeof(WOLFSSL*));

    int ret = wolfSSL_write(ssl, &buffer, 0);
    if(ret < 0)
    {
        return 0;
    }

    ret = CheckAvailableSize(ssl, reqSize);
    if(ret == 0)
    {   // success
        return reqSize;
    }
    if(minSize != 0)
    {
        ret = CheckAvailableSize(ssl, minSize);
        if(ret == 0)
        {   // success
            return minSize;
        }
    }

    return 0;
}
int32_t NET_PRES_EncProviderRead0(void * providerData, uint8_t * buffer, uint16_t size)
{
    WOLFSSL* ssl;
    memcpy(&ssl, providerData, sizeof(WOLFSSL*));
    int ret = wolfSSL_read(ssl, buffer, size);
    if (ret < 0)
    {
        return 0;
    } 
    return ret;
}

int32_t NET_PRES_EncProviderReadReady0(void * providerData)
{
    WOLFSSL* ssl;
    memcpy(&ssl, providerData, sizeof(WOLFSSL*));
    int32_t ret = wolfSSL_pending(ssl);
    if (ret == 0) // wolfSSL_pending() doesn't check the underlying layer.
    {
        char buffer;
        if (wolfSSL_peek(ssl, &buffer, 1) == 0)
        {
            return 0;
        }
        ret = wolfSSL_pending(ssl);
    }
    return ret;
}
        
int32_t NET_PRES_EncProviderPeek0(void * providerData, uint8_t * buffer, uint16_t size)
{
    WOLFSSL* ssl;
    memcpy(&ssl, providerData, sizeof(WOLFSSL*));
    int ret = wolfSSL_peek(ssl, buffer, size);
    if (ret < 0)
    {
        return 0;
    }  
    return ret;
}
int32_t NET_PRES_EncProviderOutputSize0(void * providerData, int32_t inSize)
{
    WOLFSSL* ssl;
    memcpy(&ssl, providerData, sizeof(WOLFSSL*));
    int ret = wolfSSL_GetOutputSize(ssl, inSize);
    if (ret < 0)
    {
        return 0;
    }  
    return ret;
}
int32_t NET_PRES_EncProviderMaxOutputSize0(void * providerData)
{
    WOLFSSL* ssl;
    memcpy(&ssl, providerData, sizeof(WOLFSSL*));
    int ret = wolfSSL_GetMaxOutputSize(ssl);
    if (ret < 0)
    {
        return 0;
    }  
    return ret;
}

bool NET_PRES_LoadVerifyDerBuffer(unsigned char* in, long sz)
{
    int ret = 0;
    if(net_pres_wolfSSLInfoStreamClient0.context == NULL)
    {
        g_NewCertFile = in;
        g_NewCertSz = sz;
        return true;
    }
    
    ret = wolfSSL_CTX_load_verify_buffer(net_pres_wolfSSLInfoStreamClient0.context, in, sz, SSL_FILETYPE_ASN1);
    if (ret != SSL_SUCCESS)
    {
        // Couldn't load the CA certificates
        SYS_CONSOLE_PRINT("Something went wrong (%d) loading the CA certificates\r\n", ret);
        wolfSSL_CTX_free(net_pres_wolfSSLInfoStreamClient0.context);
        return false;
    }
    return true;
}
