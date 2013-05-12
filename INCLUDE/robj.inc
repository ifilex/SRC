S_ROBJ		STRUC
rs_memsize	dw ?
rs_flag		dw ?
rs_count	db ?
rs_index	db ?
rs_rect		dd ?
S_ROBJ		ENDS

GLOBALN	@rsunzipat
GLOBALN	@rsunzipch

GLOBALP	rsopen,	   	:dword
GLOBALP	rsevent,   	:dword, :dword
GLOBALP	rsmodal,   	:dword
GLOBALP	rsreload,  	:dword, :dword
