;                    Copyright (c) 1995, 1998
;                       Pasquale J. Villani
;                       All Rights Reserved
;
; This file is part of DOS-C.
;
; DOS-C is free software; you can redistribute it and/or
; modify it under the terms of the GNU General Public License
; as published by the Free Software Foundation; either version
; 2, or (at your option) any later version.
;
; DOS-C is distributed in the hope that it will be useful, but
; WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
; the GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public
; License along with DOS-C; see the file COPYING.  If not,
; write to the Free Software Foundation, 675 Mass Ave,
; Cambridge, MA 02139, USA.
;
; Revision 1.5  2012/05/08 04:29:59  jimtabor
;
; Revision 1.2  2000/05/08 04:29:59  jimtabor
; Update CVS to 2020
;
; Revision 1.1.1.1  2000/05/06 19:34:53  jhall1
; The FreeDOS Kernel.  A DOS kernel that aims to be 100% compatible with
; MS-DOS.  Distributed under the GNU GPL.
;
; Revision 1.2  1999/08/10 17:57:12  jprice
; ror4 2011-02 patch
;
; Revision 1.1.1.1  1999/03/29 15:40:38  jprice
; New version without IPL.SYS
;
; Revision 1.4  1999/02/08 05:55:57  jprice
; Added Pat's 1937 kernel patches
;
; Revision 1.3  1999/02/01 01:48:41  jprice
; Clean up; Now you can use hex numbers in config.sys. added config.sys screen function to change screen mode (28 or 43/50 lines)
;
; Revision 1.2  1999/01/22 04:13:25  jprice
; Formating
;
; Revision 1.1.1.1  1999/01/20 05:51:01  jprice
    ; Imported sources
;
;   Rev 1.4   06 Dec 1998  8:47:02   patv
;Expanded due to new I/O subsystem.
;
;   Rev 1.3   16 Jan 1997 12:46:44   patv
;pre-Release 0.92 feature additions
;
;   Rev 1.2   29 May 1996 21:03:38   patv
;bug fixes for v0.91a
;
;   Rev 1.1   01 Sep 1995 17:54:26   patv
;First GPL release.
;
;   Rev 1.0   02 Jul 1995  9:04:50   patv
;Initial revision.
; $EndLog$

                %include "direc.inc"


segment	_TEXT

                extern  _ReqPktPtr:wrt TGROUP

STACK_SIZE      equ     384/2          

..start:
entry:		jmp	far kernel_start

segment	INIT_TEXT

		extern	_main:wrt IGROUP

kernel_start:
		mov	ax,DGROUP
		cli
		mov	ss,ax
		mov	sp,tos
                int     12h            
		mov	cl,6
		shl	ax,cl
		mov	dx,init_end+15
		mov	cl,4
		shr	dx,cl
		sub	ax,dx
		mov	es,ax
		mov	ax,cs
		mov	ds,ax
		xor	si,si
		xor	di,di
		mov	cx,init_end+1
		shr	cx,1
		cld
		rep	movsw
		push	es
		mov	ax,cont
		push	ax
		retf
cont:          
                mov     di,seg apistk_bottom
                mov     es,di
                mov     di,apistk_bottom
                mov     ax,apistk_top
                sub     ax,di
                sar     ax,1
                mov     cx,ax
                mov     ax,09090h
                cld
                rep     stosw
               
                mov     ax,ss
                mov     ds,ax
                mov     es,ax
                mov     bp,sp         
                sti                    
		inc	bl
		jns	floppy
		add	bl,3-1-128
floppy:         mov     byte [_BootDrive],bl 
		int	11h
		mov	cl,6
		shr	al,cl
		inc	al
                mov     byte [_NumFloppies],al 

                mov     ax,ds
                mov     es,ax
		jmp	_main

segment	INIT_TEXT_END
init_end:

segment	_TEXT

		global	_nul_strtgy
_nul_strtgy:
                mov     word [cs:_ReqPktPtr],bx    
                mov     word [cs:_ReqPktPtr+2],es
                retf

		global	_nul_intr
_nul_intr:
                push    es
                push    bx
                les     bx,[cs:_ReqPktPtr]            
                or      word [es:bx+3],100h 
                pop     bx
                pop     es
                retf

		extern	_init_call_printf:wrt TGROUP

		global	_printf

_printf:
		pop	ax
		push	cs
		push	ax
		jmp	_init_call_printf


segment	_FIXED_DATA

                global  DATASTART
DATASTART:
dos_data        db      0
                dw      kernel_start
                db      0              
                dw      1              

                times (0eh - ($ - DATASTART)) db 0
                global  _NetBios
_NetBios        db      0              
                global  _Num_Name
_Num_Name       db      0



                times (26h - 0ch - ($ - DATASTART)) db 0

                global  _NetRetry
_NetRetry       dw      3              
                global  _NetDelay
_NetDelay       dw      1             
                global  _DskBuffer
_DskBuffer      dd      -1             
                dw      0              
                global  _first_mcb
_first_mcb      dw      0              
                global  _DPBp
                global  MARK0026H
MARK0026H       equ     $
_DPBp           dd      0               
                global  _sfthead
_sfthead        dd      0              
                global  _clock
_clock          dd      0              
                global  _syscon
_syscon         dd      0              
                global  _maxbksize
_maxbksize      dw      0              
                global  _firstbuf
_firstbuf       dd      0              
                global  _CDSp
_CDSp           dd      0              
                global  _FCBp
_FCBp           dd      0               
                global  _nprotfcb
_nprotfcb       dw      0              
                global  _nblkdev
_nblkdev        db      0               
                global  _lastdrive
_lastdrive      db      0              
                global  _nul_dev
_nul_dev:           
                dd      -1
                dw      8004h           
                dw      _nul_strtgy
                dw      _nul_intr
                db      'NUL     '
                global  _njoined
_njoined        db      0             
                dw      0             
setverPtr       dw      0,0           
                dw      0              
                dw      0         
                dw      1             
                dw      1              
                global  _BootDrive
_BootDrive      dw      0              
                db      0            
                dw      0            
buf_info        dd      0          
                dw      0           
                dd      0            
                dw      0            
                db      0             
                dw      0             
deblock_seg     dw      0          
                times 3 db 0      
                dw      0             
                db      0, 0FFh, 0    
                db      0              
                dw      0             
dmd_upper_link  db      0             
                dw      0             
dmd_upper_root  dw      0FFFFh        
                dw      0             
SysVarEnd:


                times (01fbh - (SysVarEnd - DATASTART)) db 0

                global  MARK01FBH
MARK01FBH       equ     $
                times 128 db 0
                global  _kb_buf
_kb_buf db      129,0                   
                times 128+1 db 0  
                global  _PrinterEcho
_PrinterEcho    db      0              
                global  _verify_ena
_verify_ena     db      0             

; this byte is used for TAB's
                global _scr_pos
_scr_pos        db      0              
                global  _switchar
_switchar       db      '/'           
                global  _mem_access_mode
_mem_access_mode db     0              
                global  sharing_flag
sharing_flag    db      0              
                                       
                                       
                                       
                                       
                                       
                                       
                global  _net_set_count
_net_set_count   db      1              
                global  _net_name
_net_name       db      '               ' 
                db      00               


                global  _ErrorMode, _InDOS
                global  _CritErrLocus, _CritErrCode
                global  _CritErrAction, _CritErrClass
                global  _CritErrDev, _CritErrDrive
                global  _dta
                global  _cu_psp, _default_drive
                global  _break_ena
                global  _return_code, _return_mode
                global  _internal_data

                global  _CritPatch
_CritPatch      dw      0d0ch          
                dw      0d0ch           
                dw      0d0ch
                dw      0d0ch
                dw      0d0ch
                db      0             
_internal_data:            
_ErrorMode      db      0             
_InDOS          db      0            
_CritErrDrive   db      0            
_CritErrLocus   db      0             
_CritErrCode    dw      0              
_CritErrAction  db      0             
_CritErrClass   db      0              
_CritErrDev     dd      0           
_dta            dd      0           
_cu_psp         dw      0             
break_sp        dw      0              
_return_code    db      0              
_return_mode    db      0               
_default_drive  db      0             
_break_ena      db      0              
                db      0             
                db      0              

                global  _swap_always, _swap_indos
_swap_always:

                global  _Int21AX
_Int21AX        dw      0               

                global  owning_psp, _MachineId
owning_psp      dw      0         
_MachineId      dw      0             
                dw      0           
                dw      0            
                dw      0         
                dw      0           
                dw      0      
                db      0            
                db      0          
                db      0           
                global  _break_flg
_break_flg      db      0             
                db      0             
                db      0            
                global  _DayOfMonth
_DayOfMonth     db      1              
                global  _Month
_Month          db      1              
                global  _YearsSince1980
_YearsSince1980 dw      0              
daysSince1980   dw      0FFFFh       
                                     
                global  _DayOfWeek
_DayOfWeek      db      2             
                global  _Year
_Year           dw      1980           
                global  _dosidle_flag
_dosidle_flag   db      0             
                global  _CharReqHdr
_CharReqHdr:
                global  _ClkReqHdr
_ClkReqHdr      times 30 db 0    
                dd      0             
                global  _MediaReqHdr
_MediaReqHdr    times 22 db 0      
                global  _IoReqHdr
_IoReqHdr       times 30 db 0    
                times 6 db 0    
                global  _ClkRecord
_ClkRecord      times 6 db 0      
                dw      0             
                global  __PriPathBuffer
__PriPathBuffer times 80h db 0     
                global  __SecPathBuffer
__SecPathBuffer times 80h db 0    
                global  _TempBuffer
_TempBuffer     times 21 db 0      
                global  _SearchDir
_SearchDir      times 32 db 0     
                global  _TempCDS
_TempCDS        times 88 db 0      
                global  _DirEntBuffer
_DirEntBuffer   times 32 db 0      
                global  _wAttr
_wAttr          dw      0            


              
                times (250h - ($ - _internal_data)) db 0
                global  _lpUserStack
_lpUserStack    dd      0               

          
                times (25ch - ($ - _internal_data)) db 0

                global  _tsr          
_tsr            db      0            
                db      0              
                global  term_psp
term_psp        dw  0                 
                global  int24_esbp
int24_esbp      times 2 dw 0      
                global  _user_r, int21regs_off, int21regs_seg
_user_r:
int21regs_off   dw      0             
int21regs_seg   dw      0
                global  critical_sp
critical_sp     dw      0              
                global  current_ddsc
current_ddsc    times 2 dw 0

         
                times (27ah - ($ - _internal_data)) db 0
                global  current_device
current_device  times 2 dw 0      
                global  _lpCurSft
_lpCurSft       times 2 dw 0      
                global  current_ldt
current_ldt     times 2 dw 0      
                global  _lpFcb
_lpFcb          times 2 dw 0      
                global  current_ifn
current_ifn     dw      0               

             
                times (2aeh - ($ - _internal_data)) db 0
                global  current_filepos
current_filepos times 2 dw 0      

              
                times (2d0h - ($ - _internal_data)) db 0
                global  _prev_user_r
                global  prev_int21regs_off
                global  prev_int21regs_seg
_prev_user_r:
prev_int21regs_off      dw      0      
prev_int21regs_seg      dw      0

              
                times (300h - ($ - _internal_data)) db 0

                global  _FcbSearchBuffer       
_FcbSearchBuffer:          

                global  _LocalPath
_LocalPath:

apistk_bottom:
                times STACK_SIZE dw 0     
                global  _error_tos
_error_tos:
                times STACK_SIZE dw 0    
                global  _disk_api_tos
_disk_api_tos:
                times STACK_SIZE dw 0      
                global  _char_api_tos
_char_api_tos:
apistk_top:

_VolChange      db      0              
_VirtOpen       db      0              

                times (78ch - ($ - _internal_data)) db 0
_swap_indos:

segment	_BSS
                global  _NumFloppies
_NumFloppies	resw	1
intr_dos_stk	resw	1
intr_dos_seg	resw	1


                global  _api_sp
_api_sp         dw      0               
                global  _api_ss
_api_ss         dw      0               
                global  _usr_sp
_usr_sp         dw      0             
                global  _usr_ss
_usr_ss         dw      0
                global  _ram_top
_ram_top        dw      0




segment	_BSSEND
                global  blk_stk_top
                times 256 dw 0
blk_stk_top:

                global  clk_stk_top
                times 256 dw 0
clk_stk_top:

                times 256 dw 0
intr_stk_top:

                times 128 dw 0
tos:

                global  last
last:                    
                global  _last
_last:                   



segment	_STACK	class=STACK stack
