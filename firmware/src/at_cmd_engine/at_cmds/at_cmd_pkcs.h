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

#ifndef _AT_CMD_PKCS_H
#define _AT_CMD_PKCS_H

#ifdef __cplusplus  // Provide C++ Compatibility
extern "C" {
#endif

typedef enum
{
    PKCS_PEM_TYPE_INVALID,
    PKCS_PEM_TYPE_RSA_PRIVATE_KEY,
    PKCS_PEM_TYPE_PRIVATE_KEY,
    PKCS_PEM_TYPE_CERTIFICATE
} PKCS_PEM_TYPE;

typedef struct
{
    const uint8_t *pAddress;
    uint32_t length;
} ADDR_LENGTH;

typedef struct
{
    ADDR_LENGTH version;
    ADDR_LENGTH modulus;
    ADDR_LENGTH publicExponent;
    ADDR_LENGTH privateExponent;
    ADDR_LENGTH prime1;
    ADDR_LENGTH prime2;
    ADDR_LENGTH exponent1;
    ADDR_LENGTH exponent2;
    ADDR_LENGTH coefficient;
} PKCS1_RSA_PRIVATE_KEY;

PKCS_PEM_TYPE PKCS_PEMType(const char *pText, size_t lengthText);
size_t PKCS_PEMToDER(const char *pText, size_t lengthText, const char *pLabel, uint8_t *pBinary);
size_t PKCS_DERLength(const uint8_t *pKey, size_t keyLength);
bool PKCS1_ParseRSAPrivateKeyDER(const uint8_t *pKey, size_t keyLength, PKCS1_RSA_PRIVATE_KEY * const pPrivateKey);
bool PKCS8_ParsePrivateKeyDER(const uint8_t *pKey, size_t keyLength, PKCS1_RSA_PRIVATE_KEY * const pPrivateKey);

#ifdef __cplusplus
}
#endif

#endif /* _AT_CMD_PKCS_H */
