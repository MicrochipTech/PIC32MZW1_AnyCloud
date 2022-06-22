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

/*******************************************************************************
* Command interface prototypes
*******************************************************************************/
static ATCMD_STATUS _INFOExecute(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const int numParams, ATCMD_PARAM *pParamList);

/*******************************************************************************
* Command parameters
*******************************************************************************/
static const ATCMD_HELP_PARAM paramInfoType =
    {"TYPE", "Type of information", ATCMD_PARAM_TYPE_CLASS_INTEGER,
        .numOpts = 1,
        {
            {"1", "Task Report"}
        }
    };

/*******************************************************************************
* Command examples
*******************************************************************************/

/*******************************************************************************
* Command descriptors
*******************************************************************************/
const AT_CMD_TYPE_DESC atCmdTypeDescINFO =
    {
        .pCmdName   = "+INFO",
        .cmdInit    = NULL,
        .cmdExecute = _INFOExecute,
        .cmdUpdate  = NULL,
        .pSummary   = "This command is used to report system information",
        .numVars    = 1,
        {
            {
                .numParams   = 1,
                .pParams     =
                {
                    &paramInfoType
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

/*******************************************************************************
* Local defines and types
*******************************************************************************/

/*******************************************************************************
* Local data
*******************************************************************************/

/*******************************************************************************
* Local functions
*******************************************************************************/
static bool _INFOReport01(void)
{
#ifdef ATCMD_APP_OS_FREERTOS
    TaskStatus_t *pTaskStatusArray;
    int numTasks, x;

    numTasks = uxTaskGetNumberOfTasks();

    pTaskStatusArray = OSAL_Malloc(numTasks * sizeof(TaskStatus_t));
   
    if (NULL == pTaskStatusArray)
    {
        return false;
    }

    numTasks = uxTaskGetSystemState(pTaskStatusArray, numTasks, NULL);

    for(x=0; x<numTasks; x++)
    {
        ATCMD_Printf("%s%-*c%5d  %d\r\n", pTaskStatusArray[x].pcTaskName, (configMAX_TASK_NAME_LEN+1)-strlen(pTaskStatusArray[x].pcTaskName), ' ', pTaskStatusArray[x].usStackHighWaterMark, pTaskStatusArray[x].eCurrentState);
    }

    OSAL_Free(pTaskStatusArray);

    return true;
#else
    return false;
#endif
}

/*******************************************************************************
* Command init functions
*******************************************************************************/

/*******************************************************************************
* Command execute functions
*******************************************************************************/
static ATCMD_STATUS _INFOExecute(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const int numParams, ATCMD_PARAM *pParamList)
{
    if (1 == numParams)
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
    
    switch (pParamList[0].value.i)
    {
        case 1:
        {
            if (false == _INFOReport01())
            {
                return ATCMD_STATUS_ERROR;
            }
            
            break;
        }
        
        default:
        {
            return ATCMD_STATUS_INVALID_PARAMETER;
        }
    }

    return ATCMD_STATUS_OK;
}

/*******************************************************************************
* Command update functions
*******************************************************************************/
