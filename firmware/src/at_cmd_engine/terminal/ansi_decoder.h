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

#ifndef _ANSI_DECODER_H
#define _ANSI_DECODER_H

#ifdef __cplusplus  // Provide C++ Compatibility
extern "C" {
#endif

typedef enum
{
    ANSI_STREAM_DECISION_INIT,
    ANSI_STREAM_DECISION_UNKNOWN,
    ANSI_STREAM_DECISION_ALPHA_NUMERIC,
    ANSI_STREAM_DECISION_PUNCT,
    ANSI_STREAM_DECISION_CONTROL,
    ANSI_STREAM_DECISION_CONTROL_BS,
    ANSI_STREAM_DECISION_CONTROL_LF,
    ANSI_STREAM_DECISION_CONTROL_CR,
    ANSI_STREAM_DECISION_CONTROL_DC2,
    ANSI_STREAM_DECISION_CONTROL_ESC,
    ANSI_STREAM_DECISION_EXTENDED,
    ANSI_STREAM_DECISION_ESCAPE_CSI,
    ANSI_STREAM_DECISION_ESCAPE_CSI_KEYUP,
    ANSI_STREAM_DECISION_ESCAPE_CSI_KEYDOWN,
    ANSI_STREAM_DECISION_ESCAPE_CSI_F1,
    ANSI_STREAM_DECISION_ESCAPE_SS3
} ANSI_STREAM_DECISION;

typedef enum
{
    ANSI_ESCAPE_STATE_ESCAPE,
    ANSI_ESCAPE_STATE_CSI_ENTRY,
    ANSI_ESCAPE_STATE_CSI_PARAM,
    ANSI_ESCAPE_STATE_CSI_IGNORE,
    ANSI_ESCAPE_STATE_CSI_INTER,
    ANSI_ESCAPE_STATE_SS3,
    ANSI_ESCAPE_STATE_GROUND
} ANSI_ESCAPE_STATE;

typedef enum
{
    ANSI_DECODE_STATUS_OK           = 0,
    ANSI_DECODE_STATUS_ERR_NO_SPACE = -1
} ANSI_DECODE_STATUS;

typedef struct
{
    ANSI_STREAM_DECISION    decision;
    ANSI_ESCAPE_STATE       escapeState;
    char                    escapeCSIParam[16];
    int                     escapeCSIParamLength;
    char                    escapeCSICollect[16];
    int                     escapeCSICollectLength;
    char                    escapeCSIFinalByte;
    char                    escapeSS3Byte;
    bool                    escapeDispatched;
} ANSI_STREAM_DECODER_STATE;

void TP_ANSIStreamDecoderInit(ANSI_STREAM_DECODER_STATE *const pStreamDecoder);
ANSI_STREAM_DECISION TP_ANSIStreamDecoderDecision(ANSI_STREAM_DECODER_STATE *const pStreamDecoder);
ANSI_DECODE_STATUS TP_ANSIStreamDecoder(ANSI_STREAM_DECODER_STATE *const pStreamDecoder, const char c, char *pOut, size_t maxSpace);

#ifdef __cplusplus
}
#endif

#endif /* _ANSI_DECODER_H */
