#ifndef __INC_MOUSE
#define __INC_MOUSE
#if !defined(__INC_DEFS)
 #include <defs.h>
#endif

#define MOUSECMD      (-1)

#ifdef __cplusplus
 extern "C" {
#endif

int _CType mouseinit(void);
int _CType mousep(void);
int _CType mousex(void);
int _CType mousey(void);
int _CType mouseon(void);
int _CType mouseoff(void);
int _CType mousehide(void); /* if on: off() */
int _CType mouseshow(void); /* if was on: on() */
void _CType mousewait(int __x, int __y, int __len);
int _CType msloop(void);

#ifdef __cplusplus
 }
#endif
#endif
