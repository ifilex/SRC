_D_DOPEN	equ	0001h
_D_ONSCR	equ	0002h
_D_DMOVE	equ	0004h
_D_SHADE	equ	0008h
_D_MYBUF	equ	0010h	; do not delete on exit (static)
_D_RCNEW	equ	0020h	; twclose -- delete dialog if set
_D_RESAT	equ	0040h	; attrib is index in color table (rcedit)
_D_DHELP	equ	0080h	; execute thelp() if set
_D_CLEAR	equ	0100h   ; args on open/create
_D_BACKG	equ	0200h
_D_FOREG	equ	0400h
_D_STERR	equ	1000h	; error color (red)
_D_MENUS	equ	2000h	; menus color (gray), no title
_D_MUSER	equ	4000h

_D_COLOR	equ	_D_BACKG or _D_FOREG
_D_RESRC	equ	0030h
_D_SETRC	equ	0031h
_D_STDDLG	equ	(_D_DMOVE or _D_SHADE or _D_CLEAR or _D_COLOR)
_D_STDERR	equ	(_D_STDDLG or _D_STERR)

_C_NORMAL	equ	1
_C_RETURN	equ	2
_C_ESCAPE	equ	3
_C_REOPEN	equ	4

S_DOBJ		STRUC
dl_flag		dw ?
dl_count	db ?
dl_index	db ?
dl_rect		dd ?
dl_wp		dd ?
dl_object	dd ?
S_DOBJ		ENDS

GLOBAL	tdialog:	dword
GLOBAL	tdllist:	dword
GLOBAL	thelp:		FPTR ; int (*thelp)(void);
GLOBAL	tupdate:	FPTR ; int (*tupdate)(void);
GLOBAL	tgetevent:	FPTR ; int (*tgetevent)(void);

GLOBALP	dlopen,	 	:dword, :word, :dword
GLOBALP	dlclose,	:dword
GLOBALP	dlshow,	 	:dword
GLOBALP	dlhide,	 	:dword
GLOBALP	dlmove,		:dword
GLOBALP	dlmodal,	:dword
GLOBALP	dlevent,   	:dword
GLOBALP	dllevent,  	:dword, :dword
GLOBALP	dlinit,	   	:dword
GLOBALP	dlinitobj,	:dword, :dword
GLOBALP dledit,		:dword, :dword, :word, :word
GLOBALP	dlscreen,	:dword, :word

GLOBALD	dlpbuttevent
GLOBALD	dlradioevent
GLOBALD	dlcheckevent
GLOBALD	dlxcellevent
GLOBALD	dlteditevent
GLOBALD	dlmenusevent

GLOBALP	thelpinit,	:FPTR	; global help proc
GLOBALP	thelp_set,	:FPTR	; locale help proc
GLOBALD	thelp_pop
