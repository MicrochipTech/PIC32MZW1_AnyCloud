/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    mqtt_app.c

  Summary:
    This file contains the source code for the MPLAB Harmony application.

  Description:
    This file contains the source code for the MPLAB Harmony application.  It
    implements the logic of the application's state machine and it may call
    API routines of other MPLAB Harmony modules in the system, such as drivers,
    system services, and middleware.  However, it does not call any of the
    system interfaces (such as the "Initialize" and "Tasks" functions) of any of
    the modules in the system or make any assumptions about when those functions
    are called.  That is the responsibility of the configuration-specific system
    files.
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


// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <string.h>

#include "mqtt_app.h"
#include "system/command/sys_command.h"
#include "app_control.h"
#include "cJSON.h"
#include "system/mqtt/sys_mqtt.h"
#include "bsp/bsp.h"

MQTT_APP_DATA mqtt_appData;

#ifdef RN_MODE_DISABLED
int32_t MqttCallback(SYS_MQTT_EVENT_TYPE eEventType, void *data, uint16_t len, void* cookie) {
    static int errorCount = 0;
    switch (eEventType) {
        case SYS_MQTT_EVENT_MSG_RCVD:
        {
            SYS_MQTT_PublishConfig *psMsg = (SYS_MQTT_PublishConfig *) data;
            psMsg->message[psMsg->messageLength] = 0;
            psMsg->topicName[psMsg->topicLength] = 0;
            //SYS_CONSOLE_PRINT("\nMqttCallback(): Msg received on Topic: %s ; Msg: %s\r\n",psMsg->topicName, psMsg->message);

            if (NULL != strstr((char*) psMsg->topicName, "/shadow/update/delta")) {
                cJSON *messageJson = cJSON_Parse((char*) psMsg->message);
                if (messageJson == NULL) {
                    const char *error_ptr = cJSON_GetErrorPtr();
                    if (error_ptr != NULL) {
                        SYS_CONSOLE_PRINT(TERM_RED"Message JSON parse Error. Error before: %s\n"TERM_RESET, error_ptr);
                    }
                    cJSON_Delete(messageJson);
                    break;
                }

                //Get the desired state
                cJSON *state = cJSON_GetObjectItem(messageJson, "state");
                if (!state) {
                    cJSON_Delete(messageJson);
                    break;
                }

                //Get the toggle state
                cJSON *toggle = cJSON_GetObjectItem(state, "toggle");
                if (!state) {
                    cJSON_Delete(messageJson);
                    break;
                }

                bool desiredState = (bool) toggle->valueint;
                if (desiredState) {
                    LED_GREEN_On();
                    SYS_CONSOLE_PRINT(TERM_GREEN"LED ON\r\n"TERM_RESET);
                } else {
                    LED_GREEN_Off();
                    SYS_CONSOLE_PRINT(TERM_YELLOW"LED OFF\r\n"TERM_RESET);
                }
                cJSON_Delete(messageJson);
#if 0
                if (NULL != strstr((char*) psMsg->message, "\"state\":{\"toggle\":1}")) {
                    LED_GREEN_On();
                    SYS_CONSOLE_PRINT(TERM_GREEN"LED ON"TERM_RESET);
                } else if (NULL != strstr((char*) psMsg->message, "\"state\":{\"toggle\":0}")) {
                    LED_GREEN_Off();
                    SYS_CONSOLE_PRINT(TERM_GREEN"LED OFF"TERM_RESET);
                }
#endif
                mqtt_appData.shadowUpdate = true;
                mqtt_appData.pubFlag = true;
            }
        }
            break;

        case SYS_MQTT_EVENT_MSG_DISCONNECTED:
        {
            SYS_CONSOLE_PRINT("\nMqttCallback(): MQTT Disconnected\r\n");
            mqtt_appData.MQTTConnected = false;
            app_controlData.mqttCtrl.conStat = false;
        }
            break;

        case SYS_MQTT_EVENT_MSG_CONNECTED:
        {
            SYS_CONSOLE_PRINT("\nMqttCallback(): MQTT Connected\r\n");
            mqtt_appData.MQTTConnected = true;
            app_controlData.mqttCtrl.conStat = true;
        }
            break;

        case SYS_MQTT_EVENT_MSG_SUBSCRIBED:
        {
            SYS_MQTT_SubscribeConfig *psMqttSubCfg = (SYS_MQTT_SubscribeConfig *) data;
            SYS_CONSOLE_PRINT("\nMqttCallback(): Subscribed to Topic '%s'\r\n", psMqttSubCfg->topicName);
        }
            break;

        case SYS_MQTT_EVENT_MSG_UNSUBSCRIBED:
        {
            SYS_MQTT_SubscribeConfig *psMqttSubCfg = (SYS_MQTT_SubscribeConfig *) data;
            SYS_CONSOLE_PRINT("\nMqttCallback(): UnSubscribed to Topic '%s'\r\n", psMqttSubCfg->topicName);
        }
            break;

        case SYS_MQTT_EVENT_MSG_PUBLISHED:
        {
            //SYS_CONSOLE_PRINT("\nMqttCallback(): Published Sensor Data\r\n");
            mqtt_appData.MQTTPubQueued = false;
            errorCount = 0;
        }
            break;
        case SYS_MQTT_EVENT_MSG_CONNACK_TO:
        {
            SYS_CONSOLE_PRINT("\nMqttCallback(): CONNACK Timed out\r\n");

        }
            break;
        case SYS_MQTT_EVENT_MSG_SUBACK_TO:
        {
            SYS_CONSOLE_PRINT("\nMqttCallback(): SUBACK Timed out\r\n");

        }
            break;
        case SYS_MQTT_EVENT_MSG_PUBACK_TO:
        {
            SYS_CONSOLE_PRINT("\nMqttCallback(): PUBACK Timed out. non-Fatal error.\r\n");
            mqtt_appData.MQTTPubQueued = false;
            errorCount++;
            if (errorCount > 5) {
                SYS_CONSOLE_PRINT(TERM_RED"\nMqttCallback(): Too many failed events. Forcing a reset\r\n"TERM_RESET);
            }
            while (1); //force a WDT reset
        }
            break;
        case SYS_MQTT_EVENT_MSG_UNSUBACK_TO:
        {
            SYS_CONSOLE_PRINT("\nMqttCallback(): UNSUBACK Timed out\r\n");

        }
            break;
    }
    return SYS_MQTT_SUCCESS;
}

#endif

void MQTT_APP_Initialize(void) {
    mqtt_appData.SysMqttHandle = SYS_MODULE_OBJ_INVALID;
    mqtt_appData.state = MQTT_APP_STATE_SERVICE_TASKS;
}

void MQTT_APP_Tasks(void) {	
    switch (mqtt_appData.state) {
        case MQTT_APP_STATE_SERVICE_TASKS:
        {
            SYS_MQTT_Task(mqtt_appData.SysMqttHandle);
            break;
        }
        default:
        {
            break;
        }
    }
}


/*******************************************************************************
 End of File
 */
