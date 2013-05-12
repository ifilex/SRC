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
                %include "punto.inc"

segment	_TEXT
                extern   _int21_syscall:wrt TGROUP
                extern   _int25_handler:wrt TGROUP
                extern   _int26_handler:wrt TGROUP
                extern   _set_stack:wrt TGROUP
                extern   _restore_stack:wrt TGROUP
                extern   _error_tos:wrt DGROUP
                extern   _char_api_tos:wrt DGROUP
                extern   _disk_api_tos:wrt DGROUP
                extern   _lpUserStack:wrt DGROUP
                extern   _user_r:wrt DGROUP
                extern   _ErrorMode:wrt DGROUP
                extern   _InDOS:wrt DGROUP
                extern   _cu_psp:wrt DGROUP
                extern   _MachineId:wrt DGROUP
                extern   critical_sp:wrt DGROUP

                extern   _api_sp:wrt DGROUP      
                extern   _api_ss:wrt DGROUP      
                extern   _usr_sp:wrt DGROUP      
                extern   _usr_ss:wrt DGROUP
                extern   int21regs_seg:wrt DGROUP
                extern   int21regs_off:wrt DGROUP

                extern   _dosidle_flag:wrt DGROUP
                extern   _Int21AX:wrt DGROUP


                global  _cpm_entry
                global  _int20_handler
                global  _int21_handler
                global  _low_int25_handler
                global  _low_int26_handler
                global  _int27_handler


_cpm_entry:
                push    bp              
                mov     bp,sp
                xchg    bp,[2+bp]
                pop     bp
                pushf                  
                push    bp
                mov     bp,sp          
                push    ax
                mov     ax,[2+bp]       
                xchg    ax,[6+bp]      
                mov     [2+bp],ax
                pop     ax             
                pop     bp
                cmp     cl,024h
                jbe     cpm_error
                mov     ah,cl          
                jmp     short _int21_handler   
cpm_error:      mov     al,0
                iret

_RestartSysCall:
                cli                     
                mov     bp,word [_lpUserStack+2] 
                mov     ss,bp
                mov     bp,word [_lpUserStack]
                mov     sp,bp
                sti
                POP$ALL                
                jmp     short int21_reentry    


_int20_handler:
                mov     ah,0            ; termina la int 21        


_int21_handler:
                PUSH$ALL

                mov     bp,DGROUP
                mov     ds,bp

                mov     bp,ss
                mov     word [_lpUserStack+2],bp
                mov     word [_user_r+2],bp
                mov     bp,sp
                mov     word [_lpUserStack],bp        ; inicio        
                mov     word [_user_r],bp     ; inicio         


int21_reentry:
                cmp     ah,33h
                je      int21_user
                cmp     ah,50h
                je      int21_user
                cmp     ah,51h
                je      int21_user
                cmp     ah,62h
                jne     int21_1

int21_user:     push    word [_user_r+2]
                push    word [_user_r]
                call    _int21_syscall
                pop     cx
                pop     cx
                jmp     int21_ret

int21_1:        sti
                cmp     byte [_ErrorMode],0
                je      int21_2
                mov     bp,ds
                mov     ss,bp
                mov     bp,_error_tos
                mov     sp,bp
                cli
                push    word [_user_r+2]
                push    word [_user_r]
                call    _int21_syscall
                jmp     short int21_exit

int21_2:        inc     byte [_InDOS]
                cmp     ah,0ch
                jg      int21_3
                cmp     byte [_dosidle_flag],0
                jne     int21_user

                mov     bp,ds
                mov     ss,bp
                mov     bp,_char_api_tos
                mov     sp,bp
                cli
                push    word [_user_r+2]
                push    word [_user_r]
                call    _int21_syscall
                jmp     short int21_exit

int21_3:
                call    dos_crit_sect

                mov     bp,ds
                mov     ss,bp
                mov     bp,_disk_api_tos
                mov     sp,bp
                cli
                push    word [_user_r+2]
                push    word [_user_r]
                call    _int21_syscall

int21_exit:     sti
                mov     bp,word [_user_r+2]
                mov     ss,bp
                mov     bp,word [_user_r]     ; inicio y salida
                mov     sp,bp
                dec     byte [_InDOS]
                cli
int21_ret:      POP$ALL

                iret
dos_crit_sect:
                mov     [_Int21AX],ax       
                push    ax                
                mov     ah,82h             
                int     2ah                
                pop     ax
                ret

_int27_handler:
                add     dx,byte 0fh     
                rcr     dx,1
                shr     dx,1
                shr     dx,1
                shr     dx,1
                mov     ax,3100h
                jmp     _int21_handler  

stkframe        dd      0

_low_int25_handler:
                sti
                pushf
                push    ax
                push    cx
                push    dx
                push    bx
                push    sp
                push    bp
                push    si
                push    di
                push    ds
                push    es

                mov     word [cs:stkframe], sp     
                mov     word [cs:stkframe+2], ss

                cld
                mov     ax, DGROUP
                mov     ds, ax

                mov     word [_api_sp], _disk_api_tos
                mov     word [_api_ss], ds

                call    far _set_stack

                push    word [cs:stkframe+2]
                push    word [cs:stkframe]
                call    _int25_handler
                add     sp, byte 4

                call    far _restore_stack

                pop     es
                pop     ds
                pop     di
                pop     si
                pop     bp
                pop     bx     
                pop     bx
                pop     dx
                pop     cx
                pop     ax
                popf
                retf            


_low_int26_handler:
                sti
                pushf
                push    ax
                push    cx
                push    dx
                push    bx
                push    sp
                push    bp
                push    si
                push    di
                push    ds
                push    es

                mov     word [cs:stkframe], sp    
                mov     word [cs:stkframe+2], ss

                cld
                mov     ax, DGROUP
                mov     ds, ax

                mov     word [_api_sp], _disk_api_tos
                mov     word [_api_ss], ds

                call    far _set_stack

                push    word [cs:stkframe+2]
                push    word [cs:stkframe]
                call    _int26_handler
                add     sp, 4

                call    far _restore_stack

                pop     es
                pop     ds
                pop     di
                pop     si
                pop     bp
                pop     bx      
                pop     bx
                pop     dx
                pop     cx
                pop     ax
                popf
                retf


CONTINUE        equ     00h
RETRY           equ     01h
ABORT           equ     02h
FAIL            equ     03h

OK_IGNORE       equ     20h
OK_RETRY        equ     10h
OK_FAIL         equ     08h

PSP_PARENT      equ     16h
PSP_USERSP      equ     2eh
PSP_USERSS      equ     30h


                global  _int24_handler
_int24_handler: mov     al,FAIL
                iret

                global  _CriticalError
_CriticalError:
                cmp     byte [_ErrorMode],0
                je      CritErr05              

                mov     ax,FAIL
                retn
CritErr05:
                push    bp
                mov     bp,sp
                push    si
                push    di
                mov     ah,byte [bp+4]      ; nFlags
                mov     al,byte [bp+6]      ; nDrive
                mov     di,word [bp+8]      ; nError
                mov     si,word [bp+10]     ; lpDevice Offset
                mov     bp,word [bp+12]     ; lpDevice segment
                cli
                mov     es,[_cu_psp]
                push    word [es:PSP_USERSS]
                push    word [es:PSP_USERSP]
                push    word [_MachineId]
                push    word [int21regs_seg]
                push    word [int21regs_off]
                push    word [_api_sp]
                push    word [_api_ss]
                push    word [_usr_sp]
                push    word [_usr_ss]
                push    word [_user_r+2]
                push    word [_user_r]
                mov     [critical_sp],sp
                inc     byte [_ErrorMode]
                dec     byte [_InDOS]
                mov     ss,[es:PSP_USERSS]
                mov     sp,[es:PSP_USERSP]
                int     24h                    

                cld
                cli
                mov     bp, DGROUP
                mov     ds,bp
                mov     ss,bp
                mov     sp,[critical_sp]
                pop     word [_user_r]
                pop     word [_user_r+2]
                pop     word [_usr_ss]
                pop     word [_usr_sp]
                pop     word [_api_ss]
                pop     word [_api_sp]
                pop     word [int21regs_off]
                pop     word [int21regs_seg]
                pop     word [_MachineId]
                mov     es,[_cu_psp]
                pop     word [es:PSP_USERSP]
                pop     word [es:PSP_USERSS]
                sti                            
                mov     byte [_ErrorMode],0
                inc     byte [_InDOS]
                cmp     al,CONTINUE
                jne     CritErr10             
                test    bh,OK_IGNORE
                jnz     CritErr10
                mov     al,FAIL
CritErr10:
                cmp     al,RETRY
                jne     CritErr20              
                test    bh,OK_RETRY
                jnz     CritErr20
                mov     al,FAIL
CritErr20:
                cmp     al,FAIL
                jne     CritErr30              
                test    bh,OK_FAIL
                jnz     CritErr30
                mov     al,ABORT
CritErr30:
                cmp     al,ABORT
                je      CritErrAbort           

CritErrExit:
                xor     ah,ah                 
                pop     di
                pop     si
                pop     bp
                ret

CritErrAbort:
                mov     ax,[_cu_psp]
                mov     es,ax
                cmp     ax,[es:PSP_PARENT]
                mov     al,FAIL
                jz      CritErrExit
                cli
                mov     bp,word [_user_r+2]  
                mov     ss,bp
                mov     es,bp
                mov     bp,word [_user_r]
                mov     sp,bp
                mov     byte [_ErrorMode],1      
                mov     ax,4C00h
                mov     [es:reg_ax],ax
                sti
                jmp     int21_reentry        
