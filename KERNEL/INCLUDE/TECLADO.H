#define KBD_MAXLENGTH   256
typedef struct
{
  UBYTE kb_size;             
  UBYTE kb_count;           
  BYTE kb_buf[KBD_MAXLENGTH];   
}
keyboard;
