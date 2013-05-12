#ifndef __INC_DOS
#define __INC_DOS
#if !defined(__INC_DEFS)
 #include <defs.h>
#endif

extern  int    	_CType _argc;
extern  char **	_CType _argv;
extern  char **	_CType _environ;
extern  WORD	_CType _psp;
extern 	WORD	_CType _heaplen;
extern 	WORD	_CType _stklen;
extern  WORD	_CType _osversion;
extern  BYTE	_CType _osmajor;
extern  BYTE	_CType _osminor;
extern  BYTE	_CType _ifsmgr;
extern  WORD	_CType envseg;
extern  WORD	_CType envlen;

#define _A_NORMAL       0x00
#define _A_RDONLY       0x01
#define _A_HIDDEN       0x02
#define _A_SYSTEM       0x04
#define _A_VOLID    	0x08
#define _A_SUBDIR       0x10
#define _A_ARCH         0x20

#ifdef __cplusplus
 extern "C" {
#endif

#define  FP_OFF(__p) ((unsigned)(__p))

#ifdef __BORLANDC__
 #define MK_FP(s,o) ((void _seg *)(s)+(void near *)(o))
 #define FP_SEG(fp) ((unsigned)(void _seg *)(void far *)(fp))
#else
 #define MK_FP(s,o) (((unsigned short)(s)):>((void __near *)(o)))
 #define FP_SEG(p) ((unsigned)((unsigned long)(void __far*)(p) >> 16))
#endif

#ifndef _FFBLK_DEF
#define _FFBLK_DEF
typedef struct {
	char	reserved[21];
	char	ff_attrib;
	WORD	ff_ftime;
	WORD	ff_fdate;
	DWORD	ff_fsize;
	char	ff_name[13];
} ffblk;
#endif

int 	_CType findfirst(char *, ffblk *, int);
int 	_CType findnext(ffblk *);
int 	_CType remove(char *__fn);
void 	_CType delay(WORD __milliseconds);
int 	_CType setblock(WORD __segx, WORD __newsize);
int 	_CType filexist(char *__f);
int 	_CType setfdate(int __handle, int __date, int __time);
int 	_CType setfattr(char *__f, int __newattr);
int 	_CType getfattr(char *__f);
int	_CType getdfree(int __disk, void /*diskfree_t*/ *);
int	_CType getxdfree(char *__path, void /*xdiskfree_t*/ *);
int	_CType getfat(int __disk, void *__infostruc);
int	_CType absread(int __drv, int __nsects, long l__sect, void *__buf);
int 	_CType wabsread(int __drv, int __nsects, long l__sect, void *__buf);
void 	_CType beep(int __hz, int __time);

typedef struct {
	DWORD	attrib;
	WORD	time_create;
	WORD	date_create;
	DWORD	create;
	WORD	time_access;
	WORD	date_access;
	DWORD	access;
	WORD	time_modified;
	WORD	date_modified;
	DWORD	modified;
	DWORD	sizedx;
	DWORD	sizeax;
	char	reserved[8];
	char	name[260];
	char	shname[14];
      } wfblk;

int 	_CType wfindfirst(char *, wfblk *, unsigned);
int 	_CType wfindnext(wfblk *, int __handle);
int 	_CType wcloseff(int __handle);
char *	_CType wlongname(char *__sp, char *__f);
char *	_CType wlongpath(char *__sp, char *__f);
char *	_CType wshortname(char *__sp);
int 	_CType wsetaccessdate(int __handle, int __date, int __time);
int 	_CType wsetcreatedate(int __handle, int __date, int __time);
int 	_CType wsetfattr(char *__file, int __attrib);
int 	_CType wgetfattr(char *__file);
int 	_CType wsetacdate(char *__file, int __date);
int 	_CType wsetwrdate(char *__file, int __date, int __time);
int 	_CType wsetcrdate(char *__file, int __date, int __time);
int 	_CType wgetacdate(char *__file);
long 	_CType wgetwrdate(char *__file);
long 	_CType wgetcrdate(char *__file);
char * 	_CType wgetcwd(char *, int);
char * 	_CType wfullpath(char *, int);
int 	_CType wvolinfo(char *__drive, char *__buf32);
int 	_CType removefile(char *__file); /* clear attrib and remove */

#ifdef __cplusplus
 }
#endif
#endif
