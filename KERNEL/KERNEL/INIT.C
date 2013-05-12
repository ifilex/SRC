#include "port.h"

COUNT strlen(REG BYTE * s)
{
  REG WORD cnt = 0;

  while (*s++ != 0)
    ++cnt;
  return cnt;
}

COUNT FAR init_call_strlen(REG BYTE * s)
{
  return strlen(s);
}

COUNT fstrlen(REG BYTE FAR * s)
{
  REG WORD cnt = 0;

  while (*s++ != 0)
    ++cnt;
  return cnt;
}

VOID _fstrcpy(REG BYTE FAR * d, REG BYTE FAR * s)
{
  while (*s != 0)
    *d++ = *s++;
  *d = 0;
}

VOID strncpy(REG BYTE * d, REG BYTE * s, COUNT l)
{
  COUNT idx = 1;
  while (*s != 0 && idx++ <= l)
    *d++ = *s++;
  *d = 0;
}

COUNT strcmp(REG BYTE * d, REG BYTE * s)
{
  while (*s != '\0' && *d != '\0')
  {
    if (*d == *s)
      ++s, ++d;
    else
      return *d - *s;
  }
  return *d - *s;
}

COUNT FAR init_call_strcmp(REG BYTE * d, REG BYTE * s)
{
  return strcmp(d, s);
}

COUNT fstrcmp(REG BYTE FAR * d, REG BYTE FAR * s)
{
  while (*s != '\0' && *d != '\0')
  {
    if (*d == *s)
      ++s, ++d;
    else
      return *d - *s;
  }
  return *d - *s;
}

COUNT strncmp(REG BYTE * d, REG BYTE * s, COUNT l)
{
  COUNT index = 1;
  while (*s != '\0' && *d != '\0' && index++ <= l)
  {
    if (*d == *s)
      ++s, ++d;
    else
      return *d - *s;
  }
  return *d - *s;
}

COUNT fstrncmp(REG BYTE FAR * d, REG BYTE FAR * s, COUNT l)
{
  COUNT index = 1;
  while (*s != '\0' && *d != '\0' && index++ <= l)
  {
    if (*d == *s)
      ++s, ++d;
    else
      return *d - *s;
  }
  return *d - *s;
}

VOID fstrncpy(REG BYTE FAR * d, REG BYTE FAR * s, COUNT l)
{
  COUNT idx = 1;
  while (*s != 0 && idx++ <= l)
    *d++ = *s++;
  *d = 0;
}

BYTE *strchr(BYTE * s, BYTE c)
{
  REG BYTE *p;
  p = s - 1;
  do {
    if (*++p == c)
      return p;
  } while (*p);
  return 0;
}
