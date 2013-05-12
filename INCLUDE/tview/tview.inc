_TV_USEMLINE	= 01h
_TV_USESLINE	= 02h
_TV_HEXVIEW	= 04h
_TV_WRAPLINES	= 08h
_TV_HEXOFFSET	= 10h
_TV_HIGHCOLOR	= 20h

ifdef __TVEXE__

_TV_BINVIEW	= 40h
_TV_CLASSVIEW	= 80h

CLVERSION	= 0206h
CLCOUNT 	= 23

S_CLASS		STRUC
cl_name		db 28 dup(?)
cl_size		dw ?
cl_type		db ?
cl_format	db ?
S_CLASS		ENDS

TCLSIZE		= (CLCOUNT*SIZE S_CLASS) + 6
CLFLAGS		= _D_DOPEN or _D_ONSCR or _D_MYBUF

CLTYPE_CHAR	= 0
CLTYPE_BYTE	= 1
CLTYPE_WORD	= 2
CLTYPE_DWORD	= 3
CLTYPE_QWORD	= 4

CLFORM_SIGNED	= 0
CLFORM_UNSIGNED	= 1
CLFORM_HEX	= 2
CLFORM_BINARY	= 3

IO_MEMREAD	= 4000h	; View memory area

externdef	tv_clversion:		WORD
externdef	tv_classsize:		WORD
externdef	tv_classrow:		WORD
externdef	tv_class:		S_CLASS
externdef	IDD_TVClass:		DWORD
externdef	IDD_TVSaveClass:	DWORD
externdef	IDD_TVClassFormat:	DWORD
wgetfile	proto _CType :dword, :word

endif

ifdef __TVEXE__
tview	proto _CType :dword, :dword, :dword, :dword
else
tview	proto _CType :dword, :dword
endif
