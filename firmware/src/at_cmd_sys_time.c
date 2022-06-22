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
#include <inttypes.h>
#include <time.h>
#include <string.h>

#include "at_cmd_app.h"
#include "at_cmd_sys_time.h"

typedef struct
{
    bool        ntpSync;

    /* Internal reference tick, relative to 1/1/1970 (UNIX UTC) */
    uint64_t    intRefTick;
} ATCMD_SYSTIME_STATE;

static ATCMD_SYSTIME_STATE atCmdSysTimeState;

static time_t ConvertRFC3339StrToTime(const char *p, size_t l)
{
    time_t timeUTC;
    struct tm t;

    if (('-' != p[4]) || ('-' != p[7]) || ('T' != p[10]) || (':' != p[13]) || (':' != p[16]) || ('.' != p[19]) || ('Z' != p[22]))
    {
        return -1;
    }

    t.tm_year = atoi(&p[0]);

    if (0 == t.tm_year)
    {
        return -1;
    }

    t.tm_mon = atoi(&p[5]);

    if (0 == t.tm_mon)
    {
        return -1;
    }

    t.tm_mday = atoi(&p[8]);

    if ((t.tm_mday < 1) || (t.tm_mday > 31))
    {
        return -1;
    }

    t.tm_hour = atoi(&p[11]);

    if ((t.tm_hour < 0) || (t.tm_hour > 23))
    {
        return -1;
    }

    t.tm_min = atoi(&p[14]);

    if ((t.tm_min < 0) || (t.tm_min > 59))
    {
        return -1;
    }

    t.tm_sec = atoi(&p[17]);

    if ((t.tm_sec < 0) || (t.tm_sec > 59))
    {
        return -1;
    }

    t.tm_year -= 1900;
    t.tm_mon  -= 1;
    t.tm_isdst = 0;

    timeUTC = mktime(&t);

    return timeUTC;
}

void ATCMD_SysTimeInit(void)
{
    atCmdSysTimeState.ntpSync = false;

    atCmdSysTimeState.intRefTick = SYS_TMR_TickCountGetLong();
}

uint32_t ATCMD_SysTimeGetUTC(void)
{
    uint32_t utcTime = 0;

    if (true == atCmdSysTimeState.ntpSync)
    {
        if (SNTP_RES_OK != TCPIP_SNTP_TimeGet(&utcTime, NULL))
        {
            atCmdSysTimeState.ntpSync = false;
        }
    }

    if (false == atCmdSysTimeState.ntpSync)
    {
        utcTime = (uint32_t)((SYS_TMR_TickCountGetLong() - atCmdSysTimeState.intRefTick) / SYS_TMR_TickCounterFrequencyGet());
    }

    return utcTime;
}

bool ATCMD_SysTimeSetUTC(int format, const void *pNewTime)
{
    uint32_t utcTime = 0;

    if (NULL == pNewTime)
    {
        return false;
    }

    if ((format < 1) || (format > 3))
    {
        return false;
    }

    switch (format)
    {
        case 1:
        {
            utcTime = *((uint32_t*)pNewTime);
            break;
        }

        case 2:
        {
            utcTime = *((uint32_t*)pNewTime);

            if (utcTime <= TCPIP_NTP_EPOCH)
            {
                return false;
            }

            utcTime -= TCPIP_NTP_EPOCH;
            break;
        }

        case 3:
        {
            time_t newTime;

            if (23 != strlen(pNewTime))
            {
                return false;
            }

            newTime = ConvertRFC3339StrToTime(pNewTime, 23);

            if (newTime < 0)
            {
                return false;
            }

            utcTime = newTime;
            break;
        }
    }

    atCmdSysTimeState.intRefTick = SYS_TMR_TickCountGetLong() - ((uint64_t)utcTime * SYS_TMR_TickCounterFrequencyGet());

    atCmdSysTimeState.ntpSync = false;

    return true;
}

void ATCMD_SysTimeSetNTPSync(bool sync)
{
    atCmdSysTimeState.ntpSync = sync;
}

bool ATCMD_SysTimeDisplayTimeAEC(int format, time_t utc)
{
    if ((format < 1) || (format > 3))
    {
        return false;
    }

    ATCMD_Printf("+TIME:%d,", format);

    switch (format)
    {
        case 1:
        {
            ATCMD_Printf("%" PRIu32, utc);
            break;
        }

        case 2:
        {
            ATCMD_Printf("%" PRIu32, utc + TCPIP_NTP_EPOCH);
            break;
        }

        case 3:
        {
            struct tm *ptm;
            ptm = gmtime(&utc);

            ATCMD_Printf("\"%04d-%02d-%02dT%02d:%02d:%02d.00Z\"", ptm->tm_year+1900, ptm->tm_mon+1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
            break;
        }
    }

    ATCMD_Print("\r\n", 2);

    return true;
}
