#include "inicio.h"
#include "port.h"
#include "todo.h"

#ifdef __TURBOC__
void __int__(int);  
#endif

/* intento por emular las funciones oem.. */

UWORD init_oem(void)
{
  UWORD top_k;

#ifndef __TURBOC__
  _asm
  {
    int 12h
      mov top_k, ax
  }
#else
  __int__(0x12);
  top_k = _AX;
#endif
  return top_k;
}
