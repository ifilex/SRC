#ifndef __INC_IO
#define __INC_IO
#if !defined(__INC_DEFS)
 #include <defs.h>
#endif
#if !defined(_NFILE_)
 #include <_nfile.h>
#endif

#define SEEK_CUR    1
#define SEEK_END    2
#define SEEK_SET    0

#define _A_NORMAL       0x00
#define _A_RDONLY       0x01
#define _A_HIDDEN       0x02
#define _A_SYSTEM       0x04
#define _A_SUBDIR       0x10
#define _A_ARCH         0x20

#define _A_TEMPORARY    0x0100
#define _A_DELETE       0x0400
#define _A_SEQSCAN      0x0800
#define _A_RANDOM	0x1000

#define FH_OPEN		0x01
#define FH_EOF		0x02
#define FH_CRLF		0x04
#define FH_PIPE		0x08
#define FH_APPEND	0x20
#define FH_DEVICE	0x40
#define FH_TEXT		0x80
#define FH_INVALID	0xFF

#define A_OPEN		0x0001
#define A_TRUNC		0x0002
#define A_CREATE	0x0010

#define M_RDONLY       	0x0000
#define M_WRONLY       	0x0001
#define M_RDWR		0x0002
#define M_RDNOUPD      	0x0004
#define M_COMPAT   	0x0000
#define M_DENYRW   	0x0010
#define M_DENYWR   	0x0020
#define M_DENYRD   	0x0030
#define M_DENYNONE	0x0040
#define M_NOINHERIT    	0x0080
#define M_NOBUFFER	0x0100
#define M_NOCOMPR	0x0200
#define M_USEALIAS	0x0400
#define M_DELETE	0x1000
#define M_RETURNER	0x2000
#define M_COMMIT	0x4000

#define DEV_UNKNOWN   	0x0000
#define DEV_DISK      	0x0001
#define DEV_CHAR      	0x0002
#define DEV_PIPE      	0x0003
#define DEV_REMOTE    	0x8000

#define _MINIOBUF	0x0200
#define _INTIOBUF	0x1000
#define _MAXIOBUF       0x4000

#ifdef __cplusplus
 extern "C" {
#endif

extern int _CType _nfile;
extern int _CType _umaskval;
extern BYTE _CType _osfile[_NFILE_];

int  _CType osopen(char *, int __attrib, int __mode, int __act);
WORD _CType osread(int __hnd, void *, WORD);
WORD _CType oswrite(int __hnd, void *__buf, WORD __size);
int  _CType close(int __hnd);

int _cdecl open(char *__path, int __oflag, ...);
int _cdecl sopen(char *__path, int __oflag, int __shflag, ...);
int _CType creat(char *__path, int __amode);
int _CType _creat(char *__path, int __attrib);
int _CType access(char *__path, int __amode);
int _CType remove(char *);
int _CType rename(char *, char *);
int _CType chsize(int __hnd, long);
int _CType setmode(int __hnd, int __amode);
int _CType eof(int __hndl);
long _CType filelength(int __hnd);
long _CType lseek(int __hnd, long, int);
int _CType read(int __hnd, void *, WORD);
long _CType tell(int __hnd);
int _CType write(int __hnd, void *, WORD);

DWORD _CType readword(char *__file);
void _CType dosmaperr(int);

int _CType osfiletype(int __hndl);

int _CType isatty(int);
int _CType setfdate(int __hnd, int __date, int __time);
int _CType setfattr(char *, int __newattr);

int _CType _dup(int);
int _CType dup2(int, int);

#ifdef __cplusplus
 }
#endif
#endif
