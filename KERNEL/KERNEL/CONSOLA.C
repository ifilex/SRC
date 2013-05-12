#include "port.h"
#include "todo.h"

#define toupper(c)	((c) >= 'a' && (c) <= 'z' ? (c) + ('A' - 'a') : (c))

#define LOADNGO 0
#define LOAD    1
#define OVERLAY 3

static exe_header header;

#define CHUNK 32256
#define MAXENV 32768
#define ENV_KEEPFREE 83 

#ifndef PROTO
COUNT ChildEnv(exec_blk FAR *, UWORD *, char far *);
#else
COUNT ChildEnv();
#endif

COUNT ChildEnv(exec_blk FAR * exp, UWORD * pChildEnvSeg, char far * pathname)
{
  BYTE FAR *pSrc;
  BYTE FAR *pDest;
  UWORD nEnvSize;
  COUNT RetCode;
  psp FAR *ppsp = MK_FP(cu_psp, 0);

  pDest = pSrc = exp->exec.env_seg ?
      MK_FP(exp->exec.env_seg, 0) :
      MK_FP(ppsp->ps_environ, 0);

#if 0
  if (!pSrc)                 
  {
    *pChildEnvSeg = 0;
    return SUCCESS;
  }
#endif

	nEnvSize = 1;
  if(pSrc) {          
  	while (*pSrc != '\0')
	  {
		while (*pSrc != '\0' && pSrc < pDest + MAXENV - ENV_KEEPFREE)
		{
		  ++pSrc;
		  ++nEnvSize;
		}
		++nEnvSize;
		++pSrc;
	  }
	  pSrc = pDest;
  }

  if (nEnvSize >= MAXENV)
	return DE_INVLDENV;

  if ((RetCode = DosMemAlloc(long2para(nEnvSize + ENV_KEEPFREE),
                             FIRST_FIT, (seg FAR *) pChildEnvSeg,
                             NULL /*(UWORD FAR *) MaxEnvSize ska*/)) < 0)
    return RetCode;
  pDest = MK_FP(*pChildEnvSeg + 1, 0);

  if(pSrc)
  {
	  fbcopy(pSrc, pDest, nEnvSize);
	  pDest += nEnvSize;
  }
  else
        *pDest++ = '\0';               

#if 0
  for (; *pSrc != '\0';)
  {
    while (*pSrc)
    {
      *pDest++ = *pSrc++;
    }
    pSrc++;
    *pDest++ = 0;
  }
  *pDest++ = 0;
#endif
  *((UWORD FAR*)pDest)++ = 1;
  truename(pathname, pDest);

  return SUCCESS;
}
VOID new_psp(psp FAR * p, int psize)
{
  REG COUNT i;
  BYTE FAR *lpPspBuffer;
  psp FAR *q = MK_FP(cu_psp, 0);
  for (lpPspBuffer = (BYTE FAR *) p, i = 0; i < sizeof(psp); ++i)
    *lpPspBuffer = 0;
  p->ps_exit = 0x20cd;
  p->ps_farcall = 0xea;
  p->ps_reentry = cpm_entry;
  p->ps_unix[0] = 0xcd;
  p->ps_unix[1] = 0x21;
  p->ps_unix[2] = 0xcb;
  p->ps_parent = cu_psp;
  p->ps_prevpsp = q;
  p->ps_size = psize;
  p->ps_environ = 0;
  p->ps_dta = (BYTE FAR *) (&p->ps_cmd_count);
  p->ps_isv22 = (VOID(interrupt FAR *) (void))getvec(0x22);
  p->ps_isv23 = (VOID(interrupt FAR *) (void))getvec(0x23);
  p->ps_isv24 = (VOID(interrupt FAR *) (void))getvec(0x24);
  p->ps_stack = (BYTE FAR *) - 1;
  for (i = 0; i < 20; i++)
    p->ps_files[i] = 0xff;
  p->ps_maxfiles = 20;
  p->ps_filetab = p->ps_files;
  if (InDOS > 0)
  {
    REG COUNT i;

    for (i = 0; i < 20; i++)
    {
      if (q->ps_filetab[i] != 0xff && CloneHandle(i) >= 0)
        p->ps_filetab[i] = q->ps_filetab[i];
      else
        p->ps_filetab[i] = 0xff;
    }
  }
  else
  {
    p->ps_files[STDIN] = 0;    
    p->ps_files[STDOUT] = 1;  
    p->ps_files[STDERR] = 2;    
    p->ps_files[STDAUX] = 3;  
    p->ps_files[STDPRN] = 4;  
  }

  p->ps_fcb1.fcb_drive = 0;
  p->ps_fcb2.fcb_drive = 0;
  p->ps_cmd_count = 0;      
  p->ps_cmd[0] = 0;         
  if (RootPsp == (seg) ~ 0)
    RootPsp = FP_SEG(p);
}

static VOID patchPSP(UWORD pspseg, UWORD envseg, CommandTail FAR *cmdline
 , BYTE FAR * fnam)
{	psp FAR *psp;
	mcb FAR *pspmcb;
	int i;
	BYTE FAR * np;

	pspmcb = MK_FP(pspseg, 0);
	++pspseg;
	psp = MK_FP(pspseg, 0);

  fbcopy(cmdline->ctBuffer, psp->ps_cmd, 127);
  psp->ps_cmd_count = cmdline->ctCount;

  pspmcb->m_psp = pspseg;
  if (envseg) {
  	  psp->ps_environ = envseg + 1;
    ((mcb FAR *)MK_FP(envseg, 0))->m_psp = pspseg;
  } else
	  psp->ps_environ = 0;

  np = fnam;
  for (;;) {
    switch (*fnam++) {
      case '\0': goto set_name;
      case ':' :
      case '/' :
      case '\\': np = fnam;
    }
  }
set_name:
  for (i = 0; i < 8 && np[i] != '.' && np[i] != '\0'; i++)
  {
	  pspmcb->m_name[i] = toupper(np[i]);
  }
  if (i < 8)
    pspmcb->m_name[i] = '\0';

}

static COUNT DosComLoader(BYTE FAR * namep, exec_blk FAR * exp, COUNT mode)
{
  COUNT rc,
    env_size;
  COUNT nread;
  UWORD mem;
  UWORD env,
    asize;
  BYTE FAR *sp;
  psp FAR *p;
  psp FAR *q = MK_FP(cu_psp, 0);
  iregs FAR *irp;
  LONG com_size;

  if (mode != OVERLAY)
  {

    if ((rc = ChildEnv(exp, &env, namep)) != SUCCESS)
    {
      return rc;
    }
    if ((rc = DosMemAlloc(0, LARGEST, (seg FAR *) & mem, (UWORD FAR *) & asize)) < 0)
    {
      DosMemFree(env);          
      return rc;
    }

    ++mem;
  }
  else
    mem = exp->load.load_seg;

  if ((rc = dos_open(namep, 0)) < 0)
    fatal("(DosComLoader) archivo com perdido en la trasmicion");

  if((com_size = dos_getfsize(rc)) != 0) {
          if(mode == OVERLAY)        
		sp = MK_FP(mem, 0);
          else {                                       
		sp = MK_FP(mem, sizeof(psp));

                if(com_size > (LONG)asize << 4)  
			com_size = (LONG)asize << 4;
	  }

	  do {
		nread = dos_read(rc, sp, CHUNK);
		sp = add_far((VOID FAR *) sp, (ULONG) nread);
	  } while((com_size -= nread) > 0 && nread == CHUNK);
  }

  dos_close(rc);

  if (mode == OVERLAY)
    return SUCCESS;

  p = MK_FP(mem, 0);
  setvec(0x22, (VOID(INRPT FAR *) (VOID)) MK_FP(user_r->CS, user_r->IP));
  new_psp(p, mem + asize);

  patchPSP(mem - 1, env, exp->exec.cmd_line, namep);

  *((UWORD FAR *) MK_FP(mem, 0xfffe)) = (UWORD) 0;
  irp = MK_FP(mem, (0xfffe - sizeof(iregs)));

  irp->ES = irp->DS = mem;
  irp->CS = mem;
  irp->IP = 0x100;
  irp->AX = 0xffff;           
  irp->BX =
      irp->CX =
      irp->DX =
      irp->SI =
      irp->DI =
      irp->BP = 0;
  irp->FLAGS = 0x200;

  p->ps_parent = cu_psp;
  p->ps_prevpsp = (BYTE FAR *) MK_FP(cu_psp, 0);
  q->ps_stack = (BYTE FAR *) user_r;
  user_r->FLAGS &= ~FLG_CARRY;

  switch (mode)
  {
    case LOADNGO:
      {
        cu_psp = mem;
        dta = p->ps_dta;
        if (InDOS)
          --InDOS;
        exec_user(irp);

        fatal("Error de sistema!");
        break;
      }
    case LOAD:
      cu_psp = mem;
      exp->exec.stack = (BYTE FAR *) irp;
      exp->exec.start_addr = MK_FP(irp->CS, irp->IP);
      return SUCCESS;
  }

  return DE_INVLDFMT;
}

VOID return_user(void)
{
  psp FAR *p,
    FAR * q;
  REG COUNT i;
  iregs FAR *irp;
  long j;

  p = MK_FP(cu_psp, 0);

  setvec(0x22, p->ps_isv22);
  setvec(0x23, p->ps_isv23);
  setvec(0x24, p->ps_isv24);

  if (!tsr)
    {
    for (i = 0; i < p->ps_maxfiles; i++)
    {
      DosClose(i);
    }
    FreeProcessMem(cu_psp);
  }

  cu_psp = p->ps_parent;
  q = MK_FP(cu_psp, 0);
  dta = q->ps_dta;

  irp = (iregs FAR *) q->ps_stack;

  irp->CS = FP_SEG(p->ps_isv22);
  irp->IP = FP_OFF(p->ps_isv22);

  if (InDOS)
    --InDOS;
  exec_user((iregs FAR *) q->ps_stack);
}

static COUNT DosExeLoader(BYTE FAR * namep, exec_blk FAR * exp, COUNT mode)
{
  COUNT rc,
    env_size,
    i;
  COUNT nBytesRead;
  UWORD mem,
    env,
    asize,
    start_seg;
  ULONG image_size;
  ULONG image_offset;
  BYTE FAR *sp;
  psp FAR *p;
  psp FAR *q = MK_FP(cu_psp, 0);
  mcb FAR *mp;
  iregs FAR *irp;
  UWORD reloc[2];
  seg FAR *spot;
  LONG exe_size;

  if (mode != OVERLAY)
  {
    if ((rc = ChildEnv(exp, &env, namep)) != SUCCESS)
      return rc;
  }
  else
    mem = exp->load.load_seg;

  image_offset = (ULONG) header.exHeaderSize * 16l;

  image_size = (ULONG) (header.exPages) * 512l;
  image_size -= image_offset;
  if (mode != OVERLAY)
    image_size += (ULONG) long2para((LONG) sizeof(psp));

  if (mode != OVERLAY)
  {
    if ((rc = DosMemLargest((seg FAR *) & asize)) != SUCCESS) {
      DosMemFree(env);
      return rc;
    }

    exe_size = (LONG) long2para(image_size) + header.exMinAlloc;
    if (exe_size > asize) {
      	DosMemFree(env);
        return DE_NOMEM;
    }
    exe_size = (LONG) long2para(image_size) + header.exMaxAlloc;
    if (exe_size > asize)
        exe_size = asize;
  }

    if ((rc = DosMemAlloc((seg) exe_size, FIRST_FIT, (seg FAR *) & mem
     , (UWORD FAR *) & asize)) < 0)
    {
      if (rc == DE_NOMEM)
      {
        if ((rc = DosMemAlloc(0, LARGEST, (seg FAR *) & mem
         , (UWORD FAR *) & asize)) < 0)
        {
          DosMemFree(env);
          return rc;
        }
        if (asize < exe_size)
        {
          DosMemFree(mem);
          DosMemFree(env);
          return rc;
        }
      }
      else
      {
        DosMemFree(env);
        return rc;
      }
    }
    else
      asize = exe_size;
  if (mode != OVERLAY)
  {
    mp = MK_FP(mem, 0);
    ++mem;
  }
  else
    mem = exp->load.load_seg;

  if (mode == OVERLAY)
    start_seg = mem;
  else
  {
    start_seg = mem + long2para((LONG) sizeof(psp));
  }

  if ((rc = dos_open(namep, 0)) < 0)
  {
    fatal("(DosExeLoader) archivo exe perdido en la transferencia");
  }
  if (dos_lseek(rc, image_offset, 0) != image_offset)
  {
    if (mode != OVERLAY)
    {
      DosMemFree(--mem);
      DosMemFree(env);
    }
    return DE_INVLDDATA;
  }

  if (mode != OVERLAY)
  {
    exe_size = image_size - long2para((LONG) sizeof(psp));
  }
  else
    exe_size = image_size;

  if(exe_size > 0) {
	  sp = MK_FP(start_seg, 0x0);

	  if (mode != OVERLAY)
	  {
		if ((header.exMinAlloc == 0) && (header.exMaxAlloc == 0))
		{
		  sp = MK_FP(start_seg + mp->m_size
					 - (image_size + 15) / 16, 0);
		}
	  }

	  do {
		nBytesRead = dos_read((COUNT) rc, (VOID FAR *) sp, (COUNT) (exe_size < CHUNK ? exe_size : CHUNK));
		sp = add_far((VOID FAR *) sp, (ULONG) nBytesRead);
		exe_size -= nBytesRead;
	} while(nBytesRead && exe_size > 0);
  }

  dos_lseek(rc, (LONG) header.exRelocTable, 0);
  for (i = 0; i < header.exRelocItems; i++)
  {
    if (dos_read(rc, (VOID FAR *) & reloc[0], sizeof(reloc)) != sizeof(reloc))
    {
      return DE_INVLDDATA;
    }
    if (mode == OVERLAY)
    {
      spot = MK_FP(reloc[1] + mem, reloc[0]);
      *spot += exp->load.reloc;
    }
    else
    {
      spot = MK_FP(reloc[1] + start_seg, reloc[0]);
      *spot += start_seg;
    }
  }

  dos_close(rc);

  if (mode == OVERLAY)
    return SUCCESS;

  p = MK_FP(mem, 0);
  setvec(0x22, (VOID(INRPT FAR *) (VOID)) MK_FP(user_r->CS, user_r->IP));
  new_psp(p, mem + asize);

  patchPSP(mem - 1, env, exp->exec.cmd_line, namep);
  irp = MK_FP(header.exInitSS + start_seg,
              ((header.exInitSP - sizeof(iregs)) & 0xffff));
  irp->ES = irp->DS = mem;
  irp->CS = header.exInitCS + start_seg;
  irp->IP = header.exInitIP;
  irp->AX = 0xffff;            
  irp->BX =
  irp->CX =
  irp->DX =
  irp->SI =
  irp->DI =
  irp->BP = 0;
  irp->FLAGS = 0x200;

  p->ps_parent = cu_psp;
  p->ps_prevpsp = (BYTE FAR *) MK_FP(cu_psp, 0);
  q->ps_stack = (BYTE FAR *) user_r;
  user_r->FLAGS &= ~FLG_CARRY;

  switch (mode)
  {
    case LOADNGO:
      cu_psp = mem;
      dta = p->ps_dta;
      if (InDOS)
        --InDOS;
      exec_user(irp);
      fatal("consola.c: Error de sistema!");
      break;

    case LOAD:
      cu_psp = mem;
      exp->exec.stack = (BYTE FAR *) irp;
      exp->exec.start_addr = MK_FP(irp->CS, irp->IP);
      return SUCCESS;
  }
  return DE_INVLDFMT;
}

COUNT DosExec(COUNT mode, exec_blk FAR * ep, BYTE FAR * lp)
{
  COUNT rc;
  exec_blk leb = *ep;
  BYTE FAR *cp;
  BOOL bIsCom = FALSE;

  if ((rc = dos_open(lp, 0)) < 0)
  {
    return DE_FILENOTFND;
  }

  if (dos_read(rc, (VOID FAR *) & header, sizeof(exe_header))
      != sizeof(exe_header))
  {
    bIsCom = TRUE;
  }
  dos_close(rc);

  if (bIsCom || header.exSignature != MAGIC)
  {
    rc = DosComLoader(lp, &leb, mode);
  }
  else
  {
    rc = DosExeLoader(lp, &leb, mode);
  }
  if (mode == LOAD && rc == SUCCESS)
    *ep = leb;

  return rc;
}

COUNT FAR init_call_DosExec(COUNT mode, exec_blk FAR * ep, BYTE FAR * lp)
{
  return DosExec(mode, ep, lp);
}

VOID FAR p_0(VOID)
{
  exec_blk exb;
  CommandTail Cmd;
  BYTE FAR *szfInitialPrgm = (BYTE FAR *) Config.cfgInit;
  int rc;

  exb.exec.env_seg = master_env;
  strcpy(Cmd.ctBuffer, Config.cfgInitTail);

  for (Cmd.ctCount = 0; Cmd.ctCount < 127; Cmd.ctCount++)
    if (Cmd.ctBuffer[Cmd.ctCount] == '\r')
      break;

  exb.exec.cmd_line = (CommandTail FAR *) & Cmd;
  exb.exec.fcb_1 = exb.exec.fcb_2 = (fcb FAR *) 0;
#ifdef DEBUG
  printf("consola.c: Iniciando sistema..\n\n", (BYTE *) szfInitialPrgm);
#endif
  if ((rc = DosExec(0, (exec_blk FAR *) & exb, szfInitialPrgm)) != SUCCESS)
    printf("\nconsola.c: Error en SH.\n", rc);
  else
    printf("\nconsola.c: Sistema parado.\nconsola.c: Reiniciando..\n");
  for (;;);
}
