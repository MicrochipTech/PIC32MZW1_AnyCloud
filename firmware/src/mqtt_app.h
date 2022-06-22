/*******************************************************************************
  MPLAB Harmony Application Header File

  Company:
    Microchip Technology Inc.

  File Name:
    mqtt_app.h

  Summary:
    This header file provides prototypes and definitions for the application.

  Description:
    This header file provides function prototypes and data type definitions for
    the application.  Some of these are required by the system (such as the
    "MQTT_APP_Initialize" and "MQTT_APP_Tasks" prototypes) and some of them are only used
    internally by the application (such as the "MQTT_APP_STATES" definition).  Both
    are defined here for convenience.
*******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
Copyright (c) 2022 released Microchip Technology Inc.  All rights reserved.

Microchip licenses to you the right to use, modify, copy and distribute
Software only when embedded on a Microchip microcontroller or digital signal
controller that is integrated into your product or third party product
(pursuant to the sublicense terms in the accompanying license agreement).

You should refer to the license agreement accompanying this Software for
additional information regarding your rights and obligations.

SOFTWARE AND DOCUMENTATION ARE PROVIDED AS IS WITHOUT WARRANTY OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF
MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE.
IN NO EVENT SHALL MICROCHIP OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER
CONTRACT, NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR
OTHER LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE OR
CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT OF
SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
(INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.
 *******************************************************************************/
// DOM-IGNORE-END

#ifndef _MQTT_APP_H
#define _MQTT_APP_H

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include "configuration.h"
#include "FreeRTOS.h"
#include "task.h"
#include "system_module.h"

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

extern "C" {

#endif
// DOM-IGNORE-END

#define MQTT_APP_TOPIC_NAME_MAX_LEN 161
#define MQTT_APP_TELEMETRY_MSG_TEMPLATE "{\"Temperature (C)\": %d}"
#define MQTT_APP_TELEMETRY_MSG_GRAD_TEMPLATE "{\"Temperature (C)\": %d,\"switch\":%d}"
#define MQTT_APP_SHADOW_MSG_TEMPLATE "{\"state\":{\"reported\":{\"toggle\": %d}}}"
#define MQTT_APP_MAX_MSG_LLENGTH 64
#define MQTT_APP_SHADOW_UPDATE_TOPIC_TEMPLATE "$aws/things/%s/shadow/update"
/*Subscribe to wildcard topic (update/#) to enable AWS qualification log collection*/
#define MQTT_APP_SHADOW_DELTA_TOPIC_TEMPLATE "$aws/things/%s/shadow/update/delta" 

typedef enum
{
    MQTT_APP_STATE_INIT=0,
    MQTT_APP_STATE_SERVICE_TASKS,
} MQTT_APP_STATES;

typedef struct
{
    MQTT_APP_STATES state;
    SYS_MODULE_OBJ      SysMqttHandle;
} MQTT_APP_DATA;

void MQTT_APP_Initialize ( void );

void MQTT_APP_Tasks( void );



#endif /* _MQTT_APP_H */

//DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

/*******************************************************************************
 End of File
 */

