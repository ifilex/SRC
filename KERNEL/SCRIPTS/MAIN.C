/****************************************************************/
/*                                                              */
/*                                                              */
/*                            DOS-C                             */
/*                                                              */
/*      Block cache functions and device driver interface       */
/*                                                              */
/*                      Copyright (c) 1995                      */
/*                      Pasquale J. Villani                     */
/*                      All Rights Reserved                     */
/*                                                              */
/* This file is part of DOS-C.                                  */
/*                                                              */
/* DOS-C is free software; you can redistribute it and/or       */
/* modify it under the terms of the GNU General Public License  */
/* as published by the Free Software Foundation; either version */
/* 2, or (at your option) any later version.                    */
/*                                                              */
/* DOS-C is distributed in the hope that it will be useful, but */
/* WITHOUT ANY WARRANTY; without even the implied warranty of   */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See    */
/* the GNU General Public License for more details.             */
/*                                                              */
/* You should have received a copy of the GNU General Public    */
/* License along with DOS-C; see the file COPYING.  If not,     */
/* write to the Free Software Foundation, 675 Mass Ave,         */
/* Cambridge, MA 02139, USA.                                    */
/*                                                              */
/****************************************************************/

#define STORE_BOOT_INFO

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <io.h>
#include <dos.h>
#include <ctype.h>
#include <mem.h>
#include "port.h"
#include "memoria.h"
#include "fat16.h"
#include "fat32.h"

BYTE *pgm = "Transfer System";

BOOL fl_reset(WORD);
COUNT fl_rd_status(WORD);
COUNT fl_read(WORD, WORD, WORD, WORD, WORD, BYTE FAR *);
COUNT fl_write(WORD, WORD, WORD, WORD, WORD, BYTE FAR *);
BOOL fl_verify(WORD, WORD, WORD, WORD, WORD, BYTE FAR *);
BOOL fl_format(WORD, BYTE FAR *);
VOID get_boot(COUNT);
VOID put_boot(COUNT);
BOOL check_space(COUNT, BYTE *);
COUNT ltop(WORD *, WORD *, WORD *, COUNT, COUNT, LONG, byteptr);
BOOL copy(COUNT, BYTE *);
BOOL DiskReset(COUNT);
COUNT DiskRead(WORD, WORD, WORD, WORD, WORD, BYTE FAR *);
COUNT DiskWrite(WORD, WORD, WORD, WORD, WORD, BYTE FAR *);


#ifdef NATIVE
#define getlong(vp, lp) (*(LONG *)(lp)=*(LONG *)(vp))
#define getword(vp, wp) (*(WORD *)(wp)=*(WORD *)(vp))
#define getbyte(vp, bp) (*(BYTE *)(bp)=*(BYTE *)(vp))
#define fgetlong(vp, lp) (*(LONG FAR *)(lp)=*(LONG FAR *)(vp))
#define fgetword(vp, wp) (*(WORD FAR *)(wp)=*(WORD FAR *)(vp))
#define fgetbyte(vp, bp) (*(BYTE FAR *)(bp)=*(BYTE FAR *)(vp))
#define fputlong(lp, vp) (*(LONG FAR *)(vp)=*(LONG FAR *)(lp))
#define fputword(wp, vp) (*(WORD FAR *)(vp)=*(WORD FAR *)(wp))
#define fputbyte(bp, vp) (*(BYTE FAR *)(vp)=*(BYTE FAR *)(bp))
#else
VOID getword(VOID *, WORD *);
VOID getbyte(VOID *, BYTE *);
VOID fgetlong(VOID FAR *, LONG FAR *);
VOID fgetword(VOID FAR *, WORD FAR *);
VOID fgetbyte(VOID FAR *, BYTE FAR *);
VOID fputlong(LONG FAR *, VOID FAR *);
VOID fputword(WORD FAR *, VOID FAR *);
VOID fputbyte(BYTE FAR *, VOID FAR *);
#endif

#define SEC_SIZE        512
#define NDEV            4
#define COPY_SIZE       32768
#define NRETRY          5

static struct media_info
{
  ULONG mi_size;               
  UWORD mi_heads;              
  UWORD mi_cyls;              
  UWORD mi_sectors;           
  ULONG mi_offset;             
};

union
{
  BYTE bytes[2 * SEC_SIZE];
  boot boot_sector;
}

  buffer;

static struct media_info miarray[NDEV] =
{
  {720l, 2, 40, 9, 0l},
  {720l, 2, 40, 9, 0l},
  {720l, 2, 40, 9, 0l},
  {720l, 2, 40, 9, 0l}
};

#define PARTOFF 0x1be
#define N_PART 4

static struct
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
} partition[N_PART];

struct bootsectortype
{
  UBYTE bsJump[3];
  char OemName[8];
  UWORD bsBytesPerSec;
  UBYTE bsSecPerClust;
  UWORD bsResSectors;
  UBYTE bsFATs;
  UWORD bsRootDirEnts;
  UWORD bsSectors;
  UBYTE bsMedia;
  UWORD bsFATsecs;
  UWORD bsSecPerTrack;
  UWORD bsHeads;
  ULONG bsHiddenSecs;
  ULONG bsHugeSectors;
  UBYTE bsDriveNumber;
  UBYTE bsReserved1;
  UBYTE bsBootSignature;
  ULONG bsVolumeID;
  char bsVolumeLabel[11];
  char bsFileSysType[8];
  char unused[2];
  UWORD sysRootDirSecs;        
  ULONG sysFatStart;          
  ULONG sysRootDirStart;      
  ULONG sysDataStart;        
};


static int DrvMap[4] =
{0, 1, 0x80, 0x81};

COUNT drive, active;
UBYTE newboot[SEC_SIZE], oldboot[SEC_SIZE];

#ifdef DEBUG
FILE *log;
#endif

#define SBSIZE          51
#define SBOFFSET        11

#define SIZEOF_PARTENT  16

#define FAT12           0x01
#define FAT16SMALL      0x04
#define EXTENDED        0x05
#define FAT16LARGE      0x06

#define N_RETRY         5

COUNT get_part(COUNT drive, COUNT idx)
{
  REG retry = N_RETRY;
  REG BYTE *p = (BYTE *) & buffer.bytes[PARTOFF + (idx * SIZEOF_PARTENT)];
  REG ret;
  BYTE packed_byte, pb1;

  do
  {
    ret = fl_read((WORD) DrvMap[drive], (WORD) 0, (WORD) 0, (WORD) 1, (WORD) 1, (byteptr) & buffer);
  }
  while (ret != 0 && --retry > 0);
  if (ret != 0)
    return FALSE;
  getbyte((VOID *) p, &partition[idx].peBootable);
  ++p;
  getbyte((VOID *) p, &partition[idx].peBeginHead);
  ++p;
  getbyte((VOID *) p, &packed_byte);
  partition[idx].peBeginSector = packed_byte & 0x3f;
  ++p;
  getbyte((VOID *) p, &pb1);
  ++p;
  partition[idx].peBeginCylinder = pb1 + ((0xc0 & packed_byte) << 2);
  getbyte((VOID *) p, &partition[idx].peFileSystem);
  ++p;
  getbyte((VOID *) p, &partition[idx].peEndHead);
  ++p;
  getbyte((VOID *) p, &packed_byte);
  partition[idx].peEndSector = packed_byte & 0x3f;
  ++p;
  getbyte((VOID *) p, &pb1);
  ++p;
  partition[idx].peEndCylinder = pb1 + ((0xc0 & packed_byte) << 2);
  getlong((VOID *) p, &partition[idx].peStartSector);
  p += sizeof(LONG);
  getlong((VOID *) p, &partition[idx].peSectors);
  return TRUE;
}

VOID main(COUNT argc, char **argv)
{
  if (argc != 2)
  {
    fprintf(stderr, "Copia los archivos de sistema y el int‚rprete de comandos al disco\nque especifique.\n\nSYS [unidad]\n \n      [unidad]  Especifica la unidad en la que los archivos se copiar n.\n", pgm);
    exit(1);
  }

  drive = *argv[1] - (islower(*argv[1]) ? 'a' : 'A');
  if (drive < 0 || drive >= NDEV)
  {
    fprintf(stderr, "%s: unidad fuera de rango\n", pgm);
    exit(1);
  }

  if (!DiskReset(drive))
  {
    fprintf(stderr, "%s: no es posible resetear el driver %c:",
            drive, 'A' + drive);
    exit(1);
  }

  get_boot(drive);

  if (!check_space(drive, oldboot))
  {
    fprintf(stderr, "%s: No hay espacio en el disco para transferir el sistema.\n", pgm);
    exit(1);
  }

#ifdef DEBUG
  if ((log = fopen("system.log", "w")) == NULL)
  {
    printf("No puedo crear el archivo log.\n");
    log = NULL;
  }
#endif

  printf("Transfiriendo sistema...\n");
  put_boot(drive);

//  printf("\nCopiando 2OS-KRNL");
  if (!copy(drive, "kernel.sys"))
  {
    fprintf(stderr, "\n%s: no se pudo copiar a \"KERNEL.SYS\"\n", pgm);
#ifdef DEBUG
    fclose(log);
#endif
    exit(1);
  }

//  printf("\nCopiando SH.EXE");
//  if (!copy(drive, "sh.exe"))
//  {
//    fprintf(stderr, "\n%s: no puedo copiar \"SH.EXE\"\n", pgm);
// #ifdef DEBUG
//    fclose(log);
// #endif
//    exit(1);
//  }
  printf("\nSistema transferido.\n");
#ifdef DEBUG
  fclose(log);
#endif
  exit(0);
}

#ifdef DEBUG
VOID dump_sector(unsigned char far * sec)
{
  if (log)
  {
    COUNT x, y;
    char c;

    for (x = 0; x < 32; x++)
    {
      fprintf(log, "%03X  ", x * 16);
      for (y = 0; y < 16; y++)
      {
        fprintf(log, "%02X ", sec[x * 16 + y]);
      }
      for (y = 0; y < 16; y++)
      {
        c = oldboot[x * 16 + y];
        if (isprint(c))
          fprintf(log, "%c", c);
        else
          fprintf(log, ".");
      }
      fprintf(log, "\n");
    }
    fprintf(log, "\n");
  }
}

#endif


VOID put_boot(COUNT drive)
{
  COUNT i, z;
  WORD head, track, sector, ret;
  WORD count;
  ULONG temp;
  struct bootsectortype *bs;

  if (drive >= 2)
  {
    head = partition[active].peBeginHead;
    sector = partition[active].peBeginSector;
    track = partition[active].peBeginCylinder;
  }
  else
  {
    head = 0;
    sector = 1;
    track = 0;
  }

  if ((i = DiskRead(DrvMap[drive], head, track, sector, 1, (BYTE far *) oldboot)) != 0)
  {
    fprintf(stderr, "%s: error de lectura (codigo = 0x%02x)\n", pgm, i & 0xff);
    exit(1);
  }

#ifdef DEBUG
  fprintf(log, "Sector BOOT antiguo:\n");
  dump_sector(oldboot);
#endif

  bs = (struct bootsectortype *) & oldboot;
  if ((bs->bsFileSysType[4] == '6') && (bs->bsBootSignature == 0x29))
  {
    memcpy(newboot, Mfat32, SEC_SIZE);  
    printf("FAT Tipo: FAT32x2\n");
#ifdef DEBUG
    fprintf(log, "FAT Tipo: FAT32x2\n");
#endif
  }
  else
  {
    memcpy(newboot, Mfat16, SEC_SIZE);  
    printf("FAT Tipo: FAT16\n");
#ifdef DEBUG
    fprintf(log, "FAT Tipo: FAT16\n");
#endif
  }

  memcpy(&newboot[SBOFFSET], &oldboot[SBOFFSET], SBSIZE);

  bs = (struct bootsectortype *) & newboot;
#ifdef STORE_BOOT_INFO
  bs->sysRootDirSecs = bs->bsRootDirEnts / 16;
#endif
#ifdef DEBUG
  fprintf(log, "root dir entradas= %u\n", bs->bsRootDirEnts);
  fprintf(log, "root dir sectores= %u\n", bs->sysRootDirSecs);
#endif

  temp = bs->bsHiddenSecs + bs->bsResSectors;
#ifdef STORE_BOOT_INFO
  bs->sysFatStart = temp;
#endif
#ifdef DEBUG
  fprintf(log, "El sector del FAT comienza en %lu = (%lu + %u)\n", temp,
          bs->bsHiddenSecs, bs->bsResSectors);
#endif

  temp = temp + bs->bsFATsecs * bs->bsFATs;
#ifdef STORE_BOOT_INFO
  bs->sysRootDirStart = temp;
#endif
#ifdef DEBUG
  fprintf(log, "El Root comienza en el sector %lu = (PREVIOUS + %u * %u)\n",
          temp, bs->bsFATsecs, bs->bsFATs);
#endif

  temp = temp + bs->sysRootDirSecs;
#ifdef STORE_BOOT_INFO
  bs->sysDataStart = temp;
#endif
#ifdef DEBUG
  fprintf(log, "DATA Comienza en el sector %lu = (PREVIOUS + %u)\n", temp,
          bs->sysRootDirSecs);
#endif


#ifdef DEBUG
  fprintf(log, "\nNuevo sector BOOT:\n");
  dump_sector(newboot);
#endif

  if ((i = DiskWrite(DrvMap[drive], head, track, sector, 1, (BYTE far *) newboot)) != 0)
  {
    fprintf(stderr, "%s: error de escritura en disco (code = 0x%02x)\n", pgm, i & 0xff);
    exit(1);
  }
}

VOID get_boot(COUNT drive)
{
  COUNT i;
  COUNT ifd;
  WORD head, track, sector, ret;
  WORD count;

  if (drive >= 2)
  {
    head = partition[active].peBeginHead;
    sector = partition[active].peBeginSector;
    track = partition[active].peBeginCylinder;
  }
  else
  {
    head = 0;
    sector = 1;
    track = 0;
  }

  if ((i = DiskRead(DrvMap[drive], head, track, sector, 1, (BYTE far *) oldboot)) != 0)
  {
    fprintf(stderr, "%s: error de lectura de disco (codigo = 0x%02x)\n", pgm, i & 0xff);
    exit(1);
  }
}

BOOL check_space(COUNT drive, BYTE * BlkBuffer)
{
  BYTE *bpbp;
  BYTE nfat;
  UWORD nfsect;
  ULONG hidden;
  ULONG count;
  ULONG block;
  UBYTE nreserved;
  UCOUNT i;
  WORD track, head, sector;
  UBYTE buffer[SEC_SIZE];
  ULONG bpb_huge;
  UWORD bpb_nsize;

  getbyte((VOID *) & BlkBuffer[BT_BPB + BPB_NFAT], &nfat);
  getword((VOID *) & BlkBuffer[BT_BPB + BPB_NFSECT], &nfsect);
  getlong((VOID *) & BlkBuffer[BT_BPB + BPB_HIDDEN], &hidden);
  getbyte((VOID *) & BlkBuffer[BT_BPB + BPB_NRESERVED], &nreserved);

  getlong((VOID *) & BlkBuffer[BT_BPB + BPB_HUGE], &bpb_huge);
  getword((VOID *) & BlkBuffer[BT_BPB + BPB_NSIZE], &bpb_nsize);

  count = miarray[drive].mi_size = bpb_nsize == 0 ?
      bpb_huge : bpb_nsize;

  getword((&(((BYTE *) & BlkBuffer[BT_BPB])[BPB_NHEADS])), &miarray[drive].mi_heads);
  head = miarray[drive].mi_heads;
  getword((&(((BYTE *) & BlkBuffer[BT_BPB])[BPB_NSECS])), &miarray[drive].mi_sectors);
  if (miarray[drive].mi_size == 0)
    getlong(&((((BYTE *) & BlkBuffer[BT_BPB])[BPB_HUGE])), &miarray[drive].mi_size);
  sector = miarray[drive].mi_sectors;
  if (head == 0 || sector == 0)
  {
    fprintf(stderr, "Error de inicializacion del driver.\n");
    exit(1);
  }
  miarray[drive].mi_cyls = count / (head * sector);

  return 1;
}

static COUNT ltop(trackp, sectorp, headp, unit, count, strt_sect, strt_addr)
WORD *trackp, *sectorp, *headp;
REG COUNT unit;
LONG strt_sect;
COUNT count;
byteptr strt_addr;
{
#ifdef I86
  ULONG ltemp;
#endif
  REG ls, ps;

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
  if (((ls = *headp * miarray[unit].mi_sectors + *sectorp - 1) + count) >
      (ps = miarray[unit].mi_heads * miarray[unit].mi_sectors))
    count = ps - ls;
  return count;
}

BOOL copy(COUNT drive, BYTE * file)
{
  BYTE dest[64];
  COUNT ifd, ofd, ret;
  FILE *s, *d;
  BYTE buffer[COPY_SIZE];
  struct ftime ftime;

  sprintf(dest, "%c:\\%s", 'A' + drive, file);
  if ((s = fopen(file, "rb")) == NULL)
  {
    fprintf(stderr, "%s: \"%s\" no funciona\n", pgm, file);
    return FALSE;
  }
  _fmode = O_BINARY;
  if ((d = fopen(dest, "wb")) == NULL)
  {
    fclose(s);
    fprintf(stderr, "%s: no puede ser creado\"%s\"\n", pgm, dest);
    return FALSE;
  }

  while ((ret = fread(buffer, 1, COPY_SIZE, s)) != 0)
    fwrite(buffer, 1, ret, d);

  getftime(fileno(s), &ftime);
  setftime(fileno(d), &ftime);

  fclose(s);
  fclose(d);

  return TRUE;
}

BOOL DiskReset(COUNT Drive)
{
  REG COUNT idx;

  fl_reset(DrvMap[drive]);

  if (Drive >= 2 && Drive < NDEV)
  {
    COUNT RetCode;

    for (RetCode = TRUE, idx = 0; RetCode && (idx < N_PART); idx++)
      RetCode = get_part(Drive, idx);
    if (!RetCode)
      return FALSE;

    for (idx = 0; idx < N_PART; idx++)
    {
      if (partition[idx].peFileSystem == FAT12
          || partition[idx].peFileSystem == FAT16SMALL
          || partition[idx].peFileSystem == FAT16LARGE)
      {
        miarray[Drive].mi_offset
            = partition[idx].peStartSector;
        active = idx;
        break;
      }
    }
  }

  return TRUE;
}

COUNT DiskRead(WORD drive, WORD head, WORD track, WORD sector, WORD count, BYTE FAR * buffer)
{
  int nRetriesLeft;

  for (nRetriesLeft = NRETRY; nRetriesLeft > 0; --nRetriesLeft)
  {
    if (fl_read(drive, head, track, sector, count, buffer) == count)
      return count;
  }
  return 0;
}

COUNT DiskWrite(WORD drive, WORD head, WORD track, WORD sector, WORD count, BYTE FAR * buffer)
{
  int nRetriesLeft;

  for (nRetriesLeft = NRETRY; nRetriesLeft > 0; --nRetriesLeft)
  {
    if (fl_write(drive, head, track, sector, count, buffer) == count)
      return count;
  }
  return 0;
}
