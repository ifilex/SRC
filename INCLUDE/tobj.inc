_O_PBUTT	equ	0
_O_RBUTT	equ	1
_O_CHBOX	equ	2
_O_XCELL	equ	3
_O_TEDIT	equ	4
_O_MENUS	equ	5
_O_XHTML	equ	6
_O_MOUSE	equ	7
_O_LLMSU	equ	8
_O_LLMSD	equ	9
_O_TBUTT	equ	10

_O_RADIO	equ	0010h	; Used in (*) Radio  (one of)
_O_FLAGB	equ	0020h	; Used in [x] switch (ON/OFF)
_O_LLIST	equ	0040h	; Linked list item
_O_DTEXT	equ	0080h	; Select text on activation
_O_CONTR	equ	0100h	; Allow _CONTROL chars
_O_DEXIT	equ	0200h	; Close dialog and return 0: Cancel
_O_PBKEY	equ	0400h	; Return result if short key used
_O_GLCMD	equ	1000h	; Object contain global short-key table
_O_EVENT	equ	2000h	; Object have local event handler
_O_CHILD	equ	4000h	; Object have a child
_O_STATE	equ	8000h	; State (ON/OFF)
_O_DEACT	equ	_O_STATE

TO_OFLAG	equ	00h
TO_OCOUNT	equ	02h
TO_OASCII	equ	03h
TO_ORECT	equ	04h
TO_ODATA	equ	08h
TO_SDATA	equ	0Ah
TO_OPROC	equ	0Ch
TO_SPROC	equ	0Eh

S_TOBJ		STRUC
to_flag		dw ?
to_count	db ?
to_ascii	db ?
to_rect		dd ?
to_data		dd ?
to_proc		PF ?
ifndef LPROG	; must align 16 byte..
to_proc_dx	dw ?
endif
S_TOBJ		ENDS

GLOBALP	tosetbitflag,	:dword, :word, :word, :dword
GLOBALP	togetbitflag,	:dword, :word, :word
