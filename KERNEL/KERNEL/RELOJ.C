#include "port.h"
#include "todo.h"

#ifdef PROTO
BOOL ReadPCClock(ULONG *);
VOID WriteATClock(BYTE *, BYTE, BYTE, BYTE);
VOID WritePCClock(ULONG);
COUNT BcdToByte(COUNT);
COUNT BcdToWord(BYTE *, UWORD *, UWORD *, UWORD *);
COUNT ByteToBcd(COUNT);
VOID DayToBcd(BYTE *, UWORD *, UWORD *, UWORD *);
#else
BOOL ReadPCClock();
VOID WriteATClock();
VOID WritePCClock();
COUNT BcdToByte();
COUNT BcdToWord();
COUNT ByteToBcd();
VOID DayToBcd();
#endif


WORD days[2][13] =
{
  {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365},
  {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366}
};

static struct ClockRecord clk;

static BYTE bcdDays[4];
static UWORD Month,
  Day,
  Year;
static BYTE bcdMinutes;
static BYTE bcdHours;
static BYTE bcdHundredths;
static BYTE bcdSeconds;

static ULONG Ticks;
UWORD DaysSinceEpoch = 0;

WORD clk_driver(rqptr rp)
{
  REG COUNT count,
    c;
  BYTE FAR *cp;

  switch (rp->r_command)
  {
    case C_INIT:
      rp->r_endaddr = device_end();
      rp->r_nunits = 0;
      return S_DONE;

    case C_INPUT:
      count = rp->r_count;
      if (count > sizeof(struct ClockRecord))
          count = sizeof(struct ClockRecord);
      {
	ULONG remainder, hs;
        if (ReadPCClock(&Ticks))
          ++DaysSinceEpoch;
        clk.clkDays = DaysSinceEpoch;
	hs = 0;
	if (Ticks >= 64*19663ul) { hs += 64*108000ul; Ticks -= 64*19663ul; }
	if (Ticks >= 32*19663ul) { hs += 32*108000ul; Ticks -= 32*19663ul; }
	if (Ticks >= 16*19663ul) { hs += 16*108000ul; Ticks -= 16*19663ul; }
	if (Ticks >=  8*19663ul) { hs +=  8*108000ul; Ticks -=  8*19663ul; }
	if (Ticks >=  4*19663ul) { hs +=  4*108000ul; Ticks -=  4*19663ul; }
	if (Ticks >=  2*19663ul) { hs +=  2*108000ul; Ticks -=  2*19663ul; }
	if (Ticks >=	19663ul) { hs +=    108000ul; Ticks -=    19663ul; }
	hs += Ticks * 108000ul / 19663ul;
        clk.clkHours = hs / 360000ul;
        remainder = hs % 360000ul;
        clk.clkMinutes = remainder / 6000ul;
        remainder %= 6000ul;
        clk.clkSeconds = remainder / 100ul;
        clk.clkHundredths = remainder % 100ul;
      }
      fbcopy((BYTE FAR *) & clk, rp->r_trans, count);
      return S_DONE;

    case C_OUTPUT:
      count = rp->r_count;
      if (count > sizeof(struct ClockRecord))
          count = sizeof(struct ClockRecord);
      rp->r_count = count;
      fbcopy(rp->r_trans, (BYTE FAR *) & clk, count);

      DaysSinceEpoch = clk.clkDays;
      {
	ULONG hs;
	hs = 360000ul * clk.clkHours +
	       6000ul * clk.clkMinutes +
		100ul * clk.clkSeconds +
			clk.clkHundredths;
	Ticks = 0;
	if (hs >= 64*108000ul) { Ticks += 64*19663ul; hs -= 64*108000ul; }
	if (hs >= 32*108000ul) { Ticks += 32*19663ul; hs -= 32*108000ul; }
	if (hs >= 16*108000ul) { Ticks += 16*19663ul; hs -= 16*108000ul; }
	if (hs >=  8*108000ul) { Ticks +=  8*19663ul; hs -=  8*108000ul; }
	if (hs >=  4*108000ul) { Ticks +=  4*19663ul; hs -=  4*108000ul; }
	if (hs >=  2*108000ul) { Ticks +=  2*19663ul; hs -=  2*108000ul; }
	if (hs >=    108000ul) { Ticks +=    19663ul; hs -=    108000ul; }
	Ticks += hs * 19663ul / 108000ul;
      }
      WritePCClock(Ticks);

      for (Year = 1980, c = clk.clkDays; c > 0;)
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

      for (Month = 1; Month < 13; ++Month)
      {
        if (days[count == 366][Month] > c)
        {
          Day = c - days[count == 366][Month - 1] + 1;
          break;
        }
      }
      DayToBcd((BYTE *) bcdDays, &Month, &Day, &Year);
      bcdMinutes = ByteToBcd(clk.clkMinutes);
      bcdHours = ByteToBcd(clk.clkHours);
      bcdSeconds = ByteToBcd(clk.clkSeconds);
      WriteATClock(bcdDays, bcdHours, bcdMinutes, bcdSeconds);
      return S_DONE;

    case C_OFLUSH:
    case C_IFLUSH:
      return S_DONE;

    case C_OUB:
    case C_NDREAD:
    case C_OSTAT:
    case C_ISTAT:
    default:
      return failure(E_FAILURE);       
  }
}

COUNT ByteToBcd(COUNT x)
{
  return ((x / 10) << 4) | (x % 10);
}

VOID DayToBcd(BYTE * x, UWORD * mon, UWORD * day, UWORD * yr)
{
  x[1] = ByteToBcd(*mon);
  x[0] = ByteToBcd(*day);
  x[3] = ByteToBcd(*yr / 100);
  x[2] = ByteToBcd(*yr % 100);
}

VOID FAR init_call_WritePCClock(ULONG ticks)
{
  WritePCClock(ticks);
}
