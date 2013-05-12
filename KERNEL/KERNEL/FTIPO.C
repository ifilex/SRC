#include "port.h"
#include "todo.h"

#ifdef PROTO
UCOUNT link_fat12(struct dpb *, UCOUNT, UCOUNT);
UCOUNT link_fat16(struct dpb *, UCOUNT, UCOUNT);
UWORD next_cl12(struct dpb *, UCOUNT);
UWORD next_cl16(struct dpb *, UCOUNT);
#else
UCOUNT link_fat12();
UCOUNT link_fat16();
UWORD next_cl12();
UWORD next_cl16();
#endif

UCOUNT link_fat(struct dpb *dpbp, UCOUNT Cluster1, REG UCOUNT Cluster2)
{
  if (ISFAT12(dpbp))
    return link_fat12(dpbp, Cluster1, Cluster2);
  else if (ISFAT16(dpbp))
    return link_fat16(dpbp, Cluster1, Cluster2);
  else
    return DE_BLKINVLD;
}

UCOUNT link_fat16(struct dpb * dpbp, UCOUNT Cluster1, UCOUNT Cluster2)
{
  UCOUNT idx;
  struct buffer FAR *bp;
  UWORD Cl2 = Cluster2;

  bp = getblock((ULONG) (((ULONG) Cluster1) * SIZEOF_CLST16) / dpbp->dpb_secsize + dpbp->dpb_fatstrt,
                dpbp->dpb_unit);
#ifdef DISPLAY_GETBLOCK
  printf("ftipo.c: FAT (link_fat16)\n"); // fat 16
#endif
  if (bp == NULL)
    return DE_BLKINVLD;
  bp->b_flag &= ~(BFR_DATA | BFR_DIR);
  bp->b_flag |= BFR_FAT;
  bp->b_copies = dpbp->dpb_fats;
  bp->b_offset_lo = dpbp->dpb_fatsize;
  bp->b_offset_hi = dpbp->dpb_fatsize >> 8;

  idx = (((LONG) Cluster1) * SIZEOF_CLST16) % dpbp->dpb_secsize;

  fputword((WORD FAR *) & Cl2, (VOID FAR *) & (bp->b_buffer[idx]));
  bp->b_flag |= BFR_DIRTY;

  if (Cluster2 == FREE)
  {
    if (dpbp->dpb_nfreeclst != UNKNCLUSTER)
      ++dpbp->dpb_nfreeclst;
  }
  else
  {
    if (dpbp->dpb_nfreeclst != UNKNCLUSTER)
      --dpbp->dpb_nfreeclst;
  }

  return SUCCESS;
}

UCOUNT link_fat12(struct dpb * dpbp, UCOUNT Cluster1, UCOUNT Cluster2)
{
  REG UBYTE FAR *fbp0,
    FAR * fbp1;
  UCOUNT idx;
  struct buffer FAR *bp,
    FAR * bp1;

  bp = getblock((ULONG) ((((Cluster1 << 1) + Cluster1) >> 1) / dpbp->dpb_secsize + dpbp->dpb_fatstrt),
                dpbp->dpb_unit);
#ifdef DISPLAY_GETBLOCK
  printf("ftipo.c: FAT (link_fat12)\n"); // fat 12
#endif
  if (bp == NULL)
    return DE_BLKINVLD;
  bp->b_flag &= ~(BFR_DATA | BFR_DIR);
  bp->b_flag |= BFR_FAT;
  bp->b_copies = dpbp->dpb_fats;
  bp->b_offset_lo = dpbp->dpb_fatsize;
  bp->b_offset_hi = dpbp->dpb_fatsize >> 8;

  idx = (((Cluster1 << 1) + Cluster1) >> 1) % dpbp->dpb_secsize;

  if (idx >= dpbp->dpb_secsize - 1)
  {
    bp1 = getblock((ULONG) (dpbp->dpb_fatstrt +
               ((((Cluster1 << 1) + Cluster1) >> 1) / dpbp->dpb_secsize))
                   + 1,
                   dpbp->dpb_unit);
#ifdef DISPLAY_GETBLOCK
    printf("ftipo.c: FAT (link_fat12)\n");
#endif
    if (bp1 == (struct buffer *)0)
      return DE_BLKINVLD;
    bp1->b_flag &= ~(BFR_DATA | BFR_DIR);
    bp1->b_flag |= BFR_FAT | BFR_DIRTY;
    bp1->b_copies = dpbp->dpb_fats;
    bp1->b_offset_lo = dpbp->dpb_fatsize;
    bp1->b_offset_hi = dpbp->dpb_fatsize >> 8;
    fbp1 = (UBYTE FAR *) & (bp1->b_buffer[0]);
  }
  else
    fbp1 = (UBYTE FAR *) & (bp->b_buffer[idx + 1]);
  fbp0 = (UBYTE FAR *) & (bp->b_buffer[idx]);
  bp->b_flag |= BFR_DIRTY;

  if (Cluster1 & 0x01)
  {
    *fbp0 = (*fbp0 & 0x0f) | ((Cluster2 & 0x0f) << 4);
    *fbp1 = (Cluster2 >> 4) & 0xff;
  }
  else
  {
    *fbp0 = Cluster2 & 0xff;
    *fbp1 = (*fbp1 & 0xf0) | (Cluster2 >> 8) & 0x0f;
  }

  if (Cluster2 == FREE)
  {
    if (dpbp->dpb_nfreeclst != UNKNCLUSTER)
      ++dpbp->dpb_nfreeclst;
  }
  else
  {
    if (dpbp->dpb_nfreeclst != UNKNCLUSTER)
      --dpbp->dpb_nfreeclst;
  }

  return SUCCESS;
}

UWORD next_cluster(struct dpb * dpbp, REG UCOUNT ClusterNum)
{
  if (ISFAT12(dpbp))
    return next_cl12(dpbp, ClusterNum);
  else if (ISFAT16(dpbp))
    return next_cl16(dpbp, ClusterNum);
  else
    return LONG_LAST_CLUSTER;
}

UWORD next_cl16(struct dpb * dpbp, REG UCOUNT ClusterNum)
{
  UCOUNT idx;
  struct buffer FAR *bp;
  UWORD RetCluster;

  bp = getblock((ULONG) (((ULONG) ClusterNum) * SIZEOF_CLST16) / dpbp->dpb_secsize + dpbp->dpb_fatstrt,
                dpbp->dpb_unit);
#ifdef DISPLAY_GETBLOCK
  printf("ftipo.c: FAT (next_cl16)\n");
#endif
  if (bp == NULL)
    return DE_BLKINVLD;
  bp->b_flag &= ~(BFR_DATA | BFR_DIR);
  bp->b_flag |= BFR_FAT;
  bp->b_copies = dpbp->dpb_fats;
  bp->b_offset_lo = dpbp->dpb_fatsize;
  bp->b_offset_hi = dpbp->dpb_fatsize >> 8;

  idx = (((LONG) ClusterNum) * SIZEOF_CLST16) % dpbp->dpb_secsize;

  fgetword((VOID FAR *) & (bp->b_buffer[idx]), (WORD FAR *) & RetCluster);

  return RetCluster;
}

UWORD next_cl12(struct dpb * dpbp, REG UCOUNT ClusterNum)
{
  REG UBYTE FAR *fbp0,
    FAR * fbp1;
  UCOUNT idx;
  struct buffer FAR *bp,
    FAR * bp1;

  bp = getblock((ULONG) ((((ClusterNum << 1) + ClusterNum) >> 1) / dpbp->dpb_secsize + dpbp->dpb_fatstrt),
                dpbp->dpb_unit);
#ifdef DISPLAY_GETBLOCK
  printf("ftipo.c: FAT (next_cl12)\n"); // clusters
#endif
  if (bp == NULL)
    return LONG_BAD;
  bp->b_flag &= ~(BFR_DATA | BFR_DIR);
  bp->b_flag |= BFR_FAT;
  bp->b_copies = dpbp->dpb_fats;
  bp->b_offset_lo = dpbp->dpb_fatsize;
  bp->b_offset_hi = dpbp->dpb_fatsize >> 8;

  idx = (((ClusterNum << 1) + ClusterNum) >> 1) % dpbp->dpb_secsize;

  if (idx >= dpbp->dpb_secsize - 1)
  {
    bp1 = getblock((ULONG) (dpbp->dpb_fatstrt +
           ((((ClusterNum << 1) + ClusterNum) >> 1) / dpbp->dpb_secsize))
                   + 1,
                   dpbp->dpb_unit);
#ifdef DISPLAY_GETBLOCK
    printf("ftipo.c: FAT (next_cl12)\n");
#endif
    if (bp1 == (struct buffer *)0)
      return LONG_BAD;
    bp1->b_flag &= ~(BFR_DATA | BFR_DIR);
    bp1->b_flag |= BFR_FAT;
    bp1->b_copies = dpbp->dpb_fats;
    bp1->b_offset_lo = dpbp->dpb_fatsize;
    bp1->b_offset_hi = dpbp->dpb_fatsize >> 8;
    fbp1 = (UBYTE FAR *) & (bp1->b_buffer[0]);
  }
  else
    fbp1 = (UBYTE FAR *) & (bp->b_buffer[idx + 1]);
  fbp0 = (UBYTE FAR *) & (bp->b_buffer[idx]);

  if (ClusterNum & 0x01)
    ClusterNum = ((*fbp0 & 0xf0) >> 4) | *fbp1 << 4;
  else
    ClusterNum = *fbp0 | ((*fbp1 & 0x0f) << 8);

  if ((ClusterNum & MASK) == MASK)
    ClusterNum = LONG_LAST_CLUSTER;
  else if ((ClusterNum & BAD) == BAD)
    ClusterNum = LONG_BAD;
  return ClusterNum;
}
