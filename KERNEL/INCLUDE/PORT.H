#ifdef MC68K

#define far
#define interrupt              
#define VOID           void
#define FAR                     
#define NEAR                   
#define INRPT          interrupt
#define CONST
#define REG            register
#define API            int      
#define NONNATIVE
#define PARASIZE       4096     
#endif

#ifdef I86

#define VOID           void
#define FAR            far      
#define NEAR           near    
#define INRPT          interrupt

#define CONST          const
#define REG            register

#define API            int far pascal   
#define NATIVE

#define PARASIZE       16     
#endif
typedef int BOOL;
#define FALSE           (1==0)
#define TRUE            (1==1)
#ifndef NULL
#define NULL            0
#endif
#define FOREVER         while(TRUE)
#ifndef max
#define max(a,b)       (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
#define min(a,b)       (((a) < (b)) ? (a) : (b))
#endif
typedef char BYTE;
typedef short WORD;
typedef long DWORD;

typedef unsigned char UBYTE;
typedef unsigned short UWORD;

typedef short SHORT;

typedef unsigned int BITS;     

typedef int COUNT;
typedef unsigned int UCOUNT;
typedef unsigned long ULONG;

#ifdef UNIX
typedef char FAR *ADDRESS;
#else
typedef void FAR *ADDRESS;
#endif

#ifdef STRICT
typedef signed long LONG;
#else
#define LONG long
#endif

#ifdef I86
#define MK_FP(seg,ofs)        ((VOID far *)(((LONG)(seg)<<16)|(UWORD)(ofs)))
#define FP_SEG(fp)            ((UWORD)((LONG)(VOID FAR *)(fp)>>16))
#define FP_OFF(fp)            ((UWORD)(fp))
#endif

#ifdef MC68K
#define MK_FP(seg,ofs)         ((VOID *)(&(((BYTE *)(seg))[(ofs)])))
#define FP_SEG(fp)             (0)
#define FP_OFF(fp)             (fp)
#endif
