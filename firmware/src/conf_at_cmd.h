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

/*
 * Support and FAQ: visit <a href="https://www.microchip.com/support/">Microchip Support</a>
 */

#ifndef _CONF_AT_CMD_H
#define _CONF_AT_CMD_H

/* The Manufacturers ID to be returned from +GMI */
#define AT_CMD_CONF_MANUFACTURER_ID             "\"Microchip\""

/* The Manufacturers ID to be returned from +GMM */
#define AT_CMD_CONF_MODEL_ID                    "\"PIC32MZW1\""

/* The Manufacturers ID to be returned from +GMR */
#define AT_CMD_CONF_REVISION_ID                 "\"0.3.1\"" // ATCMD_APPExecuteGMR()

/* Maximum length of a single AT command */
//#define AT_CMD_CONF_MAX_COMMAND_LENGTH          1024

/* Maximum length of a formatting print string */
//#define AT_CMD_CONF_PRINTF_OUT_BUF_SIZE          256

/* Character to display first in command mode as a user prompt */
//#define AT_CMD_CONF_CMD_MODE_PROMPT_CHAR        '>'

/* Character to display first in binary mode as a user prompt */
//#define AT_CMD_CONF_BIN_MODE_PROMPT_CHAR        '#'

/* Binary mode escape sequence character */
//#define AT_CMD_CONF_BIN_ESCAPE_CHAR             '+'

/* Binary mode escape sequence length */
//#define AT_CMD_CONF_BIN_ESCAPE_SEQ_LENGTH       3

/* Binary mode escape character intermediate timeout (in ms), how long after each '+' before cancelling the sequence */
//#define AT_CMD_CONF_BIN_ESCAPE_INTR_TIMEOUT     250

/* Binary mode escape sequence timeout (in ms), how long after the last '+' before exiting binary mode */
//#define AT_CMD_CONF_BIN_ESCAPE_FINAL_TIMEOUT    1000

/* Binary mode fast copy buffer size */
//#define AT_CMD_CONF_BIN_FAST_BUFFER_SIZE        16

/* Attention base string. All commands must start with this. */
//#define AT_CMD_CONF_AT_BASE_STRING              "AT"

/* Support for command history recall on ANSI terminals. */
//#define AT_CMD_CONF_TERM_CMD_HISTORY_DEPTH      8

/* Time after a solitary ESC event has been received before cancelling sequence. */
//#define AT_CMD_CONF_TP_ESC_TIMEOUT_MS           250

/* String to return when an operation was successful. */
//#define AT_CMD_CONF_AT_OK_STRING                "OK"

/* Initial base string returned when an operation encountered an error. */
//#define AT_CMD_CONF_AT_ERROR_BASE_STRING        "ERROR"

/* Initial command response verbosity level. */
//#define AT_CMD_CONF_DEFAULT_VERBOSE_LEVEL       5

/* Define the default serial baud rate. */
//#define AT_CMD_CONF_DEFAULT_SERIAL_BAUD_RATE    115200

/* Is XMODEM support included */
#define AT_CMD_INCLUDE_XMODEM_SUPPORT

/* Define XMODEM support for original checksum */
//#define AT_CMD_CONF_XMODEM_SUPPORT_CSUM

/* Define XMODEM support for XMODEM-CRC */
#define AT_CMD_CONF_XMODEM_SUPPORT_CRC

/* Define XMODEM support for XMODEM-1K */
//#define AT_CMD_CONF_XMODEM_SUPPORT_1K

/* Define XMODEM support for YMODEM */
//#define AT_CMD_CONF_XMODEM_SUPPORT_YMODEM_PROTOCOL

#include "include/conf_at_cmd_defaults.h"

#endif /* _CONF_AT_CMD_H */
