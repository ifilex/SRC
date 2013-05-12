struct dpb
{
  BYTE dpb_unit;                 
  BYTE dpb_subunit;            
  UWORD dpb_secsize;           
  UBYTE dpb_clsmask;           
  UBYTE dpb_shftcnt;           
  UWORD dpb_fatstrt;      
  UBYTE dpb_fats;          
  UWORD dpb_dirents;            
  UWORD dpb_data;              
  UWORD dpb_size;               
  UWORD dpb_fatsize;            
  UWORD dpb_dirstrt;         
  struct dhdr FAR *          
    dpb_device;
  UBYTE dpb_mdb;              
  BYTE dpb_flags;               
  struct dpb FAR *           
    dpb_next;                   
  UWORD dpb_cluster;             
  UWORD dpb_nfreeclst;           
  UBYTE dpb_clssize;            
  UCOUNT dpb_count;          
  BYTE dpb_path[64];           
};

#define UNKNCLUSTER     0xffff   
