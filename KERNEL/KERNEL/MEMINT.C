#include "port.h"
#include "todo.h"

#ifdef TSC
static VOID StartTrace(VOID);
static bTraceNext = FALSE;
#endif

extern UWORD Int21AX;

#pragma argsused
VOID FAR int21_entry(iregs UserRegs)
{
  int21_handler(UserRegs);
}

VOID int21_syscall(iregs FAR * irp)
{
  Int21AX = irp->AX;

  switch (irp->AH)
  {
    case 0x33:
      switch (irp->AL)
      {
        case 0x00:
          irp->DL = break_ena ? TRUE : FALSE;
          break;

        case 0x01:
          break_ena = irp->DL ? TRUE : FALSE;
          break;

        case 0x05:
          irp->DL = BootDrive;
          break;

        case 0x06:
          irp->BL = os_major;
          irp->BH = os_minor;
          irp->DL = rev_number;
          irp->DH = version_flags;
          break;

        default:
          irp->AX = -DE_INVLDFUNC;
          irp->FLAGS |= FLG_CARRY;
          break;

        case 0xfd:
#ifdef DEBUG
          bDumpRdWrParms = !bDumpRdWrParms;
#endif
          break;

        case 0xfe:
#ifdef DEBUG
          bDumpRegs = !bDumpRegs;
#endif
          break;

        case 0xff:
          irp->DX = FP_SEG(os_release);
          irp->AX = FP_OFF(os_release);
          break;
      }
      break;

    case 0x50:
      cu_psp = irp->BX;
      break;

    case 0x51:
      irp->BX = cu_psp;
      break;

    case 0x62:
      irp->BX = cu_psp;
      break;

    default:
      int21_service(user_r);
      break;
  }
}

VOID int21_service(iregs FAR * r)
{
  COUNT rc,
    rc1;
  ULONG lrc;
  psp FAR *p = MK_FP(cu_psp, 0);

  p->ps_stack = (BYTE FAR *) r;

#ifdef DEBUG
  if (bDumpRegs)
  {
    fbcopy((VOID FAR *) user_r, (VOID FAR *) & error_regs, sizeof(iregs));
    printf("memint.c: Llamada a la interrupcion 21h: %02x\n", user_r->AX);
    dump_regs = TRUE;
    dump();
  }
#endif

dispatch:

  switch (r->AH)
  {
    default:
      if (!break_ena) break;
    case 0x01:
    case 0x02:
    case 0x03:
    case 0x04:
    case 0x05:
    case 0x08:
    case 0x09:
    case 0x0a:
    case 0x0b:
      if(control_break())
		  handle_break();
  }

  switch (r->AH)
  {
    case 0x64:
    case 0x6b:
    default:
    error_invalid:
      r->AX = -DE_INVLDFUNC;
      goto error_out;
    error_exit:
      r->AX = -rc;
    error_out:
      r->FLAGS |= FLG_CARRY;
      break;

#if 0
    case 0x00:
      if (cu_psp == RootPsp)
        break;
      else if (((psp FAR *) (MK_FP(cu_psp, 0)))->ps_parent == cu_psp)
        break;
      tsr = FALSE;
      return_mode = break_flg ? 1 : 0;
      return_code = r->AL;
      if(DosMemCheck() != SUCCESS)
        panic("memint.c: Falta memoria para el ejecutar programa");
#ifdef TSC
      StartTrace();
#endif
      return_user();
      break;
#endif

    case 0x01:
      Do_DosIdle_loop();
      r->AL = _sti();
      sto(r->AL);
      break;

    case 0x02:
      sto(r->DL);
      break;

    case 0x03:
      r->AL = _sti();
      break;

    case 0x04:
      sto(r->DL);
      break;

    case 0x05:
      sto(r->DL);
      break;

    case 0x06:
      if (r->DL != 0xff)
        sto(r->DL);
      else if (StdinBusy())
      {
        r->AL = 0x00;
        r->FLAGS |= FLG_ZERO;
      }
      else {
        r->FLAGS &= ~FLG_ZERO;
        r->AL = _sti();
      }
      break;

    case 0x07:
    case 0x08:
      Do_DosIdle_loop();
      r->AL = _sti();
      break;

    case 0x09:
      {
        static COUNT scratch;
        BYTE FAR *p = MK_FP(r->DS, r->DX), FAR *q;
        q = p;
        while (*q != '$')
          ++q;
        DosWrite(STDOUT, q - p, p, (COUNT FAR *)&scratch);
      }
      r->AL = '$';
      break;

    case 0x0a:
      ((keyboard FAR *) MK_FP(r->DS, r->DX))->kb_count = 0;
      sti((keyboard FAR *) MK_FP(r->DS, r->DX));
      ((keyboard FAR *) MK_FP(r->DS, r->DX))->kb_count -= 2;
      break;

    case 0x0b:
      if (StdinBusy())
        r->AL = 0xFF;
      else
        r->AL = 0x00;
      break;

    case 0x0c:
      KbdFlush();
      switch (r->AL)
      {
        case 0x01:
        case 0x06:
        case 0x07:
        case 0x08:
        case 0x0a:
          r->AH = r->AL;
          goto dispatch;

        default:
          r->AL = 0x00;
          break;
      }
      break;

    case 0x0d:
      flush();
      break;

    case 0x0e:
      if (((COUNT) r->DL) >= 0 && r->DL < nblkdev)
        default_drive = r->DL;

      r->AL = nblkdev;
      break;

    case 0x0f:
      if (FcbOpen(MK_FP(r->DS, r->DX)))
        r->AL = 0;
      else
        r->AL = 0xff;
      break;

    case 0x10:
      if (FcbClose(MK_FP(r->DS, r->DX)))
        r->AL = 0;
      else
        r->AL = 0xff;
      break;

    case 0x11:
      if (FcbFindFirst(MK_FP(r->DS, r->DX)))
        r->AL = 0;
      else
        r->AL = 0xff;
      break;

    case 0x12:
      if (FcbFindNext(MK_FP(r->DS, r->DX)))
        r->AL = 0;
      else
        r->AL = 0xff;
      break;

    case 0x13:
      if (FcbDelete(MK_FP(r->DS, r->DX)))
        r->AL = 0;
      else
        r->AL = 0xff;
      break;

    case 0x14:
      {
        COUNT nErrorCode;

        if (FcbRead(MK_FP(r->DS, r->DX), &nErrorCode))
          r->AL = 0;
        else
          r->AL = nErrorCode;
        break;
      }

    case 0x15:
      {
        COUNT nErrorCode;

        if (FcbWrite(MK_FP(r->DS, r->DX), &nErrorCode))
          r->AL = 0;
        else
          r->AL = nErrorCode;
        break;
      }

    case 0x16:
      if (FcbCreate(MK_FP(r->DS, r->DX)))
        r->AL = 0;
      else
        r->AL = 0xff;
      break;

    case 0x17:
      if (FcbRename(MK_FP(r->DS, r->DX)))
        r->AL = 0;
      else
        r->AL = 0xff;
      break;

    case 0x18:
    case 0x1d:
    case 0x1e:
    case 0x20:
#ifndef TSC
    case 0x61:
#endif
      r->AL = 0;
      break;

    case 0x19:
      r->AL = default_drive;
      break;

    case 0x1a:
      {
        psp FAR *p = MK_FP(cu_psp, 0);

        p->ps_dta = MK_FP(r->DS, r->DX);
        dos_setdta(p->ps_dta);
      }
      break;

    case 0x1b:
      {
        BYTE FAR *p;

        FatGetDrvData(0,
                      (COUNT FAR *) & r->AX,
                      (COUNT FAR *) & r->CX,
                      (COUNT FAR *) & r->DX,
                      (BYTE FAR **) & p);
        r->DS = FP_SEG(p);
        r->BX = FP_OFF(p);
      }
      break;

    case 0x1c:
      {
        BYTE FAR *p;

        FatGetDrvData(r->DL,
                      (COUNT FAR *) & r->AX,
                      (COUNT FAR *) & r->CX,
                      (COUNT FAR *) & r->DX,
                      (BYTE FAR **) & p);
        r->DS = FP_SEG(p);
        r->BX = FP_OFF(p);
      }
      break;

    case 0x1f:
      if (default_drive < nblkdev)
      {
        struct dpb FAR *dpb = &blk_devices[default_drive];
        r->DS = FP_SEG(dpb);
        r->BX = FP_OFF(dpb);
        r->AL = 0;
      }
      else
        r->AL = 0xff;
      break;

    case 0x21:
      {
        COUNT nErrorCode;

        if (FcbRandomRead(MK_FP(r->DS, r->DX), &nErrorCode))
          r->AL = 0;
        else
          r->AL = nErrorCode;
        break;
      }

    case 0x22:
      {
        COUNT nErrorCode;

        if (FcbRandomWrite(MK_FP(r->DS, r->DX), &nErrorCode))
          r->AL = 0;
        else
          r->AL = nErrorCode;
        break;
      }

    case 0x23:
      if (FcbGetFileSize(MK_FP(r->DS, r->DX)))
        r->AL = 0;
      else
        r->AL = 0xff;
      break;

    case 0x24:
      FcbSetRandom(MK_FP(r->DS, r->DX));
      break;

    case 0x25:
      {
        VOID(INRPT FAR * p) () = MK_FP(r->DS, r->DX);

        setvec(r->AL, p);
      }
      break;

    case 0x26:
      {
        psp FAR *p = MK_FP(cu_psp, 0);

        new_psp((psp FAR *) MK_FP(r->DX, 0), p->ps_size);
      }
      break;

    case 0x27:
      {
        COUNT nErrorCode;

        if (FcbRandomBlockRead(MK_FP(r->DS, r->DX), r->CX, &nErrorCode))
          r->AL = 0;
        else
          r->AL = nErrorCode;
        break;
      }

    case 0x28:
      {
        COUNT nErrorCode;

        if (FcbRandomBlockWrite(MK_FP(r->DS, r->DX), r->CX, &nErrorCode))
          r->AL = 0;
        else
          r->AL = nErrorCode;
        break;
      }

    case 0x29:
      {
        BYTE FAR *lpFileName;

        lpFileName = MK_FP(r->DS, r->SI);
        r->AL = FcbParseFname(r->AL,
                              &lpFileName,
                              MK_FP(r->ES, r->DI));
        r->DS = FP_SEG(lpFileName);
        r->SI = FP_OFF(lpFileName);
      }
      break;

    case 0x2a:
      DosGetDate(
                  (BYTE FAR *) & (r->AL),      
                  (BYTE FAR *) & (r->DH),    
                  (BYTE FAR *) & (r->DL),     
                  (COUNT FAR *) & (r->CX));   
      break;

    case 0x2b:
      rc = DosSetDate(
                       (BYTE FAR *) & (r->DH), 
                       (BYTE FAR *) & (r->DL), 
                       (COUNT FAR *) & (r->CX));            
      if (rc != SUCCESS)
        r->AL = 0xff;
      else
        r->AL = 0;
      break;

    case 0x2c:
      DosGetTime(
                  (BYTE FAR *) & (r->CH),       
                  (BYTE FAR *) & (r->CL),      
                  (BYTE FAR *) & (r->DH),      
                  (BYTE FAR *) & (r->DL));    
      break;

    case 0x2d:
      rc = DosSetTime(
                       (BYTE FAR *) & (r->CH), 
                       (BYTE FAR *) & (r->CL), 
                       (BYTE FAR *) & (r->DH), 
                       (BYTE FAR *) & (r->DL));
      if (rc != SUCCESS)
        r->AL = 0xff;
      else
        r->AL = 0;
      break;

    case 0x2e:
      verify_ena = (r->AL ? TRUE : FALSE);
      break;

    case 0x2f:
      r->ES = FP_SEG(dta);
      r->BX = FP_OFF(dta);
      break;

    case 0x30:
      r->AL = os_major;
      r->AH = os_minor;
      r->BH = OEM_ID;
      r->CH = REVISION_MAJOR;   
      r->CL = REVISION_MINOR;
      r->BL = REVISION_SEQ;
      break;

    case 0x31:
      DosMemChange(cu_psp, r->DX < 6 ? 6 : r->DX, 0);
      return_mode = 3;
      return_code = r->AL;
      tsr = TRUE;
      return_user();
      break;

    case 0x32:
      if (r->DL < nblkdev)
      {
        struct dpb FAR *dpb = &blk_devices[r->DL];
        r->DS = FP_SEG(dpb);
        r->BX = FP_OFF(dpb);
        r->AL = 0;
      }
      else
        r->AL = 0xFF;
      break;

    case 0x34:
      {
        BYTE FAR *p;

        p = (BYTE FAR *) ((BYTE *) & InDOS);
        r->ES = FP_SEG(p);
        r->BX = FP_OFF(p);
      }
      break;

    case 0x35:
      {
        BYTE FAR *p;

        p = getvec((COUNT) r->AL);
        r->ES = FP_SEG(p);
        r->BX = FP_OFF(p);
      }
      break;

    case 0x36:
      DosGetFree(
                  (COUNT) r->DL,
                  (COUNT FAR *) & r->AX,
                  (COUNT FAR *) & r->BX,
                  (COUNT FAR *) & r->CX,
                  (COUNT FAR *) & r->DX);
      break;

    case 0x37:
      switch (r->AL)
      {
        case 0x00:
          r->DL = switchar;
          r->AL = 0x00;
          break;

        case 0x01:
          switchar = r->DL;
          r->AL = 0x00;
          break;

        default:
          goto error_invalid;
      }
      break;

    case 0x38:
      {
        BYTE FAR *lpTable
        = (BYTE FAR *) MK_FP(r->DS, r->DX);
        BYTE nRetCode;

        if (0xffff == r->DX)
        {
          r->BX = SetCtryInfo(
                               (UBYTE FAR *) & (r->AL),
                               (UWORD FAR *) & (r->BX),
                               (BYTE FAR *) & lpTable,
                               (UBYTE *) & nRetCode);

          if (nRetCode != 0)
          {
            r->AX = 0xff;
            r->FLAGS |= FLG_CARRY;
          }
          else
          {
            r->AX = nRetCode;
            r->FLAGS &= ~FLG_CARRY;
          }
        }
        else
        {
          r->BX = GetCtryInfo(&(r->AL), &(r->BX), lpTable);
          r->FLAGS &= ~FLG_CARRY;
        }
      }
      break;

    case 0x39:
      rc = dos_mkdir((BYTE FAR *) MK_FP(r->DS, r->DX));
      if (rc != SUCCESS)
        goto error_exit;
      else
      {
        r->FLAGS &= ~FLG_CARRY;
      }
      break;

    case 0x3a:
      rc = dos_rmdir((BYTE FAR *) MK_FP(r->DS, r->DX));
      if (rc != SUCCESS)
        goto error_exit;
      else
      {
        r->FLAGS &= ~FLG_CARRY;
      }
      break;

    case 0x3b:
      if ((rc = DosChangeDir((BYTE FAR *) MK_FP(r->DS, r->DX))) < 0)
        goto error_exit;
      else
      {
        r->FLAGS &= ~FLG_CARRY;
      }
      break;

    case 0x3c:
      if ((rc = DosCreat(MK_FP(r->DS, r->DX), r->CX)) < 0)
        goto error_exit;
      else
      {
        r->AX = rc;
        r->FLAGS &= ~FLG_CARRY;
      }
      break;

    case 0x3d:
      if ((rc = DosOpen(MK_FP(r->DS, r->DX), r->AL)) < 0)
        goto error_exit;
      else
      {
        r->AX = rc;
        r->FLAGS &= ~FLG_CARRY;
      }
      break;

    case 0x3e:
      if ((rc = DosClose(r->BX)) < 0)
        goto error_exit;
      else
        r->FLAGS &= ~FLG_CARRY;
      break;

    case 0x3f:
      rc = DosRead(r->BX, r->CX, MK_FP(r->DS, r->DX), (COUNT FAR *) & rc1);

      if (rc1 != SUCCESS)
      {
        r->FLAGS |= FLG_CARRY;
        r->AX = -rc1;
      }
      else
      {
        r->FLAGS &= ~FLG_CARRY;
        r->AX = rc;
      }
      break;

    case 0x40:
      rc = DosWrite(r->BX, r->CX, MK_FP(r->DS, r->DX), (COUNT FAR *) & rc1);
      if (rc1 != SUCCESS)
      {
        r->FLAGS |= FLG_CARRY;
        r->AX = -rc1;
      }
      else
      {
        r->FLAGS &= ~FLG_CARRY;
        r->AX = rc;
      }
      break;

    case 0x41:
      rc = dos_delete((BYTE FAR *) MK_FP(r->DS, r->DX));
      if (rc < 0)
      {
        r->FLAGS |= FLG_CARRY;
        r->AX = -rc1;
      }
      else
        r->FLAGS &= ~FLG_CARRY;
      break;

    case 0x42:
      if ((rc = DosSeek(r->BX, (LONG) ((((LONG) (r->CX)) << 16) + r->DX), r->AL, &lrc)) < 0)
        goto error_exit;
      else
      {
        r->DX = (lrc >> 16);
        r->AX = lrc & 0xffff;
        r->FLAGS &= ~FLG_CARRY;
      }
      break;

    case 0x43:
      switch (r->AL)
      {
        case 0x00:
          rc = DosGetFattr((BYTE FAR *) MK_FP(r->DS, r->DX), (UWORD FAR *) & r->CX);
          if (rc < SUCCESS)
            goto error_exit;
          else
          {
            r->FLAGS &= ~FLG_CARRY;
          }
          break;

        case 0x01:
          rc = DosSetFattr((BYTE FAR *) MK_FP(r->DS, r->DX), (UWORD FAR *) & r->CX);
          if (rc != SUCCESS)
            goto error_exit;
          else
            r->FLAGS &= ~FLG_CARRY;
          break;

        default:
          goto error_invalid;
      }
      break;

    case 0x44:
      {
        rc = DosDevIOctl(r, (COUNT FAR *) & rc1);

        if (rc1 != SUCCESS)
        {
          r->FLAGS |= FLG_CARRY;
          r->AX = -rc1;
        }
        else
        {
          r->FLAGS &= ~FLG_CARRY;
        }
      }
      break;

    case 0x45:
      rc = DosDup(r->BX);
      if (rc < SUCCESS)
        goto error_exit;
      else
      {
        r->FLAGS &= ~FLG_CARRY;
        r->AX = rc;
      }
      break;

    case 0x46:
      rc = DosForceDup(r->BX, r->CX);
      if (rc < SUCCESS)
        goto error_exit;
      else
        r->FLAGS &= ~FLG_CARRY;
      break;

    case 0x47:
      if ((rc = DosGetCuDir(r->DL, MK_FP(r->DS, r->SI))) < 0)
        goto error_exit;
      else
      {
        r->FLAGS &= ~FLG_CARRY;
        r->AX = 0x0100;  /* lista de interrupciones */
      }
      break;

    case 0x48:
      if ((rc = DosMemAlloc(r->BX, mem_access_mode, &(r->AX), &(r->BX))) < 0)
      {
        DosMemLargest(&(r->BX));
        goto error_exit;
      }
      else
      {
        ++(r->AX);             
        r->FLAGS &= ~FLG_CARRY;
      }
      break;

    case 0x49:
      if ((rc = DosMemFree((r->ES) - 1)) < 0)
        goto error_exit;
      else
        r->FLAGS &= ~FLG_CARRY;
      break;

    case 0x4a:
      {
        UWORD maxSize;

        if ((rc = DosMemChange(r->ES, r->BX, &maxSize)) < 0)
        {
          if (rc == DE_NOMEM)
            r->BX = maxSize;

#if 0
          if (cu_psp == r->ES)
          {

            psp FAR *p;

            p = MK_FP(cu_psp, 0);
            p->ps_size = r->BX + cu_psp;
          }
#endif
          goto error_exit;
        }
        else
          r->FLAGS &= ~FLG_CARRY;

        break;
      }

    case 0x4b:
      break_flg = FALSE;

      if ((rc = DosExec(r->AL, MK_FP(r->ES, r->BX), MK_FP(r->DS, r->DX)))
          != SUCCESS)
        goto error_exit;
      else
        r->FLAGS &= ~FLG_CARRY;
      break;

    case 0x00:
    	r->AX = 0x4c00;

    case 0x4c:
      if (cu_psp == RootPsp
       || ((psp FAR *) (MK_FP(cu_psp, 0)))->ps_parent == cu_psp)
        break;
      tsr = FALSE;
      if (ErrorMode)
      {
        ErrorMode = FALSE;
        return_mode = 2;
      }
      else if (break_flg)
      {
        break_flg = FALSE;
        return_mode = 1;
      }
      else
      {
        return_mode = 0;
      }
      return_code = r->AL;
      if(DosMemCheck() != SUCCESS)
        panic("memint.c: Falta memoria para correr el programa");
#ifdef TSC
      StartTrace();
#endif
      return_user();
      break;

    case 0x4d:
      r->AL = return_code;
      r->AH = return_mode;
      break;

    case 0x4e:
      {
        if ((rc = DosFindFirst((UCOUNT) r->CX, (BYTE FAR *) MK_FP(r->DS, r->DX))) < 0)
          goto error_exit;
        else
        {
          r->AX = 0;
          r->FLAGS &= ~FLG_CARRY;
        }
      }
      break;

    case 0x4f:
      {
        if ((rc = DosFindNext()) < 0)
        {
          r->AX = -rc;

          if (r->AX == 2)
            r->AX = 18;

          r->FLAGS |= FLG_CARRY;
        }
        else
        {
          r->FLAGS &= ~FLG_CARRY;
        }
      }
      break;

    case 0x52:
      {
        BYTE FAR *p;

        p = (BYTE FAR *) & DPBp;
        r->ES = FP_SEG(p);
        r->BX = FP_OFF(p);
      }
      break;

    case 0x54:
      r->AL = (verify_ena ? TRUE : FALSE);
      break;

    case 0x55:
      new_psp((psp FAR *) MK_FP(r->DX, 0), r->SI);
      break;

    case 0x56:
      rc = dos_rename(
                       (BYTE FAR *) MK_FP(r->DS, r->DX),       
                       (BYTE FAR *) MK_FP(r->ES, r->DI));      
      if (rc < SUCCESS)
        goto error_exit;
      else
      {
        r->FLAGS &= ~FLG_CARRY;
      }
      break;

    case 0x57:
      switch (r->AL)
      {
        case 0x00:
          rc = DosGetFtime(
                            (COUNT) r->BX,     
                            (date FAR *) & r->DX,      
                            (time FAR *) & r->CX);   
          if (rc < SUCCESS)
            goto error_exit;
          else
            r->FLAGS &= ~FLG_CARRY;
          break;

        case 0x01:
          rc = DosSetFtime(
                            (COUNT) r->BX,     
                            (date FAR *) & r->DX,      
                            (time FAR *) & r->CX);     
          if (rc < SUCCESS)
            goto error_exit;
          else
            r->FLAGS &= ~FLG_CARRY;
          break;

        default:
          goto error_invalid;
      }
      break;

    case 0x58:
      switch (r->AL)
      {
        case 0x00:
          r->AX = mem_access_mode;
          break;

        case 0x01:
          if (((COUNT) r->BX) < 0 || r->BX > 2)
            goto error_invalid;
          else
          {
            mem_access_mode = r->BX;
            r->FLAGS &= ~FLG_CARRY;
          }
          break;

        default:
          goto error_invalid;
#ifdef DEBUG
        case 0xff:
          show_chain();
          break;
#endif
      }
      break;

    case 0x5a:
      if ((rc = DosMkTmp(MK_FP(r->DS, r->DX), r->CX)) < 0)
        goto error_exit;
      else
      {
        r->AX = rc;
        r->FLAGS &= ~FLG_CARRY;
      }
      break;

    case 0x5b:
      if ((rc = DosOpen(MK_FP(r->DS, r->DX), 0)) >= 0)
      {
        DosClose(rc);
        r->AX = 80;
        r->FLAGS |= FLG_CARRY;
      }
      else
      {
        if ((rc = DosCreat(MK_FP(r->DS, r->DX), r->CX)) < 0)
          goto error_exit;
        else
        {
          r->AX = rc;
          r->FLAGS &= ~FLG_CARRY;
        }
      }
      break;

    case 0x5d:
      switch (r->AL)
      {
        case 0x06:
          r->DS = FP_SEG(internal_data);
          r->SI = FP_OFF(internal_data);
          r->CX = swap_always - internal_data;
          r->DX = swap_indos - internal_data;
          r->FLAGS &= ~FLG_CARRY;
          break;

        default:
          goto error_invalid;
      }
      break;

    case 0xd5e:
      switch (r->AL)
      {
        case 0x00:
            r->CX = get_machine_name(MK_FP(r->DS, r->DX));
          break;

        case 0x01:
          set_machine_name(MK_FP(r->DS, r->DX), r->CX);
          break;

        default:
          goto error_invalid;
      }
      break;



    case 0x60:                
      if ((rc = truename(MK_FP(r->DS, r->SI),
                         adjust_far(MK_FP(r->ES, r->DI)))) != SUCCESS)
        goto error_exit;
      else
      {
        r->FLAGS &= ~FLG_CARRY;
      }
      break;

#ifdef TSC
    case 0x61:
#ifdef DEBUG
      switch (r->AL)
      {
        case 0x01:
          bTraceNext = TRUE;
          break;

        case 0x02:
          bDumpRegs = FALSE;
          break;
      }
#endif
#endif
      r->AL = 0x00;
      break;

    case 0x62:
      r->BX = cu_psp;
      break;

    case 0x63:
      {
#ifdef DBLBYTE
        static char dbcsTable[2] =
        {
          0, 0
        };
        void FAR *dp = &dbcsTable;

        r->DS = FP_SEG(dp);
        r->SI = FP_OFF(dp);
        r->AL = 0;
#else
        r->AL = 0x00;  
#endif
        break;
      }

    case 0x65:
      if (r->AL <= 0x7)
      {
        if (ExtCtryInfo(
                         r->AL,
                         r->BX,
                         r->CX,
                         MK_FP(r->ES, r->DI)))
          r->FLAGS &= ~FLG_CARRY;
        else
          goto error_invalid;
      }
      else if ((r->AL >= 0x20) && (r->AL <= 0x22))
      {
        switch (r->AL)
        {
          case 0x20:
            r->DL = upChar(r->DL);
            goto okay;

          case 0x21:
            upMem(
                   MK_FP(r->DS, r->DX),
                   r->CX);
            goto okay;

          case 0x22:
            upString(MK_FP(r->DS, r->DX));
          okay:
            r->FLAGS &= ~FLG_CARRY;
            break;

          case 0x23:
            r->AX = yesNo(r->DL);
            goto okay;

          default:
            goto error_invalid;
        }
      }
      else
        r->FLAGS |= FLG_CARRY;
      break;

    case 0x66:
      switch (r->AL)
      {
        case 1:
          GetGlblCodePage(
                           (UWORD FAR *) & (r->BX),
                           (UWORD FAR *) & (r->DX));
          goto okay_66;

        case 2:
          SetGlblCodePage(
                           (UWORD FAR *) & (r->BX),
                           (UWORD FAR *) & (r->DX));
        okay_66:
          r->FLAGS &= ~FLG_CARRY;
          break;

        default:
          goto error_invalid;
      }
      break;

    case 0x67:
      if ((rc = SetJFTSize(r->BX)) != SUCCESS)
        goto error_exit;
      else
      {
        r->FLAGS &= ~FLG_CARRY;
      }
      break;

    case 0x68:
      r->FLAGS &= ~FLG_CARRY;
      break;
  }

#ifdef DEBUG
  if (bDumpRegs)
  {
    fbcopy((VOID FAR *) user_r, (VOID FAR *) & error_regs,
           sizeof(iregs));
    dump_regs = TRUE;
    dump();
  }
#endif
}

VOID INRPT FAR int22_handler(void)
{
}

#if 0
        
#pragma argsused
VOID INRPT FAR int23_handler(int es, int ds, int di, int si, int bp, int sp, int bx, int dx, int cx, int ax, int ip, int cs, int flags)
{
  tsr = FALSE;
  return_mode = 1;
  return_code = -1;
  mod_sto(CTL_C);
  DosMemCheck();
#ifdef TSC
  StartTrace();
#endif
  return_user();
}
#endif

struct HugeSectorBlock
{
  ULONG blkno;
  WORD nblks;
  BYTE FAR *buf;
};

struct int25regs
{
  UWORD es,
    ds;
  UWORD di,
    si,
    bp,
    sp;
  UWORD bx,
    dx,
    cx,
    ax;
  UWORD flags,
    ip,
    cs;
};

VOID int25_handler(struct int25regs FAR * r)
{
  ULONG blkno;
  UWORD nblks;
  BYTE FAR *buf;
  UBYTE drv = r->ax & 0xFF;

  InDOS++;

  if (r->cx == 0xFFFF)
  {
    struct HugeSectorBlock FAR *lb = MK_FP(r->ds, r->bx);
    blkno = lb->blkno;
    nblks = lb->nblks;
    buf = lb->buf;
  }
  else
  {
    nblks = r->cx;
    blkno = r->dx;
    buf = MK_FP(r->ds, r->bx);
  }

  if (drv >= nblkdev)
  {
    r->ax = 0x202;
    r->flags |= FLG_CARRY;
    return;
  }

  if (!dskxfer(drv, blkno, buf, nblks, DSKREAD))
  {
    r->ax = 0x202;
    r->flags |= FLG_CARRY;
    return;
  }

  r->ax = 0;
  r->flags &= ~FLG_CARRY;
  --InDOS;
}

VOID int26_handler(struct int25regs FAR * r)
{
  ULONG blkno;
  UWORD nblks;
  BYTE FAR *buf;
  UBYTE drv = r->ax & 0xFF;

  InDOS++;

  if (r->cx == 0xFFFF)
  {
    struct HugeSectorBlock FAR *lb = MK_FP(r->ds, r->bx);
    blkno = lb->blkno;
    nblks = lb->nblks;
    buf = lb->buf;
  }
  else
  {
    nblks = r->cx;
    blkno = r->dx;
    buf = MK_FP(r->ds, r->bx);
  }

  if (drv >= nblkdev)
  {
    r->ax = 0x202;
    r->flags |= FLG_CARRY;
    return;
  }

  if (!dskxfer(drv, blkno, buf, nblks, DSKWRITE))
  {
    r->ax = 0x202;
    r->flags |= FLG_CARRY;
    return;
  }

  setinvld(drv);

  r->ax = 0;
  r->flags &= ~FLG_CARRY;
  --InDOS;
}

VOID INRPT FAR int28_handler(void)
{
}

VOID INRPT FAR int2a_handler(void)
{
}

VOID INRPT FAR empty_handler(void)
{
}

#ifdef TSC
static VOID StartTrace(VOID)
{
  if (bTraceNext)
  {
#ifdef DEBUG
    bDumpRegs = TRUE;
#endif
    bTraceNext = FALSE;
  }
#ifdef DEBUG
  else
    bDumpRegs = FALSE;
#endif
}
#endif
