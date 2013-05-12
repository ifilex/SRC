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

                global  ComTable
ComTable        db      0Ah
                dw      _IOExit
                dw      _IOExit
                dw      _IOExit
                dw      _IOCommandError
                dw      ComRead
                dw      ComNdRead
                dw      _IOExit
                dw      ComInpFlush
                dw      ComWrite
                dw      ComWrite
                dw      ComOutStat



segment	_IO_TEXT

                extern   CommonNdRdExit:wrt TGROUP

ComRead:
                jcxz    ComRd3
                call    GetComStat
                xor     ax,ax
                xchg    [bx],al
                or      al,al
                jnz     ComRd2
ComRd1:
                call    BiosRdCom
ComRd2:
                stosb
                loop    ComRd1

ComRd3:
                jmp     _IOExit


BiosRdCom:
                mov     ah,2
                call    ComIOCall
                test    ah,0Eh
                jz      BiosRdRetn
                add     sp,byte 2
                xor     al,al
                or      al,0B0h
                jmp     _IOErrCnt
BiosRdRetn:
                retn



ComNdRead:
                call    GetComStat
                mov     al,[bx]
                or      al,al
                jnz     ComNdRd1
                call    ComRdStatus
                test    ah,1
                jz      ComNdRtn
                test    al,20h
                jz      ComNdRtn
                call    BiosRdCom
                call    GetComStat
                mov     [bx],al
ComNdRd1:
                jmp     CommonNdRdExit
ComNdRtn:
                jmp     _IODone


ComOutStat:
                call    ComRdStatus
                test    al,20h
                jz      ComNdRtn
                test    ah,20h
                jz      ComNdRtn
                jmp     _IOExit


ComRdStatus:
                mov     ah,3
                call    ComIOCall
                retn


ComIOCall:
                call    GetUnitNum
                int     14h                  
                retn


ComInpFlush:
                call    GetComStat
                mov     byte [bx],0
                jmp     _IOExit


ComWrite:
                jcxz    ComRd3
ComWr1:
                mov     al,[es:di]
                inc     di
                mov     ah,1
                call    ComIOCall
                test    ah,80h
                jz      ComWr2
                mov     al,0Ah
                jmp     _IOErrCnt
ComWr2:
                loop    ComWr1
                jmp     _IOExit


GetComStat:
                call    GetUnitNum
                mov     bx,dx
                add     bx,ComStatArray
                retn

segment	_DATA

ComStatArray    db      0, 0, 0, 0
