DZ		= 0F1h
DLL		= 0F2h

; DLL interrupt functions (AH=XX)

_DLL_LOAD	= 00h
_DLL_READ	= 01h
_DLL_COPY	= 02h
_DLL_ADD	= 03h
_DLL_MOVE	= 04h
_DLL_MKDIR	= 05h
_DLL_DELETE	= 06h
_DLL_RENAME	= 07h
_DLL_EDIT	= 08h
_DLL_VIEW	= 09h
_DLL_PROPERTY	= 0Ah
_DLL_ENTER	= 0Bh
_DLL_EXIT	= 11h

; Flag bits for functions

__DLL_01	= 0001h	; dll_read(wsub:dword)
__DLL_02	= 0002h	; dll_copy(wsub:dword, fblk:dword, outpath:dword)
__DLL_03	= 0004h	; dll_add(wsub:dword, wsub:dword, fblk:dword)
__DLL_04	= 0008h ; dll_move(wsub:dword, fblk:dword, wsub:dword)
__DLL_05	= 0010h ; dll_mkdir(wsub:dword, directory:dword)
__DLL_06	= 0020h ; dll_delete(wsub:dword, fblk:dword)
__DLL_07	= 0040h ; dll_rename(wsub:dword, fblk:dword, newname:dword)
__DLL_08	= 0080h ; dll_edit(wsub:dword, fblk:dword)
__DLL_09	= 0100h ; dll_view(wsub:dword, fblk:dword)
__DLL_0A	= 0200h ; dll_property(wsub:dword, fblk:dword)
__DLL_0B	= 0400h ; dll_enter(wsub:dword, fblk:dword)

; Doszip interrupt functions

SPAWN_EXIT	equ	0000B
SPAWN_INLINE	equ	0001B
SPAWN_HIDE	equ	0011B	; Hide doszip
SPAWN_STDOUT	equ	0101B	; > %TEMP%\ziplst.tmp
SPAWN_NUL	equ	1101B	; > NUL

_DZ_dosmaperr	= 00h
_DZ_getsyserr	= 01h
_DZ_malloc	= 02h
_DZ_free	= 03h
_DZ_rsopen	= 04h
_DZ_rsmodal	= 05h
_DZ_view	= 06h
_DZ_edit	= 07h
_DZ_output	= 08h
_DZ_command	= 09h
_DZ_inientryid	= 0Ah
_DZ_tmsgbox	= 0Bh
_DZ_mklist	= 0Ch
_DZ_open	= 0Dh
_DZ_close	= 0Eh
;_DZ_getline	= 0Fh
;_DZ_dlinit	= 10h
;_DZ_dlshow	= 11h
;_DZ_dlhide	= 12h
;_DZ_dlevent	= 13h
;_DZ_dlclose	= 14h
;_DZ_prg_open	= 15h
;_DZ_prg_close	= 16h
;_DZ_prg_set	= 17h
;_DZ_prg_update	= 18h
;_DZ_prg_size	= 19h
;_DZ_filter_fblk	= 1Ah
;_DZ_filter_wblk	= 1Bh
;_DZ_ogetouth	= 1Ch
;_DZ_confirmdf	= 1Dh
;_DZ_confirmds	= 1Eh
;_DZ_preread	= 1Fh
;_DZ_predraw	= 20h
;_DZ_cpdeselect	= 21h

