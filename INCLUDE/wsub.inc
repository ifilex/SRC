WMAXPATH	= 260
MAXFBLOCK	= 3000
WMAXFBLOCK	= 3FFFh

_W_ROOTDIR	= 0001h
_W_LONGNAME	= 0010h
_W_HIDDEN	= 0040h
_W_FLOPPY	= 0080h	; wsetflag -- not used
_W_CDROOM	= 0100h
_W_NETWORK	= 0180h
_W_SORTNAME	= 0000h
_W_SORTTYPE     = 0200h
_W_SORTDATE     = 0400h
_W_SORTSIZE	= 0600h
_W_NOSORT	= 0800h
_W_SORTSUB	= 1000h
_W_ARCHEXT	= 4000h
_W_ARCHZIP	= 8000h
_W_ARCHIVE	= _W_ARCHZIP or _W_ARCHEXT

S_PATH		STRUC
wp_flag		DW ?
wp_mask		DB  32 dup(?)
wp_path		DB 260 dup(?)
wp_file		DB 260 dup(?)
wp_arch		DB 260 dup(?)
S_PATH		ENDS	;  814 byte

S_WSUB		STRUC
ws_count	DW ?
ws_maxfb	DW ?
ws_flag		DD ?
ws_mask		DD ?
ws_path		DD ?
ws_file		DD ?
ws_arch		DD ?
ws_fcb		DD ?
S_WSUB		ENDS	; 28 byte

S_DISK		STRUC
di_flag		DW ?	; align FBLK
di_time		DW ?
di_date		DW ?
di_sizeax	DD ?
di_name		DB 16 dup(?)
di_sizedx	DD ?
S_DISK		ENDS

GLOBAL	drvinfo:	S_DISK
GLOBAL	cp_stdmask:	BYTE
GLOBAL	cp_warning:	BYTE
GLOBAL	cp_emaxfb:	BYTE
GLOBAL	__srcpath:	BYTE
GLOBAL	__outpath:	BYTE
GLOBAL	__srcfile:	BYTE
GLOBAL	__outfile:	BYTE
GLOBAL	cp_errorinitsub: BYTE

GLOBALP	wsopen,	    	:DWORD
GLOBALP	wsclose,    	:DWORD
GLOBALP	wsinit,     	:DWORD
GLOBALP	wsfree,     	:DWORD
GLOBALP	wsread,     	:DWORD
GLOBALP	wssort,     	:DWORD
GLOBALP	wssetflag,  	:DWORD
GLOBALP	wslocal,    	:DWORD
GLOBALP	wschdrv,    	:DWORD, :WORD
GLOBALP	wsearch,    	:DWORD, :DWORD
GLOBALP	wsfblk,     	:DWORD, :WORD
GLOBALP	wsffirst,   	:DWORD
GLOBALP	wsmkdir,    	:DWORD, :DWORD
GLOBALP	wsmkdirlocal,	:DWORD
GLOBALP	wsetprogress
GLOBALP	wdlgopen, 	:dword, :dword, :word

wgetfile	proto _CType 	:dword, :word

GLOBALP	wscopy_open	; ax, dx (__srcfile, __outfile)
GLOBALP	wscopy_remove   ; ax (__outfile)

ifdef __ZIP__
 GLOBAL	entryname:	BYTE
 GLOBALP wsopenarch, 	:DWORD
 GLOBALP wzipread,   	:DWORD
 GLOBALP wzipopen
 GLOBALP wzipclose
 GLOBALP wsdecomp, 	:DWORD, :DWORD, :DWORD
 GLOBALP wzipadd,	:DWORD, :WORD, :WORD, :WORD
 GLOBALP wzipdel, 	:DWORD, :DWORD ; = SI:DI, CX = flag, AX = recursive
 GLOBALP wzipcopy,	:DWORD, :DWORD, :DWORD
 GLOBALP wzipcopypath, 	:DWORD, :DWORD, :DWORD
 GLOBALP wzipcopyfile, 	:DWORD, :DWORD, :DWORD
 GLOBALP wzipfindentry,	:DWORD, :WORD
endif
