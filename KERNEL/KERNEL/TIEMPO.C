#include "port.h"
#include "tiempo.h"
#include "fecha.h"
#include "todo.h"

static WORD days[2][13] =
{
  {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365},
  {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366}
};

static WORD ndays[2][13] =
{
  {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
  {0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
};

extern BYTE
  Month,
  DayOfMonth,
  DayOfWeek;

extern COUNT
  Year,
  YearsSince1980;

extern request
  ClkReqHdr;

VOID DosGetTime(BYTE FAR * hp, BYTE FAR * mp, BYTE FAR * sp, BYTE FAR * hdp)
{
  ClkReqHdr.r_length = sizeof(request);
  ClkReqHdr.r_command = C_INPUT;
  ClkReqHdr.r_count = sizeof(struct ClockRecord);
  ClkReqHdr.r_trans = (BYTE FAR *) (&ClkRecord);
  ClkReqHdr.r_status = 0;
  execrh((request FAR *) & ClkReqHdr, (struct dhdr FAR *)clock);
  if (ClkReqHdr.r_status & S_ERROR)
    return;

  *hp = ClkRecord.clkHours;
  *mp = ClkRecord.clkMinutes;
  *sp = ClkRecord.clkSeconds;
  *hdp = ClkRecord.clkHundredths;
}

COUNT DosSetTime(BYTE FAR * hp, BYTE FAR * mp, BYTE FAR * sp, BYTE FAR * hdp)
{
  DosGetDate((BYTE FAR *) & DayOfWeek, (BYTE FAR *) & Month,
             (BYTE FAR *) & DayOfMonth, (COUNT FAR *) & Year);

  ClkRecord.clkHours = *hp;
  ClkRecord.clkMinutes = *mp;
  ClkRecord.clkSeconds = *sp;
  ClkRecord.clkHundredths = *hdp;

  YearsSince1980 = Year - 1980;
  ClkRecord.clkDays = DayOfMonth - 1
      + days[is_leap_year(Year)][Month - 1]
      + ((YearsSince1980) * 365)
      + ((YearsSince1980 + 3) / 4);

  ClkReqHdr.r_length = sizeof(request);
  ClkReqHdr.r_command = C_OUTPUT;
  ClkReqHdr.r_count = sizeof(struct ClockRecord);
  ClkReqHdr.r_trans = (BYTE FAR *) (&ClkRecord);
  ClkReqHdr.r_status = 0;
  execrh((request FAR *) & ClkReqHdr, (struct dhdr FAR *)clock);
  if (ClkReqHdr.r_status & S_ERROR)
    return char_error(&ClkReqHdr, (struct dhdr FAR *)clock);
  return SUCCESS;
}

VOID DosGetDate(wdp, mp, mdp, yp)
BYTE FAR *wdp,
  FAR * mp,
  FAR * mdp;
COUNT FAR *yp;
{
  COUNT count,
    c;

  ClkReqHdr.r_length = sizeof(request);
  ClkReqHdr.r_command = C_INPUT;
  ClkReqHdr.r_count = sizeof(struct ClockRecord);
  ClkReqHdr.r_trans = (BYTE FAR *) (&ClkRecord);
  ClkReqHdr.r_status = 0;
  execrh((request FAR *) & ClkReqHdr, (struct dhdr FAR *)clock);
  if (ClkReqHdr.r_status & S_ERROR)
    return;

  for (Year = 1980, c = ClkRecord.clkDays; c > 0;)
  {
    count = is_leap_year(Year) ? 366 : 365;
    if (c >= count)
    {
      ++Year;
      c -= count;
    }
    else
      break;
  }

  Month = 1;
  while (c >= ndays[count == 366][Month])
  {
    c -= ndays[count == 366][Month];
    ++Month;
  }

  *mp = Month;
  *mdp = c + 1;
  *yp = Year;

  DayOfWeek = (ClkRecord.clkDays % 7 + 2) % 7;
  *wdp = DayOfWeek;
}

COUNT DosSetDate(mp, mdp, yp)
BYTE FAR *mp,
  FAR * mdp;
COUNT FAR *yp;
{
  Month = *mp;
  DayOfMonth = *mdp;
  Year = *yp;
  if (Year < 1980 || Year > 2099
      || Month < 1 || Month > 12
      || DayOfMonth < 1 || DayOfMonth > ndays[is_leap_year(Year)][Month])
    return DE_INVLDDATA;

  DosGetTime((BYTE FAR *) & ClkRecord.clkHours,
             (BYTE FAR *) & ClkRecord.clkMinutes,
             (BYTE FAR *) & ClkRecord.clkSeconds,
             (BYTE FAR *) & ClkRecord.clkHundredths);

  YearsSince1980 = Year - 1980;
  ClkRecord.clkDays = DayOfMonth - 1
      + days[is_leap_year(Year)][Month - 1]
      + ((YearsSince1980) * 365)
      + ((YearsSince1980 + 3) / 4);

  ClkReqHdr.r_length = sizeof(request);
  ClkReqHdr.r_command = C_OUTPUT;
  ClkReqHdr.r_count = sizeof(struct ClockRecord);
  ClkReqHdr.r_trans = (BYTE FAR *) (&ClkRecord);
  ClkReqHdr.r_status = 0;
  execrh((request FAR *) & ClkReqHdr, (struct dhdr FAR *)clock);
  if (ClkReqHdr.r_status & S_ERROR)
    return char_error(&ClkReqHdr, (struct dhdr FAR *)clock);
  return SUCCESS;
}
