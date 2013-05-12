;------------------------------------------------------------------------------
;		F R E E - D O S		X M S - D R I V E R
;------------------------------------------------------------------------------
;
; Improvments by Martin Str”mberg.
; Copyright 2001, Martin Str”mberg.
; 
; Martin can be reached at <ams@ludd.luth.se>. Please start the subject line 
; with "FDXMS". If you have a bug report, use bugtrack on the FreeDOS site,
; <http://www.freedos.org>. Make sure to include the FDXMS.SYS version number.
;
;
; Written by Till Gerken for the Free-DOS project.
;
; If you would like to use parts of this driver in one of your projects, please
; check up with me first.
;
; Till can be reached at:	till@tantalo.net (Internet)
;
; For questions concerning Free-DOS, mail the coordinator
; Morgan "Hannibal" Toal <hannibal@iastate.edu>
;
; Comments and bug reports are always appreciated.
;
; Copyright (c) 1995, Till Gerken
;------------------------------------------------------------------------------
; -- IMPLEMENTATION NOTES --
;
; - The Protected Mode handling is very simple, it fits only for the least
;   needs
; - I didn't care about reentrancy. If this thing should be used in
;   Multitasking Environments, well, somebody got to take a look at the code.
; - INT15h, Func. 87h (Move Block) has been re-implemented to preserve
;   the state of A20. (very slow!)
; - INT15h, Func. 88h (Extended Memory Size) has been re-implemented to
;   return 0.
; - Function 0Bh (Move Block) uses it's own Protected Mode handling.
;   It doesn't provide Interrupt windows.
; - The code is not very optimised, I just wrote it down for now. Later, when
;   everything is tested, I'm going to see what can be done
; - Some ideas were taken from the original XMS driver written by
;   Mark E. Huss (meh@bis.adp.com), but everything has been completely
;   rewritten, so if there are bugs, they are mine, not his. ;)
;------------------------------------------------------------------------------

ideal
P386                                                          
jumps

include "xms.inc"                                               
segment code16
assume cs:code16, ds:code16, es:nothing

 
                dd      -1                     
                dw      8000h                
                dw      offset strategy       
                dw      offset interrupt      
                db      'XMSXXXX0'            

 
request_ptr     dd      ?                    

initialized     db      0                      
                                            

xms_size        dw      ?                      

gdt32		dw	gdt_size,dummy,0
dummy		dq	0
code16dsc	db	0ffh,0ffh,0,0,0,9ah,0,0
core32dsc	db	0ffh,0ffh,0,0,0,92h,0cfh,0
gdt_size=$-(offset dummy)

code16idx	=	08h
core32idx	=	10h

old_int15       dd      ?                     
old_int2f       dd      ?                     

hma_used        db      0                  
hma_min         dw      0                   
                                               
a20_locks       dw      0                      

xms_num_handles dw      32                  

   
proc	strategy	far
        mov     [word cs:request_ptr+2],es   
        mov     [word cs:request_ptr],bx     
        ret                                   
endp	strategy

 
proc	interrupt	far
	push	di es

	mov	di,cs
	mov	ds,di

        les     di,[request_ptr]               

        cmp     [es:di+request_hdr.cmd],CMD_INIT
	jne	done
        cmp     [initialized],0              
                                             
	jne	done
        call    initialize               
done:

        lds     si,[request_ptr]               

	pop	es di
        ret                                  
endp	interrupt

 
proc	delay
	jmp	short $+2
	jmp	short $+2
	ret
endp	delay

 
proc	empty_8042
        call    delay                 
	in	al,64h
        test    al,1                  
        jz      no_output             
        call    delay                  
        in      al,60h                 
        jmp     short empty_8042       
no_output:
        test    al,2              
        jnz     empty_8042          
        ret                     
endp	empty_8042

 
proc	enable_a20
        call    empty_8042           
        mov     al,0d1h            
	out	64h,al
	call	empty_8042
        mov     al,0dfh             
	out	60h,al
	call	empty_8042
	ret
endp	enable_a20

 
proc	disable_a20
        call    empty_8042             
        mov     al,0d1h              
	out	64h,al
	call	empty_8042
        mov     al,0ddh             
	out	60h,al
	call	empty_8042
	ret
endp	disable_a20

 
proc	test_a20
	push	cx si di ds es

	xor	si,si
	mov	ds,si

	mov	di,0ffffh
	mov	es,di
	mov	di,10h

	mov	cx,100h/4
	rep	cmpsd

	pop	es ds di si cx
	ret
endp	test_a20

 
vdisk_id	db	VDISK_IDSTR

proc	check_vdisk
	push	ax bx cx si di es

;       call    enable_a20            

        xor     ax,ax               
	mov	es,ax
	les	bx,[es:19h*4]

	mov	di,VDISK_IDOFS
	mov	si,offset vdisk_id
	mov	cx,VDISK_IDLEN
        rep     cmpsb                  
;	jnz	no_vdisk
;
;	mov	ax,0ffffh
;	mov	es,ax
;	mov	di,VDISK_IDOFS+1
;	mov	si,offset vdisk_id
;	mov	cx,VDISK_IDLEN
;	rep	cmpsb
;
;no_vdisk:
;
;       pushf                         
;	call	disable_a20
;	popf

	pop	es di si cx bx ax
	ret
endp	check_vdisk

 
 
a20state        db      ?              

proc	int15_handler
        cmp     ah,87h                        
	je	do_move
        cmp     ah,88h                   
	je	ext_mem_size
        jmp     [cs:old_int15]            
do_move:
        call    test_a20                    
	jz	a20disabled
        mov     [cs:a20state],1             
	jmp	call_old_mover
a20disabled:
	mov	[cs:a20state],0
call_old_mover:
        pushf                                 
	call	[cs:old_int15]
        pushf                                   
	push	ax
        cmp     [cs:a20state],0               
	jz	disable_it
	call	enable_a20
	jmp	move_done
disable_it:
	call	disable_a20
move_done:
	pop	ax
	popf
	iret
ext_mem_size:
        xor     ax,ax                          
        clc                                   
	iret
endp	int15_handler

 
proc	int2f_handler
	pushf
        cmp     ax,4300h                       
	jne	driver_address
        mov     al,80h                     
	popf
	iret
driver_address:
        cmp     ax,4310h                     
	jne	call_old2f
	mov	bx,cs
	mov	es,bx
	mov	bx,offset xms_dispatcher
	popf
	iret
call_old2f:
	popf
        jmp     [cs:old_int2f]               
endp	int2f_handler

proc	xms_get_version
	pop	bx
	mov	ax,INTERFACE_VER
	mov	bx,ax
        mov     dx,1                          
	popf
	retf
endp	xms_get_version

proc	xms_request_hma
	pop	bx
        cmp     [cs:hma_used],0                 
	mov	bl,XMS_HMA_IN_USE
	jnz	xrh_err
        cmp     dx,[cs:hma_min]              
	mov	bl,XMS_HMAREQ_TOO_SMALL
	jb	xrh_err
        mov     [cs:hma_used],1                
	mov	ax,1
	xor	bl,bl
	popf
	retf
xrh_err:
	xor	ax,ax
	popf
	retf
endp	xms_request_hma

proc	xms_release_hma
	pop	bx
        cmp     [cs:hma_used],0                 
	mov	bl,XMS_HMA_NOT_USED
	jz	xrh_err
        mov     [cs:hma_used],0            
	mov	ax,1
	xor	bl,bl
	popf
	retf
endp	xms_release_hma

proc	xms_global_enable_a20
	pop	bx
        call    enable_a20                    
        call    test_a20                   
	jz	xge_a20_err
	mov	ax,1
	xor	bl,bl
	popf
	retf
xge_a20_err:
	xor	ax,ax
	mov	bl,XMS_A20_FAILURE
	popf
	retf
endp	xms_global_enable_a20

proc	xms_global_disable_a20
	pop	bx
        call    disable_a20                   
        call    test_a20                      
	jnz	xge_a20_err
	mov	ax,1
	xor	bl,bl
	popf
	retf
endp	xms_global_disable_a20

proc	xms_local_enable_a20
	pop	bx
        inc     [cs:a20_locks]                 
        call    enable_a20                   
        call    test_a20                        
	jz	xge_a20_err
	mov	ax,1
	xor	bl,bl
	popf
	retf
endp	xms_local_enable_a20

proc	xms_local_disable_a20
	pop	bx
        dec     [cs:a20_locks]                
        jnz     xld_dont_disable            
        call    disable_a20                   
        call    test_a20                     
	jnz	xge_a20_err
xld_dont_disable:
	mov	ax,1
	xor	bl,bl
	popf
	retf
endp	xms_local_disable_a20

proc	xms_query_a20
	pop	bx
        xor     ax,ax                   
	call	test_a20
	jz	xqa_a20dis
	mov	ax,1
xqa_a20dis:
	xor	bl,bl
	popf
	retf
endp	xms_query_a20

proc	xms_find_free_block
        mov     bx,offset driver_end  
        mov     cx,[xms_num_handles]   
find_free_block:
        cmp     [bx+xms_handle.used],0  
        jnz     xms_find_next_free_block
        cmp     [bx+xms_handle.xbase],0 
        jnz     found_block            
xms_find_next_free_block:
        add     bx,size xms_handle     
        loop    find_free_block        
        stc                            
	ret
found_block:
        clc                          
	ret
endp	xms_find_free_block

proc	xms_find_free_handle
        mov     bx,offset driver_end    
        mov     cx,[xms_num_handles]   
find_free_handle:
        cmp     [bx+xms_handle.used],0        
        jnz     xms_find_next_free_handle     
        cmp     [bx+xms_handle.xbase],0        
        jz      found_handle                
xms_find_next_free_handle:
        add     bx,size xms_handle      
        loop    find_free_handle       
        stc                            
	ret
found_handle:
        clc                         
	ret
endp	xms_find_free_handle

proc	xms_query_free_xms
	push	cx si ds

	mov	ax,cs
	mov	ds,ax

        xor     ax,ax                       
        xor     dx,dx                         

        call    xms_find_free_block         
	jc	no_free_xms

check_next:
        mov     si,[bx+xms_handle.xsize]        
        add     dx,si                         
        cmp     si,ax                          
	jbe	not_larger
        mov     ax,si                      
not_larger:
	call	xms_find_next_free_block
	jnc	check_next

	pop	ds si cx bx
	xor	bl,bl
	popf
	retf

no_free_xms:
	pop	ds si cx bx
	mov	bl,XMS_ALREADY_ALLOCATED
	popf
	retf
endp	xms_query_free_xms

proc	xms_alloc_xms
	push	cx si ds

	mov	ax,cs
	mov	ds,ax

        call    xms_find_free_block    
        jnc     check_size           
no_free_mem:
	pop	ds si cx bx
	xor	ax,ax
	mov	bl,XMS_ALREADY_ALLOCATED
	popf
	retf

get_next_block:
	call	xms_find_next_free_block
	jc	no_free_mem
check_size:
        cmp     dx,[bx+xms_handle.xsize]       
        jg      get_next_block                

        mov     si,bx                 
        mov     [bx+xms_handle.used],1 

        call    xms_find_free_handle   
        jc      perfect_fit            
        mov     ax,[si+xms_handle.xsize]    
        sub     ax,dx                          
        jz      perfect_fit                  
        mov     [bx+xms_handle.xsize],ax       
	mov	[si+xms_handle.xsize],dx
        mov     ax,[si+xms_handle.xbase]       
        add     ax,dx                        
        mov     [bx+xms_handle.xbase],ax      
        mov     [bx+xms_handle.locks],0  

perfect_fit:
        mov     [si+xms_handle.locks],0         

        mov     dx,si                        

	pop	ds si cx bx
	mov	ax,1
	xor	bl,bl
	popf
	retf
endp	xms_alloc_xms

proc	xms_free_xms
	push	cx dx si ds

	mov	ax,cs
	mov	ds,ax

        mov     si,dx                        

        cmp     [si+xms_handle.locks],0    
        jz      not_locked                     
	pop	ds si dx cx bx
	xor	ax,ax
	mov	bl,XMS_BLOCK_LOCKED
	popf
	retf

not_locked:
        cmp     [si+xms_handle.xsize],0        
	jnz	normal_handle
        mov     [si+xms_handle.xbase],0     
	jmp	xms_free_done

normal_handle:
        mov     ax,[si+xms_handle.xbase]      
        add     ax,[si+xms_handle.xsize]       

        call    xms_find_free_block            
        jc      xms_free_done                  
try_concat:
        cmp     ax,[bx+xms_handle.xbase]        
	jne	not_adjacent
        mov     dx,[bx+xms_handle.xsize]     
	add	ax,dx
	add	[si+xms_handle.xsize],dx
        mov     [bx+xms_handle.xbase],0      
	mov	[bx+xms_handle.xsize],0
not_adjacent:
        call    xms_find_next_free_block       
	jnc	try_concat

xms_free_done:
        mov     [si+xms_handle.used],0         
	pop	ds si dx cx bx
	mov	ax,1
	xor	bl,bl
	popf
	retf
endp	xms_free_xms

proc	xms_get_move_addr
        or      bx,bx                   
	jnz	dont_translate
        movzx   ecx,dx                
        xor     dx,dx                 
        shr     edx,12                 
        add     edx,ecx              
        xor     ebx,ebx                 
	jmp	no_handle
dont_translate:
        movzx   ebx,[cs:bx+xms_handle.xbase]   
        shl     ebx,10                        
no_handle:
        add     ebx,edx                        
	ret
endp	xms_get_move_addr

proc	xms_move_xms
	pop	bx
	pushad
	push	ds es
        cli                                     

	mov	eax,cr0
        test    al,1                       
        jnz     a20_failure                  

	mov	bx,[si+xms_move_strc.dest_handle]
	mov	edx,[si+xms_move_strc.dest_offset]
        call    xms_get_move_addr              
        mov     edi,ebx                        

	mov	bx,[si+xms_move_strc.src_handle]
	mov	edx,[si+xms_move_strc.src_offset]
        call    xms_get_move_addr            

        mov     ecx,[si+xms_move_strc.len]    
        test    cl,1                          
	jnz	invalid_length

        mov     esi,ebx                        

        call    test_a20                     
        pushf                                 
        call    enable_a20                    

        lgdt    [fword cs:gdt32]           
	mov	eax,cr0
        or      al,1                          
        mov     cr0,eax                       
        db      0eah                          
        dw      offset to_pm,code16idx         
to_pm:

	mov	ax,core32idx
	mov	ds,ax
	mov	es,ax

        shr     ecx,2                          
        jnc     dword_boundary                 
        movs    [word esi],[word edi]          
dword_boundary:
        rep     movs [dword esi],[dword edi]    

	mov	eax,cr0
        and     al,not 1                       
        mov     cr0,eax                        

        db      0eah                         
	dw	offset to_rm
code_seg dw     ?                              

to_rm:
        popf                          
        jnz     a20_was_enabled        
        call    disable_a20            
a20_was_enabled:

	pop	es ds
        popad                                  
	popf
        mov     ax,1                        
	retf

a20_failure:
	pop	es ds
	popad
	popf
	xor	ax,ax
	mov	bl,XMS_A20_FAILURE
	retf
invalid_length:
	pop	es ds
	popad
	popf
	xor	ax,ax
	mov	bl,XMS_INVALID_LENGTH
	retf
endp	xms_move_xms

proc	xms_lock_xms
	pop	bx
	mov	bx,dx
        inc     [cs:bx+xms_handle.locks]        
        jnc     locked_successful             
	xor	ax,ax
        mov     bl,XMS_LOCK_COUNT_OVERFLOW      
	popf
	retf
locked_successful:
        push    eax                           
        movzx   eax,[cs:bx+xms_handle.xbase]  
        shl     eax,10                        
        mov     bx,ax                          
	shr	eax,16
        mov     dx,ax                       
        pop     eax                             
	mov	ax,1
	popf
	retf
endp	xms_lock_xms

proc	xms_unlock_xms
	mov	bx,dx
        cmp     [cs:bx+xms_handle.locks],0      
        jnz     is_locked                    
	pop	bx
	xor	ax,ax
	mov	bl,XMS_BLOCK_NOT_LOCKED
	popf
	retf
is_locked:
        dec     [cs:bx+xms_handle.locks]       
	pop	bx
	mov	ax,1
	xor	bl,bl
	popf
	retf
endp	xms_unlock_xms

proc	xms_get_handle_info
	pop	bx
	push	cx si ds

	mov	ax,cs
	mov	ds,ax

	mov	si,dx

        xor     dl,dl                         
        call    xms_find_free_handle            
	jc	nothing_free
find_next_free:
        inc     dl                            
        call    xms_find_next_free_handle      
	jnc	find_next_free

nothing_free:
        mov     bl,dl                         
        mov     bh,[si+xms_handle.locks]      
        mov     dx,[si+xms_handle.xsize]      

	pop	ds si cx
	mov	ax,1
	popf
	retf
endp	xms_get_handle_info

proc	xms_realloc_xms
	pop	bx
	push	bx dx si ds
	mov	ax,cs
	mov	ds,ax

	xchg	bx,dx
	cmp	dx,[bx+xms_handle.xsize]
	jbe	shrink_it

no_xms_handles_left:
	pop	ds si dx bx
	xor	ax,ax
        mov     bl,XMS_NO_HANDLE_LEFT           
	popf
	retf

shrink_it:
	mov	si,bx
        call    xms_find_free_handle          
        jc      no_xms_handles_left            
        mov     ax,[si+xms_handle.xsize]    
	mov	[si+xms_handle.xsize],dx
        sub     ax,dx                          
        jz      dont_need_handle              
        add     dx,[si+xms_handle.xbase]      
        mov     [bx+xms_handle.xbase],dx     
        mov     [bx+xms_handle.xsize],ax       
        mov     [bx+xms_handle.locks],0      
        mov     [bx+xms_handle.used],0          
dont_need_handle:
	pop	ds si dx bx
	mov	ax,1
	xor	bl,bl
	popf
	retf
endp	xms_realloc_xms

proc	xms_request_umb
	pop	bx
        xor     ax,ax                   
        mov     bl,XMS_NO_UMB_AVAILABLE
        xor     dx,dx                  
	popf
	retf
endp	xms_request_umb

proc	xms_release_umb
	pop	bx
	xor	ax,ax
	mov	bl,XMS_NOT_IMPLEMENTED
	popf
	retf
endp	xms_release_umb

proc	xms_realloc_umb
	pop	bx
	xor	ax,ax
	mov	bl,XMS_NOT_IMPLEMENTED
	popf
	retf
endp	xms_realloc_umb
xms_table	dw	xms_get_version,xms_request_hma,xms_release_hma
		dw	xms_global_enable_a20,xms_global_disable_a20
		dw	xms_local_enable_a20,xms_local_disable_a20
		dw	xms_query_a20,xms_query_free_xms,xms_alloc_xms
		dw	xms_free_xms,xms_move_xms,xms_lock_xms,xms_unlock_xms
		dw	xms_get_handle_info,xms_realloc_xms,xms_request_umb
		dw	xms_release_umb,xms_realloc_umb

proc	xms_dispatcher
	jmp	short dispatcher_entry
        nop                                    
        nop                              
        nop                                 
dispatcher_entry:
        pushf                              
	cld
        cmp     ah,12h                       
	ja	not_supported
        call    check_vdisk                     
	jz	vdisk_installed
hook_patch:
	jmp	short hook_ints
hook_return:
	push	bx
	movzx	bx,ah
	shl	bx,1
	jmp	[cs:xms_table+bx]
not_supported:
        xor     ax,ax                       
	mov	bl,XMS_NOT_IMPLEMENTED
        popf                                 
	retf
vdisk_installed:
	xor	ax,ax
	mov	bl,XMS_VDISK_DETECTED
        popf                                  
	retf

hook_ints:
        or      ah,ah                          
        jz      hook_return                 

        push    ax bx cx dx ds es             
	mov	ax,cs
	mov	ds,ax
        xor     ax,ax                        
	mov	es,ax
	les	bx,[es:15h*4]
	mov	[word old_int15+2],es
	mov	[word old_int15],bx

        mov     ax,2515h                       
	mov	dx,offset int15_handler
	int	21h

        mov     cx,[xms_num_handles]           
        mov     bx,offset driver_end            
clear_table:
        mov     [bx+xms_handle.xbase],0    
        mov     [bx+xms_handle.xsize],0     
        mov     [bx+xms_handle.used],0       
	add	bx,size xms_handle
	loop	clear_table

	mov	bx,offset driver_end
        mov     [bx+xms_handle.xbase],XMS_START 
        mov     ax,[xms_size]                  
	mov	[bx+xms_handle.xsize],ax

        pop     es ds dx cx bx ax              

        mov     [word cs:hook_patch],9090h  
        jmp     short hook_return            
endp	xms_dispatcher

label	driver_end	near
proc	check_cpu
	pushf
	xor	ax,ax
	push	ax
	popf
	pushf
	pop	ax
	and	ah,0fh
	cmp	ah,0fh
	je	not386
	mov	ah,7
	push	ax
	popf
	pushf
	pop	ax
	and	ah,7
	je	not386
	popf
	clc
	ret
not386:
	popf
	stc
	ret
endp	check_cpu

proc	check_a20
	call	enable_a20
        call    test_a20                       
	jz	a20failed
	call	disable_a20
        call    test_a20                      
	jnz	a20failed
	clc
	ret
a20failed:
	stc
	ret
endp	check_a20


init_message            db      13,10,80 dup ('*'),'Administrador de memoria',13,10
                        db      'Copyright (c) 1995, Till Gerken',13,10,13,10
                        db      'Version: ',DRIVER_VERSION,9
                        db      'Interface de la version: ',INTERFACE_VERSION,13,10
                        db      'Informacion de sistema: ',INFO_STR,13,10,'$'

old_dos                 db      'Para usar los XMS es necesaria la version 3.00 de DOS.$'
xms_twice               db      'XMS ya instalado.$'
vdisk_detected          db      'VDISK ha sido detectado.$'
no_386                  db      'Es requerido un ordenador superior a 80386.$'
a20_error               db      'Imposible de pasar a la direccion A20.$'
xms_sizeerr             db      'Imposible de determinar el tama¤o de la memoria.$'
xms_toosmall            db      'Memoria extendida muy peque¤a para utilizarse.$'

error_msg               db      ' Driver instalado.',7,13,10,'$'

init_finished           db      80 dup ('*'),13,10,'$'

proc	initialize
	pushf
	push	ax bx cx dx si

	cld

        mov     ah,9                            
	mov	dx,offset init_message
	int	21h

        mov     ax,3000h                        
	int	21h
        xchg    ah,al                           
        cmp     ax,300h                       
	mov	dx,offset old_dos
	jb	error_exit

        mov     ax,4300h                       
        int     2fh                       
	cmp	al,80h
	mov	dx,offset xms_twice
	je	error_exit

        call    check_cpu                      
	mov	dx,offset no_386
	jc	error_exit

        call    check_a20                  
	mov	dx,offset a20_error
	jc	error_exit

        call    check_vdisk                  
	mov	dx,offset vdisk_detected
	jz	error_exit

	clc
        mov     ah,88h                      
	int	15h
	mov	dx,offset xms_sizeerr
	jc	error_exit
	mov	dx,offset xms_toosmall
        sub     ax,64                           
        jc      error_exit                    
                                               

        mov     [xms_size],ax                  

        push    eax                         

        mov     ax,cs                         
        mov     [code_seg],ax                
	movzx	eax,ax
	shl	eax,4
	or	[dword code16dsc+2],eax
	add	[dword gdt32+2],eax

        pop     eax                            

	push	es
        xor     ax,ax                          
	mov	es,ax
	les	bx,[es:2fh*4]
	mov	[word old_int2f+2],es
	mov	[word old_int2f],bx

        mov     ax,252fh                       
	mov	dx,offset int2f_handler
	int	21h
	pop	es

        mov     [word es:di+2+init_strc.end_addr],cs    
	xor	dx,dx
	mov	ax,size xms_handle
	mul	[xms_num_handles]
	add	ax,offset driver_end
	mov	[word es:di+init_strc.end_addr],ax
        mov     [es:di+request_hdr.status],STATUS_OK   
	jmp	short exit

error_exit:
        mov     [word es:di+2+init_strc.end_addr],cs    
        mov     [word es:di+init_strc.end_addr],0     
                                                        
        mov     [es:di+request_hdr.status],STATUS_BAD  
        mov     ah,9                                   
	int	21h
	mov	dx,offset error_msg
	int	21h

exit:
	mov	ah,9
	mov	dx,offset init_finished
	int	21h
	pop	si dx cx bx ax
	popf
	ret
endp	initialize


ends	code16


end
