#ifndef __INC_DEFS
#define __INC_DEFS

#define _CType	_pascal

#if defined(__LARGE__) || defined(__HUGE__) || defined(__COMPACT__)
 #define LDATA	1
 #define NULL	0L
#else
 #define LDATA	0
 #define NULL	0
#endif
#if defined(__LARGE__) || defined(__HUGE__) || defined(__MEDIUM__)
 #define LPROG 1
#else
 #define LPROG 0
#endif

#ifdef __cplusplus
 extern "C" {
#endif

typedef unsigned char   BYTE;
typedef unsigned int	WORD;
typedef unsigned long   DWORD;
typedef unsigned int    size_t;

#define MIN(a,b)	(((a) < (b)) ? (a) : (b))
#define MAX(a,b)	(((a) > (b)) ? (a) : (b))
#define INRANGE(x,s,e)	(((x) >= (s)) && ((x) <= (e)))
#define TOUPPER(c)	((c) & 0xDF)
#define TOLOWER(c)	((c) | 0x20)
#define MKW(hb,lb)  	((WORD)(((BYTE)hb<<8)|((BYTE)lb)))

#ifndef DEBUG
 #define assert(exp)  ((void)0)
#else
 void _CType debugmsg(char *, int, char *, int);
 #define assert(exp)\
    do { if (!(exp)) debugmsg(__FILE__, __LINE__, #exp, 1); } while(0)
#endif

#ifdef __cplusplus
 }
#endif
#endif
