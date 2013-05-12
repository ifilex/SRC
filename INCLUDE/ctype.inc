_UPPER	= 01h	; upper case letter
_LOWER	= 02h	; lower case letter
_DIGIT	= 04h	; digit[0-9]
_SPACE	= 08h	; tab, carriage return, newline, vertical tab or form feed
_PUNCT	= 10h	; punctuation character
_CONTROL= 20h	; control character
_BLANK	= 40h	; space char
_HEX	= 80h	; hexadecimal digit


GLOBAL	__ctype: BYTE	; 257
GLOBALN	getctype	; arg: AL, return AH
			; arg: AL, return ZF
GLOBALN	isalpha		; letter (A to Z or a to z)
GLOBALN	isascii		; the low order byte of c is in the range 0 to 127
			; (0x00--0x7F)
GLOBALN	iscntrl		; delete character or ordinary control character
			; (0x7F or 0x00 to 0x1F)
GLOBALN	isdigit		; digit (0 to 9)
GLOBALN	isprint		; printing character
GLOBALN	isgraph		; printing character, like isprint, except that
			; a space character is excluded
GLOBALN	islower		; lowercase letter (a to z)
GLOBALN	isprint		; printing character (0x20 to 0x7E)
GLOBALN	ispunct		; punctuation character (iscntrl or isspace)
GLOBALN	isspace		; space, tab, carriage return, new line, vertical tab,
			; or formfeed (0x09 to 0x0D, 0x20)
GLOBALN	isupper		; uppercase letter (A to Z)
GLOBALN	isxdigit	; hexadecimal digit (0 to 9, A to F, a to f)
GLOBALN	islabel		; lower | upper | digit | '_'

