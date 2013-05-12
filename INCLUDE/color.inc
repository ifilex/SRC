B_Black		= 	0
B_Desktop	= 	1
B_Dialog	= 	2
B_Menus		= 	3
B_Red		= 	4
B_Title		= 	5
B_DarkGray	= 	6
B_Gray		= 	7

F_Black		= 	0
F_Panel		= 	1
F_Frame		= 	2
F_Subdir	= 	3
F_Files		= 	4
F_System	= 	5
F_Hidden	= 	6
F_Gray		= 	7
F_DarkGray	= 	8
F_Desktop	= 	9
F_Dialog	= 	10
F_Menus		= 	11
F_TitleKey	= 	12
F_DesktopKey	= 	13
F_DialogKey	= 	14
F_MenusKey	= 	15
F_Title		=	0

S_COLOR		STRUC
cl_backgr	db 8  dup(?)
cl_foregr	db 16 dup(?)
cl_palett	db 16 dup(?)
S_COLOR		ENDS

GLOBAL	at_palett:	byte
GLOBAL	at_background:	byte
GLOBAL	at_foreground:	byte

GLOBALP	setpal,     	:word, :word
GLOBALD	resetpal
GLOBALP	loadpal,	:dword
GLOBALD	editpal
GLOBALD	editattrib

