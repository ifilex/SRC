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

                %include "2os.inc"


segment	_IO_FIXED_DATA

                global  ConTable
ConTable        db      0Ah
                dw      _IOExit
                dw      _IOExit
                dw      _IOExit
                dw      _IOCommandError
                dw      ConRead
                dw      CommonNdRdExit
                dw      ConInStat
                dw      ConInpFlush
                dw      ConWrite
                dw      ConWrite
                dw      _IOExit

PRT_SCREEN      equ     7200h
CTL_P           equ     10h

segment	_IO_TEXT

uScanCode       db      0              

                global  ConRead
ConRead:
                jcxz    ConRead2                

ConRead1:
                call    KbdRdChar              
                stosb                          
                loop    ConRead1              

ConRead2:
                jmp     _IOExit


                global  KbdRdChar
KbdRdChar:
                xor     ax,ax                
                xchg    [cs:uScanCode],al      
                or      al,al                 
                jnz     KbdRdRtn               
                int     16h                  
                or      ax,ax               
                jz      KbdRdChar             
                cmp     ax,PRT_SCREEN          
                jne     KbdRd1                
                mov     al,CTL_P                     
KbdRd1:
                or      al,al                 
                jnz     KbdRdRtn              
                mov     [cs:uScanCode],ah      
KbdRdRtn:
                retn



                global  CommonNdRdExit
CommonNdRdExit:
                mov     al,[cs:uScanCode]      
                or      al,al                 
                jnz     ConNdRd2              
                mov     ah,1
                int     16h                   
                jz      ConNdRd4              
                or      ax,ax               
                jnz     ConNdRd1               
                int     16h                  
                jmp     short CommonNdRdExit

ConNdRd1:
                cmp     ax,PRT_SCREEN          
                jne     ConNdRd2             
                mov     al,CTL_P

ConNdRd2:
                lds     bx,[_ReqPktPtr]       
                mov     [bx+0Dh],al

ConNdRd3:
                jmp     _IOExit

ConNdRd4:
                jmp     _IODone



                global  ConInpFlush
ConInpFlush:
                call    KbdInpChar
                jmp     _IOExit



KbdInpChar:
                mov     byte [cs:uScanCode],0
KbdInpCh1:
                mov     ah,1
                int     16h                     
                jz      KbdInpRtn             
                xor     ah,ah                 
                int     16h                  
                jmp     short KbdInpCh1
KbdInpRtn:
                retn


                global  ConWrite
ConWrite:
                jcxz    ConNdRd3               
ConWr1:
                mov     al,[es:di]
                inc     di
                int     29h                   
                loop    ConWr1                 
                jmp     _IOExit

CBreak:
                mov     byte [cs:uScanCode],3  
IntRetn:
                iret

                global  _int29_handler
_int29_handler:
                push    ax
                push    si
                push    di
                push    bp
                push    bx
                mov     ah,0Eh
                mov     bh,0
                mov     bl,7
                int     10h                   
                pop     bx
                pop     bp
                pop     di
                pop     si
                pop     ax
                iret


      	  	global  ConInStat
ConInStat:
                mov     al,[cs:uScanCode]      
                or      al,al                 
                jnz     ConCharReady           
		mov     ah,1
                int     16h                    
                jz      ConNoChar             

                or      ax,ax                  
                jnz     ConIS1                  
                int     16h                     
                jmp     short ConInStat

ConIS1:
                cmp     ax,PRT_SCREEN          
                jne     ConIS2                 
                mov     al,CTL_P

ConIS2:
                lds     bx,[_ReqPktPtr]         
                mov     [bx+0Dh],al
ConCharReady:
                jmp     _IODone           
ConNoChar:
                jmp     _IOExit               

