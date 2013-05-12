struct f_node
{
  UWORD f_count;                
  COUNT f_mode;            

  struct
  {
    BOOL f_dmod:1;             
    BOOL f_droot:1;         
    BOOL f_dnew:1;            
    BOOL f_ddir:1;            
    BOOL f_dfull:1;          
  }
  f_flags;                  
  struct dirent f_dir;        
  ULONG f_diroff;             
  UWORD f_dirstart;            
  struct dpb *f_dpb;            
  ULONG f_dsize;            
  ULONG f_offset;            
  ULONG f_highwater;     
  UWORD f_back;                
  ULONG f_cluster_offset;   
  UWORD f_cluster;            
  UWORD f_sector;           
  UWORD f_boff;              
};
