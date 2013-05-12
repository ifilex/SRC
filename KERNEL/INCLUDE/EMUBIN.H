#define LARGEST         -1
#define FIRST_FIT       0
#define BEST_FIT        1
#define LAST_FIT        2
#define MCB_NORMAL      0x4d
#define MCB_LAST        0x5a
#define DOS_PSP         0x0008
#define FREE_PSP        0
#define MCB_SIZE(x)     ((((LONG)(x))<<4)+sizeof(mcb))

typedef UWORD seg;
typedef UWORD offset;

typedef struct
{
  BYTE m_type;                  
  UWORD m_psp;                  
  UWORD m_size;                
  BYTE m_fill[3];
  BYTE m_name[8];               
}
mcb;
