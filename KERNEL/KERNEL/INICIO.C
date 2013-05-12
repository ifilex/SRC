#include "inicio.h"
#define MAIN
#include "port.h"
#include "todo.h"

extern UWORD DaysSinceEpoch;
extern WORD days[2][13];

INIT BOOL ReadATClock(BYTE *, BYTE *, BYTE *, BYTE *);
VOID FAR init_call_WritePCClock(ULONG);

INIT VOID configDone(VOID);
INIT static void InitIO(void);
INIT static COUNT BcdToByte(COUNT);
INIT static COUNT BcdToDay(BYTE *);

INIT static VOID init_kernel(VOID);
INIT static VOID signon(VOID);
INIT VOID kernel(VOID);
INIT VOID FsConfig(VOID);

INIT VOID main(void)
{
#ifdef KDB
  BootDrive = 1;
#endif
  init_kernel();
#ifdef DEBUG
  printf("inicio.c: Unidad logica de inicio %c\n", 'A' + BootDrive - 1);
#endif
  signon();
  kernel();
}

INIT static VOID init_kernel(void)
{
  COUNT i;

  os_major = MAJOR_RELEASE;
  os_minor = MINOR_RELEASE;

  cu_psp = DOS_PSP;
  nblkdev = 0;
  maxbksize = 0x200;
  switchar = '/';

  ram_top = init_oem();

#ifndef KDB
  for (i = 0x20; i <= 0x3f; i++)
    setvec(i, empty_handler);
#endif

  InitIO();
  syscon = (struct dhdr FAR *)&con_dev;
  clock = (struct dhdr FAR *)&clk_dev;

#ifndef KDB
  setvec(0x1b, got_cbreak);
  setvec(0x20, int20_handler);
  setvec(0x21, int21_handler);
  setvec(0x22, int22_handler);
  setvec(0x23, empty_handler);
  setvec(0x24, int24_handler);
  setvec(0x25, low_int25_handler);
  setvec(0x26, low_int26_handler);
  setvec(0x27, int27_handler);
  setvec(0x28, int28_handler);
  setvec(0x2a, int2a_handler);
  setvec(0x2f, int2f_handler);
#endif

  scr_pos = 0;
  break_ena = TRUE;

  lastdrive = Config.cfgLastdrive;
  PreConfig();

  FsConfig();

#ifndef KDB
  DoConfig();

  lastdrive = Config.cfgLastdrive;
  if (lastdrive < nblkdev)
    lastdrive = nblkdev;

  PostConfig();

  FsConfig();

  DoConfig();
  configDone();
#endif

  mem_access_mode = FIRST_FIT;
  verify_ena = FALSE;
  InDOS = 0;
  version_flags = 0;
  pDirFileNode = 0;
}

INIT VOID FsConfig(VOID)
{
  REG COUNT i;
  date Date;
  time Time;

  Date = dos_getdate();
  Time = dos_gettime();

  for (i = 0; i < Config.cfgFiles; i++)
    f_nodes[i].f_count = 0;

  sfthead->sftt_next = (sfttbl FAR *) - 1;
  sfthead->sftt_count = Config.cfgFiles;
  for (i = 0; i < sfthead->sftt_count; i++)
  {
    sfthead->sftt_table[i].sft_count = 0;
    sfthead->sftt_table[i].sft_status = -1;
  }
  sfthead->sftt_table[0].sft_count = 1;
  sfthead->sftt_table[0].sft_mode = SFT_MREAD;
  sfthead->sftt_table[0].sft_attrib = 0;
  sfthead->sftt_table[0].sft_flags =
      (con_dev.dh_attr & ~SFT_MASK) | SFT_FDEVICE | SFT_FEOF | SFT_FCONIN | SFT_FCONOUT;
  sfthead->sftt_table[0].sft_psp = DOS_PSP;
  sfthead->sftt_table[0].sft_date = Date;
  sfthead->sftt_table[0].sft_time = Time;
  fbcopy(
          (VOID FAR *) "CON        ",
          (VOID FAR *) sfthead->sftt_table[0].sft_name, 11);
  sfthead->sftt_table[0].sft_dev = (struct dhdr FAR *)&con_dev;

  sfthead->sftt_table[1].sft_count = 1;
  sfthead->sftt_table[1].sft_mode = SFT_MWRITE;
  sfthead->sftt_table[1].sft_attrib = 0;
  sfthead->sftt_table[1].sft_flags =
      (con_dev.dh_attr & ~SFT_MASK) | SFT_FDEVICE | SFT_FEOF | SFT_FCONIN | SFT_FCONOUT;
  sfthead->sftt_table[1].sft_psp = DOS_PSP;
  sfthead->sftt_table[1].sft_date = Date;
  sfthead->sftt_table[1].sft_time = Time;
  fbcopy(
          (VOID FAR *) "CON        ",
          (VOID FAR *) sfthead->sftt_table[1].sft_name, 11);
  sfthead->sftt_table[1].sft_dev = (struct dhdr FAR *)&con_dev;

  sfthead->sftt_table[2].sft_count = 1;
  sfthead->sftt_table[2].sft_mode = SFT_MWRITE;
  sfthead->sftt_table[2].sft_attrib = 0;
  sfthead->sftt_table[2].sft_flags =
      (con_dev.dh_attr & ~SFT_MASK) | SFT_FDEVICE | SFT_FEOF | SFT_FCONIN | SFT_FCONOUT;
  sfthead->sftt_table[2].sft_psp = DOS_PSP;
  sfthead->sftt_table[2].sft_date = Date;
  sfthead->sftt_table[2].sft_time = Time;
  fbcopy(
          (VOID FAR *) "CON        ",
          (VOID FAR *) sfthead->sftt_table[2].sft_name, 11);
  sfthead->sftt_table[2].sft_dev = (struct dhdr FAR *)&con_dev;

  sfthead->sftt_table[3].sft_count = 1;
  sfthead->sftt_table[3].sft_mode = SFT_MRDWR;
  sfthead->sftt_table[3].sft_attrib = 0;
  sfthead->sftt_table[3].sft_flags =
      (aux_dev.dh_attr & ~SFT_MASK) | SFT_FDEVICE;
  sfthead->sftt_table[3].sft_psp = DOS_PSP;
  sfthead->sftt_table[3].sft_date = Date;
  sfthead->sftt_table[3].sft_time = Time;
  fbcopy(
          (VOID FAR *) "AUX        ",
          (VOID FAR *) sfthead->sftt_table[3].sft_name, 11);
  sfthead->sftt_table[3].sft_dev = (struct dhdr FAR *)&aux_dev;

  sfthead->sftt_table[4].sft_count = 1;
  sfthead->sftt_table[4].sft_mode = SFT_MWRITE;
  sfthead->sftt_table[4].sft_attrib = 0;
  sfthead->sftt_table[4].sft_flags =
      (prn_dev.dh_attr & ~SFT_MASK) | SFT_FDEVICE;
  sfthead->sftt_table[4].sft_psp = DOS_PSP;
  sfthead->sftt_table[4].sft_date = Date;
  sfthead->sftt_table[4].sft_time = Time;
  fbcopy(
          (VOID FAR *) "PRN        ",
          (VOID FAR *) sfthead->sftt_table[4].sft_name, 11);
  sfthead->sftt_table[4].sft_dev = (struct dhdr FAR *)&prn_dev;

  default_drive = BootDrive - 1;

  for (i = 0; i < NDEVS; i++)
    scopy("\\", blk_devices[i].dpb_path);

  init_buffers();
}

INIT static VOID signon()
{
  printf("\n  \n",
         os_major, os_minor, copyright);
  printf(os_release,
         REVISION_MAJOR, REVISION_MINOR, REVISION_SEQ,
         BUILD);
}

INIT static VOID kernel()
{
  seg asize;
  BYTE FAR *ep,
   *sp;
  COUNT ret_code;
#ifndef KDB
  static BYTE *path = "PATH=BIN;SBIN;USR\\BIN;USR\\SBIN;USR\\LIB;LIB";
#endif

#ifdef KDB
  kdb();
#else
  if (DosMemAlloc(0x20, FIRST_FIT, (seg FAR *) & master_env, (seg FAR *) & asize) < 0)
    fatal("no hay espacio en memoria suficiente para iniciar el sistema");

  ++master_env;
  ep = MK_FP(master_env, 0);

  for (sp = path; *sp != 0;)
    *ep++ = *sp++;

  *ep++ = '\0';
  *ep++ = '\0';
  *((int FAR *)ep) = 0;
  ep += sizeof(int);
#endif
  RootPsp = ~0;
  p_0();
}

extern BYTE FAR *lpBase;


VOID init_device(struct dhdr FAR * dhp, BYTE FAR * cmdLine)
{
  request rq;
  ULONG memtop = ((ULONG) ram_top) << 10;
  ULONG maxmem = memtop - ((ULONG) FP_SEG(dhp) << 4);

  if (maxmem >= 0x10000)
    maxmem = 0xFFFF;

  rq.r_unit = 0;
  rq.r_status = 0;
  rq.r_command = C_INIT;
  rq.r_length = sizeof(request);
  rq.r_endaddr = MK_FP(FP_SEG(dhp), maxmem);
  rq.r_bpbptr = (void FAR *)(cmdLine ? cmdLine : "\n");
  rq.r_firstunit = nblkdev;

  execrh((request FAR *) & rq, dhp);

  if (cmdLine)
    lpBase = rq.r_endaddr;

  if (!(dhp->dh_attr & ATTR_CHAR) && (rq.r_nunits != 0))
  {
    REG COUNT Index;

    for (Index = 0; Index < rq.r_nunits; Index++)
    {
      if (nblkdev)
        blk_devices[nblkdev - 1].dpb_next = &blk_devices[nblkdev];

      blk_devices[nblkdev].dpb_unit = nblkdev;
      blk_devices[nblkdev].dpb_subunit = Index;
      blk_devices[nblkdev].dpb_device = dhp;
      blk_devices[nblkdev].dpb_flags = M_CHANGED;

      ++nblkdev;
    }
    blk_devices[nblkdev - 1].dpb_next = (void FAR *)0xFFFFFFFF;
  }
  DPBp = &blk_devices[0];
}

struct dhdr FAR *link_dhdr(struct dhdr FAR * lp, struct dhdr FAR * dhp, BYTE FAR * cmdLine)
{
  lp->dh_next = dhp;
  init_device(dhp, cmdLine);
  return dhp;
}

INIT static void InitIO(void)
{
  BYTE bcd_days[4], bcd_minutes, bcd_hours, bcd_seconds;
  ULONG ticks;

  nul_dev.dh_next = (struct dhdr FAR *)&con_dev;
  setvec(0x29, int29_handler);  
  init_device((struct dhdr FAR *)&con_dev, NULL);
  init_device((struct dhdr FAR *)&clk_dev, NULL);
  init_device((struct dhdr FAR *)&blk_dev, NULL);
  if (!ReadATClock(bcd_days, &bcd_hours, &bcd_minutes, &bcd_seconds))
  {
    DaysSinceEpoch = BcdToDay(bcd_days);
    ticks = (3600ul * BcdToByte(bcd_hours) +
	       60ul * BcdToByte(bcd_minutes) +
		      BcdToByte(bcd_seconds)) * 19663ul / 1080ul;
    WritePCClock(ticks);
  }
}

INIT static COUNT BcdToByte(COUNT x)
{
  return ((((x) >> 4) & 0xf) * 10 + ((x) & 0xf));
}

INIT static COUNT BcdToDay(BYTE * x)
{
  UWORD mon, day, yr;

  mon = BcdToByte(x[1]) - 1;
  day = BcdToByte(x[0]) - 1;
  yr = 100 * BcdToByte(x[3]) + BcdToByte(x[2]);
  if (yr < 1980)
    return 0;
  else
  {
    day += days[is_leap_year(yr)][mon];
    while (--yr >= 1980)
      day += is_leap_year(yr) ? 366 : 365;
    return day;
  }
}
