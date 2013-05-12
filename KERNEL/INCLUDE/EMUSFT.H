#define SFTMAX  128
typedef struct
{
  WORD sft_count;               
  WORD sft_mode;               
  BYTE sft_attrib;             
  union {
    WORD _sft_flags;        
    struct {
      BYTE _sft_flags_lo;
      BYTE _sft_flags_hi;
    } _split_sft_flags;
  } sft_flags_union;
  WORD sft_flags;              
  union
  {
    struct dpb FAR *
      _sft_dcb;                
    struct dhdr FAR *
      _sft_dev;                
  }
  sft_dcb_or_dev;
  WORD sft_stclust;             
  time sft_time;           
  date sft_date;             
  LONG sft_size;              
  LONG sft_posit;               
  WORD sft_relclust;           
  WORD sft_cuclust;            
  WORD sft_dirdlust;           
  BYTE sft_diridx;             
  BYTE sft_name[11]; 
  BYTE FAR *
    sft_bshare;              
  WORD sft_mach;            
  WORD sft_psp;               
  WORD sft_shroff;            
  WORD sft_status;             
}
sft;
typedef struct _sftheader
{
  struct _sfttbl FAR *         
    sftt_next;
  WORD sftt_count;             
}
sftheader;

typedef struct _sfttbl
{
  struct _sfttbl FAR *         
    sftt_next;
  WORD sftt_count;             
  sft sftt_table[SFTMAX];      
}
sfttbl;
#define SFT_MASK        0x0060  
#define SFT_MFCB        0x8000  
#define SFT_MDENYNONE   0x0040  
#define SFT_MDENYREAD   0x0030  
#define SFT_MDENYWRITE  0x0020  
#define SFT_MEXCLUSIVE  0x0010  
#define SFT_NOINHERIT   0x0080  
#define SFT_NETFCB      0x0070  
#define SFT_MSHAREMASK  0x0070 
#define SFT_MRDWR       0x0002  
#define SFT_MWRITE      0x0001  
#define SFT_MREAD       0x0000 
#define SFT_OMASK       0x00f3 
#define SFT_FSHARED     0x8000  
#define SFT_FDEVICE     0x0080  
#define SFT_FDATE       0x4000  
#define SFT_FDIRTY      0x0040  
#define SFT_FDMASK      0x003f  
#define SFT_FIOCTL      0x4000  
#define SFT_FEOF        0x0040  
#define SFT_FBINARY     0x0020 
#define SFT_FSPECIAL    0x0010  
#define SFT_FCLOCK      0x0008  
#define SFT_FNUL        0x0004  
#define SFT_FCONOUT     0x0002 
#define SFT_FCONIN      0x0001  
#define sft_dcb         sft_dcb_or_dev._sft_dcb
#define sft_dev         sft_dcb_or_dev._sft_dev
#define sft_flags	sft_flags_union._sft_flags
#define sft_flags_hi	sft_flags_union._split_sft_flags._sft_flags_hi
#define sft_flags_lo	sft_flags_union._split_sft_flags._sft_flags_lo
