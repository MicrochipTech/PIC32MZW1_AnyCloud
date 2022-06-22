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

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include "include/at_cmds.h"
#include "at_cmd_pkcs.h"

#define ASN1_TAG_SEQUENCE       (0x30)
#define ASN1_TAG_INTEGER        (0x02)
#define ASN1_TAG_OCTET_STRING   (0x04)

static const int rsaPriKeyMap[] = {
    offsetof(PKCS1_RSA_PRIVATE_KEY, version),
    offsetof(PKCS1_RSA_PRIVATE_KEY, modulus),
    offsetof(PKCS1_RSA_PRIVATE_KEY, publicExponent),
    offsetof(PKCS1_RSA_PRIVATE_KEY, privateExponent),
    offsetof(PKCS1_RSA_PRIVATE_KEY, prime1),
    offsetof(PKCS1_RSA_PRIVATE_KEY, prime2),
    offsetof(PKCS1_RSA_PRIVATE_KEY, exponent1),
    offsetof(PKCS1_RSA_PRIVATE_KEY, exponent2),
    offsetof(PKCS1_RSA_PRIVATE_KEY, coefficient),
    -1
};

static const uint8_t objIDrsaEncryption[] = {0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x01,0x01};         // OBJECT IDENTIFIER  1.2.840.113549.1.1.1 rsaEncryption (PKCS #1)

static int8_t base64char(const char C)
{
    // base64char = ALPHA / DIGIT / "+" / "/"

    if ('+' == C)
        return 62;

    if ('/' == C)
        return 63;

    if ('9' >= C)
        return C-'0'+52;

    if ('Z' >= C)
        return C-'A';

    if ('z' >= C)
        return C-'a'+26;

    return -1;
}

static bool isWSP(const char C)
{
    // WSP = SP / HTAB

    if ((' ' == C) || (0x09 == C))
        return true;

    return false;
}

static bool iseol(const char C)
{
    // eol = CRLF / CR / LF

    if (('\n' == C) || ('\r' == C))
        return true;

    return false;
}

static int asn1DecodeTag(const uint8_t *pASN1, uint8_t *pTag, uint32_t *pLength, const uint8_t **pContents)
{
    int numTagBytes;

    if (NULL != pTag)
        *pTag = *pASN1;
    pASN1++;
    numTagBytes = 1;

    if (*pASN1 & 0x80)
    {
        int numLenBytes;

        numLenBytes = (*pASN1 & 0x7f);

        pASN1++;
        numTagBytes += (1+numLenBytes);

        *pLength = 0;
        while(numLenBytes--)
        {
            *pLength = (*pLength << 8) | *pASN1++;
        }
    }
    else
    {
        numTagBytes++;
        *pLength = *pASN1++;
    }

    numTagBytes += *pLength;

    if (NULL != pContents)
    {
        *pContents = pASN1;
    }

    return numTagBytes;
}

PKCS_PEM_TYPE PKCS_PEMType(const char *pText, size_t lengthText)
{
    if (NULL == pText)
    {
        return PKCS_PEM_TYPE_INVALID;
    }

    // preeb      = "-----BEGIN " label "-----"

    if (!strcmp(pText, "-----BEGIN "))
        return 0;

    pText += 11;

    if (0 == memcmp(pText, "RSA PRIVATE KEY", 15))
    {
        return PKCS_PEM_TYPE_RSA_PRIVATE_KEY;
    }
    else if (0 == memcmp(pText, "PRIVATE KEY", 11))
    {
        return PKCS_PEM_TYPE_PRIVATE_KEY;
    }
    else if (0 == memcmp(pText, "CERTIFICATE", 11))
    {
        return PKCS_PEM_TYPE_CERTIFICATE;
    }

    return PKCS_PEM_TYPE_INVALID;
}

size_t PKCS_PEMToDER(const char *pText, size_t lengthText, const char *pLabel, uint8_t *pBinary)
{
    uint32_t shiftReg;
    int numBits;
    size_t binaryLength = 0;
    int padding;
    int8_t index;

    if ((NULL == pText) || (NULL == pBinary))
        return 0;

    // preeb      = "-----BEGIN " label "-----"

    if (!strcmp(pText, "-----BEGIN "))
        return 0;

    pText += 11;

    if (NULL != pLabel)
    {
        if (!strcmp(pText, pLabel))
            return 0;

        pText += strlen(pLabel);
    }

    pText = strstr(pText, "-----");

    if (NULL == pText)
        return 0;

    pText += 5;

    // *WSP

    while(true == isWSP(*pText))
        pText++;

    // eol

    if (false == iseol(*pText))
        return 0;

    while(true == iseol(*pText))
        pText++;

    // *eolWSP
    // eolWSP     = WSP / CR / LF

    while((true == iseol(*pText)) || (true == isWSP(*pText)))
        pText++;

    shiftReg = 0;
    numBits  = 0;
    padding  = 0;

    while((0 != *pText) && ('-' != *pText))
    {
        if ((true == iseol(*pText)) || (true == isWSP(*pText)))
        {
            pText++;
            continue;
        }

        if ('=' != *pText)
        {
            index = base64char(*pText);

            if (index < 0)
            {
                return 0;
            }
        }
        else
        {
            index = 0;
            padding++;
        }

        shiftReg = (shiftReg << 6) | (index & 0x3f);
        numBits += 6;

        if (numBits >= 8)
        {
            numBits -= 8;
            *pBinary++ = (uint8_t)(shiftReg >> numBits);
            binaryLength++;
        }

        if ('=' == *pText)
        {
            break;
        }

        pText++;
    }

    // posteb     = "-----END " label "-----"

    if (!strcmp(pText, "-----END "))
    {
        return 0;
    }

    pText += 9;

    if (NULL != pLabel)
    {
        if (!strcmp(pText, pLabel))
        {
            return 0;
        }

        pText += strlen(pLabel);
    }

    pText = strstr(pText, "-----");

    if (NULL == pText)
    {
        return 0;
    }

    return binaryLength-padding;
}

size_t PKCS_DERLength(const uint8_t *pKey, size_t keyLength)
{
    uint8_t tagID;
    uint32_t tagLength;
    int numTagBytes;

    numTagBytes = asn1DecodeTag(pKey, &tagID, &tagLength, NULL);

    if (ASN1_TAG_SEQUENCE != tagID)
    {
        return 0;
    }

    if ((int)keyLength < numTagBytes)
    {
        return false;
    }

    return numTagBytes;
}

bool PKCS1_ParseRSAPrivateKeyDER(const uint8_t *pKey, size_t keyLength, PKCS1_RSA_PRIVATE_KEY * const pPrivateKey)
{
    uint8_t tagID;
    uint32_t tagLength;
    const uint8_t *pTagContents;
    int numTagBytes;
    const int *pAddrOffset;
    ADDR_LENGTH *pPriKeyTag;

/*
    RSAPrivateKey ::= SEQUENCE {
        version           Version,
        modulus           INTEGER,  -- n
        publicExponent    INTEGER,  -- e
        privateExponent   INTEGER,  -- d
        prime1            INTEGER,  -- p
        prime2            INTEGER,  -- q
        exponent1         INTEGER,  -- d mod (p-1)
        exponent2         INTEGER,  -- d mod (q-1)
        coefficient       INTEGER,  -- (inverse of q) mod p
        otherPrimeInfos   OtherPrimeInfos OPTIONAL
    }
*/

    if ((NULL == pKey) || (NULL == pPrivateKey))
    {
        return false;
    }

    memset(pPrivateKey, 0, sizeof(PKCS1_RSA_PRIVATE_KEY));

    numTagBytes = asn1DecodeTag(pKey, &tagID, &tagLength, &pTagContents);

    if (ASN1_TAG_SEQUENCE != tagID)
    {
        // Not SEQUENCE
        return false;
    }

    if ((int)keyLength < numTagBytes)
    {
        return false;
    }

    pKey = pTagContents;
    keyLength = tagLength;
    pAddrOffset = rsaPriKeyMap;

    do
    {
        numTagBytes = asn1DecodeTag(pKey, &tagID, &tagLength, &pTagContents);

        if (ASN1_TAG_INTEGER == tagID)
        {
            if ((tagLength > 1) && (0x00 == pTagContents[0]) && (0x00 != (pTagContents[1] & 0x80)))
            {
                pTagContents++;
                tagLength--;
            }

            if (*pAddrOffset >= 0)
            {
                pPriKeyTag = (ADDR_LENGTH*)&((uint8_t*)pPrivateKey)[*pAddrOffset];
                pAddrOffset++;

                pPriKeyTag->pAddress  = pTagContents;
                pPriKeyTag->length    = tagLength;
            }
        }

        pKey += numTagBytes;
        keyLength -= numTagBytes;
    }
    while(keyLength);

    return true;
}

bool PKCS8_ParsePrivateKeyDER(const uint8_t *pKey, size_t keyLength, PKCS1_RSA_PRIVATE_KEY * const pPrivateKey)
{
    uint8_t tagID;
    uint32_t tagLength;
    const uint8_t *pTagContents;
    int numTagBytes;

/*
      PrivateKeyInfo ::= SEQUENCE {
        version                   Version,
        privateKeyAlgorithm       PrivateKeyAlgorithmIdentifier,
        privateKey                PrivateKey,
        attributes           [0]  IMPLICIT Attributes OPTIONAL }

      Version ::= INTEGER

      PrivateKeyAlgorithmIdentifier ::= AlgorithmIdentifier

      PrivateKey ::= OCTET STRING

      Attributes ::= SET OF Attribute
*/

    if ((NULL == pKey) || (NULL == pPrivateKey))
    {
        return false;
    }

    memset(pPrivateKey, 0, sizeof(PKCS1_RSA_PRIVATE_KEY));

    numTagBytes = asn1DecodeTag(pKey, &tagID, &tagLength, &pTagContents);

    if (ASN1_TAG_SEQUENCE != tagID)
    {
        // Not SEQUENCE
        return false;
    }

    pKey = pTagContents;
    keyLength = tagLength;

    numTagBytes = asn1DecodeTag(pKey, &tagID, &tagLength, &pTagContents);

    if (ASN1_TAG_INTEGER != tagID)
    {
        // Not INTEGER
        return false;
    }

    pKey += numTagBytes;
    keyLength -= numTagBytes;

    numTagBytes = asn1DecodeTag(pKey, &tagID, &tagLength, &pTagContents);

    if (ASN1_TAG_SEQUENCE != tagID)
    {
        // Not SEQUENCE
        return false;
    }

    asn1DecodeTag(pTagContents, &tagID, &tagLength, &pTagContents);

    if ((sizeof(objIDrsaEncryption) == tagLength) && (0 == memcmp(objIDrsaEncryption, pTagContents, tagLength)))
    {
        pKey += numTagBytes;
        keyLength -= numTagBytes;

        numTagBytes = asn1DecodeTag(pKey, &tagID, &tagLength, &pTagContents);

        if (ASN1_TAG_OCTET_STRING != tagID)
        {
            // Not OCTET_STRING
            return false;
        }

        return PKCS1_ParseRSAPrivateKeyDER(pTagContents, tagLength, pPrivateKey);
    }

    return false;
}
