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

                extern   ConTable:wrt TGROUP
                extern   LptTable:wrt TGROUP
                extern   ComTable:wrt TGROUP
                extern   uPrtNo:wrt TGROUP
                extern   CommonNdRdExit:wrt TGROUP
                extern   _NumFloppies:wrt DGROUP
                extern   blk_stk_top:wrt DGROUP
                extern   clk_stk_top:wrt DGROUP
                extern   _blk_driver:wrt TGROUP
                extern   _clk_driver:wrt TGROUP

cmdlen  equ     0                       
unit    equ     1                    
cmd     equ     2                     
status  equ     3                      
media   equ     13                     
trans   equ     14                     
count   equ     18                     
start   equ     20                     
vid     equ     22                     
huge    equ     26                    

segment	_IO_FIXED_DATA

                global  _con_dev
_con_dev        equ     $
                dw      _prn_dev,TGROUP
                dw      8013h           
                dw      GenStrategy
                dw      ConIntr
                db      'CON     '

                global  _prn_dev
_prn_dev        dw      _aux_dev,TGROUP
                dw      0A040h
                dw      GenStrategy
                dw      PrnIntr
                db      'PRN     '

                global  _aux_dev
_aux_dev        dw      _Lpt1Dev,TGROUP
                dw      8000h
                dw      GenStrategy
                dw      AuxIntr
                db      'AUX     '

_Lpt1Dev        dw      _Lpt2Dev,TGROUP
                dw      0A040h
                dw      GenStrategy
                dw      Lpt1Intr
                db      'LPT1    '
_Lpt2Dev        dw      _Lpt3Dev,TGROUP
                dw      0A040h
                dw      GenStrategy
                dw      Lpt2Intr
                db      'LPT2    '
_Lpt3Dev        dw      _Com1Dev,TGROUP
                dw      0A040h
                dw      GenStrategy
                dw      Lpt3Intr
                db      'LPT3    '

_Com1Dev        dw      _Com2Dev,TGROUP
                dw      8000h
                dw      GenStrategy
                dw      AuxIntr
                db      'COM1    '
_Com2Dev        dw      _Com3Dev,TGROUP
                dw      8000h
                dw      GenStrategy
                dw      Com2Intr
                db      'COM2    '
_Com3Dev        dw      _Com4Dev,TGROUP
                dw      8000h
                dw      GenStrategy
                dw      Com3Intr
                db      'COM3    '
_Com4Dev        dw      _clk_dev,TGROUP
                dw      8000h
                dw      GenStrategy
                dw      Com4Intr
                db      'COM4    '

                global  _clk_dev
_clk_dev        equ     $
                dw      _blk_dev,TGROUP
                dw      8004h          
                dw      GenStrategy
                dw      clk_entry
                db      'CLOCK$  '

                global  _blk_dev
_blk_dev        equ     $
                dd      -1
                dw      0000h           ; block device
                dw      GenStrategy
                dw      blk_entry
                db      4
                db      0,0,0,0,0,0,0



segment	_IO_FIXED_DATA
DiskTable       db      0


segment	_BSS
blk_dos_stk	resw	1
blk_dos_seg	resw	1
clk_dos_stk	resw	1
clk_dos_seg	resw	1

segment	_IO_TEXT

		global	_ReqPktPtr
_ReqPktPtr	dd	0
uUnitNumber	dw	0

GenStrategy:
                mov     word [cs:_ReqPktPtr],bx
                mov     word [cs:_ReqPktPtr+2],es
                retf


ConIntr:
                push    si
                mov     si,ConTable
                jmp     short CharIntrEntry

PrnIntr:
                push    si
                push    ax
                xor     ax,ax
                jmp     short LptCmnIntr

Lpt1Intr:
                push    si
                push    ax
                xor     al,al
                mov     ah,1
                jmp     short LptCmnIntr

Lpt2Intr:
                push    si
                push    ax
                mov     al,1
                mov     ah,2
                jmp     short LptCmnIntr

Lpt3Intr:
                push    si
                push    ax
                mov     al,2
                mov     ah,3

LptCmnIntr:
                mov     si,LptTable
                mov     [cs:uPrtNo],ah
                jmp     short DiskIntrEntry


AuxIntr:
                push    si
                push    ax
                xor     al,al
                jmp     short ComCmnIntr

Com2Intr:
                push    si
                push    ax
                mov     al,1
                jmp     short ComCmnIntr

Com3Intr:
                push    si
                push    ax
                mov     al,2
                jmp     short ComCmnIntr

Com4Intr:
                push    si
                push    ax
                mov     al,3
                jmp     short ComCmnIntr

ComCmnIntr:
                mov     si,ComTable
                jmp     short DiskIntrEntry


DskIntr:
                push    si
                mov     si,DiskTable
CharIntrEntry:
                push    ax
DiskIntrEntry:
                push    cx
                push    dx
                push    di
                push    bp
                push    ds
                push    es
                push    bx
                mov     byte [cs:uUnitNumber],al
                lds     bx,[cs:_ReqPktPtr]
                test    byte [cs:si],80h
                je      AsmType

                mov     al,[bx+cmd]
                cmp     al,[cs:si]
                ja      _IOCommandError
                cbw
                shl     ax,1
                add     si,ax
                xchg    di,ax

                push    ds
                push    bx
                mov     bp,sp
                mov     ax,DGROUP
                mov     ds,ax
                cld
                call    word [cs:si+1]
                pop     cx
                pop     cx
                jmp     short StoreStatus

AsmType:        mov     al,[bx+unit]
                mov     ah,[bx+media]
                mov     cx,[bx+count]
                mov     dx,[bx+start]
                xchg    di,ax
                mov     al,[bx+cmd]
                cmp     al,[cs:si]
                ja      _IOCommandError
                cbw
                shl     ax,1
                add     si,ax
                xchg    di,ax

                les     di,[bx+trans]
                push    ax
                mov     ax,DGROUP
                mov     ds,ax
                pop     ax
                cld
                jmp     word [cs:si+1]

                global  _IOSuccess
_IOSuccess:
                lds     bx,[cs:_ReqPktPtr]
                xor     ax,ax
                mov     [bx+count],ax

                global  _IOExit
_IOExit:
                mov     ah,1

StoreStatus:
                lds     bx,[cs:_ReqPktPtr]
                mov     [bx+status],ax
                pop     bx
                pop     es
                pop     ds
                pop     bp
                pop     di
                pop     dx
                pop     cx
                pop     ax
                pop     si
                retf


                global  _IODone
_IODone:
                mov     ah,3
                jmp     short StoreStatus

                global  _IOCommandError
_IOCommandError:
                mov     al,3

                global  _IOErrCnt
_IOErrCnt: 
                lds     bx,[cs:_ReqPktPtr]
                sub     [bx+count],cx
                global  _IOErrorExit
_IOErrorExit:
                mov     ah,81h
                jmp     short StoreStatus

                global  GetUnitNum
GetUnitNum:
                mov     dx,[cs:uUnitNumber]
                ret

blk_entry:
                pushf
                push    ax
                push    bx
                push    cx
                push    dx
                push    bp
                push    si
                push    di
                push    ds
                push    es

             
                mov     ax,DGROUP                      
                mov     ds,ax                          
                mov     word [blk_dos_stk],sp         
                mov     word [blk_dos_seg],ss
                pushf                                  
                pop     bx
                cli                                 
                mov     ss,ax
                mov     sp,blk_stk_top
                push    bx
                popf                                   
                mov     bp,sp                          
                push    word [cs:_ReqPktPtr+2]
                push    word [cs:_ReqPktPtr]
                call    _blk_driver
                pop     cx
                pop     cx
                les     bx,[cs:_ReqPktPtr]             
                mov     word [es:bx+status],ax      
                cli                                    
                mov     sp,[blk_dos_stk]            
                mov     ss,[blk_dos_seg]
                pop     es
                pop     ds
                pop     di
                pop     si
                pop     bp
                pop     dx
                pop     cx
                pop     bx
                pop     ax
                popf
                retf



clk_entry:
                pushf
                push    ax
                push    bx
                push    cx
                push    dx
                push    bp
                push    si
                push    di
                push    ds
                push    es

            
                mov     ax,DGROUP                     
                mov     ds,ax                         
                mov     word [clk_dos_stk],sp         
                mov     word [clk_dos_seg],ss
                pushf                                
                pop     bx
                cli                                    
                mov     ss,ax
                mov     sp,clk_stk_top
                push    bx
                popf                       
                mov     bp,sp                          
                push    word [cs:_ReqPktPtr+2]
                push    word [cs:_ReqPktPtr]
                call    _clk_driver
                pop     cx
                pop     cx
                les     bx,[cs:_ReqPktPtr]           
                mov     word [es:bx+status],ax         
                cli                                  
                mov     sp,[clk_dos_stk]              
                mov     ss,[clk_dos_seg]
                pop     es
                pop     ds
                pop     di
                pop     si
                pop     bp
                pop     dx
                pop     cx
                pop     bx
                pop     ax
                popf
                retf
