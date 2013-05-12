#ifdef MAIN
#endif
#define D_NORMAL        0      
#define D_RDONLY        0x01   
#define D_HIDDEN        0x02   
#define D_SYSTEM        0x04    
#define D_VOLID         0x08    
#define D_DIR           0x10   
#define D_ARCHIVE       0x20    
#define FNAME_SIZE              8
#define FEXT_SIZE               3
#define DELETED         0xe5    
#define clus_add(cl_no)         ((ULONG) (((ULONG) cl_no - 2L) \
                                        * (ULONG) cluster_size \
                                        + (ULONG) data_start))

#define clus2phys(cl_no,cl_size,d_st)   ((ULONG) (((ULONG) cl_no - 2L) \
                                        * (ULONG) cl_size \
                                        + (ULONG) d_st))
#define SIZEOF_CLST16   2
#define FAT_MAGIC       4086
#define ISFAT16(dpbp)   (((dpbp)->dpb_size)>FAT_MAGIC)
#define ISFAT12(dpbp)   (((dpbp)->dpb_size)<=FAT_MAGIC)
struct dirent
{
  UBYTE dir_name[FNAME_SIZE];   
  UBYTE dir_ext[FEXT_SIZE];     
  UBYTE dir_attrib;             
  BYTE dir_reserved[10];      
  time dir_time;               
  date dir_date;            
  UWORD dir_start;           
  ULONG dir_size;              
};
#define DIR_NAME        0
#define DIR_EXT         FNAME_SIZE
#define DIR_ATTRIB      FNAME_SIZE+FEXT_SIZE
#define DIR_RESERVED    FNAME_SIZE+FEXT_SIZE+1
#define DIR_TIME        FNAME_SIZE+FEXT_SIZE+11
#define DIR_DATE        FNAME_SIZE+FEXT_SIZE+13
#define DIR_START       FNAME_SIZE+FEXT_SIZE+15
#define DIR_SIZE        FNAME_SIZE+FEXT_SIZE+17
#define DIRENT_SIZE     32
