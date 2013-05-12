#ifndef __INC_CTYPE
#define __INC_CTYPE
#if !defined(__INC_DEFS)
 #include <defs.h>
#endif

#define _UPPER	= 01h	// upper case letter
#define _LOWER	= 02h	// lower case letter
#define _DIGIT	= 04h	// digit[0-9]
#define _SPACE	= 08h	// tab, carriage return, newline, vertical tab or form feed
#define _PUNCT	= 10h	// punctuation character
#define _CONTROL= 20h	// control character
#define _BLANK	= 40h	// space char
#define _HEX	= 80h	// hexadecimal digit

#ifdef __cplusplus
 extern "C" {
#endif

#define _tolower(_c)	((_c)-'A'+'a')
#define _toupper(_c)	((_c)-'a'+'A')

int _fastcall ftolower(char __ch);
int _fastcall ftoupper(char __ch);

int _CType tolower(int __ch);
int _CType toupper(int __ch);

#ifdef __cplusplus
 }
#endif
#endif /* __INC_CTYPE */
