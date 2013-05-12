CURSOR_NORMAL	equ	0607h
CURSOR_HIDDEN	equ	0F00h

S_CURSOR 	STRUC
cr_xy		dw ?
cr_type		dw ?
S_CURSOR 	ENDS

GLOBALD	cursorx
GLOBALD	cursory
GLOBALD	cursoron
GLOBALD	cursoroff
GLOBALP	gotoxy, 	:word, :word
GLOBALP	getcursor, 	:dword		; (cursor *)
GLOBALP	setcursor, 	:dword		; (cursor)

