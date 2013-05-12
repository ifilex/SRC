typedef struct
{
  BYTE dm_drive;
  BYTE dm_name_pat[FNAME_SIZE + FEXT_SIZE];
  BYTE dm_attr_srch;
  UWORD dm_entry;
  UWORD dm_cluster;
  struct
  {
    UWORD                       
    f_dmod:1;
    UWORD                   
    f_droot:1;
    UWORD                    
    f_dnew:1;
    UWORD              
    f_ddir:1;
    UWORD                   
    f_dfull:1;
    UWORD                      
    f_filler:11;       
  }
  dm_flags;                     
  UWORD dm_dirstart;
  BYTE dm_attr_fnd;            
  time dm_time;                
  date dm_date;                
  LONG dm_size;                
  BYTE dm_name[FNAME_SIZE + FEXT_SIZE + 2];    
}
dmatch;
