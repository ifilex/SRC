include dobj.inc

TIMAXLINE	= 1024
STYLESIZE	= 4096
EMSMINPAGES	= 4
TIMINLINE	= 128
WMAXPATH	= 260

S_TFILE		STRUC
tf_name		db WMAXPATH dup (?)
tf_time		dw ?
tf_xoff		dw ?
tf_boff		dw ?
tf_yoff		dw ?
tf_loff		dw ?
S_TFILE		ENDS

S_TEDIT		STRUC
te_info 	S_TINFO <?>
ti_stat		db ?	; style attrib
ti_stch		db ?	; style char
ti_lcnt		dw ?	; line count in file
ti_emmh		dw ?	; EMM handle
ti_emmp		dw ?	; EMM page
ti_file		dd ?	; file name
ti_time		dw ?	; file time
ti_date		dw ?	; file date
ti_size		dd ?	; file size
ti_next		dw ?	; next file
ti_prev		dw ?	; prev file
ti_dialog	dd ?
ti_style	dd ?
ti_DOBJ		S_DOBJ <?>
S_TEDIT		ENDS	; 48 + S_TINFO byte

externdef	teflag:		WORD
externdef	tepages:	WORD
externdef	telsize:	WORD

tigetline	PROTO 	; AX line
tigetfile	PROTO DIST
topen		PROTO DIST PASCAL :DWORD
tclose  	PROTO DIST
tcloseall	PROTO DIST
tmodal		PROTO DIST PASCAL
