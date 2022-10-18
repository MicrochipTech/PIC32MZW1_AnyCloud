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

#ifndef _AT_CMDS_H
#define _AT_CMDS_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "conf_at_cmd.h"
#include "platform/platform.h"

#define AT_CMD_MAX_NUM_PARAMS       32
#define AT_CMD_MAX_VERBOSITY_LVL    5

typedef enum
{
    ATCMD_STATUS_PENDING    = -1,
    ATCMD_STATUS_OK         = 0,
    ATCMD_STATUS_ERROR,
    ATCMD_STATUS_INVALID_CMD,
    ATCMD_STATUS_UNKNOWN_CMD,
    ATCMD_STATUS_INVALID_PARAMETER,
    ATCMD_STATUS_INCORRECT_NUM_PARAMS,
    ATCMD_STATUS_STORE_UPDATE_BLOCKED,
    ATCMD_STATUS_STORE_ACCESS_FAILED,
    ATCMD_STATUS_CUSTOM_MSG_BASE
} ATCMD_STATUS;

typedef enum
{
    ATCMD_PARAM_TYPE_INVALID,
    ATCMD_PARAM_TYPE_INTEGER,
    ATCMD_PARAM_TYPE_ASCII_STRING,
    ATCMD_PARAM_TYPE_HEX_STRING
} ATCMD_PARAM_TYPE;

typedef enum
{
    ATCMD_PARAM_TYPE_CLASS_ANY,
    ATCMD_PARAM_TYPE_CLASS_INTEGER,
    ATCMD_PARAM_TYPE_CLASS_STRING
} ATCMD_PARAM_TYPE_CLASS;

typedef enum
{
    ATCMD_STORE_TYPE_INVALID,
    ATCMD_STORE_TYPE_INT,
    ATCMD_STORE_TYPE_STRING,
    ATCMD_STORE_TYPE_IPV4ADDR,
    ATCMD_STORE_TYPE_BOOL,
    ATCMD_STORE_TYPE_MACADDR
} ATCMD_STORE_TYPE;

typedef enum
{
    ATCMD_STORE_ACCESS_NONE     = 0,
    ATCMD_STORE_ACCESS_READ     = 1,
    ATCMD_STORE_ACCESS_WRITE    = 2,
    ATCMD_STORE_ACCESS_RW       = 3
} ATCMD_STORE_ACCESS;

typedef enum
{
    ATCMD_INT_APP_VAL_GMI,
    ATCMD_INT_APP_VAL_GMM,
    ATCMD_INT_APP_VAL_GMR,
    ATCMD_INT_APP_VAL_IPR
} ATCMD_INT_APP_VAL;

typedef union
{
    uint8_t *p;
    int32_t  i;
    uint32_t u;
} AT_CMD_PARAM_VALUE;

typedef struct
{
    AT_CMD_PARAM_VALUE  value;
    int                 length;
    ATCMD_PARAM_TYPE    type;
} ATCMD_PARAM;

typedef struct
{
    int                 id;
    int                 offset;
    ATCMD_STORE_TYPE    type;
    int                 maxSize;
    ATCMD_STORE_ACCESS  access;
    uint32_t            userType;
} ATCMD_STORE_MAP_ELEMENT;

typedef struct
{
    const char *pName;
    const char *pDescription;
} ATCMD_HELP_PARAM_OPT;

typedef struct
{
    const char              *pName;
    const char              *pDescription;
    ATCMD_PARAM_TYPE_CLASS  typeClass;
    uint8_t                 numOpts;
    ATCMD_HELP_PARAM_OPT    opts[];
} ATCMD_HELP_PARAM;

typedef struct
{
    const char *pText;
    const char *pDescription;
} ATCMD_HELP_EXAMPLE;

typedef struct
{
    uint8_t                     numParams;
    const ATCMD_HELP_PARAM      *pParams[16];
    uint8_t                     numExamples;
    const ATCMD_HELP_EXAMPLE    *pExamples[4];
} ATCMD_HELP_CMD_VAR;

typedef struct at_cmd_type_desc AT_CMD_TYPE_DESC;

typedef ATCMD_STATUS (*tpfATCMDInit)(const AT_CMD_TYPE_DESC* pCmdTypeDesc);
typedef ATCMD_STATUS (*tpfATCMDExecute)(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const int numParams, ATCMD_PARAM *pParamList);
typedef ATCMD_STATUS (*tpfATCMDUpdate)(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const AT_CMD_TYPE_DESC* pCurrentCmdTypeDesc);

struct at_cmd_type_desc
{
    const char*         pCmdName;
    tpfATCMDInit        cmdInit;
    tpfATCMDExecute     cmdExecute;
    tpfATCMDUpdate      cmdUpdate;
    const char          *pSummary;
    uint32_t            appVal;
    uint8_t             numVars;
    ATCMD_HELP_CMD_VAR  vars[];
};

typedef void (*tpfATCMDBinaryDataHandler)(const uint8_t *pBuf, size_t numBufBytes);

#ifdef __cplusplus  // Provide C++ Compatibility
extern "C" {
#endif

void ATCMD_EnterAECMode(void);
void ATCMD_LeaveAECMode(void);
void ATCMD_Print(const char *pMsg, size_t msgLength);
void ATCMD_Printf(const char *format, ...);
void ATCMD_PrintMACAddress(const uint8_t *pMACAddr);
void ATCMD_PrintIPv4Address(const uint32_t ipv4Addr);
void ATCMD_PrintStringASCIIEsc(const char *pStr, size_t strLength);
void ATCMD_PrintStringHex(const uint8_t *pBytes, size_t strLength);
void ATCMD_PrintStringSafe(const char *pStr, size_t strLength);
void ATCMD_PrintStringSafeWithDelimiterInfo(const char *pStr, size_t strLength, bool startDelimiter, bool endDelimiter);
void ATCMD_SetStatusVerbosityLevel(int newLevel);
void ATCMD_ReportStatus(const ATCMD_STATUS statusCode);
void ATCMD_ReportAECStatus(const char *pCmdName, const ATCMD_STATUS statusCode);
void ATCMD_ShowHelp(const AT_CMD_TYPE_DESC *pCmd);
bool ATCMD_CompleteCommand(ATCMD_STATUS status);
void ATCMD_Init(void);
void ATCMD_EnterBinaryMode(tpfATCMDBinaryDataHandler pBinDataHandler);
void ATCMD_LeaveBinaryMode(void);
bool ATCMD_ModeIsBinary(void);
void ATCMD_Update(int termPollRateMs);
void ATCMD_BinaryInit(void);
bool ATCMD_BinaryProcess(void);

const ATCMD_STORE_MAP_ELEMENT* ATCMD_StructStoreFindNext(const ATCMD_STORE_MAP_ELEMENT *pstaConfMap);
const ATCMD_STORE_MAP_ELEMENT* ATCMD_StructStoreFindElementByID(const ATCMD_STORE_MAP_ELEMENT *pstaConfMap, int id);
int ATCMD_StructStoreWriteParam(const ATCMD_STORE_MAP_ELEMENT *pstaConfMap, const void *pStruct, int id, ATCMD_PARAM *pParam);
bool ATCMD_StructStorePrint(const char *pATCmd, const ATCMD_STORE_MAP_ELEMENT *pstaConfMap, const void *pStruct, int id);

#ifdef __cplusplus
}
#endif

#endif /* _AT_CMDS_H */
