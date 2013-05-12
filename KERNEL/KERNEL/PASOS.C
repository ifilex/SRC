#include "port.h"
#include "todo.h"

VOID scopy(REG BYTE * s, REG BYTE * d)
{
  while (*s)
    *d++ = *s++;
  *d = '\0';
}

VOID FAR init_call_scopy(REG BYTE * s, REG BYTE * d)
{
  scopy(s, d);
}

VOID fscopy(REG BYTE FAR * s, REG BYTE FAR * d)
{
  while (*s)
    *d++ = *s++;
  *d = '\0';
}

VOID fsncopy(BYTE FAR * s, BYTE FAR * d, REG COUNT n)
{
  while (*s && n--)
    *d++ = *s++;
  *d = '\0';
}

#ifndef ASMSUPT
VOID bcopy(REG BYTE * s, REG BYTE * d, REG COUNT n)
{
  while (n--) *d++ = *s++;
}

VOID fbcopy(REG VOID FAR * s, REG VOID FAR * d, REG COUNT n)
{
  while (n--) *((BYTE FAR *) d)++ = *((BYTE FAR *) s)++;
}

VOID fmemset(REG VOID FAR * s, REG int ch, REG COUNT n)
{
  while(n--) *((BYTE FAR *)s)++ = ch;
}

#endif

VOID FAR init_call_fbcopy(REG VOID FAR * s, REG VOID FAR * d, REG COUNT n)
{
  fbcopy(s, d, n);
}

VOID fmemset(VOID FAR *, int, COUNT);

VOID FAR init_call_fmemset(REG VOID FAR * s, REG int ch, REG COUNT n)
{
  fmemset(s, ch, n);
}
