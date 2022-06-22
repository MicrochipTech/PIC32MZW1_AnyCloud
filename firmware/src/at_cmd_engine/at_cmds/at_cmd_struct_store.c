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
#include <stdarg.h>
#include <string.h>

#include "include/at_cmds.h"
#include "at_cmds/at_cmd_inet.h"
#include "at_cmds/at_cmd_parser.h"

const ATCMD_STORE_MAP_ELEMENT* ATCMD_StructStoreFindNext(const ATCMD_STORE_MAP_ELEMENT *pstaConfMap)
{
    if (NULL == pstaConfMap)
    {
        return NULL;
    }

    if (pstaConfMap->type != ATCMD_STORE_TYPE_INVALID)
    {
        return ++pstaConfMap;
    }

    return NULL;
}

const ATCMD_STORE_MAP_ELEMENT* ATCMD_StructStoreFindElementByID(const ATCMD_STORE_MAP_ELEMENT *pstaConfMap, int id)
{
    if ((NULL == pstaConfMap) || (id < 0))
    {
        return NULL;
    }

    while (pstaConfMap->type != ATCMD_STORE_TYPE_INVALID)
    {
        if (id == pstaConfMap->id)
        {
            return pstaConfMap;
        }

        pstaConfMap++;
    }

    return NULL;
}

/*****************************************************************************
 * Search a map using the provided ID and write to the element offset
 *****************************************************************************/
int ATCMD_StructStoreWriteParam(const ATCMD_STORE_MAP_ELEMENT *pstaConfMap, const void *pStruct, int id, ATCMD_PARAM *pParam)
{
    pstaConfMap = ATCMD_StructStoreFindElementByID(pstaConfMap, id);

    if ((NULL == pstaConfMap) || (NULL == pStruct) || (NULL == pParam))
    {
        return 0;
    }

    if (0 == (pstaConfMap->access & ATCMD_STORE_ACCESS_WRITE))
    {
        /* Writing to a non-writable element */
        return 0;
    }

    switch(pstaConfMap->type)
    {
        case ATCMD_STORE_TYPE_INT:
        {
            /* Access integer type from structure using offset provided by map */

            int *pElemIntPtr = (int*)&((uint8_t*)pStruct)[pstaConfMap->offset];

            if (ATCMD_PARAM_TYPE_INTEGER != pParam->type)
            {
                return 0;
            }

            *pElemIntPtr = pParam->value.i;

            return pstaConfMap->maxSize;
        }

        case ATCMD_STORE_TYPE_STRING:
        {
            /* Access string/array type from structure using offset provided by map */

            char *pElemStrPtr = (char*)&((uint8_t*)pStruct)[pstaConfMap->offset];

            if ((ATCMD_PARAM_TYPE_ASCII_STRING != pParam->type) && (ATCMD_PARAM_TYPE_HEX_STRING != pParam->type))
            {
                return 0;
            }
            else if (pParam->length > pstaConfMap->maxSize)
            {
                return 0;
            }

            memset(&pElemStrPtr[1], 0, pstaConfMap->maxSize);
            pElemStrPtr[0] = pParam->length;
            memcpy(&pElemStrPtr[1], pParam->value.p, pParam->length);

            return pParam->length;
        }

        case ATCMD_STORE_TYPE_IPV4ADDR:
        {
            /* Access packed IPv4 address type from structure using offset provided by map */

            uint32_t *pElemIPv4Ptr = (uint32_t*)&((uint8_t*)pStruct)[pstaConfMap->offset];

            if ((ATCMD_PARAM_TYPE_ASCII_STRING != pParam->type) && (ATCMD_PARAM_TYPE_HEX_STRING != pParam->type))
            {
                return 0;
            }

            *pElemIPv4Ptr = at_cmd_inet_addr((char*)pParam->value.p);

            return pstaConfMap->maxSize;
        }

        case ATCMD_STORE_TYPE_BOOL:
        {
            /* Access boolean flag type from structure using offset provided by map */

            bool *pElemBoolPtr = (bool*)&((uint8_t*)pStruct)[pstaConfMap->offset];

            switch(pParam->value.i)
            {
                case 0:
                {
                    *pElemBoolPtr = false;
                    break;
                }

                case 1:
                {
                    *pElemBoolPtr = true;
                    break;
                }

                default:
                {
                    return 0;
                }
            }

            return pstaConfMap->maxSize;
        }

        case ATCMD_STORE_TYPE_MACADDR:
        {
            /* Access mac address type from structure using offset provided by map */

            uint8_t *pElemMACAddrPtr = (uint8_t*)&((uint8_t*)pStruct)[pstaConfMap->offset];
            int i;

            if (((6*2)+5) != pParam->length)
            {
                return 0;
            }

            for (i=0; i<pParam->length; i += 3)
            {
                ATCMD_HexStringToBytes((char*)&pParam->value.p[i], 2, pElemMACAddrPtr++);
            }

            return pstaConfMap->maxSize;

            break;
        }

        default:
        {
            return 0;
        }
    }

    return 0;
}

/*****************************************************************************
 * Search a map using the provided ID and read from the element offset
 *****************************************************************************/
bool ATCMD_StructStorePrint(const char *pATCmd, const ATCMD_STORE_MAP_ELEMENT *pstaConfMap, const void *pStruct, int id)
{
    pstaConfMap = ATCMD_StructStoreFindElementByID(pstaConfMap, id);

    if ((NULL == pstaConfMap) || (NULL == pATCmd) || (NULL == pStruct))
    {
        return false;
    }

    /* Pre-display reading output including AT command string and ID */

    ATCMD_Printf("%s:%d,", pATCmd, pstaConfMap->id);

    if (0 == (pstaConfMap->access & ATCMD_STORE_ACCESS_READ))
    {
        ATCMD_Print("******\r\n", 8);

        return true;
    }

    switch(pstaConfMap->type)
    {
        case ATCMD_STORE_TYPE_INT:
        {
            /* Access integer type from structure using offset provided by map */

            int *pElemIntPtr = (int*)&((uint8_t*)pStruct)[pstaConfMap->offset];

            ATCMD_Printf("%d", *pElemIntPtr);
            break;
        }

        case ATCMD_STORE_TYPE_STRING:
        {
            /* Access string/array type from structure using offset provided by map */

            char *pElemStrPtr = (char*)&((uint8_t*)pStruct)[pstaConfMap->offset];

            ATCMD_PrintStringSafe(&pElemStrPtr[1], pElemStrPtr[0]);
            break;
        }

        case ATCMD_STORE_TYPE_IPV4ADDR:
        {
            /* Access packed IPv4 address type from structure using offset provided by map */

            uint32_t *pElemIPv4Ptr = (uint32_t*)&((uint8_t*)pStruct)[pstaConfMap->offset];

            ATCMD_PrintIPv4Address(*pElemIPv4Ptr);
            break;
        }

        case ATCMD_STORE_TYPE_BOOL:
        {
            /* Access boolean flag type from structure using offset provided by map */

            bool *pElemBoolPtr = (bool*)&((uint8_t*)pStruct)[pstaConfMap->offset];

            ATCMD_Printf("%d", *pElemBoolPtr ? 1 : 0);
            break;
        }

        case ATCMD_STORE_TYPE_MACADDR:
        {
            /* Access mac address type from structure using offset provided by map */

            uint8_t *pElemMACAddrPtr = (uint8_t*)&((uint8_t*)pStruct)[pstaConfMap->offset];

            ATCMD_PrintMACAddress(pElemMACAddrPtr);
            break;
        }

        default:
        {
            return false;
        }
    }

    ATCMD_Printf("\r\n");

    return true;
}
