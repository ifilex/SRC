#include "memoria.h"
#include "emubin.h"
#include "binary.h"
#include "fecha.h"
#include "tiempo.h"
#include "mfat.h"
#include "cambio.h"
#include "lineas.h"
#include "proceso.h"
#include "dosmem.h"
#include "emusft.h"
#include "raiz.h"
#include "exe.h"
#include "nodos.h"
#include "dir.h"
#include "archivo.h"
#include "reloj.h"
#include "teclado.h"
#include "error.h"
#include "ver.h"


#ifdef MAIN
#define GLOBAL
#else
#define GLOBAL extern
#endif

#ifndef TRUE
#define TRUE (1)
#endif
#ifndef FALSE
#define FALSE (0)
#endif

#define PARSE_MAX       67      
#define NFILES          16     
#define NFCBS           16     
#define NDEVS           8      
#define NSTACKS         8       
#define NLAST           2      
#define NAMEMAX         PARSE_MAX       
#define NUMBUFF         6      
                                       

#define STDIN           0
#define STDOUT          1
#define STDERR          2
#define STDAUX          3
#define STDPRN          4

#define ERROR           -1
#define OK              0

#define XFR_READ        1
#define XFR_WRITE       2

#define RDONLY          0
#define WRONLY          1
#define RDWR            2

#define SPCL            0x00
#define CTL_C           0x03
#define CTL_F           0x06
#define BELL            0x07
#define BS              0x08
#define HT              0x09
#define LF              0x0a
#define CR              0x0d
#define CTL_Q           0x11
#define CTL_S           0x13
#define CTL_Z           0x1a
#define ESC             0x1b
#define CTL_BS          0x7f

#define F3              0x3d
#define LEFT            0x4b
#define RIGHT           0x4d

#define DSKWRITE        1       
#define DSKREAD         2

#define FREE                    0x000

#define LONG_LAST_CLUSTER       0xFFFF
#define LONG_MASK               0xFFF8
#define LONG_BAD                0xFFF0
#define LAST_CLUSTER            0x0FFF
#define MASK                    0xFF8
#define BAD                     0xFF0

#ifdef LINESIZE
#undef LINESIZE
#endif
#define LINESIZE 256

#define BUFFERSIZE 512
struct buffer
{
  struct buffer
  FAR *b_next;                 
  BYTE b_unit;                
  BYTE b_flag;                
  UWORD b_blkno;            
                             
  BYTE b_copies;             
  UBYTE b_offset_lo;          
  union {
    struct dpb FAR *_b_dpbp;   
    LONG _b_huge_blkno;        
  } _b;
  UBYTE b_offset_hi;        
  UBYTE b_unused;
  BYTE b_buffer[BUFFERSIZE];  
};

#define b_dpbp		_b._b_dpbp
#define b_huge_blkno	_b._b_huge_blkno

#define BFR_DIRTY       0x40   
#define BFR_VALID       0x20   
#define BFR_DATA        0x08   
#define BFR_DIR         0x04   
#define BFR_FAT         0x02   
#define BFR_BOOT        0x01   

typedef BYTE *UPMAP;

extern struct dhdr
FAR clk_dev,                    
  FAR con_dev,          
  FAR prn_dev,               
  FAR aux_dev,               
  FAR blk_dev;                
extern UWORD
  ram_top,                  
#ifdef I86

  api_sp,                     
#endif

  api_ss,                 
  usr_sp,                     
  usr_ss;
extern COUNT *
#ifdef MC68K
  api_sp,             
#endif

  error_tos,               
  disk_api_tos,             
  char_api_tos;             
extern BYTE
  FAR last;                  
extern struct ClockRecord
  ClkRecord;

GLOBAL
seg master_env;               

GLOBAL BYTE
  os_major,                    
  os_minor,                   

  rev_number                    
#ifdef MAIN
= REV_NUMBER,
#else
 ,
#endif

  version_flags;              
#ifdef DEBUG
GLOBAL WORD bDumpRegs
#ifdef MAIN
= FALSE;
#else
 ;
#endif
GLOBAL WORD bDumpRdWrParms
#ifdef MAIN
= FALSE;
#else
 ;
#endif
#endif

GLOBAL BYTE *copyright
#if 0
="\n";
#else
 ;
#endif

GLOBAL BYTE *os_release
#ifdef MAIN
# if 0
="\n";
# else
= "Kernel %d.%d.%d \n\n";
# endif
#else
 ;
#endif


extern UWORD NetBios;
extern BYTE * net_name;
extern BYTE net_set_count;

extern UWORD
  first_mcb;                 
extern struct dpb
FAR *DPBp;                      
extern sfttbl
  FAR * sfthead;         
extern struct dhdr
FAR *clock,                  
  FAR * syscon;            
extern WORD
  maxbksize;                  
extern struct buffer
FAR *firstbuf;              
extern VOID
  FAR * CDSp;                 
extern sfttbl
  FAR * FCBp;               
extern WORD
  nprotfcb;                  
extern BYTE
  nblkdev,                  
  lastdrive;                 
extern struct dhdr
  nul_dev;
extern BYTE
  LocalPath[PARSE_MAX + 3],     
  mem_access_mode,            
  ErrorMode,                  
  InDOS,                     
  njoined;                    

extern struct dirent
  SearchDir;

extern struct
{
  COUNT nDrive;
  BYTE szName[FNAME_SIZE + 1];
  BYTE szExt[FEXT_SIZE + 1];
}
FcbSearchBuffer;

extern union              
{
  BYTE _PriPathName[128];
  struct
  {
    BYTE _dname[NAMEMAX];
    BYTE _fname[FNAME_SIZE];
    BYTE _fext[FEXT_SIZE];
  }
  _f;
}
_PriPathBuffer;
#define PriPathName _PriPathBuffer._PriPathName
#define szDirName _PriPathBuffer._f._dname
#define szFileName _PriPathBuffer._f._fname
#define szFileExt _PriPathBuffer._f._fext
#define szPriDirName _PriPathBuffer._f._dname
#define szPriFileName _PriPathBuffer._f._fname
#define szPriFileExt _PriPathBuffer._f._fext

extern union               
{
  BYTE _SecPathName[128];
  struct
  {
    BYTE _dname[NAMEMAX];
    BYTE _fname[FNAME_SIZE];
    BYTE _fext[FEXT_SIZE];
  }
  _f;
}
_SecPathBuffer;
#define SecPathName _SecPathBuffer._SecPathName
#define szSecDirName _SecPathBuffer._f._dname
#define szSecFileName _SecPathBuffer._f._fname
#define szSecFileExt _SecPathBuffer._f._fext

extern UWORD
  wAttr;

extern BYTE
  default_drive;             

extern BYTE
  TempBuffer[],               
  FAR internal_data[],          
  FAR swap_always[],            
  FAR swap_indos[],          
  tsr,                   
  break_flg,                  
  break_ena,                  
  FAR * dta;                  
extern seg
  cu_psp;                     
extern iregs
  FAR * user_r;               

extern struct dirent          
  DirEntBuffer;

extern request                
  CharReqHdr,
  IoReqHdr,
  MediaReqHdr;

extern fcb
  FAR * lpFcb;                  

extern sfttbl
  FAR * lpCurSft;

extern BYTE
  verify_ena,            
  switchar,                 
  return_mode,             
  return_code;              

extern BYTE
  scr_pos;                   
extern WORD
  BootDrive,                  
  NumFloppies;              

extern keyboard
  kb_buf;

extern struct cds
  TempCDS;

GLOBAL seg
  RootPsp;                    

GLOBAL struct f_node
 *pDirFileNode;

GLOBAL iregs error_regs;       

GLOBAL WORD
  dump_regs;                  

GLOBAL struct dpb
  blk_devices[NDEVS];

GLOBAL struct f_node FAR
* f_nodes;                    

GLOBAL struct buffer
FAR *lastbuf,                  
  FAR * buffers;               

GLOBAL BYTE                    
  FAR * dma_scratch;            

GLOBAL iregs
  FAR * ustackp,              
  FAR * kstackp;        

GLOBAL struct config
{
  UBYTE cfgBuffers;           
  UBYTE cfgFiles;             
  UBYTE cfgFcbs;               
  UBYTE cfgProtFcbs;         
  BYTE cfgInit[NAMEMAX];      
  BYTE cfgInitTail[NAMEMAX];   
  BYTE cfgLastdrive;            
  BYTE cfgStacks;             
  UWORD cfgStackSize;           
}
Config
#ifdef MAIN
=
{
  NUMBUFF,
      NFILES,
      NFCBS,
      0,
      "BIN\\SH.EXE",   
      "/P \r\n",
      NLAST,
      NSTACKS,
      128
};
#else
;
#endif

#include "init.h"

VOID
FAR cpm_entry(VOID),
INRPT FAR re_entry(VOID)/*,
INRPT FAR handle_break(VOID)*/;  
VOID
enable(VOID),
disable(VOID);
COUNT
CriticalError(
    COUNT nFlag, COUNT nDrive, COUNT nError, struct dhdr FAR * lpDevice);

#ifdef PROTO
VOID FAR CharMapSrvc(VOID);
VOID FAR set_stack(VOID);
VOID FAR restore_stack(VOID);
#ifndef IN_INIT_MOD
VOID execrh(request FAR *, struct dhdr FAR *);
#endif
VOID FAR init_call_execrh(request FAR *, struct dhdr FAR *);
VOID exit(COUNT);
VOID tmark(VOID);
BOOL tdelay(LONG);
BYTE FAR *device_end(VOID);
COUNT kb_data(VOID);
COUNT kb_input(VOID);
COUNT kb_init(VOID);
VOID setvec(UWORD, VOID(INRPT FAR *) ());
BYTE FAR *getvec(UWORD);
COUNT con(COUNT);
VOID getdirent(BYTE FAR *, struct dirent FAR *);
VOID putdirent(struct dirent FAR *, BYTE FAR *);
#else
VOID FAR CharMapSrvc();
VOID FAR set_stack();
VOID FAR restore_stack();
VOID execrh();
VOID exit();
VOID tmark();
BOOL tdelay();
BYTE FAR *device_end();
COUNT kb_data();
COUNT kb_input();
COUNT kb_init();
VOID setvec();
BYTE FAR *getvec();
COUNT con();
VOID getdirent();
VOID putdirent();
#endif

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
#ifdef PROTO
VOID getword(VOID *, WORD *);
VOID getbyte(VOID *, BYTE *);
VOID fgetlong(VOID FAR *, LONG FAR *);
VOID fgetword(VOID FAR *, WORD FAR *);
VOID fgetbyte(VOID FAR *, BYTE FAR *);
VOID fputlong(LONG FAR *, VOID FAR *);
VOID fputword(WORD FAR *, VOID FAR *);
VOID fputbyte(BYTE FAR *, VOID FAR *);
#else
VOID getword();
VOID getbyte();
VOID fgetlong();
VOID fgetword();
VOID fgetbyte();
VOID fputlong();
VOID fputword();
VOID fputbyte();
#endif
#endif

#ifdef I86
#define setvec(n, isr)	(void)(*(VOID (INRPT FAR * FAR *)())(4 * (n)) = (isr))
#endif
#define is_leap_year(y)	((y) & 3 ? 0 : (y) % 100 ? 1 : (y) % 400 ? 1 : 0)

void spawn_int23(void);                
int control_break(void);              
void handle_break(void);              

