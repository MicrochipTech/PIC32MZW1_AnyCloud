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
#include <stddef.h>

#include "ansi_decoder.h"

/*****************************************************************************
 * Transition to a new state
 *****************************************************************************/
static void _EscapeEnterState(ANSI_STREAM_DECODER_STATE *const pStreamDecoder, ANSI_ESCAPE_STATE state)
{
    switch (state)
    {
        case ANSI_ESCAPE_STATE_ESCAPE:
        case ANSI_ESCAPE_STATE_CSI_ENTRY:
        {
            pStreamDecoder->escapeCSICollectLength = 0;
            pStreamDecoder->escapeCSIParamLength   = 0;
            pStreamDecoder->escapeSS3Byte          = 0;
            pStreamDecoder->escapeDispatched       = false;
            break;
        }

        case ANSI_ESCAPE_STATE_SS3:
        {
            pStreamDecoder->escapeSS3Byte = 0;
            break;
        }

        default:
        {
            break;
        }
    }

    pStreamDecoder->escapeState = state;
}

/*****************************************************************************
 * Collect data while in CSI mode
 *****************************************************************************/
static void _EscapeCSICollect(ANSI_STREAM_DECODER_STATE *const pStreamDecoder, const char c)
{
    if (pStreamDecoder->escapeCSICollectLength < 16)
    {
        pStreamDecoder->escapeCSICollect[pStreamDecoder->escapeCSICollectLength] = c;
        pStreamDecoder->escapeCSICollectLength++;
    }
}

/*****************************************************************************
 * Collect parameter while in CSI mode
 *****************************************************************************/
static void _EscapeCSIParam(ANSI_STREAM_DECODER_STATE *const pStreamDecoder, const char c)
{
    if (pStreamDecoder->escapeCSIParamLength < 16)
    {
        pStreamDecoder->escapeCSIParam[pStreamDecoder->escapeCSIParamLength] = c;
        pStreamDecoder->escapeCSIParamLength++;
    }
}

/*****************************************************************************
 * Dispatch CSI
 *****************************************************************************/
static void _EscapeCSIDispatch(ANSI_STREAM_DECODER_STATE *const pStreamDecoder, const char c)
{
    pStreamDecoder->escapeCSIFinalByte = c;
    pStreamDecoder->escapeDispatched   = true;
}

/*****************************************************************************
 * Parse escape sequences
 *****************************************************************************/
static bool _EscapeParser(ANSI_STREAM_DECODER_STATE *const pStreamDecoder, const char c)
{
    if (0x1b == c)
    {
        _EscapeEnterState(pStreamDecoder, ANSI_ESCAPE_STATE_ESCAPE);
        return false;
    }
    else if (((c >= 0) && (c <= 0x17)) || (0x19 == c) || ((c >= 0x1c) && (c <= 0x1f)))
    {
        return true;
    }
    else
    {
    }

    switch (pStreamDecoder->escapeState)
    {
        case ANSI_ESCAPE_STATE_ESCAPE:
        {
            if ((c <= 0x3f) || (c >= 0x60))
            {
                _EscapeEnterState(pStreamDecoder, ANSI_ESCAPE_STATE_GROUND);
                return true;
            }
            else if (0x4f == c)
            {
                _EscapeEnterState(pStreamDecoder, ANSI_ESCAPE_STATE_SS3);
            }
            else if (0x5b == c)
            {
                _EscapeEnterState(pStreamDecoder, ANSI_ESCAPE_STATE_CSI_ENTRY);
            }
            else
            {
                _EscapeEnterState(pStreamDecoder, ANSI_ESCAPE_STATE_GROUND);
                return true;
            }

            break;
        }

        case ANSI_ESCAPE_STATE_CSI_ENTRY:
        {
            if ((c >= 0x20) && (c <= 0x2f))
            {
                _EscapeCSICollect(pStreamDecoder, c);
                _EscapeEnterState(pStreamDecoder, ANSI_ESCAPE_STATE_CSI_INTER);
            }
            else if (0x3a == c)
            {
                _EscapeEnterState(pStreamDecoder, ANSI_ESCAPE_STATE_CSI_IGNORE);
            }
            else if ((c >= 0x30) && (c <= 0x3b))
            {
                _EscapeCSICollect(pStreamDecoder, c);
                _EscapeEnterState(pStreamDecoder, ANSI_ESCAPE_STATE_CSI_PARAM);
            }
            else if ((c >= 0x3c) && (c <= 0x3f))
            {
                _EscapeCSIParam(pStreamDecoder, c);
                _EscapeEnterState(pStreamDecoder, ANSI_ESCAPE_STATE_CSI_PARAM);
            }
            else if ((c >= 0x40) && (c <= 0x7e))
            {
                _EscapeCSIDispatch(pStreamDecoder, c);
                _EscapeEnterState(pStreamDecoder, ANSI_ESCAPE_STATE_GROUND);
            }
            else
            {
            }

            break;
        }

        case ANSI_ESCAPE_STATE_CSI_PARAM:
        {
            if ((c >= 0x20) && (c <= 0x2f))
            {
                _EscapeCSICollect(pStreamDecoder, c);
                _EscapeEnterState(pStreamDecoder, ANSI_ESCAPE_STATE_CSI_INTER);
            }
            else if ((0x3a == c) || ((c >= 0x3c) && (c <= 0x3f)))
            {
                _EscapeEnterState(pStreamDecoder, ANSI_ESCAPE_STATE_CSI_IGNORE);
            }
            else if ((c >= 0x30) && (c <= 0x3b))
            {
                _EscapeCSIParam(pStreamDecoder, c);
            }
            else if ((c >= 0x40) && (c <= 0x7e))
            {
                _EscapeCSIDispatch(pStreamDecoder, c);
                _EscapeEnterState(pStreamDecoder, ANSI_ESCAPE_STATE_GROUND);
            }
            else
            {
            }

            break;
        }

        case ANSI_ESCAPE_STATE_CSI_IGNORE:
        {
            if ((c >= 0x40) && (c <= 0x7e))
            {
                _EscapeEnterState(pStreamDecoder, ANSI_ESCAPE_STATE_GROUND);
            }
            else
            {
            }

            break;
        }

        case ANSI_ESCAPE_STATE_CSI_INTER:
        {
            if ((c >= 0x20) && (c <= 0x2f))
            {
                _EscapeCSICollect(pStreamDecoder, c);
            }
            else if ((c >= 0x30) && (c <= 0x3f))
            {
                _EscapeEnterState(pStreamDecoder, ANSI_ESCAPE_STATE_CSI_IGNORE);
            }
            else if ((c >= 0x40) && (c <= 0x7e))
            {
                _EscapeCSIDispatch(pStreamDecoder, c);
                _EscapeEnterState(pStreamDecoder, ANSI_ESCAPE_STATE_GROUND);
            }
            else
            {
            }

            break;
        }

        case ANSI_ESCAPE_STATE_SS3:
        {
            if ((c >= 0x20) && (c <= 0x7e))
            {
                pStreamDecoder->escapeSS3Byte = c;
                _EscapeEnterState(pStreamDecoder, ANSI_ESCAPE_STATE_GROUND);
            }
        }

        case ANSI_ESCAPE_STATE_GROUND:
        {
            break;
        }

        default:
        {
            break;
        }
    }

    return false;
}

/*****************************************************************************
 * Initialise parser
 *****************************************************************************/
void TP_ANSIStreamDecoderInit(ANSI_STREAM_DECODER_STATE *const pStreamDecoder)
{
    pStreamDecoder->decision = ANSI_STREAM_DECISION_INIT;
    pStreamDecoder->escapeState = ANSI_ESCAPE_STATE_GROUND;
}

/*****************************************************************************
 * Return the parser decision
 *****************************************************************************/
ANSI_STREAM_DECISION TP_ANSIStreamDecoderDecision(ANSI_STREAM_DECODER_STATE *const pStreamDecoder)
{
    return pStreamDecoder->decision;
}

/*****************************************************************************
 * Stream decoder
 *****************************************************************************/
ANSI_DECODE_STATUS TP_ANSIStreamDecoder(ANSI_STREAM_DECODER_STATE *const pStreamDecoder, const char c, char *pOut, size_t maxSpace)
{
    pStreamDecoder->decision = ANSI_STREAM_DECISION_UNKNOWN;

    if (1 > maxSpace)
    {
        return ANSI_DECODE_STATUS_ERR_NO_SPACE;
    }

    if (ANSI_ESCAPE_STATE_GROUND != pStreamDecoder->escapeState)
    {
        if (false == _EscapeParser(pStreamDecoder, c))
        {
            if (ANSI_ESCAPE_STATE_GROUND == pStreamDecoder->escapeState)
            {
                if (true == pStreamDecoder->escapeDispatched)
                {
                    pStreamDecoder->decision = ANSI_STREAM_DECISION_ESCAPE_CSI;

                    switch (pStreamDecoder->escapeCSIFinalByte)
                    {
                        case 0x41:
                        {
                            pStreamDecoder->decision = ANSI_STREAM_DECISION_ESCAPE_CSI_KEYUP;
                            break;
                        }

                        case 0x42:
                        {
                            pStreamDecoder->decision = ANSI_STREAM_DECISION_ESCAPE_CSI_KEYDOWN;
                            break;
                        }

                        case 0x7e:
                        {
                            if (0x31 == pStreamDecoder->escapeCSIParam[0])
                            {
                                pStreamDecoder->decision = ANSI_STREAM_DECISION_ESCAPE_CSI_F1;
                            }
                            break;
                        }

                        default:
                        {
                            break;
                        }
                    }
                }
                else if (0 != pStreamDecoder->escapeSS3Byte)
                {
                    pStreamDecoder->decision = ANSI_STREAM_DECISION_ESCAPE_SS3;
                }
            }

            return ANSI_DECODE_STATUS_OK;
        }
    }

    if (((c >= '0') && (c <= '9')) ||
        ((c >= 'A') && (c <= 'Z')) ||
        ((c >= 'a') && (c <= 'z')))
    {
        pStreamDecoder->decision = ANSI_STREAM_DECISION_ALPHA_NUMERIC;
    }
    else if (((c >= 0x20) && (c <= 0x2f)) ||
             ((c >= 0x3a) && (c <= 0x40)) ||
             ((c >= 0x5b) && (c <= 0x60)) ||
             ((c >= 0x7b) && (c <= 0x7e)))
    {
        pStreamDecoder->decision = ANSI_STREAM_DECISION_PUNCT;
    }
    else if (((c >= 0x00) && (c <= 0x1f)) || (c == 0x7f))
    {
        switch(c)
        {
            case 0x08:
            {
                pStreamDecoder->decision = ANSI_STREAM_DECISION_CONTROL_BS;
                break;
            }

            case 0x0a:
            {
                pStreamDecoder->decision = ANSI_STREAM_DECISION_CONTROL_LF;
                break;
            }

            case 0x0d:
            {
                pStreamDecoder->decision = ANSI_STREAM_DECISION_CONTROL_CR;
                break;
            }

            case 0x12:
            {
                pStreamDecoder->decision = ANSI_STREAM_DECISION_CONTROL_DC2;
                break;
            }

            case 0x1b:
            {
                pStreamDecoder->decision = ANSI_STREAM_DECISION_CONTROL_ESC;
                _EscapeEnterState(pStreamDecoder, ANSI_ESCAPE_STATE_ESCAPE);
                break;
            }

            default:
            {
                pStreamDecoder->decision = ANSI_STREAM_DECISION_CONTROL;
                break;
            }
        }

        return ANSI_DECODE_STATUS_OK;
    }
    else if (c >= 0x80)
    {
        pStreamDecoder->decision = ANSI_STREAM_DECISION_EXTENDED;

        return ANSI_DECODE_STATUS_OK;
    }

    *pOut = c;

    return 1;
}
