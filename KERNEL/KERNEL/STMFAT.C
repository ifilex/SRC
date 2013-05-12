#include "port.h"
#include "todo.h"

struct f_node FAR *xlt_fd(COUNT);
COUNT xlt_fnp(struct f_node FAR *);
struct f_node FAR *split_path(BYTE FAR *, BYTE *, BYTE *, BYTE *);
BOOL find_fname(struct f_node FAR *, BYTE *, BYTE *);
date dos_getdate(VOID);
time dos_gettime(VOID);
BOOL find_free(struct f_node FAR *);
UWORD find_fat_free(struct f_node FAR *);
VOID wipe_out(struct f_node FAR *);
BOOL last_link(struct f_node FAR *);
BOOL extend(struct f_node FAR *);
COUNT extend_dir(struct f_node FAR *);
BOOL first_fat(struct f_node FAR *);
COUNT map_cluster(struct f_node FAR *, COUNT);

COUNT dos_open(BYTE FAR * path, COUNT flag)
{
  REG struct f_node FAR *fnp;
  COUNT i;
  BYTE FAR *fnamep;

  if (flag < 0 || flag > 2)
    return DE_INVLDACC;

  if ((fnp = split_path(path, szDirName, szFileName, szFileExt)) == NULL)
  {
    dir_close(fnp);
    return DE_PATHNOTFND;
  }

  if (!find_fname(fnp, szFileName, szFileExt))
  {
    dir_close(fnp);
    return DE_FILENOTFND;
  }

  fnp->f_mode = flag;

  fnp->f_offset = 0l;
  fnp->f_highwater = fnp->f_dir.dir_size;

  fnp->f_back = LONG_LAST_CLUSTER;
  fnp->f_cluster = fnp->f_dir.dir_start;
  fnp->f_cluster_offset = 0l;  

  fnp->f_flags.f_dmod = FALSE;
  fnp->f_flags.f_dnew = FALSE;
  fnp->f_flags.f_ddir = FALSE;

  return xlt_fnp(fnp);
}

COUNT FAR init_call_dos_open(BYTE FAR * path, COUNT flag)
{
  return dos_open(path, flag);
}

BOOL fcmp(BYTE FAR * s1, BYTE FAR * s2, COUNT n)
{
  while (n--)
    if (*s1++ != *s2++)
      return FALSE;
  return TRUE;
}

BOOL fcmp_wild(BYTE FAR * s1, BYTE FAR * s2, COUNT n)
{
  while (n--)
  {
    if (*s1 == '?')
    {
      ++s1, ++s2;
      continue;
    }
    if (*s1++ != *s2++)
      return FALSE;
  }
  return TRUE;
}

COUNT dos_close(COUNT fd)
{
  struct f_node FAR *fnp;

  fnp = xlt_fd(fd);

  if (fnp == (struct f_node FAR *)0 || fnp->f_count <= 0)
    return DE_INVLDHNDL;

  if (fnp->f_mode != RDONLY)
  {
    fnp->f_dir.dir_size = fnp->f_highwater;
    fnp->f_flags.f_dmod = TRUE;
  }
  fnp->f_flags.f_ddir = TRUE;

  dir_close(fnp);
  return SUCCESS;
}

COUNT FAR init_call_dos_close(COUNT fd)
{
  return dos_close(fd);
}

static struct f_node FAR *split_path(BYTE FAR * path, BYTE * dname,
                                     BYTE * fname, BYTE * fext)
{
  REG struct f_node FAR *fnp;
  COUNT nDrive;
  struct dpb *dpbp;

  if (ParseDosName(adjust_far(path), &nDrive, &dname[2], fname, fext, FALSE)
      != SUCCESS)
    return (struct f_node FAR *)0;
  if (nDrive < 0)
    nDrive = default_drive;

  dname[0] = 'A' + nDrive;
  dname[1] = ':';

  SpacePad(fname, FNAME_SIZE);
  SpacePad(fext, FEXT_SIZE);

  if (!dname[2])
  {
    dpbp = &blk_devices[nDrive];
    fsncopy((BYTE FAR *) dpbp->dpb_path,
            (BYTE FAR *) & dname[2],
            PARSE_MAX);
  }

  fnp = dir_open((BYTE FAR *) dname);

  if (fnp == (struct f_node FAR *)0 || fnp->f_count <= 0)
  {
    dir_close(fnp);
    return (struct f_node FAR *)0;
  }

  upMem((BYTE FAR *) dname, strlen(dname));
  upMem((BYTE FAR *) fname, FNAME_SIZE);
  upMem((BYTE FAR *) fext, FEXT_SIZE);

  return fnp;
}

static BOOL find_fname(struct f_node FAR * fnp, BYTE * fname, BYTE * fext)
{
  BOOL found = FALSE;

  while (dir_read(fnp) == DIRENT_SIZE)
  {
    if (fnp->f_dir.dir_name[0] != '\0')
    {
      if (fnp->f_dir.dir_name[0] == DELETED)
        continue;

      if (fcmp((BYTE FAR *) fname, (BYTE FAR *) fnp->f_dir.dir_name, FNAME_SIZE)
          && fcmp((BYTE FAR *) fext, (BYTE FAR *) fnp->f_dir.dir_ext, FEXT_SIZE)
					&& ((fnp->f_dir.dir_attrib & D_VOLID) == 0))
      {
        found = TRUE;
        break;
      }
    }
  }
  return found;
}

COUNT dos_creat(BYTE FAR * path, COUNT attrib)
{
  REG struct f_node FAR *fnp;

  if ((fnp = split_path(path, szDirName, szFileName, szFileExt)) == NULL)
  {
    dir_close(fnp);
    return DE_PATHNOTFND;
  }

  if (find_fname(fnp, szFileName, szFileExt))
  {
    if ((fnp->f_dir.dir_attrib & (D_RDONLY | D_DIR | D_VOLID))
        || (fnp->f_dir.dir_attrib & ~D_ARCHIVE & ~attrib))
    {
      dir_close(fnp);
      return DE_ACCESS;
    }

    wipe_out(fnp);
  }
  else
  {
    BOOL is_free;
    REG COUNT idx;
    struct buffer FAR *bp;
    BYTE FAR *p;

    fnp->f_flags.f_dmod = FALSE;
    dir_close(fnp);
    fnp = dir_open((BYTE FAR *) szDirName);

    if (!(is_free = find_free(fnp)) && (fnp->f_flags.f_droot))
    {
      fnp->f_flags.f_dmod = FALSE;
      dir_close(fnp);
      return DE_TOOMANY;
    }

    else if (!is_free && !(fnp->f_flags.f_droot))
    {
      COUNT ret;

      if ((ret = extend_dir(fnp)) != SUCCESS)
        return ret;
    }

    fbcopy((BYTE FAR *) szFileName,
           (BYTE FAR *) fnp->f_dir.dir_name, FNAME_SIZE);
    fbcopy((BYTE FAR *) szFileExt,
           (BYTE FAR *) fnp->f_dir.dir_ext, FEXT_SIZE);
  }
  fnp->f_mode = RDWR;

  fnp->f_dir.dir_size = 0l;
  fnp->f_dir.dir_start = FREE;
  fnp->f_dir.dir_attrib = attrib | D_ARCHIVE;
  fnp->f_dir.dir_time = dos_gettime();
  fnp->f_dir.dir_date = dos_getdate();

  fnp->f_flags.f_dmod = TRUE;
  fnp->f_flags.f_dnew = FALSE;
  fnp->f_flags.f_ddir = TRUE;
  if (dir_write(fnp) != DIRENT_SIZE)
  {
    release_f_node(fnp);
    return DE_ACCESS;
  }

  fnp->f_offset = 0l;
  fnp->f_highwater = 0l;

  fnp->f_back = LONG_LAST_CLUSTER;
  fnp->f_cluster = fnp->f_dir.dir_start = FREE;
  fnp->f_cluster_offset = 0l;  
  fnp->f_flags.f_dmod = TRUE;
  fnp->f_flags.f_dnew = FALSE;
  fnp->f_flags.f_ddir = FALSE;

  return xlt_fnp(fnp);
}

COUNT dos_delete(BYTE FAR * path)
{
  REG struct f_node FAR *fnp;

  if ((fnp = split_path(path, szDirName, szFileName, szFileExt)) == NULL)
  {
    dir_close(fnp);
    return DE_PATHNOTFND;
  }

  if (find_fname(fnp, szFileName, szFileExt))
  {
    if (fnp->f_dir.dir_attrib & ~D_ARCHIVE)
    {
      dir_close(fnp);
      return DE_ACCESS;
    }

    wipe_out(fnp);
    fnp->f_dir.dir_size = 0l;
    *(fnp->f_dir.dir_name) = DELETED;

    fnp->f_flags.f_dmod = TRUE;
    dir_close(fnp);

    return SUCCESS;
  }
  else
  {
    dir_close(fnp);
    return DE_FILENOTFND;
  }
}

COUNT dos_rmdir(BYTE FAR * path)
{
  REG struct f_node FAR *fnp;
  REG struct f_node FAR *fnp1;
  BOOL found;

  if ((fnp = split_path(path, szDirName, szFileName, szFileExt)) == NULL)
  {
    dir_close(fnp);
    return DE_PATHNOTFND;
  }

  if ((path[0] == '\\') && (path[1] == NULL))
  {
    dir_close(fnp);
    return DE_ACCESS;
  }

  if (find_fname(fnp, szFileName, szFileExt))
  {
    if (fnp->f_dir.dir_attrib & ~D_DIR)
    {
      dir_close(fnp);
      return DE_ACCESS;
    }

    fnp->f_flags.f_dmod = FALSE;
    fnp1 = dir_open((BYTE FAR *) path);
    dir_read(fnp1);
    if (fnp1->f_dir.dir_name[0] != '.')
    {
      dir_close(fnp);
      return DE_ACCESS;
    }

    dir_read(fnp1);
    if (fnp1->f_dir.dir_name[0] != '.')
    {
      dir_close(fnp);
      return DE_ACCESS;
    }

    found = FALSE;
    while (dir_read(fnp1) == DIRENT_SIZE)
    {
      if (fnp1->f_dir.dir_name[0] == '\0')
        break;
      if (fnp1->f_dir.dir_name[0] == DELETED)
        continue;
      else
      {
        found = TRUE;
        break;
      }
    }

    dir_close(fnp1);
    if (found)
    {
      dir_close(fnp);
      return DE_ACCESS;
    }

    wipe_out(fnp);
    fnp->f_dir.dir_size = 0l;
    *(fnp->f_dir.dir_name) = DELETED;

    fnp->f_flags.f_dmod = TRUE;
    dir_close(fnp);

    return SUCCESS;
  }
  else
  {
    dir_close(fnp);
    return DE_FILENOTFND;
  }
}

COUNT dos_rename(BYTE FAR * path1, BYTE FAR * path2)
{
  REG struct f_node FAR *fnp1;
  REG struct f_node FAR *fnp2;
  BOOL is_free;

  if ((fnp2 = split_path(path2, szSecDirName, szSecFileName, szSecFileExt)) == NULL)
  {
    dir_close(fnp2);
    return DE_PATHNOTFND;
  }

  if (find_fname(fnp2, szSecFileName, szSecFileExt))
  {
    dir_close(fnp2);
    return DE_ACCESS;
  }

  if ((fnp1 = split_path(path1, szPriDirName, szPriFileName, szPriFileExt)) == NULL)
  {
    dir_close(fnp1);
    dir_close(fnp2);
    return DE_PATHNOTFND;
  }

  fnp2->f_flags.f_dmod = FALSE;
  dir_close(fnp2);
  fnp2 = dir_open((BYTE FAR *) szSecDirName);

  if (!(is_free = find_free(fnp2)) && (fnp2->f_flags.f_droot))
  {
    fnp2->f_flags.f_dmod = FALSE;
    dir_close(fnp1);
    dir_close(fnp2);
    return DE_TOOMANY;
  }

  else if (!is_free && !(fnp2->f_flags.f_droot))
  {
    COUNT ret;

    if ((ret = extend_dir(fnp2)) != SUCCESS)
      return ret;
  }

  if (!find_fname(fnp1, szPriFileName, szPriFileExt))
  {
    dir_close(fnp1);
    dir_close(fnp2);
    return DE_FILENOTFND;
  }

  fbcopy((BYTE FAR *) szSecFileName,
         (BYTE FAR *) fnp2->f_dir.dir_name, FNAME_SIZE);
  fbcopy((BYTE FAR *) szSecFileExt,
         (BYTE FAR *) fnp2->f_dir.dir_ext, FEXT_SIZE);

  fnp2->f_dir.dir_size = fnp1->f_dir.dir_size;
  fnp2->f_dir.dir_start = fnp1->f_dir.dir_start;
  fnp2->f_dir.dir_attrib = fnp1->f_dir.dir_attrib;
  fnp2->f_dir.dir_time = fnp1->f_dir.dir_time;
  fnp2->f_dir.dir_date = fnp1->f_dir.dir_date;

  fnp1->f_flags.f_dmod = fnp2->f_flags.f_dmod = TRUE;
  fnp1->f_flags.f_dnew = fnp2->f_flags.f_dnew = FALSE;
  fnp1->f_flags.f_ddir = fnp2->f_flags.f_ddir = TRUE;

  fnp2->f_highwater = fnp2->f_offset = fnp1->f_dir.dir_size;

  fnp1->f_dir.dir_size = 0l;
  *(fnp1->f_dir.dir_name) = DELETED;

  dir_close(fnp1);
  dir_close(fnp2);

  return SUCCESS;
}

static VOID wipe_out(struct f_node FAR * fnp)
{
  REG UWORD st,
    next;
  struct dpb *dpbp = fnp->f_dpb;

  if ((fnp == NULL) || (fnp->f_dir.dir_start == FREE))
    return;

  if (fnp->f_dir.dir_start == FREE)
    return;

  for (st = fnp->f_dir.dir_start;
       st != LONG_LAST_CLUSTER;)
  {
    next = next_cluster(dpbp, st);

    if (next == FREE)
      return;

    link_fat(dpbp, st, FREE);

    if ((dpbp->dpb_cluster == UNKNCLUSTER)
        || (dpbp->dpb_cluster > st))
      dpbp->dpb_cluster = st;

    st = next;
  }
}

static BOOL find_free(struct f_node FAR * fnp)
{
  while (dir_read(fnp) == DIRENT_SIZE)
  {
    if (fnp->f_dir.dir_name[0] == '\0'
        || fnp->f_dir.dir_name[0] == DELETED)
    {
      return TRUE;
    }
  }
  return !fnp->f_flags.f_dfull;
}

date dos_getdate()
{
#ifndef NOTIME
  BYTE WeekDay,
    Month,
    MonthDay;
  COUNT Year;
  date Date;

  DosGetDate((BYTE FAR *) & WeekDay,
             (BYTE FAR *) & MonthDay,
             (BYTE FAR *) & Month,
             (COUNT FAR *) & Year);
  Date = DT_ENCODE( MonthDay, Month, Year - EPOCH_YEAR);
  return Date;

#else

  return 0;

#endif
}

date FAR init_call_dos_getdate()
{
  return dos_getdate();
}

time dos_gettime()
{
#ifndef NOTIME
  BYTE Hour,
    Minute,
    Second,
    Hundredth;
  time Time;
  BYTE h;

  DosGetTime((BYTE FAR *) & Hour,
             (BYTE FAR *) & Minute,
             (BYTE FAR *) & Second,
             (BYTE FAR *) & Hundredth);
  h = Second * 10 + ((Hundredth + 5) / 10);
  Time = TM_ENCODE(Hour, Minute, h);
  return Time;
#else
  return 0;
#endif
}

time FAR init_call_dos_gettime()
{
  return dos_gettime();
}

COUNT dos_getftime(COUNT fd, date FAR * dp, time FAR * tp)
{
  struct f_node FAR *fnp;

  fnp = xlt_fd(fd);

  if (fnp == (struct f_node FAR *)0 || fnp->f_count <= 0)
    return DE_INVLDHNDL;

  *dp = fnp->f_dir.dir_date;
  *tp = fnp->f_dir.dir_time;

  return SUCCESS;
}

COUNT dos_setftime(COUNT fd, date FAR * dp, time FAR * tp)
{
  struct f_node FAR *fnp;

  fnp = xlt_fd(fd);

  if (fnp == (struct f_node FAR *)0 || fnp->f_count <= 0)
    return DE_INVLDHNDL;

  fnp->f_dir.dir_date = *dp;
  fnp->f_dir.dir_time = *tp;

  return SUCCESS;
}

LONG dos_getcufsize(COUNT fd)
{
  struct f_node FAR *fnp;

  fnp = xlt_fd(fd);

  if (fnp == (struct f_node FAR *)0 || fnp->f_count <= 0)
    return -1l;

  return fnp->f_highwater;
}

LONG dos_getfsize(COUNT fd)
{
  struct f_node FAR *fnp;

  fnp = xlt_fd(fd);

  if (fnp == (struct f_node FAR *)0 || fnp->f_count <= 0)
    return -1l;

  return fnp->f_dir.dir_size;
}

BOOL dos_setfsize(COUNT fd, LONG size)
{
  struct f_node FAR *fnp;

  fnp = xlt_fd(fd);

  if (fnp == (struct f_node FAR *)0 || fnp->f_count <= 0)
    return FALSE;

  fnp->f_dir.dir_size = size;
  fnp->f_highwater = size;
  return TRUE;
}

static UWORD find_fat_free(struct f_node FAR * fnp)
{
  REG UWORD idx;

#ifdef DISPLAY_GETBLOCK
  printf("[find_fat_free]\n");
#endif
  if (fnp->f_dpb->dpb_cluster != UNKNCLUSTER)
    idx = fnp->f_dpb->dpb_cluster;
  else
    idx = 2;

  for (; idx < fnp->f_dpb->dpb_size; idx++)
  {
    if (next_cluster(fnp->f_dpb, idx) == FREE)
      break;
  }

  if (idx >= fnp->f_dpb->dpb_size)
  {
    fnp->f_dpb->dpb_cluster = UNKNCLUSTER;
    dir_close(fnp);
    return LONG_LAST_CLUSTER;
  }

  fnp->f_dpb->dpb_cluster = idx;
  return idx;
}

COUNT dos_mkdir(BYTE FAR * dir)
{
  REG struct f_node FAR *fnp;
  REG COUNT idx;
  struct buffer FAR *bp;
  BYTE FAR *p;
  UWORD free_fat;
  UWORD parent;

  if ((fnp = split_path(dir, szDirName, szFileName, szFileExt)) == NULL)
  {
    dir_close(fnp);
    return DE_PATHNOTFND;
  }

  if (find_fname(fnp, szFileName, szFileExt))
  {
    dir_close(fnp);
    return DE_ACCESS;
  }
  else
  {
    BOOL is_free;

    fnp->f_flags.f_dmod = FALSE;
    parent = fnp->f_dirstart;
    dir_close(fnp);
    fnp = dir_open((BYTE FAR *) szDirName);

    if (!(is_free = find_free(fnp)) && (fnp->f_flags.f_droot))
    {
      fnp->f_flags.f_dmod = FALSE;
      dir_close(fnp);
      return DE_TOOMANY;
    }

    else if (!is_free && !(fnp->f_flags.f_droot))
    {
      COUNT ret;

      if ((ret = extend_dir(fnp)) != SUCCESS)
        return ret;
    }

    fbcopy((BYTE FAR *) szFileName,
           (BYTE FAR *) fnp->f_dir.dir_name, FNAME_SIZE);
    fbcopy((BYTE FAR *) szFileExt,
           (BYTE FAR *) fnp->f_dir.dir_ext, FEXT_SIZE);

    fnp->f_mode = WRONLY;
    fnp->f_back = LONG_LAST_CLUSTER;

    fnp->f_dir.dir_size = 0l;
    fnp->f_dir.dir_start = FREE;
    fnp->f_dir.dir_attrib = D_DIR;
    fnp->f_dir.dir_time = dos_gettime();
    fnp->f_dir.dir_date = dos_getdate();

    fnp->f_flags.f_dmod = TRUE;
    fnp->f_flags.f_dnew = FALSE;
    fnp->f_flags.f_ddir = TRUE;

    fnp->f_highwater = 0l;
    fnp->f_offset = 0l;
  }

  free_fat = find_fat_free(fnp);

  if (free_fat == LONG_LAST_CLUSTER)
  {
    dir_close(fnp);
    return DE_HNDLDSKFULL;
  }

  fnp->f_dir.dir_start = fnp->f_cluster = free_fat;
  link_fat(fnp->f_dpb, (UCOUNT) free_fat, LONG_LAST_CLUSTER);

  bp = getblock((ULONG) clus2phys(free_fat,
                                 fnp->f_dpb->dpb_clssize,
                                 fnp->f_dpb->dpb_data),
                fnp->f_dpb->dpb_unit);
#ifdef DISPLAY_GETBLOCK
  printf("stmfat.c: FAT (dos_mkdir)\n");
#endif
  if (bp == NULL)
  {
    dir_close(fnp);
    return DE_BLKINVLD;
  }

  bcopy(".       ", (BYTE *) DirEntBuffer.dir_name, FNAME_SIZE);
  bcopy("   ", (BYTE *) DirEntBuffer.dir_ext, FEXT_SIZE);
  DirEntBuffer.dir_attrib = D_DIR;
  DirEntBuffer.dir_time = dos_gettime();
  DirEntBuffer.dir_date = dos_getdate();
  DirEntBuffer.dir_start = free_fat;
  DirEntBuffer.dir_size = 0l;

  putdirent((struct dirent FAR *)&DirEntBuffer, (BYTE FAR *) bp->b_buffer);

  bcopy("..      ", (BYTE *) DirEntBuffer.dir_name, FNAME_SIZE);
  DirEntBuffer.dir_start = parent;

  putdirent((struct dirent FAR *)&DirEntBuffer, (BYTE FAR *) & bp->b_buffer[DIRENT_SIZE]);

  for (p = (BYTE FAR *) & bp->b_buffer[2 * DIRENT_SIZE];
       p < &bp->b_buffer[BUFFERSIZE];)
    *p++ = NULL;

  bp->b_flag |= BFR_DIRTY;

  for (idx = 1; idx < fnp->f_dpb->dpb_clssize; idx++)
  {
    REG COUNT i;

    bp = getblock(
                   (ULONG) clus2phys(fnp->f_dir.dir_start,
                                    fnp->f_dpb->dpb_clssize,
                                    fnp->f_dpb->dpb_data) + idx,
                   fnp->f_dpb->dpb_unit);
#ifdef DISPLAY_GETBLOCK
    printf("stmfat.c: DIR (dos_mkdir)\n");
#endif
    if (bp == NULL)
    {
      dir_close(fnp);
      return DE_BLKINVLD;
    }
    for (i = 0, p = (BYTE FAR *) bp->b_buffer; i < BUFFERSIZE; i++)
      *p++ = NULL;
    bp->b_flag |= BFR_DIRTY;
  }

  flush_buffers((COUNT) (fnp->f_dpb->dpb_unit));

  fnp->f_flags.f_dmod = TRUE;
  dir_close(fnp);

  return SUCCESS;
}

BOOL last_link(struct f_node FAR * fnp)
{
  return (((UWORD) fnp->f_cluster == (UWORD) LONG_LAST_CLUSTER));
}

static BOOL extend(struct f_node FAR * fnp)
{
  UWORD free_fat;

#ifdef DISPLAY_GETBLOCK
  printf("extendido\n");
#endif
  free_fat = find_fat_free(fnp);

  if (free_fat == LONG_LAST_CLUSTER)
    return FALSE;

  link_fat(fnp->f_dpb, (UCOUNT) fnp->f_back, free_fat);
  fnp->f_cluster = free_fat;
  link_fat(fnp->f_dpb, (UCOUNT) free_fat, LONG_LAST_CLUSTER);

  fnp->f_flags.f_dmod = TRUE;
  return TRUE;
}

static COUNT extend_dir(struct f_node FAR * fnp)
{
  REG COUNT idx;

  if (!extend(fnp))
  {
    dir_close(fnp);
    return DE_HNDLDSKFULL;
  }

  for (idx = 0; idx < fnp->f_dpb->dpb_clssize; idx++)
  {
    REG COUNT i;
    REG BYTE FAR *p;
    REG struct buffer FAR *bp;

    bp = getblock(
                   (ULONG) clus2phys(fnp->f_cluster,
                                    fnp->f_dpb->dpb_clssize,
                                    fnp->f_dpb->dpb_data) + idx,
                   fnp->f_dpb->dpb_unit);
#ifdef DISPLAY_GETBLOCK
    printf("stmfat.c: DIR (extend_dir)\n");
#endif
    if (bp == NULL)
    {
      dir_close(fnp);
      return DE_BLKINVLD;
    }
    for (i = 0, p = (BYTE FAR *) bp->b_buffer; i < BUFFERSIZE; i++)
      *p++ = NULL;
    bp->b_flag |= BFR_DIRTY;
  }

  if (!find_free(fnp))
  {
    dir_close(fnp);
    return DE_HNDLDSKFULL;
  }

  flush_buffers((COUNT) (fnp->f_dpb->dpb_unit));

  return SUCCESS;

}

static BOOL first_fat(struct f_node FAR * fnp)
{
  UWORD free_fat;

  free_fat = find_fat_free(fnp);

  if (free_fat == LONG_LAST_CLUSTER)
    return FALSE;

  fnp->f_dir.dir_start = free_fat;
  link_fat(fnp->f_dpb, (UCOUNT) free_fat, LONG_LAST_CLUSTER);

  fnp->f_flags.f_dmod = TRUE;
  return TRUE;
}


COUNT map_cluster(REG struct f_node FAR * fnp, COUNT mode)
{
  ULONG idx;
  UWORD clssize;
  UWORD secsize;

#ifdef DISPLAY_GETBLOCK
  printf("stmfat.c: map_cluster, corriente %lu, fuera de rango %lu, diff=%lu ",
      fnp->f_cluster_offset,fnp->f_offset,
      fnp->f_offset - fnp->f_cluster_offset);
#endif
  secsize = fnp->f_dpb->dpb_secsize;
  clssize = secsize * fnp->f_dpb->dpb_clssize;

  if ((mode == XFR_WRITE) && (fnp->f_dir.dir_start == FREE))
  {
    if (!first_fat(fnp))
      return DE_HNDLDSKFULL;
  }

  if (fnp->f_offset >= fnp->f_cluster_offset)  
  {
    idx = fnp->f_offset - fnp->f_cluster_offset;
  }
  else
  {
    idx = fnp->f_offset;

    fnp->f_cluster = fnp->f_flags.f_ddir ? fnp->f_dirstart :
                                           fnp->f_dir.dir_start;
    fnp->f_cluster_offset = 0;
  }

  for (; idx >= clssize; idx -= clssize)
  {
    if ((mode == XFR_READ) && last_link(fnp))
      return DE_SEEK;
    else if ((mode == XFR_WRITE) && last_link(fnp))
    {

      if (!extend(fnp))
      {
        dir_close(fnp);
        return DE_HNDLDSKFULL;
      }
    }
    fnp->f_back = fnp->f_cluster;

    fnp->f_cluster = next_cluster(fnp->f_dpb, fnp->f_cluster);
    fnp->f_cluster_offset += clssize;
  }
#ifdef DISPLAY_GETBLOCK
    printf("listo.\n");
#endif

  return SUCCESS;
}


UCOUNT readblock(COUNT fd, VOID FAR * buffer, UCOUNT count, COUNT * err)
{
  REG struct f_node FAR *fnp;
  REG struct buffer FAR *bp;
  UCOUNT xfr_cnt = 0;
  UCOUNT ret_cnt = 0;
  ULONG idx;
  UWORD secsize;
  UCOUNT to_xfer = count;

#ifdef DEBUG
  if (bDumpRdWrParms)
  {
    printf("stmfat.c: readblock:\n");
    printf("stmfat.c: FD   BUFFERS    COUNT\nstmfat.c: --   -------    -----\n");
    printf("stmfat.c: %02d   %04x:%04x   %d\n",
           fd, (COUNT) FP_SEG(buffer), (COUNT) FP_OFF(buffer), count);
  }
#endif
  fnp = xlt_fd(fd);

  if (fnp == (struct f_node FAR *)0 || fnp->f_count <= 0)
  {
    *err = DE_INVLDHNDL;
    return 0;
  }

  if (count == 0)
  {
    *err = SUCCESS;
    return 0;
  }

  if (!fnp->f_flags.f_ddir && (fnp->f_offset >= fnp->f_dir.dir_size))
  {
    *err = SUCCESS;
    return 0;
  }

  if (fnp->f_mode != RDONLY && fnp->f_mode != RDWR)
  {
    *err = DE_INVLDACC;
    return 0;
  }

  secsize = fnp->f_dpb->dpb_secsize;

  buffer = adjust_far((VOID FAR *) buffer);

  while (ret_cnt < count)
  {
    if (fnp->f_offset == 0l)
    {
      fnp->f_cluster = fnp->f_flags.f_ddir ? fnp->f_dirstart :
                                           fnp->f_dir.dir_start;

      fnp->f_cluster_offset = 0l;
      fnp->f_back = LONG_LAST_CLUSTER;
      fnp->f_sector = 0;
      fnp->f_boff = 0;
    }
    else
    {
#ifdef DISPLAY_GETBLOCK
      printf("stmfat.c: readblock: ");
#endif
      switch (map_cluster(fnp, XFR_READ))
      {
        case DE_SEEK:
          *err = DE_SEEK;
          dir_close(fnp);
          return ret_cnt;

        default:
          dir_close(fnp);
          *err = DE_HNDLDSKFULL;
          return ret_cnt;

        case SUCCESS:
          break;
      }
    }

    fnp->f_sector = (fnp->f_offset / secsize) & fnp->f_dpb->dpb_clsmask;
    fnp->f_boff = fnp->f_offset & (secsize - 1);

#ifdef DSK_DEBUG
    printf("stmfat.c: lectura de %d interlazado; directorio %ld, cluster %d\n",
           fnp->f_count,
           fnp->f_diroff,
           fnp->f_cluster);
#endif
    if (!(fnp->f_flags.f_ddir)
        && (fnp->f_offset >= fnp->f_dir.dir_size))
    {
      *err = SUCCESS;
      return ret_cnt;
    }

    bp = getblock(
                   (ULONG) clus2phys(fnp->f_cluster,
                                    fnp->f_dpb->dpb_clssize,
                                    fnp->f_dpb->dpb_data) + fnp->f_sector,
                   fnp->f_dpb->dpb_unit);

#ifdef DISPLAY_GETBLOCK
    printf("stmfat.c: DATA (readblock)\n");
#endif
    if (bp == (struct buffer *)0)
    {
      *err = DE_BLKINVLD;
      return ret_cnt;
    }

    if (fnp->f_flags.f_ddir)
      xfr_cnt = min(to_xfer, secsize - fnp->f_boff);
    else
      xfr_cnt = min(min(to_xfer, secsize - fnp->f_boff),
                    fnp->f_dir.dir_size - fnp->f_offset);

    fbcopy((BYTE FAR *) & bp->b_buffer[fnp->f_boff], buffer, xfr_cnt);

    ret_cnt += xfr_cnt;
    to_xfer -= xfr_cnt;
    fnp->f_offset += xfr_cnt;
    buffer = add_far((VOID FAR *) buffer, (ULONG) xfr_cnt);
  }
  *err = SUCCESS;
  return ret_cnt;
}

UCOUNT writeblock(COUNT fd, VOID FAR * buffer, UCOUNT count, COUNT * err)
{
  REG struct f_node FAR *fnp;
  struct buffer FAR *bp;
  UCOUNT xfr_cnt = 0;
  UCOUNT ret_cnt = 0;
  ULONG idx;
  UWORD secsize;
  UCOUNT to_xfer = count;

#ifdef DEBUG
  if (bDumpRdWrParms)
  {
    printf("stmfat.c: writeblock:\n");
    printf("stmfat.c: FD   BUFFERS    COUNT\nstmfat.c: --   -------    -----\n");
    printf("stmfat.c: %02d   %04x:%04x   %d\n",
           fd, (COUNT) FP_SEG(buffer), (COUNT) FP_OFF(buffer), count);
  }
#endif
  fnp = xlt_fd(fd);

  if (fnp == (struct f_node FAR *)0 || fnp->f_count <= 0)
  {
    *err = DE_INVLDHNDL;
    return 0;
  }

  if (count == 0)
  {
    fnp->f_highwater = fnp->f_offset;

    *err = SUCCESS;
    return 0;
  }

  if (fnp->f_mode != WRONLY && fnp->f_mode != RDWR)
  {
    *err = DE_INVLDACC;
    return 0;
  }

  secsize = fnp->f_dpb->dpb_secsize;

  buffer = adjust_far((VOID FAR *) buffer);

  while (ret_cnt < count)
  {
    if (fnp->f_offset == 0l)
    {
      if (fnp->f_dir.dir_start == FREE)
        if (!first_fat(fnp))
        {                   
          dir_close(fnp);
          *err = DE_HNDLDSKFULL;
          return ret_cnt;
        }
      fnp->f_cluster = fnp->f_flags.f_ddir ? fnp->f_dirstart :
                                           fnp->f_dir.dir_start;

      fnp->f_cluster_offset = 0l;
      fnp->f_back = LONG_LAST_CLUSTER;
      fnp->f_sector = 0;
      fnp->f_boff = 0;
    }

    else
    {
#ifdef DISPLAY_GETBLOCK
      printf("stmfat.c: writeblock: ");
#endif
      switch (map_cluster(fnp, XFR_WRITE))
      {
        case DE_SEEK:
          *err = DE_SEEK;
          dir_close(fnp);
          return ret_cnt;

        default:
          dir_close(fnp);
          *err = DE_HNDLDSKFULL;
          return ret_cnt;

        case SUCCESS:
          break;
      }
    }

    if (last_link(fnp))
      if (!extend(fnp))
      {
        dir_close(fnp);
        *err = DE_HNDLDSKFULL;
        return ret_cnt;
      }

    fnp->f_sector =
        (fnp->f_offset / secsize) & fnp->f_dpb->dpb_clsmask;
    fnp->f_boff = fnp->f_offset & (secsize - 1);

#ifdef DSK_DEBUG
    printf("stmfat.c: escritura de %d interlazado; directorio %ld, cluster %d\n",
           fnp->f_count,
           fnp->f_diroff,
           fnp->f_cluster);
#endif

    if (!getbuf(&bp,
                   (ULONG) clus2phys(fnp->f_cluster,
                                    fnp->f_dpb->dpb_clssize,
                                    fnp->f_dpb->dpb_data) + fnp->f_sector,
                   fnp->f_dpb->dpb_unit))
    {
      *err = DE_BLKINVLD;
      return ret_cnt;
    }

    xfr_cnt = min(to_xfer, secsize - fnp->f_boff);
    fbcopy(buffer, (BYTE FAR *) & bp->b_buffer[fnp->f_boff], xfr_cnt);
    bp->b_flag |= BFR_DIRTY | BFR_VALID;

    ret_cnt += xfr_cnt;
    to_xfer -= xfr_cnt;
    fnp->f_offset += xfr_cnt;
    buffer = add_far((VOID FAR *) buffer, (ULONG) xfr_cnt);
    if (fnp->f_offset > fnp->f_highwater)
    {
      fnp->f_highwater = fnp->f_offset;
      fnp->f_dir.dir_size = fnp->f_highwater;
    }
  }
  *err = SUCCESS;
  return ret_cnt;
}


COUNT dos_read(COUNT fd, VOID FAR * buffer, UCOUNT count)
{
  COUNT err;
  UCOUNT xfr;

  xfr = readblock(fd, buffer, count, &err);
  return err != SUCCESS ? err : xfr;
}

COUNT FAR init_call_dos_read(COUNT fd, VOID FAR * buffer, UCOUNT count)
{
  return dos_read(fd, buffer, count);
}

#ifndef IPL
COUNT dos_write(COUNT fd, VOID FAR * buffer, UCOUNT count)
{
  REG struct f_node FAR *fnp;
  COUNT err,
    xfr;

  fnp = xlt_fd(fd);

  if (fnp == (struct f_node FAR *)0 || fnp->f_count <= 0)
  {
    return DE_INVLDHNDL;
  }

  if (fnp->f_offset > fnp->f_highwater)
  {
    ULONG lCount = fnp->f_offset - fnp->f_highwater;

    while (lCount > 0)
    {
      writeblock(fd, buffer,
                lCount > 512l ? 512 : (UCOUNT) lCount,
                &err);
      lCount -= 512;
    }
  }

  xfr = writeblock(fd, buffer, count, &err);
  return err != SUCCESS ? err : xfr;
}
#endif

LONG dos_lseek(COUNT fd, LONG foffset, COUNT origin)
{
  REG struct f_node FAR *fnp;


  fnp = xlt_fd(fd);


  if (fnp == (struct f_node FAR *)0 || fnp->f_count <= 0)
    return (LONG) DE_INVLDHNDL;


  switch (origin)
  {
    case 0:
      return fnp->f_offset = (ULONG) foffset;

    case 1:
      return fnp->f_offset += foffset;

    case 2:
      return fnp->f_offset = fnp->f_highwater + foffset;

    default:
      return (LONG) DE_INVLDFUNC;
  }
}

UWORD dos_free(struct dpb * dpbp)
{
  REG UWORD i;
  REG UWORD cnt = 0;
  UWORD max_cluster = ((ULONG) dpbp->dpb_size
                       * (ULONG) dpbp->dpb_clssize - dpbp->dpb_data + 1)
  / dpbp->dpb_clssize + 2;

  if (dpbp->dpb_nfreeclst != UNKNCLUSTER)
    return dpbp->dpb_nfreeclst;
  else
  {
    for (i = 2; i < max_cluster; i++)
    {
      if (next_cluster(dpbp, i) == 0)
        ++cnt;
    }
    dpbp->dpb_nfreeclst = cnt;
    return cnt;
  }
}

VOID dos_pwd(struct dpb * dpbp, BYTE FAR * s)
{
  fsncopy((BYTE FAR *) & dpbp->dpb_path[1], s, 64);
}

#ifndef IPL
COUNT dos_cd(struct dpb *dpbp, BYTE FAR * s)
{
  BYTE FAR *p;
  struct f_node FAR *fnp;

  truename(s, PriPathName);

  if ((fnp = dir_open((BYTE FAR *) PriPathName)) == NULL)
    return DE_PATHNOTFND;
  else
  {
    dir_close(fnp);
    scopy(&PriPathName[2], dpbp->dpb_path);
    return SUCCESS;
  }
}
#endif


struct f_node FAR *get_f_node(void)
{
  REG i;

  for (i = 0; i < NFILES; i++)
  {
    if (f_nodes[i].f_count == 0)
    {
      ++f_nodes[i].f_count;
      return &f_nodes[i];
    }
  }
  return (struct f_node FAR *)0;
}

VOID release_f_node(struct f_node FAR * fnp)
{
  if (fnp->f_count > 0)
    --fnp->f_count;
  else
    fnp->f_count = 0;
}

#ifndef IPL
VOID dos_setdta(BYTE FAR * newdta)
{
  dta = newdta;
}

COUNT dos_getfattr(BYTE FAR * name, UWORD FAR * attrp)
{
  struct f_node FAR *fnp;
  COUNT fd;

  if ((fd = dos_open(name, O_RDONLY)) < SUCCESS)
    return DE_FILENOTFND;

  if ((fnp = xlt_fd(fd)) == (struct f_node FAR *)0)
    return DE_TOOMANY;

  if (fnp->f_count <= 0)
    return DE_FILENOTFND;

  *attrp = fnp->f_dir.dir_attrib;
  dos_close(fd);
  return SUCCESS;
}

COUNT dos_setfattr(BYTE FAR * name, UWORD FAR * attrp)
{
  struct f_node FAR *fnp;
  COUNT fd;

  if ((fd = dos_open(name, O_RDONLY)) < SUCCESS)
    return DE_FILENOTFND;

  if ((fnp = xlt_fd(fd)) == (struct f_node FAR *)0)
    return DE_TOOMANY;

  if (fnp->f_count <= 0)
    return DE_FILENOTFND;

  if ((*attrp & (D_VOLID | D_DIR | 0xC0)) != 0)
  {
    return DE_ACCESS;
  }

  fnp->f_dir.dir_attrib &= (D_VOLID | D_DIR);  

  fnp->f_dir.dir_attrib |= *attrp;    
  fnp->f_flags.f_dmod = TRUE;
  dos_close(fd);
  return SUCCESS;
}
#endif

COUNT media_check(REG struct dpb * dpbp)
{
  bpb FAR *bpbp;
  ULONG size;
  REG COUNT i;

  FOREVER
  {
    MediaReqHdr.r_length = sizeof(request);
    MediaReqHdr.r_unit = dpbp->dpb_subunit;
    MediaReqHdr.r_command = C_MEDIACHK;
    MediaReqHdr.r_mcmdesc = dpbp->dpb_mdb;
    MediaReqHdr.r_status = 0;
    execrh((request FAR *) & MediaReqHdr, dpbp->dpb_device);
    if (!(MediaReqHdr.r_status & S_ERROR) && (MediaReqHdr.r_status & S_DONE))
      break;
    else
    {
    loop1:
      switch (block_error(&MediaReqHdr, dpbp->dpb_unit, dpbp->dpb_device))
      {
        case ABORT:
        case FAIL:
          return DE_INVLDDRV;

        case RETRY:
          continue;

        case CONTINUE:
          break;

        default:
          goto loop1;
      }
    }
  }

  switch (MediaReqHdr.r_mcretcode | dpbp->dpb_flags)
  {
    case M_NOT_CHANGED:
      return SUCCESS;

    case M_DONT_KNOW:
      flush_buffers(dpbp->dpb_unit);

    case M_CHANGED:
    default:
      setinvld(dpbp->dpb_unit);
      FOREVER
      {
        MediaReqHdr.r_length = sizeof(request);
        MediaReqHdr.r_unit = dpbp->dpb_subunit;
        MediaReqHdr.r_command = C_BLDBPB;
        MediaReqHdr.r_mcmdesc = dpbp->dpb_mdb;
        MediaReqHdr.r_status = 0;
        execrh((request FAR *) & MediaReqHdr, dpbp->dpb_device);
        if (!(MediaReqHdr.r_status & S_ERROR) && (MediaReqHdr.r_status & S_DONE))
          break;
        else
        {
        loop2:
          switch (block_error(&MediaReqHdr,
                              dpbp->dpb_unit, dpbp->dpb_device))
          {
            case ABORT:
            case FAIL:
              return DE_INVLDDRV;

            case RETRY:
              continue;

            case CONTINUE:
              break;

            default:
              goto loop2;
          }
        }
      }
      bpbp = MediaReqHdr.r_bpptr;
      dpbp->dpb_mdb = bpbp->bpb_mdesc;
      dpbp->dpb_secsize = bpbp->bpb_nbyte;
      dpbp->dpb_clssize = bpbp->bpb_nsector;
      dpbp->dpb_clsmask = bpbp->bpb_nsector - 1;
      dpbp->dpb_fatstrt = bpbp->bpb_nreserved;
      dpbp->dpb_fats = bpbp->bpb_nfat;
      dpbp->dpb_dirents = bpbp->bpb_ndirent;
      size = bpbp->bpb_nsize == 0 ?
          bpbp->bpb_huge :
          (ULONG) bpbp->bpb_nsize;
      dpbp->dpb_size = size / ((ULONG) bpbp->bpb_nsector);
      dpbp->dpb_fatsize = bpbp->bpb_nfsect;
      dpbp->dpb_dirstrt = dpbp->dpb_fatstrt
          + dpbp->dpb_fats * dpbp->dpb_fatsize;
      dpbp->dpb_data = dpbp->dpb_dirstrt
          + ((DIRENT_SIZE * dpbp->dpb_dirents
              + (dpbp->dpb_secsize - 1))
             / dpbp->dpb_secsize);
      dpbp->dpb_flags = 0;
      dpbp->dpb_next = (struct dpb FAR *)-1;
      dpbp->dpb_cluster = UNKNCLUSTER;
      dpbp->dpb_nfreeclst = UNKNCLUSTER;  
      for (i = 1, dpbp->dpb_shftcnt = 0;
           i < (sizeof(dpbp->dpb_shftcnt) * 8); 
           dpbp->dpb_shftcnt++, i <<= 1)
      {
        if (i >= bpbp->bpb_nsector)
          break;
      }
      return SUCCESS;
  }
}

struct f_node FAR *xlt_fd(COUNT fd)
{
  return fd >= NFILES ? (struct f_node FAR *)0 : &f_nodes[fd];
}

COUNT xlt_fnp(struct f_node FAR * fnp)
{
  return fnp - f_nodes;
}

struct dhdr FAR *select_unit(COUNT drive)
{
  return blk_devices[drive].dpb_device;
}
