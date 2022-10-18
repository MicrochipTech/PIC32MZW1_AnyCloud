/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app.c

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

/*******************************************************************************
Copyright (C) 2022 released Microchip Technology Inc.  All rights reserved.

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

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include "app.h"
#include "app_commands.h"
#include <wolfssl/ssl.h>
#include <tcpip/src/hash_fnv.h>
#include "system/debug/sys_debug.h"
#include "at_cmd_app.h"

void ATCMD_Init(void);

void ATCMD_PlatformInit();
static void APP_INT_StatusClear()
{
    SYS_INT_SourceStatusClear(INT_SOURCE_TIMER_2);
    SYS_INT_SourceStatusClear(INT_SOURCE_TIMER_3);

    SYS_INT_SourceStatusClear(INT_SOURCE_UART1_FAULT);
    SYS_INT_SourceStatusClear(INT_SOURCE_UART1_TX);
    SYS_INT_SourceStatusClear(INT_SOURCE_UART1_RX);

    SYS_INT_SourceStatusClear(INT_SOURCE_I2C1_MASTER);
    SYS_INT_SourceStatusClear(INT_SOURCE_I2C1_BUS);

    SYS_INT_SourceStatusClear(INT_SOURCE_RFMAC);
    SYS_INT_SourceStatusClear(INT_SOURCE_RFSMC);
    SYS_INT_SourceStatusClear(INT_SOURCE_RFTM0);

    SYS_INT_SourceStatusClear(INT_SOURCE_CRYPTO1);
    SYS_INT_SourceStatusClear(INT_SOURCE_CRYPTO1_FAULT);

    SYS_INT_SourceStatusClear(INT_SOURCE_FLASH_CONTROL);

#ifndef RTCC    
    SYS_INT_SourceStatusClear(INT_SOURCE_RTCC);
#endif
    
    //SYS_INT_SourceStatusClear(INT_SOURCE_EXTERNAL_0);
    //SYS_INT_SourceStatusClear(INT_SOURCE_CHANGE_NOTICE_A);
}

static void APP_INT_SourceDisable()
{
           
    SYS_INT_SourceDisable(INT_SOURCE_TIMER_2);
    SYS_INT_SourceDisable(INT_SOURCE_TIMER_3);

    SYS_INT_SourceDisable(INT_SOURCE_UART1_FAULT);
    SYS_INT_SourceDisable(INT_SOURCE_UART1_TX);
    SYS_INT_SourceDisable(INT_SOURCE_UART1_RX);

    SYS_INT_SourceDisable(INT_SOURCE_I2C1_MASTER);
    SYS_INT_SourceDisable(INT_SOURCE_I2C1_BUS);

    SYS_INT_SourceDisable(INT_SOURCE_RFMAC);
    SYS_INT_SourceDisable(INT_SOURCE_RFSMC);
    SYS_INT_SourceDisable(INT_SOURCE_RFTM0);

    SYS_INT_SourceDisable(INT_SOURCE_CRYPTO1);
    SYS_INT_SourceDisable(INT_SOURCE_CRYPTO1_FAULT);

    SYS_INT_SourceDisable(INT_SOURCE_FLASH_CONTROL);
#ifndef RTCC    
    SYS_INT_SourceDisable(INT_SOURCE_RTCC);
#endif    
    //SYS_INT_SourceDisable(INT_SOURCE_EXTERNAL_0);
    //SYS_INT_SourceDisable(INT_SOURCE_CHANGE_NOTICE_A);
    
}

static void APP_INT_SourceRestore()
{

    SYS_INT_SourceRestore(INT_SOURCE_TIMER_2, true);
    SYS_INT_SourceRestore(INT_SOURCE_TIMER_3, true);

    SYS_INT_SourceRestore(INT_SOURCE_UART1_FAULT, true);
    SYS_INT_SourceRestore(INT_SOURCE_UART1_TX, true);
    SYS_INT_SourceRestore(INT_SOURCE_UART1_RX, true);

    SYS_INT_SourceRestore(INT_SOURCE_I2C1_MASTER, true);
    SYS_INT_SourceRestore(INT_SOURCE_I2C1_BUS, true);

    SYS_INT_SourceRestore(INT_SOURCE_RFMAC, true);
    SYS_INT_SourceRestore(INT_SOURCE_RFSMC, true);
    SYS_INT_SourceRestore(INT_SOURCE_RFTM0, true);

    
    SYS_INT_SourceRestore(INT_SOURCE_CRYPTO1, true);
    SYS_INT_SourceRestore(INT_SOURCE_CRYPTO1_FAULT, true);

    SYS_INT_SourceRestore(INT_SOURCE_FLASH_CONTROL, true);
#ifndef RTCC    
    SYS_INT_SourceRestore(INT_SOURCE_RTCC, true);
#endif    

    //SYS_INT_SourceRestore(INT_SOURCE_EXTERNAL_0, true);
    //SYS_INT_SourceRestore(INT_SOURCE_CHANGE_NOTICE_A, true);
}

#define RTCC

#ifdef RTCC
struct tm sys_time;
struct tm alarm_time;
volatile bool rtcc_alarm = false;

//#define APP_CFG_WITH_MQTT_API
void RTCC_Callback( uintptr_t context)
{

    rtcc_alarm = true;    
}
#endif

extern ATCMD_APP_CONTEXT atCmdAppContext;

void APP_SetSleepMode(uint8_t val)
{
    /* Set RTCC alarm to 10 seconds */
    if (RTCC_AlarmSet(&alarm_time, RTCC_ALARM_MASK_SS/*RTCC_ALARM_MASK_HHMISS*/) == false) {
        SYS_CONSOLE_PRINT("Error setting alarm\r\n");
        return;
    }
    
    switch (val) {
        case 0:     /* PIC IDLE + Wi-Fi WSM ON*/
        case 1:     /* PIC IDLE + Wi-Fi WOFF*/
        case 2:     /* PIC SLEEP + Wi-Fi WSM ON*/
        case 3:     /* PIC SLEEP + Wi-Fi WOFF*/
            if(atCmdAppContext.appState == ATCMD_APP_STATE_STA_CONNECTED){
                if(val ==1 || val ==3)
                    PMUCLKCTRLbits.WLDOOFF = 1;
                
                APP_INT_StatusClear();
                APP_INT_SourceDisable();
                if(val == 0 || val ==1)
                    POWER_LowPowerModeEnter(LOW_POWER_IDLE_MODE);
                else
                    POWER_LowPowerModeEnter(LOW_POWER_SLEEP_MODE);
                APP_INT_SourceRestore();
                
                if(val ==1 || val ==3){
#if 0					
					WDRV_PIC32MZW_SYS_INIT wdrvPIC32MZW1InitData = {
					 .pCryptRngCtx	= NULL,
					 .pRegDomName	= "GEN",
					 .powerSaveMode = WDRV_PIC32MZW_POWERSAVE_RUN_MODE,
					 .powerSavePICCorrelation = WDRV_PIC32MZW_POWERSAVE_PIC_ASYNC_MODE
					 };
#endif
					SYS_CONSOLE_PRINT("Waking up from sleep\n");
                    WDRV_PIC32MZW_Deinitialize(sysObj.drvWifiPIC32MZW1);
//					PMUCLKCTRLbits.WLDOOFF = 0;
//					sysObj.drvWifiPIC32MZW1 = WDRV_PIC32MZW_Initialize(WDRV_PIC32MZW_SYS_IDX_0, (SYS_MODULE_INIT*)&wdrvPIC32MZW1InitData);
//                    appData.wOffRequested = true;
                }
                break;
            }
            SYS_CONSOLE_PRINT("STA is not connected\r\n");
            break;
            
        /* PIC DS or XDS mode */
        case 4:
            PMD3bits.W24GMD = 1;
            POWER_LowPowerModeEnter(LOW_POWER_DEEP_SLEEP_MODE);
            break;
            
        /* Wi-Fi WON */
        case 5:
#if 0			
            if(!MQTT_IS_CONNECTED){
                appData.wOnRequested = true;
                break;
            }
#endif			
            SYS_CONSOLE_PRINT(" MQTT is connected\r\n");
            break;
        
        default:
            SYS_CONSOLE_PRINT("Invalid PM configuration\r\n");
            break;
    }
}

void ATCMD_Init(void);

void APP_Initialize(void) {
//    APP_Commands_Init();
	RCON_RESET_CAUSE resetCause;

//	ATCMD_Init();

	resetCause = RCON_ResetCauseGet();

	POWER_ReleaseGPIO();

	/* Check if RESET was after deep sleep wakeup */
	if (((resetCause & RCON_RESET_CAUSE_DPSLP) == RCON_RESET_CAUSE_DPSLP))
	{
		RCON_ResetCauseClear(RCON_RESET_CAUSE_DPSLP);
	}

//	if (POWER_WakeupSourceGet() == POWER_WAKEUP_SOURCE_DSRTC)
//	{
//		SYS_CONSOLE_PRINT("\r\n\r\nDevice woke up after XDS/DS mode Using RTCC\r\n");
//	}
//	else if (POWER_WakeupSourceGet() == POWER_WAKEUP_SOURCE_DSINT0)
//	{
//		SYS_CONSOLE_PRINT("\r\n\r\nDevice woke up after XDS/DS mode Using EXT INT0(SW1 button press)\r\n");
//	}
 
	/* INT0 interrupt is used to wake up from Deep Sleep */
	EVIC_ExternalInterruptEnable(EXTERNAL_INT_0);
	//EVIC_ExternalInterruptEnable(INT_SOURCE_RFSMC);

	/* CN interrupt is used to wake up from Idle or Sleep mode */
    GPIO_PinInterruptEnable(CN_INT_PIN);
	//GPIO_PinInterruptEnable(INT_SOURCE_RFSMC);

#ifdef RTCC   

    // Time setting 31-12-2018 23:59:55 Monday
    sys_time.tm_hour = 23;
    sys_time.tm_min = 59;
    sys_time.tm_sec = 55;

    sys_time.tm_year = 18;
    sys_time.tm_mon = 12;
    sys_time.tm_mday = 31;
    sys_time.tm_wday = 1;

    // Alarm setting 01-01-2019 00:00:05 Tuesday
    alarm_time.tm_hour = 00;
    alarm_time.tm_min = 00;
    alarm_time.tm_sec = 55;

    alarm_time.tm_year = 19;
    alarm_time.tm_mon = 01;
    alarm_time.tm_mday = 01;
    alarm_time.tm_wday = 2;

    RTCC_CallbackRegister(RTCC_Callback, (uintptr_t) NULL);
            
    if (RTCC_TimeSet(&sys_time) == false)
    {
        /* Error setting up time */
        while(1);
    }

    if (RTCC_AlarmSet(&alarm_time, RTCC_ALARM_MASK_SS /*RTCC_ALARM_MASK_HHMISS*/) == false)
    {
        /* Error setting up alarm */
        while(1);
    }

#endif    

}

void ATCMD_Update(int termPollRateMs);

extern SYS_MODULE_OBJ netSrvcHdl;
extern SYS_MODULE_OBJ netSrvcHdl;
extern uint8_t response_buffer[];
extern int32_t response_buffer_length;

void APP_Tasks(void) {
    static int8_t atcmd_initialize = 0;
    
    if ((atcmd_initialize == 0) && (SYS_STATUS_READY == WDRV_PIC32MZW_Status(sysObj.drvWifiPIC32MZW1)))
    {
        atcmd_initialize = 1;
        ATCMD_Init();
    }

   if (atcmd_initialize == 0)
        return;
 
    ATCMD_Update(50);

    if(atCmdAppContext.respond_to_app == 2)
    {
        atCmdAppContext.respond_to_app = 0;

        SYS_NET_SendMsg(netSrvcHdl, response_buffer, response_buffer_length);
    }
    return;
}

/*******************************************************************************
 End of File
 */

