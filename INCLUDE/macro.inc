.xlist
;
; Far/near code
;
ifdef	LPROG
 _CType textequ	<far pascal>
 ARG1	equ	6
 DIST	textequ <far>
 PF	textequ	<dd>
 FPTR	textequ	<dword>
 LMOV	textequ	<mov>
 LPUSH	textequ	<push>
 LPOP	textequ	<pop>
else
 _CType textequ	<pascal>
 ARG1	equ	4
 DIST   textequ <near>
 PF	textequ	<dw>
 FPTR	textequ	<word>
 LMOV	macro	a,b,c,d,e,f
	endm
 LPUSH	macro	a,b,c
	endm
 LPOP	macro	a,b,c
	endm
endif
;
; Far/near data
;
ifdef 	LDATA
 DPTRZ	equ	4
 PD	textequ	<dd>
 MLDS	textequ	<lds>
 NLES	textequ	<les>
 MLES	textequ	<les>
 MPOP	textequ	<pop>
 MMOV	textequ	<mov>
 MPUSH	textequ	<push>
 DPTR	textequ	<dword>
 NMOV	macro	a,b,c,d,e,f
	endm
 NPUSH	macro	a,b,c
	endm
else
 DPTRZ 	equ	2
 PD	textequ	<dw>
 NPUSH	textequ	<push>
 MLDS	textequ	<mov>
 DPTR	textequ	<word>
 NMOV	textequ	<mov>
 MLES	macro	r,p
	push	ds
	pop	es
	mov	r,p
	endm
 NLES	macro	r,p
	mov	r,p
 ENDM
 MPOP	macro	reg
	endm
 MPUSH	macro	a,b,c
	endm
 MMOV	macro	a,b,c,d,e,f
	endm
endif
;
; Proc macros
;
global	textequ	<externdef>
globaln	macro name
name 	proto near
	endm
globald	macro name
name	proto dist
	endm

externd	macro w, t, n, a, b, x, d, e, f
	ifnb <a>
	 ifnb <b>
	  ifnb <x>
	   ifnb <d>
	    ifnb <e>
	     ifnb <f>
	      n w dist t a, b, x, d, e, f
	     else
	      n w dist t a, b, x, d, e
	     endif
	    else
	     n w dist t a, b, x, d
	    endif
	   else
	    n w dist t a, b, x
	   endif
	  else
	   n w dist t a, b
	  endif
	 else
	  n w dist t a
	 endif
	else
	 n w dist t
	endif
	endm

globalp	macro name, a, b, x, d, e, f
	externd proto, pascal, name, a, b, x, d, e, f
	endm

pproc	macro name, a, b, x, d, e, f
_TEXT	segment
	public	name
	externd	proc, pascal, name, a, b, x, d, e, f
	endm

globalc	macro name, a, b, x, d, e, f
	externd proto, c, name, a, b, x, d, e, f
	endm

cproc	macro 	name, a, b, x, d, e, f
_TEXT	segment
	public 	name
	externd	proc, c, name, a, b, x, d, e, f
	endm

dproc	macro 	name, a, b, x, d, e, f
_TEXT	segment
	public 	name
	name	proc dist
	endm

pend	macro name
name	endp
_TEXT	ends
	endm
;
; Data macros
;
pushm	macro	src
ifdef __386__
	push	dword ptr src
else
	push	word ptr src+2
	push	word ptr src
endif
	endm

popm	macro	src
	pop	word ptr src
	pop	word ptr src+2
	endm

stoso	macro	src, s, o
	mov	word ptr src,o
	mov	word ptr src+2,s
	endm

lodso	macro	src, s, o
	mov	o,word ptr src
	mov	s,word ptr src+2
	endm

lodm	macro	src
	mov	ax,word ptr src
	mov	dx,word ptr src+2
	endm

stom	macro	des
	mov	word ptr des,ax
	mov	word ptr des+2,dx
	endm

movmm	macro	des, src
	mov	ax,word ptr src
	mov	dx,word ptr src+2
	mov	word ptr des,ax
	mov	word ptr des+2,dx
	endm

movmw	macro	des, src
	mov	ax,word ptr src
	mov	word ptr des,ax
	endm

movmx	macro	des, src
      if @cpu and 8
      	mov	eax,dword ptr src
	mov	dword ptr des,eax
      else
	mov	ax,word ptr src+2
	mov	word ptr des+2,ax
	mov	ax,word ptr src
	mov	word ptr des,ax
      endif
	endm

incm	macro	src
ifdef __386__
	inc	dword ptr src
else
	add	word ptr src,1
	adc	word ptr src+2,0
endif
	endm

decm	macro	src
ifdef __386__
	dec	dword ptr src
else
	sub	word ptr src,1
	sbb	word ptr src+2,0
endif
	endm

cmprm	macro	src
local	@@
	cmp	dx,word ptr src+2
	jne	@@
	cmp	ax,word ptr src
	@@:
	endm

cmpmm	macro	des,src
ifdef __386__
	mov	eax,des
	cmp	eax,src
else
	lodm	des
	cmprm	src
endif
	endm

fpmov	macro	fp, lp
	LMOV	word ptr fp+2,cs
	mov	word ptr fp,offset lp
	endm

inv macro func, args:VARARG
    local id
    id = 0
    for arg,<args>
	if id eq 0
	    ifdifi <arg>,<ax>
	   	mov ax,arg
	    endif
	endif
	if id eq 1
	    ifdifi <arg>,<dx>
	    	mov dx,arg
	    endif
	endif
	if id eq 2
	    ifdifi <arg>,<bx>
	      	mov bx,arg
	    endif
	endif
	if id eq 3
	    ifdifi <arg>,<cx>
	    	mov cx,arg
	    endif
	endif
	id=id+1
	endm
    call func
    endm

cstr	macro text
	local xxx
	_data segment
	xxx db text,0
	_data ends
	exitm <ds:xxx>
	endm

func	macro args:VARARG
	invoke args
	exitm <ax>
	endm
;
;
; Debug macros
;
ifdef 	DEBUG
 assert_out  proto far
 assert_exit proto dist
 externdef _time_start:dword
 DADD	textequ	<add>
 DSUB	textequ	<sub>
 DMOV	textequ	<mov>
 DPUSH	textequ	<push>
 DPOP	textequ	<pop>
else
 DADD	macro	a,b,c,d,e,f
	endm
 DSUB	macro	a,b,c,d,e,f
	endm
 DMOV	macro	a,b,c,d,e,f
	endm
 DPUSH	macro	a,b,c
	endm
 DPOP	macro	a,b,c
	endm
endif

_clockS	macro
  ifdef DEBUG
	mov ax,0040h
	mov es,ax
	mov bx,006Ch
	movmm _time_start,es:[bx]
  endif
	endm

_clockE	macro
  ifdef DEBUG
	mov ax,0040h
	mov es,ax
	mov bx,006Ch
	lodm es:[bx]
	sub ax,word ptr _time_start
	sbb dx,word ptr _time_start+2
  endif
	endm

tracev	macro message,len
	local @@
  ifdef DEBUG
	push offset @@
	call assert_out
	db message,0
	@@:
	add sp,len
  endif
	endm

assert	macro A,B,J,message
	local @@
  ifdef DEBUG
	pushf
	cmp A,B
	&J @@
	popf
	call assert_exit
	DB "cmp &A&,&B& (&J&) - ",message,0
	@@:
	popf
  endif
	endm

assertf	macro J,message
	local @@
  ifdef DEBUG
	&J @@
	call assert_exit
	DB "flag (&J&) - ",message,0
	@@:
  endif
	endm

assertt	macro A,B,J,message
	local @@
  ifdef DEBUG
	pushf
	test A,B
	&J @@
	popf
	call assert_exit
	DB "test &A&,&B& (&J&) - ",message,0
	@@:
	popf
  endif
	endm
.list

