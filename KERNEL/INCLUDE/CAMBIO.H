#define FID_CHARDEV     0x80    
#define FID_NOWRITE     0x40    
#define FID_MASK        0x3f  
#define FID_EOF         0x40    
#define FID_BINARY      0x20  
#define FID_CLOCK       0x08   
#define FID_NULL        0x04  
#define FID_CONOUT      0x02   
#define FID_CONIN       0x01   
#ifndef FNAME_SIZE
#define FNAME_SIZE      8       
#endif
#ifndef FEXT_SIZE
#define FEXT_SIZE       3     
#endif
#ifndef FDFLT_DRIVE
#define FDFLT_DRIVE     0       
#endif
#define PARSE_SEP_STOP          0x01
#define PARSE_DFLT_DRIVE        0x02
#define PARSE_BLNK_FNAME        0x04
#define PARSE_BLNK_FEXT         0x08
#define PARSE_RET_NOWILD        0
#define PARSE_RET_WILD          1
#define PARSE_RET_BADDRIVE      0xff
#define FCB_READ  0
#define FCB_WRITE 1
typedef struct
{
  BYTE fcb_drive;               
  BYTE fcb_fname[FNAME_SIZE];  
  BYTE fcb_fext[FEXT_SIZE];     
  UWORD fcb_cublock;          
  UWORD fcb_recsiz;             
  ULONG fcb_fsize;             
  date fcb_date;               
  time fcb_time;              
  BYTE fcb_sftno;             
  BYTE fcb_attrib_hi;         
  BYTE fcb_attrib_lo;          
  UWORD fcb_strtclst;      
  UWORD fcb_dirclst;         
  UBYTE fcb_diroff;          
  UBYTE fcb_curec;             
  ULONG fcb_rndm;              
}
fcb;

typedef struct
{
  UBYTE xfcb_flag;             
  BYTE xfcb_resvrd[5];         
  UBYTE xfcb_attrib;           
  fcb xfcb_fcb;
}
xfcb;

typedef struct
{
  UBYTE renDriveID;            
  BYTE renOldName[8];      
  BYTE renOldExtent[3];       
  BYTE renReserved1[5];
  BYTE renNewName[8];         
  BYTE renNewExtent[3];        
  BYTE renReserved2[9];
}
rfcb;
