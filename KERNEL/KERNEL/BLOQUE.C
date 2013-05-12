#include "port.h"
#include "todo.h"

VOID FAR init_buffers(void)
{
  REG WORD i;
  REG WORD count;

  for (i = 0; i < Config.cfgBuffers; ++i)
  {
    buffers[i].b_unit = 0;
    buffers[i].b_flag = 0;
    buffers[i].b_blkno = 0;
    buffers[i].b_copies = 0;
    buffers[i].b_offset_lo = 0;
    buffers[i].b_offset_hi = 0;
    if (i < (Config.cfgBuffers - 1))
      buffers[i].b_next = &buffers[i + 1];
    else
      buffers[i].b_next = NULL;
  }
  firstbuf = &buffers[0];
  lastbuf = &buffers[Config.cfgBuffers - 1];
}

ULONG getblkno(struct buffer FAR *bp)
{
  if (bp->b_blkno == 0xffffu)
    return bp->b_huge_blkno;
  else
    return bp->b_blkno;
}

VOID setblkno(struct buffer FAR *bp, ULONG blkno)
{
  if (blkno >= 0xffffu)
  {
    bp->b_blkno = 0xffffu;
    bp->b_huge_blkno = blkno;
  }
  else
  {
    bp->b_blkno = blkno;
    bp->b_dpbp = &blk_devices[bp->b_unit];
  }
}

struct buffer FAR *getblock(ULONG blkno, COUNT dsk)
{
  REG struct buffer FAR *bp;
  REG struct buffer FAR *lbp;
  REG struct buffer FAR *mbp;
  REG BYTE fat_count=0;


#ifdef DISPLAY_GETBLOCK
  printf("[getblock %d, blk %ld, buf ",dsk,blkno);
#endif
  bp = firstbuf;
  lbp = NULL;
  mbp = NULL;
  while (bp != NULL)
  {
    if ((bp->b_flag & BFR_VALID) && (bp->b_unit == dsk)
        && (getblkno(bp) == blkno))
    {
      if (lbp != NULL)
      {
        lbp->b_next = bp->b_next;
        bp->b_next = firstbuf;
        firstbuf = bp;
      }
#ifdef DISPLAY_GETBLOCK
      printf("HIT]\n");
#endif
      return (bp);
    }
    else
    {
      if (bp->b_flag & BFR_FAT) fat_count++;
      mbp = lbp;               
      lbp = bp;
      bp = bp->b_next;
    }
  }
#ifdef DISPLAY_GETBLOCK
  printf("MISS]\n");
#endif


  if ((lbp->b_flag & BFR_FAT) && (fat_count < 3))
  {
    bp = firstbuf;
    lbp = NULL;
    mbp = NULL;
    while ((bp != NULL) && (bp->b_flag & BFR_FAT))
    {
      mbp = lbp;                
      lbp = bp;
      bp = bp->b_next;
    }

    if (bp == NULL)
    {
      if (mbp != NULL)
        mbp->b_next = NULL;
      lbp->b_next = firstbuf;
      firstbuf = bp = lbp;
    }
    else if (lbp != NULL)
    {
      lbp->b_next = bp->b_next;
      bp->b_next = firstbuf;
      firstbuf = bp;
    }
    lbp = bp;
  }
  else
  {
    if (mbp != NULL)
      mbp->b_next = NULL;
    lbp->b_next = firstbuf;
    firstbuf = lbp;
  }

  if (flush1(lbp) && fill(lbp, blkno, dsk)) 
    mbp = lbp;
  else
    mbp = NULL;               
  return (mbp);
}

BOOL getbuf(struct buffer FAR ** pbp, ULONG blkno, COUNT dsk)
{
  REG struct buffer FAR *bp;
  REG struct buffer FAR *lbp;
  REG struct buffer FAR *mbp;
  REG BYTE fat_count=0;


#ifdef DISPLAY_GETBLOCK
  printf("[getbuf %d, blk %ld, buf ",dsk,blkno);
#endif
  bp = firstbuf;
  lbp = NULL;
  mbp = NULL;
  while (bp != NULL)
  {
    if ((bp->b_flag & BFR_VALID) && (bp->b_unit == dsk)
        && (getblkno(bp) == blkno))
    {
      if (lbp != NULL)
      {
        lbp->b_next = bp->b_next;
        bp->b_next = firstbuf;
        firstbuf = bp;
      }
      *pbp = bp;
#ifdef DISPLAY_GETBLOCK
      printf("HIT]\n");
#endif
      return TRUE;
    }
    else
    {
      if (bp->b_flag & BFR_FAT) fat_count++;
      mbp = lbp;               
      lbp = bp;
      bp = bp->b_next;
    }
  }

#ifdef DISPLAY_GETBLOCK
  printf("MISS]\n");
#endif

  if ((lbp->b_flag & BFR_FAT) && (fat_count < 3))
  {
    bp = firstbuf;
    lbp = NULL;
    mbp = NULL;
    while ((bp != NULL) && (bp->b_flag & BFR_FAT))
    {
      mbp = lbp;                
      lbp = bp;
      bp = bp->b_next;
    }

    if (bp == NULL)
    {
      if (mbp != NULL)
        mbp->b_next = NULL;
      lbp->b_next = firstbuf;
      firstbuf = bp = lbp;
    }
    else if (lbp != NULL)
    {
      lbp->b_next = bp->b_next;
      bp->b_next = firstbuf;
      firstbuf = bp;
    }
    lbp = bp;
  }
  else
  {
    if (mbp != NULL)
      mbp->b_next = NULL;
    lbp->b_next = firstbuf;
    firstbuf = lbp;
  }

  if (flush1(lbp))             
  {
    lbp->b_flag = 0;
    lbp->b_unit = dsk;
    setblkno(lbp, blkno);
    *pbp = lbp;
    return TRUE;
  }
  else
  {
    *pbp = NULL;
    return FALSE;
  }
}


VOID setinvld(REG COUNT dsk)
{
  REG struct buffer FAR *bp;

  bp = firstbuf;
  while (bp)
  {
    if (bp->b_unit == dsk)
      bp->b_flag = 0;
    bp = bp->b_next;
  }
}

BOOL flush_buffers(REG COUNT dsk)
{
  REG struct buffer FAR *bp;
  REG BOOL ok = TRUE;

  bp = firstbuf;
  while (bp)
  {
    if (bp->b_unit == dsk)
      if (!flush1(bp))
        ok = FALSE;
    bp = bp->b_next;
  }
  return ok;
}

BOOL flush1(struct buffer FAR * bp)
{
  REG WORD ok;

  if ((bp->b_flag & BFR_VALID) && (bp->b_flag & BFR_DIRTY))
  {
    ok = dskxfer(bp->b_unit, getblkno(bp),
                 (VOID FAR *) bp->b_buffer, 1, DSKWRITE);
    if (bp->b_flag & BFR_FAT)
    {
      int i = bp->b_copies;
      LONG blkno = getblkno(bp);
      UWORD offset = ((UWORD)bp->b_offset_hi << 8) | bp->b_offset_lo;

      while (--i > 0)
      {
	blkno += offset;
        ok &= dskxfer(bp->b_unit, blkno,
                      (VOID FAR *) bp->b_buffer, 1, DSKWRITE);
      }
    }
  }
  else
    ok = TRUE;
  bp->b_flag &= ~BFR_DIRTY;     
  if (!ok)                    
    bp->b_flag &= ~BFR_VALID;   
  return (ok);
}

BOOL flush(void)
{
  REG struct buffer FAR *bp;
  REG BOOL ok;

  ok = TRUE;
  bp = firstbuf;
  while (bp)
  {
    if (!flush1(bp))
      ok = FALSE;
    bp->b_flag &= ~BFR_VALID;
    bp = bp->b_next;
  }
  return (ok);
}

BOOL fill(REG struct buffer FAR * bp, ULONG blkno, COUNT dsk)
{
  REG WORD ok;

  ok = dskxfer(dsk, blkno, (VOID FAR *) bp->b_buffer, 1, DSKREAD);
  bp->b_flag = BFR_VALID | BFR_DATA;
  bp->b_unit = dsk;
  setblkno(bp, blkno);
  return (ok);
}

BOOL dskxfer(COUNT dsk, ULONG blkno, VOID FAR * buf, UWORD numblocks, COUNT mode)
{
  REG struct dpb *dpbp = &blk_devices[dsk];

  for (;;)
  {
    IoReqHdr.r_length = sizeof(request);
    IoReqHdr.r_unit = dpbp->dpb_subunit;
    IoReqHdr.r_command =
        mode == DSKWRITE ?
        (verify_ena ? C_OUTVFY : C_OUTPUT)
        : C_INPUT;
    IoReqHdr.r_status = 0;
    IoReqHdr.r_meddesc = dpbp->dpb_mdb;
    IoReqHdr.r_trans = (BYTE FAR *) buf;
    IoReqHdr.r_count = numblocks;
    if (blkno >= MAXSHORT)
    {
      IoReqHdr.r_start = HUGECOUNT;
      IoReqHdr.r_huge = blkno;
    }
    else
      IoReqHdr.r_start = blkno;
    execrh((request FAR *) & IoReqHdr, dpbp->dpb_device);
    if (!(IoReqHdr.r_status & S_ERROR) && (IoReqHdr.r_status & S_DONE))
      break;
    else
    {
    loop:
      switch (block_error(&IoReqHdr, dpbp->dpb_unit, dpbp->dpb_device))
      {
        case ABORT:
        case FAIL:
          return FALSE;

        case RETRY:
          continue;

        case CONTINUE:
          break;

        default:
          goto loop;
      }
    }
  }
  return TRUE;
}
