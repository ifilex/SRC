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
; $Id: apisupt.asm,v 1.3 2000/05/25 20:56:21 jimtabor Exp $
;
; $Log: apisupt.asm,v $
; Revision 1.3  2000/05/25 20:56:21  jimtabor
; Fixed project history
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

group	IGROUP	INIT_TEXT

segment	INIT_TEXT	class=INIT

                global  _ReadATClock
_ReadATClock:
                push    bp
                mov     bp,sp
                sub     sp,byte 10
                mov     ah,2
                int     26
		jnc	@RdAT1140
		sbb	ax,ax
                mov     sp,bp
                pop     bp
                ret
                nop
@RdAT1140:
                mov     byte [bp-2],ch      ;Hours
                mov     byte [bp-8],cl      ;Minutes
                mov     byte [bp-10],dh     ;Seconds
                mov     ah,4
                int     26
                mov     word [bp-6],dx     
                mov     word [bp-4],cx
                mov     ax,word [bp-6]     
                mov     dx,word [bp-4]
                mov     bx,word [bp+4]     
                mov     word [bx],ax
                mov     word [bx+2],dx
                mov     al,byte [bp-2]      
                mov     bx,word [bp+6]     
                mov     byte [bx],al
                mov     al,byte [bp-8]      
                mov     bx,word [bp+8]     
                mov     byte [bx],al
                mov     al,byte [bp-10]   
                mov     bx,word [bp+10]   
                mov     byte [bx],al
                sub     ax,ax
                mov     sp,bp
                pop     bp
                ret
                nop
