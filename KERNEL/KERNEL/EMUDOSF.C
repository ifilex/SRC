#include "port.h"
#include "todo.h"

sft FAR *get_sft(COUNT);
WORD get_free_hndl(VOID);
sft FAR *get_free_sft(WORD FAR *);
BYTE FAR *get_root(BYTE FAR *);
BOOL cmatch(COUNT, COUNT, COUNT);
BOOL fnmatch(BYTE FAR *, BYTE FAR *, COUNT, COUNT);

struct f_node FAR *xlt_fd(COUNT);

static VOID DosGetFile(BYTE FAR * lpszPath, BYTE FAR * lpszDosFileName)
{
  BYTE szLclName[FNAME_SIZE + 1];
  BYTE szLclExt[FEXT_SIZE + 1];

  ParseDosName(lpszPath, (COUNT *) 0, (BYTE *) 0,
               szLclName, szLclExt, FALSE);
  SpacePad(szLclName, FNAME_SIZE);
  SpacePad(szLclExt, FEXT_SIZE);
  fbcopy((BYTE FAR *) szLclName, lpszDosFileName, FNAME_SIZE);
  fbcopy((BYTE FAR *) szLclExt, &lpszDosFileName[FNAME_SIZE], FEXT_SIZE);
}

sft FAR *get_sft(COUNT hndl)
{
  psp FAR *p = MK_FP(cu_psp, 0);
  WORD sys_idx;
  sfttbl FAR *sp;

  if (hndl >= p->ps_maxfiles)
    return (sft FAR *) - 1;

  if (p->ps_filetab[hndl] == 0xff)
    return (sft FAR *) - 1;

  sys_idx = p->ps_filetab[hndl];
  for (sp = sfthead; sp != (sfttbl FAR *) - 1; sp = sp->sftt_next)
  {
    if (sys_idx < sp->sftt_count)
      break;
    else
      sys_idx -= sp->sftt_count;
  }

  if (sp == (sfttbl FAR *) - 1)
    return (sft FAR *) - 1;

  return (sft FAR *) & (sp->sftt_table[sys_idx]);
}

UCOUNT GenericRead(COUNT hndl, UCOUNT n, BYTE FAR * bp, COUNT FAR * err,
BOOL force_binary)
{
  sft FAR *s;
  WORD sys_idx;
  sfttbl FAR *sp;
  UCOUNT ReadCount;

  if (hndl < 0)
  {
    *err = DE_INVLDHNDL;
    return 0;
  }

  if ((s = get_sft(hndl)) == (sft FAR *) - 1)
  {
    *err = DE_INVLDHNDL;
    return 0;
  }

  if (s->sft_count == 0 || (s->sft_mode & SFT_MWRITE))
  {
    *err = DE_INVLDACC;
    return 0;
  }

  if (s->sft_flags & SFT_FDEVICE)
  {
    request rq;

    if (!(s->sft_flags & SFT_FEOF) || (s->sft_flags & SFT_FNUL))
    {
      s->sft_flags &= ~SFT_FEOF;
      *err = SUCCESS;
      return 0;
    }

    if (force_binary || (s->sft_flags & SFT_FBINARY))
    {
      rq.r_length = sizeof(request);
      rq.r_command = C_INPUT;
      rq.r_count = n;
      rq.r_trans = (BYTE FAR *) bp;
      rq.r_status = 0;
      execrh((request FAR *) & rq, s->sft_dev);
      if (rq.r_status & S_ERROR)
      {
        char_error(&rq, s->sft_dev);
      }
      else
      {
        *err = SUCCESS;
        return rq.r_count;
      }
    }
    else if (s->sft_flags & SFT_FCONIN)
    {
      kb_buf.kb_size = LINESIZE - 1;
      kb_buf.kb_count = 0;
      sti((keyboard FAR *) & kb_buf);
      fbcopy((BYTE FAR *) kb_buf.kb_buf, bp, kb_buf.kb_count);
      *err = SUCCESS;
      return kb_buf.kb_count;
    }
    else
    {
      *bp = _sti();
      *err = SUCCESS;
      return 1;
    }
  }
  else
  {
    COUNT rc;

    ReadCount = readblock(s->sft_status, bp, n, &rc);
    if (rc != SUCCESS)
    {
      *err = rc;
      return 0;
    }
    else
    {
      *err = SUCCESS;
      return ReadCount;
    }
  }
  *err = SUCCESS;
  return 0;
}

UCOUNT DosRead(COUNT hndl, UCOUNT n, BYTE FAR * bp, COUNT FAR * err)
{
  return GenericRead(hndl, n, bp, err, FALSE);
}

UCOUNT DosWrite(COUNT hndl, UCOUNT n, BYTE FAR * bp, COUNT FAR * err)
{
  sft FAR *s;
  WORD sys_idx;
  sfttbl FAR *sp;
  UCOUNT ReadCount;

  if (hndl < 0)
  {
    *err = DE_INVLDHNDL;
    return 0;
  }

  if ((s = get_sft(hndl)) == (sft FAR *) - 1)
  {
    *err = DE_INVLDHNDL;
    return 0;
  }

  if (s->sft_count == 0 ||
      (!(s->sft_mode & SFT_MWRITE) && !(s->sft_mode & SFT_MRDWR)))
  {
    *err = DE_ACCESS;
    return 0;
  }

  if (s->sft_flags & SFT_FDEVICE)
  {
    request rq;

    s->sft_flags &= ~SFT_FEOF;

    if (s->sft_flags & SFT_FNUL)
    {
      *err = SUCCESS;
      return n;
    }

    if (s->sft_flags & SFT_FBINARY)
    {
      rq.r_length = sizeof(request);
      rq.r_command = C_OUTPUT;
      rq.r_count = n;
      rq.r_trans = (BYTE FAR *) bp;
      rq.r_status = 0;
      execrh((request FAR *) & rq, s->sft_dev);
      if (rq.r_status & S_ERROR)
      {
        char_error(&rq, s->sft_dev);
      }
      else
      {
	if (s->sft_flags & SFT_FCONOUT)
	{
	  WORD cnt = rq.r_count;
	  while (cnt--)
	  {
	    switch (*bp++)
	    {
	      case CR:
		scr_pos = 0;
		break;
	      case LF:
	      case BELL:
		break;
	      case BS:
		--scr_pos;
		break;
	      default:
		++scr_pos;
	    }
	  }
	}
        *err = SUCCESS;
        return rq.r_count;
      }
    }
    else
    {
      REG WORD c,
        cnt = n,
	spaces_left = 0,
	next_pos,
        xfer = 0;
      static BYTE space = ' ';

    start:
      if (cnt-- == 0) goto end;
      if (*bp == CTL_Z) goto end;
      if (s->sft_flags & SFT_FCONOUT)
      {
	switch (*bp)
	{
	  case CR:
	    next_pos = 0;
	    break;
	  case LF:
	  case BELL:
	    next_pos = scr_pos;
	    break;
	  case BS:
	    next_pos = scr_pos ? scr_pos - 1 : 0;
	    break;
	  case HT:
	    spaces_left = 8 - (scr_pos & 7);
	    next_pos = scr_pos + spaces_left;
	    goto output_space;
	  default:
	    next_pos = scr_pos + 1;
	}
      }
      rq.r_length = sizeof(request);
      rq.r_command = C_OUTPUT;
      rq.r_count = 1;
      rq.r_trans = bp;
      rq.r_status = 0;
      execrh((request FAR *) & rq, s->sft_dev);
      if (rq.r_status & S_ERROR)
	char_error(&rq, s->sft_dev);
      goto post;
    output_space:
      rq.r_length = sizeof(request);
      rq.r_command = C_OUTPUT;
      rq.r_count = 1;
      rq.r_trans = &space;
      rq.r_status = 0;
      execrh((request FAR *) & rq, s->sft_dev);
      if (rq.r_status & S_ERROR)
	char_error(&rq, s->sft_dev);
      --spaces_left;
    post:
      if (spaces_left) goto output_space;
      ++bp;
      ++xfer;
      if (s->sft_flags & SFT_FCONOUT)
	scr_pos = next_pos;
      if (break_ena && control_break())
      {
	handle_break();
	goto end;
      }
      goto start;
    end:
      *err = SUCCESS;
      return xfer;
    }
  }
  else
  {
    COUNT rc;

    ReadCount = writeblock(s->sft_status, bp, n, &rc);
    if (rc < SUCCESS)
    {
      *err = rc;
      return 0;
    }
    else
    {
      *err = SUCCESS;
      return ReadCount;
    }
  }
  *err = SUCCESS;
  return 0;
}

COUNT DosSeek(COUNT hndl, LONG new_pos, COUNT mode, ULONG * set_pos)
{
  sft FAR *s;

  if (mode < 0 || mode > 2)
    return DE_INVLDFUNC;

  if (hndl < 0)
    return DE_INVLDHNDL;

  if ((s = get_sft(hndl)) == (sft FAR *) - 1)
    return DE_INVLDHNDL;

  if (s->sft_flags & SFT_FDEVICE)
  {
    *set_pos = 0l;
    return SUCCESS;
  }
  else
  {
    *set_pos = dos_lseek(s->sft_status, new_pos, mode);
    if ((LONG) * set_pos < 0)
      return (int)*set_pos;
    else
      return SUCCESS;
  }
}

static WORD get_free_hndl(void)
{
  psp FAR *p = MK_FP(cu_psp, 0);
  WORD hndl;

  for (hndl = 0; hndl < p->ps_maxfiles; hndl++)
  {
    if (p->ps_filetab[hndl] == 0xff)
      return hndl;
  }
  return 0xff;
}

static sft FAR *get_free_sft(WORD FAR * sft_idx)
{
  WORD sys_idx = 0;
  sfttbl FAR *sp;

  for (sp = sfthead; sp != (sfttbl FAR *) - 1; sp = sp->sftt_next)
  {
    REG WORD i;

    for (i = 0; i < sp->sftt_count; i++)
    {
      if (sp->sftt_table[i].sft_count == 0)
      {
        *sft_idx = sys_idx + i;
        return (sft FAR *) & sp->sftt_table[sys_idx + i];
      }
    }
    sys_idx += i;
  }
  return (sft FAR *) - 1;
}

static BYTE FAR *get_root(BYTE FAR * fname)
{
  BYTE FAR *froot;
  REG WORD length;

  for (length = 0, froot = fname; *froot != '\0'; ++froot)
    ++length;
/* se idenfitica el root de la raiz.. */
  for (--froot; length > 0 && !(*froot == '/' || *froot == '\\'); --froot)
    --length;
  return ++froot;
}

static BOOL cmatch(COUNT s, COUNT d, COUNT mode)
{
  if (s >= 'a' && s <= 'z')
    s -= 'a' - 'A';
  if (d >= 'a' && d <= 'z')
    d -= 'a' - 'A';
  if (mode && s == '?' && (d >= 'A' && s <= 'Z'))
    return TRUE;
  return s == d;
}

static BOOL fnmatch(BYTE FAR * s, BYTE FAR * d, COUNT n, COUNT mode)
{
  while (n--)
  {
    if (!cmatch(*s++, *d++, mode))
      return FALSE;
  }
  return TRUE;
}

COUNT DosCreat(BYTE FAR * fname, COUNT attrib)
{
  psp FAR *p = MK_FP(cu_psp, 0);
  WORD hndl,
    sft_idx;
  sft FAR *sftp;
  struct dhdr FAR *dhp;
  BYTE FAR *froot;
  WORD i;

  if ((hndl = get_free_hndl()) == 0xff)
    return DE_TOOMANY;

  if ((sftp = get_free_sft((WORD FAR *) & sft_idx)) == (sft FAR *) - 1)
    return DE_TOOMANY;

  froot = get_root(fname);
  for (i = 0; i < FNAME_SIZE; i++)
  {
    if (*froot != '\0' && *froot != '.')
      PriPathName[i] = *froot++;
    else
      break;
  }

  for (; i < FNAME_SIZE; i++)
    PriPathName[i] = ' ';

  if (*froot != '.')
  {
    for (dhp = (struct dhdr FAR *)&nul_dev; dhp != (struct dhdr FAR *)-1; dhp = dhp->dh_next)
    {
      if (fnmatch((BYTE FAR *) PriPathName, (BYTE FAR *) dhp->dh_name, FNAME_SIZE, FALSE))
      {
        sftp->sft_count += 1;
        sftp->sft_mode = SFT_MRDWR;
        sftp->sft_attrib = attrib;
        sftp->sft_flags =
            (dhp->dh_attr & ~SFT_MASK) | SFT_FDEVICE | SFT_FEOF;
        sftp->sft_psp = cu_psp;
        fbcopy((BYTE FAR *) PriPathName, sftp->sft_name, FNAME_SIZE + FEXT_SIZE);
        sftp->sft_dev = dhp;
        p->ps_filetab[hndl] = sft_idx;
        return hndl;
      }
    }
  }
  sftp->sft_status = dos_creat(fname, attrib);
  if (sftp->sft_status >= 0)
  {
    p->ps_filetab[hndl] = sft_idx;
    sftp->sft_count += 1;
    sftp->sft_mode = SFT_MRDWR;
    sftp->sft_attrib = attrib;
    sftp->sft_flags = 0;
    sftp->sft_psp = cu_psp;
    DosGetFile(fname, sftp->sft_name);
    return hndl;
  }
  else
    return sftp->sft_status;
}

COUNT CloneHandle(COUNT hndl)
{
  sft FAR *sftp;

  if ((sftp = get_sft(hndl)) == (sft FAR *) - 1)
    return DE_INVLDHNDL;

  sftp->sft_count += 1;
  return SUCCESS;
}

COUNT DosDup(COUNT Handle)
{
  psp FAR *p = MK_FP(cu_psp, 0);
  COUNT NewHandle;
  sft FAR *Sftp;

  if ((Sftp = get_sft(Handle)) == (sft FAR *) - 1)
    return DE_INVLDHNDL;

  if (Sftp->sft_count <= 0)
    return DE_INVLDHNDL;

  if ((NewHandle = get_free_hndl()) == 0xff)
    return DE_TOOMANY;

  if ((Sftp->sft_flags & SFT_FDEVICE) || (Sftp->sft_status >= 0))
  {
    p->ps_filetab[NewHandle] = p->ps_filetab[Handle];
    Sftp->sft_count += 1;
    return NewHandle;
  }
  else
    return DE_INVLDHNDL;
}

COUNT DosForceDup(COUNT OldHandle, COUNT NewHandle)
{
  psp FAR *p = MK_FP(cu_psp, 0);
  sft FAR *Sftp;

  if ((Sftp = get_sft(OldHandle)) == (sft FAR *) - 1)
    return DE_INVLDHNDL;

  if (Sftp->sft_count <= 0)
    return DE_INVLDHNDL;

  if ((UBYTE) p->ps_filetab[NewHandle] != 0xff)
  {
    COUNT ret;

    if ((ret = DosClose(NewHandle)) != SUCCESS)
      return ret;
  }

  if ((Sftp->sft_flags & SFT_FDEVICE) || (Sftp->sft_status >= 0))
  {
    p->ps_filetab[NewHandle] = p->ps_filetab[OldHandle];

    Sftp->sft_count += 1;
    return NewHandle;
  }
  else
    return DE_INVLDHNDL;
}

COUNT DosOpen(BYTE FAR * fname, COUNT mode)
{
  psp FAR *p = MK_FP(cu_psp, 0);
  WORD hndl;
  WORD sft_idx;
  sft FAR *sftp;
  struct dhdr FAR *dhp;
  BYTE FAR *froot;
  WORD i;

  if ((mode & ~SFT_OMASK) != 0)
    return DE_INVLDACC;

  mode &= 3;

  if (cu_psp != DOS_PSP)
  {
    if ((hndl = get_free_hndl()) == 0xff)
      return DE_TOOMANY;
  }

  if ((sftp = get_free_sft((WORD FAR *) & sft_idx)) == (sft FAR *) - 1)
    return DE_TOOMANY;

  froot = get_root(fname);
  for (i = 0; i < FNAME_SIZE; i++)
  {
    if (*froot != '\0' && *froot != '.')
      PriPathName[i] = *froot++;
    else
      break;
  }

  for (; i < FNAME_SIZE; i++)
    PriPathName[i] = ' ';

  if (*froot != '.')
  {
    for (dhp = (struct dhdr FAR *)&nul_dev; dhp != (struct dhdr FAR *)-1; dhp = dhp->dh_next)
    {
      if (fnmatch((BYTE FAR *) PriPathName, (BYTE FAR *) dhp->dh_name, FNAME_SIZE, FALSE))
      {
        sftp->sft_count += 1;
        sftp->sft_mode = mode;
        sftp->sft_attrib = 0;
        sftp->sft_flags =
            (dhp->dh_attr & ~SFT_MASK) | SFT_FDEVICE | SFT_FEOF;
        sftp->sft_psp = cu_psp;
        fbcopy((BYTE FAR *) PriPathName, sftp->sft_name, FNAME_SIZE + FEXT_SIZE);
        sftp->sft_dev = dhp;
				sftp->sft_date = dos_getdate();
				sftp->sft_time = dos_gettime();

        if (cu_psp != DOS_PSP)
          p->ps_filetab[hndl] = sft_idx;
        return hndl;
      }
    }
  }
  sftp->sft_status = dos_open(fname, mode);

  if (sftp->sft_status >= 0)
  {
		struct f_node FAR *fnp = xlt_fd(sftp->sft_status);

		sftp->sft_attrib = fnp->f_dir.dir_attrib;

		if ((sftp->sft_attrib & (D_DIR | D_VOLID)) ||
		    ((sftp->sft_attrib & D_RDONLY) && (mode != O_RDONLY)))
	  {
			return DE_ACCESS;
	  }

		if (cu_psp != DOS_PSP)
      p->ps_filetab[hndl] = sft_idx;

    sftp->sft_count += 1;
    sftp->sft_mode = mode;
    sftp->sft_attrib = 0;
    sftp->sft_flags = 0;
    sftp->sft_psp = cu_psp;
    DosGetFile(fname, sftp->sft_name);
    return hndl;
  }
  else
    return sftp->sft_status;
}

COUNT DosClose(COUNT hndl)
{
  psp FAR *p = MK_FP(cu_psp, 0);
  sft FAR *s;

  if (hndl < 0)
    return DE_INVLDHNDL;

  if ((s = get_sft(hndl)) == (sft FAR *) - 1)
    return DE_INVLDHNDL;

  if (s->sft_count == 0)
    return DE_ACCESS;

  if (s->sft_flags & SFT_FDEVICE)
  {
    p->ps_filetab[hndl] = 0xff;
    s->sft_count -= 1;
    return SUCCESS;
  }
  else
  {
    p->ps_filetab[hndl] = 0xff;
    s->sft_count -= 1;
    if (s->sft_count > 0)
      return SUCCESS;
    else
      return dos_close(s->sft_status);
  }
}

VOID DosGetFree(COUNT drive, COUNT FAR * spc, COUNT FAR * navc, COUNT FAR * bps, COUNT FAR * nc)
{
  struct dpb *dpbp;

  if (drive < 0 || drive > nblkdev)
  {
    *spc = -1;
    return;
  }

  drive = (drive == 0 ? default_drive : drive - 1);
  dpbp = &blk_devices[drive];
  ++(dpbp->dpb_count);
  if ((media_check(dpbp) < 0) || (dpbp->dpb_count <= 0))
  {
    *spc = -1;
    return;
  }

  *nc = dpbp->dpb_size;
  *spc = dpbp->dpb_clssize;
  *bps = dpbp->dpb_secsize;

  *navc = dos_free(dpbp);
  --(dpbp->dpb_count);
}

COUNT DosGetCuDir(COUNT drive, BYTE FAR * s)
{
  REG struct dpb *dpbp;

  if (drive < 0 || drive > nblkdev)
    return DE_INVLDDRV;

  drive = (drive == 0 ? default_drive : drive - 1);
  dpbp = &blk_devices[drive];
  ++(dpbp->dpb_count);
  if ((media_check(dpbp) < 0) || (dpbp->dpb_count <= 0))
    return DE_INVLDDRV;

  dos_pwd(dpbp, s);
  --(dpbp->dpb_count);
  return SUCCESS;
}

COUNT DosChangeDir(BYTE FAR * s)
{
  REG struct dpb *dpbp;
  REG COUNT drive;
  struct f_node FAR *fp;
  COUNT ret;

  if ((fp = dir_open((BYTE FAR *) s)) == (struct f_node FAR *)0)
    return DE_PATHNOTFND;
  else
    dir_close(fp);

  if (*(s + 1) == ':')
  {
    drive = *s - '@';
    if (drive > 26)
      drive -= 'a' - 'A';
  }
  else
    drive = 0;

  if (drive < 0 || drive > nblkdev)
    return DE_INVLDDRV;

  drive = (drive == 0 ? default_drive : drive - 1);
  dpbp = &blk_devices[drive];
  ++(dpbp->dpb_count);
  if ((media_check(dpbp) < 0) || (dpbp->dpb_count <= 0))
    return DE_INVLDDRV;

  ret = dos_cd(dpbp, s);
  --(dpbp->dpb_count);
  return ret;
}

COUNT DosFindFirst(UCOUNT attr, BYTE FAR * name)
{
  return dos_findfirst(attr, name);
}

COUNT DosFindNext(void)
{
  return dos_findnext();
}

COUNT DosGetFtime(COUNT hndl, date FAR * dp, time FAR * tp)
{
  sft FAR *s;
  sfttbl FAR *sp;

  if (hndl < 0)
    return DE_INVLDHNDL;

  if ((s = get_sft(hndl)) == (sft FAR *) - 1)
    return DE_INVLDHNDL;

  if (s->sft_count == 0)
    return DE_ACCESS;

  if (s->sft_flags & SFT_FDEVICE)
  {
    *dp = s->sft_date;
    *tp = s->sft_time;
    return SUCCESS;
  }

  return dos_getftime(s->sft_status, dp, tp);
}

COUNT DosSetFtime(COUNT hndl, date FAR * dp, time FAR * tp)
{
  sft FAR *s;
  sfttbl FAR *sp;

  if (hndl < 0)
    return DE_INVLDHNDL;

  if ((s = get_sft(hndl)) == (sft FAR *) - 1)
    return DE_INVLDHNDL;

  if (s->sft_count == 0)
    return DE_ACCESS;

  if (s->sft_flags & SFT_FDEVICE)
    return SUCCESS;

  return dos_setftime(s->sft_status, dp, tp);
}

COUNT DosGetFattr(BYTE FAR * name, UWORD FAR * attrp)
{
  return dos_getfattr(name, attrp);
}

COUNT DosSetFattr(BYTE FAR * name, UWORD FAR * attrp)
{
  return dos_setfattr(name, attrp);
}
