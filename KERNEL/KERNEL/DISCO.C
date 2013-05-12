#include "port.h"
#include "todo.h"

#ifdef PROTO
BOOL fl_reset(WORD);
COUNT fl_readdasd(WORD);
COUNT fl_diskchanged(WORD);
COUNT fl_rd_status(WORD);
COUNT fl_read(WORD, WORD, WORD, WORD, WORD, BYTE FAR *);
COUNT fl_write(WORD, WORD, WORD, WORD, WORD, BYTE FAR *);
COUNT fl_verify(WORD, WORD, WORD, WORD, WORD, BYTE FAR *);
BOOL fl_format(WORD, BYTE FAR *);
#else
BOOL fl_reset();
COUNT fl_readdasd();
COUNT fl_diskchanged();
COUNT fl_rd_status();
COUNT fl_read();
COUNT fl_write();
COUNT fl_verify();
BOOL fl_format();
#endif

#define NDEV            8       
#define SEC_SIZE        512     
#define N_RETRY         5      
#define NENTRY          25     

union
{
  BYTE bytes[2 * SEC_SIZE];
  boot boot_sector;
}
buffer;

static struct media_info
{
  ULONG mi_size;               
  UWORD mi_heads;               
  UWORD mi_cyls;          
  UWORD mi_sectors;         
  ULONG mi_offset;          
  BYTE mi_drive;             
  COUNT mi_partidx;            
};

static struct media_info miarray[NDEV]; 
static bpb bpbarray[NDEV];     
static bpb *bpbptrs[NDEV];      

#define N_PART 4                
                            

static WORD head,
  track,
  sector,
  ret;                          
static WORD count;
static COUNT nUnits;         
static COUNT nPartitions;    

#define PARTOFF 0x1be

static struct
{
  BYTE peDrive;               
  BYTE peBootable;
  BYTE peBeginHead;
  BYTE peBeginSector;
  UWORD peBeginCylinder;
  BYTE peFileSystem;
  BYTE peEndHead;
  BYTE peEndSector;
  UWORD peEndCylinder;
  LONG peStartSector;
  LONG peSectors;
  LONG peAbsStart;             
}
dos_partition[NDEV - 2];

#ifdef PROTO
WORD init(rqptr),
  mediachk(rqptr),
  bldbpb(rqptr),
  blockio(rqptr),
  blk_error(rqptr);
COUNT ltop(WORD *, WORD *, WORD *, COUNT, COUNT, LONG, byteptr);
WORD dskerr(COUNT);
COUNT processtable(COUNT ptDrive, BYTE ptHead, UWORD ptCylinder, BYTE ptSector, LONG ptAccuOff);
#else
WORD init(),
  mediachk(),
  bldbpb(),
  blockio(),
  blk_error();
WORD dskerr();
COUNT processtable();
#endif


#ifdef PROTO
static WORD(*dispatch[NENTRY]) (rqptr) =
#else
static WORD(*dispatch[NENTRY]) () =
#endif
{
  init,                  
  mediachk,                
  bldbpb,                  
  blk_error,               
  blockio,                 
  blk_error,                 
  blk_error,              
  blk_error,               
  blockio,              
  blockio,                 
  blk_error,                 
  blk_error,                  
  blk_error,        
  blk_error,              
  blk_error,                
  blk_error,                  
  blk_error,               
  blk_error,               
  blk_error,                 
  blk_error,                 
  blk_error,                  
  blk_error,                 
  blk_error,                   
  blk_error,                
  blk_error                    
};

#define SIZEOF_PARTENT  16

#define FAT12           0x01
#define FAT16SMALL      0x04
#define EXTENDED        0x05
#define FAT16LARGE      0x06

#define hd(x)   ((x) & 0x80)

COUNT processtable(COUNT ptDrive, BYTE ptHead, UWORD ptCylinder,
             BYTE ptSector, LONG ptAccuOff)
{
  struct                       
  {
    BYTE peBootable;
    BYTE peBeginHead;
    BYTE peBeginSector;
    UWORD peBeginCylinder;
    BYTE peFileSystem;
    BYTE peEndHead;
    BYTE peEndSector;
    UWORD peEndCylinder;
    LONG peStartSector;
    LONG peSectors;
  }
  temp_part[N_PART];

  REG retry = N_RETRY;
  UBYTE packed_byte,
    pb1;
  COUNT Part;

  do
  {
    ret = fl_read((WORD) ptDrive, (WORD) ptHead, (WORD) ptCylinder,
                  (WORD) ptSector, (WORD) 1, (byteptr) & buffer);
  }
  while (ret != 0 && --retry > 0);
  if (ret != 0)
    return FALSE;

  for (Part = 0; Part < N_PART; Part++)
  {
    REG BYTE *p =
    (BYTE *) & buffer.bytes[PARTOFF + (Part * SIZEOF_PARTENT)];

    getbyte((VOID *) p, &temp_part[Part].peBootable);
    ++p;
    getbyte((VOID *) p, &temp_part[Part].peBeginHead);
    ++p;
    getbyte((VOID *) p, &packed_byte);
    temp_part[Part].peBeginSector = packed_byte & 0x3f;
    ++p;
    getbyte((VOID *) p, &pb1);
    ++p;
    temp_part[Part].peBeginCylinder = pb1 + ((UWORD) (0xc0 & packed_byte) << 2);
    getbyte((VOID *) p, &temp_part[Part].peFileSystem);
    ++p;
    getbyte((VOID *) p, &temp_part[Part].peEndHead);
    ++p;
    getbyte((VOID *) p, &packed_byte);
    temp_part[Part].peEndSector = packed_byte & 0x3f;
    ++p;
    getbyte((VOID *) p, &pb1);
    ++p;
    temp_part[Part].peEndCylinder = pb1 + ((UWORD) (0xc0 & packed_byte) << 2);
    getlong((VOID *) p, &temp_part[Part].peStartSector);
    p += sizeof(LONG);
    getlong((VOID *) p, &temp_part[Part].peSectors);
  };

  for (Part = 0; Part < N_PART && nUnits < NDEV; Part++)
  {
    if (temp_part[Part].peFileSystem == FAT12 ||
        temp_part[Part].peFileSystem == FAT16SMALL ||
        temp_part[Part].peFileSystem == FAT16LARGE)
    {
      miarray[nUnits].mi_offset =
          temp_part[Part].peStartSector + ptAccuOff;
      miarray[nUnits].mi_drive = ptDrive;
      miarray[nUnits].mi_partidx = nPartitions;
      nUnits++;

      dos_partition[nPartitions].peDrive = ptDrive;
      dos_partition[nPartitions].peBootable =
          temp_part[Part].peBootable;
      dos_partition[nPartitions].peBeginHead =
          temp_part[Part].peBeginHead;
      dos_partition[nPartitions].peBeginSector =
          temp_part[Part].peBeginSector;
      dos_partition[nPartitions].peBeginCylinder =
          temp_part[Part].peBeginCylinder;
      dos_partition[nPartitions].peFileSystem =
          temp_part[Part].peFileSystem;
      dos_partition[nPartitions].peEndHead =
          temp_part[Part].peEndHead;
      dos_partition[nPartitions].peEndSector =
          temp_part[Part].peEndSector;
      dos_partition[nPartitions].peEndCylinder =
          temp_part[Part].peEndCylinder;
      dos_partition[nPartitions].peStartSector =
          temp_part[Part].peStartSector;
      dos_partition[nPartitions].peSectors =
          temp_part[Part].peSectors;
      dos_partition[nPartitions].peAbsStart =
          temp_part[Part].peStartSector + ptAccuOff;
      nPartitions++;
    }
    else if (temp_part[Part].peFileSystem == EXTENDED)
    {
      processtable(ptDrive,
                   temp_part[Part].peBeginHead,
                   temp_part[Part].peBeginCylinder,
                   temp_part[Part].peBeginSector,
                   temp_part[Part].peStartSector + ptAccuOff);
    };
  };
  return TRUE;
}

COUNT blk_driver(rqptr rp)
{
  if (rp->r_unit >= nUnits && rp->r_command != C_INIT)
    return failure(E_UNIT);
  if (rp->r_command > NENTRY)
  {
    return failure(E_FAILURE);  
  }
  else
    return ((*dispatch[rp->r_command]) (rp));
}

static WORD init(rqptr rp)
{
  extern COUNT fl_nrdrives(VOID);
  COUNT HardDrive,
    nHardDisk,
    Unit;

  fl_reset(0x80);

  nUnits = 2;
  nPartitions = 0;

  for (Unit = 0; Unit < NDEV; Unit++)
  {
    miarray[Unit].mi_size = 720l;
    miarray[Unit].mi_heads = 2;
    miarray[Unit].mi_cyls = 40;
    miarray[Unit].mi_sectors = 9;
    miarray[Unit].mi_offset = 0l;
    miarray[Unit].mi_drive = Unit;

    bpbarray[Unit].bpb_nbyte = SEC_SIZE;
    bpbarray[Unit].bpb_nsector = 2;
    bpbarray[Unit].bpb_nreserved = 1;
    bpbarray[Unit].bpb_nfat = 2;
    bpbarray[Unit].bpb_ndirent = 112;
    bpbarray[Unit].bpb_nsize = 720l;
    bpbarray[Unit].bpb_mdesc = 0xfd;
    bpbarray[Unit].bpb_nfsect = 2;

    bpbptrs[Unit] = &bpbarray[Unit];
  };

  nHardDisk = fl_nrdrives();
  for (HardDrive = 0; HardDrive < nHardDisk; HardDrive++)
  {
    if (!processtable((HardDrive | 0x80), 0, 0l, 1, 0l))
      break;
  };

  rp->r_nunits = nUnits;
  rp->r_bpbptr = bpbptrs;
  rp->r_endaddr = device_end();
  return S_DONE;
}

static WORD mediachk(rqptr rp)
{
  COUNT drive = miarray[rp->r_unit].mi_drive;
  COUNT result;

  if (hd(drive))
    rp->r_mcretcode = M_NOT_CHANGED;
  else  
  {
    if ((result = fl_readdasd(drive)) == 2) 
    {
      if ((result = fl_diskchanged(drive)) == 1) 
        rp->r_mcretcode = M_CHANGED;
      else if (result == 0)
        rp->r_mcretcode = M_NOT_CHANGED;
      else
        rp->r_mcretcode = tdelay((LONG) 37) ? M_DONT_KNOW : M_NOT_CHANGED;
    }
    else if (result == 3)  
      rp->r_mcretcode = M_NOT_CHANGED;
    else 
      rp->r_mcretcode = tdelay((LONG) 37) ? M_DONT_KNOW : M_NOT_CHANGED;
  }

  return S_DONE;
}

static WORD bldbpb(rqptr rp)
{
  REG retry = N_RETRY;
  ULONG count;
  byteptr trans;
  WORD local_word;

  if (hd(miarray[rp->r_unit].mi_drive))
  {
    COUNT partidx = miarray[rp->r_unit].mi_partidx;
    head = dos_partition[partidx].peBeginHead;
    sector = dos_partition[partidx].peBeginSector;
    track = dos_partition[partidx].peBeginCylinder;
  }
  else
  {
    head = 0;
    sector = 1;
    track = 0;
  }

  do
  {
    ret = fl_read((WORD) miarray[rp->r_unit].mi_drive,
                  (WORD) head, (WORD) track, (WORD) sector, (WORD) 1, (byteptr) & buffer);
  }
  while (ret != 0 && --retry > 0);
  if (ret != 0)
    return (dskerr(ret));

  getword(&((((BYTE *) & buffer.bytes[BT_BPB]))[BPB_NBYTE]), &bpbarray[rp->r_unit].bpb_nbyte);
  getbyte(&((((BYTE *) & buffer.bytes[BT_BPB]))[BPB_NSECTOR]), &bpbarray[rp->r_unit].bpb_nsector);
  getword(&((((BYTE *) & buffer.bytes[BT_BPB]))[BPB_NRESERVED]), &bpbarray[rp->r_unit].bpb_nreserved);
  getbyte(&((((BYTE *) & buffer.bytes[BT_BPB]))[BPB_NFAT]), &bpbarray[rp->r_unit].bpb_nfat);
  getword(&((((BYTE *) & buffer.bytes[BT_BPB]))[BPB_NDIRENT]), &bpbarray[rp->r_unit].bpb_ndirent);
  getword(&((((BYTE *) & buffer.bytes[BT_BPB]))[BPB_NSIZE]), &bpbarray[rp->r_unit].bpb_nsize);
  getword(&((((BYTE *) & buffer.bytes[BT_BPB]))[BPB_NSIZE]), &bpbarray[rp->r_unit].bpb_nsize);
  getbyte(&((((BYTE *) & buffer.bytes[BT_BPB]))[BPB_MDESC]), &bpbarray[rp->r_unit].bpb_mdesc);
  getword(&((((BYTE *) & buffer.bytes[BT_BPB]))[BPB_NFSECT]), &bpbarray[rp->r_unit].bpb_nfsect);
  getword(&((((BYTE *) & buffer.bytes[BT_BPB]))[BPB_NSECS]), &bpbarray[rp->r_unit].bpb_nsecs);
  getword(&((((BYTE *) & buffer.bytes[BT_BPB]))[BPB_NHEADS]), &bpbarray[rp->r_unit].bpb_nheads);
  getlong(&((((BYTE *) & buffer.bytes[BT_BPB])[BPB_HIDDEN])), &bpbarray[rp->r_unit].bpb_hidden);
  getlong(&((((BYTE *) & buffer.bytes[BT_BPB])[BPB_HUGE])), &bpbarray[rp->r_unit].bpb_huge);
#ifdef DSK_DEBUG
  printf("BPB_NBYTE     = %04x\n", bpbarray[rp->r_unit].bpb_nbyte);
  printf("BPB_NSECTOR   = %02x\n", bpbarray[rp->r_unit].bpb_nsector);
  printf("BPB_NRESERVED = %04x\n", bpbarray[rp->r_unit].bpb_nreserved);
  printf("BPB_NFAT      = %02x\n", bpbarray[rp->r_unit].bpb_nfat);
  printf("BPB_NDIRENT   = %04x\n", bpbarray[rp->r_unit].bpb_ndirent);
  printf("BPB_NSIZE     = %04x\n", bpbarray[rp->r_unit].bpb_nsize);
  printf("BPB_MDESC     = %02x\n", bpbarray[rp->r_unit].bpb_mdesc);
  printf("BPB_NFSECT    = %04x\n", bpbarray[rp->r_unit].bpb_nfsect);
#endif
  rp->r_bpptr = &bpbarray[rp->r_unit];
  count = miarray[rp->r_unit].mi_size =
      bpbarray[rp->r_unit].bpb_nsize == 0 ?
      bpbarray[rp->r_unit].bpb_huge :
      bpbarray[rp->r_unit].bpb_nsize;
  getword((&(((BYTE *) & buffer.bytes[BT_BPB])[BPB_NHEADS])), &miarray[rp->r_unit].mi_heads);
  head = miarray[rp->r_unit].mi_heads;
  getword((&(((BYTE *) & buffer.bytes[BT_BPB])[BPB_NSECS])), &miarray[rp->r_unit].mi_sectors);
  if (miarray[rp->r_unit].mi_size == 0)
    getlong(&((((BYTE *) & buffer.bytes[BT_BPB])[BPB_HUGE])), &miarray[rp->r_unit].mi_size);
  sector = miarray[rp->r_unit].mi_sectors;
  if (head == 0 || sector == 0)
  {
    tmark();
    return failure(E_FAILURE);
  }
  miarray[rp->r_unit].mi_cyls = count / (head * sector);
  tmark();
#ifdef DSK_DEBUG
  printf("BPB_NSECS     = %04x\n", sector);
  printf("BPB_NHEADS    = %04x\n", head);
  printf("BPB_HIDDEN    = %08lx\n", bpbarray[rp->r_unit].bpb_hidden);
  printf("BPB_HUGE      = %08lx\n", bpbarray[rp->r_unit].bpb_huge);
#endif
  return S_DONE;
}

static COUNT write_and_verify(WORD drive, WORD head, WORD track, WORD sector,
WORD count, BYTE FAR *buffer)
{
  REG COUNT ret;

  ret = fl_write(drive, head, track, sector, count, buffer);
  if (ret != 0)
    return ret;
  return fl_verify(drive, head, track, sector, count, buffer);
}

static WORD blockio(rqptr rp)
{
  REG retry = N_RETRY,
    remaining;
  UWORD cmd,
    total;
  ULONG start;
  byteptr trans;
  COUNT (*action)(WORD, WORD, WORD, WORD, WORD, BYTE FAR *);

  cmd = rp->r_command;
  total = 0;
  trans = rp->r_trans;
  tmark();
  for (
        remaining = rp->r_count,
        start = (rp->r_start != HUGECOUNT ? rp->r_start : rp->r_huge)
        + miarray[rp->r_unit].mi_offset;
        remaining > 0;
        remaining -= count, trans += count * SEC_SIZE, start += count
      )
  {
    count = ltop(&track, &sector, &head, rp->r_unit, remaining, start, trans);
    do
    {
      switch (cmd)
      {
        case C_INPUT:
	  action = fl_read;
	  break;
        case C_OUTPUT:
	  action = fl_write;
	  break;
	case C_OUTVFY:
	  action = write_and_verify;
          break;
        default:
          return failure(E_FAILURE);
      }
      if (count)
	ret = action((WORD)miarray[rp->r_unit].mi_drive, head, track, sector,
		count, trans);
      else
      {
	count = 1;
	if (cmd != C_INPUT)
	  fbcopy(trans, dma_scratch, SEC_SIZE);
	ret = action((WORD)miarray[rp->r_unit].mi_drive, head, track, sector,
		1, dma_scratch);
	if (cmd == C_INPUT)
	  fbcopy(dma_scratch, trans, SEC_SIZE);
      }
      if (ret != 0)
	fl_reset((WORD)miarray[rp->r_unit].mi_drive);
    }
    while (ret != 0 && --retry > 0);
    if (ret != 0)
    {
      rp->r_count = total;
      return dskerr(ret);
    }
    total += count;
  }
  rp->r_count = total;
  return S_DONE;
}

static WORD blk_error(rqptr rp)
{
  rp->r_count = 0;
  return failure(E_FAILURE);    
}

static WORD dskerr(COUNT code)
{
  switch (code & 0x03)
  {
    case 1:                
      if (code & 0x08)
        return (E_FAILURE);
      else
        return failure(E_CMD);

    case 2:                   
      return failure(E_FAILURE);

    case 3:                   
      return failure(E_WRPRT);

    default:
      if (code & 0x80)      
        return failure(E_NOTRDY);
      else if (code & 0x40)    
        return failure(E_SEEK);
      else if (code & 0x10)    
        return failure(E_CRC);
      else if (code & 0x04)
        return failure(E_NOTFND);
      else
        return failure(E_FAILURE);
  }
}

static COUNT ltop(WORD * trackp, WORD * sectorp, WORD * headp, REG COUNT unit, COUNT count, LONG strt_sect, byteptr strt_addr)
{
#ifdef I86
  ULONG ltemp;
#endif
  REG ls,
    ps;

#ifdef I86
  ltemp = (((ULONG) mk_segment(strt_addr) << 4) + mk_offset(strt_addr)) & 0xffff;
  count = (((ltemp + SEC_SIZE * count) & 0xffff0000l) != 0l)
      ? (0xffffl - ltemp) / SEC_SIZE
      : count;
#endif

  *trackp = strt_sect / (miarray[unit].mi_heads * miarray[unit].mi_sectors);
  *sectorp = strt_sect % miarray[unit].mi_sectors + 1;
  *headp = (strt_sect % (miarray[unit].mi_sectors * miarray[unit].mi_heads))
      / miarray[unit].mi_sectors;
  if (*sectorp + count > miarray[unit].mi_sectors + 1)
    count = miarray[unit].mi_sectors + 1 - *sectorp;
  return count;
}
