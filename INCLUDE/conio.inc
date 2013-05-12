include console.inc
include cursor.inc
include wchar.inc
include color.inc
include rect.inc
include dobj.inc
include tobj.inc

S_OOBJ		STRUC
ro_flag		dw ?
ro_mem		db ?
ro_key		db ?
ro_rcx		db ?
ro_rcy		db ?
ro_rcc		db ?
ro_rcr		db ?
S_OOBJ		ENDS

S_ROBJ		STRUC
rs_memsize	dw ?
rs_flag		dw ?
rs_count	db ?
rs_index	db ?
rs_rect		dd ?
S_ROBJ		ENDS

S_LOBJ		STRUC
ll_dlgoff	dw ?	; start index in dialog
ll_dcount	dw ?	; number of cells (max)
ll_celoff	dw ?	; cell offset
ll_numcel	dw ?	; number of visible cells
ll_count	dw ?	; total number of items in list
ll_index	dw ?	; index in list buffer
ll_list		dd ?	; pointer to list buffer
ll_proc		PF ?
S_LOBJ		ENDS

S_GLCMD		STRUC
gl_key		dw ?
gl_proc		PF ?
S_GLCMD		ENDS

GCMD	MACRO	key, name	; GlobalCommand: <ShortKey>, <cmdProc>
	dw	key
 ifdef LPROG
	dd	name
 else
	dw	name
 endif
	ENDM

GLOBALN	@rsunzipat
GLOBALN	@rsunzipch

GLOBALD	@getxya
GLOBALD	@getxyc
GLOBALD	@getxyw
GLOBALD	@getxyp
GLOBALD	@scputa
GLOBALD	@scputc
GLOBALD	@scputw
GLOBALD	@scputfg
GLOBALD	@scputbg
GLOBALD	@scpath
GLOBALD	@scputs
GLOBALD	@scputds
GLOBALD	@scenter
GLOBALD	@scputf

GLOBALN	@putstr

GLOBAL	tdialog:	dword
GLOBAL	tdllist:	dword
GLOBAL	key_scancode:	byte
GLOBAL	cp_error: 	byte
GLOBAL	thelp:		FPTR ; int (*thelp)(void);
GLOBAL	tupdate:	FPTR ; int (*tupdate)(void);
GLOBAL	tgetevent:	FPTR ; int (*tgetevent)(void);

GLOBALN	@pbuttms
GLOBALD	@ermsg
GLOBALD	@ermsgt
GLOBALD	@stdmsg
GLOBALP	tmsgbox,   	:dword, :word
GLOBALD	getevent
GLOBALD	tupdtime
GLOBALD	tupddate
GLOBALD	scroll_delay
GLOBALP	rsopen,	   	:dword
GLOBALP	rsevent,   	:dword, :dword
GLOBALP	rsmodal,   	:dword
GLOBALP	rsreload,  	:dword, :dword
GLOBALP tedit,		:dword, :word
GLOBALP wedit,		:dword, :word
GLOBALP	getxys,		:word, :word, :dword, :word, :word
GLOBALP	tgetline,	:dword, :dword, :word, :word
GLOBALP	tihndlevent,	:word, :word

; Macros

_m_vpush	macro a1, a2, a3, a4, a5, a6
local	E1
	E1 = 0
	ifnb <a6>
	    push a6
	    E1 = E1 + 2
	endif
	ifnb <a5>
	    push a5
	    E1 = E1 + 2
	endif
	ifnb <a4>
	    push a4
	    E1 = E1 + 2
	endif
	ifnb <a3>
	    push a3
	    E1 = E1 + 2
	endif
	ifnb <a2>
	    push a2
	    E1 = E1 + 2
	endif
	ifnb <a1>
	    push a1
	    E1 = E1 + 2
	endif
	exitm %E1
	endm

m_vargs	macro cp, a1, a2, a3, a4, a5, a6
local   E1
	E1 = m_vpush a1, a2, a3, a4, a5, a6
	call cp
	ifdif <E1>,<0>
	    add sp,E1
	endif
	endm

m_vpush	macro a1, a2, a3, a4, a5, a6
	ifnb <a6>
	    push a6
	endif
	ifnb <a5>
	    push a5
	endif
	ifnb <a4>
	    push a4
	endif
	ifnb <a3>
	    push a3
	endif
	ifnb <a2>
	    push a2
	endif
	ifnb <a1>
	    push a1
	endif
	endm

scputf@	macro x, y, at, max, format, a1, a2, a3, a4, a5, a6
local   val
	ifnb <a1>
	    m_vpush a1, a2, a3, a4, a5, a6
	endif
	ifnb <a6>
	    val = 12
	elseifnb <a5>
	    val = 10
	elseifnb <a4>
	    val = 8
	elseifnb <a3>
	    val = 6
	elseifnb <a2>
	    val = 4
	elseifnb <a1>
	    val = 2
	else
	    val = 0
	endif
	ifdif <x>,<bl>
	    mov bl,x
	endif
	ifdif <y>,<bh>
	    mov bh,y
	endif
	ifdif <at>,<ch>
	    mov ch,at
	endif
	ifdif <max>,<cl>
	    mov cl,max
	endif
	ifdif <format>,<ax>
	    mov ax,format
	endif
	call @scputf
	if val
	    add sp,val
	endif
	endm

sprint@	macro b_s, b_o, format, a1, a2, a3, a4, a5, a6
	ifdif <b_s>,<dx>
	   mov dx,b_s
	endif
	ifdif <b_o>,<bx>
	   mov bx,b_o
	endif
	ifdif <format>,<ax>
	   mov ax,format
	endif
	m_vargs	@sprintf, a1, a2, a3, a4, a5, a6
	endm

scputw@	macro x, y, cnt, w
	ifdif <x>,<bl>
	  mov bl,x
	endif
	ifdif <y>,<bh>
	  mov bh,y
	endif
	ifdif <cnt>,<cx>
	  mov cx,cnt
	endif
	ifdif <w>,<ax>
	  mov ax,w
	endif
	call @scputw
	endm

scputc@	macro x, y, cnt, b
	mov ah,0
	ifdif <b>,<al>
	    mov al,b
	endif
	scputw@	x, y, cnt, ax
	endm

scputs@	macro x, y, at, cnt, b_s, b_o
	ifdif <x>,<bl>
	    mov bl,x
	endif
	ifdif <y>,<bh>
	    mov bh,y
	endif
	ifdif <at>,<ch>
	    mov ch,at
	endif
	ifdif <cnt>,<cl>
	    mov cl,cnt
	endif
	ifdif <b_s>,<dx>
	    mov dx,b_s
	endif
	ifdif <b_o>,<ax>
	    mov ax,b_o
	endif
	call @scputs
	endm

scpath@	macro x, y, cnt, b_s, b_o
	ifdif <x>,<bl>
	    mov bl,x
	endif
	ifdif <y>,<bh>
	    mov bh,y
	endif
	ifdif <cnt>,<cx>
	    mov cx,cnt
	endif
	ifdif <b_s>,<dx>
	    mov dx,b_s
	endif
	ifdif <b_o>,<ax>
	    mov ax,b_o
	endif
	call @scpath
	endm
