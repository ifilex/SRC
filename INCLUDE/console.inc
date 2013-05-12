CON_UBEEP	equ	0001h	; Use Beep
CON_MOUSE	equ	0002h	; Use Mouse
CON_IOLFN	equ	0004h	; Use Long File Names
CON_CLIPB	equ	0008h	; Use System Clipboard
CON_INT2F	equ	0010h	; Use Idle Int 2Fh
CON_NTCMD	equ	0020h	; Use NT Prompt
CON_DOSIO	equ	0040h	; Use DOS2 Open/Create
CON_COLOR	equ	0080h	; Use Color
CON_UTIME	equ	0100h	; Use Time
CON_UDATE	equ	0200h	; Use Date
				; Doszip flags
CON_IMODE	equ	1000h	; Init screen mode on startup
CON_REVGA	equ	2000h	; Restore VGA palette on exit

GLOBAL	console: 	word	; BEEP/MOUSE/LFN/CLIPB/2F/COLOR/TIME/DATE
GLOBAL	console_wp: 	dword 	; Init screen (user screen)
GLOBAL	console_xy: 	word 	; Init cursor (set on exit)
GLOBAL	console_oz: 	byte 	; Init screen size

GLOBAL	_scrseg: 	word	; Screen segment (B800 or B000)
GLOBAL	_scrrow: 	byte	; Screen rows - 1
GLOBAL	_scrcol: 	byte	; Screen columns

GLOBALD _screct			; Update _scr* (_INIT_)
GLOBALD	consinit
GLOBALD	conssetl
GLOBALD	consuser
GLOBALD	consexit
