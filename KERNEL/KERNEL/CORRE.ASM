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

                extern  _api_sp:wrt DGROUP      
                extern  _api_ss:wrt DGROUP     
                extern  _usr_sp:wrt DGROUP     
                extern  _usr_ss:wrt DGROUP

                extern  _kstackp:wrt TGROUP    
                extern  _ustackp:wrt TGROUP    

                extern  _break_flg:wrt DGROUP  
                extern  _int21_handler:wrt TGROUP 

                %include "punto.inc"

segment	_TEXT

                global  _exec_user
_exec_user:

                PUSH$ALL
                mov     ax,DGROUP
                mov     ds,ax
                mov     bp,sp
                cld
                cli
                mov     ax,word [bp+irp_low]        
                mov     dx,word [bp+irp_hi]
                mov     sp,ax                  
                mov     ss,dx
                sti
                POP$ALL
                iret




                global  _got_cbreak
_got_cbreak:
	push ds
	push ax
	mov ax, 40h
	mov ds, ax
        or byte [71h], 80h      
	pop ax
	pop ds
	iret


                global  _spawn_int23
_spawn_int23:

                                mov ax, DGROUP          
				mov ds, ax

                cli                                     
                mov     ss,[_usr_ss]
                mov     sp,[_usr_sp]
                sti

                POP$ALL


                sub sp, byte 8          
                push ss                
                push bp                 
  		mov bp, sp
                add bp, byte 4         
                mov word [bp], 23cdh        
                mov byte [bp+2], 9ah                  
  		mov word [bp+3], ??regain_control_int23
  		mov word [bp+5], cs

  		xchg word [bp-4], bp

                stc                   
                retf

??regain_control_int23:


				push bp
				mov bp, sp
                                mov bp, [bp+2]         
                                mov word [bp-3], ax            
                                pop ax                         
                                mov word [bp-1], ax            
				mov ax, bp
                                dec ax                 
                                dec ax                
				dec ax
                                xchg ax, sp          
                                pushf                  
				add ax, byte 4 + 7
                                sub ax, bp             
                                pop ax              
                                jz ??int23_ign_carry 
                                                                     
				push ax
                                popf                   

??int23_ign_carry:
                                pop ax                                 
				jnc ??int23_respawn
                                push ds                 
				mov bp, DGROUP
				mov ds, bp
				inc byte [_break_flg]
				pop ds

                                xor ah, ah            

??int23_respawn:
                                pop bp                                 
                jmp 	_int21_handler



