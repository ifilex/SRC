#include "port.h"
static BYTE *charp;

#ifdef PROTO
VOID handle_char(COUNT);
VOID put_console(COUNT);
BYTE *ltob(LONG, BYTE *, COUNT);
static BYTE *itob(COUNT, BYTE *, COUNT);
COUNT do_printf(CONST BYTE *, REG BYTE **);
#else
VOID handle_char();
VOID put_console();
BYTE *ltob();
static BYTE *itob();
COUNT do_printf();
#endif

#ifdef PROTO
VOID cso(COUNT);
#else
VOID cso();
#endif

VOID
put_console(COUNT c)
{
  if (c == '\n')
    cso('\r');
  cso(c);
}

static VOID
  handle_char(COUNT c)
{
  if (charp == 0)
    put_console(c);
  else
    *charp++ = c;
}

static BYTE *
  ltob(LONG n, BYTE * s, COUNT base)
{
  ULONG u;
  REG BYTE *p,
   *q;
  REG negative,
    c;

  if (n < 0 && base == -10)
  {
    negative = 1;
    u = -n;
  }
  else
  {
    negative = 0;
    u = n;
  }
  if (base == -10)             
    base = 10;
  p = q = s;
  do
  {                          
    *p++ = "0123456789abcdef"[u % base];
  }
  while ((u /= base) > 0);
  if (negative)
    *p++ = '-';
  *p = '\0';                  
  while (q < --p)
  {                           
    c = *q;
    *q++ = *p;
    *p = c;
  }
  return s;
}

static BYTE *
  itob(COUNT n, BYTE * s, COUNT base)
{
  UWORD u;
  REG BYTE *p,
   *q;
  REG negative,
    c;

  if (n < 0 && base == -10)
  {
    negative = 1;
    u = -n;
  }
  else
  {
    negative = 0;
    u = n;
  }
  if (base == -10)            
    base = 10;
  p = q = s;
  do
  {                           
    *p++ = "0123456789abcdef"[u % base];
  }
  while ((u /= base) > 0);
  if (negative)
    *p++ = '-';
  *p = '\0';                   
  while (q < --p)
  {                           
    c = *q;
    *q++ = *p;
    *p = c;
  }
  return s;
}

#define NONE    0
#define LEFT    1
#define RIGHT   2

WORD FAR
init_call_printf(CONST BYTE * fmt, BYTE * args)
{
  charp = 0;
  return do_printf(fmt, &args);
}

WORD
sprintf(BYTE * buff, CONST BYTE * fmt, BYTE * args)
{
  WORD ret;

  charp = buff;
  ret = do_printf(fmt, &args);
  handle_char(NULL);
  return ret;
}

static COUNT
  do_printf(CONST BYTE * fmt, REG BYTE ** arg)
{
  REG base;
  BYTE s[11],
   *p,
   *ltob();
  BYTE c,
    slen,
    flag,
    size,
    fill;

  flag = NONE;
  size = 0;
  while ((c = *fmt++) != '\0')
  {
    if (size == 0 && flag == NONE && c != '%')
    {
      handle_char(c);
      continue;
    }
    if (flag == NONE && *fmt == '0')
    {
      flag = RIGHT;
      fill = '0';
    }
    switch (*fmt)
    {
      case '-':
        flag = RIGHT;
        fill = *(fmt + 1) == '0' ? '0' : ' ';
        continue;

      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
        if (flag == NONE)
          flag = LEFT;
        size = *fmt++ - '0';
        while ((c = *fmt++) != '\0')
        {
          switch (c)
          {
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
              size = size * 10 + (c - '0');
              continue;

            default:
              --fmt;
              break;
          }
          break;
        }
        break;
    }
    switch (c = *fmt++)
    {
      case 'c':
        handle_char(*(COUNT *) arg++);
        continue;

      case 'd':
        base = -10;
        goto prt;

      case 'o':
        base = 8;
        goto prt;

      case 'u':
        base = 10;
        goto prt;

      case 'x':
        base = 16;

      prt:
        itob(*((COUNT *) arg)++, s, base);
        if (flag == RIGHT || flag == LEFT)
        {
          for (slen = 0, p = s; *p != '\0'; p++)
            ++slen;
        }
        if (flag == RIGHT && slen < size)
        {
          WORD i;

          for (i = size - slen; i > 0; i--)
            handle_char(fill);
        }
        for (p = s; *p != '\0'; p++)
          handle_char(*p);
        if (flag == LEFT)
        {
          WORD i;
          BYTE sp = ' ';

          for (i = size - slen; i > 0; i--)
            handle_char(sp);
        }
        size = 0;
        flag = NONE;
        continue;

      case 'l':
        switch (c = *fmt++)
        {
          case 'd':
            base = -10;
            goto lprt;

          case 'o':
            base = 8;
            goto lprt;

          case 'u':
            base = 10;
            goto lprt;

          case 'x':
            base = 16;

          lprt:
            ltob(*((LONG *) arg)++, s, base);
            if (flag == RIGHT || flag == LEFT)
            {
              for (slen = 0, p = s; *p != '\0'; p++)
                ++slen;
            }
            if (flag == RIGHT && slen < size)
            {
              WORD i;

              for (i = size - slen; i > 0; i--)
                handle_char(fill);
            }
            for (p = s; *p != '\0'; p++)
              handle_char(*p);
            if (flag == LEFT)
            {
              WORD i;
              BYTE sp = ' ';

              for (i = size - slen; i > 0; i--)
                handle_char(sp);
            }
            size = 0;
            flag = NONE;
            continue;

          default:
            handle_char(c);
        }

      case 's':
        for (p = *arg; *p != '\0'; p++)
        {
          --size;
          handle_char(*p);
        }
        for (; size > 0; size--)
          handle_char(' ');
        ++arg;
        size = 0;
        flag = NONE;
        continue;

      default:
        handle_char(c);
        continue;
    }
  }
  return 0;
}
