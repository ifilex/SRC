FILT_MAXSTRING	= 128

S_FILT	STRUC
of_flag		DW ?
of_max_date	DW ?
of_min_date	DW ?
of_max_size	DD ?
of_min_size	DD ?
of_include	DB FILT_MAXSTRING dup(?)
of_exclude	DB FILT_MAXSTRING dup(?)
S_FILT	ENDS

GLOBAL 	filter:		DWORD
GLOBALP	filter_fblk,	:DWORD
GLOBALP	filter_wblk,	:DWORD
GLOBALP	filter_edit,	:DWORD, :DWORD
