S_RECT		STRUC
rc_x		db ?
rc_y		db ?
rc_col		db ?
rc_row		db ?
S_RECT		ENDS

GLOBALN	@rcsprc

GLOBALP	rcalloc,	:dword, :word
GLOBALP	rcbprc,		:dword, :dword, :word
GLOBALP	rcclose,	:dword, :word, :dword
GLOBALP	rcclrshade,	:dword, :dword
GLOBALP	rcframe,	:dword, :dword, :word, :word
GLOBALP	rchide,		:dword, :word, :dword
GLOBALP	rcinside,	:dword, :dword
GLOBALP	rcmemsize,	:dword, :word
GLOBALP	rcopen,		:dword, :word, :word, :dword, :dword
GLOBALP	rcread,		:dword, :dword
GLOBALP	rcsetshade,	:dword, :dword
GLOBALP	rcshow,		:dword, :word, :dword
GLOBALP	rcsprc,		:dword
GLOBALP	rcsprcrc,	:dword, :dword
GLOBALP	rcwrite,	:dword, :dword
GLOBALP	rcxchg,		:dword, :dword
GLOBALP	rcxyrow,	:dword, :word, :word
GLOBALP	rcmoveup,	:dword, :dword, :word
GLOBALP	rcmovedn,	:dword, :dword, :word
GLOBALP	rcmoveleft,	:dword, :dword, :word
GLOBALP	rcmoveright,	:dword, :dword, :word

ifdef __WCC__
 GLOBALP rcaddrc,	:dword, :dword, :word
 GLOBALP rcmove,	:dword, :dword, :word, :word, :word, :word
 GLOBALP rcmsmove,	:dword, :dword, :word, :word
else
 GLOBALP rcaddrc,	:dword, :dword
 GLOBALP rcmove,	:dword, :dword, :word, :word, :word
 GLOBALP rcmsmove,	:dword, :dword, :word
endif

