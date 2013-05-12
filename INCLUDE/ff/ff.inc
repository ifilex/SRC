MAXHIT		= 999

S_SBLK		STRUC
sb_flag		DW ?
sb_size		DW ?
sb_line		DW ?
sb_offs		DD ?
sb_file		DB 2 dup(?)
S_SBLK		ENDS

INFOSIZE	= 38
BLOCKSIZE 	= SIZE S_SBLK + INFOSIZE

GLOBAL		DLG_FindFile:DWORD
GLOBAL		FCB_FindFile:WORD

ifndef __TVEXE__
FFOpen		PROTO DIST PASCAL lobj_offset:word
FFClose		PROTO DIST PASCAL lobj_offset:word
FFAlloc		PROTO DIST PASCAL path:dword, offs:dword, line:word, lobj_offset:word
FindFile 	PROTO DIST PASCAL path:dword, lobj_offset:word
endif
