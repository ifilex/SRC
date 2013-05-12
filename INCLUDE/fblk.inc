_A_FATTRIB	= 003Fh
_A_SELECTED	= 0040h
_A_UPDIR	= 0080h
_A_CDROOM	= 0100h
_A_ENCRYPTED	= 0200h
_A_EXTLOCHD	= 0400h
_A_FFBLOCK	= 0800h
_A_TEBLOCK	= 1000h
_A_ROOTDIR	= 2000h
_A_ARCHEXT	= 4000h
_A_ARCHZIP	= 8000h
_A_ARCHIVE	= 0C000h

S_FBLK	STRUC
fb_flag	DW ?
fb_time	DW ?
fb_date	DW ?
fb_size	DD ?
fb_name	DB 2 dup(?)
S_FBLK	ENDS

GLOBAL	cp_dotdot:  BYTE	; ".."
GLOBAL	cp_datefrm: BYTE	; "%2u.%02u.%02u"
GLOBAL	cp_timefrm: BYTE	; "%2u:%02u"

GLOBALP	fballoc, 	:dword, :word, :word, :dword, :word
GLOBALP	fbstaticff,  	:DWORD
GLOBALP	fballocff,   	:DWORD, :WORD
GLOBALP	fballocwf,   	:DWORD, :WORD
GLOBALP	fbupdir,     	:WORD
GLOBALP	fbinvert,    	:DWORD
GLOBALP	fbselect,    	:DWORD
GLOBALP	fbffirst,   	:DWORD, :WORD

fbcolor proto DIST
fbputsl	proto pascal :dword, :word, :word
fbputsd	proto pascal :dword, :word, :word
ifdef __LFN__
fbputll	proto pascal :dword, :word, :word
fbputld	proto pascal :dword, :word, :word
endif
fbputdate proto pascal :dword, :word, :word
fbputtime proto pascal :dword, :word, :word
fbputfile proto pascal :dword, :word, :word
