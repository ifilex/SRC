typedef union
{
  UWORD x;                      
  struct
  {
    UBYTE l;                   
    UBYTE h;                   
  }
  b;
}
xreg;
typedef struct
{
  xreg a,
    b,
    c,
    d;
  UWORD si,
    di,
    bp,
    ds,
    es;
  UWORD ip,
    cs,
    flags;
}
iregs;
typedef struct
{
  UWORD pc_ss;
  UWORD pc_sp;
  iregs pc_regs;
}
pcb;
#define BP      bp
#define DI      di
#define SI      si
#define DS      ds
#define ES      es
#define DX      d.x
#define CX      c.x
#define BX      b.x
#define AX      a.x
#define DH      d.b.h
#define CH      c.b.h
#define BH      b.b.h
#define AH      a.b.h
#define DL      d.b.l
#define CL      c.b.l
#define BL      b.b.l
#define AL      a.b.l
#define IP      ip
#define CS      cs
#define FLAGS   flags
#define FLG_ZERO        0x0040
#define FLG_CARRY       0x0001
