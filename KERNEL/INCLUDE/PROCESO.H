#define P_WAIT    0             
#define P_NOWAIT  1            
#define P_OVERLAY 2            

typedef struct
{
  union
  {
    struct
    {
      UWORD load_seg;
      UWORD reloc;
    }
    _load;
    struct
    {
      UWORD env_seg;
      CommandTail FAR *cmd_line;
      BYTE FAR *fcb_1;
      BYTE FAR *fcb_2;
      BYTE FAR *stack;
      BYTE FAR *start_addr;
    }
    _exec;
  }
  ldata;
}
exec_blk;

#define exec    ldata._exec
#define load    ldata._load

typedef struct
{
  UWORD ps_exit;                
  UWORD ps_size;             
  BYTE ps_fill1;             
  BYTE ps_farcall;             
    VOID(FAR * ps_reentry) ();  
    VOID(interrupt FAR * ps_isv22) (),  
    (interrupt FAR * ps_isv23) (),     
    (interrupt FAR * ps_isv24) ();     
  UWORD ps_parent;             
  UBYTE ps_files[20];           
  UWORD ps_environ;             
  BYTE FAR *ps_stack;        
  WORD ps_maxfiles;           
  UBYTE FAR *ps_filetab;        
  VOID FAR *ps_prevpsp;         
  BYTE FAR *ps_dta;         
  BYTE ps_fill2[16];
  BYTE ps_unix[3];              
  BYTE ps_fill3[9];
  union
  {
    struct
    {
      fcb
      _ps_fcb1;            
    }
    _u1;
    struct
    {
      BYTE
      fill4[16];
      fcb
      _ps_fcb2;                 
    }
    _u2;
    struct
    {
      BYTE fill5[36];
      struct
      {
        BYTE _ps_cmd_count;
        BYTE _ps_cmd[127];      
      }
      _u4;
    }
    _u3;
  }
  _u;
}
psp;

#define ps_fcb1 _u._u1._ps_fcb1
#define ps_fcb2 _u._u2._ps_fcb2
#define ps_cmd  _u._u3._u4._ps_cmd
#define ps_cmd_count    _u._u3._u4._ps_cmd_count
