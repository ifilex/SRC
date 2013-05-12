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

                global  LptTable
LptTable        db      18h
                dw      _IOExit
                dw      _IOExit
                dw      _IOExit
                dw      _IOCommandError
                dw      _IOSuccess
                dw      _IODone
                dw      _IOExit
                dw      _IOExit
                dw      PrtWrite
                dw      PrtWrite
                dw      PrtOutStat
                dw      _IOExit
                dw      _IOExit
                dw      _IOExit
                dw      _IOExit
                dw      _IOExit
                dw      PrtOutBsy
                dw      _IOExit
                dw      _IOExit
                dw      PrtGenIoctl
                dw      _IOExit
                dw      _IOExit
                dw      _IOExit
                dw      _IOCommandError
                dw      _IOCommandError


segment	_IO_TEXT
                global  uPrtNo
uPrtNo          db      0
uPrtQuantum     dw      50h
                dw      50h, 50h
                db      50h, 00h

PrtWrite:
                jcxz    PrtWr3                 
PrtWr1:
                mov     bx,2
PrtWr2:
                mov     al,[es:di]
                inc     di
                xor     ah,ah                  
                call    PrtIOCall             
                jnz     PrtWr4                 
                loop    PrtWr1              
PrtWr3:
                jmp     _IOExit
PrtWr4:
                dec     di
                dec     bx
                jnz     PrtWr2
PrtWr5:
                jmp     _IOErrCnt



PrtOutStat:
                call    GetPrtStat
                jnz     PrtWr5
                mov     al,9
                test    ah,20h
                jnz     PrtWr5
                test    ah,80h
                jnz     PrtWr3
                jmp     _IODone



GetPrtStat:
                mov     ah,2

PrtIOCall:
                call    GetUnitNum
                int     17h                    
                test    ah,8
                jz      PrtIOCal2
                mov     al,9
                test    ah,20h
                jnz     PrtIOCal1
                inc     al
PrtIOCal1:
                retn
PrtIOCal2:
                mov     al,2
                test    ah,1
                retn



PrtOutBsy:
                push    ds
                push    es
                pop     ds
                mov     si,di
PrtOtBsy1:
                push    cx
                push    bx
                xor     bx,bx
                mov     bl,[cs:uPrtNo]
                shl     bx,1
                mov     cx,[cs:uPrtQuantum+bx]
                pop     bx
PrtOtBsy2:
                call    GetPrtStat
                jnz     PrtOtBsy3
                test    ah,80h
                loopz   PrtOtBsy2
                pop     cx
                jz      PrtOtBsy4
                lodsb
                xor     ah,ah
                call    PrtIOCall
                jnz     PrtOtBsy4
                loop    PrtOtBsy1
                pop     ds
                lds     bx,[cs:_ReqPktPtr]
                sub     [bx+12h],cx
                jmp     _IOExit
PrtOtBsy3:
                pop     cx
PrtOtBsy4:
                pop     ds
                lds     bx,[cs:_ReqPktPtr]
                sub     [bx+12h],cx
                jmp     _IOErrorExit



PrtGenIoctl:
                les     di,[cs:_ReqPktPtr]
                cmp     byte [es:di+0Dh],5
                je      PrtGnIoctl2
PrtGnIoctl1:
                jmp     _IOCommandError
PrtGnIoctl2:
                mov     al,[es:di+0Eh]
                les     di,[es:di+13h]
                xor     bx,bx
                mov     bl,[cs:uPrtNo]
                shl     bx,1
                mov     cx,[cs:uPrtQuantum+bx]
                cmp     al,65h
                je      PrtGnIoctl3
                cmp     al,45h
                jne     PrtGnIoctl1
                mov     cx,[es:di]
PrtGnIoctl3:
                mov     [cs:uPrtQuantum+bx],cx
                mov     [es:di],cx
                jmp     _IOExit
