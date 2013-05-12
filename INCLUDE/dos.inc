SEEK_CUR = 1
SEEK_END = 2
SEEK_SET = 0

_A_NORMAL EQU	00h
_A_RDONLY EQU	01h
_A_HIDDEN EQU	02h
_A_SYSTEM EQU	04h
_A_VOLID  EQU	08h
_A_SUBDIR EQU	10h
_A_ARCH	  EQU	20h

_A_STDFILES = _A_ARCH or _A_RDONLY or _A_SYSTEM or _A_SUBDIR
_A_ALLFILES = _A_STDFILES or _A_HIDDEN

S_FFBLK		STRUC
ff_reserved	DB 21 dup(?)
ff_attrib	DB ?
ff_ftime	DW ?
ff_fdate	DW ?
ff_fsize	DD ?
ff_name		DB 13 dup(?)
S_FFBLK		ENDS

S_WFBLK		STRUC
wf_attrib	DW ?	; DWORD File attributes
		DW ?
wf_timecreate	DW ?	; QWORD	File creation time
wf_datecreate	DW ?
wf_createdate	DD ?
wf_timeaccess	DW ?	; QWORD	Last access time
wf_dateaccess	DW ?
wf_accessdate	DD ?
wf_timemodified DW ?	; QWORD	Last modification time
wf_datemodified DW ?
wf_modified	DD ?
wf_sizedx	DD ?	; QWORD	File size
wf_sizeax	DD ?	; - low DWORD
wf_reserved	DB   8 dup(?)
wf_name		DB 260 dup(?)
wf_shortname	DB  14 dup(?)
S_WFBLK		ENDS	;  318 byte

S_DISKFREE 	STRUC
df_size		DW ?
df_version	DW ?
df_sclus	DD ?
df_bsec		DD ?
df_avail	DD ?
df_total	DD ?
df_phavail	DD ?
df_phtotal	DD ?
df_alavail	DD ?
df_altotal	DD ?
df_reserved	DB 8 dup(?)
S_DISKFREE 	ENDS

ifdef __LFN__
GLOBAL _ifsmgr:	BYTE
endif

GLOBALD	dosdate
GLOBALD	dostime
GLOBALD	getday
GLOBALD	getsec
GLOBALD	getmin
GLOBALD	gethour
GLOBALD	getmnd
GLOBALD	getyear
GLOBALD	getdatew
GLOBALN	@dosmaper

GLOBALP	dosmaperr,	:WORD
GLOBALP	setblock,	:WORD, :WORD
GLOBALP	freemem,	:WORD
GLOBALP	getcbrk,	:WORD
GLOBALP	getdta,		:WORD
GLOBALP	getpsp,		:WORD
GLOBALP	setdta,		:WORD
GLOBALP	setvect,	:WORD
GLOBALP	delay,		:WORD
GLOBALP	findfirst,	:DPTR, :DPTR, :WORD
GLOBALP	findnext,	:DPTR
GLOBALP	remove,		:DPTR
GLOBALP	rename,		:DPTR, :DPTR
GLOBALP	beep,		:WORD, :WORD
GLOBALP	drvready,	:WORD
GLOBALP	filexist,	:DPTR
GLOBALP	setfdate,	:WORD, :WORD, :WORD
GLOBALP	setfattr,	:DPTR, :WORD
GLOBALP	getfattr,	:DPTR
GLOBALP	getftime,	:WORD
GLOBALP	absread, 	:WORD, :WORD, :DWORD, :DWORD

ifdef __LFN__

GLOBALP	wfindfirst,	:DPTR, :DPTR, :WORD
GLOBALP	wfindnext,	:DPTR, :WORD
GLOBALP	wcloseff,	:WORD
GLOBALP	wsetaccessdate,	:WORD, :WORD, :WORD
GLOBALP	wsetcreatedate,	:WORD, :WORD, :WORD
GLOBALP	wsetfattr,	:DPTR, :WORD
GLOBALP	wgetfattr,	:DPTR
GLOBALP	wsetacdate,	:DPTR, :WORD
GLOBALP	wsetwrdate,	:DPTR, :WORD, :WORD
GLOBALP	wsetcrdate,	:DPTR, :WORD, :WORD
GLOBALP	wgetacdate,	:DPTR
GLOBALP	wgetwrdate,	:DPTR
GLOBALP	wgetcrdate,	:DPTR
GLOBALP	wfullpath,	:DPTR, :WORD
GLOBALP	wvolinfo,	:DPTR, :DPTR

endif

GLOBALP	wgetcwd,	:DPTR, :WORD
GLOBALP	wlongname,	:DPTR, :DPTR
GLOBALP	wlongpath,	:DPTR, :DPTR
GLOBALP	wshortname,	:DPTR
GLOBALP	removefile,	:DWORD

getfat		proto dist pascal :word, :dword
getdfree	proto dist pascal :word, :dword
getxdfree	proto dist pascal :dword, :dword
getexdpb 	proto dist pascal :word, :dword
