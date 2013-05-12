include	clib.inc
include dir.inc
include fblk.inc
include wsub.inc
include filter.inc
include	version.inc
include	doszip/dzidd.inc
include plugins/plugins.inc
include tview/tview.inc
ifdef __ZIP__
 __ARC__ = 1
else
ifdef __DLL__
 __ARC__ = 1
endif
endif

; Configuration file:	DZ.CFG

_C_PANELID	= 0001h	; config.c_lflag
_C_HORIZONTAL	= 0002h
_C_ESCUSERSCR	= 0004h
_C_EGALINE	= 0008h
_C_SELECTDIR	= 0010h
_C_INSMOVDN	= 0020h
_C_CDCLRDONLY	= 0040h
_C_AUTOSAVE	= 0080h
_C_SORTDIR	= 0100h
_C_WIDEVIEW	= 0200h	;*
_C_SAVEHISTORY	= 0400h
_C_DELTEMP	= 0800h
_C_MENUSLINE	= 1000h
_C_STATUSLINE	= 2000h
_C_COMMANDLINE	= 4000h
_C_DEFAULT	= 44B0h

_C_CONFCOPY	= 01h	; config.c_confirm
_C_CONFMOVE	= 02h
_C_CONFDELETE	= 04h
_C_CONFDELSUB	= 08h
_C_CONFSYSTEM	= 10h
_C_CONFRDONLY	= 20h
_C_CONFEXIT	= 40h

_C_ZINCSUBDIR	= 01h	; config.c_compress

S_CONFIG	STRUC
c_version	dw ?	; hex: 0.1.3.8 --> 0138h
c_lflag		dw ?
c_confirm	db ?
c_console	db ?	; -- console.inc
c_exconsole	db ?	; -- extended
c_fsflag	db ?	; -- iost.inc
c_tvflag	db ?	; -- tview.asm
c_teflag	dw ?	; -- tedit.asm
c_tepages	dw ?	; -- tedit.asm Max EMS pages to alloc
c_telsize	dw ?	; -- tedit.asm Max line length
c_compress	db ?	; -- inline .ZIP
c_ffflag	db ?	; -- ff.asm
c_comprlevel	db ?
c_panelsize	db ?	; Alt-Up/Down
c_flaga		dw ?
c_fcb_indexa	dw ?
c_cel_indexa	dw ?
c_flagb		dw ?
c_fcb_indexb	dw ?
c_cel_indexb	dw ?
c_apath		db SIZE S_PATH dup(?)
c_bpath		db SIZE S_PATH dup(?)
c_filter	db SIZE S_FILT dup(?)
c_color		db 40 dup(?)
c_pending_file	db 260 dup(?)
S_CONFIG 	ENDS

S_XCELL		STRUC
xl_flag		dw ?	; DOBJ.dl_flag
xl_cols		db ?	; number of files in one line
xl_rows		db ?	; number of lines in panel
xl_rect		dd ?	; the current position of cell
xl_bp		dd ?	; DOBJ.dl_wp
xl_cpos		dd ?	; position of first cell in panel
S_XCELL		ENDS

_P_PANELID	= 0001h
_P_VISIBLE	= 0002h
_P_MINISTATUS	= 0004h
_P_WIDEVIEW	= 0008h
_P_DETAIL	= 0010h
_P_DRVINFO	= 0020h

S_PANEL		STRUC
pn_path		dw ?
pn_flag		dw ?
pn_fcb_count	dw ?
pn_cel_count	dw ?
pn_fcb_index	dw ?
pn_cel_index	dw ?
pn_xl		dd ?
pn_dialog	dd ?
pn_wsub		dd ?
pn_putfcb	dw ?
S_PANEL		ENDS

S_MKLST		STRUC
mkl_macro	db ?	; use output macro
mkl_append	db ?	; add to list or create new
mkl_unix	db ?	; convert to unix
mkl_excl_cd	db ?	; use local directory
mkl_excl_drv	db ?	; use drive in directory
mkl_mask	db ?	; add mask to directory\[*.*]
mkl_offspath	dw ?	; length of local directory
mkl_offset	dd ?	; search offset from findfile
mkl_handle	dw ?
mkl_count	dd ?	; total file count in list
S_MKLST		ENDS

S_DZPATH	STRUC
dzp_dir		db MAXPATH dup(?)
dzp_flag	dw ?
dzp_bufid	dw ?
dzp_celid	dw ?
S_DZPATH	ENDS

GLOBAL	config:		word
GLOBAL	cflag:		word
GLOBAL	flaga:		word
GLOBAL	flagb:		word
GLOBAL	fsflag:		byte
GLOBAL	tvflag:		byte
GLOBAL	teflag:		word
GLOBAL	tepages:	word
GLOBAL	telsize:	word
GLOBAL	path_a:		word
GLOBAL	path_b:		word
GLOBAL	ffflag:		byte
GLOBAL	compressflag:	byte
GLOBAL	compresslevel:	byte
GLOBAL	history:	dword
GLOBAL	opfilter:	S_FILT

GLOBAL	envconf:	dword	; %DZ%
GLOBAL	programpath:	byte	; _argv[0]/C<directory>
GLOBAL	configpath:	byte	; envconf/programpath
GLOBAL	configfile:	byte	; envconf/programpath
GLOBAL	searchstring:	byte
GLOBAL	dzexe:		dword	; DZ.EXE
GLOBAL	envconf:	dword	; %DZ%
GLOBAL	envpath:	dword	; %PATH%
GLOBAL	envtemp:	dword	; %TEMP%
GLOBAL	comspec:	dword	; %COMSPEC%

GLOBAL	cpanel:		word
GLOBAL	panela:		word
GLOBAL	panelb:		word
GLOBAL	spanela:	word
GLOBAL	spanelb:	word
GLOBAL	wspanela:	word
GLOBAL	wspanelb:	word
GLOBAL	prect_a:	word
GLOBAL	prect_b:	word
GLOBAL	mklist:		S_MKLST

getinifile		proto
getcfgfile		proto
getcmdfile		proto
config_read		proto
config_save		proto
history_open		proto
history_save		proto
history_delete		proto
history_close		proto
doskey_save		proto

prect_show		proto
prect_hide		proto
prect_close     	proto
prect_open      	proto pascal
prect_clear		proto pascal rect:dword, ptype:word
fblk_select		proto pascal fblk:dword
fblk_invert		proto pascal fblk:dword
pcell_select		proto
pcell_set		proto
pcell_open		proto
pcell_show		proto
pcell_update		proto
pcell_move		proto pascal
pcell_setxy		proto pascal panel:word, xpos:word, ypos:word

ifdef __DLL__
loadplugin		proto pascal magic:dword, archive:dword
freeplugin		proto
endif

cpanel_state		proto
cpanel_findfirst	proto
cpanel_gettarget	proto
cpanel_setpath		proto
cpanel_deselect		proto pascal fblk:dword

panel_state		proto
panel_open		proto pascal
panel_close		proto
panel_hide		proto
panel_show		proto
panel_setactive		proto
panel_curobj		proto
panel_findnext		proto
panel_openmsg		proto
panel_putinfo		proto pascal
panel_putmini		proto pascal
panel_read		proto
panel_reread		proto
panel_redraw		proto
panel_toggle		proto
panel_update		proto
panel_xormini		proto
panel_xorinfo		proto
panel_setid		proto	; panel:AX, index:DX
panel_putitem		proto pascal panel:word, index:word
panel_xycmd		proto pascal panel:word, xpos:word, ypos:word
panel_sethdd		proto pascal panel:word, hdd:word
panel_event		proto pascal panel:word, event:word
panel_stateab		proto
redraw_panels		proto

GLOBALD	cmalong
GLOBALD	cmadetail
GLOBALD	cmahidden
GLOBALD	cmamini
GLOBALD	cmavolinfo
GLOBALD	cmaname
GLOBALD	cmatype
GLOBALD	cmadate
GLOBALD	cmasize
GLOBALD	cmanosort
GLOBALD	cmatoggle
GLOBALD	cmafilter
GLOBALD	cmasubinfo
GLOBALD	cmaupdate
GLOBALD	cmachdrv

GLOBALD	cmrename
GLOBALD	cmview
GLOBALD	cmedit
GLOBALD	cmtmodal
GLOBALD	cmcopy
GLOBALD	cmmove
GLOBALD	cmmkdir
GLOBALD	cmdelete
GLOBALD	cmattrib
ifdef __FF__
GLOBALD	cmsearch
endif
GLOBALD	cmhistory
GLOBALD	cmexit

GLOBALD	cmselect
GLOBALD	cmdeselect
GLOBALD	cminvert
GLOBALD	cmquicksearch	; Ctrl-Q
GLOBALD	cmcompare

GLOBALD	cmtoggleon
GLOBALD	cmtogglehz
GLOBALD	cmtogglesz
GLOBALD	cmegaline
GLOBALD	cmswap
GLOBALD	cmconfirm
GLOBALD	cmscreen
GLOBALD	cmpanel
GLOBALD	cmcompression
GLOBALD	cmsystem
GLOBALD	cmoptions

GLOBALD	cmhelp
GLOBALD	cmabout

GLOBALD	cmblong
GLOBALD	cmbdetail
GLOBALD	cmbhidden
GLOBALD	cmbmini
GLOBALD	cmbvolinfo
GLOBALD	cmbname
GLOBALD	cmbtype
GLOBALD	cmbdate
GLOBALD	cmbsize
GLOBALD	cmbnosort
GLOBALD	cmbtoggle
GLOBALD	cmbfilter
GLOBALD	cmbsubinfo
GLOBALD	cmbupdate
GLOBALD	cmbchdrv

GLOBALD	cmawideview
GLOBALD	cmbwideview
GLOBALD	cmcwideview

GLOBALD	cmcopycell
GLOBALD	cmlastmenu
GLOBALD	cmtool0
GLOBALD	cmtool1
GLOBALD	cmtool2
GLOBALD	cmtool3
GLOBALD	cmtool4
GLOBALD	cmtool5
GLOBALD	cmtool6
GLOBALD	cmtool7
GLOBALD	cmtool8
GLOBALD	cmtool9
GLOBALD	cmpath0
GLOBALD	cmpath1
GLOBALD	cmpath2
GLOBALD	cmpath3
GLOBALD	cmpath4
GLOBALD	cmpath5
GLOBALD	cmpath6
GLOBALD	cmpath7
GLOBALD	cmpath8
GLOBALD	cmpath9
GLOBALD	cmcname
GLOBALD	cmctype
GLOBALD	cmcdate
GLOBALD	cmcsize
GLOBALD	cmuserscreen
GLOBALD	cmchidden
GLOBALD	cmclong
GLOBALD	cmcmini
GLOBALD	cmcupdate
GLOBALD	cmcdetail
GLOBALD	cmcnosort
GLOBALD	cmvolinfo
GLOBALD	cmxormenubar
GLOBALD	cmxorkeybar
GLOBALD	cmxorcmdline
GLOBALD	cmupdir
GLOBALD	cmsubdir
GLOBALD	cmctrlenter
GLOBALD	cmclrcmdl
GLOBALD	cmpsizeup
GLOBALD	cmpsizedn
GLOBALD	cmpathleft      ; Alt-Left  - Previous Directory
GLOBALD	cmpathright	; Alt-Right - Next Directory
GLOBALD	cmdoskey_up	; Alt-Up    - Previous Command
GLOBALD	cmdoskey_dn	; Alt-Down  - Next Command
GLOBALD	cmsavesetup	; Shift-F10
GLOBALD	cmsubsize	; F3
GLOBALD	cmsubinfo	; Menus
GLOBALD	cmquit		; Alt-X
GLOBALD	clrcmdl		; Esc
GLOBALD	cmfilter
GLOBALD	cmmklist	; Alt-L
GLOBALD	cmloadpath	; Alt-P
ifdef __CAL__
 GLOBALP cmcalendar	; Ctrl-G
endif
ifdef __ARC__
 GLOBALD cmmkzip	; Shift-F7
endif
ifdef DEBUG
 GLOBALD cmdebug	; Shift-F1
endif
GLOBALD	cmmemory	; Alt-M
GLOBALD	cmhomedir

setconfirmflag		proto

GLOBAL	mainswitch:	word
GLOBAL	dzexitcode:	word
GLOBAL	global_key:	word
GLOBAL	com_base: 	byte
GLOBAL	com_wsub:	word

cominit		proto pascal :dword
comshow		proto
comhide		proto
comevent	proto pascal :word

GLOBAL	convbuf: 	byte
GLOBAL	cp_dzcmd:	byte

command 	proto pascal :dword
expand_environ	proto pascal :dword
fbinitype	proto pascal :dword, :dword
inicommand 	proto dist pascal :dword, :dword, :dword

load_tedit	proto pascal :dword, :word
load_tview	proto pascal :dword, :word
loadiniproc     proto pascal :dword, :dword, :word
cm_loadfblk	proto

GLOBAL	cp_copy:	byte
GLOBAL	cp_space:	byte
GLOBAL	cp_missingTEMP:	byte

GLOBAL	numfblock:	word

GLOBAL	DLG_Commandline:dword
GLOBAL	DLG_Statusline:	dword
GLOBAL	DLG_Menusline:	dword

GLOBAL	cp_tools:	byte

open_idd		proto
modal_idd		proto
close_idd		proto
tools_idd		proto pascal :word, :dword, :dword
statusline_xy		proto
prect_open_ab		proto
panel_open_ab		proto
apiopen			proto
apiclose		proto
apiupdate		proto

GLOBAL	cp_search:	byte

GLOBALD	notsup
GLOBALD	cmfilter_load
GLOBALD	view_readme

GLOBAL	cp_compare:	byte
GLOBAL	cp_copyselected:byte
GLOBAL	cp_delete:	byte
GLOBAL	cp_ziplst:	byte

ret_update_AB		proto
GLOBALD	mkziplst
GLOBALD	mkziplst_open

GLOBAL	cp_directory:	byte

_COPY_SELECTED	= 01h	; copy selected files
_COPY_IARCHIVE	= 02h	; source is archive
_COPY_OARCHIVE	= 04h	; target is archive
_COPY_IEXTFILE	= 08h	; source is .DLL archive - %doszip%/dll/.dll
_COPY_IZIPFILE	= 10h	; source is .ZIP archive
_COPY_OZIPFILE	= 20h	; target is .ZIP archive
_COPY_OEXTFILE	= 40h	; target is .DLL archive
_COPY_RECURSIV	= 80h	; recursive error

GLOBAL	cp_move:	byte
GLOBAL	copy_jump:	word
GLOBAL	copy_flag:	byte

fp_copydirectory	proto dist pascal :dword
fp_copyfile		proto dist pascal :dword, :dword
copyfile 		proto pascal :dword, :word, :word, :word

GLOBAL	cp_select:	byte
GLOBAL	cp_deselect:	byte

GLOBALP	removetemp,	:dword

ifdef 	LPROG
_DZIP	SEGMENT word USE16 public 'CODE'
_DZIP	ENDS
	ASSUME cs:_DZIP, ds:DGROUP
else
_DZIP	equ 	<_TEXT>
endif


