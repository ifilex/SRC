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

MAX_EXTRA_BYTES = 3     
MIN_GOOD_SYNCS  = 5    
MAX_BAD_SYNCS   = 20   
;


movSeg		macro	dest, src
		push	src
		pop	dest
	endm

saveFAR		macro	addr, segm, offs
		mov	word ptr addr,offs
		mov	word ptr addr[2],segm
	endm

IFz		macro	var, addr
		cmp	var,0
		jz	addr
	endm

IFnz		macro	var, addr
		cmp	var,0
		jnz	addr
	endm

PS2serv		macro	serv, errlabel
		mov	ax,serv
		int	15h
		jc	errlabel
		or	ah,ah
		jne	errlabel
	endm

driverversion   equ     626h  

.model tiny                             
.code
.286
		org	100h

start:		jmp	real_start

oldint10        dd      0              
oldint33        dd      0         
oldIRQaddr      dd      0            
newIRQaddr      dd      0            
userproc        dd      0              

mem_type        db      0       

StartSaveArea = $

int10pointer    dd      0     
Xmovement	dw	0
Ymovement	dw	0

disabled?       db      0       

StartInitArea = $

nowdrawing?     db      0     
userproc?       db      0      
regionchk?      db      0      

callmask        dw      0     
hrangemin       dw      0      
vrangemin       dw      0    
hrangemax       dw      0     
vrangemax       dw      0     
Xcalc		dw	0
Ycalc		dw	0
upleftx         dw      0      
uplefty         dw      0     
lowrightx       dw      0     
lowrighty       dw      0     
mickeyX		dw	0
mickeyY		dw	0

newbuttstat	dw	0
buttstatus      dw      0       
butt1press      dw      0      
butt1pc         dw      0      
butt1pr         dw      0       
butt1rel        dw      0       
butt1rc         dw      0       
butt1rr         dw      0       
butt3press	dw	0
butt3pc		dw	0
butt3pr		dw	0
butt3rel	dw	0
butt3rc		dw	0
butt3rr		dw	0
butt2press	dw	0
butt2pc		dw	0
butt2pr		dw	0
butt2rel	dw	0
butt2rc		dw	0
butt2rr		dw	0

cursortype      db      0      

LenInitArea = $ - StartInitArea

column		dw	320
row		dw	64h
column2		dw	320
row2		dw	60h
Xcoord          dw      0      
Ycoord          dw      0    

autores?        db      0    
lefthand?       db      0  

Xcoordold       dw      320    
Ycoordold       dw      60h    
h8mickey        dw      8       
v8mickey        dw      10h    
mul2indic	db	0
hspotcol        dw      0     
hspotrow        dw      0     
treshspeed      dw      2       
lightpen?       db      1       
startscan       dw      0FFFFh  
endscan         dw      7700h   
restoreindic    db      1     
Xcheck		db	0
screenmask      db      48 dup (0)   
cursormask      db      48 dup (0)   
grbuf1          dw      72 dup (0)     
grbuf2          dw      72 dup (0)     
activepage      db      0FFh         
vidmemoffs	dw	0
datav1		db	7 dup (0)
datav1b		db	0,0
mapmask		db	0
datav2		db	5 dup (0)
graphwritemod	db	1
datav3		db	0Fh,0FFh
graphsegindic   db      0               
VGAindicat      db      0             
EGAport         dw      3D4h            
graphmodeseg    dw      0               
ptr2gbuf2       dw      offset grbuf2   
ptr2gbuf1       dw      offset grbuf1 
cursorshown     db      0FFh           
rowcheck	db	0

mresolution     dw      0303h          
IRQintnum       db      0Ch            

IFNDEF PS2
COM_incoming    dw      0              
syncCount       dw      0              
tmpButStat      dw      0              
extraBytes      dw      0              
errorCont       dw      0               
IO_number       dw      3F8h         
PICstate        db      10h           
forced          db      0               
noMouseSyst     db      0               
MSCbuttstate    dw      0              
logitech?       db      0              
extrabyte       db      0              
LGbstat         db      0               
X_LO            db      0          

ELSE
newPS2data	db	0
ENDIF

mousetype       db      2               

shape		dw	03FFFh, 01FFFh, 00FFFh, 007FFh
		dw	003FFh, 001FFh, 000FFh, 0007Fh
		dw	0003Fh, 0001Fh, 001FFh, 000FFh
		dw	030FFh, 0F87Fh, 0F87Fh, 0FCFFh
		dw	00000h, 04000h, 06000h, 07000h
		dw	07800h, 07C00h, 07E00h, 07F00h
		dw	07F80h, 07C00h, 06C00h, 04600h
		dw	00600h, 00300h, 00300h, 00000h

LenSaveArea = $ - StartSaveArea

butpresstatus   dw      offset butt1press    
                dw      offset butt2press     
                dw      offset butt3press     
                dw      offset butt1press     

butrelstatus    dw      offset butt1rel         
                dw      offset butt2rel       
                dw      offset butt3rel         
                dw      offset butt1rel        

crightmsg       db      'Mouse Driver 1.2',0
msversion	db	6,26h

;----------------------------

funcsoffsets	dw	offset resetdriver_00
		dw	offset showcursor_01
		dw	offset hidecursor_02
		dw	offset status_03
		dw	offset setpos_04
		dw	offset butpresdata_05
		dw	offset buttreldata_06
		dw	offset hrange_07
		dw	offset vrange_08
		dw	offset graphcursor_09
		dw	offset textcursor_0A
		dw	offset readmcounter_0B
		dw	offset intpar_0C
		dw	offset lightpenon_0D
		dw	offset lightpenoff_0E
		dw	offset micperpixel_0F
		dw	offset defregion_10
                dw      offset nullfunc         
                dw      offset nullfunc       
		dw	offset doublespeed_13
		dw	offset exchangeint_14
		dw	offset storagereq_15
		dw	offset savestate_16
		dw	offset restorestate_17
                dw      offset nullfunc        
                dw      offset nullfunc        
		dw	offset setsens_1A
		dw	offset getsens_1B
                dw      offset nullfunc     
		dw	offset setpage_1D
		dw	offset getpage_1E
		dw	offset disabledrv_1F
		dw	offset enabledriver_20
		dw	offset softreset_21
                dw      offset nullfunc        
                dw      offset nullfunc         
		dw	offset getversion_24
                dw      offset nullfunc         
		dw	offset getmaxvirt_26

handler		proc	far
                cmp     ax,26h                  
                ja      @checkother           

		sti
		cld
		push	ax bx cx dx ds es di si bp
		mov	bp,sp
		movSeg	ds,cs
		shl	ax,1
		mov	si,ax
                call    funcsoffsets[si]       
IFNDEF PS2
		movSeg	ds,cs
		IFnz	disabled?,@rethandler
		call	enableCOMint
ENDIF

@rethandler:	pop	bp si di es ds dx cx bx ax
		iret

@checkother:    cmp     ax,4dh                
		je	@msstr
                cmp     ax,6dh                
		je	@msver

;------------ 

                IFz     <cs:word ptr oldint33[2]>,@iret 
                                                      
                jmp     cs:oldint33             

;------------ 

@msstr:		mov	di,offset crightmsg
@retstr:	movSeg	es,cs
@iret:		iret

;------------ 

@msver:		mov	di,offset msversion
		jmp	@retstr
handler		endp

resetdriver_00	proc
IFDEF PS2
		call	checkPS2
		jc	@ret
ELSE
		call	disableCOMint
		call	chkcom
                jnz     @ret                  
		call	setCOMparams
ENDIF
		call	enabledriver_20
		jmp	softreset_21
resetdriver_00	endp

showcursor_01	proc
		mov	regionchk?,0
                mov     ax,maxycoord          
                add     ax,1Fh                 
                mov     lowrighty,ax           
                IFz     cursorshown,@ret       
		inc	cursorshown
		jmp	@showptr
showcursor_01	endp

hidecursor_02	proc
		dec	cursorshown
                movSeg  es,cs                   
		jmp	restorescreen
hidecursor_02	endp

status_03	proc
		cli
		mov	ax,column2
		mov	[bp+0Ch],ax
		mov	ax,row2
		mov	[bp+0Ah],ax
		mov	ax,buttstatus
		call	swap2lowbits
		mov	[bp+0Eh],ax
@ret:		ret
status_03	endp

setpos_04	proc
		mov	ax,dx
		mov	si,vrangemin
		mov	di,vrangemax
		call	cutrange
		mov	bx,ax

		mov	ax,cx
		mov	si,hrangemin
		mov	di,hrangemax
		call	cutrange

@pcurs:		cli
		mov	column,ax
		xor	dx,dx
		div	colgranu
		mul	colgranu
		mov	column2,ax

		mov	ax,bx
		mov	row,ax
		div	rowgranu
		mul	rowgranu
		mov	row2,ax

		IFnz	cursorshown,@ret
                mov     cursortype,0           
                mov     startscan,77FFh        
                mov     endscan,7700h          
		jmp	showpointer
setpos_04	endp

butpresdata_05	proc
		and	bx,7
		shl	bx,1
		mov	bx,butpresstatus[bx]
		jmp	@retbuttstat
butpresdata_05	endp

buttreldata_06	proc
		and	bx,7
		shl	bx,1
		mov	bx,butrelstatus[bx]

@retbuttstat:	cli
		xor	ax,ax
		xchg	[bx],ax
		mov	[bp+0Eh],ax
		mov	ax,[bx+2]
		mov	[bp+0Ch],ax
		mov	ax,[bx+4]
		mov	[bp+0Ah],ax
		mov	ax,buttstatus
		call	swap2lowbits
		mov	[bp+10h],ax
@ret2:		ret
buttreldata_06	endp

hrange_07	proc
		cmp	dx,cx
		jb	@swminmaxh
		xchg	dx,cx

@swminmaxh:	cli
		mov	hrangemin,dx
		mov	hrangemax,cx
@hsetpos:	mov	cx,column
		mov	dx,row
		jmp	setpos_04
hrange_07	endp

vrange_08	proc
		cmp	dx,cx
		jb	@swminmaxv
		xchg	dx,cx

@swminmaxv:	cli
		mov	vrangemin,dx
		mov	vrangemax,cx
		jmp	@hsetpos
vrange_08	endp

graphcursor_09	proc
		push	dx es
		mov	hspotrow,cx
                mov     ax,bx                  
                xor     dx,dx                  
                div     colgranu               
                mul     colgranu          
		mov	hspotcol,ax

                movSeg  es,cs                  
		call	restorescreen
		pop	ds si
		call	setusershape
		movSeg	ds,cs

@showptr:       IFnz    cursorshown,@ret2       
                jmp     showpointer            
graphcursor_09	endp

textcursor_0A	proc
		mov	cursortype,bl
		mov	startscan,cx
		mov	endscan,dx
                test    bl,bl                   
		jz	@showptr

		mov	ch,cl
		mov	cl,dl
		and	cx,0F0Fh
		mov	ah,1
                int     10h                    
		ret
textcursor_0A	endp

readmcounter_0B	proc
		cli
		xor	ax,ax
		xchg	mickeyX,ax
		mov	[bp+0Ch],ax
		xor	ax,ax
		xchg	mickeyY,ax
		mov	[bp+0Ah],ax
		ret
readmcounter_0B	endp

intpar_0C	proc
		cli
		saveFAR userproc,es,dx
		call	swap12to34bits
		mov	callmask,cx
		ret
intpar_0C	endp

lightpenon_0D	proc
		mov	lightpen?,1
		ret
lightpenon_0D	endp

lightpenoff_0E	proc
		mov	lightpen?,0FFh
		ret
lightpenoff_0E	endp

micperpixel_0F	proc
		cli
		mov	h8mickey,cx
		mov	v8mickey,dx
		ret
micperpixel_0F	endp

defregion_10	proc
		cmp	si,cx
		jb	@nosw1
		xchg	si,cx
@nosw1:		cmp	di,dx
		jb	@nosw2
		xchg	di,dx
@nosw2:		cli
		mov	lowrightx,si
		mov	upleftx,cx
		mov	lowrighty,di
		mov	uplefty,dx
		mov	regionchk?,1
		jmp	@showptr
defregion_10	endp

exchangeint_14	proc
		cli
		push	es cx
		mov	cx,callmask
		call	swap12to34bits
		mov	[bp+0Ch],cx
		les	ax,userproc
		mov	[bp+0Ah],ax
		mov	[bp+6],es
		pop	cx es
		jmp	intpar_0C
exchangeint_14	endp

storagereq_15	proc
		mov	word ptr [bp+0Eh],LenSaveArea+1
		ret
storagereq_15	endp

savestate_16	proc
		mov	di,dx
		mov	si,offset cursorshown
		movsb
		mov	si,offset StartSaveArea
@savemov:	mov	cx,LenSaveArea
		rep	movsb
		ret
savestate_16	endp

restorestate_17	proc
		push	ds es
		pop	ds es
		mov	si,dx
		mov	di,offset cursorshown
		movsb
		mov	di,offset StartSaveArea
		jmp	@savemov
restorestate_17	endp

setsens_1A	proc
		cli
		mov	h8mickey,bx
		mov	v8mickey,cx

doublespeed_13	proc
		mov	ax,dx
		xor	dx,dx
		mov	bx,30
		div	bx
		mov	treshspeed,ax
		ret
doublespeed_13	endp

setsens_1A 	endp

getsens_1B	proc
		cli
		mov	ax,h8mickey
		mov	[bp+0Eh],ax
		mov	ax,v8mickey
		mov	[bp+0Ch],ax
		mov	ax,30
		mul	treshspeed
		mov	[bp+0Ah],ax
		ret
getsens_1B	endp


setpage_1D	proc
		and	bx,1Fh
		mov	activepage,bl
		xor	ax,ax
		mov	es,ax
		mov	ax,es:[44ch]
		mul	bx
		mov	vidmemoffs,ax
		ret
setpage_1D	endp

getpage_1E	proc
		mov	al,activepage
		cmp	al,0FFh
		jne	@penab
		xor	ax,ax
		mov	es,ax
		mov	al,es:[462h]
@penab:		xor	ah,ah
		mov	[bp+0Eh],ax
		ret
getpage_1E	endp

disabledrv_1F	proc
                IFnz    disabled?,@disabd      
IFDEF PS2
		call	disablePS2
ELSE
		call	disableCOMint
ENDIF
;------------

		push	ds
		mov	al,IRQintnum
		mov	ah,25h
		lds	dx,oldIRQaddr
                int     21h                    
		pop	ds

;------------ 

		push	ds
                lds     dx,oldint10             
		mov	ax,2510h
		int	21h
		pop	ds
;------------ 
		mov	disabled?,1

@disabd:	les	ax,oldint33
		mov	[bp+0Eh],ax
		mov	[bp+06],es
		ret
disabledrv_1F	endp

enabledriver_20	proc
IFDEF PS2
		call	disablePS2
ELSE
		call	disableCOMint
ENDIF
		IFz	disabled?,@enabd
IFNDEF PS2
		call	setCOMparams
ENDIF
		push	ds
		lds	dx,int10pointer
		mov	ax,2510h
                int     21h                    
		pop	ds

@enabd:		call	setnewIRQh
		mov	disabled?,0
IFDEF PS2
		jmp	enablePS2
ELSE
		jmp	enableCOMint
ENDIF
enabledriver_20	endp

softreset_21	proc
		mov	word ptr [bp+0Eh],2
IFNDEF PS2
                cmp     mousetype,3           
		jne	@no3but
		inc	word ptr [bp+0Eh]
ENDIF

@no3but:	mov	word ptr [bp+10h],0FFFFh
		mov	ah,0Fh
                int     10h                   
                                            
		call	setvidparams

		movSeg	es,cs
		xor	al,al
		mov	di,offset StartInitArea
		mov	cx,LenInitArea
		rep	stosb

		mov	startscan,77FFh
		mov	endscan,7700h
		mov	h8mickey,8
		mov	v8mickey,16
		mov	treshspeed,2
		mov	ax,maxycoord
		dec	ax
		mov	vrangemax,ax
		mov	ax,maxxcoord
		dec	ax
		mov	cl,mul2indic
		shl	ax,cl
		mov	hrangemax,ax
		xor	cx,cx
		mov	bx,cx
		mov	dx,offset shape
		call	graphcursor_09
		mov	cursorshown,0
		call	hidecursor_02
		mov	bx,maxycoord
                mov     cx,bx                   
                add     cx,1Fh                 
                mov     lowrighty,cx           
		mov	ax,maxxcoord
		shr	ax,1
		shr	bx,1
		call	@pcurs
		mov	cursorshown,0FFh
		ret
softreset_21	endp

getversion_24	proc
                mov     ax,driverversion        
		mov	[bp+0Eh],ax
IFNDEF PS2
		mov	ah,2
		mov	al,IRQintnum
		sub	al,8
ELSE
		mov	ah,mousetype
		xor	al,al
ENDIF
		mov	[bp+0Ch],ax
		ret
getversion_24	endp

getmaxvirt_26	proc
		xor	ah,ah
		mov	al,disabled?
		mov	[bp+0Eh],ax
		mov	ax,maxxcoord
		dec	ax
		mov	[bp+0Ch],ax
		mov	ax,maxycoord
		dec	ax
		mov	[bp+0Ah],ax

nullfunc	proc
		ret
nullfunc	endp

getmaxvirt_26	endp


swap2lowbits	proc
		IFz	lefthand?,@rhand
		ror	ax,1			; 165_432
		ror	al,1			; 165_243
		rol	ax,1			; 652_431
		ror	ah,1			; 265_431
		rol	ax,1			; 654_312
@rhand:		ret
swap2lowbits	endp

swap12to34bits	proc
		IFz	lefthand?,@rhand2
		ror	cx,3			; 210FEDCB_A9876543
		ror	cl,2			; 210FEDCB_43A98765
		rol	cx,2			; 0FEDCB43_A9876521
		ror	ch,2			; 430FEDCB_A9876521
		rol	cx,3			; FEDCBA98_76521430
@rhand2:	ret
swap12to34bits	endp

IFNDEF PS2
setCOMparams	proc
		cli
		mov	al,80h
		mov	dx,IO_number
		add	dx,3
                out     dx,al                  

                mov     al,60h              
		sub	dx,3
                out     dx,al                  

		xor	al,al
		inc	dx
                out     dx,al                 

                mov     al,07h                 

@nologicor:	inc	dx
		inc	dx
                out     dx,al                   
		mov	al,0Bh
		inc	dx
                out     dx,al                  
                                            
		mov	al,1
		sub	dx,3
                out     dx,al                   

		add	dx,4
                in      al,dx                 
		sub	dx,5
                in      al,dx                  

		sti
		ret
setCOMparams	endp

ENDIF

setnewIRQh	proc
		push	ds
		mov	al,IRQintnum
		mov	ah,25h
		lds	dx,newIRQaddr
                int     21h                    
		pop	ds
		ret
setnewIRQh	endp

IFNDEF PS2
enableCOMint	proc
		cli
                in      al,21h                  
		mov	ah,PICstate
		not	ah
		and	al,ah
@outserial:     out     21h,al                
		sti
		ret
enableCOMint	endp

disableCOMint	proc
		cli
                in      al,21h                  
		or	al,PICstate
                jmp     @outserial           
disableCOMint	endp

ELSE
disablePS2	proc
		cli
		mov	bh,0
		mov	ax,0C200h
                int     15h                    
		xor	bx,bx
		mov	es,bx
		mov	ax,0C207h
                int     15h                   
		sti
		ret
disablePS2	endp

enablePS2	proc
		cli
		movSeg	es,cs
		mov	bx,offset IRQhandler
		mov	ax,0C207h
                int     15h                     
		mov	bh,1
		mov	ax,0C200h
                int     15h              
		sti
		ret
enablePS2	endp
ENDIF

setusershape	proc
		movSeg	es,cs
		mov	bx,0FFFFh
                lea     di,screenmask           
		call	@copybitmap

		xor	bx,bx
		mov	cs:Xcheck,bl
                lea     di,cursormask         

@copybitmap:	mov	cx,16
@loccp:		lodsw
		xchg	al,ah
		stosw
		mov	al,bl
		stosb
		loop	@loccp
		ret
setusershape	endp

cutrange	proc
		cmp	ax,si
		jl	@toosmall
		cmp	ax,di
		jle	@valueOK
		mov	ax,di
		ret
@toosmall:	mov	ax,si
@valueOK:	ret
cutrange	endp

showpointer	proc
                mov     nowdrawing?,1          
                cmp     rowgranu,1             
                jne     @textshp                
                call    drawgraphcurs           
		jmp	@graphshp

@textshp:	call	drawtextcursor
@graphshp:      mov     nowdrawing?,0           
		ret
showpointer	endp

drawtextcursor	proc
		mov	ax,row2
                mov     bx,column2              
                IFz     cursortype,@swcur      
                call    drawHWcursor           
                jmp     @exitdwcurs       

@swcur:         call    softtextcurs           

@exitdwcurs:	cli
                mov     ax,Xcoord          
		mov	Xcoordold,ax
		mov	ax,Ycoord
		mov	Ycoordold,ax
		sti
		ret
drawtextcursor	endp

drawHWcursor	proc
		call	calcaddress
		shr	di,1
		mov	bx,di
		mov	dx,EGAport
		mov	al,0Fh
                out     dx,al                  
		inc	dx
		mov	al,bl
		out	dx,al
		dec	dx
		mov	al,0Eh
                out     dx,al                 
		mov	al,bh
		inc	dx
		out	dx,al
		ret
drawHWcursor	endp

softtextcurs	proc
		cli
		mov	Ycoord,ax
		mov	Xcoord,bx
		sti
		call	restorescreen
		mov	ax,Ycoord
		mov	bx,Xcoord
		mov	cx,ax
		mov	dx,bx
		add	cx,pixboxwidth
		add	dx,pixboxheight
                call    checkregion           
		jc	@exitswcurs

		call	calcaddress
		cli
		call	wait_VRT
                mov     bx,es:[di]            
		sti
		mov	grbuf1,bx
		and	bx,startscan
		xor	bx,endscan
                mov     es:[di],bx              
                mov     restoreindic,0          

@exitswcurs:	ret
softtextcurs	endp

drawgraphcurs	proc
		cli
		mov	ax,row2
		sub	ax,hspotrow
                mov     Ycoord,ax               

		mov	bx,column2
		sub	bx,hspotcol
		mov	cl,mul2indic
		shr	bx,cl
                mov     Xcoord,bx             

		sti
                IFz     regionchk?,@notout      
                                               
		mov	cx,ax
		add	cx,10h
		sub	ax,8
		and	bl,0F8h
		mov	dx,bx
		add	dx,18h
                call    checkregion            
		jc	restorescreen

@notout:	call	graphportparams
		mov	ax,Ycoord
		mov	bx,Xcoord
		cmp	ax,Ycoordold
		jne	@moved
		cmp	bx,Xcoordold
		jne	@moved
		IFz	restoreindic,@nomoved

@moved:		mov	cx,Ycoordold
		cmp	cx,maxycoord
		jg	@dontrest
                call    restoreoldscr           
@dontrest:      mov     ax,Xcoord             
		mov	si,Ycoord
		call	retvidmem
		mov	di,ptr2gbuf1
		mov	dx,Ycoord
		mov	rowcheck,0
		mov	es,graphmodeseg
		mov	ds,videoseg
		call	copyscrmap
		call	setsegs

@nomoved:       call    trans                   
                call    setsegs                
		cli
		mov	ax,Xcoord
		mov	Xcoordold,ax
		mov	si,Ycoord
		mov	Ycoordold,si
		sti
		mov	di,ptr2gbuf2
                call    spritecopy             
		mov	restoreindic,0
		jmp	restorevidport
drawgraphcurs	endp

restorescreen	proc
		mov	nowdrawing?,1
		call	graphportparams
		call	restoreoldscr
		call	restorevidport
		mov	nowdrawing?,0
		ret
restorescreen	endp

wait_VRT	proc
		mov	ax,es
		cmp	ax,0B800h
		jne	@notextmod

		push	ds
		xor	ax,ax
		mov	ds,ax
                mov     dx,ds:[463h]            
		pop	ds
		add	dx,6

@vrt1:		in	al,dx
		test	al,1
		jnz	@vrt1

@vrt2:		in	al,dx
		test	al,1
		jz	@vrt2

@notextmod:	ret
wait_VRT	endp

restoreoldscr	proc
                IFnz    restoreindic,@norestor  
                mov     restoreindic,1         
                cmp     rowgranu,1             
                jne     @textmod                

		mov	di,ptr2gbuf1
		mov	ax,Xcoordold
                mov     si,Ycoordold           
                jmp     spritecopy             

@textmod:       IFnz    cursortype,@norestor   

		mov	ax,Ycoordold
		mov	bx,Xcoordold
		call	calcaddress
		mov	bx,grbuf1
		cli
		call	wait_VRT
                mov     es:[di],bx              
		sti
		movSeg	es,cs
@norestor:	ret
restoreoldscr	endp

spritecopy	proc
		call	retvidmem
		xchg	di,si
		mov	dx,Ycoordold
		mov	rowcheck,1
		mov	es,videoseg
		mov	ds,graphmodeseg
		call	copyscrmap

setsegs		proc
		push	cs cs
		pop	ds es
		ret
setsegs		endp

spritecopy	endp

copyscrmap	proc
		cmp	dx,cs:maxycoord
		jge	@maxed
		mov	ax,cs:nextrowoffset
		mov	bx,cs:nextpagoffset
		mov	cx,dx
		add	cx,16
		cmp	cx,cs:maxycoord
		jl	@nomaxx
		mov	cx,cs:maxycoord

@nomaxx:	sub	cx,dx

@dwrow:		push	cx
		mov	cx,3
		cmp	cs:videomode,13h
		jne	@not13m
		mov	cx,16

@not13m:	rep	movsb
		pop	cx

		IFz	cs:rowcheck,@norowck
		add	di,ax
		cmp	di,bx
		jb	@looprow
		sub	di,bx
		jmp	@looprow

@norowck:	add	si,ax
		cmp	si,bx
		jb	@looprow
		sub	si,bx
@looprow:	loop	@dwrow

@maxed:		ret
copyscrmap	endp

trans		proc
		cmp	videomode,13h
		je	@tvid13
		call	trans2

@tvid13:        mov     si,ptr2gbuf1           
                mov     di,ptr2gbuf2           
                push    di ds                   
		mov	cx,100h
		cmp	videomode,13h
		je	@tvid13_2
		mov	cx,30h
@tvid13_2:	mov	es,graphmodeseg
		mov	ds,graphmodeseg
		rep	movsb
		pop	ds di

		mov	dx,Ycoord
		mov	si,Xcoord
		cmp	cs:videomode,13h
		je	@tvid13_3
		and	si,0FFF8h
@tvid13_3:	xor	bx,bx
		call	setvidpar
		call	transform
		jmp	resvidpar
trans		endp

setvidpar	proc
		push	dx
		IFz	cs:graphsegindic,@nogrv
		mov	dx,3CEh
		mov	ax,5
		cmp	cs:videomode,14h
		jae	@ta14h
		mov	ah,graphwritemod
		and	ah,0FEh
                out     dx,ax                  
		mov	al,3
                out     dx,al               
		mov	dx,Ycoord
		jmp	@nogrv

@ta14h:         out     dx,ax                   
		mov	al,8
                out     dx,al                  
@nogrv:		pop	dx
		ret
setvidpar	endp

resvidpar	proc
		push	dx
		IFz	cs:graphsegindic,@resnogr
		mov	dx,3CFh
		xor	al,al
		out	dx,al
		dec	dx
		mov	al,5
		mov	ah,graphwritemod
                out     dx,al                   
		inc	dx
		mov	al,ah
                out     dx,al                  

@resnogr:	pop	dx
		ret
resvidpar	endp

transform	proc
                mov     cx,10h                 

@loopytr:	test	dx,dx
		jl	@noloopdx
                cmp     dx,maxycoord         
		jg	@moreymax

		push	bx cx di si
		mov	cx,3
		cmp	cs:videomode,13h
		jne	@noloop13
		dec	cx

@nextx13tr:	call	mode13trans
		inc	bx
		loop	@nextx13tr

		jmp	@nextytr

@noloop13:	call	modeno13curs
		inc	bx
		loop	@noloop13

@nextytr:	pop	si di cx bx

@noloopdx:	mov	ax,3
		cmp	cs:videomode,13h
		jne	@no13lo
		mov	ax,10h

@no13lo:	add	di,ax
		add	bx,3
		inc	dx
		loop	@loopytr

@moreymax:	ret
transform	endp

mode13trans	proc
		push	cx dx
		mov	dh,screenmask[bx]
		mov	dl,cursormask[bx]
                mov     cx,8                    

@nextrb:	test	si,si
		jl	@maxxskip
		cmp	si,maxxcoord
		jge	@maxxskip
		mov	al,es:[di]
		shl	dh,1
		jc	@pthere
		xor	al,al
@pthere:	shl	dl,1
		jnc	@nopthere
		xor	al,0Fh
@nopthere:	mov	es:[di],al
@maxxskip:	inc	di
		inc	si
		loop	@nextrb

		pop	dx cx
		ret
mode13trans	endp

modeno13curs	proc
		push	dx
		test	si,si
		jl	@skipn13x
		cmp	si,maxxcoord
		jge	@skipn13x

		IFnz	cs:graphsegindic,@textn13
		mov	al,es:[di]
		and	al,screenmask[bx]
		xor	al,cursormask[bx]
		mov	es:[di],al
		jmp	@skipn13x

@textn13:	mov	dx,3CFh
		cmp	cs:videomode,14h
		jb	@b14tr
		mov	al,screenmask[bx]
		out	dx,al
		mov	al,es:[di]
		mov	al,cursormask[bx]
		mov	es:[di],al
		jmp	@skipn13x

@b14tr:         mov     al,8                  
		out	dx,al
		mov	al,es:[di]
		mov	al,screenmask[bx]
		mov	es:[di],al
                mov     al,18h                 
		out	dx,al
		mov	al,es:[di]
		mov	al,cursormask[bx]
		mov	es:[di],al

@skipn13x:	inc	di
		add	si,8
		pop	dx
		ret
modeno13curs	endp

trans2		proc
		mov	al,byte ptr Xcoord
		and	al,7
		mov	bl,Xcheck
		mov	Xcheck,al
		sub	al,bl
		jz	@noscroll
		jl	@scrleft
		lea	si,cursormask
		call	scrollright
		lea	si,screenmask
		jmp	scrollright

@scrleft:	neg	al
		mov	si,offset cursormask
		call	scrolleft
		mov	si,offset screenmask
		jmp	scrolleft

@noscroll:	ret
trans2		endp

scrollright	proc
		push	di
		mov	bl,al

@scrn:		mov	di,si
		mov	cx,30h

@scrloop:	rcr	byte ptr [di],1
		inc	di
		loop	@scrloop

		jnc	@scrc
		or	byte ptr [si],80h
		jmp	@scrnc

@scrc:		and	byte ptr [si],7Fh
@scrnc:		dec	bl
		jnz	@scrn
		pop	di
		ret
scrollright	endp

scrolleft	proc
		push	di
		mov	bl,al
		add	si,30h

@scln:		mov	di,si
		mov	cx,30h

@sclloop:	dec	di
		rcl	byte ptr [di],1
		loop	@sclloop

		mov	di,si
		jnc	@sclc
		or	byte ptr [di-1],1
		jmp	@sclnc

@sclc:		and	byte ptr [di-1],0FEh
@sclnc:		dec	bl
		jnz	@scln
		pop	di
		ret
scrolleft	endp

retvidmem	proc
		xor	bx,bx
		xchg	si,ax
		IFnz	graphsegindic,@cvidmem
		cmp	videomode,13h
		je	@cvidmem
		mov	bx,ax
		cmp	videomode,14h
		jg	@m14cvid
		and	bx,1
		shr	ax,1
		jmp	@cvidmem

@m14cvid:	and	bx,3
		shr	ax,1
		shr	ax,1

@cvidmem:	mul	colswidth
		cmp	videomode,13h
		je	@m13calc
		mov	cl,3
		sar	si,cl

@m13calc:	add	si,ax
		mov	ax,vidmemoffs
		cmp	activepage,0FFh
		jne	@pageyes
		push	ds
		xor	ax,ax
		mov	ds,ax
		mov	ax,ds:[44eh]
		pop	ds
@pageyes:	add	si,ax
		mov	cl,3
		ror	bx,cl
		add	si,bx
		ret
retvidmem	endp

calcaddress	proc
                div     byte ptr pixboxwidth    
		xor	ah,ah
		mul	colswidth
		mov	di,ax
		mov	ax,bx
                div     byte ptr pixboxheight   
		xor	ah,ah
                add     di,ax                
                shl     di,1
                mov     ax,vidmemoffs
		cmp	activepage,0FFh
		jne	@pageno
		xor	ax,ax
		mov	es,ax
		mov	ax,es:[44eh]
@pageno:        add     di,ax                   
		mov	es,videoseg
		ret
calcaddress	endp

checkregion	proc
		dec	cx
		dec	dx
		cmp	ax,uplefty
		jg	@regexit
		cmp	bx,upleftx
		jg	@regexit
		cmp	cx,lowrighty
		jl	@regexit
		cmp	dx,lowrightx
		jl	@regexit
		stc
		ret

@regexit:	clc
		ret
checkregion	endp

graphportparams	proc
                IFz     graphsegindic,@nogrparam 
		push	ax bx cx dx es
		movSeg	es,cs

		mov	cx,9
                mov     dx,10h                 
                mov     bx,offset datav1       
		mov	ah,0F2h
		int	10h

                mov     bx,2                    
                mov     dx,8                 
		mov	ah,0F0h
		int	10h
                mov     mapmask,bl             

                mov     bx,0F02h                
                mov     dx,8                 
                mov     ah,0F1h               
		int	10h

                mov     cx,6                  
                mov     dx,10h                  
                mov     bx,offset datav2    
		mov	ah,0F3h
		int	10h

		mov	ch,7
                mov     cl,2                    
                mov     dx,10h             
                mov     bx,offset datav3      
                mov     ah,0F3h              
		int	10h

@popEDCBA:	pop	es
@popDCBA:	pop	dx cx bx ax

@nogrparam:	ret
graphportparams	endp

restorevidport	proc
                IFz     graphsegindic,@nogrparam 
		push	ax bx cx dx

		mov	bl,2
                mov     dx,8                 
		mov	bh,mapmask
		mov	ah,0F1h
		int	10h

		mov	cx,6
		mov	dx,10h
		mov	bx,offset datav1
		mov	ah,0F3h
                int     10h                     
		mov	ch,7
		mov	cl,2
		mov	dx,10h
		mov	bx,offset datav1b
		mov	ah,0F3h
                int     10h                  
		jmp	@popDCBA
restorevidport	endp

videomode       db      7             
videoseg        dw      0B000h          
maxxcoord       dw      640             
maxycoord       dw      200             
nextpagoffset	dw	16304
nextrowoffset   dw      8189            
pixboxheight	dw	8
pixboxwidth	dw	8
colgranu        dw      8              
rowgranu        dw      8               
colswidth       dw      80             

vmodeparams	db	0
		dw	0B800h,640,200,16304,8189
		dw	16,8,16,8
		dw	40

		db	1
		dw	0B800h,640,200,16304,8189
		dw	16,8,16,8
		dw	40

		db	2
		dw	0B800h,640,200,16304,8189
		dw	8,8,8,8
		dw	80

		db	3
		dw	0B800h,640,200,16304,8189
		dw	8,8,8,8
		dw	80

		db	4
		dw	0B800h,640,200,16304,8189
		dw	8,8,2,1
		dw	80

		db	5
		dw	0B800h,640,200,16304,8189
		dw	8,8,2,1
		dw	80

		db	6
		dw	0B800h,640,200,16304,8189
		dw	8,8,1,1
		dw	80

		db	7
		dw	0B000h,640,200,16304,8189
		dw	8,8,8,8
		dw	80

		db	8
		dw	0B800h,640,200,32688,8189
		dw	8,8,4,1
		dw	80

		db	9
		dw	0B800h,1280,200,32608,8189
		dw	8,8,1,1
		dw	160

		db	0Ah
		dw	0B800h,1280,200,32608,8189
		dw	8,8,1,1
		dw	160

		db	0Bh
		dw	0B800h,640,200,16304,8189
		dw	8,8,1,1
		dw	80

		db	0Ch
		dw	0B800h,640,200,16304,8189
		dw	8,8,1,1
		dw	80

		db	0Dh
		dw	0A000h,320,200,0,37
		dw	8,8,1,1			; !!! 8,8,2,1
		dw	40

		db	0Eh
		dw	0A000h,640,200,0,77
		dw	8,8,1,1
		dw	80

		db	0Fh
		dw	0A000h,640,350,0,77
		dw	8,14,1,1
		dw	80

		db	10h
		dw	0A000h,640,350,0,77
		dw	8,14,1,1
		dw	80

		db	11h
		dw	0A000h,640,480,0,77
		dw	9,16,1,1
		dw	80

		db	12h
		dw	0A000h,640,480,0,77
		dw	9,16,1,1
		dw	80

		db	13h
		dw	0A000h,320,200,0,304
		dw	8,8,1,1			; !!! 8,8,2,1
		dw	320

		db	14h
		dw	0A000h,640,200,0,317
		dw	8,8,1,1
		dw	320

setvidparams	proc
		push	ax bx cx dx es ds si di
		cli
		xchg	bl,al
		movSeg	ds,cs
		cmp	bl,13h
		jbe	@l13set
                mov     bl,14h                  
                                               
@l13set:	xor	ah,ah
                mov     al,bl                   
		mov	cx,15h
		imul	cx
		mov	si,ax
		lea	si,vmodeparams[si]
                lea     di,videomode           
		movSeg	es,cs
		rep	movsb
		mov	videomode,bl
                mov     ax,maxycoord            
		dec	ax
		mov	vrangemax,ax
		mov	ax,maxxcoord
		dec	ax
		shl	ax,1
		mov	hrangemax,ax
		mov	mul2indic,0

		mov	ptr2gbuf1,offset grbuf1
		mov	ptr2gbuf2,offset grbuf2
		mov	graphmodeseg,cs
		mov	graphsegindic,0
                cmp     bl,0Dh                 
                jb      @otherset             
                cmp     bl,13h                 
                ja      @otherset           
		jnz	@d13set

                mov     graphmodeseg,0A000h     
                mov     ptr2gbuf1,0FA00h     
                mov     ptr2gbuf2,0FB80h        
                inc     mul2indic             
                jmp     @otherset               

@d13set:	cmp	bl,0Dh
		jne	@nod13set
                inc     mul2indic            

@nod13set:      call    firstfreevid            
@otherset:	sti
		pop	di si ds
		jmp	@popEDCBA

;-------------

firstfreevid:	mov	graphwritemod,1
		mov	ptr2gbuf1,3E82h		; 0D, 0E
		mov	ptr2gbuf2,3F12h		; 16002
		mov	graphmodeseg,0A000h
		mov	graphsegindic,bl
		cmp	bl,0Eh
		jle	@l11set

		mov	ax,4000h
		add	ptr2gbuf1,ax		; 0F, 10h, 11h, 12h
		add	ptr2gbuf2,ax		; 32386
		cmp	bl,11h
		jl	@l11set

		mov	ax,2000h
                add     ptr2gbuf1,ax           
                add     ptr2gbuf2,ax           

@l11set:        mov     ah,0F0h                
		mov	bx,5
		mov	dx,10h
                int     10h                    

		test	bl,2
		jz	@svidex
		mov	graphwritemod,10h

@svidex:	ret
setvidparams	endp

RD1		dw	0,0
		db	0
RD2		db	0
d121		db	0
		db	24 dup (0)
d125		db	21 dup (0)
d122		db	8 dup (0)
RD3		db	0
RD4		db	0
RD123		db	0
RD124		db	0
RD125		db	0
		db	0,0,0,0
RD126		db	0
d127		db	0
		db	24 dup (0)
d126		db	21 dup (0)
d128		db	8 dup (0)
RD127		db	0
RD128		db	0
RD129		db	0
RD130		db	0

CRTCbase        dw      3D4h         
RD132		dw	offset d121
RD133		dw	offset d127
RD134		db	19h
RD135		db	0

                dw      3C4h           
		dw	offset RD1
		dw	offset RD125
		db	5
RD136		db	0

                dw      3CEh          
		dw	offset d122
		dw	offset d128
		db	9
RD137		db	0

                dw      3C0h           
		dw	offset d125
		dw	offset d126
VGAmoncol	db	15h
RD139		db	0

                dw      3C2h        
		dw	offset RD2
		dw	offset RD126
		db	1
		db	0

STATbase        dw      3DAh            
		dw	offset RD4
		dw	offset RD128
		db	1
		db	0

                dw      3CCh          
datapro		dw	offset RD123
		dw	offset RD129
		db	1
		db	0

                dw      3CAh            
		dw	offset RD124
		dw	offset RD130
		db	1
		db	0

RD142		db	0
RD143		db	0
		dw	0101h
		db	0

RILfuncs        dw      offset RIL_F0   
		dw	offset RIL_F1
		dw	offset RIL_F2
		dw	offset RIL_F3
		dw	offset RIL_F4
		dw	offset RIL_F5
		dw	offset RIL_F6
		dw	offset RIL_F7
RD152		dw	offset orig10func
		dw	offset orig10func
		dw	offset RIL_FA

RIL_F0		proc
		mov	si,dx
		mov	si,RD132[si]
		cmp	dx,20h
		jge	@R0
		add	si,bx
@R0:		mov	bl,[si]
		ret
RIL_F0		endp

RIL_F1		proc
		mov	ax,bx
		mov	si,dx
		cmp	dl,20h
		mov	dx,CRTCbase[si]
		mov	RD135[si],1
		mov	si,RD132[si]
		jge	@R4
		xor	bh,bh
		mov	[bx+si],ah
		cmp	dl,0C0h
		jne	@R1
		push	ax
		mov	dl,byte ptr STATbase
                in      al,dx                 
		pop	ax
		mov	dl,0C0h
		out	dx,ax
		mov	al,ah
		out	dx,al
		ret

@R4:		mov	[si],al
		mov	RD142,1
                out     dx,al                  
		ret

@R1:		out	dx,ax
		ret
RIL_F1		endp

RIL_F2		proc
		sti
		mov	di,bx
		mov	si,dx
		mov	si,RD132[si]
		xor	ax,ax
		xchg	al,ch
		add	si,ax
		shr	cx,1
		rep	movsw
		adc	cx,cx
		rep	movsb
		ret
RIL_F2		endp

RIL_F3		proc
		sti
		push	es
		mov	si,bx
		mov	di,dx
		mov	RD135[di],1
		mov	dx,CRTCbase[di]
		mov	di,RD132[di]
		mov	ax,es
		mov	bx,ds
		mov	es,bx
		mov	ds,ax
		xor	ax,ax
		xchg	al,ch
		add	di,ax

		push	cx
		shr	cx,1
		rep	movsw
		adc	cx,cx
		rep	movsb
		mov	ds,bx
		pop	cx
		sub	di,cx

@looprega:	mov	ah,[di]
		cmp	dl,0C0h
		jne	@setREGA
		push	ax
		mov	dl,byte ptr STATbase
                in      al,dx                   
		pop	ax
		mov	dl,0C0h
		out	dx,al
		push	ax
		mov	al,ah
		out	dx,al
		pop	ax
		jmp	@rega2

@setREGA:	out	dx,ax
@rega2:		inc	di
		inc	al
		loop	@looprega
		pop	es
		ret
RIL_F3		endp

RIL_F4		proc
		sti
		mov	di,bx

@RILoo1:	mov	si,es:[di]
		mov	si,RD132[si]
		mov	al,es:[di+2]
		cbw
		add	si,ax
		add	di,3
		movsb
		loop	@RILoo1

		ret
RIL_F4		endp

RIL_F5		proc
		sti
		push	dx
		mov	di,bx

@RILoo2:	mov	si,es:[di]
		mov	RD135[si],1
		mov	dx,CRTCbase[si]
		mov	si,RD132[si]
		mov	al,es:[di+2]
		cbw
		add	si,ax
		mov	ah,es:[di+3]
		mov	[si],ah
		out	dx,al
		inc	dx
		mov	al,ah
		out	dx,al
		dec	dx
		add	di,4
		loop	@RILoo2

		pop	dx
		ret
RIL_F5		endp

RIL_F6		proc
		sti
		push	bx cx dx es
		movSeg	es,ds
		lea	bx,CRTCbase
		xor	cx,cx
@R12:		cmp	[bx+7],ch
		je	@R18
		mov	[bx+7],ch
		mov	cl,[bx+6]
		mov	si,[bx+4]
		mov	di,[bx+2]
		mov	dx,[bx]
		mov	al,ch

		push	cx
		shr	cx,1
		rep	movsw
		adc	cx,cx
		rep	movsb
		pop	cx
		sub	si,cx

@RILoo3:	mov	ah,[si]
		cmp	dx,03ceh
		jne	@Rlo3
		cmp	al,6
		jz	@R17

@Rlo3:		out	dx,al
		inc	dx
		push	ax
		mov	al,ah
		out	dx,al
		dec	dx
		pop	ax

@R17:		inc	si
		inc	ax
		loop	@RILoo3

@R18:		add	bx,8
		cmp	bx,offset datapro+1
		jl	@R12
		cmp	RD142,ch
		je	@R19
		mov	RD142,ch
		mov	dx,[bx+8]
		mov	al,RD128
		mov	RD4,al
                out     dx,al                
                                             
		mov	dx,3C2h
		mov	al,RD126
		mov	RD2,al
                out     dx,al                   
		mov	dx,3cch
		mov	al,RD129
		mov	RD123,al
                out     dx,al                 
		mov	dx,3CAh
		mov	al,RD130
		mov	RD124,al
                out     dx,al                  

@R19:		mov	dx,03c0h
		mov	al,20h
		out	dx,al
		mov	dx,STATbase
                in      al,dx                   
		pop	es dx cx bx
		ret
RIL_F6		endp

RIL_F7		proc
		sti
		push	cx
		mov	si,bx
		mov	di,dx
		xor	ch,ch
		mov	cl,RD134[di]
		mov	RD135[di],cl
		mov	RD142,cl
		mov	di,RD133[di]

		mov	ax,es
		mov	bx,ds
		mov	es,bx
		mov	ds,ax
		shr	cx,1
		rep	movsw
		adc	cx,cx
		rep	movsb
		mov	es,ax
		pop	cx
		ret
RIL_F7		endp

RIL_FA		proc
		movSeg	es,ds
		mov	bx,offset RD152
		ret
RIL_FA		endp

orig10func	proc
		push	bp
		mov	bp,sp
		mov	ds,[bp+8]
		mov	si,[bp+6]
		mov	ax,[bp+4]
		pop	bp
		pushf
		call	cs:oldint10
		push	bp
		mov	bp,sp
		mov	[bp+4],ax
		mov	[bp+6],si
		mov	[bp+8],dx
		pop	bp
		ret
orig10func	endp

int10handler	proc
		cld
		cli
                cmp     ah,4                    
                jne     @nolpen              
		cmp	cs:lightpen?,1
		je	@lightpen

@nolpen:        test    ah,ah                
                jz      setmodreq               

                cmp     ah,0F0h               
                jae     RILreq                 
@exitRIL:       jmp     cs:oldint10           

RILreq:		cmp	ah,0FBh
		jae	@exitRIL
		push	ax ds si di
		movSeg	ds,cs
		mov	al,ah
		and	ax,0Fh
		shl	ax,1
		mov	si,ax
		call	RILfuncs[si]
		pop	di si ds ax
		iret

@lightpen:	IFnz	cs:buttstatus,@lightbutt
		xor	ah,ah
		jmp	@lightbutt

@lightbutt:	sti
		mov	ax,cs:row2
		cmp	cs:videomode,0Fh
		jb	@lpenbf
		mov	cx,ax
		jmp	@lpenaf

@lpenbf:	mov	ch,al
@lpenaf:	div	cs:pixboxwidth
		mov	dh,al
		mov	ax,cs:column2
		mov	bx,ax
		cwd
		div	cs:colgranu
		xchg	bx,ax
		div	cs:pixboxheight
		mov	dl,al
		mov	ah,1
		iret

setmodreq:	push	ax
		mov	cs:cursorshown,0
		mov	ax,2
                int     33h                   
		pop	ax

		call	setvidparams

		push	ax
		pushf
		call	cs:oldint10
		pop	ax

		mov	cs:RD143,al
                call    monoorcolor            

		iret
int10handler	endp

monoorcolor	proc
		push	ds
		xor	ax,ax
		mov	ds,ax
                mov     al,ds:[410h]           
                and     al,30h                
                test    byte ptr ds:[487h],2    
                jz      @cmon                  
                cmp     al,30h                  
                jne     @80mon                 

                mov     ah,cs:RD143           
		and	ah,7Fh
		mov	byte ptr cs:CRTCbase,0B4h
		mov	byte ptr cs:STATbase,0BAh
		cmp	ah,0Fh
		je	@skipcol
		mov	ah,7
		jmp	@skipcol

@cmon:          cmp     al,30h                 
                je      @80mon                  

                mov     ah,cs:RD143         
		and	ah,7Fh
		mov	cs:byte ptr CRTCbase,0D4h
		mov	cs:byte ptr STATbase,0DAh

@skipcol:       call    displayports           
@80mon:		pop	ds
		ret
monoorcolor	endp

displayports	proc
		push	cx di si es
		pushf
		cli
		xor	cx,cx
		mov	ds,cx
		les	si,dword ptr ds:[4a8h]
		les	si,dword ptr es:[si]
		test	byte ptr ds:[487h],60h
                jz      @noRAM                 

                cmp     ah,0Fh                
		jne	@nogmono

		add	si,440h
		jmp	@disdet

@nogmono:       cmp     ah,10h                
		jne	@noRAM

		add	si,480h
		jmp	@disdet

@noRAM:         cmp     ah,3                    
		ja	@noMDAEGA

                mov     al,ds:[488h]            
		and	al,0Fh
                cmp     al,3                  
		je	@MDAEGA
                cmp     al,9                   
		je	@MDAEGA
		jmp	@noMDAEGA

@MDAEGA:	add	si,4c0h

@noMDAEGA:      cmp     ah,11h                  
		jl	@noVmono
		add	ah,9
@noVmono:	xor	al,al
		shr	ax,1
		shr	ax,1
		add	si,ax

@disdet:	movSeg	ds,es
		movSeg	es,cs
		lea	di,RD1
		mov	al,3
		stosb
		add	si,5
		mov	cx,3Ch SHR 1
		push	cx si
		rep	movsw
		pop	si cx
		lea	di,RD125
		mov	al,3
		stosb
		rep	movsw
		movSeg	ds,cs
		std
		lea	di,RD3
		mov	cl,9
		mov	si,di
		dec	si
		rep	movsb
		lea	di,RD127
		mov	cl,9
		mov	si,di
		dec	si
		rep	movsb
		IFz	cs:VGAindicat,@nommon
		mov	VGAmoncol,14h

@nommon:	mov	cx,0100h
		mov	RD123,cl
		mov	RD129,cl
		mov	RD124,ch
		mov	RD130,ch
		mov	RD142,cl
		mov	RD136,cl
		mov	RD135,cl
		mov	RD139,cl
		mov	RD137,cl
		popf
		pop	es si di cx
		sti
		ret
displayports	endp

IFNDEF PS2
chkcom		proc
		mov	dx,IO_number
		xor	al,al
		add	dx,3
                out     dx,al      

		dec	dx
		dec	dx
                in      al,dx                  
		and	al,0F0h
                mov     ah,al                

		add	dx,3
                in      al,dx                  
                and     al,0E0h                
                or      al,ah                  
		ret
chkcom		endp

ELSE
checkPS2	proc
		cli
		mov	bh,3
                PS2serv 0C205h,@noPSdet         
                PS2serv 0C201h,@noPSdet        

		mov	cx,2
@try2:		push	es
		movSeg	es,cs
		mov	bx,offset IRQhandler
		mov	ax,0C207h
                int     15h                    
		pop	es
		jnc	@PSokyet
		cmp	ah,4
		jne	@noPSdet
		loop	@try2
		jmp	@noPSdet

@PSokyet:	mov	bh,3
                PS2serv 0C203h, @noPSdet      
		mov	bh,1
                PS2serv 0C200h, @noPSdet        
		xor	dl,dl
		xor	bh,bh
                PS2serv 0C206h, @noPSdet        

		test	bl,20h
		jz	@noPSdet
		or	dl,dl
		jz	@noPSdet
		mov	mousetype,4
		clc
		sti
		ret

@noPSdet:	stc
		sti
		ret
checkPS2	endp
ENDIF

updatebuttstat	proc
		mov	cx,buttstatus
		mov	di,row2
		mov	si,column2
		mov	newbuttstat,0
		xor	cl,al
		mov	byte ptr buttstatus,al

                test    cl,1                 
                jz      @chk2press           
                test    al,1             
                jz      @rel1                   

                or      newbuttstat,2         
		mov	butt1pc,si
		mov	butt1pr,di
		inc	butt1press
		jmp	@chk2press

@rel1:          or      newbuttstat,4           
		mov	butt1rc,si
		mov	butt1rr,di
		inc	butt1rel

@chk2press:     test    cl,2                
                jz      @chk3press             
                test    al,2                 
                jz      @rel2             

                or      newbuttstat,8       
		mov	butt2pr,di
		mov	butt2pc,si
		inc	butt2press
		jmp	@chk3press

@rel2:          or      newbuttstat,10h        
		mov	butt2rc,si
		mov	butt2rr,di
		inc	butt2rel

IFNDEF PS2
                cmp     mousetype,3             
                jne     @nomorebutt            
ELSE
		ret
ENDIF

@chk3press:     test    cl,4                  
                jz      @nomorebutt           
                test    al,4                
                jz      @rel3                  

                or      newbuttstat,20h        
		mov	butt3pc,si
		mov	butt3pr,di
		inc	butt3press
		ret

@rel3:          or      newbuttstat,40h        
		mov	butt3rc,si
		mov	butt3rr,di
		inc	butt3rel

@nomorebutt:	ret

updatebuttstat	endp


IRQhandler	proc	far
		cld
IFNDEF PS2
		cli
		push	ax bx cx dx ds es di si bp
		push	cs cs
		pop	ds es

		mov	dx,cs:IO_number
		add	dx,5
                in      al,dx                   
		sub	dx,5
		test	al,2
                jz      @nooverrun             

                in      al,dx                  
                mov     COM_incoming,0       
                jmp     @exitIRQh              

@nooverrun:     test    al,1                   
                jnz     @dataready              
                in      al,dx                
                jmp     @exitIRQh              

@dataready:     in      al,dx                 
                cmp     cs:mousetype,2         
                je      @MSproc               

                IFnz    cs:logitech?,@MSproc    
                call    msystemsproc          
		jmp	@exitIRQh

@MSproc:	call	microsoftproc

@exitIRQh:	mov	al,20h
                out     20h,al                 
		jmp	@rethandler
ELSE
                push    bp                    
		mov	bp,sp
		push	ax bx cx dx ds es di si
		push	cs cs
		pop	ds es

		mov	ax,[bp+0Ch]
		test	ah,ah
		jnz	@invPS2data
		and	al,3
		call	updatebuttstat
		mov	ax,[bp+0Ch]
		mov	bx,[bp+0Ah]
		mov	cx,[bp+8]
		test	al,10h
		jz	@PSxneg
		mov	bh,0FFh

@PSxneg:	test	cx,cx
		jz	@noymov
		neg	cl
		test	al,20h
		jnz	@noymov
		mov	ch,0FFh

@noymov:	add	Ymovement,cx
		add	Xmovement,bx
		mov	newPS2data,1

@invPS2data: 	pop	si di es ds dx cx bx ax bp
		retf
ENDIF

IRQhandler	endp

IFNDEF PS2

microsoftproc	proc
                and     ax,07fh             
                IFnz    COM_incoming,@MSsecond 

                cmp     al,40h                
                jnb     @MSfirst               

                IFz     logitech?,@MSextra      
                IFnz    syncCount,@MSextra     
                cmp     extraBytes,MAX_EXTRA_BYTES
		jnz	@MSextra
		jmp	@logiMiddle

@MSextra:       IFz     extraBytes,@nosync1    
                dec     extraBytes            
		ret
@MSfirst:	xor	cx,cx
		mov	Xmovement,cx
		mov	Ymovement,cx
                shr     al,1                 
		rcr	byte ptr Xmovement,1
                shr     al,1                   
		rcr	byte ptr Xmovement,1
                shr     al,1                   
		rcr	byte ptr Ymovement,1
                shr     al,1                    
		rcr	byte ptr Ymovement,1
		and	al,3
                sar     al,1                  
                jnc     @MSrrel                 
                or      al,2                   

@MSrrel:        and     al,03                  
                mov     tmpButStat,ax         
                mov     COM_incoming,1          
		ret

@MSsecond:      test    al,40h                 
                jnz     @nosync1              
                cmp     COM_incoming,1          
                jne     @MSthird               
                mov     COM_incoming,2         
		and	al,3Fh
                or      byte ptr Xmovement,al  
		ret

@nosync1:       mov     syncCount,MIN_GOOD_SYNCS
                mov     COM_incoming,0         
		mov	extraBytes,0
		cmp	errorCont,MAX_BAD_SYNCS
		jb	@nosync1a
                mov     mousetype,3       
		mov	logitech?,0
@nosync1a:	inc	errorCont
		ret

@MSthird:       test    al,40h                
                jnz     @nosync1              
                cmp     COM_incoming,2       
                jne     @nosync1           
                mov     COM_incoming,0         
		mov	extraBytes,MAX_EXTRA_BYTES
		and	al,3Fh
                or      byte ptr Ymovement,al   

                IFnz    syncCount,@decSync     
                mov     errorCont,0            
		mov	ax,buttstatus
                and     ax,04                 
                or      ax,tmpButStat          
		call	updatebuttstat
		mov	al,byte ptr Ymovement
		cbw
		mov	cx,ax
		mov	al,byte ptr Xmovement
		cbw
		mov	bx,ax
		jmp	movepointer

@decSync:       dec     syncCount             
                mov     COM_incoming,0          
		mov	extraBytes,MAX_EXTRA_BYTES
		ret

@logiMiddle:	mov	cx,buttstatus
                and     cx,03               
                shr     ax,3                
                and     ax,04                
		or	ax,cx
		call	updatebuttstat
                xor     cx,cx               
                xor     bx,bx              
                dec     extraBytes              
		jmp	movepointer

microsoftproc	endp

MSMoffsets      dw      offset MSM1     
		dw	offset MSM24
		dw	offset MSM3
		dw	offset MSM24
		dw	offset MSM5

msystemsproc	proc
		cbw
		mov	si,COM_incoming
		shl	si,1
		jmp	MSMoffsets[si]

MSM1:		push	ax
		and	al,0F8h
                cmp     al,80h                 
                jne     @nosync2            
		pop	ax

		not	ax
		and	ax,7
                mov     COM_incoming,1         
                mov     MSCbuttstate,ax      
		xor	ax,ax
		mov	Xmovement,ax
                mov     Ymovement,ax            
		ret

@nosync2:	xor	ax,ax
                mov     COM_incoming,ax        
                mov     MSCbuttstate,ax       
		mov	Xmovement,ax
                mov     Ymovement,ax         
		pop	ax
                mov     syncCount,MIN_GOOD_SYNCS
		cmp	errorCont,MAX_BAD_SYNCS
		jb	@nosync2a
                mov     mousetype,2             
		mov	logitech?,1
@nosync2a:	inc	errorCont
		ret

MSM24:		add	Xmovement,ax
		inc	COM_incoming
		ret

MSM3:		mov	Ymovement,ax
		inc	COM_incoming
		ret

MSM5:		add	Ymovement,ax
		mov	COM_incoming,0

                IFnz    syncCount,@MSMdecSync   
                mov     errorCont,0            
		mov	ax,MSCbuttstate
		mov	cl,0Eh
		shl	ax,cl
		rcl	ax,1
		mov	cl,3
		rcl	ah,cl
		or	al,ah
		call	updatebuttstat
		mov	bx,Xmovement
		mov	cx,Ymovement
		neg	cx
		jmp	movepointer


@MSMdecSync:    dec     syncCount              
                mov     COM_incoming,0         
		ret

msystemsproc	endp

IF 0    
logiproc	proc
		and	ax,07fh
                mov     cx,COM_incoming         
                test    al,40h                  
                jz      @3f47                   
		test	cx,cx			; CX = 0?
                jz      @3f47                   
		cmp	cx,3
                jz      @3f35                   
		xor	cx,cx
                mov     COM_incoming,cx         
		jmp	@3f54

@3f35:          xor     cx,cx                  
		mov	COM_incoming,cx
                test    extrabyte,4             
                jz      @3f54                  
		and	extrabyte,0FBh
		jmp	@3f54

@3f47:          test    al,40h                 
                jnz     @3f54                  
                test    cx,cx                  
                jnz     @3f54                 

@endLGproc:	ret

@3f54:          test    cx,cx                   
                jnz     @3f5e               
                mov     LGbstat,al             
		jmp	@3f67

@3f5e:          cmp     cx,1                   
                jnz     @3f67                   
                mov     X_LO,al               
@3f67:          inc     COM_incoming            
                cmp     cx,2                  
                jb      @endLGproc              
                ja      @LG4th                

		mov	bl,X_LO
		shl	bl,2
		mov	bh,LGbstat
		shr	bx,2
                mov     dl,bl                  

		mov	bl,al
		shl	bl,2
		shr	bx,2
                mov     dh,bl                  

		ror	bx,1
		ror	bh,1
		shl	bl,1
		rcl	bh,2
		and	bh,3
                mov     bl,bh                  

		mov	bh,extrabyte
		test	bh,4
		jz	@3faa
		or	bl,4
@3faa:          mov     extrabyte,bl         
		mov	al,dl
		cbw
		mov	Xmovement,ax
		mov	al,dh
		cbw
		mov	Ymovement,ax
		jmp	@3fed

@LG4th:         mov     COM_incoming,0          
		mov	bl,extrabyte
		and	bl,4
		mov	cl,3
		and	al,20h
		shr	al,cl
		xor	bl,al
		jnz	@3fd9
		jmp	@endLGproc

@3fd9:		xor	bl,extrabyte
		mov	extrabyte,bl
		xor	bx,bx
		mov	Ymovement,bx
		mov	Xmovement,bx

@3fed:		mov	al,extrabyte
		call	updatebuttstat
		mov	cx,Ymovement
		mov	bx,Xmovement
		jmp	movepointer

logiproc	endp
ENDIF

ELSE

handleIRQ	proc
		cli
		pushf
		call	cs:oldIRQaddr
		IFz	cs:newPS2data,@iret2

		cld
		push	ax bx cx dx ds es di si bp
		push	cs cs
		pop	ds es
		mov	newPS2data,0
		mov	cx,Ymovement
		mov	bx,Xmovement
		call	movepointer
		xor	ax,ax
		mov	Ymovement,ax
		mov	Xmovement,ax
		jmp	@rethandler

@iret2: 	iret

handleIRQ	endp
ENDIF

movepointer	proc
		mov	ax,cx
                or      ax,bx                 
                jnz     @itsmoved               
		jmp	@buttonly

@itsmoved:	mov	ax,bx
		IFz	autores?,@noautor
		sar	ax,2
		push	ax
		and	ax,0Fh
		cmp	ax,0Ah
		jle	@leax
		mov	ax,0Ah

@leax:		mov	ah,al
		mov	mresolution,ax
		pop	ax

@noautor:	xor	bx,bx
		call	resolute
		add	mickeyX,ax

		push	ax
		mov	ax,cx
		IFz	autores?,@noautor2
		sar	ax,2
		push	ax
		and	ax,0Fh
		cmp	ax,0Ah
		jle	@leax2
		mov	al,0Ah

@leax2:		mov	ah,al
		mov	mresolution,ax
		pop	ax

@noautor2:	mov	bx,1
		call	resolute
		add	mickeyY,ax

                mov     cx,ax                 
                pop     bx                      

                test    bx,bx                 
                jz      @xmov0                  
		shl	bx,3
		mov	ax,Xcalc
		add	ax,bx
		cwd
		idiv	h8mickey
		add	ax,column
		mov	Xcalc,dx
		mov	si,hrangemin
		sub	si,10h
		mov	di,hrangemax
		add	di,10h
		call	cutrange
		mov	column,ax
		cmp	ax,hrangemin
		jl	@xsmall
		cmp	ax,hrangemax
		jle	@xgood
		mov	ax,hrangemax
		jmp	@xgood

@xsmall:	mov	ax,hrangemin

@xgood:		cwd
		div	colgranu
		mul	colgranu
                mov     column2,ax           
		sub	ax,Xcoordold
                mov     bx,ax                   

@xmov0:         test    cx,cx                  
                jz      @ymov0                 
		shl	cx,3
		mov	ax,Ycalc
		add	ax,cx
		cwd
		idiv	v8mickey
		add	ax,row
		mov	Ycalc,dx
		mov	si,vrangemin
		sub	si,10h
		mov	di,vrangemax
		add	di,10h
		call	cutrange
		mov	row,ax
		cmp	ax,vrangemin
		jl	@ysmall
		cmp	ax,vrangemax
		jle	@ygood
		mov	ax,vrangemax
		jmp	@ygood

@ysmall:	mov	ax,vrangemin

@ygood:		cwd
		idiv	rowgranu
		imul	rowgranu
                mov     row2,ax                 
		sub	ax,Ycoordold
                mov     cx,ax                  

@ymov0:         or      cx,bx                   
                jz      @buttonly              
                or      newbuttstat,1          

@buttonly:	mov	ax,newbuttstat
                IFnz    userproc?,@exitmove   
                call    calluserproc            
                test    al,1                    
                jz      @exitmove              
		mov	al,nowdrawing?
                or      al,cursorshown        
                jnz     @exitmove               
		sti
                call    showpointer             
		cli

@exitmove:	ret

movepointer	endp

resolute	proc
		mov	bl,byte ptr mresolution[bx]
                cmp     bx,1           
                jle     @res01          

                cmp     ax,3           
                jb      @res01        
                cmp     ax,-3           
                ja      @res01      

                cmp     ax,8            
                jb      @resother     
                cmp     ax,-8          
                ja      @resother       

		cwd
                imul    bx             
;*              shr     ax,1            
                ret                   

@resother:      shl     ax,1          

@res01:		ret

resolute	endp

calluserproc	proc
		mov	userproc?,1
		push	ax
                and     ax,callmask             
                jz      @nocmask               

IFNDEF PS2
		push	ax
                mov     al,20h                  
                out     20h,al                 
		pop	ax
ENDIF

		mov	si,mickeyX
		mov	di,mickeyY
		mov	bx,buttstatus
		mov	cx,column2
		mov	dx,row2
		sti
		call	userproc
		cli
		call	setsegs
@nocmask:	pop	ax
		mov	userproc?,0
		ret
calluserproc	endp

;==========================================================================
IRQ_number      db      0             
indicator       db      0             


IFDEF LANG_PT_ES
cutestring      db      'MOUSE DRIVER 1.2',0dh,0ah
                db      ' ',0dh,0ah,'$'
IFNDEF PS2
MSMStr          db      'Driver : Mouse Systems',0dh,0ah,'$'
MSStr           db      'Driver : Microsoft',0dh,0ah,'$'
datalogi        db      'Driver : Logitech MouseMan',0dh,0ah,'$'
ELSE
drivmod         db      'Driver : PS/2',0dh,0ah,'$'
ENDIF

relStr          db      'Driver de mouse retirado de la memoria.',0dh,0ah,'$'
mnsStr          db      'Driver de mouse no instalado.',0dh,0ah,'$'

IFNDEF PS2
ncfStr          db      'Error: puerto COM no encontrado',0dh,0ah,'$'
nmfStr          db      'Error: mouse no encontrado',0dh,0ah,'$'
ELSE
dnfStr          db      'Dispositivo mouse no encontrado',0dh,0ah,'$'
ENDIF

alrStr          db      'Mouse ya instalado.',0dh,0ah,'$'
instStr         db      'Instalando en: COM$'
resol           db      'Resolucion: $'
auto		db	'Auto',0dh,0ah,'$'
times           db      '3/3 veces horizontal/vertical',0dh,0ah,'$'
badswstring     db      0dh,0ah,'Error: parametro inv lido',0dh,0ah
                db      'Digite /? en la linea de comandos para la ayuda.',0dh,0ah,'$'
com_port	db	0,0dh,0ah,'$'

IFNDEF PS2
CMDStr          db      'MOUSE DRIVER 1.2',0dh,0ah
                db      'Opciones:',0dh,0ah
                db      '  /n   - N£mero del puerto COM',0dh,0ah
                db      '  /R0  - Resolucion autom tica por hardware',0dh,0ah
                db      '  /Rnm - n,m=1-9 resolucion horizontal/vertical (R33)',0dh,0ah
                db      '  /M   - Fuerza a modo Microsoft (2 botones)',0dh,0ah
                db      '  /T   - Fuerza a modo Logitech MouseMan (3 botones)',0dh,0ah
                db      '  /S   - Fuerza a modo Mouse Systems (3 botones)',0dh,0ah
                db      '  /N   - Instalacion no encotnrada (se testea Microsoft/Logitech)',0dh,0ah
                db      '  /In  - Fuerza al n£mero de IRQ (n ‚ em hex: n=3-F)',0dh,0ah
ELSE
CMDStr          db      'MOUSE DRIVER FOR PS/2 0.2',0dh,0ah
                db      'Opciones:',0dh,0ah
                db      '  /R0  - Resolucion autom tica por hardware',0dh,0ah
                db      '  /Rnm - n,m=1-9 resolucion horizontal/vertical (R33)',0dh,0ah
ENDIF
                db      '  /L   - Modo Canhoto (por defecto modo dextro)',0dh,0ah
                db      '  /U   - Retira el driver de la mem¢ria',0dh,0ah
                db      '  /?   - Muestra la ayuda',0dh,0ah,'$'

ELSE
cutestring      db      'MOUSE DRIVER 1.2',0dh,0ah
                db      'Type mouse /? for help',0dh,0ah,'$'
IFNDEF PS2
MSMStr		db	'Driver Mode: Mouse Systems',0dh,0ah,'$'
MSStr		db	'Driver Mode: Microsoft',0dh,0ah,'$'
datalogi	db	'Driver Mode: Logitech MouseMan',0dh,0ah,'$'
ELSE
		db	'Type ctmousep /? for help',0dh,0ah,'$'
drivmod		db	'Driver Mode: PS/2',0dh,0ah,'$'
ENDIF

relStr		db	'Mouse driver has been released from memory',0dh,0ah,'$'
mnsStr		db	'Mouse driver is not installed!',0dh,0ah,'$'

IFNDEF PS2
ncfStr		db	'Error: Cannot find COM port',0dh,0ah,'$'
nmfStr		db	'Error: Cannot find mouse',0dh,0ah,'$'
ELSE
dnfStr		db	'Cannot find pointer device',0dh,0ah,'$'
ENDIF

alrStr		db	'Mouse already installed',0dh,0ah,'$'
instStr		db	'Installed on: COM $'
resol		db	'Resolution: $'
auto		db	'Auto',0dh,0ah,'$'
times		db	'3/3 times horizontally/vertically',0dh,0ah,'$'
badswstring	db	0dh,0ah,'Error: Invalid parameter',0dh,0ah
		db	'Enter /? on command line for help',0dh,0ah,'$'
com_port	db	0,0dh,0ah,'$'

IFNDEF PS2
CMDStr          db      'MOUSE 1.2',0dh,0ah
		db	'Options:',0dh,0ah
		db	'  /n   - where n is the COM port number',0dh,0ah
		db	'  /R0  - Auto hardware resolution',0dh,0ah
		db	'  /Rnm - n,m=1-9 resolution horizontally/vertically (default is R33)',0dh,0ah
		db	'  /M   - Force Microsoft mode (2 buttons)',0dh,0ah
		db	'  /T   - Force Logitech MouseMan mode (3 buttons)',0dh,0ah
		db	'  /S   - Force Mouse Systems mode (3 buttons)',0dh,0ah
		db	'  /N   - Do Not install if no mouse found (test only Microsoft/Logitech)',0dh,0ah
		db	'  /In  - Force IRQ number (n is in hex: n=3-F)',0dh,0ah
ELSE
CMDStr          db      'MOUSE DRIVER FOR PS/2 0.2',0dh,0ah
		db	'Options:',0dh,0ah
		db	'  /R0  - Auto hardware resolution',0dh,0ah
		db	'  /Rnm - n,m=1-9 resolution horizontally/vertically (default is R33)',0dh,0ah
ENDIF
		db	'  /L   - Left hand mode (default is right hand mode)',0dh,0ah
		db	'  /U   - Release driver',0dh,0ah
		db	'  /?   - Show help',0dh,0ah,'$'
ENDIF
IFNDEF PS2
microsoft?      db      0              
ENDIF

noemem          db      'No hay mas memoria!$'
ctstr           db      'MOUSE '

paragraphs	dw	0
dest_seg	dw	0
XMSentry	dd	0

real_start:	cld
                mov     ah,4Ah                  
                mov     bx,15+offset endProgram 
		shr	bx,4
		int	21h

                call    commandline             
		jnc	@goodparam
		jmp	@badparam

IFNDEF PS2
@goodparam:	lea	dx,cutestring
		mov	ah,9
                int     21h                     

		mov	ax,0
                int     33h                     
		IFz	ax,@goOn
                call    instexam             
		jz	@goOn
                lea     dx,alrStr               
		jmp	@printexit

@goOn:          IFz     forced,@detit         
@checkCom:	call	chkcom
		jz	@comOk
                jmp     @comnotfound            
@comOk:
                call    instexam               
		jnz	@notinstyet
                call    changeparam          


@detit:		mov	al,11h
		call	comport			; set COM1
                call    detmoustype           
                jnc     @notinstyet           
		mov	al,12h
                call    comport                
                call    detmoustype            
                jnc     @notinstyet            
		IFnz	noMouseSyst,@mouseNotFound;[AM]
		mov	al,11h
                call    comport                 
                jmp     @checkCom              
@mouseNotFound: lea     dx,nmfStr              
		jmp	@printexit

@notinstyet:    call    allochmem           
                call    saveoldIRQ              

ELSE

@goodparam:	lea	dx,cutestring
		mov	ah,9
                int     21h                    

		mov	ax,0
                int     33h                     
		IFz	ax,@goOn
                call    instexam                
		jz	@goOn
                lea     dx,alrStr             
		jmp	@printexit

@goOn:          call    checkPS2                
		jnc	@devfound
                lea     dx,dnfStr            
		jmp	@printexit

@devfound:      call    instexam             
		jnz	@notinstalled
                call    changeparam             
                                              
@notinstalled:  call    allochmem            
		mov	IRQintnum,74h
                call    saveoldIRQ             
ENDIF
                call    newvideohandler        
                call    saveold33             

                call    relocate                

                cli                             
                                             
                call    setnewIRQ               
		lds	dx,int10pointer
		mov	ax,2510h
                int     21h                    

		movSeg	ds,es
		lea	dx,handler
		mov	ax,2533h
                int     21h                   
		sti
		movSeg	ds,cs

		xor	ax,ax
                int     33h                     

                call    printmod&res            
IFNDEF PS2
                call    printport              
ENDIF
                cmp     mem_type,3          
		je	tsrexit

		mov	ax,4c00h
                int     21h                    

tsrexit:	movSeg	ds,cs
                mov     es,ds:[2ch]           
		mov	ah,49h
		int	21h

		mov	ax,3100h
		lea	dx,IRQ_number
                shr     dx,4                   
		inc	dx
		int	21h

;--------------------------
IFNDEF PS2
@comnotfound:   lea     dx,ncfStr            
		jmp	@printexit
ENDIF
;-------------------------

@badparam:      lea     dx,badswstring         
@printexit:	mov	ah,9
		int	21h
                mov     ax,4CFFh               
		int	21h

IFNDEF PS2

checktype	proc

                cmp     mousetype,2    
		je	@Mavail
                cmp     mousetype,3    
		jne	@Mnotavail

@Mavail:	call	disableCOMint
                call    chkcom         
                jnz     @Mnotavail     

		mov	si,IO_number
		call	mouseHWreset
                cmp     mousetype,2    
		jne	@notMSm
		call	detmicrosoft
		jc	@Mnotavail
		jmp	@setMSm

@notMSm:        mov     al,8            
		IFz	logitech?,@write8
                add     al,3            
@write8:	mov	dx,si
		add	dx,4
                out     dx,al          
                                       
@setMSm:	mov	al,1
		mov	dx,si
		inc	dx
                out     dx,al           
		clc
		ret

@Mnotavail:	stc
		ret

checktype	endp


mouseHWreset	proc

		mov	al,80h
		mov	dx,si
		add	dx,3		; Reg3
                out     dx,al         

		mov	al,60h
		mov	dx,si		; Reg0
                out     dx,al           

		xor	al,al
		inc	dx		; Reg1
                out     dx,al         

                mov     al,07h         
		inc	dx
		inc	dx		; Reg3
                out     dx,al         

		xor	al,al
		dec	dx
		dec	dx		; Reg1
                out     dx,al           

		mov	al,1
		add	dx,3		; Reg4
                out     dx,al        
		xor	cx,cx
		loop	$	;[AM99]

		inc	dx		; Reg5
                in      al,dx          
		ret

mouseHWreset	endp


detmicrosoft	proc
                                        
		mov	al,0Bh
		mov	dx,si
		add	dx,4
                out     dx,al          
		mov	dx,si
                in      al,dx           
		xor	bx,bx

getM:		xor	cx,cx
chkDTR:		mov	dx,si
		add	dx,5
                in      al,dx         
                and     al,1          
                jnz     DTRok       
                loop    chkDTR         
                jmp     DTRnotok      

DTRok:		mov	dx,si
                in      al,dx          
                and     ax,07fh         
                cmp     al,33h         
		jne	@not3
		mov	logitech?,1
@not3:          cmp     al,4Dh         
                jne     @notM        
		mov	microsoft?,1
@notM:		inc	bx
                cmp     bx,4            
                jb      getM            

DTRnotok:	IFz	microsoft?,@setdeterror
		IFnz	logitech?,@setdeterror
		clc
		ret

@setdeterror:	stc
		ret

detmicrosoft	endp

comport		proc

		push	ds
		push	0
		pop	ds
		cmp	al,11h			; '1'
		jne	notcom1
		mov	cs:com_port,'1'
		mov	ax,ds:[400h]
		mov	cs:IO_number,ax
                mov     cs:PICstate,10h        
		mov	cs:IRQ_number,4
		mov	cs:IRQintnum,0Ch
		jmp	endportset

notcom1:	cmp	al,12h			; '2'
		jne	notcom2
		mov	cs:com_port,'2'
		mov	ax,ds:[402h]
		mov	cs:IO_number,ax
                mov     cs:PICstate,8           
		mov	cs:IRQ_number,3
		mov	cs:IRQintnum,0Bh
		jmp	endportset

notcom2:	cmp	al,13h			; '3'
		jne	notcom3
		mov	cs:com_port,'3'
		mov	ax,ds:[404h]
		mov	cs:IO_number,ax
		mov	cs:PICstate,10h
		mov	cs:IRQ_number,4
		mov	cs:IRQintnum,0Ch
		jmp	endportset

notcom3:	cmp	al,14h			; '4'
		jne	notCOM
		mov	cs:com_port,'4'
		mov	ax,ds:[406h]
		mov	cs:IO_number,ax
		mov	cs:PICstate,8
		mov	cs:IRQ_number,3
		mov	cs:IRQintnum,0Bh

endportset:	pop	ds
		clc
		ret

notCOM:		pop	ds
		stc
		ret

comport		endp


printport	proc

                lea     dx,instStr           
		mov	ah,9
		int	21h

		lea	dx,com_port		; 'COM x'
		mov	ah,9
                int     21h                    
		ret

printport	endp

setIRQ		proc

		cmp	al,'I'
		jne	endsetIRQ
		lodsb
		cmp	al,33h
		jb	endsetIRQ
		cmp	al,40h
		jb	numa
		and	al,0dfh
		cmp	al,'F'
		ja	endsetIRQ
		sub	al,7
numa:		sub	al,30h
		mov	cs:IRQ_number,al
		cmp	al,8
		jb	numa2
		add	al,60h
numa2:		add	al,8
		mov	cs:IRQintnum,al
		jmp	oksetIRQ

endsetIRQ:	stc
		ret

oksetIRQ:	clc
		ret

setIRQ		endp


detmoustype	proc

                mov     mousetype,2             
		call	checktype
		jnc	microinst
                mov     mousetype,3          
                call    checktype              
		jc	nomice
                IFz     logitech?,nomice     

microinst:      call    instexam               
                jnz     notyetinst            
                call    changeparam            

notyetinst:	clc
		ret
nomice:         stc
		ret

detmoustype	endp

changeIRQ	proc

		push	cs
		pop	ds
		cli
		call	setIRQparams
		mov	al,PICstate
		mov	es:PICstate,al
		mov	ax,IO_number
		mov	es:IO_number,ax
		mov	al,IRQ_number
		mov	es:IRQ_number,al
		mov	ah,25h
		lds	dx,oldIRQaddr
                int     21h                   

                call    setnewIRQ        
		mov	al,cs:IRQintnum
		mov	es:IRQintnum,al
		sti
		ret

changeIRQ	endp

setIRQparams	proc

		push	dx si
		mov	si,es:IO_number
		xor	al,al
		mov	dx,si
		inc	dx
                out     dx,al         

		add	dx,3
                in      al,dx         
                and     al,0F3h         
		out	dx,al

		inc	dx
                in      al,dx         

		mov	dx,si
                in      al,dx         
		pop	si dx
		ret

setIRQparams	endp

ENDIF

printhelp	proc

		lea	dx,CMDStr
		mov	ah,9
                int     21h                   

		mov	ax,4C00h
                int     21h                 

printhelp	endp

instexam	proc

		mov	ax,3533h
                int     21h                    
		cmp	bx,offset handler
		ret

instexam	endp

commandline	proc

		mov	si,81h
		cld
newopt:		call	parsecmdl
		lodsb
                cmp     al,0Dh                
		je	noswitch
                cmp     al,0Ah                
		je	noswitch
                test    al,al                   
		je	noswitch
		cmp	al,'/'
		jne	badswitch

		lodsb
		cmp	al,'?'
                jne     @nohelp               
		call	printhelp

@nohelp:	and	al,0dfh
                cmp     al,'U'               
		jne	@noreldriv
		call	reldriver

@noreldriv:	call	resolution
		jnc	newopt
		cmp	al,'L'
		jne	@notleft
		mov	lefthand?,1
		jmp	newopt

@notleft:
IFNDEF PS2
                call    comport                
		jnc	newopt
		call	setIRQ
		jnc	newopt
                cmp     al,'M'                  
		jne	nofmi
		mov	mousetype,2
		mov	logitech?,0
force:		mov	forced,1
		jmp	newopt

nofmi:          cmp     al,'T'                  
		jne	noflog
		mov	mousetype,3
		mov	logitech?,1
		jmp	force

noflog:         cmp     al,'S'              
		jne	nofsys
		mov	mousetype,3
		mov	logitech?,0
		jmp	force

nofsys:
                cmp     al,'N'                 
		jne	noNoDef
		mov	noMouseSyst,1
		jmp	newopt
noNoDef:
ENDIF
badswitch:	stc
		ret

noswitch:	clc
		ret

commandline	endp

;ßßßßßßßßßßßßßßßßßßßßßßßßßßßßß

parsecmdl	proc

pars_st:	lodsb
		cmp	al,' '
		je	pars_st
		cmp	al,9			;tab?
		je	pars_st
		dec	si
		ret

parsecmdl	endp

whichhand	proc

		cmp	al,'L'
		jne	righthand
                mov     lefthand?,1           
		clc
		ret

righthand:      mov     lefthand?,0         
		stc
		ret

whichhand	endp

resolution	proc

IFNDEF PS2
		and	al,0DFh
ENDIF
		cmp	al,'R'
		jne	@endsetres

		lodsb
		cmp	al,'0'
		je	@autores
                cmp     al,'1'                  
		jb	@endsetres
                cmp     al,'9'                 
		ja	@endsetres
                mov     times,al               
		sub	al,31h
		mov	byte ptr mresolution,al
		lodsb
		cmp	al,'0'
		je	@autores
		cmp	al,'1'
		jb	@no2ndres
		cmp	al,'9'
		ja	@no2ndres
@store2nd:	mov	times[2],al
		sub	al,31h
		mov	byte ptr mresolution[1],al
		mov	autores?,0
		jmp	@oksetres

@no2ndres:	mov	al,byte ptr times
                dec     si                     
		jmp	@store2nd

@autores:	mov	autores?,1
		mov	word ptr mresolution,0
@oksetres:	clc
		ret

@endsetres:	stc
		ret

resolution	endp

reldriver	proc

		call	instexam
		jnz	notinst

                push    es                    
                lea     dx,relStr              
		mov	ah,9
                int     21h                     
		mov	ax,1Fh
                int     33h                    
		pop	es

		push	ds
		lds	dx,es:oldint33
		mov	ax,2533h
                int     21h                    

		mov	ax,es
		dec	ax
		mov	ds,ax
		mov	ah,62h
		int	21h
                mov     ds:[1],bx               
		pop	ds
		call	FreeMem
		jc	notinst

		mov	ax,4C00h
                int     21h                    

notinst:        lea     dx,mnsStr           
		mov	ah,9
                int     21h                    
		mov	ax,4CFFh
                int     21h                 

reldriver	endp

changeparam	proc

		mov	al,lefthand?
		mov	es:lefthand?,al
		mov	ax,mresolution
		mov	es:mresolution,ax
                mov     al,autores?             
		mov	es:autores?,al
                call    printmod&res       

		push	es
		mov	ax,1Fh
                int     33h                     
		pop	es

		mov	al,mousetype
                mov     es:mousetype,al     

IFNDEF PS2
		mov	al,logitech?
		mov	es:logitech?,al
ENDIF
		mov	ax,20h
                int     33h                    

IFNDEF PS2
		call	printport
ENDIF

		call	printalready

IFNDEF PS2
                mov     al,IRQintnum           
                cmp     al,es:IRQintnum       
                je      nointch              
                call    changeIRQ            
ENDIF

nointch:	mov	ax,4C00h
                int     21h                   

changeparam	endp

newvideohandler	proc

                call    detVGA                
		xor	ax,ax
		mov	ds,ax
                mov     ah,ds:[449h]          
                call    displayports            
		movSeg	ds,cs

		push	es
		mov	ax,3510h
                int     21h                   
		saveFAR	oldint10,es,bx
		pop	es
		saveFAR int10pointer,es,<offset int10handler>

		ret

newvideohandler	endp

detVGA		proc

		mov	ax,1A00h
		xor	bx,bx
                int     10h             
		cmp	al,1Ah
		jne	@itsnotVGA

                cmp     bl,7         
		je	@itsaVGA
                cmp     bl,8           
		jne	@itsnotVGA

@itsaVGA:	mov	VGAindicat,1

@itsnotVGA:	ret

detVGA		endp


saveoldIRQ	proc

IFNDEF PS2
		lea	dx,IRQhandler
ELSE
		lea	dx,handleIRQ
ENDIF
		saveFAR newIRQaddr,es,dx

		push	es
		mov	al,IRQintnum
		mov	ah,35h
                int     21h                     
		saveFAR oldIRQaddr,es,bx
		pop	es
		ret

saveoldIRQ	endp

saveold33	proc

		push	es
		mov	ax,3533h
                int     21h                   
		mov	word ptr oldint33,bx
		mov	word ptr oldint33+2,es
		pop	es
		ret

		endp

setnewIRQ	proc

		push	ds
		mov	al,IRQintnum
		mov	ah,25h
		lds	dx,newIRQaddr
                int     21h                    
		pop	ds
		ret

		endp

printalready	proc

                lea     dx,alrStr              
		mov	ah,9
                int     21h                   

		ret

printalready	endp

printmod&res	proc

		push	es
IFNDEF PS2
                cmp     mousetype,3             
		jne	@micromode
                IFz     logitech?,@notlogi             

		lea	dx,datalogi
		jmp	@printmode

@notlogi:       lea     dx,MSMStr              
		jmp	@printmode

@micromode:     lea     dx,MSStr                
ELSE
		lea	dx,drivmod
ENDIF

@printmode:	mov	ah,9
		int	21h

                lea     dx,resol         
		mov	ah,9
		int	21h

		IFz	autores?,@noprauto
		lea	dx,auto
		jmp	@prres
@noprauto:	lea	dx,times
@prres:		mov	ah,9
		int	21h

		pop	es
		ret

printmod&res	endp

;-------------------

SaveMemStrat	dw	0
SaveUMBLink	db	0

getXMSaddr	proc
		xor	bx,bx
		mov	es,bx
                mov     ax,4310h       
		int	2Fh
		saveFAR XMSentry,es,bx
		mov	ax,es
                or      ax,bx        
		ret
getXMSaddr	endp

GetAllocStrat	proc
                mov     ax,5800h       
		int	21h
                jc      @@fingas        
		mov	SaveMemStrat,ax
                mov     ax,5802h        
		int	21h
		mov	SaveUMBLink,al
@@fingas:	ret
GetAllocStrat	endp


ResAllocStrat	proc
                mov     ax,5801h                
		mov	bx,SaveMemStrat
		int	21h
                mov     ax,5803h               
		mov	bl,SaveUMBLink
		xor	bh,bh
		int	21h
		ret
ResAllocStrat	endp

AllocMem        proc                            

		push	ds es
		mov	mem_type,0
		mov	dest_seg,0
		add	ax,0Fh
		shr	ax,4
		mov	paragraphs,ax


		mov	ah,30h
		int	21h
                cmp     al,5                   
		jb	@@noDOS5UMBs

		call	GetAllocStrat
		jc	@@noDOS5UMBs

                mov     ax,5801h               
                xor     bx,bx               
		int	21h
                jc      @@noDOS5UMBs            

                mov     ax,5803h             
                mov     bx,1                    
		int	21h
		jc	@@noDOS5UMBs

                mov     ax,5801h              
                mov     bx,41h          
		int	21h
                jnc     @@linkUMB               

                mov     ax,5801h       
                mov     bx,81h         
		int	21h
                jc      @@dev5        

@@linkUMB:      mov     ax,5803h        
		mov	bx,1
		int	21h
                mov     ah,48h         
		mov	bx,paragraphs
		int	21h
		jc	@@dev5
                cmp     ax,0A000h       
                ja      @@Dumb_OK       

                mov     es,ax           
		mov	ah,49h
		int	21h
                stc                   
		jmp	@@dev5

@@Dumb_OK:	clc
@@dev5:		pushf
		push	ax
                call    ResAllocStrat   
		pop	ax
		popf
                jc      @@noDOS5UMBs    
                mov     dest_seg,ax  
                mov     mem_type,1     
		jmp	@@fin

; try XMS driver to allocate UMB

@@noDOS5UMBs:   mov     ax,4300h     
		int	2Fh
		cmp	al,80h
		jne	@@noXMS_UMB
		call	getXMSaddr
		jz	@@noXMS_UMB
                mov     ah,10h         
		mov	dx,paragraphs
		call	XMSentry
		cmp	ax,1
		jne	@@noXMS_UMB
		cmp	dx,paragraphs
		jb	@@noAlcanza
		mov	dest_seg,bx
                mov     mem_type,2     
		jmp	@@fin

@@noAlcanza:	mov	dx,bx
                mov     ah,11h         
		call	XMSentry

; use conventional memory

@@noXMS_UMB:	mov	ax,cs
                mov     dest_seg,ax    
                mov     mem_type,3     
		clc

@@fin:		pop	es ds
		mov	ax,dest_seg
		ret
AllocMem	endp

FreeMem		proc

		push	ds es
		mov	al,es:mem_type
		cmp	al,2
		je	@@xms
		cmp	al,3
		je	@@finConv

@@dos5:		call	GetAllocStrat
                jc      @@finfree      
                mov     ax,5803h      
		xor	bx,bx
		int	21h
                mov     ah,49h         
		int	21h
		call	ResAllocStrat
		jmp	@@finfree

@@xms:		call	getXMSaddr
                jz      @@finfree     
                mov     ah,11h        
		mov	dx,es
		call	XMSentry
		jmp	@@finfree

@@finConv:      mov     ah,49h         
		int	21h
@@finfree:	pop	es ds
		ret

FreeMem		endp


allochmem	proc

                mov     ax,offset IRQ_number   
                call    AllocMem             
		jnc	@okmem

		mov	ah,9
                lea     dx,noemem              
		int	21h
		mov	ax,4cffh
		int	21h

@okmem:		cmp	mem_type,3
		je	convent
		mov	bx,ax
		dec	bx
                mov     es,bx                 
		mov	word ptr es:[1],ax

		lea	si,ctstr
		mov	di,8
		mov	cx,8
                rep     movsb                  

		mov	bx,ax
		add	bx,word ptr es:[3h]

		mov	es,ax
		mov	dx,ax
                mov     ah,26h                
		int	21h
                mov     es:[2h],bx             
		ret

convent:	mov	es,ax
		ret

		endp

relocate	proc

                mov     si,100h               
                mov     di,si                  
		mov	cx,offset IRQ_number-offset start
		rep	movsb
		ret

		endp

label endProgram

		end	start
