/*******************************************************************************
  MPLAB Harmony Application Header File

  Company:
    Microchip Technology Inc.

  File Name:
    msd_app.h

  Summary:
    This header file provides prototypes and definitions for the application.

  Description:
    This header file provides function prototypes and data type definitions for
    the application.  Some of these are required by the system (such as the
    "MSD_APP_Initialize" and "MSD_APP_Tasks" prototypes) and some of them are only used
    internally by the application (such as the "MSD_APP_STATES" definition).  Both
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

#ifndef _MSD_APP_H
#define _MSD_APP_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include "configuration.h"
#include "usb/usb_chapter_9.h"
#include "usb/usb_device.h"
#include "system/fs/sys_fs.h"
#include "app_wifi.h"

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

extern "C" {

#endif
    
#define MSD_APP_FS_FORMA_BUFFER_SIZE 512
    
#define MSD_APP_TXT_CONFIG
    
#define MSD_APP_VERSION_FILE_NAME "version.txt"    
#define MSD_APP_SEC_DIR_NAME "sec"
#define MSD_APP_ERR_FILE_NAME "ERROR.txt"
#define MSD_APP_SERIAL_FILE_NAME "serial.txt"
#define MSD_APP_CLICKME_FILE_NAME "clickme.html"
#define MSD_APP_VOICE_CLICKME_FILE_NAME "voice.html"
#define MSD_APP_CLOUD_CONFIG_FILE_NAME "cloud.json"
#define MSD_APP_SIGNER_FILE_NAME MSD_APP_SEC_DIR_NAME"/signerCert.der"
#define MSD_APP_DEVCERT_FILE_NAME MSD_APP_SEC_DIR_NAME"/deviceCert.der"
#define MSD_APP_ROOTCERT_FILE_NAME MSD_APP_SEC_DIR_NAME"/rootCert.der"
#define MSD_APP_SLOT_0_PUBKEY_FILE_NAME MSD_APP_SEC_DIR_NAME"/slot0.key"
#define MSD_APP_SLOT_1_PUBKEY_FILE_NAME MSD_APP_SEC_DIR_NAME"/slot1.key"
#define MSD_APP_SLOT_2_PUBKEY_FILE_NAME MSD_APP_SEC_DIR_NAME"/slot2.key"
#define MSD_APP_SLOT_3_PUBKEY_FILE_NAME MSD_APP_SEC_DIR_NAME"/slot3.key"
#define MSD_APP_SLOT_4_PUBKEY_FILE_NAME MSD_APP_SEC_DIR_NAME"/slot4.key"
#define MSD_APP_ROOT_PUBKEY_FILE_NAME MSD_APP_SEC_DIR_NAME"/root.key"
#define MSD_APP_SIGNER_PUBKEY_FILE_NAME MSD_APP_SEC_DIR_NAME"/signer.key"
#define MSD_APP_DEVICE_PUBKEY_FILE_NAME MSD_APP_SEC_DIR_NAME"/device.key"

#define APP_CTRL_CLIENTID_SIZE ((2 * KEYID_SIZE) + 1)

#if !SYS_FS_AUTOMOUNT_ENABLE
    #define SYS_FS_MEDIA_IDX0_MOUNT_NAME_VOLUME_IDX0 			"/mnt/myDrive1"
    #define SYS_FS_MEDIA_IDX0_DEVICE_NAME_VOLUME_IDX0			"/dev/mtda1"
#endif
    
#ifdef MSD_APP_TXT_CONFIG
#define MSD_APP_TXT_CONFIG_FILE_NAME "WIFI.CFG"
#define MSD_APP_TXT_CONFIG_DATA "CMD:SEND_UART=wifi "DEFAULT_SSID","DEFAULT_SSID_PSK","DEFAULT_AUTH_MODE_NUM
#endif
    
#define MSD_APP_CLICKME_DATA_TEMPLATE "<html><body><script type=\"text/javascript\">window.location.href =\"\
                              https://pic-iot.com/pic32mzw1/aws/%s\";</script></body></html>"

#define MSD_APP_VOICE_CLICKME_DATA_TEMPLATE "<html><body><script type=\"text/javascript\">window.location.href =\"\
                              https://microchiptech.github.io/mchpiotvoice?thingName=%s&boardType=w1Curiosity\";</script></body></html>"

#define MSD_APP_CLOUD_CONFIG_DATA_TEMPLATE "{\r\n\"broker\":\"%s\",\r\n\"clientID\":\"%s\"\r\n}"
    
    typedef enum {
        /* Application's state machine's initial state. */
        MSD_APP_STATE_INIT = 0,
        MSD_APP_STATE_RUNNING,
        MSD_APP_STATE_TOUCH_FILE,
        MSD_APP_STATE_CLEAR_DRIVE,
        MSD_APP_STATE_WAIT_FS_MOUNT,
        MSD_APP_CONNECT_USB,
        MSD_APP_STATE_ERROR,
    } MSD_APP_STATES;

    typedef struct {
        /* The application's current state */
        MSD_APP_STATES state;

        /* USB Device Handle */
        USB_DEVICE_HANDLE usbDeviceHandle;

        /* SYS_FS File handle */
        SYS_FS_HANDLE fileHandle;
        SYS_FS_FSTAT fileStatus;
        volatile bool fsMounted;
        SYS_FS_TIME fileTime_old, fileTime_new;
        bool checkHash;
        unsigned char configHash[32];
    } MSD_APP_DATA;

    void MSD_APP_Initialize(void);

    void MSD_APP_Tasks(void);

    int MSD_APP_Read_RootCert(uint8_t *rootCert, size_t *pRootCertSize);
    int MSD_APP_Read_SignerCert(uint8_t *signerCert, size_t *pSignerCertSize);
    int MSD_APP_Read_DeviceCert(uint8_t *deviceCert, size_t *pDeviceCertSize);
    int MSD_APP_Read_DeviceCertPem(uint8_t *deviceCertPem, size_t *pDeviceCertPemSize);
    
#endif /* _MSD_APP_H */

    //DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END
