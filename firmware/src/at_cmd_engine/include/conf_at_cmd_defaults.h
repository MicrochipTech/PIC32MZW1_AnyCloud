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

#ifndef _CONF_AT_CMD_DEFAULTS_H
#define _CONF_AT_CMD_DEFAULTS_H

/* Maximum length of a single AT command */
#ifndef AT_CMD_CONF_MAX_COMMAND_LENGTH
#define AT_CMD_CONF_MAX_COMMAND_LENGTH          256
#endif

/* Maximum length of a formatting print string */
#ifndef AT_CMD_CONF_PRINTF_OUT_BUF_SIZE
#define AT_CMD_CONF_PRINTF_OUT_BUF_SIZE         1500
#endif

/* Character to display first in command mode as a user prompt */
#ifndef AT_CMD_CONF_CMD_MODE_PROMPT_CHAR
#define AT_CMD_CONF_CMD_MODE_PROMPT_CHAR        '>'
#endif

/* Character to display first in binary mode as a user prompt */
#ifndef AT_CMD_CONF_BIN_MODE_PROMPT_CHAR
#define AT_CMD_CONF_BIN_MODE_PROMPT_CHAR        '#'
#endif

/* Binary mode escape sequence character */
#ifndef AT_CMD_CONF_BIN_ESCAPE_CHAR
#define AT_CMD_CONF_BIN_ESCAPE_CHAR             '+'
#endif

/* Binary mode escape sequence length */
#ifndef AT_CMD_CONF_BIN_ESCAPE_SEQ_LENGTH
#define AT_CMD_CONF_BIN_ESCAPE_SEQ_LENGTH       3
#endif

/* Binary mode escape character intermediate timeout (in ms), how long after each '+' before cancelling the sequence */
#ifndef AT_CMD_CONF_BIN_ESCAPE_INTR_TIMEOUT
#define AT_CMD_CONF_BIN_ESCAPE_INTR_TIMEOUT     250
#endif

/* Binary mode escape sequence timeout (in ms), how long after the last '+' before exiting binary mode */
#ifndef AT_CMD_CONF_BIN_ESCAPE_FINAL_TIMEOUT
#define AT_CMD_CONF_BIN_ESCAPE_FINAL_TIMEOUT    1000
#endif

/* Binary mode fast copy buffer size */
#ifndef AT_CMD_CONF_BIN_FAST_BUFFER_SIZE
#define AT_CMD_CONF_BIN_FAST_BUFFER_SIZE        16
#endif
#define AT_CMD_CONF_BIN_MAX_BUFFER_SIZE         1400
        
/* Attention base string. All commands must start with this. */
#ifndef AT_CMD_CONF_AT_BASE_STRING
#define AT_CMD_CONF_AT_BASE_STRING              "AT"
#endif

/* Support for command history recall on ANSI terminals. */
#ifndef AT_CMD_CONF_TERM_CMD_HISTORY_DEPTH
#define AT_CMD_CONF_TERM_CMD_HISTORY_DEPTH      8
#endif

/* Time after a solitary ESC event has been received before cancelling sequence. */
#ifndef AT_CMD_CONF_TP_ESC_TIMEOUT_MS
#define AT_CMD_CONF_TP_ESC_TIMEOUT_MS           250
#endif

/* String to return when an operation was successful. */
#ifndef AT_CMD_CONF_AT_OK_STRING
#define AT_CMD_CONF_AT_OK_STRING                "OK"
#endif

/* Initial base string returned when an operation encountered an error. */
#ifndef AT_CMD_CONF_AT_ERROR_BASE_STRING
#define AT_CMD_CONF_AT_ERROR_BASE_STRING        "ERROR"
#endif

/* Initial command response verbosity level. */
#ifndef AT_CMD_CONF_DEFAULT_VERBOSE_LEVEL
#define AT_CMD_CONF_DEFAULT_VERBOSE_LEVEL       5
#endif

/* Define the default serial baud rate. */
#ifndef AT_CMD_CONF_DEFAULT_SERIAL_BAUD_RATE
#define AT_CMD_CONF_DEFAULT_SERIAL_BAUD_RATE    115200
#endif

/*--------------------------------------------------------------------------------------*/

#if (AT_CMD_CONF_CMD_MODE_PROMPT_CHAR + 0)
#define AT_CMD_CONF_CMD_MODE_USE_PROMPT
#endif

#if (AT_CMD_CONF_BIN_MODE_PROMPT_CHAR + 0)
#define AT_CMD_CONF_BIN_MODE_USE_PROMPT
#endif

#define AT_CMD_CONF_AT_BASE_STRING_SZ           (sizeof(AT_CMD_CONF_AT_BASE_STRING)-1)

#if AT_CMD_CONF_TERM_CMD_HISTORY_DEPTH > 0
#define TP_HISTORY_ENABLED
#endif

#endif /* _CONF_AT_CMD_DEFAULTS_H */
