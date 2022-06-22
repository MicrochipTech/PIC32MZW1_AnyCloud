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

#include "include/at_cmds.h"
#include "platform/platform.h"
#include "at_cmd_xmodem.h"

#ifndef AT_CMD_CONF_XMODEM_SUPPORT_CSUM
#define AT_CMD_CONF_XMODEM_SUPPORT_CRC
#endif

typedef enum
{
    XMODEM_DECODE_OK,
    XMODEM_DECODE_OLD_BLOCK,
    XMODEM_DECODE_INVALID_HDR
} XMODEM_DECODE_STATUS;

#define XMDM_SOH     0x01       // Start of heading
#define XMDM_STX     0x02       // Start of text
#define XMDM_EOT     0x04       // End of text
#define XMDM_ACK     0x06       // Acknowledge
#define XMDM_NAK     0x15       // Negative acknowledge
#define XMDM_CAN     0x18       // Cancel

#ifdef AT_CMD_CONF_XMODEM_SUPPORT_CSUM
#define XMODEM_CSUM_FRAME_SZ    (3-1+128+1)             /* 3 byte header - SOH, 128 bytes, 1 checksum */
#define XMODEM_MAX_FRAME_SZ     XMODEM_CSUM_FRAME_SZ
#endif

#ifdef AT_CMD_CONF_XMODEM_SUPPORT_CRC
#undef XMODEM_MAX_FRAME_SZ
#define XMODEM_CRC_FRAME_SZ     (3-1+128+2)             /* 3 byte header - SOH, 128 bytes, 2 CRC */
#define XMODEM_MAX_FRAME_SZ     XMODEM_CRC_FRAME_SZ
#endif

#ifdef AT_CMD_CONF_XMODEM_SUPPORT_1K
#undef XMODEM_MAX_FRAME_SZ
#define XMODEM_1K_CRC_FRAME_SZ  (3-1+1024+2)            /* 3 byte header - STX, 1024 bytes, 2 CRC */
#define XMODEM_MAX_FRAME_SZ     XMODEM_1K_CRC_FRAME_SZ
#define AT_CMD_CONF_XMODEM_SUPPORT_CRC
#endif

static bool isStarted;
static uint8_t expectedBN;
static uint32_t lastStartTimeMs;
static uint8_t currentFrameType;
static uint8_t receiveBuffer[XMODEM_MAX_FRAME_SZ];
static size_t receiveBufferLength;
static uint_fast16_t expectedFrameSz;
#ifdef AT_CMD_CONF_XMODEM_SUPPORT_CRC
static bool useCRCs;
#endif
static size_t receiveFileLength;
static bool sendInitialSeq;
#ifdef AT_CMD_CONF_XMODEM_SUPPORT_YMODEM_PROTOCOL
static bool isYModem;
#endif
static tpfATCMDXModemDataHandler pfDataHandler;

static XMODEM_DECODE_STATUS _XModemDecodePacketHdr(uint8_t expPN, uint8_t *ppkt)
{
    uint8_t pn;

    pn = *ppkt++;

    if (pn < expPN)
    {
        return XMODEM_DECODE_OLD_BLOCK;
    }

    if ((pn > expPN) || (*ppkt++ != (255-pn)))
    {
        return XMODEM_DECODE_INVALID_HDR;
    }

    return XMODEM_DECODE_OK;
}

#ifdef AT_CMD_CONF_XMODEM_SUPPORT_CRC
static bool _XModemDecodePacketCRC(uint8_t expPN, uint8_t *ppkt, uint_fast16_t length)
{
    uint16_t crc16Calc;
    uint16_t crc16Pkt;
    uint8_t i;

    ppkt += 2;

    crc16Calc = 0;
    while (length--)
    {
        crc16Calc ^= ((uint16_t)*ppkt++)<<8;
        for(i=0; i<8; i++)
        {
            if(crc16Calc & 0x8000)
            {
                crc16Calc <<= 1;
                crc16Calc ^= 0x1021;
            }
            else
            {
                crc16Calc <<= 1;
            }
        }
    }

    crc16Pkt = (uint16_t)*ppkt++ << 8;
    crc16Pkt |= *ppkt;

    if(crc16Calc != crc16Pkt)
    {
        return false;
    }

    return true;
}
#endif

#ifdef AT_CMD_CONF_XMODEM_SUPPORT_CSUM
static bool _XModemDecodePacketCS(uint8_t expPN, uint8_t *ppkt, uint_fast16_t length)
{
    uint8_t chkSum = XMDM_SOH;

    length += 2;

    while (length--)
    {
        chkSum += *ppkt++;
    }

    if (chkSum != *ppkt)
    {
        return false;
    }

    return true;
}
#endif

void ATCMD_XModemInit(void)
{
    isStarted     = false;
    pfDataHandler = NULL;
}

bool ATCMD_XModemIsStarted(void)
{
    return isStarted;
}

void ATCMD_XModemStart(bool requireCRCs, tpfATCMDXModemDataHandler pXMDataHandler)
{
    if (true == isStarted)
    {
        return;
    }

#ifdef AT_CMD_CONF_XMODEM_SUPPORT_CRC
    useCRCs             = requireCRCs;
#endif

    expectedBN          = 1;
    currentFrameType    = 0;
    receiveFileLength   = 0;
    sendInitialSeq      = true;

#ifdef AT_CMD_CONF_XMODEM_SUPPORT_YMODEM_PROTOCOL
    isYModem            = false;
#endif

    lastStartTimeMs = ATCMD_PlatformGetSysTimeMs() - 3000;

    pfDataHandler = pXMDataHandler;
    isStarted = true;
}

void ATCMD_XModemStop(void)
{
    if (NULL != pfDataHandler)
    {
        pfDataHandler(expectedBN, NULL, 0);
    }

    isStarted     = false;
    pfDataHandler = NULL;
}

void ATCMD_XModemProcess(void)
{
    uint32_t curTimeMs;
    size_t numBytes;

    if (false == isStarted)
    {
        return;
    }

    curTimeMs = ATCMD_PlatformGetSysTimeMs();

    numBytes = ATCMD_PlatformUARTReadGetCount();

    if ((0 == currentFrameType) && (numBytes > 0))
    {
        currentFrameType = ATCMD_PlatformUARTReadGetByte();

        if (XMDM_SOH == currentFrameType)
        {
#ifdef AT_CMD_CONF_XMODEM_SUPPORT_CRC
            if (true == useCRCs)
            {
                expectedFrameSz = XMODEM_CRC_FRAME_SZ;
            }
            else
#endif
            {
#ifdef AT_CMD_CONF_XMODEM_SUPPORT_CSUM
                expectedFrameSz = XMODEM_CSUM_FRAME_SZ;
#endif
            }
        }
#ifdef AT_CMD_CONF_XMODEM_SUPPORT_1K
        else if (XMDM_STX == currentFrameType)
        {
            expectedFrameSz = XMODEM_1K_CRC_FRAME_SZ;
        }
#endif

        receiveBufferLength = 0;
        numBytes--;
        sendInitialSeq = false;
    }
    else if (true == sendInitialSeq)
    {
        if ((curTimeMs - lastStartTimeMs) > 3000)
        {
#ifdef AT_CMD_CONF_XMODEM_SUPPORT_CRC
            if (true == useCRCs)
            {
                ATCMD_PlatformUARTWritePutByte('C');
            }
            else
#endif
            {
#ifdef AT_CMD_CONF_XMODEM_SUPPORT_CSUM
                ATCMD_PlatformUARTWritePutByte(XMDM_NAK);
#endif
            }

            lastStartTimeMs = curTimeMs;
        }
    }

    switch (currentFrameType)
    {
        case XMDM_SOH:
#ifdef AT_CMD_CONF_XMODEM_SUPPORT_1K
        case XMDM_STX:
#endif
        {
            while (receiveBufferLength < expectedFrameSz)
            {
                numBytes = ATCMD_PlatformUARTReadGetBuffer(&receiveBuffer[receiveBufferLength], expectedFrameSz-receiveBufferLength);

                if (0 == numBytes)
                {
                    break;
                }

                receiveBufferLength += numBytes;
            }

            if (receiveBufferLength == expectedFrameSz)
            {
                XMODEM_DECODE_STATUS decodeStatus;

#ifdef AT_CMD_CONF_XMODEM_SUPPORT_YMODEM_PROTOCOL
                if ((false == isYModem) && (1 == expectedBN) && (0 == receiveBuffer[0]) && (0xff == receiveBuffer[1]))
                {
                    isYModem = true;
                    expectedBN = 0;
                }
#endif
                decodeStatus = _XModemDecodePacketHdr(expectedBN, receiveBuffer);

                if (XMODEM_DECODE_OK == decodeStatus)
                {
                    bool blockOK = false;
                    uint_fast16_t blockSize = 128;

#ifdef AT_CMD_CONF_XMODEM_SUPPORT_CRC
                    if (true == useCRCs)
                    {
#ifdef AT_CMD_CONF_XMODEM_SUPPORT_1K
                        if (XMDM_STX == currentFrameType)
                        {
                            blockSize = 1024;
                        }
#endif

                        blockOK = _XModemDecodePacketCRC(expectedBN, receiveBuffer, blockSize);
                    }
                    else
#endif
                    {
#ifdef AT_CMD_CONF_XMODEM_SUPPORT_CSUM
                        blockOK = _XModemDecodePacketCS(expectedBN, receiveBuffer, blockSize);
#endif
                    }

                    if (true == blockOK)
                    {
                        if (NULL != pfDataHandler)
                        {
                            pfDataHandler(expectedBN, &receiveBuffer[2], blockSize);
                        }

                        ATCMD_PlatformUARTWritePutByte(XMDM_ACK);

#ifdef AT_CMD_CONF_XMODEM_SUPPORT_YMODEM_PROTOCOL
                        if ((true == isYModem) && (0 == expectedBN))
                        {
                            ATCMD_PlatformUARTWritePutByte('C');
                        }
#endif
                        expectedBN++;
                        receiveFileLength += blockSize;
                    }
                    else
                    {
                        ATCMD_PlatformUARTWritePutByte(XMDM_NAK);
                    }
                }
                else if (XMODEM_DECODE_INVALID_HDR == decodeStatus)
                {
                    ATCMD_PlatformUARTWritePutByte(XMDM_NAK);
                }

                currentFrameType = 0;
            }

            break;
        }

        case XMDM_EOT:
        {
            ATCMD_PlatformUARTWritePutByte(XMDM_ACK);

#ifdef AT_CMD_CONF_XMODEM_SUPPORT_YMODEM_PROTOCOL
            if (true == isYModem)
            {
                ATCMD_PlatformUARTWritePutByte('C');
                expectedBN = 1;
                isYModem = false;
            }
            else
#endif
            {
                ATCMD_XModemStop();
            }

            currentFrameType = 0;
            break;
        }
    }
}
