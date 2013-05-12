#include "port.h"
#include "todo.h"

VOID dump(void)
{
  printf("falla.c: Error [AH = %02x CS:IP = %04x:%04x]\n",
         error_regs.AH,
         error_regs.CS,
         error_regs.IP);
  printf("falla.c: AX:%04x BX:%04x CX:%04x DX:%04x\n",
         error_regs.AX,
         error_regs.BX,
         error_regs.CX,
         error_regs.DX);
  printf("falla.c: SI:%04x DI:%04x DS:%04x ES:%04x\n",
         error_regs.SI,
         error_regs.DI,
         error_regs.DS,
         error_regs.ES);
}

VOID panic(BYTE * s)
{
  printf("\nfalla.c: ATENCION %s\nSistema parado\n", s);
  for (;;);
}

#ifdef IPL
VOID fatal(BYTE * err_msg)
{
  printf("\nfalla.c: Error interno de INITTAB - %s\nSistema parado\n", err_msg);
  exit(-1);
}
#else
VOID fatal(BYTE * err_msg)
{
  printf("\nfalla.c: Error - %s\nSistema parado\n", err_msg);
  for (;;);
}

VOID FAR init_call_fatal(BYTE * err_msg)
{
  fatal(err_msg);
}
#endif

COUNT char_error(request * rq, struct dhdr FAR * lpDevice)
{
  return CriticalError(
                        EFLG_CHAR | EFLG_ABORT | EFLG_RETRY | EFLG_IGNORE,
                        0,
                        rq->r_status & S_MASK,
                        lpDevice);
}

COUNT block_error(request * rq, COUNT nDrive, struct dhdr FAR * lpDevice)
{
  return CriticalError(
                        EFLG_ABORT | EFLG_RETRY | EFLG_IGNORE,
                        nDrive,
                        rq->r_status & S_MASK,
                        lpDevice);
}
