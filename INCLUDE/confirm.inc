CFREADONY	=	0001h
CFSYSTEM	=	0002h
CFDIRECTORY	=	0004h
CFSELECTED	=	0008h
CFDELETEALL	=	0010h

GLOBAL 	confirmflag:	WORD

GLOBALP confirm_continue, 	:DWORD
GLOBALP confirm_delete,		:DWORD, :WORD
GLOBALP confirm_delete_file, 	:DWORD, :WORD
GLOBALP confirm_delete_sub,	:DWORD


