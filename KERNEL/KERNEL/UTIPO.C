#include "port.h"
#include "todo.h"

VOID pop_dmp(dmatch FAR *, struct f_node FAR *);

struct f_node FAR *dir_open(BYTE FAR * dirname)
{
  struct f_node FAR *fnp;
  COUNT drive;
  BYTE *p;
  WORD i;
  BYTE *s;
  BYTE *pszPath = &TempCDS.cdsCurrentPath[2];

  if ((fnp = get_f_node()) == (struct f_node FAR *)0)
  {
    return (struct f_node FAR *)NULL;
  }

  fnp->f_mode = RDWR;

  TempCDS.cdsFlags = 0;

  dirname = adjust_far(dirname);
  if (ParseDosName(dirname, &drive, (BYTE *) 0, (BYTE *) 0, (BYTE *) 0, FALSE)
      != SUCCESS)
  {
    release_f_node(fnp);
    return NULL;
  }

  if (drive >= 0)
  {
    dirname += 2;               
    TempCDS.cdsDpb = &blk_devices[drive];
  }
  else
  {
    TempCDS.cdsDpb = &blk_devices[drive = default_drive];
  }

  fnp->f_dpb = (struct dpb *)TempCDS.cdsDpb;
  TempCDS.cdsCurrentPath[0] = 'A' + drive;
  TempCDS.cdsCurrentPath[1] = ':';
  TempCDS.cdsJoinOffset = 2;

  if (drive >= nblkdev)
  {
    release_f_node(fnp);
    return NULL;
  }

  ParseDosPath(dirname,
               (COUNT *) 0, pszPath, (BYTE *) TempCDS.cdsDpb->dpb_path);

  ++TempCDS.cdsDpb->dpb_count;

  if (media_check((struct dpb *)TempCDS.cdsDpb) < 0)
  {
    --TempCDS.cdsDpb->dpb_count;
    release_f_node(fnp);
    return (struct f_node FAR *)0;
  }
  fnp->f_diroff = 0l;
  fnp->f_flags.f_dmod = FALSE;  
  fnp->f_flags.f_dnew = TRUE;
  fnp->f_dsize = DIRENT_SIZE * TempCDS.cdsDpb->dpb_dirents;


  fnp->f_flags.f_droot = TRUE;
  for (p = pszPath; *p != '\0';)
  {
    while (*p == '\\')
      ++p;
    if (*p == '\0')
      break;

    for (i = 0; i < FNAME_SIZE; i++)
    {
      if (*p != '\0' && *p != '.' && *p != '/' && *p != '\\')
        TempBuffer[i] = *p++;
      else
        break;
    }

    for (; i < FNAME_SIZE; i++)
      TempBuffer[i] = ' ';

    if (*p == '.')
      ++p;
    for (i = 0; i < FEXT_SIZE; i++)
    {
      if (*p != '\0' && *p != '.' && *p != '/' && *p != '\\')
        TempBuffer[i + FNAME_SIZE] = *p++;
      else
        break;
    }
    for (; i < FEXT_SIZE; i++)
      TempBuffer[i + FNAME_SIZE] = ' ';

    i = FALSE;

    upMem((BYTE FAR *) TempBuffer, FNAME_SIZE + FEXT_SIZE);

    while (dir_read(fnp) == DIRENT_SIZE)
    {
      if (fnp->f_dir.dir_name[0] != '\0' && fnp->f_dir.dir_name[0] != DELETED)
      {
        if (fcmp((BYTE FAR *) TempBuffer, (BYTE FAR *) fnp->f_dir.dir_name, FNAME_SIZE + FEXT_SIZE))
        {
          i = TRUE;
          break;
        }
      }
    }

    if (!i || !(fnp->f_dir.dir_attrib & D_DIR))
    {
      --TempCDS.cdsDpb->dpb_count;
      release_f_node(fnp);
      return (struct f_node FAR *)0;
    }
    else
    {
      fnp->f_flags.f_droot = FALSE;
      fnp->f_flags.f_ddir = TRUE;
      fnp->f_offset = 0l;
      fnp->f_cluster_offset = 0l; 
      fnp->f_highwater = 0l;
      fnp->f_cluster = fnp->f_dir.dir_start;
      fnp->f_dirstart = fnp->f_dir.dir_start;
      fnp->f_diroff = 0l;
      fnp->f_flags.f_dmod = FALSE;
      fnp->f_flags.f_dnew = TRUE;
      fnp->f_dsize = DIRENT_SIZE * TempCDS.cdsDpb->dpb_dirents;
    }
  }

  return fnp;
}

COUNT dir_read(REG struct f_node FAR * fnp)
{
  REG i;
  REG j;
  struct buffer FAR *bp;

  if (fnp->f_flags.f_dnew)
    fnp->f_flags.f_dnew = FALSE;
  else
    fnp->f_diroff += DIRENT_SIZE;

  if (fnp->f_flags.f_droot && fnp->f_diroff >= fnp->f_dsize)
  {
    fnp->f_diroff -= DIRENT_SIZE;
    return 0;
  }
  else
  {
    if (fnp->f_flags.f_droot)
    {
      if ((fnp->f_diroff / fnp->f_dpb->dpb_secsize
           + fnp->f_dpb->dpb_dirstrt)
          >= fnp->f_dpb->dpb_data)
      {
        fnp->f_flags.f_dfull = TRUE;
        return 0;
      }

      bp = getblock((ULONG) (fnp->f_diroff / fnp->f_dpb->dpb_secsize
                            + fnp->f_dpb->dpb_dirstrt),
                    fnp->f_dpb->dpb_unit);
      bp->b_flag &= ~(BFR_DATA | BFR_FAT);
      bp->b_flag |= BFR_DIR;
#ifdef DISPLAY_GETBLOCK
      printf("DIR (dir_read)\n");
#endif
    }
    else
    {
      REG UWORD secsize = fnp->f_dpb->dpb_secsize;

      fnp->f_offset = fnp->f_diroff;

#ifdef DISPLAY_GETBLOCK
      printf("dir_read: ");
#endif
      if (map_cluster(fnp, XFR_READ) != SUCCESS)
      {
        fnp->f_flags.f_dfull = TRUE;
        return 0;
      }

      if (fnp->f_cluster == FREE)
        return 0;

      if (last_link(fnp))
      {
        fnp->f_diroff -= DIRENT_SIZE;
        fnp->f_flags.f_dfull = TRUE;
        return 0;
      }

      fnp->f_sector =
          (fnp->f_offset / secsize)
          & fnp->f_dpb->dpb_clsmask;
      fnp->f_boff = fnp->f_offset % secsize;

      bp = getblock(
                     (ULONG) clus2phys(fnp->f_cluster,
                                      fnp->f_dpb->dpb_clssize,
                                      fnp->f_dpb->dpb_data)
                     + fnp->f_sector,
                     fnp->f_dpb->dpb_unit);
      bp->b_flag &= ~(BFR_DATA | BFR_FAT);
      bp->b_flag |= BFR_DIR;
#ifdef DISPLAY_GETBLOCK
      printf("DIR (dir_read)\n");
#endif
    }

    if (bp != NULL)
      getdirent((BYTE FAR *) & bp->b_buffer[fnp->f_diroff % fnp->f_dpb->dpb_secsize],
                (struct dirent FAR *)&fnp->f_dir);
    else
    {
      fnp->f_flags.f_dfull = TRUE;
      return 0;
    }

    fnp->f_flags.f_dfull = FALSE;
    fnp->f_flags.f_dmod = FALSE;

    if (fnp->f_dir.dir_name[0] == '\0')
      return 0;
    else
      return DIRENT_SIZE;
  }
}

#ifndef IPL
COUNT dir_write(REG struct f_node FAR * fnp)
{
  struct buffer FAR *bp;

  if (fnp->f_flags.f_dmod)
  {
    if (fnp->f_flags.f_droot)
    {
      bp = getblock(
                     (ULONG) (fnp->f_diroff / fnp->f_dpb->dpb_secsize
                             + fnp->f_dpb->dpb_dirstrt),
                     fnp->f_dpb->dpb_unit);
      bp->b_flag &= ~(BFR_DATA | BFR_FAT);
      bp->b_flag |= BFR_DIR;
#ifdef DISPLAY_GETBLOCK
     printf("DIR (dir_write)\n");
#endif
    }

    else
    {
      REG UWORD secsize = fnp->f_dpb->dpb_secsize;

      fnp->f_offset = fnp->f_diroff;
      fnp->f_back = LONG_LAST_CLUSTER;
      fnp->f_cluster = fnp->f_dirstart;
      fnp->f_cluster_offset = 0l;  

#ifdef DISPLAY_GETBLOCK
      printf("dir_write: ");
#endif
      if (map_cluster(fnp, XFR_READ) != SUCCESS)
      {
        fnp->f_flags.f_dfull = TRUE;
        release_f_node(fnp);
        return 0;
      }

      if (fnp->f_cluster == FREE)
      {
        release_f_node(fnp);
        return 0;
      }

      fnp->f_sector =
          (fnp->f_offset / secsize)
          & fnp->f_dpb->dpb_clsmask;
      fnp->f_boff = fnp->f_offset % secsize;

      bp = getblock(
                     (ULONG) clus2phys(fnp->f_cluster,
                                      fnp->f_dpb->dpb_clssize,
                                      fnp->f_dpb->dpb_data)
                     + fnp->f_sector,
                     fnp->f_dpb->dpb_unit);
      bp->b_flag &= ~(BFR_DATA | BFR_FAT);
      bp->b_flag |= BFR_DIR;
#ifdef DISPLAY_GETBLOCK
      printf("DIR (dir_write)\n");
#endif
    }

    if (bp == NULL)
    {
      release_f_node(fnp);
      return 0;
    }
    putdirent((struct dirent FAR *)&fnp->f_dir,
    (VOID FAR *) & bp->b_buffer[fnp->f_diroff % fnp->f_dpb->dpb_secsize]);
    bp->b_flag |= BFR_DIRTY;
  }
  return DIRENT_SIZE;
}
#endif

VOID dir_close(REG struct f_node FAR * fnp)
{
  REG COUNT disk = fnp->f_dpb->dpb_unit;

  if (fnp == NULL)
    return;

#ifndef IPL
  dir_write(fnp);

#endif
  flush_buffers(disk);

  --(fnp->f_dpb)->dpb_count;
  release_f_node(fnp);
}

#ifndef IPL
COUNT dos_findfirst(UCOUNT attr, BYTE FAR * name)
{
  REG struct f_node FAR *fnp;
  REG dmatch FAR *dmp = (dmatch FAR *) dta;
  REG COUNT i;
  COUNT nDrive;
  BYTE *p;
  static BYTE local_name[FNAME_SIZE + 1], local_ext[FEXT_SIZE + 1];

  dmp->dm_drive = default_drive;
  dmp->dm_entry = 0;
  dmp->dm_cluster = 0;

  dmp->dm_attr_srch = attr | D_RDONLY | D_ARCHIVE;

  i = ParseDosName(name, &nDrive, &LocalPath[2], local_name, local_ext, TRUE);
  if (i != SUCCESS)
    return i;

  if (nDrive >= 0)
  {
    dmp->dm_drive = nDrive;
  }
  else
    nDrive = default_drive;

  if (!LocalPath[2])
    strcpy(&LocalPath[2], ".");

  for (p = local_name, i = 0; i < FNAME_SIZE && *p && *p != '*'; ++p, ++i)
    SearchDir.dir_name[i] = *p;
  if (*p == '*') {
    for (; i < FNAME_SIZE; ++i)
      SearchDir.dir_name[i] = '?';
    while (*++p);
  }
  else
    for (; i < FNAME_SIZE; i++)
      SearchDir.dir_name[i] = ' ';

  for (p = local_ext, i = 0; i < FEXT_SIZE && *p && *p != '*'; ++p, ++i)
    SearchDir.dir_ext[i] = *p;
  if (*p == '*') {
    for (; i < FEXT_SIZE; ++i)
      SearchDir.dir_ext[i] = '?';
    while (*++p);
  }
  else
    for (; i < FEXT_SIZE; i++)
      SearchDir.dir_ext[i] = ' ';

  upMem(SearchDir.dir_name, FNAME_SIZE + FEXT_SIZE);

  fbcopy((BYTE FAR *)SearchDir.dir_name, dmp->dm_name_pat,
    FNAME_SIZE + FEXT_SIZE);

  if ((attr & ~(D_RDONLY | D_ARCHIVE)) == D_VOLID)
  {
    if ((fnp = dir_open((BYTE FAR *) "\\")) == NULL)
      return DE_PATHNOTFND;

    while (dir_read(fnp) == DIRENT_SIZE)
    {
      if ((fnp->f_dir.dir_attrib & ~(D_RDONLY | D_ARCHIVE)) == D_VOLID)
      {
        pop_dmp(dmp, fnp);
        dir_close(fnp);
        return SUCCESS;
      }
    }

    dir_close(fnp);
    return DE_FILENOTFND;
  }

  else
  {
    if (nDrive >= 0)
      LocalPath[0] = 'A' + nDrive;
    else
      LocalPath[0] = 'A' + default_drive;
    LocalPath[1] = ':';

    if ((fnp = dir_open((BYTE FAR *) LocalPath)) == NULL)
      return DE_PATHNOTFND;

    pop_dmp(dmp, fnp);
    dmp->dm_entry = 0;
    if (!fnp->f_flags.f_droot)
    {
      dmp->dm_cluster = fnp->f_dirstart;
      dmp->dm_dirstart = fnp->f_dirstart;
    }
    else
    {
      dmp->dm_cluster = 0;
      dmp->dm_dirstart = 0;
    }
    dir_close(fnp);
    return dos_findnext();
  }
}

COUNT dos_findnext(void)
{
  REG dmatch FAR *dmp = (dmatch FAR *) dta;
  REG struct f_node FAR *fnp;
  BOOL found = FALSE;
  BYTE FAR *p;
  BYTE FAR *q;

  dmp = (dmatch FAR *) dta;

  if ((fnp = get_f_node()) == (struct f_node FAR *)0)
  {
    return DE_FILENOTFND;
  }

  fnp->f_mode = RDWR;

  fnp->f_dpb = &blk_devices[dmp->dm_drive];
  ++(fnp->f_dpb)->dpb_count;
  if (media_check(fnp->f_dpb) < 0)
  {
    --(fnp->f_dpb)->dpb_count;
    release_f_node(fnp);
    return DE_FILENOTFND;
  }

  fnp->f_dsize = DIRENT_SIZE * (fnp->f_dpb)->dpb_dirents;

  if (dmp->dm_entry > 0)
    fnp->f_diroff = (dmp->dm_entry - 1) * DIRENT_SIZE;

  fnp->f_offset = fnp->f_highwater = fnp->f_diroff;
  fnp->f_cluster = dmp->dm_cluster;
  fnp->f_cluster_offset = 0l;  
  fnp->f_flags.f_dmod = dmp->dm_flags.f_dmod;
  fnp->f_flags.f_droot = dmp->dm_flags.f_droot;
  fnp->f_flags.f_dnew = dmp->dm_flags.f_dnew;
  fnp->f_flags.f_ddir = dmp->dm_flags.f_ddir;
  fnp->f_flags.f_dfull = dmp->dm_flags.f_dfull;

  fnp->f_dirstart = dmp->dm_dirstart;

  while (dir_read(fnp) == DIRENT_SIZE)
  {
    ++dmp->dm_entry;
    if (fnp->f_dir.dir_name[0] != '\0' && fnp->f_dir.dir_name[0] != DELETED)
    {
      if (fcmp_wild((BYTE FAR *) (dmp->dm_name_pat), (BYTE FAR *) fnp->f_dir.dir_name, FNAME_SIZE + FEXT_SIZE))
      {
        if (!(~dmp->dm_attr_srch & fnp->f_dir.dir_attrib))
        {
          found = TRUE;
          break;
        }
        else
          continue;
      }
    }
  }

  if (found)
    pop_dmp(dmp, fnp);

  --(fnp->f_dpb)->dpb_count;
  release_f_node(fnp);

  return found ? SUCCESS : DE_FILENOTFND;
}

static VOID pop_dmp(dmatch FAR * dmp, struct f_node FAR * fnp)
{
  COUNT idx;
  BYTE FAR *p;
  BYTE FAR *q;

  dmp->dm_attr_fnd = fnp->f_dir.dir_attrib;
  dmp->dm_time = fnp->f_dir.dir_time;
  dmp->dm_date = fnp->f_dir.dir_date;
  dmp->dm_size = fnp->f_dir.dir_size;
  dmp->dm_flags.f_droot = fnp->f_flags.f_droot;
  dmp->dm_flags.f_ddir = fnp->f_flags.f_ddir;
  dmp->dm_flags.f_dmod = fnp->f_flags.f_dmod;
  dmp->dm_flags.f_dnew = fnp->f_flags.f_dnew;
  p = dmp->dm_name;
  if (fnp->f_dir.dir_name[0] == '.')
  {
    for (idx = 0, q = (BYTE FAR *) fnp->f_dir.dir_name;
         idx < FNAME_SIZE; idx++)
    {
      if (*q == ' ')
        break;
      *p++ = *q++;
    }
  }
  else
  {
    for (idx = 0, q = (BYTE FAR *) fnp->f_dir.dir_name;
         idx < FNAME_SIZE; idx++)
    {
      if (*q == ' ')
        break;
      *p++ = *q++;
    }
    if (fnp->f_dir.dir_ext[0] != ' ')
    {
      *p++ = '.';
      for (idx = 0, q = (BYTE FAR *) fnp->f_dir.dir_ext; idx < FEXT_SIZE; idx++)
      {
        if (*q == ' ')
          break;
        *p++ = *q++;
      }
    }
  }
  *p++ = NULL;
}
#endif
