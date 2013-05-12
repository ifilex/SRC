nalloc		proto _CType :word
nfree		proto _CType :word
palloc		proto _CType
malloc		proto _CType :word
free		proto _CType :dword
coreleft	proto _CType

EMMPAGE	=	4000h

emminit		proto _CType		; ax 1 : 0
emmversion	proto _CType		; ax version : 0
emmcoreleft	proto _CType		; dx:ax core left : NULL
emmnumfreep	proto _CType		; ax unallocated page count : 0
emmalloc	proto _CType :word	; (pages) ax handle : -1
emmfree		proto _CType :word	; (handle) ax 0 : ?
emmread		proto _CType :dword, :word, :word ; (void *, handle, page) ax 0 : ?
emmwrite	proto _CType :dword, :word, :word ; (void *, handle, page) ax 0 : ?
