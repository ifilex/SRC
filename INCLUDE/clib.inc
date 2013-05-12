__386__ = 1

ifndef __c__
 ifndef __l__
    .err you must supply a model symbol.
 endif
endif

ifdef	__c__
	LDATA	equ	1
endif

ifdef	__l__
	LPROG 	equ	1
	LDATA	equ	1
endif

ifdef 	__DZ__
	__ROT__ 	= 1
	__ZIP__		= 1	; ZIP
	__DLL__		= 1	; Plugins
	__CAL__		= 1	; Calendar
	__FF__		= 1	; File Find
	__AT__		= 1	; Edit Color
	__TV__		= 1	; Text View
	__TE__		= 1	; Text Edit
endif

ifdef 	__TVEXE__
	__TV__		= 1	; TV.EXE
endif

ifdef 	__FFEXE__
	__TVEXE__	= 1	; FF.EXE
	__TE__		= 1
endif

ifndef	__X__
	__CLIP__	= 1
	__LFN__		= 1
	__MOUSE__	= 1
endif

include	macro.inc

extrn	_psp:		word
extrn	envseg:		word
extrn	envlen:		word
extrn	envsize:	word
extrn	_osminor:	byte
extrn	_osmajor:	byte
extrn	_osversion:	word
extrn	errno:		word
extrn	doserrno:	word
extrn	sys_erdevice:	dword
extrn	sys_ercode:	word
extrn	sys_erflag:	byte
extrn	sys_erdrive:	byte
extrn	sys_erproc:	FPTR
ifdef LDATA
 extrn	heaptop:	word
 extrn	heapbase:	word
 extrn	heapfree:	word
 extrn	brklvl:		word
else
 extrn	_brklvl:	word
endif
extrn	_dsstack:	word
extrn	stderrmsg:	near
extrn	_restorezero:	near
extrn	_abort:		dist
extrn	_terminate:	dist
extrn	exit:		dist
ifdef DEBUG
;extrn	settime:	DIST
;extrn	gettime:	DIST
endif

_TEXT	SEGMENT word USE16 public 'CODE'
ifdef	__386__
	.386
else
	.186
endif
_TEXT	ENDS
_DATA	SEGMENT word USE16 public 'DATA'
_DATA	ENDS
_INIT_	SEGMENT word USE16 public 'INITDATA'
_INIT_	ENDS

	DGROUP GROUP _DATA,_INIT_
	ASSUME cs:_TEXT, ss:DGROUP, ds:DGROUP
