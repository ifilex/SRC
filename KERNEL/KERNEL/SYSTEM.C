#include "inicio.h"
#include "port.h"
#include "todo.h"

#ifdef __TURBOC__
void __int__(int); 
#endif

#ifdef KDB
#include <alloc.h>

#define KernelAlloc(x) adjust_far((void far *)malloc((unsigned long)(x)))
#endif

BYTE FAR *lpBase;
static BYTE FAR *lpOldLast;
static COUNT nCfgLine;
static COUNT nPass;
static BYTE szLine[256];
static BYTE szBuf[256];

int singleStep = 0;

INIT VOID Buffers(BYTE * pLine);
INIT VOID sysScreenMode(BYTE * pLine);
INIT VOID sysVersion(BYTE * pLine);
INIT VOID Break(BYTE * pLine);
INIT VOID Device(BYTE * pLine);
INIT VOID Files(BYTE * pLine);
INIT VOID Fcbs(BYTE * pLine);
INIT VOID Lastdrive(BYTE * pLine);
INIT VOID Country(BYTE * pLine);
INIT VOID InitPgm(BYTE * pLine);
INIT VOID Switchar(BYTE * pLine);
INIT VOID CfgFailure(BYTE * pLine);
INIT VOID Stacks(BYTE * pLine);
INIT BYTE *GetNumArg(BYTE * pLine, COUNT * pnArg);
INIT BYTE *GetStringArg(BYTE * pLine, BYTE * pszString);
INIT struct dhdr FAR *linkdev(struct dhdr FAR * dhp);
INIT UWORD initdev(struct dhdr FAR * dhp, BYTE FAR * cmdTail);
INIT int SkipLine(char *pLine);

INIT static VOID FAR *AlignParagraph(VOID FAR * lpPtr);
#ifndef I86
#define AlignParagraph(x) (x)
#endif


#define EOF 0x1a

INIT struct table *LookUp(struct table *p, BYTE * token);

struct table
{
  BYTE *entry;
  BYTE pass;
    VOID(*func) (BYTE * pLine);
};

static struct table commands[] =
{
  {"break", 1, Break},
  {"buffers", 1, Buffers},
  {"command", 1, InitPgm},
  {"init", 1, InitPgm},
  {"country", 1, Country},
  {"device", 2, Device},
  {"mem", 1, Fcbs},
  {"files", 1, Files},
  {"lastdrive", 1, Lastdrive},
  {"rem", 0, CfgFailure},
  {"load", 2, Device},
  {"shell", 1, InitPgm},
  {"option", 1, Switchar},
  {"run", 1, InitPgm},
  {"swap", 1, Stacks},
  {"switchar", 1, Switchar},
  {"modo", 1, sysScreenMode}, 
  {"ver", 1, sysVersion},
  {"#", 0, CfgFailure},
  {"", -1, CfgFailure}
};

#ifndef KDB
INIT BYTE FAR *KernelAlloc(WORD nBytes);
INIT BYTE FAR *KernelAllocDma(WORD);
#endif

BYTE *pLineStart;

INIT void PreConfig(void)
{
  nPass = 0;

  lpOldLast = lpBase = AlignParagraph((BYTE FAR *) & last);

  dma_scratch = (BYTE FAR *)KernelAllocDma(BUFFERSIZE);
  buffers = (struct buffer FAR *)
      KernelAlloc(Config.cfgBuffers * sizeof(struct buffer));
#ifdef DEBUG
  printf(" \n", Config.cfgBuffers,
         FP_SEG(buffers), FP_OFF(buffers));
#endif

  f_nodes = (struct f_node FAR *)
      KernelAlloc(Config.cfgFiles * sizeof(struct f_node));
  FCBp = (sfttbl FAR *)
      KernelAlloc(sizeof(sftheader)
                  + Config.cfgFiles * sizeof(sft));
  sfthead = (sfttbl FAR *)
      KernelAlloc(sizeof(sftheader)
                  + Config.cfgFiles * sizeof(sft));


#ifdef DEBUG
  printf("system.c: Carga primaria terminada\n",
         FP_SEG(lpBase), FP_OFF(lpBase));
#endif

#ifdef KDB
  lpBase = malloc(4096);
  first_mcb = FP_SEG(lpBase) + ((FP_OFF(lpBase) + 0x0f) >> 4);
#else
  first_mcb = FP_SEG(lpBase) + ((FP_OFF(lpBase) + 0x0f) >> 4);
#endif

  mcb_init((mcb FAR *) (MK_FP(first_mcb, 0)),
           (ram_top << 6) - first_mcb - 1);
  nPass = 1;
}

INIT void PostConfig(void)
{
  nPass = 2;

  lpBase = AlignParagraph(lpOldLast);

  dma_scratch = (BYTE FAR *)KernelAllocDma(BUFFERSIZE);

  buffers = (struct buffer FAR *)
      KernelAlloc(Config.cfgBuffers * sizeof(struct buffer));
#ifdef DEBUG
  printf("%d buffers cargados\n", Config.cfgBuffers,
         FP_SEG(buffers), FP_OFF(buffers));
#endif

  f_nodes = (struct f_node FAR *)
      KernelAlloc(Config.cfgFiles * sizeof(struct f_node));
  FCBp = (sfttbl FAR *)
      KernelAlloc(sizeof(sftheader)
                  + Config.cfgFiles * sizeof(sft));
  sfthead = (sfttbl FAR *)
      KernelAlloc(sizeof(sftheader)
                  + Config.cfgFiles * sizeof(sft));


  if (Config.cfgStacks)
  {
    VOID FAR *stackBase = KernelAlloc(Config.cfgStacks * Config.cfgStackSize);
    init_stacks(stackBase, Config.cfgStacks, Config.cfgStackSize);

#ifdef  DEBUG
    printf("Swap cargados \n",
           FP_SEG(stackBase), FP_OFF(stackBase));
#endif
  }
}

INIT VOID configDone(VOID)
{
  COUNT i;

  first_mcb = FP_SEG(lpBase) + ((FP_OFF(lpBase) + 0x0f) >> 4);

  mcb_init((mcb FAR *) (MK_FP(first_mcb, 0)),
           (ram_top << 6) - first_mcb - 1);

}

INIT VOID DoConfig(VOID)
{
  COUNT nFileDesc;
  COUNT nRetCode;
  BYTE *pLine,
   *pTmp;
  BOOL bEof;

  if ((nFileDesc = dos_open((BYTE FAR *) ".\\ETC\\INITTAB", 0)) < 0)
  {
#ifdef DEBUG
    printf("system.c: No se encontro inittab, cargando rc_local.\n");
#endif
          if ((nFileDesc = dos_open((BYTE FAR *) ".\\ETC\\RC.D\\RC_LOCAL", 0)) < 0)
	  {
#ifdef DEBUG
            printf("system.c: No se encontro rc_local.\n");
#endif
	    return;
		}
  }

  nCfgLine = 0;
  bEof = 0;
  pLine = szLine;

  while (!bEof)
  {
    struct table *pEntry;
    UWORD bytesLeft = 0;

    if (pLine > szLine)
      bytesLeft = LINESIZE - (pLine - szLine);

    if (bytesLeft)
    {
      fbcopy(pLine, szLine, LINESIZE - bytesLeft);
      pLine = szLine + bytesLeft;
    }

    if ((nRetCode = dos_read(nFileDesc, pLine, LINESIZE - bytesLeft)) <= 0)
      break;


    if (nRetCode + bytesLeft < LINESIZE)
      szLine[nRetCode + bytesLeft] = EOF;
    pLine = szLine;

    while (!bEof && *pLine != EOF)
    {
      for (pTmp = pLine; pTmp - szLine < LINESIZE; pTmp++)
      {
        if (*pTmp == '\r' || *pTmp == EOF)
          break;
      }

      if (pTmp - szLine >= LINESIZE)
        break;

      if (*pTmp == EOF)
        bEof = TRUE;

      *pTmp = '\0';
      pLineStart = pLine;

      pLine = scan(pLine, szBuf);

      for (pTmp = szBuf; *pTmp != '\0'; pTmp++)
        *pTmp = tolower(*pTmp);

      if (*szBuf != '\0')
      {
        pEntry = LookUp(commands, szBuf);

        if (pEntry->pass < 0 || pEntry->pass == nPass)
        {
          if (!singleStep || !SkipLine(pLineStart))
          {
            skipwh(pLine);

            if ('=' != *pLine)
              CfgFailure(pLine);
            else
              (*(pEntry->func)) (++pLine);
          }
        }
      }
    skipLine:nCfgLine++;
      pLine += strlen(pLine) + 1;
    }
  }
  dos_close(nFileDesc);
}

INIT struct table *LookUp(struct table *p, BYTE * token)
{
  while (*(p->entry) != '\0')
  {
    if (strcmp(p->entry, token) == 0)
      break;
    else
      ++p;
  }
  return p;
}

INIT BOOL SkipLine(char *pLine)
{
  char kbdbuf[16];
  keyboard *kp = (keyboard *) kbdbuf;
  char *pKbd = &kp->kb_buf[0];

  kp->kb_size = 12;
  kp->kb_count = 0;

  printf("%s (S/N) ?", pLine);
  sti(kp);

  pKbd = skipwh(pKbd);

  if (*pKbd == 'n' || *pKbd == 'N')
    return TRUE;

  return FALSE;
}

INIT BYTE *GetNumArg(BYTE * pLine, COUNT * pnArg)
{
  pLine = skipwh(pLine);
  if (!isnum(pLine))
  {
    CfgFailure(pLine);
    return (BYTE *) 0;
  }
  return GetNumber(pLine, pnArg);
}

INIT BYTE *GetStringArg(BYTE * pLine, BYTE * pszString)
{
  pLine = skipwh(pLine);

  return scan(pLine, pszString);
}

INIT static VOID Buffers(BYTE * pLine)
{
  COUNT nBuffers;

  if (GetNumArg(pLine, &nBuffers) == (BYTE *) 0)
    return;

  Config.cfgBuffers = max(Config.cfgBuffers, nBuffers);
}

INIT static VOID sysScreenMode(BYTE * pLine)
{
  COUNT nMode;

  if (GetNumArg(pLine, &nMode) == (BYTE *) 0)
    return;

  if ((nMode != 0x11) && (nMode != 0x12) && (nMode != 0x14))
    return;

  _AX = (0x11 << 8) + nMode;
  _BL = 0;
  __int__(0x10);
}

INIT static VOID sysVersion(BYTE * pLine)
{
  COUNT major,minor;
  char *p;

  p = pLine;
  while(*p && *p != '.') p++;

  if (*p++ == '\0') return;

  if (GetNumArg(pLine, &major) == (BYTE *) 0)
    return;

  if (GetNumArg(p, &minor) == (BYTE *) 0)
    return;

  printf("Kernel %d.%d.%d\n",major,minor);

  os_major = major;
  os_minor = minor;
}

INIT static VOID Files(BYTE * pLine)
{
  COUNT nFiles;

  if (GetNumArg(pLine, &nFiles) == (BYTE *) 0)
    return;

  Config.cfgFiles = max(Config.cfgFiles, nFiles);
}

INIT static VOID Lastdrive(BYTE * pLine)
{
  COUNT nFiles;
  BYTE drv;

  pLine = skipwh(pLine);
  drv = *pLine & ~0x20;

  if (drv < 'A' || drv > 'Z')
  {
    CfgFailure(pLine);
    return;
  }
  drv -= 'A';
  Config.cfgLastdrive = max(Config.cfgLastdrive, drv);
}

INIT static VOID Switchar(BYTE * pLine)
{

  GetStringArg(pLine, szBuf);
  switchar = *szBuf;
}

INIT static VOID Fcbs(BYTE * pLine)
{
  COUNT fcbs;

  if ((pLine = GetNumArg(pLine, &fcbs)) == 0)
    return;
  Config.cfgFcbs = fcbs;

  pLine = skipwh(pLine);

  if (*pLine == ',')
  {
    GetNumArg(++pLine, &fcbs);
    Config.cfgProtFcbs = fcbs;
  }

  if (Config.cfgProtFcbs > Config.cfgFcbs)
    Config.cfgProtFcbs = Config.cfgFcbs;
}

INIT static VOID Country(BYTE * pLine)
{
  COUNT ctryCode;
  COUNT codePage;

  if ((pLine = GetNumArg(pLine, &ctryCode)) == 0)
    return;

  pLine = skipwh(pLine);
  if (*pLine == ',')
  {
    pLine = skipwh(pLine);

    if (*pLine == ',')
    {
      codePage = 0;
      ++pLine;
    }
    else
    {
      if ((pLine = GetNumArg(pLine, &codePage)) == 0)
        return;
    }

    pLine = skipwh(pLine);
    if (*pLine == ',')
    {
      GetStringArg(++pLine, szBuf);

      if (LoadCountryInfo(szBuf, ctryCode, codePage))
        return;
    }
  }
  CfgFailure(pLine);
}

INIT static VOID Stacks(BYTE * pLine)
{
  COUNT stacks;

  pLine = GetNumArg(pLine, &stacks);
  Config.cfgStacks = stacks;

  pLine = skipwh(pLine);

  if (*pLine == ',')
  {
    GetNumArg(++pLine, &stacks);
    Config.cfgStackSize = stacks;
  }

  if (Config.cfgStacks)
  {
    if (Config.cfgStackSize < 32)
      Config.cfgStackSize = 32;
    if (Config.cfgStackSize > 512)
      Config.cfgStackSize = 512;
    if (Config.cfgStacks > 64)
      Config.cfgStacks = 64;
  }
}

INIT static VOID InitPgm(BYTE * pLine)
{
  pLine = GetStringArg(pLine, Config.cfgInit);

  strcpy(Config.cfgInitTail, pLine);

  strcat(Config.cfgInitTail, "\r\n");
}

INIT static VOID Break(BYTE * pLine)
{
  BYTE *pTmp;

  GetStringArg(pLine, szBuf);
  break_ena = strcmp(szBuf, "OFF") ? 1 : 0;
}

INIT static VOID Device(BYTE * pLine)
{
  VOID FAR *driver_ptr;
  BYTE *pTmp;
  exec_blk eb;
  struct dhdr FAR *dhp;
  struct dhdr FAR *next_dhp;
  UWORD dev_seg = (((ULONG) FP_SEG(lpBase) << 4) + FP_OFF(lpBase) + 0xf) >> 4;

  GetStringArg(pLine, szBuf);

  eb.load.reloc = eb.load.load_seg = dev_seg;
  dhp = MK_FP(dev_seg, 0);

#ifdef DEBUG
  printf("system.c: Cargando el driver %s en memoria\n",
         szBuf, dev_seg);
#endif

  if (DosExec(3, &eb, szBuf) == SUCCESS)
  {
    while (FP_OFF(dhp) != 0xFFFF)
    {
      next_dhp = MK_FP(FP_SEG(dhp), FP_OFF(dhp->dh_next));
      dhp->dh_next = nul_dev.dh_next;
      link_dhdr(&nul_dev, dhp, pLine);
      dhp = next_dhp;
    }
  }
  else
    CfgFailure(pLine);
}

INIT static VOID CfgFailure(BYTE * pLine)
{
  BYTE *pTmp = pLineStart;

  printf("system.c: Error en la linea %d de INITTAB.\n", nCfgLine);
  printf("=>%s\n", pTmp);
  while (++pTmp != pLine)
    printf(" ");
  printf("^\n");
}

#ifndef KDB
INIT static BYTE FAR *KernelAlloc(WORD nBytes)
{
  BYTE FAR *lpAllocated;

  lpBase = AlignParagraph(lpBase);
  lpAllocated = lpBase;

  if (0x10000 - FP_OFF(lpBase) <= nBytes)
  {
    UWORD newOffs = (FP_OFF(lpBase) + nBytes) & 0xFFFF;
    UWORD newSeg = FP_SEG(lpBase) + 0x1000;

    lpBase = MK_FP(newSeg, newOffs);
  }
  else
    lpBase += nBytes;

  return lpAllocated;
}
#endif

#ifdef I86
INIT static BYTE FAR *KernelAllocDma(WORD bytes)
{
  BYTE FAR *allocated;

  lpBase = AlignParagraph(lpBase);
  if ((FP_SEG(lpBase) & 0x0fff) + (bytes >> 4) > 0x1000)
    lpBase = MK_FP((FP_SEG(lpBase) + 0x0fff) & 0xf000, 0);
  allocated = lpBase;
  lpBase += bytes;
  return allocated;
}

INIT static VOID FAR *AlignParagraph(VOID FAR * lpPtr)
{
  ULONG lTemp;
  UWORD uSegVal;

  lTemp = FP_SEG(lpPtr);
  lTemp = (lTemp << 4) + FP_OFF(lpPtr);

  lTemp += 0x0f;
  lTemp &= 0xfffffff0l;

  uSegVal = (UWORD) (lTemp >> 4);

  return MK_FP(uSegVal, 0);
}
#endif

INIT BYTE *
  skipwh(BYTE * s)
{
  while (*s && (*s == 0x0d || *s == 0x0a || *s == ' ' || *s == '\t'))
    ++s;
  return s;
}

INIT BYTE *
  scan(BYTE * s, BYTE * d)
{
  s = skipwh(s);
  while (*s &&
         !(*s == 0x0d
           || *s == 0x0a
           || *s == ' '
           || *s == '\t'
           || *s == '='))
    *d++ = *s++;
  *d = '\0';
  return s;
}

INIT BYTE *scan_seperator(BYTE * s, BYTE * d)
{
  s = skipwh(s);
  if (*s)
    *d++ = *s++;
  *d = '\0';
  return s;
}

INIT BOOL isnum(BYTE * pLine)
{
  return (*pLine >= '0' && *pLine <= '9');
}

INIT BYTE *GetNumber(REG BYTE * pszString, REG COUNT * pnNum)
{
  BYTE Base = 10;

  *pnNum = 0;
  while (isnum(pszString) || toupper(*pszString) == 'X')
  {
    if (toupper(*pszString) == 'X')
    {
      Base = 16;
      pszString++;
    }
    else
      *pnNum = *pnNum * Base + (*pszString++ - '0');
  }
  return pszString;
}

INIT COUNT tolower(COUNT c)
{
  if (c >= 'A' && c <= 'Z')
    return (c + ('a' - 'A'));
  else
    return c;
}

INIT COUNT toupper(COUNT c)
{
  if (c >= 'a' && c <= 'z')
    return (c - ('a' - 'A'));
  else
    return c;
}


#ifdef KERNEL
INIT VOID
mcb_init(mcb FAR * mcbp, UWORD size)
{
  COUNT i;

  mcbp->m_type = MCB_LAST;
  mcbp->m_psp = FREE_PSP;
  mcbp->m_size = size;
  for (i = 0; i < 8; i++)
    mcbp->m_name[i] = '\0';
  mem_access_mode = FIRST_FIT;
}
#endif

INIT VOID
strcat(REG BYTE * d, REG BYTE * s)
{
  while (*d != 0)
    ++d;
  strcpy(d, s);
}
