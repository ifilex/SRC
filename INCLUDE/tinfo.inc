_T_MALLOC	= 0001h
_T_EMMBUF	= 0002h
_T_LINEBUF	= 0003h
_T_FILE		= 0004h
_T_MODIFIED	= 0008h
_T_USECONTROL	= 0010h
_T_USESTYLE	= 0020h
_T_SHOWTABS	= 0040h
_T_USEMENUS	= 0080h
_T_OPTIMALFILL	= 0100h
_T_OVERWRITE	= 0200h
_T_USEINDENT	= 0400h
_T_USEBAKFILE	= 0800h
_T_USECRLF	= 1000h
_T_PROMPTONREP	= 2000h

_T_TEDEFAULT	= 1F20h
_T_TESAVEMASK	= 1FE0h

_TI_CONTINUE 	= 0	; continue edit
_TI_RETEVENT	= 1	; return current event (keystroke)
_TI_CMFAILED 	= 2	; operation fail (end of line/buffer)

S_TINFO		STRUC
ti_bp		dd ?	; base pointer
ti_flag		dw ?	; config
ti_xpos		dw ?	; text window x,y
ti_ypos		dw ?
ti_cols		dw ?	; size of screen-line
ti_bcol		dw ?	; size of buffer-line
ti_xoff		dw ?	; x offset on screen
ti_boff		dw ?	; base offset - start of screen-line
ti_clat		db ?	; clear attrib
ti_clch		db ?	; clear char
ti_cursor	dd ?	; this cursor
ti_bcnt		dw ?	; byte count in line
ifdef __CLIP__
 ti_clso	dw ?	; Clipboard start offset
 ti_cleo	dw ?	; Clipboard end offset
 ti_clsl	dw ?	; Clipboard start line
 ti_clel	dw ?	; Clipboard end line
endif
ti_brow		dw ?	; number of lines in buffer
ti_rows		dw ?	; number of lines on screen
ti_loff		dw ?	; line offset in buffer
ti_yoff		dw ?	; y offset on screen
S_TINFO		ENDS

GLOBAL	tinfo:	word	; offset SS/DS

TITABCHAR	= 0Bh	; expand/trunc tabs with this char (Vertical Tab)
TIMAXSCRLINE	= 128

ifdef __TE__
 include tedit/tedit.inc
endif
