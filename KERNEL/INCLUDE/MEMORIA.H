#define S_ERROR         0x8000  
#define S_BUSY          0x0200  
#define S_DONE          0x0100  
#define S_MASK          0x00ff 
#define MD_2SIDE        1       
#define MD_8SECTOR      2    
#define MD_REMOVABLE    4      
#define M_CHANGED       -1      
#define M_DONT_KNOW     0      
#define M_NOT_CHANGED   1      
#define E_WRPRT         0  
#define E_UNIT          1     
#define E_NOTRDY        2       
#define E_CMD           3       
#define E_CRC           4     
#define E_LENGTH        5     
#define E_SEEK          6      
#define E_MEDIA         7       
#define E_NOTFND        8      
#define E_PAPER         9       
#define E_WRITE         10     
#define E_READ          11      
#define E_FAILURE       12      
#define C_INIT          0x00    
#define C_MEDIACHK      0x01    
#define C_BLDBPB        0x02    
#define C_IOCTLIN       0x03   
#define C_INPUT         0x04  
#define C_NDREAD        0x05   
#define C_ISTAT         0x06    
#define C_IFLUSH        0x07    
#define C_OUTPUT        0x08    
#define C_OUTVFY        0x09    
#define C_OSTAT         0x0a    
#define C_OFLUSH        0x0b    
#define C_IOCTLOUT      0x0c   
#define C_OPEN          0x0d   
#define C_CLOSE         0x0e    
#define C_REMMEDIA      0x0f    
#define C_OUB           0x10   
#define C_GENIOCTL      0x13   
#define C_GETLDEV       0x17   
#define C_SETLDEV       0x18    
#define C_IOCTLQRY      0x19   
#define failure(x)      (S_ERROR+S_DONE+x)
#ifndef TRUE
#define TRUE            1
#endif
#ifndef FALSE
#define FALSE           0
#endif
#define mk_offset(far_ptr)      ((UWORD)(far_ptr))
#define mk_segment(far_ptr) ((UWORD)((ULONG)(far_ptr) >> 16))
#define far_ptr(seg, off) ((VOID FAR *)(((ULONG)(off))+((ULONG)(seg) << 16)))

struct dhdr
{
  struct dhdr
  FAR *dh_next;
  UWORD dh_attr;
    VOID(*dh_strategy) ();
    VOID(*dh_interrupt) ();
  BYTE dh_name[8];
};

#define ATTR_CHAR       0x8000
#define ATTR_IOCTL      0x4000
#define ATTR_EXCALLS    0x0800
#define ATTR_QRYIOCTL   0x0080
#define ATTR_GENIOCTL   0x0040
#define ATTR_RAW        0x0400
#define ATTR_FASTCON    0x0010
#define ATTR_CLOCK      0x0008
#define ATTR_NULL       0x0004
#define ATTR_CONOUT     0x0002
#define ATTR_CONIN      0x0001
#define BPB_NBYTE       0
#define BPB_NSECTOR     2
#define BPB_NRESERVED   3
#define BPB_NFAT        5
#define BPB_NDIRENT     6
#define BPB_NSIZE       8
#define BPB_MDESC       10
#define BPB_NFSECT      11
#define BPB_NSECS       13
#define BPB_NHEADS      15
#define BPB_HIDDEN      17
#define BPB_HUGE        21
#define BPB_SIZEOF      25

typedef struct
{
  UWORD bpb_nbyte;             
  UBYTE bpb_nsector;            
  UWORD bpb_nreserved;          
  UBYTE bpb_nfat;              
  UWORD bpb_ndirent;            
  UWORD bpb_nsize;        
  UBYTE bpb_mdesc;          
  UWORD bpb_nfsect;         
  UWORD bpb_nsecs;              
  UWORD bpb_nheads;            
  ULONG bpb_hidden;             
  ULONG bpb_huge;           
}
bpb;
#define BT_JUMP         0
#define BT_OEM          3
#define BT_BPB          11
#define BT_SIZEOF       36

typedef struct
{
  BYTE bt_jump[3];              
  BYTE bt_oem[8];               
  bpb bt_bpb;                   
  WORD bt_nsecs;                
  WORD bt_nheads;             
  WORD bt_hidden;              
  LONG bt_huge;                
}
boot;

typedef boot super;             

typedef struct
{
  BYTE r_length;                
  BYTE r_unit;                
  BYTE r_command;             
  WORD r_status;               
  BYTE r_reserved[8];          
  union
  {
    struct
    {
      BYTE _r_nunits;          
      BYTE FAR *_r_endaddr;     
      bpb *FAR * _r_bpbptr;     
      BYTE _r_firstunit;
    }
    _r_init;
    struct
    {
      BYTE _r_meddesc;     
      BYTE _r_retcode;         
      BYTE FAR
      * _r_vid;            
    }
    _r_media;
    struct
    {
      BYTE _r_meddesc;          
      boot FAR
      * _r_fat;               
      bpb FAR
      * _r_bpbpt;              
    }
    _r_bpb;
    struct
    {
      BYTE _r_meddesc;          
      BYTE FAR
      * _r_trans;            
      UWORD _r_count;         
      UWORD _r_start;       
      BYTE FAR
      * _r_vid;              
      LONG _r_huge;            
    }
    _r_rw;
    struct
    {
      BYTE _r_ndbyte;          
    }
    _r_nd;
  }
  _r_x;
}
request;

#define HUGECOUNT       0xffff
#define MAXSHORT        0xffffl
#define r_nunits        _r_x._r_init._r_nunits
#define r_endaddr       _r_x._r_init._r_endaddr
#define r_bpbptr        _r_x._r_init._r_bpbptr
#define r_firstunit     _r_x._r_init._r_firstunit
#define r_mcmdesc       _r_x._r_media._r_meddesc
#define r_mcretcode     _r_x._r_media._r_retcode
#define r_mcvid         _r_x._r_media._r_vid
#define r_bpmdesc       _r_x._r_bpb._r_meddesc
#define r_bpfat         _r_x._r_bpb._r_fat
#define r_bpptr         _r_x._r_bpb._r_bpbpt
#define r_meddesc       _r_x._r_rw._r_meddesc
#define r_trans         _r_x._r_rw._r_trans
#define r_count         _r_x._r_rw._r_count
#define r_start         _r_x._r_rw._r_start
#define r_rwvid         _r_x._r_rw._r_vid
#define r_huge          _r_x._r_rw._r_huge
#define r_ndbyte        _r_x._r_nd._r_ndbyte
#define I_NONE          0        
#define IRQ0             0x01     
#define IRQ1            0x02
#define IRQ2            0x04
#define IRQ3            0x08
#define IRQ4            0x10
#define IRQ5            0x20
#define IRQ6            0x40
#define IRQ7            0x80     
#define I_RTC           IRQ0     
#define I_KBD           IRQ1     
#define I_COM2          IRQ3     
#define I_COM1          IRQ4     
#define I_HDC           IRQ5     
#define I_FDC           IRQ6     
#define I_PRT           IRQ7    
#define V_RTC           0x08     
#define V_KBD           0x09     
#define V_LEV2          0x0a     
#define V_COM2          0x0b     
#define V_COM1          0x0c     
#define V_HDC           0x0d     
#define V_FDC           0x0e     
#define V_PRT           0x0f    
#define V_LEV0          0x08     
#define V_LEV1          0x09
#define V_LEV2          0x0a     
#define V_LEV3          0x0b
#define V_LEV4          0x0c
#define V_LEV5          0x0d
#define V_LEV6          0x0e
#define V_LEV7          0x0f     
typedef request FAR *rqptr;
typedef bpb FAR *bpbptr;
typedef BYTE FAR *byteptr;
typedef struct dhdr FAR *dhdrptr;
extern BYTE FAR *device_end();
