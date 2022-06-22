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
#include <stdlib.h>
#include <string.h>

#include "include/at_cmds.h"
#include "at_cmd_parser.h"
#include "terminal/terminal.h"

extern const AT_CMD_TYPE_DESC* atCmdTypeDescTable[];

#ifdef AT_CMD_CONF_MANUFACTURER_ID
extern const AT_CMD_TYPE_DESC atCmdTypeDescGMI;
#endif
#ifdef AT_CMD_CONF_MODEL_ID
extern const AT_CMD_TYPE_DESC atCmdTypeDescGMM;
#endif
#ifdef AT_CMD_CONF_REVISION_ID
extern const AT_CMD_TYPE_DESC atCmdTypeDescGMR;
#endif
extern const AT_CMD_TYPE_DESC atCmdTypeDescIPR;

const AT_CMD_TYPE_DESC* atCmdTypeDescTableInt[] =
{
#ifdef AT_CMD_CONF_MANUFACTURER_ID
    &atCmdTypeDescGMI,
#endif
#ifdef AT_CMD_CONF_MODEL_ID
    &atCmdTypeDescGMM,
#endif
#ifdef AT_CMD_CONF_REVISION_ID
    &atCmdTypeDescGMR,
#endif
    &atCmdTypeDescIPR,
    NULL,
};

extern const AT_CMD_TYPE_DESC* pCurrentCmdTypeDesc;

/*****************************************************************************
 * Convert a char representation of a digit into a number
 *****************************************************************************/
static uint8_t CharToByte(char c)
{
    if ((c >= '0') && (c <= '9'))
    {
        c -= '0';
    }
    else if ((c >= 'a') && (c <= 'f'))
    {
        c -= ('a' - 10);
    }
    else if ((c >= 'A') && (c <= 'F'))
    {
        c -= ('A' - 10);
    }
    else
    {
        return 0;
    }

    return c;
}

/*****************************************************************************
 * Translate a string of digit characters into numbers
 *****************************************************************************/
uint16_t ATCMD_HexStringToBytes(const char *pStr, uint16_t strLength, uint8_t *pBytes)
{
    uint16_t retLength;

    if ((NULL == pStr) || (NULL == pBytes))
    {
        return 0;
    }

    retLength = strLength >> 1;

    while(strLength >= 2)
    {
        *pBytes = CharToByte(*pStr++) << 4;
        *pBytes++ |= CharToByte(*pStr++);

        strLength -= 2;
    }

    return retLength;
}

/*****************************************************************************
 * Translate command string into the command descriptor
 *****************************************************************************/
static const AT_CMD_TYPE_DESC* _FindCmdDesc(const AT_CMD_TYPE_DESC **pCmdTableEntry, const char *pCmd)
{
    if (NULL == pCmd)
    {
        return NULL;
    }

    while (NULL != *pCmdTableEntry)
    {
        if (NULL != (*pCmdTableEntry)->pCmdName)
        {
            if (0 == strcmp((*pCmdTableEntry)->pCmdName, pCmd))
            {
                return *pCmdTableEntry;
            }
        }

        pCmdTableEntry++;
    }

    return NULL;
}

/*****************************************************************************
 * Execute the command
 *****************************************************************************/
static bool _ExecuteCommand(const AT_CMD_TYPE_DESC* pCmdTypeDesc, int numParams, ATCMD_PARAM *pParamList)
{
    if (NULL == pCmdTypeDesc)
    {
        return false;
    }

    if (NULL != pCmdTypeDesc->cmdExecute)
    {
        ATCMD_STATUS status;

        pCurrentCmdTypeDesc = pCmdTypeDesc;

        status = pCmdTypeDesc->cmdExecute(pCmdTypeDesc, numParams, pParamList);

        if (ATCMD_STATUS_PENDING != status)
        {
            ATCMD_CompleteCommand(status);
        }
    }

    return true;
}

/*****************************************************************************
 * Try executing command internally
 *****************************************************************************/
static bool _ExecuteInternalCommand(const char *pCmd)
{
    char cmd;
    int num;
    int cmdNameLen = strlen(pCmd) + 1;

    cmd = 0;
    num = 0;

    while (cmdNameLen--)
    {
        if (0 == cmd)
        {
            if ((*pCmd < 'A') || (*pCmd > 'Z'))
            {
                return false;
            }

            cmd = *pCmd++;

            continue;
        }
        else
        {
            if ((*pCmd >= '0') && (*pCmd <= '9'))
            {
                num = (num * 10) + (*pCmd++ - '0');

                continue;
            }

            switch (cmd)
            {
                case 'E':
                {
                    if (0 == num)
                    {
                        TP_EchoSet(false);
                    }
                    else if (1 == num)
                    {
                        TP_EchoSet(true);
                    }
                    else
                    {
                        return false;
                    }

                    break;
                }

                case 'V':
                {
                    if ((num < 0) || (num > AT_CMD_MAX_VERBOSITY_LVL))
                    {
                        return false;
                    }

                    ATCMD_SetStatusVerbosityLevel(num);
                    break;
                }

                default:
                {
                    return false;
                }
            }

            cmd = 0;
            num = 0;

            if (0 == *pCmd)
            {
                return true;
            }

            cmdNameLen++;
        }
    }

    return false;
}

/*****************************************************************************
 * Translate parameters from string form into data form
 *****************************************************************************/
static uint8_t* _TranslateParam(ATCMD_PARAM *pParam)
{
    switch(pParam->type)
    {
        case ATCMD_PARAM_TYPE_INTEGER:
        {
            uint8_t *pTmp = pParam->value.p;

            pParam->value.p[pParam->length] = '\0';

            if ('-' == pParam->value.p[0])
            {
                pParam->value.i = strtol((char*)pParam->value.p, NULL, 0);
            }
            else
            {
                pParam->value.u = strtoul((char*)pParam->value.p, NULL, 0);
            }

            pParam->length = 1;
            return pTmp;
        }

        case ATCMD_PARAM_TYPE_ASCII_STRING:
            pParam->value.p[pParam->length] = '\0';
            return &pParam->value.p[pParam->length+1];

        case ATCMD_PARAM_TYPE_HEX_STRING:
            if (pParam->length & 1)
            {
                return NULL;
            }

            pParam->length = ATCMD_HexStringToBytes((char*)pParam->value.p, (uint16_t)pParam->length, pParam->value.p);
            pParam->value.p[pParam->length] = '\0';
            return &pParam->value.p[pParam->length+1];

        default:
            break;
    }

    return NULL;
}

/*****************************************************************************
 * Create list of parameters to pass to execute commands
 *****************************************************************************/
static int _ParseCommandParams(char *pCmd, ATCMD_PARAM *pParamList)
{
    int numParams = 0;
    uint8_t *pCmdDst = (uint8_t*)pCmd;
    int cmdLength;
    bool expectCommas = true;
    bool hexInt = false;

    cmdLength = (int)strlen(pCmd);

    if (0 == cmdLength)
    {
        return -1;
    }

    pParamList->value.p = pCmdDst;
    pParamList->length  = 0;
    pParamList->type    = ATCMD_PARAM_TYPE_INVALID;

    while ((cmdLength > 0) && ('\0' != *pCmd))
    {
        char token;

        token = *pCmd++;
        cmdLength--;

        if ((',' == token) && (true == expectCommas))
        {
            /* Commas delimit parameters, but only when not in the middle
               of strings etc, so only split parameters when expecting to do so */

            /* Translate string representation of parameter into it actual value */
            pCmdDst = _TranslateParam(pParamList);

            if (NULL == pCmdDst)
            {
                return -1;
            }

            numParams++;

            /* Initialise the next parameter */
            pParamList++;
            pParamList->value.p = pCmdDst;
            pParamList->length  = 0;
            pParamList->type    = ATCMD_PARAM_TYPE_INVALID;
            continue;
        }
        else if ((0 == pParamList->length) && (ATCMD_PARAM_TYPE_INVALID == pParamList->type))
        {
            /* Special case processing if type is not yet known and no characters have been accepted */
            if (' ' == token)
            {
                /* White space is ignored at the start of a parameters */
                continue;
            }
            else if ('"' == token)
            {
                /* Double quotes indicate an ASCII string parameter */
                pParamList->type = ATCMD_PARAM_TYPE_ASCII_STRING;
                expectCommas = false;
                continue;
            }
            else if ('[' == token)
            {
                /* Opening square braces indicates a hex string sequence */
                pParamList->type = ATCMD_PARAM_TYPE_HEX_STRING;
                expectCommas = false;
                continue;
            }
            else if ('-' == token)
            {
                /* Minus sign indicates a signed integer, as we must have further
                   numeric values we don't accept commas at this point */
                pParamList->type = ATCMD_PARAM_TYPE_INTEGER;

                expectCommas = false;
                hexInt = false;
            }
            else if ((token >= '0') && (token <= '9'))
            {
                /* Numeric values indicate integer */
                pParamList->type = ATCMD_PARAM_TYPE_INTEGER;

                hexInt = false;
            }
            else
            {
                /* Anything else is not permitted */
                return -1;
            }
        }

        switch(pParamList->type)
        {
            case ATCMD_PARAM_TYPE_INTEGER:
            {
                /* Integers can be signed or unsigned decimal or unsigned hex conforming to 0xn* or 0Xn* format */

                if ((1 == pParamList->length) && (('x' == token) || ('X' == token)) && ('0' == *pParamList->value.p) && (0 != cmdLength))
                {
                    /* String must start with '0' and have either 'x' or 'X' in the second position and not be the last character of all.
                       we don't expect commas after this as there must be more digits following 'x'/'X' */

                    hexInt = true;
                    expectCommas = false;
                }
                else if ((true == hexInt) && (pParamList->length >= 2) && ((token < '0') || ((token > '9') && (token < 'A')) || ((token >  'F') && (token < 'a')) || (token > 'f')))
                {
                    /* For hex integers: digits 2+ must be 0-9, A-F or a-f */
                    return -1;
                }
                else if ((false == hexInt) && (pParamList->length >= 1) && ((token < '0') || (token > '9')))
                {
                    /* For decimal integers: digits 1+ must be 0-9 */
                    return -1;
                }
                else
                {
                    /* A valid character not meeting the above criteria will clear the expect commas condition */
                    expectCommas = true;
                }

                break;
            }

            case ATCMD_PARAM_TYPE_ASCII_STRING:
            {
                /* ASCII string must be encased in double quotes "...". Escape sequences are permitted using '\' */

                if (('"' == token) && (false == expectCommas))
                {
                    /* If double quotes is encountered and we are still within the previous double quotes clear
                       the expect commas condition, the string is now terminated */
                    expectCommas = true;
                    continue;
                }
                else if (true == expectCommas)
                {
                    /* The only characters allowed after the quoted string is white space */
                    if (' ' != token)
                    {
                        return -1;
                    }

                    continue;
                }
                else if ((token <= 0x1f) || (token >= 0x7f))
                {
                    /* Accept only alpha-numeric + symbols only */
                    return -1;
                }
                else if ('\\' == token)
                {
                    /* '\' indicates an escape sequence, but only if not the last character */
                    if (0 == cmdLength)
                    {
                        return -1;
                    }

                    if ('e' == *pCmd)
                    {
                        /* Handle \e (escape) */
                        token = 0x1b;
                    }
                    else if ('"' == *pCmd)
                    {
                        /* Handle \" to allow double quotes within string delimiters */
                        token = '"';
                    }
                    else if ('\\' != *pCmd)
                    {
                        /* Handle everything else but '\\' used to encode '\' */
                        int i;

                        /* Translate \a \b \t \n \v \f \r are grouped together */
                        for (i=0; i<7; i++)
                        {
                            if (*pCmd == "abtnvfr"[i])
                            {
                                token = (char)(0x07+i);
                                break;
                            }
                        }

                        if (i == 7)
                        {
                            /* Didn't find the character so fail here */
                            return -1;
                        }
                    }

                    /* Swallow the extra character */
                    pCmd++;
                    cmdLength--;
                }

                break;
            }

            case ATCMD_PARAM_TYPE_HEX_STRING:
            {
                /* Hex string must be encased in square braces [...], only alpha-numeric pairs allowed */

                if ((']' == token) && (false == expectCommas))
                {
                    /* If closing square brace is encountered and we are still within the previous opening square brace clear
                       the expect commas condition, the string is now terminated */
                    expectCommas = true;
                    continue;
                }
                else if (true == expectCommas)
                {
                    /* The only characters allowed after the square braced string is white space */
                    if (' ' != token)
                    {
                        return -1;
                    }

                    continue;
                }
                else if ((token < '0') || ((token > '9') && (token < 'A')) || ((token >  'Z') && (token < 'a')) || (token > 'z'))
                {
                    /* For hex integers: digits must be 0-9, A-F or a-f, reject all others */
                    return -1;
                }

                break;
            }

            default:
            {
                break;
            }
        }

        *pCmdDst++ = token;
        pParamList->length++;
    }

    pCmdDst = _TranslateParam(pParamList);

    if (NULL == pCmdDst)
    {
        return -1;
    }

    numParams++;

    return numParams;
}

/*****************************************************************************
 * Parse a complete AT command line
 *****************************************************************************/
bool ATCMD_ParseCommandLine(char *pCmdLine)
{
    char *pCmdName;
    char *pSrchStr;
    const AT_CMD_TYPE_DESC* pCmdTypeDesc;
    int numParamsFound;
    bool helpRequest;
    bool readRequest;
    ATCMD_PARAM params[AT_CMD_MAX_NUM_PARAMS];
    int maxCmdLen;

    if (NULL == pCmdLine)
    {
        ATCMD_ReportStatus(ATCMD_STATUS_ERROR);
        return false;
    }

    if (0 != strncmp(pCmdLine, AT_CMD_CONF_AT_BASE_STRING, AT_CMD_CONF_AT_BASE_STRING_SZ))
    {
        ATCMD_ReportStatus(ATCMD_STATUS_INVALID_CMD);
        return false;
    }

    /* Skip past "AT". */
    pCmdLine += AT_CMD_CONF_AT_BASE_STRING_SZ;

    /* Respond to 'AT' on it's own. */
    if ('\0' == *pCmdLine)
    {
        ATCMD_ReportStatus(ATCMD_STATUS_OK);
        return true;
    }

    pCmdName = pCmdLine;

    /* Set limit on scanning command line */

    maxCmdLen = AT_CMD_CONF_MAX_COMMAND_LENGTH - AT_CMD_CONF_AT_BASE_STRING_SZ;

    while (maxCmdLen--)
    {
        if ('=' == *pCmdLine)
        {
            /* Command has arguments, drop '\0' to split commands from arguments and move pointer on past '=' */
            *pCmdLine++ = '\0';
            maxCmdLen--;
            break;
        }

        if ('\0' == *pCmdLine)
        {
            /* Command has no arguments */
            break;
        }

        pCmdLine++;
    }

    if (maxCmdLen < 0)
    {
        /* No '=' or end of string found. */
        ATCMD_ReportStatus(ATCMD_STATUS_INVALID_CMD);

        return false;
    }

    /* Look for command ending in '/?', set a flag and remove it for translation. */

    helpRequest = false;
    readRequest = false;

    pSrchStr = strstr(pCmdName, "/?");

    if (NULL != pSrchStr)
    {
        if (strlen(pSrchStr) == 2)
        {
            helpRequest = true;

            /* Remove "\?" from end of command */

            *pSrchStr = '\0';
        }
    }
    else
    {
        int cmdStrLen;

        cmdStrLen = strlen(pCmdName);

        if ((cmdStrLen > 1) && ('?' == pCmdName[cmdStrLen-1]))
        {
            cmdStrLen--;
            pCmdName[cmdStrLen] = '\0';

            readRequest = true;
        }
    }

    /* Translate CMD field. */
    pCmdTypeDesc = _FindCmdDesc(atCmdTypeDescTable, pCmdName);

    if (NULL == pCmdTypeDesc)
    {
        pCmdTypeDesc = _FindCmdDesc(atCmdTypeDescTableInt, pCmdName);

        if (NULL == pCmdTypeDesc)
        {
            if (true == _ExecuteInternalCommand(pCmdName))
            {
                ATCMD_ReportStatus(ATCMD_STATUS_OK);

                return true;
            }
            else
            {
                /* Unknown CMD field found. */
                ATCMD_ReportStatus(ATCMD_STATUS_UNKNOWN_CMD);

                return false;
            }
        }
    }

    if ((true == helpRequest) || ('?' == *pCmdLine))
    {
        ATCMD_ShowHelp(pCmdTypeDesc);

        ATCMD_ReportStatus(ATCMD_STATUS_OK);

        return true;
    }

    if (('\0' == *pCmdLine) || (true == readRequest))
    {
        /* No parameters for command. */

        if (false == _ExecuteCommand(pCmdTypeDesc, 0, NULL))
        {
            return false;
        }
    }
    else
    {
        /* Search for null terminate within limits */

        pSrchStr = pCmdLine;
        while (maxCmdLen--)
        {
            if ('\0' == *pSrchStr)
            {
                break;
            }

            pSrchStr++;
        }

        if (maxCmdLen < 0)
        {
            /* Null terminate not found, arguments too long */
            return false;
        }

        numParamsFound = _ParseCommandParams(pCmdLine, params);

        if (numParamsFound < 0)
        {
            ATCMD_ReportStatus(ATCMD_STATUS_INCORRECT_NUM_PARAMS);

            return false;
        }

        if (false == _ExecuteCommand(pCmdTypeDesc, numParamsFound, params))
        {
            return false;
        }
    }

    return true;
}

/*****************************************************************************
 * Validate parameter list against the command variant descriptor
 *****************************************************************************/
bool ATCMD_ParamValidateTypes(const AT_CMD_TYPE_DESC *pCmdDesc, const int varIdx, const int numParams, ATCMD_PARAM *pParamList)
{
    int i;
    const ATCMD_HELP_CMD_VAR *pVar;

    if (varIdx >= pCmdDesc->numVars)
    {
        return false;
    }

    pVar = &pCmdDesc->vars[varIdx];

    if (numParams != pVar->numParams)
    {
        return false;
    }

    for (i=0; i<numParams; i++)
    {
        if (pVar->pParams[i]->typeClass == ATCMD_PARAM_TYPE_CLASS_ANY)
            continue;

        if (pVar->pParams[i]->typeClass == ATCMD_PARAM_TYPE_CLASS_INTEGER)
        {
            if (ATCMD_PARAM_TYPE_INTEGER != pParamList[i].type)
            {
                return false;
            }
        }
        else if (pVar->pParams[i]->typeClass == ATCMD_PARAM_TYPE_CLASS_STRING)
        {
            if ((ATCMD_PARAM_TYPE_ASCII_STRING != pParamList[i].type) && (ATCMD_PARAM_TYPE_HEX_STRING != pParamList[i].type))
            {
                return false;
            }
        }
    }

    return true;
}
