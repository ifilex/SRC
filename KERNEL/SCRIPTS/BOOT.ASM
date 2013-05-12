;
; File:
;                            boot.asm
; Description:
;                           DOS-C boot
;
;                       Copyright (c) 1997;
;                           Svante Frey
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
;
; $Log: boot.asm,v $
; Revision 1.3  2000/05/25 20:56:19  jimtabor
; Fixed project history
;
; Revision 1.2  2000/05/11 03:56:42  jimtabor
; Clean up and Release
;
; Revision 1.1.1.1  2000/05/06 19:34:53  jhall1
; The FreeDOS Kernel.  A DOS kernel that aims to be 100% compatible with
; MS-DOS.  Distributed under the GNU GPL.
;
; Revision 1.12  1999/09/25 06:42:18  jprice
; Optimize boot loader.  Documentation.
;
; Revision 1.11  1999/09/24 19:04:55  jprice
; Added changes recommended by Jens Horstmeier

; to make their bootable CD work.
;
; Revision 1.10  1999/09/23 04:39:02  jprice
; *** empty log message ***
;
; Revision 1.7  1999/04/23 03:43:46  jprice
; Ported to NASM by ror4
;
; Revision 1.6  1999/04/17 19:14:03  jprice
; Fixed multi-sector code
;
; Revision 1.5  1999/04/17 06:23:26  jprice
; Changed so multi-sector IO is optional.
;
; Revision 1.4  1999/04/13 15:52:22  jprice
; Moves boot sector to top of mem
;
; Revision 1.3  1999/04/06 22:53:36  jprice
; Put back code to read multiple sectors at a time.
;
; Revision 1.2  1999/04/01 07:23:20  jprice
; New boot loader
;
; Revision 1.1.1.1  1999/03/29 15:39:39  jprice
; New version without IPL.SYS
;
; Revision 1.3  1999/03/02 06:57:14  jprice
; Added entry address for more recent versions of TLINK
;
; Revision 1.2  1999/01/21 05:03:58  jprice
; Formating.
;
; Revision 1.1.1.1  1999/01/20 05:51:00  jprice
; Imported sources
;
;
;          Rev 1.5   10 Jan 1997  4:58:06   patv
;       Corrected copyright
;
;          Rev 1.4   10 Jan 1997  4:52:50   patv
;       Re-written to support C drive and eliminate restrictions on IPL.SYS
;
;          Rev 1.3   29 Aug 1996 13:06:50   patv
;       Bug fixes for v0.91b
;
;          Rev 1.2   01 Sep 1995 17:56:44   patv
;       First GPL release.
;
;          Rev 1.1   30 Jul 1995 20:37:38   patv
;       Initialized stack before use.
;
;          Rev 1.0   02 Jul 1995 10:57:52   patv
;       Initial revision.
;

;	+--------+
;	|        |
;	|        |
;	|--------| 4000:0000
;	|        |
;	|  FAT   |
;	|        |
;	|--------| 2000:0000
;	|BOOT SEC|
;	|RELOCATE|
;	|--------| 1FE0:0000
;	|        |
;	|        |
;	|        |
;	|        |
;	|--------|
;	|BOOT SEC|
;	|ORIGIN  | 07C0:0000
;	|--------|
;	|        |
;	|        |
;	|        |
;	|--------|
;	|KERNEL  |
;	|LOADED  |
;	|--------| 0060:0000
;	|        |
;	+--------+

segment	.text

%define BASE            7c00h

                org     BASE

Entry:          jmp     short real_start
		nop

 
%define bsOemName       bp+3         
%define bsBytesPerSec   bp+0bh       
%define bsSecPerClust   bp+0dh       
%define bsResSectors    bp+0eh       
%define bsFATs          bp+10h    
%define bsRootDirEnts   bp+11h       
%define bsSectors       bp+13h      
%define bsMedia         bp+15h      
%define sectPerFat      bp+16h      
%define sectPerTrack    bp+18h     
%define nHeads          bp+1ah      
%define nHidden         bp+1ch    
%define nSectorHuge     bp+20h    
%define drive           bp+24h     
%define extBoot         bp+26h     
%define volid           bp+27h
%define vollabel        bp+2bh
%define filesys         bp+36h

%define LOADSEG         0060h

%define FATBUF          4000h           
                                    

%define StoreSI         bp+3h         


%macro		GETDRIVEPARMS	0
                mov     si, word [nHidden]
                mov     di, word [nHidden+2]
                add     si, word [bsResSectors]
                adc     di, byte 0              

                mov     word [fat_start], si
                mov     word [fat_start+2], di

                mov     al, [bsFATs]
                xor     ah, ah
                mul     word [sectPerFat]      

                add     si, ax
                adc     di, dx                
                mov     word [root_dir_start], si
                mov     word [root_dir_start+2], di

                 
                mov     bx, [bsBytesPerSec]
                mov     cl, 5                  
                shr     bx, cl                

                mov     ax, [bsRootDirEnts]
                xor     dx, dx
                div     bx

                mov     word [RootDirSecs], ax  

                add     si, ax
                adc     di, byte 0             

                mov     [data_start], si
                mov     [data_start+2], di
%endmacro


		times	3Eh-$+$$ db 0

%define tempbuf         bp+3Eh
                dw      LOADSEG

%ifdef CALCPARAMS
%define RootDirSecs     bp+27h          

%define fat_start       bp+29h         

%define root_dir_start  bp+2Dh       

%define data_start      bp+31h      

%else
%define RootDirSecs     bp+40h         
                dw      0

%define fat_start       bp+42h         
                dd      0

%define root_dir_start  bp+46h          
                dd      0

%define data_start      bp+4Ah        
                dd      0
%endif


real_start:     cli
                cld
;                mov     ax, cs
		xor	ax, ax
                mov     ss, ax         
		mov	ds, ax
                mov     bp, 7c00h
                lea     sp, [bp-20h]
                sti
;               int     13h             
;               int     12h             
		mov	ax, 480
                mov     cl, 6           
		shl	ax, cl
		sub	ax, 07e0h
		mov	es, ax
		mov	si, bp
		mov	di, bp
		mov	cx, 256
		rep	movsw
		push	es
		mov	bx, cont
		push	bx
		retf

cont:           mov     ds, ax
		mov	ss, ax
                mov     [drive], dl     

                call    print
                db      13,10,"boot.asm: Cargando ..",0

%ifdef CALCPARAMS
                GETDRIVEPARMS
%endif



                mov     ax, word [root_dir_start]
                mov     dx, word [root_dir_start+2]
                mov     di, word [RootDirSecs]
                xor     bx, bx
                mov     es, [tempbuf]
                call    readDisk
                jc      jmp_boot_error

                xor     di, di

next_entry:     mov     cx, 11
                mov     si, filename
                push    di
                repe    cmpsb
                pop     di
                mov     ax, [es:di+1ah]     
                je      ffDone

                add     di, byte 20h        
                cmp     byte [es:di], 0     
                jnz     next_entry          

                jc      boot_error     
ffDone:





                push    ax                      


                mov     es, [tempbuf]
                xor     bx, bx
                mov     di, [sectPerFat]
                mov     ax, word [fat_start]
                mov     dx, word [fat_start+2]
                call    readDisk
                pop     ax                      
jmp_boot_error: jc      boot_error
                push    ds
                push    es
                pop     ds
                pop     es
                mov     di, FATBUF

next_clust:     stosw                          
                mov     si, ax                   

%ifdef ISFAT12

fat_12:         add     si, si          
                add     si, ax
                shr     si, 1         
                lodsw

                jnc     fat_even
                mov     cl, 4
                shr     ax, cl          

fat_even:       and     ah, 0fh        
                cmp     ax, 0fffh       
                jb      next_clust      

%endif
%ifdef ISFAT16

fat_16:         mov     dx, [tempbuf]
                add     si, si          
                jnc     first_half     
                add     dh, 10h        

first_half:     mov     ds, dx       
                lodsw                 

                cmp     ax, 0fff8h      
                jb      next_clust      
%endif

finished:    
                

                xor     ax, ax
                stosw

                push    cs
                pop     ds



                mov     es, [tempbuf]  
                xor     bx, bx

                mov     si, FATBUF    

cluster_next:   lodsw                         
                or      ax, ax                 
                je      boot_success          

                dec     ax                    
                dec     ax

                mov     di, word [bsSecPerClust]
                and     di, 0ffh               
                mul     di
                add     ax, [data_start]
                adc     dx, [data_start+2]     
                call    readDisk
                jnc     cluster_next


boot_error:     call    print
                db      13,10,"boot.asm: Error!",0

                jmp     short $          

boot_success:   
                mov     bl, [drive]
		jmp	word LOADSEG:0



print:          mov   [StoreSI], si
                pop   si                     
                push  ax
                push  bx
                push  di
                xor   bx, bx                  
                mov   ah, 0Eh                
print1:         lodsb                      
                cmp   al, 0                  
                je    print2                
                int   10h                      
                jmp   short print1             

print2:         pop   di
                pop   bx
                pop   ax
                push  si                      
                mov   si, [StoreSI]
                ret                            



readDisk:       push    si
read_next:      push    dx
                push    ax

                div     word [sectPerTrack]
                mov     cx, dx
                xor     dx, dx
                div     word [nHeads]
                mov     dh, dl                  
                ror     ah, 1                   
                ror     ah, 1                
                xchg    al, ah                
                mov     dl, byte [sectPerTrack]
                sub     dl, cl
                inc     cl                   
                or      cx, ax            
                mov     al, dl               

%ifdef MULTI_SEC_READ
                push    dx

                mov     si, di          
                mov     ax, bx               
                neg     ax                 
                jz      ax_min_1             

                xor     dx, dx              
                div     word [bsBytesPerSec]

                cmp     ax, di                 
                jb      ax_min_1              
                mov     si, ax                  

ax_min_1:       pop     dx

                mov     si, [sectPerTrack]
                inc     si
                mov     ax, cx               
                and     ax, 03fh              
                sub     si, ax               
                mov     ax, di
                cmp     si, di                 
                jge     ax_min_2
                mov     ax, si              

;ax_min_2:       mov     ah, 2
ax_min_2:       push    ax
                mov     ah, 2
                mov     dl, [drive]
                int     13h
                pop     ax
%else
                mov     ax, 0201h
                mov     dl, [drive]
                int     13h
%endif
                jnc     read_ok            
                xor     ah, ah             
                int     13h
                pop     ax
                pop     dx                      
                jmp     short read_next         

read_ok:
%ifdef MULTI_SEC_READ
                mul     word [bsBytesPerSec]     
                add     bx, ax
%else
                add     bx, word [bsBytesPerSec]
%endif
                jnc     no_incr_es            

                mov     ax, es
                add     ah, 10h               
                mov     es, ax

no_incr_es:     pop     ax
                pop     dx                  

;                call    print
;                db      ".",0

%ifdef MULTI_SEC_READ
                add     ax, si
                adc     dx, byte 0            
                sub     di,si                  
                jg      read_next              
%else
                add     ax, 1
                adc     dx, byte 0             
                dec     di                     
                jnz     read_next           
%endif

                clc
                pop     si
                ret

filename        db      "KERNEL  SYS"

		times	01feh-$+$$ db 0

sign            dw      0aa55h
