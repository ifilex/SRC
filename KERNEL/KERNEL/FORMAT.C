#include "port.h"
#include "todo.h"
#include "proto.h"

int SetJFTSize(UWORD nHandles)
{
  UWORD block,
    maxBlock;
  psp FAR *ppsp = MK_FP(cu_psp, 0);
  UBYTE FAR *newtab;
  COUNT i;

  if (nHandles <= ppsp->ps_maxfiles)
  {
    ppsp->ps_maxfiles = nHandles;
    return SUCCESS;
  }

  if ((DosMemAlloc((nHandles + 0xf) >> 4, mem_access_mode, &block, &maxBlock)) < 0)
    return DE_NOMEM;

  ++block;
  newtab = MK_FP(block, 0);

  for (i = 0; i < ppsp->ps_maxfiles; i++)
    newtab[i] = ppsp->ps_filetab[i];

  for (; i < nHandles; i++)
    newtab[i] = 0xff;

  ppsp->ps_maxfiles = nHandles;
  ppsp->ps_filetab = newtab;

  return SUCCESS;
}

int DosMkTmp(BYTE FAR * pathname, UWORD attr) // iniciamos las rutas
{
  static const char tokens[] = "0123456789ABCDEF";
  char FAR *ptmp = pathname;
  BYTE wd,
    month,
    day;
  BYTE h,
    m,
    s,
    hund;
  WORD sh;
  WORD year;
  int rc;

  while (*ptmp)
    ptmp++;

  if (ptmp == pathname || (ptmp[-1] != '\\' && ptmp[-1] != '/'))
    *ptmp++ = '\\';

  DosGetDate(&wd, &month, &day, (COUNT FAR *) & year);
  DosGetTime(&h, &m, &s, &hund);

  sh = s * 100 + hund;

  ptmp[0] = tokens[year & 0xf];
  ptmp[1] = tokens[month];
  ptmp[2] = tokens[day & 0xf];
  ptmp[3] = tokens[h & 0xf];
  ptmp[4] = tokens[m & 0xf];
  ptmp[5] = tokens[(sh >> 8) & 0xf];
  ptmp[6] = tokens[(sh >> 4) & 0xf];
  ptmp[7] = tokens[sh & 0xf];
  ptmp[8] = '.';
  ptmp[9] = 'A';
  ptmp[10] = 'A';
  ptmp[11] = 'A';
  ptmp[12] = 0;

  while ((rc = DosOpen(pathname, 0)) >= 0)
  {
    DosClose(rc);

    if (++ptmp[11] > 'Z')
    {
      if (++ptmp[10] > 'Z')
      {
        if (++ptmp[9] > 'Z')
          return DE_TOOMANY;

        ptmp[10] = 'A';
      }
      ptmp[11] = 'A';
    }
  }

  if (rc == DE_FILENOTFND)
  {
    rc = DosCreat(pathname, attr);
  }
  return rc;
}

int truename(char FAR * src, char FAR * dest) 
{
  static char buf[128] = "C:\\";  // unidad por defecto a ser cargada..
  char *bufp = buf + 3;
  BYTE far *test;

  src = adjust_far(src);

  if (src[1] == ':')
  {
    buf[0] = (src[0] | 0x20) + 'A' - 'a';

    if (buf[0] >= nblkdev + 'A')
      return DE_PATHNOTFND;

    src += 2;
  }
  else
    buf[0] = default_drive + 'A';

  if (*src != '\\' && *src != '/')    
  {
    DosGetCuDir(buf[0] - '@', bufp);

    if (*bufp)
    {
      while (*bufp)
        bufp++;
      *bufp++ = '\\';
    }
  }
  else
    src++;

  while (*src)
  {
    char c;

    switch ((c = *src++))
    {
      case '/':               
      case '\\':

	if (bufp[-1] != '\\')
	  *bufp++ = '\\';
        break;

      case '.':
        if (bufp[-1] == '\\')
        {
          if (*src == '.' && (src[1] == '/' || src[1] == '\\' || !src[1]))
          {

            for (bufp -= 2; *bufp != '\\'; bufp--)
            {
              if (bufp < buf + 2)      
                return DE_PATHNOTFND;
            }
            src++;
            if (bufp[-1] == ':')
              bufp++;
          }
          else if (*src == '/' || *src == '\\' || *src == 0)
	    --bufp;
        }
        else
          *bufp++ = c;
        break;

      default:
        *bufp++ = c;
        break;
    }
  }
  while (bufp[-1] == '\\')
    --bufp;
  if (bufp == buf + 2)
    ++bufp;
  *bufp++ = 0;
  upString(buf);

  fbcopy(buf, dest, bufp - buf);
  return SUCCESS;
}
