S_WCHAR		STRUC
wc_ch		db ?
wc_at		db ?
S_WCHAR		ENDS

GLOBALD	@wcenter
GLOBALD	@wcputa
GLOBALD	@wcputbg
GLOBALD	@wcputfg
GLOBALD	@wcputc
GLOBALD	@wcputw
GLOBALD	@wctitle
GLOBALD	@wcputs
GLOBALD	@wcputf

GLOBALN	@wgetxyp
GLOBALN	@wcputxg
GLOBALN	@wcpath
GLOBALN	@wcputnum
GLOBALN	@putesbxal

GLOBALP	wcunzip,	:dword, :dword, :word
GLOBALP	wcpbutt,	:dword, :word, :word, :dword
GLOBALP	wcpushst,	:dword, :dword
GLOBALP	wcpopst,	:dword


