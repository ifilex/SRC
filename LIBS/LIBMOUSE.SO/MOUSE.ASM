; CuteMouse driver v1.8. Copyright (c) 1997-2001 Nagy Daniel
;
; Credits:
; --------
;
; - Arkady V.Belousov <ark@mos.ru>: many bugfixes, optimizations and features
; - Alain M. <alainm@pobox.com>: many ideas and message files
; - Frank Pruefer <fp666@t-online.de>: documentation proofreading
; - Mike Millen <mikemillen@ukgateway.net>: documentation proofreading
; - Robert Riebisch <riebisch@bercom-berlin.de>: mode 13h positioning bugfix
; - Fernando Papa Budzyn: self-loadhigh capability
; - Martin <nocash@work.de>: optimizations
; - Paul Schubert: much faster PS/2 codes
; - All who helped me with ideas and codes
;
;
; Contacts:
; ---------
;
; mailto:nagyd@almos.vein.hu
; http://www.vein.hu/~nagyd

WARN
.model tiny

driverversion   equ     626h                    

PUSHALL		equ	push	ax cx dx bx bp si di
POPALL		equ	pop	di si bp bx dx cx ax

_ARG_DI_	equ	word ptr [bp]
_ARG_SI_	equ	word ptr [bp+2]
_ARG_BP_	equ	word ptr [bp+4]
_ARG_BX_	equ	word ptr [bp+6]
_ARG_DX_	equ	word ptr [bp+8]
_ARG_CX_	equ	word ptr [bp+10]
_ARG_AX_	equ	word ptr [bp+12]
_ARG_ES_	equ	word ptr [bp+14]
_ARG_DS_	equ	word ptr [bp+16]
_ARG_OFFS_	equ	22

j		equ	jmp short

movSeg		macro	dest,src
		push	src
		pop	dest
	endm

saveFAR		macro	addr,segm,offs
		mov	word ptr addr[0],offs
		mov	word ptr addr[2],segm
	endm

PS2serv		macro	serv,errlabel
		mov	ax,serv
		int	15h
	IFNB <errlabel>
		jc	errlabel
		or	ah,ah
		jne	errlabel
	ENDIF
	endm

PrintS		macro	addr
	IFNB <addr>
	 IFDIFI <addr>,<dx>
		mov	dx,offset addr
	 ENDIF
	ENDIF
		mov	ah,9
		int	21h
	endm

POINT		struc
  X		dw	0
  Y		dw	0
	ends

.code
		org	100h
start:		jmp	real_start

                org     4Eh             

		evendata
StartSaveArea = $

mickey8         POINT   ?              
;;+*doublespeed dw      ?             
startscan       dw      ?               
endscan         dw      ?              
hotspot         POINT   ?               
screenmask      db      2*16 dup (?)    
cursormask      db      2*16 dup (?)  
cursorhidden    db      ?             
		evendata
LenDefArea = $ - StartSaveArea        

rangemax        POINT   ?               
upleft          POINT   ?               
lowright        POINT   ?              
pos             POINT   ?            
granpos         POINT   ?             
UIR@            dd      ?              

		evendata
StartClearArea = $

rounderr        POINT   ?               
		evendata
LenClearArea1 = $ - StartClearArea      

rangemin        POINT   ?               
		evendata
LenClearArea2 = $ - StartClearArea     

cursortype      db      ?               
callmask        db      ?              
mickeys         POINT   ?              
BUTTLASTSTATE	struc
  counter	dw	?
  lastrow	dw	?
  lastcol	dw	?
	ends
buttpress	BUTTLASTSTATE	?,?,?,?
buttrelease	BUTTLASTSTATE	?,?,?,?
		evendata
LenClearArea3 = $ - StartClearArea      
LenSaveArea = $ - StartSaveArea

		evendata
StartVRegsArea = $

regs_SEQC	db	5 dup (?)
reg_MISC	db	?
regs_CRTC	db	25 dup (?)
regs_ATC	db	21 dup (?)
regs_GRC	db	9 dup (?)
reg_FC		db	?
reg_GPOS1	db	?
reg_GPOS2	db	?
		evendata
LenVRegsArea = $ - StartVRegsArea

		evendata
StartDefVRegsArea = $

def_SEQC	db	5 dup (?)
def_MISC	db	?
def_CRTC	db	25 dup (?)
def_ATC		db	21 dup (?)
def_GRC		db	9 dup (?)
def_FC		db	?
def_GPOS1	db	?
def_GPOS2	db	?
		evendata
ERRIF ($-StartDefVRegsArea ne LenVRegsArea) "Area vga corrompida!!"

; VECTORES ANTIGUOS -------------------------------------------------------

		evendata
oldint33        dd      ?              
oldIRQaddr      dd      ?              


; INICIA LA INFORMACION ---------------------------------------------------

		evendata
StartTSRpart	label

StartDefArea = $
                POINT   {X=8,Y=16}      
;;+*            dw      64              
                dw      77FFh,7700h     

                POINT   {X=0,Y=0}      
                dw      03FFFh, 01FFFh, 00FFFh, 007FFh 
		dw	003FFh, 001FFh, 000FFh, 0007Fh
		dw	0003Fh, 0001Fh, 001FFh, 000FFh
		dw	030FFh, 0F87Fh, 0F87Fh, 0FCFFh
		dw	00000h, 04000h, 06000h, 07000h	; cursormask
		dw	07800h, 07C00h, 07E00h, 07F00h
		dw	07F80h, 07C00h, 06C00h, 04600h
		dw	00600h, 00300h, 00300h, 00000h

		db	1		; cursorhidden
;;+*            db      0               
		evendata
ERRIF ($-StartDefArea ne LenDefArea) "Area por defecto corrompida!!"

; DRIVER Y ESTADOS DE VIDEO -----------------------------------------------

		evendata
UIRunlock       db      1               
videounlock     db      1               

cursorptr       dd      0              
cursordrawn     db      0              

videomode       db      ?               
granumask	POINT	{X=-1,Y=-1}
textbuf		label	word
buffer@         dd      ?               

vdata1		dw	10h,1, 10h,3, 10h,4, 10h,5, 10h,8, 08h,2
VDATA1cnt	equ	($-vdata1)/4
vdata2		dw	10h,1, 10h,4, 10h,0105h, 10h,0FF08h, 08h,0F02h
VDATA2cnt	equ	($-vdata2)/4

; TABLA DE PUNTEROS AL REGISTRO -------------------------------------------

RGROUPDEF	struc
  port@		dw	?
  regs@		dw	?
  def@		dw	?
  regscnt	db	1
  rmodify?	db	0
RGROUPDEF	ends

		evendata
videoregs@	label
			; CRTC
	RGROUPDEF {port@=3D4h,regs@=regs_CRTC,def@=def_CRTC,regscnt=25}
			; Sequencer
	RGROUPDEF {port@=3C4h,regs@=regs_SEQC,def@=def_SEQC,regscnt=5}
			; Graphics controller
	RGROUPDEF {port@=3CEh,regs@=regs_GRC, def@=def_GRC, regscnt=9}
			; VGA attrib controller
	RGROUPDEF {port@=3C0h,regs@=regs_ATC, def@=def_ATC, regscnt=20}
			; VGA misc output and input
	RGROUPDEF {port@=3C2h,regs@=reg_MISC, def@=def_MISC}
			; Feature Control
	RGROUPDEF {port@=3DAh,regs@=reg_FC,   def@=def_FC}
			; Graphics 1 Position
	RGROUPDEF {port@=3CCh,regs@=reg_GPOS1,def@=def_GPOS1}
			; Graphics 2 Position
	RGROUPDEF {port@=3CAh,regs@=reg_GPOS2,def@=def_GPOS2}



; LLAMADAS IRQ ------------------------------------------------------------

IRQhandler	proc
		;cld
		push	ds es
		PUSHALL
		movSeg	ds,cs
IRQproc         db      0EBh                   
		db	PS2proc-$-1		; else "mov al,20h"
                out     20h,al                 

		db	0BAh			; MOV DX,word
IO_address	dw	?			; COM port IO address
		push	dx
		inc	dx
		inc	dx
		in	al,dx			; {3FAh} IIR (intr id reg)
                add     dx,3                    ; {3FDh} LSR
                in      ax,dx                   ; {3FEh} MSR
		pop	dx
		xchg	bx,ax			; OPTIMIZE: instead MOV BL,AL
                in      al,dx                   ; {3F8h} flush 
		db	0B9h			; MOV CX,word
IOdone          db      ?,0                     

                test    bl,00001010b            
		jz	@@checkdata
                mov     [IOdone],ch           

@@checkdata:	shr	bl,1
                jnc     @@exitIRQ               
                db      0E8h                    
mouseproc	dw	MSMproc-$-2

@@exitIRQ:	jmp	@rethandler
IRQhandler	endp

bufferSERIAL    label                           

; PS/2 ABILITADO ----------------------------------------------------------

enablePS2	proc
		call	disablePS2
		movSeg	es,cs
		mov	bx,offset DGROUP:IRQhandler
                PS2serv 0C207h                 
                mov     bh,1                 
		PS2serv	0C200h
		ret
enablePS2	endp

; PS/2 DESABILITADO -------------------------------------------------------

disablePS2	proc
                xor     bx,bx                 
		PS2serv	0C200h
                mov     es,bx                  
                PS2serv 0C207h                 
		ret
disablePS2	endp

; -------------------------------------------------------------------------

PS2proc		proc
		mov	bp,sp
                mov     cx,[bp+_ARG_OFFS_+2]   
                mov     bx,[bp+_ARG_OFFS_+4]   
                mov     ax,[bp+_ARG_OFFS_+6]   
                test    al,20h                  
		jz	@@checkxsign
		mov	ch,-1
@@checkxsign:   test    al,10h                 
		jz	@@yreverse
		mov	bh,-1
@@yreverse:     neg     cx                      
		call	mouseupdate
		POPALL
		pop	es ds
		retf
PS2proc		endp
ERRIF ($-bufferSERIAL lt 3*16) "Buffer muy chico!"

; COPIADO DE BUFFER -------------------------------------------------------

bufferPS2       label                          

enableCOM	proc
                db      0B8h                  
IRQintnum       dw      2500h                  
		mov	dx,offset DGROUP:IRQhandler
                int     21h                    

		mov	si,[IO_address]
		mov	dx,si
		add	dx,3
		mov	al,80h
                out     dx,al                  

		xchg	dx,si
                mov     ax,96                   
                out     dx,ax                   

		xchg	dx,si
                db      0B8h                   
COMLCR          db      2,00001011b             
                out     dx,ax                 
                                               
		inc	dx
                inc     dx                     
                in      ax,dx                   
		mov	dx,si
                in      al,dx                  

		inc	dx
                mov     ax,1                    
                out     dx,ax                  

                dec     ax                     
		mov	[IOdone],al
		mov	[MSLTmiddle],al
                in      al,21h             
                db      24h                    
notPICstate     db      ?                       
                out     21h,al              
		ret
enableCOM	endp

disableCOM	proc
                in      al,21h                 
                db      0Ch                   
PICstate        db      ?                      
                out     21h,al              
		mov	dx,[IO_address]
		add	dx,3
                xor     ax,ax                 
                out     dx,ax                 
		dec	dx
		dec	dx
		;mov	al,0
                out     dx,al                  

		mov	ax,[IRQintnum]
		push	ds
		lds	dx,[oldIRQaddr]
                int     21h                    
		pop	ds
		ret
disableCOM	endp

MSLTproc	proc
                test    al,40h                
                jz      @@MSLTbyte2            

                mov     [IOdone],1           
		mov	[MSLTbyte1],al
MSLTCODE1       db      091h                   
                sub     al,3                    
		jnz	@@MSLTret
                mov     [MSLTmiddle],al         
@@MSLTret:	ret

@@MSLTbyte2:    jcxz    @@MSLTret              
                inc     [IOdone]                
		loop	@@MSLTbyte3
                mov     [MSLTbyteX],al         
		ret

@@MSLTbyte3:	loop	@@LTbyte4
                xchg    cx,ax                  
                db      0B4h                 
MSLTbyte1       db      ?                      

                ;mov    al,0              
                shr     ax,2                  
                db      0Ch                    
MSLTbyteX	db	?
		mov	bh,ah
		cbw
                xchg    bx,ax                  

		mov	al,0
                shr     ax,2                   
		or	al,cl
                mov     cl,ah                   
		cbw
                xchg    cx,ax                  

		xor	al,[MSLTmiddle]
                and     al,3                   
		mov	dl,al
                or      dl,bl                
                or      dl,cl                   
MSLTCODE2       db      0EBh                  
		db	@@MSLTupdate-$-1
                or      al,4                   
		j	@@MSLTupdate

@@LTbyte4:      mov     [IOdone],ch            
MSLTCODE3       db      0B1h,3                 
                                                
                shr     al,cl                   
		xor	al,[MSLTmiddle]
                and     al,4                   
		jz	@@MSLTret
		xor	cx,cx
		xor	bx,bx
@@MSLTupdate:   db      034h                    
MSLTmiddle      db      ?                       
		mov	[MSLTmiddle],al
		j	mouseupdate
MSLTproc	endp

MSMproc		proc
		jcxz	@@MSMbyte1
		cbw
		dec	cx
		jz	@@MSMbyte24
		dec	cx
		jz	@@MSMbyte3
		loop	@@MSMbyte5

@@MSMbyte24:	add	[MSMincrX],ax
@@MSMnext:      inc     [IOdone]               
@@MSMret:	ret

@@MSMbyte1:     mov     [MSMbyte1],al          
                mov     [MSMincrX],cx           
		and	al,not 7
                cmp     al,80h                  
		;je	@@MSMnext
		;ret
		jne	@@MSMret

@@MSMbyte3:	mov	[MSMincrY],ax
		j	@@MSMnext

@@MSMbyte5:     mov     [IOdone],ch            
                db      05h                   
MSMincrY	dw	?
                xchg    cx,ax                   
		neg	cx
                db      0BBh                  
MSMincrX	dw	?
                db      0B0h                   
MSMbyte1        db      ?                      
                not     al                     
                test    al,6                   
                jpe     @@MSMupdate             
                xor     al,6                 
@@MSMupdate:	;j	mouseupdate
MSMproc		endp
ERRIF ($-bufferPS2 lt 3*16) "Llamador serial muy chico."

mouseupdate	proc
                db      024h                   
buttonsmask	db	7
                test    al,3                  
LEFTHANDCODE    db      ?                      
		db	@@updatebut-$-1
                xor     al,3                
@@updatebut:	push	ax

                xchg    ax,bx                  
                db      0B2h            
mresolutionX	db	0
		mov	bx,offset X
		call	updateposition

		xchg	ax,cx
                db      0B2h                  
mresolutionY	db	0
                mov     bl,offset Y           
		call	updateposition
                or      cl,al                   

		pop	dx
		mov	dh,dl
                xchg    dl,byte ptr [buttstatus] 
                xor     dh,dl                 
                                               
                xor     bx,bx                 

                mov     al,2                   
		call	updatebutton
                mov     al,8                  
		call	updatebutton
                mov     al,20h                  
		call	updatebutton

		dec	[UIRunlock]
                jnz     @@updatedone           

                xchg    ax,cx                   
                and     al,[callmask]           
                jz      @@updateshow            
                cbw                             
                db      0BBh                   
buttstatus      dw      ?                     
		mov	cx,[granpos.X]
		mov	dx,[granpos.Y]
		mov	si,[mickeys.X]
		mov	di,[mickeys.Y]
		push	ds
		sti
		call	[UIR@]
		pop	ds
@@updateshow:	sti
		call	showcursor
@@updatedone:	inc	[UIRunlock]
		ret
mouseupdate	endp

updateposition	proc
		or	ax,ax
		jz	@@uposret
		mov	si,ax
		jg	@@resolute
		neg	si

@@resolute:	mov	dh,0
		dec	dx
                jz      @@newpos                
		inc	dx
                jz      @@resauto              

		cmp	si,3
                jb      @@newpos               
		cmp	si,8
                jae     @@resmul            
                shl     ax,1                    
		j	@@newpos

@@resauto:	cmp	si,8
                jb      @@newpos             
                mov     dl,10                  
                cmp     si,40                  
		jae	@@resmul
                mov     dx,si                   
                shr     dx,2                   

@@resmul:       imul    dx                      

@@newpos:	add	word ptr mickeys[bx],ax
		shl	ax,3
		add	ax,word ptr rounderr[bx]
		cwd
		idiv	word ptr mickey8[bx]
		mov	word ptr rounderr[bx],dx
		add	ax,word ptr pos[bx]

@savecutpos:	cmp	ax,word ptr rangemin[bx]
		jg	@@cuthi
		mov	ax,word ptr rangemin[bx]
@@cuthi:	cmp	ax,word ptr rangemax[bx]
		jl	@@savepos
		mov	ax,word ptr rangemax[bx]

@@savepos:      mov     word ptr pos[bx],ax     
		and	al,byte ptr granumask[bx]
                mov     word ptr granpos[bx],ax 
		mov	al,1
@@uposret:	ret
updateposition	endp

updatebutton	proc
		mov	si,offset DGROUP:buttpress
		shr	dl,1
                jnc     @@updset               
                shl     al,1                    
		mov	si,offset DGROUP:buttrelease
@@updset:	shr	dh,1
                jnc     @@updret                
		or	cl,al
		inc	[si+bx].counter
		mov	ax,[granpos.Y]
		mov	[si+bx].lastrow,ax
		mov	ax,[granpos.X]
		mov	[si+bx].lastcol,ax
@@updret:	add	bx,SIZE BUTTLASTSTATE
		ret
updatebutton	endp

		evendata
RILtable        dw      offset DGROUP:RIL_F0  
		dw	offset DGROUP:RIL_F1
		dw	offset DGROUP:RIL_F2
		dw	offset DGROUP:RIL_F3
		dw	offset DGROUP:RIL_F4
		dw	offset DGROUP:RIL_F5
		dw	offset DGROUP:RIL_F6
		dw	offset DGROUP:RIL_F7

int10handler	proc
		cld
                or      ah,ah                  
		jz	@@setmodreq
                cmp     ah,0F0h                
		jb	@@jmpold10
		cmp	ah,0F7h
		jbe	@@RIL
		cmp	ah,0FAh
		je	@@RIL_FA
@@jmpold10:     db      0EAh                    
oldint10	dd	?

@@setmodreq:	push	ax
                mov     al,2                 
                pushf                           
                push    cs                     
                call    handler33              
		pop	ax
		pushf
		call	cs:[oldint10]
		push	ds es
		PUSHALL
		movSeg	ds,cs
                mov     [cursorhidden],1       
		call	setupvideo
@@exitINT10:	jmp	@rethandler

@@RIL:		push	ds es
		PUSHALL
		movSeg	ds,cs
		mov	bp,sp
		mov	al,ah
                and     ax,0Fh                 
		mov	si,ax
		shl	si,1
		call	RILtable[si]
		j	@@exitINT10
@@RIL_FA:       movSeg  es,cs                   
		mov	bx,offset DGROUP:RILversion
		iret
int10handler	endp

RIL_F0		proc
		mov	si,dx
		mov	si,videoregs@[si].regs@
		cmp	dx,20h
                jae     @@retBL                
		add	si,bx
@@retBL:	lodsb
		mov	byte ptr [_ARG_BX_],al
		ret
RIL_F0		endp

RIL_F1		proc
		mov	ah,bl
		cmp	dx,20h
                jae     RILwrite                
                xchg    ax,bx                  
		mov	byte ptr [_ARG_BX_],ah
		;j	RILwrite
RIL_F1		endp

RILwrite	proc
		xor	bx,bx
		mov	di,dx
		cmp	dx,20h
		mov	dx,videoregs@[di].port@
                mov     videoregs@[di].rmodify?,dl 
		mov	di,videoregs@[di].regs@
                jae     @@RWsingle             
		mov	bl,al
@@RWsingle:	mov	[di+bx],ah
		jae	RILoutAH
		;j	RILgroupwrite
RILwrite	endp

RILgroupwrite	proc
		cmp	dl,0C0h
                je      @@writeATC             
		out	dx,ax
		ret
@@writeATC:	push	ax dx
		mov	dx,videoregs@[(SIZE RGROUPDEF)*5].port@
                in      al,dx                   
		pop	dx ax
                out     dx,al                 
RILoutAH:	xchg	al,ah
                out     dx,al                  
		xchg	al,ah
		ret
RILgroupwrite	endp

RIL_F2		proc
		sti
		mov	di,bx
		mov	si,dx
		mov	si,videoregs@[si].regs@
		mov	al,ch
		;mov	ah,0
		add	si,ax
RILmemcopy:	mov	ch,0
		shr	cx,1
		rep	movsw
		adc	cx,cx
		rep	movsb
		ret
RIL_F2		endp

RIL_F3		proc
		mov	di,dx
		mov	dx,videoregs@[di].port@
		mov	videoregs@[di].rmodify?,dl ; OPTIMIZE: DL instead 1
		mov	di,videoregs@[di].regs@
RILgrouploop:	xor	ax,ax
		xchg	al,ch
		add	di,ax
@@R3loop:	mov	ah,es:[bx]
		mov	[di],ah
		inc	bx
		inc	di
		call	RILgroupwrite
                inc     ax                     
		loop	@@R3loop
		ret
RIL_F3		endp

RIL_F4		proc
		sti
		mov	di,bx
@@R4loop:	mov	bx,es:[di]
		inc	di
		inc	di
		mov	bx,videoregs@[bx].regs@
		mov	al,es:[di]
		inc	di
		xlat
		stosb
		loop	@@R4loop
		ret
RIL_F4		endp

RIL_F5		proc
		mov	si,bx
@@R5loop:	lods	word ptr es:[si]
                xchg    dx,ax                   
		lods	word ptr es:[si]
		call	RILwrite
		loop	@@R5loop
		ret
RIL_F5		endp

RIL_F7		proc
		sti
		mov	si,bx
		mov	di,dx
		mov	cl,videoregs@[di].regscnt
                mov     videoregs@[di].rmodify?,cl 
		mov	di,videoregs@[di].def@
		push	es ds
		pop	es ds
		j	RILmemcopy
RIL_F7		endp

RIL_F6		proc
		movSeg	es,ds
		mov	si,offset DGROUP:videoregs@+(SIZE RGROUPDEF)*8

@@R6loop:	sub	si,SIZE RGROUPDEF
		xor	cx,cx
		xchg	cl,[si].rmodify?
		jcxz	@@R6next

		mov	bx,[si].def@
		mov	di,[si].regs@
		mov	dx,[si].port@
		mov	cl,[si].regscnt
                dec     cx                   
		jnz	@@R6group
		mov	al,[bx]
		stosb
		out	dx,al
                ;j      @@R6next               
                j       @@R6loop                

@@R6group:      inc     cx               
		;mov	ch,0
		call	RILgrouploop

@@R6next:	cmp	si,offset DGROUP:videoregs@
		ja	@@R6loop
		ret
RIL_F6		endp

drawcursor	proc
                jz      @graphcursor           
		call	gettxtoffset
                cmp     [cursortype],ch    
                jz      @@softcursor            

		shr	di,1
		mov	dx,videoregs@[0].port@	; CRTC port

		mov	ax,di
		mov	ah,al
		mov	al,0Fh
                out     dx,ax                 

                xchg    ax,di                  
		mov	al,0Eh
                out     dx,ax                  
		ret

@@softcursor:   jcxz    @@drawtext          
		cmp	di,word ptr cursorptr[0]
                je      @@drawtextdone         
		push	di
		call	restoretext
		pop	di

@@drawtext:     mov     ax,[granpos.Y]        
		mov	bx,[granpos.X]
                mov     si,8                   
		call	checkifseen
                jc      @@drawret            

		mov	word ptr cursorptr[0],di
		mov	es,word ptr cursorptr[2]
                mov     ax,es:[di]             
		mov	textbuf[0],ax
		and	ax,[startscan]
		xor	ax,[endscan]
                stosw                      
		mov	textbuf[2],ax

@@drawtextdone: inc     [cursordrawn]           
@@drawret:	ret

@graphcursor:   
		db	?
		call	updatevregs
		mov	bx,[granpos.X]
		sub	bx,[hotspot.X]
                and     bx,[granumask.X]       
		mov	ax,[granpos.Y]
                sub     ax,[hotspot.Y]       

		push	ax
		call	getgroffset
		pop	ax cx

                jcxz    @@drawgraph            
		cmp	bx,[cursorX]
		jne	@@restsprite
                db      081h,0FFh             
cursorrow	dw	?
                je      @@drawgrdone            

@@restsprite:	push	ax bx
		call	restoresprite
		pop	bx ax

@@drawgraph:	mov	[cursorX],bx
		mov	[cursorY],ax
		mov	[cursorrow],di
		mov	[nextrow],si

		db	0B1h			; MOV CL,byte
bitmapshift     db      ?                       ; mode 13h=1, 0Dh=4, otro=3
		shr	bx,cl
                shl     bx,cl               
                mov     si,16                  
		call	checkifseen
                jc      @@drawgrret             

                push    ax                     
                shr     dx,cl                   
		mov	ax,[scanline]
		cmp	dx,ax
		jbe	@@spritesize
		xchg	dx,ax			; DX=min(DX,scanline)
@@spritesize:   sar     bx,cl                   
		db	0B8h			; MOV AX,word
cursorX		dw	?
		sub	cl,3			; mode 0Dh=1
                sar     ax,cl                   
		neg	bx
		jge	@@savespritesz
		add	dx,bx
		sub	di,bx
		xor	bx,bx
                and     al,7                    
@@savespritesz:	mov	word ptr cursorptr[0],di
		mov	[spritewidth],dx
		neg	al
                add     al,8                    ; 8+|X| o -X%8
                cbw                             
		push	ax bx

                mov     al,0D6h              
                call    copysprite             

                pop     si cx ax                
		les	di,[cursorptr]
		;mov	dx,[nextrow]
                ;xor    bx,bx                  

@@makeloop:	cmp	ax,[maxcoordY]
                jae     @@makenexty             

		push	ax dx bx di
		call	makerow
		pop	di bx dx ax

@@makenexty:	inc	ax
		add	di,dx
		xor	dx,[nextxor]
		inc	bx
		inc	bx
		cmp	bx,2*16
		jb	@@makeloop
@@drawgrdone:	inc	[cursordrawn]
@@drawgrret:	;j	restorevregs
drawcursor	endp

restorevregs	proc
              
		db	?
		pop	es
		mov	bx,offset DGROUP:vdata1
		mov	cx,VDATA1cnt
		j	@writevideo
restorevregs	endp

updatevregs	proc
               
		db	?
		pop	es
		mov	bx,offset DGROUP:vdata1
		mov	cx,VDATA1cnt
                mov     ah,0F4h                
		int	10h

		mov	bx,offset DGROUP:vdata2
		mov	cx,VDATA2cnt
@writevideo:    mov     ah,0F5h                
		int	10h
		ret
updatevregs	endp

restorescreen	proc
                jcxz    @restret               
                jnz     restoretext           
		call	updatevregs
		call	restoresprite
		j	restorevregs
restorescreen	endp

restoretext	proc
		les	di,[cursorptr]
		mov	ax,es:[di]
		cmp	ax,textbuf[2]
		jne	@restret
		mov	ax,textbuf[0]
                stosw                          
@restret:	ret
restoretext	endp

restoresprite	proc
                mov     al,0D7h                
                ;j      copysprite             
restoresprite	endp

copysprite	proc
		push	si di ds
		cmp	al,0D6h
		mov	NEXTOFFSCODE[1],al
                db      0B8h                   
cursorY         dw      ?                    
                db      0BAh                  
nextrow         dw      ?                     
                mov     bx,16                  
		les	di,[buffer@]
		lds	si,[cursorptr]
		je	@@copyloop
		push	ds es
		pop	ds es
		xchg	si,di

@@copyloop:	push	dx
		db	03Dh			; CMP AX,word
maxcoordY       dw      ?                      
                jae     @@copynexty            

		db	0B9h			; MOV CX,word
spritewidth     dw      ?                      
		sub	dx,cx
		rep	movsb

@@copynexty:	inc	ax
NEXTOFFSCODE	db	01h,?			; ADD SI,DX/ADD DI,DX
		pop	dx
                db      081h,0F2h        
nextxor         dw      ?                       
		dec	bx
		jnz	@@copyloop
		pop	ds di si
		ret
copysprite	endp

makerow		proc
		mov	dx,word ptr screenmask[bx]
		mov	bx,word ptr cursormask[bx]
		mov	ax,[spritewidth]
		cmp	[videomode],13h
		jne	makerowno13
		mov	cx,si
		shl	dx,cl
		shl	bx,cl
                xchg    cx,ax                  

@@loopx13:	mov	al,0
		shl	dx,1
                jnc     @@chkcmask             
		mov	al,es:[di]
@@chkcmask:	shl	bx,1
                jnc     @@putbyte13            
		xor	al,0Fh
@@putbyte13:	stosb
		loop	@@loopx13
		ret

makerowno13:	push	cx
                xchg    si,ax                  
		mov	ax,0FFh
@@maskshift:	stc
		rcl	dx,1
                rcl     al,1                  
		shl	bx,1
                rcl     ah,1                   
		loop	@@maskshift
		xchg	dh,bl			; al:bl:dl - ah:bh:dh

@@loopxno13:	push	dx
                cmp     [videomode],0Dh         
                jae     @@planar               
		and	al,es:[di]
		xor	al,ah
		stosb
		j	@@nextxno13

@@planar:       xchg    cx,ax                 
		mov	dx,3CEh
                mov     ax,5                   
		out	dx,ax
                mov     ax,0803h               
		out	dx,ax
		xchg	es:[di],cl
                mov     ah,18h                 
		out	dx,ax
		xchg	es:[di],ch
		inc	di

@@nextxno13:    xchg    ax,bx               
		pop	bx
		dec	si
		jnz	@@loopxno13
		pop	cx
		ret
makerow		endp

		xor	di,di
		mov	es,di
		db	0BAh			; MOV DX,word
scanline	dw	?

getgroffset     proc
                mov     si,dx
		cmp	byte ptr cursorptr[3],0A0h
                je      @getoffsret             
		mov	si,2000h
                sar     ax,1                    
		jnc	@getoffsret
                mov     di,si                   
		mov	si,-(2000h-80)
		j	@getoffsret
getgroffset	endp

gettxtoffset	proc
		xor	dx,dx
		mov	es,dx
		mov	di,[granpos.X]
		mov	al,[bitmapshift]
                dec     ax                    
		xchg	cx,ax
                sar     di,cl                 
                xchg    cx,ax                 
		mov	ax,[granpos.Y]
                shr     ax,1                  
                shr     ax,1                   
                mov     dx,es:[44Ah]          

@getoffsret:	imul	dx
                add     ax,es:[44Eh]           
		add	di,ax
		ret
gettxtoffset	endp

checkifseen	proc
		db	0BAh			; MOV DX,word
cursorwidth	db	?,0

		add	si,ax
		jle	@@retunseen
		add	dx,bx
		jle	@@retunseen
		cmp	ax,[maxcoordY]
		jge	@@retunseen
		cmp	bx,[maxcoordX]
		jge	@@retunseen

		cmp	ax,[lowright.Y]
		jg	@@retseen
		cmp	bx,[lowright.X]
		jg	@@retseen
		cmp	si,[upleft.Y]
		jle	@@retseen
		cmp	dx,[upleft.X]
		jle	@@retseen

@@retunseen:	stc
		ret
@@retseen:	clc
		ret
checkifseen	endp


setupvideo	proc
                mov     si,LenClearArea2/2      
ERRIF (LenClearArea2 mod 2 ne 0) "LenClearArea2 must be even!"
		j	@setvideo
setupvideo	endp

softreset_21	proc
		db	0B8h			; MOV AX,word
buttonscnt      db      3,0                    
		mov	[_ARG_BX_],ax
		mov	[_ARG_AX_],0FFFFh

		movSeg	es,ds
		mov	si,offset DGROUP:StartDefArea
		mov	di,offset DGROUP:StartSaveArea
		mov	cx,LenDefArea/2
ERRIF (LenDefArea mod 2 ne 0) "LenDefArea must be even!"
                rep     movsw                  
		call	hidecursor
                mov     si,LenClearArea3/2     
ERRIF (LenClearArea3 mod 2 ne 0) "LenClearArea3 must be even!"

@setvideo:	push	si
		movSeg	es,ds
		xor	ax,ax
		mov	ds,ax
                mov     ax,ds:[463h]            
		mov	es:videoregs@[0].port@,ax ; 3D4h/3B4h
		add	ax,6
		mov	es:videoregs@[(SIZE RGROUPDEF)*5].port@,ax
                mov     al,ds:[449h]            
		push	ax

                cmp     al,11h              
		jb	@@checkv1
		add	al,9
		j	@@getshift

@@checkv1:      cmp     al,0Fh               
		jb	@@checkv2
                test    byte ptr ds:[487h],60h 
		jz	@@getshift
		add	al,2
		j	@@getshift

@@checkv2:      cmp     al,4                    
		jae	@@getshift
                mov     ah,ds:[488h]            
		and	ah,1Fh
		cmp	ah,3			; MDA+EGA/ECD?
		je	@@lines350
		cmp	ah,9			; EGA/ECD+MDA?
		jne	@@getshift
@@lines350:	add	al,13h

@@getshift:	lds	si,dword ptr ds:[4A8h]
		lds	si,dword ptr ds:[si]
                cbw                             
		mov	cl,6
		shl	ax,cl
                add     al,5                 
		add	si,ax			; SI += AL*64+5

		mov	di,offset DGROUP:StartDefVRegsArea
		push	di
		mov	al,3
		stosb				; def_SEQ[0]=3
		mov	cx,50/2
                rep     movsw                  
		mov	al,0
                stosb                        
                movsw                           
		movsw
		movsw
		movsw
		movsb
		stosw				; def_FC=0, def_GPOS1=0
                inc     ax                   
                stosb                          

		movSeg	ds,es
		pop	si
		mov	di,offset DGROUP:StartVRegsArea
		mov	cx,LenVRegsArea/2
ERRIF (LenVRegsArea mod 2 ne 0) "LenVRegsArea "
                rep     movsw                   

                dec     ax                     
		mov	cx,8
		mov	di,offset DGROUP:videoregs@[0].rmodify?
@@setmodify:	stosb
		add	di,(SIZE RGROUPDEF)-1
		loop	@@setmodify

                pop     ax                      
; mode 0-3
                mov     bx,0B8C3h              
		mov	cx,0304h		; 16x8: 0-1
		mov	dx,0FFFFh
		mov	di,200			; x200: 4-6, 0Dh-0Eh, 13h
		cmp	al,2
		jb	@@settext
		dec	cx			; 8x8: 2-3, 7
		cmp	al,4
		jb	@@settext
; mode 7
		cmp	al,7
		jne	@@checkgraph
		mov	bh,0B0h			; B000h: 7

@@settext:	mov	ch,1
		mov	dh,-8
		shl	dl,cl

		xor	ax,ax
		mov	es,ax
                add     al,es:[484h]           
		jz	@@screenw
                inc     ax                      
		mov	ah,8
		mul	ah
                xchg    di,ax                  

@@screenw:      mov     ax,es:[44Ah]           
		j	@@setcommon

@@checkgraph:   ;mov    cx,0303h               
		;mov	dx,0FFFFh		; 1x1: 6, 0Eh-12h
                ;mov    bx,0B8C3h              
		push	bx
		;mov	di,200			; x200: 4-6, 0Dh-0Eh, 13h
                mov     si,2000h xor -(2000h-80) 
		;movSeg	es,ds
		db	0BBh			; MOV BX,word
BUFFADDR	dw	offset DGROUP:bufferPS2
		cmp	al,6
		je	@@setgraphics
		jb	@@set2x1
		pop	bx
		mov	bh,0A0h			; A000h: 0Dh-
		mov	si,0A000h
		mov	es,si
                xor     si,si                   
		cmp	al,13h
		ja	@@nonstandard
		jb	@@mode0812
		push	bx
		mov	bx,0FA00h		; =320*200
                mov     cx,1000h              
@@set320:       inc     cx                    
@@set2x1:       dec     dx                   
		j	@@setgraphics
@@mode0812:	cmp	al,0Dh
		jb	@@nonstandard
                mov     bl,1Eh                
		push	bx
		mov	bx,3E82h		; 16002: 0Dh-0Eh
		je	@@set320
		cmp	al,0Fh
		jb	@@setgraphics
		mov	di,350			; x350: 0Fh-10h
		mov	bh,7Eh			; 32386: 0Fh-10h
		cmp	al,11h
		jb	@@setgraphics
		mov	di,480			; x480: 11h-12h
		mov	bh,9Eh			; 40578: 11h-12h

@@setgraphics:	mov	[videomode],al
		mov	[nextxor],si
		saveFAR	[buffer@],es,bx
                pop     ax                     
		mov	byte ptr [restorevregs],al
		mov	byte ptr [updatevregs],al
                xchg    bx,ax                  
                mov     al,51h                 
		j	@@setgcommon

@@nonstandard:
                mov     al,0C3h               

@@setgcommon:	mov	byte ptr [@graphcursor],al
                mov     ax,640                  
		shr	ax,cl
                mov     [scanline],ax           

@@setcommon:	shl	ax,cl
		mov	[maxcoordX],ax
		mov	[maxcoordY],di
		mov	byte ptr [granumask.X],dl
		mov	byte ptr [granumask.Y],dh
		mov	byte ptr cursorptr[3],bh
                mov     [bitmapshift],cl       
                                              
		shl	ch,cl
		mov	[cursorwidth],ch
		pop	si

		mov	cx,ax
		dec	ax
                mov     [rangemax.X],ax        
                shr     cx,1                   

		mov	dx,di
		dec	di
                mov     [rangemax.Y],di         
                shr     dx,1                   

@setpos:	cli
		movSeg	es,ds
		mov	di,offset DGROUP:StartClearArea
		xchg	cx,si
		xor	ax,ax
		rep	stosw

                xchg    ax,dx                   
		mov	bx,offset Y
		call	@savecutpos
                xchg    ax,si                   
                mov     bl,offset X             
		jmp	@savecutpos
softreset_21	endp

disabledrv_1F	proc
		les	ax,[oldint33]
		mov	[_ARG_ES_],es
		mov	[_ARG_BX_],ax

                db      0E8h                    
disableproc	dw	disablePS2-$-2

		mov	al,1
		cmp	[disabled?],al
		je	@@disabled
                mov     [cursorhidden],al      
		call	hidecursor

		mov	cx,cs
		mov	ax,3533h
		int	21h
		mov	dx,es
		cmp	dx,cx
		jne	althandler_18

                mov     al,10h                 
		int	21h
		mov	ax,es
		sub	ax,cx
		jne	althandler_18

                inc     ax                      
		mov	[disabled?],al

		mov	ax,2510h
		lds	dx,[oldint10]
		int	21h
@@disabled:	ret
disabledrv_1F	endp

althandler_18	proc
		mov	[_ARG_AX_],0FFFFh
		ret
althandler_18	endp

althandler_19	proc
		mov	[_ARG_CX_],0
		ret
althandler_19	endp

resetdriver_00	proc
		call	softreset_21
		;j	enabledriver_20
resetdriver_00	endp

enabledriver_20	proc
		cli
		xor	cx,cx
		mov	[buttstatus],cx
		xchg	cl,[disabled?]
		jcxz	@@enabled

		mov	ax,3510h
                int     21h                     
		saveFAR	[oldint10],es,bx
                mov     ah,25h                  
		mov	dx,offset DGROUP:int10handler
                int     21h                     
@@enabled:	call	setupvideo
                db      0E9h                   
enableproc	dw	enablePS2-$-2
enabledriver_20	endp

status_03	proc
		mov	ax,[buttstatus]
		mov	cx,[granpos.X]
		mov	dx,[granpos.Y]
		j	@retBCDX
status_03	endp

pressdata_05	proc
		mov	ax,offset DGROUP:buttpress
		j	@retbuttstat
pressdata_05	endp

releasedata_06	proc
		mov	ax,offset DGROUP:buttrelease
@retbuttstat:	and	bx,7
ERRIF (6 ne SIZE BUTTLASTSTATE) "BUTTLASTSTATE estructura cambiada"
		shl	bx,1
		add	ax,bx
		shl	bx,1
                add     bx,ax                   
		mov	ax,[buttstatus]
		mov	[_ARG_AX_],ax
		xor	ax,ax
		xchg	[bx.counter],ax
		mov	cx,[bx.lastcol]
		mov	dx,[bx.lastrow]
@retBCDX:	mov	[_ARG_CX_],cx
		mov	[_ARG_DX_],dx
@retBX:		mov	[_ARG_BX_],ax
		ret
releasedata_06	endp

mickeys_0B	proc
		xchg	ax,bx
		xor	cx,cx
		xor	dx,dx
		xchg	[mickeys.X],cx
		xchg	[mickeys.Y],dx
		j	@retBCDX
mickeys_0B	endp

storagereq_15	proc
		mov	ax,LenSaveArea
		j	@retBX
storagereq_15	endp

sensetivity_1B	proc
		mov	dx,64
		mov	ax,[mickey8.X]
		mov	cx,[mickey8.Y]
		j	@retBCDX
sensetivity_1B	endp

videopage_1E	proc
		xor	ax,ax
		mov	ds,ax
		mov	al,ds:[462h]
		j	@retBX
videopage_1E	endp

showcursor_01	proc
                mov     byte ptr lowright.Y[1],-1 
                cmp     [cursorhidden],ah      
                jz      @showcursor             
		dec	[cursorhidden]
		j	@showcheck
showcursor_01	endp

hidecursor_02	proc
		inc	[cursorhidden]
                jnz     hidecursor              
		dec	[cursorhidden]

hidecursor:	mov	dx,offset DGROUP:restorescreen
		j	@cursorproc
hidecursor_02	endp
hrange_07	proc
		mov	bx,offset X
		j	@setnewrange
hrange_07	endp
vrange_08	proc
		mov	bx,offset Y
@setnewrange:	cmp	cx,dx
		jle	@@saverange
		xchg	cx,dx
@@saverange:	mov	word ptr rangemin[bx],cx
		mov	word ptr rangemax[bx],dx
		mov	cx,[pos.X]
		mov	dx,[pos.Y]
		;j	setpos_04
vrange_08	endp

setpos_04	proc
                mov     si,LenClearArea1/2      
ERRIF (LenClearArea1 mod 2 ne 0) "LenClearArea1 debe existir"
		call	@setpos

showcursor:	cmp	[cursorhidden],0
@showcheck:     jnz     @showret               

@showcursor:	mov	dx,offset DGROUP:drawcursor
@cursorproc:	dec	[videounlock]
                jnz     @@showdone             
		xor	cx,cx
                xchg    cl,[cursordrawn]     
		mov	ax,[granumask.Y]
                inc     ax                    
                call    dx                    

@@showdone:     inc     [videounlock]          
@showret:	ret
setpos_04	endp

graphcursor_09	proc
		mov	si,offset DGROUP:hotspot
		lodsw
		cmp	ax,bx
		jne	@@redrawgraph
		lodsw
		xor	ax,cx
		jne	@@redrawgraph
		mov	di,dx
                mov     al,16+16             
		xchg	ax,cx
		repe	cmpsw
                xchg    cx,ax                   
		je	@showret

@@redrawgraph:	push	ds ds es
		pop	ds es
		mov	di,offset DGROUP:hotspot
                xchg    ax,bx                  
		stosw
                xchg    ax,cx                   
		stosw
		mov	si,dx
		mov	cx,16+16
		rep	movsw
		pop	ds

redrawscreen:	call	hidecursor
		j	showcursor
graphcursor_09	endp

textcursor_0A	proc
		or	bl,bh
                jz      @@difftext          

		push	cx
		mov	ch,cl
		mov	cl,dl
		mov	ah,1
                int     10h                    
		pop	cx
		mov	bl,1

@@difftext:	cmp	bl,[cursortype]
		jne	@@redrawtext
		cmp	cx,[startscan]
		jne	@@redrawtext
		cmp	dx,[endscan]
                je      @showret         

@@redrawtext:	mov	[cursortype],bl
		mov	[startscan],cx
		mov	[endscan],dx
		j	redrawscreen
textcursor_0A	endp

updateregion_10	proc
		mov	ax,[_ARG_SI_]
		cmp	cx,ax
		jle	@@regionX
		xchg	cx,ax
@@regionX:	mov	[upleft.X],cx
		mov	[lowright.X],ax
                xchg    ax,di                 
		cmp	dx,ax
		jle	@@regionY
		xchg	dx,ax
@@regionY:	mov	[upleft.Y],dx
		mov	[lowright.Y],ax
		j	redrawscreen
updateregion_10	endp

savestate_16	proc
		mov	di,dx
		mov	si,offset DGROUP:StartSaveArea
		mov	cx,LenSaveArea/2
ERRIF (LenSaveArea mod 2 ne 0) "LenSaveArea debe existir"
		rep	movsw
@stateret:	ret
savestate_16	endp

restorestate_17	proc
ERRIF (LenSaveArea mod 2 ne 0) "LenSaveArea debe ser cargada"

		push	es dx
		call	hidecursor
		movSeg	es,ds
		pop	si ds
		mov	di,offset DGROUP:StartSaveArea
		mov	cx,LenSaveArea/2
ERRIF (LenSaveArea mod 2 ne 0) "LenSaveArea debe ser cargada"
		rep	movsw
		jmp	showcursor
restorestate_17	endp

exchangeUIR_14	proc
		mov	al,[callmask]
		;mov	ah,0
		mov	[_ARG_CX_],ax
		mov	ax,word ptr UIR@[0]
		mov	[_ARG_DX_],ax
		mov	ax,word ptr UIR@[2]
		mov	[_ARG_ES_],ax
		;j	UIR_0C
exchangeUIR_14	endp

UIR_0C		proc
		saveFAR [UIR@],es,dx
		mov	[callmask],cl
		ret
UIR_0C		endp

sensetivity_0F	proc
		mov	bx,cx
		mov	cx,dx
;;+*		mov	dx,[doublespeed]
		;j	sensetivity_1A
sensetivity_0F	endp

sensetivity_1A	proc
		or	bx,bx
		jnz	@@setmickX
		inc	bx
@@setmickX:	mov	[mickey8.X],bx
		or	cx,cx
		jnz	@@setmickY
		inc	cx
@@setmickY:	mov	[mickey8.Y],cx
		;j	doublespeed_13
sensetivity_1A	endp

doublespeed_13	proc
doublespeed_13	endp

nullfunc	proc
		ret
nullfunc	endp

		evendata
handler33table	dw	offset DGROUP:resetdriver_00
		dw	offset DGROUP:showcursor_01
		dw	offset DGROUP:hidecursor_02
		dw	offset DGROUP:status_03
		dw	offset DGROUP:setpos_04
		dw	offset DGROUP:pressdata_05
		dw	offset DGROUP:releasedata_06
		dw	offset DGROUP:hrange_07
		dw	offset DGROUP:vrange_08
		dw	offset DGROUP:graphcursor_09
		dw	offset DGROUP:textcursor_0A
		dw	offset DGROUP:mickeys_0B
		dw	offset DGROUP:UIR_0C
                dw      offset DGROUP:nullfunc  
                dw      offset DGROUP:nullfunc 
		dw	offset DGROUP:sensetivity_0F
		dw	offset DGROUP:updateregion_10
                dw      offset DGROUP:nullfunc 
                dw      offset DGROUP:nullfunc  
		dw	offset DGROUP:doublespeed_13
		dw	offset DGROUP:exchangeUIR_14
		dw	offset DGROUP:storagereq_15
		dw	offset DGROUP:savestate_16
		dw	offset DGROUP:restorestate_17
		dw	offset DGROUP:althandler_18
		dw	offset DGROUP:althandler_19
		dw	offset DGROUP:sensetivity_1A
		dw	offset DGROUP:sensetivity_1B
                dw      offset DGROUP:nullfunc  
                dw      offset DGROUP:nullfunc  
		dw	offset DGROUP:videopage_1E
		dw	offset DGROUP:disabledrv_1F
		dw	offset DGROUP:enabledriver_20
		dw	offset DGROUP:softreset_21

handler33	proc
		sti
		cld
		or	ah,ah
		jnz	@@iret
		push	ds
		movSeg	ds,cs
		cmp	al,21h
		ja	language_23
		push	es
		PUSHALL
		mov	bp,sp
                mov     si,ax                 
		shl	si,1
                call    handler33table[si]      
@rethandler:	POPALL
		pop	es ds
@@iret:		iret

language_23:	cmp	al,23h
		je	@@iretBX0

version_24:	cmp	al,24h
		jne	no_version_24
		mov	bx,driverversion
		db	0B9h			; MOV CX,word
mouseinfo	db	?,4
no_version_24:

maxscreen_26:	cmp	al,26h
		jne	no_maxscreen_26
		db	0BBh			; MOV BX,word
disabled?       db      1,0                    
		db	0B9h			; MOV CX,word
maxcoordX       dw      ?                      
		mov	dx,[maxcoordY]
		dec	cx
		dec	dx
no_maxscreen_26:

active_32:	cmp	al,32h
		jne	no_active_32
                mov     ax,0100010000000100b   
		xor	cx,cx
		xor	dx,dx
@@iretBX0:	xor	bx,bx
		pop	ds
		iret
no_active_32:

copyright_4D:	cmp	al,4Dh
		jne	no_copyright_4D
		movSeg	es,ds
		mov	di,offset DGROUP:IDstring
no_copyright_4D:

version_6D:	cmp	al,6Dh
		jne	no_version_6D
		movSeg	es,ds
		mov	di,offset DGROUP:msversion
no_version_6D:

hotspot_2A:	cmp	al,2Ah
		jne	@@iretDS
		mov	al,[cursorhidden]
		mov	bx,[hotspot.X]
		mov	cx,[hotspot.Y]
		db	0BAh			; MOV DX,word
mouseinfo1	db	4,0
@@iretDS:	pop	ds
		iret
handler33	endp



RILversion	label
msversion	db	driverversion / 100h,driverversion MOD 100h
IDstring        db      'Mouse',0
IDstringlen = $ - IDstring

EndTSRpart	label



.data

paragraphs	dw	0
SaveMemStrat	dw	0
SaveUMBLink	dw	0
XMSentry	dd	0

mousetype	db	?			; 0=PS/2,1=MSys,2=LT,3=MS,4=WM
options		db	0
OPT_PS2		equ	00000001b
OPT_SERIAL	equ	00000010b
OPT_COMforced	equ	00000100b
OPT_PS2after	equ	00001000b
OPT_3button	equ	00010000b
OPT_NOMSYS	equ	00100000b
OPT_LEFTHAND	equ	01000000b
OPT_NOUMB	equ	10000000b

OPTION		struc
  C		db	?
  M		db	0
  PROC@		dw	?
	ends

		evendata
OPTABLE		OPTION	{C='P',M=OPT_PS2,	PROC@=@ret}
		OPTION	{C='S',M=OPT_SERIAL,	PROC@=_serialopt}
;;+		OPTION	{C='A',M=OPT_SERIAL+OPT_COMforced,PROC@=_addropt}
		OPTION	{C='Y',M=OPT_NOMSYS,	PROC@=@ret}
		OPTION	{C='V',M=OPT_PS2after,	PROC@=@ret}
		OPTION	{C='3' and not 20h,M=OPT_3button,PROC@=@ret}
		OPTION	{C='R',			PROC@=_resolution}
		OPTION	{C='L',M=OPT_LEFTHAND,	PROC@=@ret}
		OPTION	{C='B',			PROC@=_checkdriver}
		OPTION	{C='W',M=OPT_NOUMB,	PROC@=@ret}
		OPTION	{C='U',			PROC@=unloadTSR}
		OPTION	{C='?' and not 20h,	PROC@=EXITMSG}
OPTABLEend	label

INCLUDE mouse.msg

.code

real_start:	cld
		mov	ax,3533h
		int	21h
		saveFAR [oldint33],es,bx

@@start:        PrintS  Copyright              
		mov	si,80h
		lodsb
                cbw                          
		mov	bx,ax
                mov     [si+bx],ah           
                call    commandline          

		mov	cx,word ptr oldint33[2]
		jcxz	@@startfind
                mov     ax,1Fh                
                pushf                         
                call    [oldint33]            
@@startfind:	mov	al,[options]
		test	al,OPT_PS2+OPT_SERIAL
		jnz	@@find1COM
		or	al,OPT_PS2+OPT_SERIAL

@@find1COM:	test	al,OPT_PS2after
		jz	@@findPS2
		push	ax
		call	searchCOM
		pop	ax
		jnc	@@serialfound

@@findPS2:	test	al,OPT_PS2+OPT_PS2after
		jz	@@find2COM
		push	ax
		mov	al,0
		mov	mouseinfo[0],al
		mov	[mousetype],al
		call	checkPS2
		pop	ax
		jnc	@@mousefound

@@find2COM:	test	al,OPT_PS2after
		jnz	@@devnotfound
		test	al,OPT_SERIAL+OPT_NOMSYS
		jz	@@devnotfound
                call    searchCOM              
		jnc	@@serialfound

@@devnotfound:  mov     dx,offset DGROUP:E_notfound 
		jmp	EXITENABLE

@@serialfound:	mov	al,2
		mov	mouseinfo[1],al
		mov	[mouseinfo1],al

@@mousefound:	call	getCuteMouse
                mov     dx,offset DGROUP:S_reset 
                mov     bl,2                  
		je	@@setupdriver

                call    prepareTSR            
		mov	si,offset DGROUP:oldint33
		mov	di,si
                movsw                           
		movsw
		push	ds
		movSeg	ds,es
                mov     [disabled?],1         
                mov     ax,2533h               
		mov	dx,offset DGROUP:handler33
                int     21h                   
		pop	ds
                mov     dx,offset DGROUP:S_installed 
                mov     bl,0                    
@@setupdriver:	PrintS
		mov	cl,[mousetype]
		mov	dx,offset DGROUP:S_forPS2 ; 'PS/2 port'
		or	cl,cl
		jz	@@printmode

		PrintS	S_atCOM
                mov     dx,offset DGROUP:S_forWM 
		cmp	cl,4
		je	@@printmode
                mov     dx,offset DGROUP:S_forMS 
		cmp	cl,2
		ja	@@printmode
                mov     dx,offset DGROUP:S_forLT 
		je	@@printmode
                mov     dx,offset DGROUP:S_forMSYS 
                inc     bx                     

@@printmode:	mov	exitfunc[0],bl
		PrintS
		call	setupdriver

		mov	ax,es
		mov	bx,cs
		cmp	ax,bx
                jne     @@exit                

                mov     es,ds:[2Ch]             
		mov	ah,49h
		int	21h

		mov	dx,offset DGROUP:EndTSRpart+15
		mov	cl,4
		shr	dx,cl
                mov     exitfunc[1],31h        
@@exit:		jmp	EXIT

setupdriver	proc
		;mov	cl,[mousetype]

		mov	ax,1A00h
                int     10h                     
		cmp	al,1Ah
		jne	@@lhand
                xchg    ax,bx                   
                cmp     al,7                   
		je	@@setATCVGA
                cmp     al,8                  
		jne	@@lhand
@@setATCVGA:	inc	videoregs@[(SIZE RGROUPDEF)*3].regscnt

@@lhand:	mov	al,0EBh			; JMP SHORT
		mov	bl,07Ah			; JPE
		or	cl,cl
                jz      @@checklhand          
		xchg	ax,bx
@@checklhand:	test	[options],OPT_LEFTHAND
		jz	@@setlhand
		xchg	ax,bx
@@setlhand:	mov	[LEFTHANDCODE],al

		mov	al,3
		test	[options],OPT_3button
		jnz	@@sethandler
		or	cl,cl
                je      @@setbuttons            
		cmp	cl,3
                jne     @@sethandler           
@@setbuttons:   mov     [buttonsmask],al        
                dec     ax                      
                mov     [buttonscnt],al        

@@sethandler:	cmp	cl,1
                jb      @@setother            

                mov     word ptr [IRQproc],20B0h 
		mov	[BUFFADDR],offset DGROUP:bufferSERIAL
		mov	[enableproc],enableCOM-enableproc-2
		mov	[disableproc],disableCOM-disableproc-2
                je      @@setother             
		mov	[mouseproc],MSLTproc-mouseproc-2
		cmp	cl,3
                jb      @@setother            
		mov	MSLTCODE3[1],2
                ja      @@setother              

		cmp	al,2
                je      @@setMSproc             
		mov	[MSLTCODE2],075h	; JNZ
@@setMSproc:    mov     al,0C3h                 
		mov	[MSLTCODE1],al
		mov	[MSLTCODE3],al

@@setother:	push	es ds es ds
		pop	es ds
		mov	si,offset DGROUP:oldint10
		mov	di,si
                movsw                         
		movsw
		mov	al,[disabled?]
		pop	ds
                mov     [disabled?],al          

                mov     al,byte ptr [IRQintnum] 
		mov	ah,35h
                int     21h                    
		mov	ax,es
		pop	es
		mov	di,offset DGROUP:oldIRQaddr
		xchg	ax,bx
		stosw
                xchg    ax,bx            
		stosw

		mov	si,offset DGROUP:StartTSRpart
		mov	di,si
		mov	cx,(EndTSRpart+1-StartTSRpart)/2
		rep	movsw

		pop	ax
                pushf                          
                push    cs ax                
		mov	ax,offset DGROUP:handler33
		push	es ax
                xor     ax,ax                  
		retf
setupdriver	endp

searchCOM	proc
		;mov	[COMLCR],2
		mov	dx,offset DGROUP:detectmouse
		call	COMloop
		jnc	@searchret

		test	[options],OPT_NOMSYS
		stc
		jnz	@searchret

		mov	[COMLCR],3
                mov     [mousetype],1           
		mov	dx,offset DGROUP:COMexist?
		;j	COMloop
searchCOM	endp

COMloop		proc
                mov     cx,1             
		test	[options],OPT_COMforced
		jnz	@@checkCOM
                mov     cl,4                   

@@COMloop:	push	cx
		mov	al,'5'
		sub	al,cl
		call	setCOMport
		pop	cx
@@checkCOM:	push	cx dx
		mov	si,[IO_address]
		call	dx
		pop	dx cx
		jnc	@searchret
		loop	@@COMloop
@searchret:	ret
COMloop		endp

COMexist?	proc
		or	si,si
		jz	@@noUART

		mov	dx,si
                add     dx,4               
                in      ax,dx                  
		test	al,11100000b
                jnz     @@noUART             
		inc	dx
                in      al,dx                  
		inc	ax
		jz	@@noUART		; AX=0FFFFh

		dec	dx
		dec	dx
		mov	al,00011011b		; =1Bh
		cli
                out     dx,al                  
                in      al,dx                
		mov	ah,al
		mov	al,00000011b		; =3
                out     dx,al                 
                in      al,dx               
		sti
		cmp	ax,0001101100000011b
		jne	@@noUART

		dec	dx
		dec	dx
                in      al,dx                   
		test	al,11110000b
		;clc
                jz      @@UARTret               

@@noUART:	stc
@@UARTret:	ret
COMexist?	endp

detectmouse	proc
		call	COMexist?
		jc	@searchret

		mov	dx,si
		add	dx,3
		mov	al,80h
                out     dx,al                  
		xchg	dx,si
                mov     ax,96             
                out     dx,ax                   

		xchg	dx,si
		mov	al,[COMLCR]
                out     dx,al                
                                               
		inc	dx
		push	dx
                inc     dx                     
                in      ax,dx                  
		mov	dx,si
                in      al,dx                   

		inc	dx
                xor     ax,ax                   ; {3F9h} IER
                out     dx,ax                   ; {3FAh} FCR
                pop     dx                      ; {3FCh} MCR 
                call    pulseRTS              

		mov	bx,301h			; bl=no MS, bh=mousetype
                mov     cx,4                   
		inc	dx			; DX=3FDh, SI=3F8h
@@detectloop:	call	readCOMbyte
		jc	@@detdone
		cmp	al,8
                je      @@skipdata              
		cmp	al,'M'
		jne	@@checkWM
                mov     bl,0                   
@@checkWM:	cmp	al,'Z'
		jne	@@checkLT
                mov     bh,4                  
@@checkLT:	cmp	al,'3'
		jne	@@detectnext
                mov     bh,2                   
@@detectnext:	loop	@@detectloop

@@skipdata:	call	readCOMbyte
		jnc	@@skipdata

@@detdone:      mov     [mousetype],bh         
                neg     bl                      
		ret
detectmouse	endp

pulseRTS	proc
                xor     ax,ax
		mov	es,ax
                call    @@tickOUT             
                mov     al,3                   

@@tickOUT:	mov	di,es:[46Ch]
@@tickwait:	cmp	di,es:[46Ch]
                je      @@tickwait           
                out     dx,al                  
		ret
pulseRTS	endp

readCOMbyte	proc
                mov     ah,2+1             
                                              
@@timeloop:	mov	di,es:[46Ch]
@@waitloop:     in      al,dx              
		test	al,1
                jnz     @@readret               
		cmp	di,es:[46Ch]
                je      @@waitloop            
		dec	ah
                jnz     @@timeloop           
@retC:		stc
		ret
@@readret:	xchg	dx,si
                in      al,dx                   
		xchg	dx,si
		;clc
		ret
readCOMbyte	endp

checkPS2	proc
		mov	bh,3
                PS2serv 0C205h,@retC         
		mov	bh,3
                PS2serv 0C203h,@retC            
		movSeg	es,cs
		mov	bx,offset DGROUP:IRQhandler
                PS2serv 0C207h,@retC           
		xor	bx,bx
		mov	es,bx
                PS2serv 0C207h                
		;clc
		ret
checkPS2	endp

setCOMport	proc
		mov	[com_port],al
		sub	al,'1'

		xor	bx,bx
		mov	es,bx
		mov	bl,al

		shr	al,1			; 0=COM1, 1=COM2, ...
                mov     al,'4'                  ; IRQ4 para COM1/3
                sbb     al,0                    ; IRQ3 para COM2/4
		call	setIRQ

		shl	bx,1
		mov	ax,es:400h[bx]
		movSeg	es,ds
		mov	di,offset DGROUP:S_atIO
		;j	setIOaddr
setCOMport	endp

setIOaddr	proc
		mov	[IO_address],ax
		mov	cl,12
@@loophex:	push	ax
		shr	ax,cl
		and	al,0Fh
		cmp	al,10
		sbb	al,69h
		das
		stosb
		pop	ax
		sub	cl,4
		jae	@@loophex
		ret
setIOaddr	endp

setIRQ		proc
		mov	[IRQno],al
		sub	al,'0'
		mov	mouseinfo[0],al
		mov	cl,al
		add	al,8
		mov	byte ptr [IRQintnum],al
		mov	al,1
		shl	al,cl
                mov     [PICstate],al          
		not	al
                mov     [notPICstate],al       
		ret
setIRQ		endp

; CHEQUEA SI EL DRIVER ESTA EN LA MEMORIA ---------------------------------

getCuteMouse	proc
		mov	ax,word ptr oldint33[2]
		neg	ax
		sbb	ax,ax
		inc	ax
                jne     @ret                   
                mov     al,4Dh                
                pushf                        
                call    [oldint33]             
		cmp	di,offset DGROUP:IDstring
		jne	@ret
		mov	si,di
		mov	cx,IDstringlen
		rep	cmpsb
@ret:		ret
getCuteMouse	endp


; LINEA DE COMANDOS -------------------------------------------------------

; FACE SERIAL -------------------------------------------------------------

_serialopt	proc
		lodsb
		cmp	al,'1'
		jb	@@fixret
		cmp	al,'4'
		ja	@@fixret
		or	[options],OPT_COMforced
                call    setCOMport             
		lodsb
		cmp	al,'2'
		jb	@@fixret
		cmp	al,'7'
                jbe     setIRQ                
@@fixret:       dec     si                     
		ret
_serialopt	endp

; RESOLUCION --------------------------------------------------------------

_resolution	proc
		;mov	ah,0
                lodsb                          
		sub	al,'0'
		cmp	al,9
                ja      @@fixupres             

		mov	ah,al
                lodsb                        
		sub	al,'0'
		cmp	al,9
                jbe     @@setres              

@@fixupres:     dec     si                  
                mov     al,ah                  
@@setres:	mov	[mresolutionX],ah
		mov	[mresolutionY],al
		ret
_resolution	endp

; CHEQUEO DE SERVICIOS ----------------------------------------------------

_checkdriver	proc
		mov	cx,word ptr oldint33[2]
		jcxz	@ret
		;mov	ah,0
                mov     al,21h                 
		int	33h
		inc	ax
		jnz	@ret
                mov     dx,offset DGROUP:E_mousepresent 
		j	EXITMSG
_checkdriver	endp

commandline	proc
		lodsb
		or	al,al
                jz      @ret                   
		cmp	al,' '
                jbe     commandline             
                cmp     al,'/'                  
		jne	@@badoption
		lodsb
                and     al,not 20h            

                mov     dx,offset DGROUP:Syntax 
		mov	bx,offset DGROUP:OPTABLE
@@optloop:	cmp	al,[bx].C
		jne	@@nextopt
		mov	ah,[bx].M
		or	[options],ah
		call	[bx].PROC@
		j	commandline
@@nextopt:	add	bx,SIZE OPTION
		cmp	bx,offset DGROUP:OPTABLEend
		jb	@@optloop

@@badoption:    mov     dx,offset DGROUP:E_option 

EXITMSG:	mov	bx,dx
		mov	al,[bx]
		mov	exitfunc[0],al
		inc	dx
		PrintS
EXIT:		db	0B8h			; MOV AX,word
exitfunc        db      ?,4Ch                   
		int	21h
commandline	endp


; ADMINISTRADOR TSR -------------------------------------------------------

; DESCARGA DE DRIVER ------------------------------------------------------

unloadTSR	proc
                call    getCuteMouse           
                mov     dx,offset DGROUP:E_nocute 
		jne	EXITMSG

		push	es
                mov     ax,1Fh                 
                pushf                          
                call    [oldint33]            
		mov	cx,es
		pop	es
		cmp	al,1Fh
                mov     dx,offset DGROUP:E_notunload 
		jne	@@exitenable

		saveFAR	[oldint33],cx,bx
		push	ds
		mov	ds,cx
		mov	dx,bx
                mov     ax,2533h               
                int     21h                  
		pop	ds
		call	FreeMem
                mov     dx,offset DGROUP:S_unloaded 
EXITENABLE:	mov	cx,word ptr oldint33[2]
                jcxz    EXITMSG                
@@exitenable:   mov     ax,20h                 
                pushf                         
                call    [oldint33]              
		j	EXITMSG
unloadTSR	endp

; -------------------------------------------------------------------------

prepareTSR	proc
                mov     ax,offset DGROUP:EndTSRpart   
                call    AllocUMB                      
		mov	ax,es
		mov	bx,cs
		cmp	ax,bx
                je      @@prepareret            

		push	ds
		dec	ax
                mov     es,ax                  
		inc	ax
                mov     es:[1],ax             
		dec	bx
                mov     ds,bx                
		mov	si,8
		mov	di,si
                movsw                          
		movsw
		movsw
		movsw
		pop	ds

		mov	bx,ax
                add     bx,es:[3]               
                mov     es,ax                  
                xchg    dx,ax                 
		mov	ah,26h
                int     21h                  
                mov     es:[2],bx           
@@prepareret:	ret
prepareTSR	endp

; LLAMADA DE MEMORIA ------------------------------------------------------

getXMSaddr	proc
		push	es
		xor	bx,bx
		mov	es,bx
                mov     ax,4310h               
		int	2Fh
		mov	ax,es
		saveFAR [XMSentry],ax,bx
                or      ax,bx                  
		pop	es
		ret
getXMSaddr	endp

SaveStrategy	proc
		mov	ax,5800h
                int     21h                    
		mov	[SaveMemStrat],ax
		mov	ax,5802h
                int     21h                  
		mov	byte ptr [SaveUMBLink],al
		ret
SaveStrategy	endp

RestoreStrategy	proc
		mov	bx,[SaveMemStrat]
		mov	ax,5801h
                int     21h                     
		mov	bx,[SaveUMBLink]
		mov	ax,5803h
                int     21h                   
		ret
RestoreStrategy	endp

; ALOJA MEMORIA UMB -------------------------------------------------------

AllocUMB	proc
		add	ax,15
		mov	cl,4
		shr	ax,cl
		mov	[paragraphs],ax

		test	[options],OPT_NOUMB
                jnz     @@allocasis            
		mov	ax,cs
		cmp	ah,0A0h
                jae     @@allocasis           

; CHEQUEO DE TIPO DE UMB --------------------------------------------------
		call	SaveStrategy
                mov     bx,1                    
		mov	ax,5803h
                int     21h                    

                mov     bl,41h               
                mov     ax,5801h               
                int     21h                   
		jnc	@@allocDOSUMB

                mov     bl,81h                  
                mov     ax,5801h                
                int     21h                    

@@allocDOSUMB:	mov	bx,[paragraphs]
		mov	ah,48h
                int     21h                   
		pushf
		push	ax
                call    RestoreStrategy        
		pop	ax
		popf
		jc	@@allocXMSUMB
		mov	es,ax
                cmp     ah,0A0h                
                jae     @@allocret             
                mov     ah,49h                 
		int	21h

; EMPLEO DE XMS EN VES DE UMB ---------------------------------------------
@@allocXMSUMB:	call	getXMSaddr
		jz	@@allocasis
                mov     ah,10h                  
		mov	dx,[paragraphs]
		call	[XMSentry]
		or	ax,ax
		jz	@@allocasis
                mov     es,bx                  
		cmp	dx,[paragraphs]
		jae	@@allocret
		mov	dx,es
                mov     ah,11h                 
		call	[XMSentry]

; SEGMENTACION ------------------------------------------------------------
@@allocasis:	movSeg	es,cs

@@allocret:	ret
AllocUMB	endp

FreeMem		proc
		call	getXMSaddr
		jz	@@DOSfree

		mov	dx,es
                mov     ah,11h                   
		call	[XMSentry]

@@DOSfree:	mov	ah,49h
                int     21h                     
		ret
FreeMem		endp

; -------------------------------------------------------------------------
		end	start
