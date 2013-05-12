MOUSECMD = -1
GLOBALD	msloop
ifdef __MOUSE__
MOUSEINT = 33h
GLOBALD	mousep
GLOBALD	mousey
GLOBALD	mousex
GLOBALD	mouseon
GLOBALD	mouseoff
GLOBALD	mouseinit
GLOBALD	mousehide
GLOBALD	mouseshow
GLOBALD	mouseget
GLOBALD	mouseset
GLOBALP	mousewait, :WORD, :WORD, :WORD
endif
