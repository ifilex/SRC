PVERSION equ '1.0.2'
TRUE equ -1
FALSE equ not TRUE
NOWARN PDC
ENABLECURSOR     equ TRUE  
ENABLEPASTE      equ TRUE  
ENABLEFILEAPPEND equ TRUE  

IFNDEF ENABLEPASTE
 ENABLEPASTE equ FALSE
 %OUT  ENABLEPASTE == FALSE. (Paste functions)
ENDIF
IFNDEF ENABLEFILEAPPEND
 ENABLEFILEAPPEND equ FALSE
 %OUT  ENABLEFILEAPPEND == FALSE. (File append, Win clipboard)
ENDIF
IFNDEF ENABLECURSOR
 ENABLECURSOR equ FALSE
 %OUT  ENABLECURSOR == FALSE. (Cursor functions)
ENDIF

IF ENABLEFILEAPPEND and (not ENABLEPASTE)
 %OUT Bad definitions for ENABLEFILEAPPEND and ENABLEPASTE.
 .ERR
ENDIF

;XT equ TRUE            
XT equ FALSE          
IF XT
 %OUT XT == TRUE. (Assuming PC-XT 8086/8088 cpu)
ELSE
 %OUT XT == FALSE. (Assuming 80286+ cpu)
ENDIF

if ENABLEPASTE
BUFFERLEN EQU 4096      
                      
                      

BUFLENDEFAULT EQU 2048  
                       
if  BUFFERLEN lt BUFLENDEFAULT
 %OUT  Inequality BUFLENDEFAULT <= BUFFERLEN violated.
 .ERR
endif
else
if ENABLECURSOR
BUFFERLEN EQU 0        
BUFLENDEFAULT EQU 0    
endif 
endif
M_MOVED       EQU 1B      
M_LT_PRESSED  EQU 10B    
M_LT_RELEASED EQU 100B     
M_RT_PRESSED  EQU 1000B   
M_RT_RELEASED EQU 10000B   
M_MI_PRESSED  EQU 100000B 
M_MI_RELEASED EQU 1000000B 
XPCMOUSEMASK  EQU (M_MOVED or M_LT_PRESSED or M_LT_RELEASED or M_RT_PRESSED or M_RT_RELEASED or M_MI_PRESSED or M_MI_RELEASED)

if not XT
 .286  
endif

locals                
PUSHR macro regs   
local reg
   irp reg,<regs>
     push reg
   endm
endm

POPR  macro regs  
local reg
   irp reg,<regs>
     pop  reg
   endm
endm

 XPUSHA macro
if XT
        PUSHR <AX,BX,CX,DX,BP,SI,DI>
else
        pusha
endif
 endm

 XPOPA macro
if XT
        POPR <DI,SI,BP,DX,CX,BX,AX>
else
        popa
endif
 endm

code segment
assume cs:code

begin_resident equ $   

XPC_pspadr dw 0          
BUFLEN  dw BUFLENDEFAULT     
biosSEG dw 40h             
flag_reinstall_exec db FALSE 
flag_reinstall_exit db TRUE  
flag_busy_stuffing db 0      
FlagREINSTALL  equ 00010000B 
FlagFILEAPPEND equ 00100000B
FlagWINCLIP    equ 01000000B 
FlagPASTE      equ 00000001B
FlagCURSOR     equ 00000010B 
flag_event db 00000000B     
counter_int10 dw 0           
flag_Left db FALSE          
flag_Right db FALSE         
flag_MouseOn db FALSE     
flag_swapbuttons db FALSE   
button_1fix db 1          
button_2fix db 3            
button_3fix db 5            
lineSize dw 0               
lasttime dw 0,0            
PositionLeft dw 0          
NextPositionLeft dw 0       
EndOfBuffer dw pastebuf+BUFFERLEN-1  

if ENABLEPASTE
lastChar db 0                
flag_Highlight db FALSE     
flag_CtrlRight db FALSE      
xor_mask db 01010000b  
videosegment  dw 0       
videoOffset dw 0          
counter_blanks db 0         
nextpair dw offset pastebuf 
lastpair dw offset pastebuf 

flag_pastebuffer db TRUE   
flag_double  db FALSE       

unix_pairs db 9       
unix_ranges db 35,35  
            db 45,58   
            db 64,90   
            db 92,92   
            db 95,95  
            db 97,122  
            db 126,126 
            db 128,165  
            db 224,238 
            db 0,0     
            db 0,0    
            db 0,0     
;            db 0,0     
;            db 0,0     
MAX_unix_pairs equ  ($-unix_pairs)/2
SPEED_LDC equ 9       

marginleft dw 0       
marginright dw 0      
dolimits db TRUE        

if ENABLEFILEAPPEND
stringf db 0,0                 
filen   db 'c:\tmp\'
        db 'xpcm####.tmp',0   
endif 
endif 

if ENABLECURSOR         
flag_arrowkeys   db TRUE     
flag_editor  db 1          
current_X        dw 0        
current_Y        dw 0       
FlagButton13   equ 00001000B 
FlagButton3    equ 00000100B
FlagButton2    equ 00000010B
FlagButton1    equ 00000001B
FlagButtonCR   equ 00010000B 
FlagButtonsOFF equ 00010000B 
flag_doButtons  db 00000000B 
MAX_of_A equ 3              
SPEED_LC equ 3             
col_present  db 0         
row_present  db 0          
col_target   db 0        
row_target   db 0           
Button1_ESC db 1bh, 01h 
            db '[',1ah 
            db 'M',32h 
            db 20h,39h 
Button1_COL db 00h,00h 
            db 00h,00h 
Button1_LEN equ 6      
Button2_ESC db 1bh, 01h
            db '[',1ah 
            db 'M',32h 
            db 21h,02h 
Button2_COL db 00h,00h
            db 00h,00h 
Button2_LEN equ 6     
Button3_ESC db 1bh, 01h 
            db '[',1ah 
            db 'M',32h  
            db 22h,28h 
Button3_COL db 00h,00h  
            db 00h,00h 
Button3_LEN equ 6      
CR_ESC      db 0dh,1ch 
CR_LEN      equ 1       
Xterm_PAD equ ' '+1   
endif  
MouseHandler proc far       
      XPUSHA              
      push ds             
      cmp counter_int10,0   
        jne short @@exit1   

@@not_in10:

if ENABLECURSOR        
      mov current_X,cx      
      mov current_Y,dx       
endif   ;end ENABLECURSOR 

      cmp flag_swapbuttons,TRUE 
       jnz @@swap_done
      call NewButtonOrder   
@@swap_done:
      push ax
      mov ds,biosSEG         
      mov al,byte ptr [ds:49h]
      cmp al,3
        jna short @@test_events
      cmp al,7  ; BIOS current video mode
        jz  short @@test_events
      cmp al,13h ; Other video modes are graphics
        jna short @@exit0
     cmp word ptr [ds:4ch],0800h 
        jbe short @@test_events 
@@exit0:
      pop ax
      jmp short @@exit1
@@test_events:
      pop ax
@@tst_moved:
      test ax,M_MOVED         
        jz short @@tst_released
      jmp @@mouse_moved
@@tst_released:
      test ax,M_LT_RELEASED    
        jnz short @@left_released
@@tst_right:
      test ax,M_RT_PRESSED      
        jnz short @@test_button2_shifts
@@tst_left:
      test ax,M_LT_PRESSED      
        jz short @@tst_button3
        jmp  @@test_button1_shifts
@@tst_button3:

if ENABLECURSOR       
      test ax,M_MI_PRESSED     
        jz short @@tst_exit
        jmp @@exit_button3
@@tst_exit:
endif   ;end ENABLECURSOR

@@exit1:
      jmp @@exit
@@left_released:
      cmp flag_Left,FALSE       
        je short @@skip_cursor
if ENABLEPASTE
      cmp flag_double,TRUE
       je @@copy_region1
      mov ax,NextPositionLeft
      cmp ax,PositionLeft
        jz  short @@no_selection 
@@copy_region1:
      mov flag_double,FALSE     
      mov flag_Highlight,TRUE   
      mov flag_Left,FALSE
      call Fill_pastebuffer      
       jnc short @@skip_cursor  
@@copy_region2:
if ENABLECURSOR        
      or flag_doButtons,FlagButton13 
endif   ;end ENABLECURSOR 
      jmp short @@skip_cursor
endif ;ENABLEPASTE

@@no_selection:

if ENABLECURSOR        
      call ClickTimeCursor      
endif   ;end ENABLECURSOR 

@@skip_cursor:
      mov flag_Left,FALSE
      call HIDE_MOUSE       
@@exit2:
      jmp short @@exit1

@@test_button2_shifts:
      mov al,byte ptr ds:[17h]
      and al,00001111B
        jnz short @@rt_shifts
      cmp flag_Left,TRUE
        jz short @@exit2    
if ENABLECURSOR
      or flag_doButtons,FlagButton2  
endif ;ENABLECURSOR
@@test_paste:
if ENABLEPASTE
      test flag_event,FlagPASTE  
       jnz short @@test_paste_exit 
      mov flag_Right,TRUE     
      mov nextpair,offset pastebuf
      or flag_event,FlagPASTE  
@@test_paste_exit:
endif ;ENABLEPASTE
      jmp short @@exit2
@@rt_shifts:                  
if ENABLEPASTE
      cmp al,00000100B         
        jne short @@test_ctrl_exit
      mov  flag_CtrlRight,TRUE  
      jmp short @@test_paste
@@test_ctrl_exit:
      cmp al,00000001B                
        jne short @@endtest_Rshift
        not flag_pastebuffer          
        jmp short @@exit2
@@endtest_Rshift:
if ENABLEFILEAPPEND
      cmp al,00000110B                  
        jne short @@endtest_LshiftCtrl
        or flag_event,FlagFILEAPPEND
@@endtest_LshiftCtrl:
      cmp al,00001010B             
        jne short @@endtest_LshiftAlt
        or flag_event,FlagWINCLIP       
@@endtest_LshiftAlt:
endif ;ENABLEFILEAPPEND
endif ;ENABLEPASTE

if ENABLECURSOR        
      cmp al,00001000B          
       jne short @@exit3
@@exit_button3:
      or flag_doButtons,FlagButton3+FlagButtonCR  
endif   ;end ENABLECURSOR 
      jmp short @@exit3

@@mouse_moved:
      call SHOW_MOUSE           
if ENABLEPASTE
      cmp flag_Left,FALSE
        je short @@mouse_moved_exit 
      call highlight_text     
@@mouse_moved_exit:
endif ;ENABLEPASTE            
@@exit3:
      jmp @@exit
@@test_button1_shifts:      

if ENABLECURSOR        
      mov al,byte ptr [ds:17h]
      and al,00001111B          
        jz @@test_button1_exit 
      cmp al,00001010B         
        je short @@do_cursormethod
      cmp al,00000010B
        jne short @@leftshift_exit
        not flag_arrowkeys     
      jmp  short @@leftshift_exit
@@do_cursormethod:
      mov al,flag_editor
      cmp al,MAX_of_A           
        jb short @@do_cursormethod_exit
        xor al,al              
@@do_cursormethod_exit:
      inc al
      mov flag_editor,al
@@leftshift_exit:
      jmp short @@exit         
@@test_button1_exit:
endif   ;end ENABLECURSOR

@@left_pressed:                
      mov ax,[ds:4ah]
      mov lineSize,ax        
if ENABLEPASTE
      mov ax,word ptr ds:[4eh]
      mov videoOffset,ax       
      cmp word ptr ds:[63h],3B4h       
      mov videosegment,0b000h 
        jz short @@left_pressed1
      mov videosegment,0b800h
@@left_pressed1:
endif ;ENABLEPASTE
      mov flag_Left,TRUE     
if ENABLECURSOR
      or flag_doButtons,FlagButton1
endif ;ENABLECURSOR
if ENABLEPASTE
      call unhighlight_text    
      mov flag_Right,FALSE
      mov counter_blanks,0
      call xy2offs            
      mov PositionLeft,ax      
      mov NextPositionLeft,ax  
      call setLimits           

      mov flag_double,FALSE  
      call ClickTimeDouble   
        jz short @@exit      
      mov flag_double,TRUE
      call highlight_word
if ENABLECURSOR
      and flag_doButtons,(not FlagButton1)
endif 
endif 
@@exit:
if ENABLECURSOR
      cmp flag_doButtons,0
        jz @@@exit
        or flag_event,FlagCursor
@@@exit:
endif ;ENABLECURSOR
      pop ds
      XPOPA
      ret
MouseHandler endp

SHOW_MOUSE proc
    push ax
    cmp flag_MouseOn,TRUE
     jz  short @@is_on
    mov ax,1                
    int 33h
    mov flag_MouseOn,TRUE
@@is_on:
    pop ax
    ret
SHOW_MOUSE endp

HIDE_MOUSE proc
    push ax
    cmp flag_MouseOn,FALSE
     jz short @@no_hide
    mov ax,2
    int 33h
    mov flag_MouseOn,FALSE
@@no_hide:
    pop ax
    ret
HIDE_MOUSE endp

NewButtonOrder proc
     push cx
     push bx
     mov bl,al          
     xor bh,bh        
     shl bl,1          
     mov cl,button_1fix 
     call BitBuster     
     mov cl,button_2fix
     call BitBuster    
     mov cl,button_3fix
     call BitBuster    
     and al,10000001B   
     or al,bh           
     pop bx
     pop cx
     ret
NewButtonOrder endp


BitBuster proc
     shr bl,1           
     shr bl,1
     mov ch,bl
     and ch,00000011B   
     shl ch,cl         
     or bh,ch       
     ret
BitBuster endp

killed_flg db FALSE

InstallMouseHandler proc far
      XPUSHA
      cmp killed_flg,TRUE    
       je short @@exit      
      push es
       push cs
       pop es
       mov dx,offset MouseHandler
       mov cx,XPCMOUSEMASK
       mov ax,14h             
       int 33h
       and flag_event,(not FlagREINSTALL)
      pop es
@@exit:
      XPOPA
      ret
InstallMouseHandler endp

new_21h proc far
      pushf
      cmp flag_reinstall_exit,FALSE
       je short @@test_exec    
      cmp ah,4ch              
       je short @@reinstall
      cmp ah,00h               
       je short @@reinstall
@@test_exec:
      cmp flag_reinstall_exec,FALSE
       je short @@do_old_21h   

@@test_exec_x:
      cmp ax,4b00h             
       jne short @@do_old_21h  
@@reinstall:
      or flag_event,FlagREINSTALL

@@do_old_21h:
      popf
      db 0eah           
old_21h dw 0,0
new_21h endp

old_10h dw 0,0

new_10h proc far
      pushf                   
      call dword ptr cs:[old_10h]
      pushf
      cmp counter_int10,0
       jne @@exit
      inc counter_int10    
      call HIDE_MOUSE       
if ENABLEPASTE
      cmp flag_Highlight,FALSE
        je short @@no_unselect
      call unhighlight_text    
      mov flag_Left,FALSE
@@no_unselect:
endif ;ENABLEPASTE
      dec counter_int10         
@@exit:
      popf
      iret
new_10h endp

biosdata segment at 40h
org 1ah
headptr dw (?)  
tailptr dw (?)  
org 80h
bufstrt dw (?)  
bufend  dw (?)  
biosdata ends

stuffkeyCX proc
   push ds
   push bx
    cli
    mov ax,biosdata
    mov ds,ax
    mov ax,ds:tailptr
    inc ax
    inc ax            
    xor bl,bl
    inc bl          
    cmp ax,ds:headptr 
    jz  short @@full 
    cmp ax,ds:bufend  
    jnz short @@no_wrap
    mov ax,ds:bufstrt 
@@no_wrap:            
    cmp ax,ds:headptr 
    jz  short @@full  
@@go_ahead:
    mov bx,ds:tailptr 
    mov [ds:bx],cx    
    mov ds:tailptr,ax
    xor bl,bl
@@full:
    mov al,bl
   pop bx
   pop ds
   or  al,al           
   ret                   
stuffkeyCX endp

count_08h db 0
count_waitkey db 0
new_08h proc far
      pushf
      call dword ptr cs:[old_08h]
      pushf
      cmp count_08h,0
       jne @@exit1
      inc count_08h
      push ax
      cmp flag_event,0
       jz @@getshiftstatus
       call HouseKeeping        
@@getshiftstatus:
      push ds
       mov ds,biosSEG
       mov al,byte ptr ds:[17h]
      pop ds
      test al,00000001B        
      jz   @@keyloop_exit
      mov ah,00001111B
      and ah,al
      cmp ah,00000111B        
      jne @@wait_exit
      inc count_waitkey
      cmp count_waitkey,20
       jb @@keyloop_exit
       or flag_event,FlagREINSTALL
@@wait_exit:
      mov count_waitkey,0
@@keyloop_exit:
      pop ax

@@exit:
      dec count_08h
@@exit1:
      popf
      iret
old_08h dw 0,0
new_08h endp

HouseKeeping proc near
      mov al,flag_event
      test al,FlagPASTE+FlagCURSOR
       jz @@stuff2key_exit
       call stuff2keybuffer
       ret
@@stuff2key_exit:
      test al,FlagREINSTALL   
       jz @@install_exit
       call InstallMouseHandler 
       ret
@@install_exit:
if ENABLEFILEAPPEND
      test al,FlagFILEAPPEND 
       jz @@fileappend_exit
       call appendPaste2File   
       ret
@@fileappend_exit:
      test al,FlagWINCLIP    
       jz @@winclip_exit
       call appendPaste2Windows  
@@winclip_exit:
endif ;ENABLEFILEAPPEND
      ret
HouseKeeping endp

MOUSE_FN      equ 8080h 
                        
MOUSE_FNDEC   equ 7f80h
IDENTCODE     equ 16879 
                      

new_16h         proc    far
      cmp  ax,MOUSE_FN       
        jnz  short @@skip1
      dec  ah             
      mov  cx,IDENTCODE     
      push cs             
      pop  es              
      mov  bx,offset pastebuf
      iret
@@skip1:
      pushf
      call stuff2keybuffer
      popf
      db 0eah           
old_16h dw 0,0
new_16h endp

stuff2keybuffer proc             
      cmp flag_busy_stuffing,1 
        je @@exitfast          
      mov flag_busy_stuffing,1 
      XPUSHA

if ENABLEPASTE
      xor cx,cx
      xchg cl,lastChar       
      or cl,cl             
        jnz short @@after_rd 
@@loop:
      call GetPastebuffer2AL
      mov cl,al             
        jc short @@@exit    
@@after_rd:
      push cx
      xor ch,ch              
      cmp cl,13            
       jne @@scan_code      
      mov ch,1ch            
@@scan_code:
      call stuffkeyCX        
      pop cx
        jz  short @@loop   
      mov lastChar,cl       
      jmp short @@exit

@@@exit:                     
      mov lastChar,0
      and flag_event,(not FlagPASTE)  
endif ;ENABLEPASTE

if ENABLECURSOR        
      call StuffArrows      
endif   ;end ENABLECURSOR 

@@exit:
      XPOPA
      mov flag_busy_stuffing,0  
@@exitfast:                
      ret
stuff2keybuffer endp


if ENABLEPASTE
xy2offs proc
      PUSHR <dx,cx>
      shr cx,1
      shr cx,1
      shr cx,1
      cmp lineSize,40
      jnz short @@no_40
      shr cx,1         
    @@no_40:
      shr dx,1
      shr dx,1
      shr dx,1
      mov ax,dx
      mul lineSize
      add ax,cx        
      add ax,ax      
      add ax,videoOffset
;      inc ax          
      POPR <cx,dx>
      ret
xy2offs endp

xor_screen proc
      PUSHR <bx,cx,dx,ds>
      call HIDE_MOUSE        
      mov ds,videosegment
      mov dolimits,TRUE
      mov cx,PositionLeft
      mov bx,NextPositionLeft
      cmp cx,bx
      je @@exit
      jb @@doswap
        mov ax,cx
        call findColumn
        cmp dx,marginleft
        je short @@dolimits
        jmp short @@normal   
@@doswap:
        xchg cx,bx            
@@dolimits:
        mov ax,lineSize       
        dec ax
        add ax,ax
        mov marginright,ax
        mov marginleft,0
        mov dolimits,FALSE
@@normal:
      mov dl,xor_mask
@@loop:
        mov ax,bx
        inc bx
        call column_test
         jnz @@failed
        xor byte ptr [ds:bx],dl 
@@failed:
      inc bx
      cmp bx,cx
      jbe short @@loop
@@exit:
;      call SHOW_MOUSE
      POPR <ds,dx,cx,bx>    
    ret
xor_screen endp

column_test proc near
        push ax
        push dx
         call findColumn      
        cmp marginleft,dx
          ja @@failed
        cmp dx,marginright
          ja @@failed
          je @@failedright
        xor ax,ax           
@@exit:
        pop dx
        pop ax
         ret
@@failedright:
        xor ax,ax          
        stc                   
        jmp short @@exit
@@failed:
        xor ax,ax
        inc ax                 
        jmp short @@exit
column_test endp
findColumn proc near
         push ax
         push cx
          xor dx,dx
          mov cx,lineSize
          add cx,cx
          div cx                
         pop cx
         pop ax
         ret
findColumn endp

setLimits proc near
      cmp dolimits,FALSE
        je @@quit
      push dx
      push cx
      push ax

      mov ax,NextPositionLeft
      call findColumn
      mov cx,dx               

      mov ax,PositionLeft
      call findColumn           

      cmp dx,cx              
       jb @@next
      xchg cx,dx             
@@next:
       mov marginleft,dx
       mov marginright,cx
       pop ax
       pop cx
       pop dx
@@quit:
       ret
setLimits endp

highlight_word proc
      PUSHR <bx,ds>
      xor bx,bx
      mov lasttime,bx
      mov lasttime+2,bx       
      mov ds,videosegment
      mov bx,PositionLeft  
      call @@tst_letter
      jc @@exit               
  @@go_left:                  
      call @@tst_letter
      dec bx
      dec bx
      jnc  @@go_left
      add bx,4                 
      mov  PositionLeft,bx
  @@go_right:               
      inc bx
      inc bx
      call @@tst_letter
      jnc @@go_right
      dec bx
      dec bx
      mov NextPositionLeft,bx
      call setLimits
      call xor_screen          
  @@exit:
      POPR <ds,bx>
      ret

  @@tst_letter:
      push si
      mov si,offset unix_ranges
      xor ah,ah
      mov al,[ds:bx]
  @@tst_loop:
      cmp ah,unix_pairs
      je @@tst_failed
      inc ah
      cmp byte ptr [cs:si],al
      jbe short @@tst_1
      inc si
      inc si
      jmp short @@tst_loop
  @@tst_1:
      inc si
      cmp al,byte ptr [cs:si]
      jbe short @@tst_worked
      inc si
      jmp short @@tst_loop
  @@tst_worked:
      clc
      jmp short @@tst_exit
  @@tst_failed:
      stc
  @@tst_exit:
      pop si
      ret
highlight_word endp
highlight_text proc
      push bx
      call xy2offs            
      push ax              
      sub ax,NextPositionLeft 
      pop bx
      jz short @@exit       
      call unhighlight_text  
      mov NextPositionLeft,bx 
      call setLimits          
      call xor_screen        
      mov flag_Highlight,TRUE
    @@exit:
      pop bx
      ret
highlight_text endp

unhighlight_text proc
    push ax
      cmp flag_Highlight,FALSE
        je short @@exit     
      call xor_screen       
      mov flag_Highlight,FALSE
   @@exit:
   pop ax
   ret
unhighlight_text endp

Fill_pastebuffer proc
      XPUSHA
      PUSHR <es,ds>
      cmp flag_pastebuffer,FALSE 
        stc
        je short @@quit      
      push cs                 
      pop es                   
      mov bx,PositionLeft
      mov cx,NextPositionLeft
      cmp cx,bx
        ja  short @@set_loop 
      xchg bx,cx           
        jnz  short @@set_loop 
@@exit_nochars:            
      clc
      jmp short @@@exit

@@set_loop:
      mov di,offset pastebuf 
      mov nextpair,di       
      cld
      mov ds,videosegment
      dec bx
      dec bx                 
      xor dl,dl       
@@loop:
      cmp di,EndOfBuffer
        ja short @@OutofRoom 
      or dl,dl
        jnz short @@trim
      cmp bx,cx
        jz short @@OutofData
      inc bx
      inc bx                
       mov ax,bx
      call column_test 
        jc short @@fixEOL       
        jnz short @@loop       
@@regularData:
      mov al,[ds:bx]
      cmp al,' '               
        jb short @@loop
@@SaveCodedByte:
      stosb                
      jmp short @@loop
@@fixEOL:
      inc dl                   
      jmp short @@regularData  
@@trim:                        
      call @@trimFunction
      jmp short @@SaveCodedByte

@@OutofRoom:
@@OutofData:
      mov lastpair,di        
      dec di
      cmp byte ptr [es:di],10
       jne short @@quit
      dec di
      mov lastpair,di        
@@quit:
      stc                

@@@exit:
      POPR <ds,es>                 
      XPOPA
      ret
@@trimFunction:
      cmp dl,2                  
       jne @@trimLoop
       xor dl,dl              
       mov al,10               
       ret
@@trimLoop:
      dec di
      cmp byte ptr [es:di],' '  
        ja  short @@trimDone
      cmp byte ptr [es:di],10  
        jne short @@trimLoop
@@trimDone:
      inc di
      mov al,13                
      inc dl                    
      ret
Fill_pastebuffer endp

ClickTimeDouble proc
      PUSHR <cx,dx>
      xor ah,ah
      int 1ah               
      xor ax,ax         
      push cx             
      push dx               
      sub  dx,lasttime
      sbb  cx,lasttime+2    
      pop  cs:lasttime
      pop  cs:lasttime+2    
      or cx,cx
        jnz short @@exit   
      cmp dx,SPEED_LDC
        ja short @@exit      
      inc ax               
@@exit:
     or ax,ax
     POPR <dx,cx>
     ret
ClickTimeDouble endp

GetPastebuffer2AL proc
      push es
      cmp flag_pastebuffer,FALSE       
        je short @@buf_empty
      cmp flag_Right,FALSE        
        je  short @@buf_empty
      push cs
      pop es
@@loop:
      call GetPasteBuf                 
       jc @@buf_empty                  
      cmp al,10                  
       je short @@loop
      clc
      jmp short @@exit
@@buf_empty:
      stc
@@exit:
      pop es
      ret
GetPastebuffer2AL endp

GetPasteBuf proc
      push bx
      mov bx,nextpair
      cmp bx,lastpair
        jnz short @@not_empty
      cmp flag_CtrlRight,TRUE     
        jne short @@buf_empty
      mov flag_CtrlRight,FALSE  
      mov al,13                  
      jmp short @@char_found

@@not_empty:
      mov al,[es:bx]              
      inc nextpair
@@char_found:
      clc
@@exit:
      pop bx
      ret
@@buf_empty:
      stc
      jmp short @@exit
GetPasteBuf endp

if ENABLEFILEAPPEND
appendPaste2File proc
      PUSHR <bx,cx,dx,ds,es>
      cmp cs:counter_int13,0
       jnz @@done             
      mov ax,cs
      mov ds,ax
      mov es,ax
      mov nextpair,offset pastebuf
      mov flag_CtrlRight,FALSE         
      mov counter_blanks,0
        mov     ax,3d02h             
        mov     dx,offset filen         
        int     21h
        jnc     @@goodopen
        xor     cx,cx                
        mov     ah,03ch              
        int     21h
        jnc     @@goodopen           
@@bad_open:
          call bell                   
          jmp short @@finished       
@@goodopen:
        mov     bx,ax                 
@@seek:                              
        xor     cx,cx                 
        mov     dx,cx             
        mov     ax,4202h             
        int     21h
@@loop:
      call GetPasteBuf                 
        jc short @@@exit            
      call @@writeByte
      jmp short @@loop
@@@exit:
      mov al,13
      call @@writeByte
      mov al,10
      call @@writeByte
        mov     ah,3eh
        int     21h                     
@@finished:
        and flag_event,(not FlagFILEAPPEND)
@@done:
        POPR <es,ds,dx,cx,bx>              
        ret
@@writeByte:
        mov     byte ptr cs:stringf,al
        mov     cx,1
        mov     dx,offset stringf       
        mov     ah,40h
        int     21h                    
        ret
appendPaste2File endp

count_winclip db 0
appendPaste2Windows proc
      XPUSHA
      push es
      mov     ax,1600h               
      int     2fh
      or      al,al
      je      @@finished            

        mov     di,1
        mov     ax,1700h              
        int     2fh
        or      ax,ax                 
        je      @@exit_error
        mov     ax,1701h               
        int     2fh
        or      ax,ax
        je      @@exit_error
        mov     ax,1702h               
        int     2fh
        or      ax,ax
        je      @@closeClip            
        xor     di,di
        push    cs
        pop     es
        mov     bx,offset pastebuf    
        mov     cx,lastpair
        sub     cx,bx                 
        mov     ax,1703h               
        xor     si,si                  
        mov     dx,7              
        int     2fh
@@closeClip:
        mov     ax,1708h              
        int     2fh
        or      di,di
         jnz    @@exit_error
@@finished:
        mov     count_winclip,0         
        and flag_event,(not FlagWINCLIP) 
@@done:
        pop es
        XPOPA
        ret
@@exit_error:
        inc count_winclip
        cmp count_winclip,18
         jb short @@done            
        call bell                     
        jmp short @@finished          
appendPaste2Windows endp


bell    proc near                       
        mov ax,0E07h
        int 10h
        ret
bell    endp


; Patch for Interrupt 13h, disk

old_13h dw 0,0
counter_int13 db 0

new_13h proc far
      inc cs:counter_int13
      pushf                     
      call dword ptr cs:[old_13h]
      pushf
      dec cs:counter_int13
      popf
      ret     2           
new_13h endp

endif 

endif 

if ENABLECURSOR         

ClickTimeCursor proc
      PUSHR <cx,dx>
      xor ah,ah
      int 1ah                
if ENABLEPASTE
      sub  dx,lasttime
      sbb  cx,lasttime+2     
      or cx,cx
        jnz short @@setcursor
      cmp dx,SPEED_LC
        jb short @@exit     
endif ;ENABLEPASTE
@@setcursor:
        call set_cursor    
@@exit:
     POPR <dx,cx>
     ret
ClickTimeCursor endp

LeftArrow proc
      mov cx,4b00h    
      call stuffkeyCX   
      ret
LeftArrow endp
DownArrow proc
      mov cx,5000h      
      call stuffkeyCX   
      ret
DownArrow endp
UpArrow proc
      mov cx,4800h     
      call stuffkeyCX   
      ret
UpArrow endp
RightArrow proc
      mov cx,4d00h     
      call stuffkeyCX   
      ret
RightArrow endp
Check4Room proc
   PUSHR <ds,bx,ax>
    mov bl,al          
    mov ax,biosdata
    mov ds,ax
    mov ax,ds:tailptr
@@loop:
    add ax,2
    cmp ax,ds:bufend  
      jnz short @@no_wrap
    mov ax,ds:bufstrt 
@@no_wrap:           
    cmp ax,ds:headptr 
    jz  short @@full  
@@go_ahead:
    dec bl
    or bl,bl
    jnz short @@loop
@@full:
   or  bl,bl             
   POPR <ax,bx,ds>
   ret
Check4Room endp
StuffCodes2keybuffer proc
      push cx
      cmp flag_arrowkeys,FALSE  
        je short @@exit        
      call Check4Room           
        jnz short @@exit      
@@loop:
      cmp al,0
      je @@exit
      dec al
      mov cl,byte ptr [cs:bx]     
      inc bx
      mov ch,byte ptr [cs:bx]    
      inc bx
      push ax
      call stuffkeyCX             
      pop ax
      jmp short @@loop
@@exit:
      pop cx
      ret
StuffCodes2keybuffer endp

StuffLoc2buffer proc
      add ah,Xterm_PAD          
      mov byte ptr [cs:bx],ah
      inc bx
      inc bx
      add al,Xterm_PAD       
      mov byte ptr [cs:bx],al
      ret
StuffLoc2buffer endp

StuffButton1 proc
      push bx
      mov bx,offset Button1_COL 
      call StuffLoc2buffer      
      mov bx,offset Button1_ESC 
      mov al,Button1_LEN        
      call StuffCodes2keybuffer 
      pop bx
      ret
StuffButton1 endp

StuffButton3 proc
      push bx
      mov bx,offset Button3_COL 
      call StuffLoc2buffer     
      mov bx,offset Button3_ESC
      mov al,Button3_LEN       
      call StuffCodes2keybuffer 
      pop bx
      ret
StuffButton3 endp

StuffCR proc
      push bx
      mov bx,offset CR_ESC 
      mov al,CR_LEN       
      call StuffCodes2keybuffer 
      pop bx
      ret
StuffCR endp
StuffButton2 proc
      push bx
      mov bx,offset Button2_COL 
      call StuffLoc2buffer      
      mov bx,offset Button2_ESC 
      mov al,Button2_LEN       
      call StuffCodes2keybuffer 
      pop bx
      ret
StuffButton2 endp
GetMousePosition proc
      PUSHR <dx,cx>
      mov cx,current_X
      shr cx,1          
      shr cx,1
      shr cx,1
      cmp lineSize,40
      jnz short @@not_40
      shr cx,1      
    @@not_40:
      mov dx,current_Y
      shr dx,1         
      shr dx,1
      shr dx,1
      mov al,dl
      mov ah,cl
      POPR <cx,dx>
      ret
GetMousePosition endp
offs2xy proc
      push bx
      shr ax,1               
      mov bx,lineSize       
      idiv bl               
      pop bx
      ret
offs2xy endp
DoXtermButtons proc
      and flag_doButtons,(not FlagButtonCR)     
@@top:
      mov al,flag_doButtons
      test al,FlagButton1
        jz short @@Button1_exit
@@do_xterm3:
      mov ah,col_target
      mov al,row_target
      cmp ah,col_present
        jne short @@do_xterma
      cmp al,row_present
        je short @@do_xterm_exit
@@do_xterma:                    
      mov col_present,ah
      mov row_present,al        
      call StuffButton1        
@@do_xterm_exit:
      and flag_doButtons,(not FlagButton1)
      jmp short @@top
@@Button1_exit:
      test al,FlagButton13
        jz short @@Button13_exit
      push cx
      mov ax,PositionLeft
      mov cx,NextPositionLeft
      cmp ax,cx
        jc  short @@no_swap  
      xchg ax,cx            
@@no_swap:
      push cx
      call offs2xy           
      call StuffButton1     
      pop ax                
      call offs2xy         
      call StuffButton3      
      pop cx
      and flag_doButtons,(not FlagButton13)
      jmp short @@top
@@Button13_exit:
      test al,FlagButton2
        jz short @@Button2_exit
      call GetMousePosition  ;al=row, ah=col
      call StuffButton2
      and flag_doButtons,(not FlagButton2)
      jmp short @@top
@@Button2_exit:
      test al,FlagButton3
        jz short @@exit
      call GetMousePosition  ;al=row, ah=col
      call StuffButton3
      and flag_doButtons,(not FlagButton3)
@@exit:
      ret
DoXtermButtons endp

StuffArrows proc
@@arrows:
      cmp flag_arrowkeys,FALSE      
        je short @@test_editor_exit 
@@test_editor:
      mov al,flag_editor
      cmp al,1        
        je short @@do_emacs_vi
      cmp al,2       
        je short @@do_matrix  
      cmp al,3      
        jne short @@kill_allflag
      call DoXtermButtons
@@test_editor_exit:
      jmp short @@kill_eventflag
@@do_emacs_vi:
      mov al,row_present
      cmp row_target,al
      je short @@downarrow      
@@arrow_loop:
      cmp col_present,0
      je short  @@downarrow   
      call LeftArrow
        jnz short @@exit        
      dec col_present         
      jmp short @@arrow_loop
@@do_matrix:
@@downarrow:
      mov al,row_present
      cmp row_target,al
      je short @@rightarrow     
      jl short @@uparrow        
      call DownArrow
        jnz short @@exit        
      inc row_present        
      jmp short @@downarrow
@@uparrow:                     
      call UpArrow
        jnz short @@exit     
      dec row_present          
      jmp short @@downarrow
@@rightarrow:
      mov al,col_present
      cmp col_target,al
      je short @@kill_allflag  
      jl short @@leftarrow     
      call RightArrow
        jnz short @@exit      
      inc col_present        
      jmp short @@rightarrow
@@leftarrow:
      call LeftArrow
        jnz short @@exit       
      dec col_present         
      jmp short @@rightarrow
@@kill_allflag:
      and flag_doButtons,FlagButtonsOFF 
@@kill_eventflag:
      and flag_event,(not FlagCURSOR) 
@@exit:
      test flag_doButtons,FlagButtonCR
        jz short @@@exit
      call StuffCR            
        jnz short @@@exit      
      and flag_doButtons,(not FlagButtonCR)
@@@exit:
      ret
StuffArrows endp
set_cursor proc
      PUSHR <bx,ds>
      call GetMousePosition  
      mov col_target,ah
      mov row_target,al
      mov ds,biosSEG
      xor ah,ah
      mov al,byte ptr [ds:62h]   
      add al,al         
      add al,50h       
      mov bx,ax        
      mov ax,[ds:bx]  
      mov row_present,ah
      mov col_present,al
      xor bx,bx
      mov lasttime,bx
      mov lasttime+2,bx 
@@exit:
      POPR <ds,bx>     
      or flag_event,FlagCURSOR
      ret
set_cursor endp

endif   

guardbyte db 0  
pastebuf equ $

DE_INSTALL    equ 2     
REACTIVATE    equ 3    
if ENABLEPASTE
APPENDPASTE   equ 4     
SHOWXTCLASS   equ 5    
SETXTCLASS    equ 6   
SETTMP        equ 7     
endif ;ENABLEPASTE

NOT_INSTALLED equ 40h   
ALL_OK        equ 41h  
WRO_VEC       equ 42h  

 TESTCPU macro
if not XT
      mov cl,33       
      shl ax,cl
      or  cl,cl       
      jnz @@TESTCPU_exit
      mov dx,offset wro_cpu_str
      call messageAlways
      jmp @@errexit
@@TESTCPU_exit:
endif 
 endm

 TESTBIOS macro
if not XT
      mov es,biosSEG
      test byte ptr [es:96h],10000b 
      jz @@errexit
endif ; not XT
 endm

quiet_flag db FALSE          
copying_flag db FALSE      
counter_Xoption db 0       
how_to_flag db FALSE        
ExecExit_flag db FALSE      
settmpname_flag db FALSE    

get_opt proc
        mov si,80h            
        seges
        lodsb         
        mov bh,0
        mov bl,al
        mov es:[si+bx],bh       
        or  al,al
        jnz  @@findslash
@@@parm_done:
        jmp  @@parm_done        
@@findslash:
        seges
        lodsb               
        or al,al
        jz  @@@parm_done        
        cmp al," "
        jz short @@findslash
        cmp al,"/"
        jz  short @@good_sep
        cmp al,"-"
        jz short @@good_sep
@@@parm_err:
        jmp @@parm_err
@@good_sep:
@@loop:                       
        seges                
        lodsb
@@HELP_setup:
        cmp al,'?'            
          jz short @@@parm_err

        and al,not ('a'-'A')  

        cmp al,'H'           
          jnz @@HELP_setup_exit
          mov how_to_flag,TRUE
          jz short @@@parm_err
@@HELP_setup_exit:
@@U_setup:
        cmp al,'U'            
          jnz short @@U_setup_exit
        mov ah,DE_INSTALL
        jmp @@exit
@@U_setup_exit:
@@R_setup:
        cmp al,'R'           
          jnz short @@R_setup_exit
        mov ah,REACTIVATE
        jmp @@exit
@@R_setup_exit:
if ENABLEPASTE
@@D_setup:
        cmp al,'D'           
         jnz short @@D_setup_exit
        cmp     byte ptr es:[si],'='
          je @@D_setup1
        mov ah,SHOWXTCLASS
        call showXtermClasses
        jmp @@exit
@@D_setup1:
        inc si              
        mov di,offset fileappend
@@D_nameloop:
        mov al,es:[si]
        mov byte ptr cs:[di],al
        inc si
        inc di
        cmp al,0
        je @@D_nameloop_exit
        cmp al,' '
        jne @@D_nameloop
@@D_nameloop_exit:
        mov byte ptr cs:[di],0
        call setXtermClasses
        mov ah,SETXTCLASS
@@D_setup2:
        jmp @@findslash
@@D_setup_exit:
@@F_setup:
        cmp al,'F'           
         jnz short @@F_setup_exit
        mov ah,byte ptr es:[si]
        cmp ah,'='             
         je short @@F_setup_getname
        cmp ah,':'             
         jne short @@F_setup_exit
@@F_setup_getname:
        inc si              
        mov di,offset fileappend
        mov settmpname_flag,ah
        xor ah,ah             
@@nameloop:
        mov al,es:[si]
        mov byte ptr cs:[di],al
        cmp ah,128
        jb @@nameloop_go
         mov dx,offset MSGfilesize
         call messageAlways
         jmp @@parm_err
@@nameloop_go:
        cmp al,' '
        je @@nameloop_exit
        cmp al,0
        jz @@nameloop_exit
        inc si
        inc di
        inc ah
        jmp short @@nameloop
@@nameloop_exit:
         mov byte ptr cs:[di],0
         cmp settmpname_flag,':'
         mov settmpname_flag,TRUE       
         je @@F_setup_setname
        call appendfile
        mov ah,APPENDPASTE    
        jmp @@exit
@@F_setup_setname:
if ENABLEFILEAPPEND
        mov al,ah              
        call ChangeTmpName
        jc short @@parm_err
endif ;ENABLEFILEAPPEND
        jmp @@findslash
@@F_setup_exit:
endif ;ENABLEPASTE
@@N_setup:
        cmp al,'N'            
          jnz short @@N_setup_exit
        mov ExecExit_flag,TRUE
        call parse_num       
        mov flag_reinstall_exec,FALSE
        mov flag_reinstall_exit,FALSE
        test ax,1
          jz @@N_1
        mov flag_reinstall_exec,TRUE
@@N_1:
        test ax,2
          jz @@N_2
        mov flag_reinstall_exit,TRUE
@@N_2:
        jmp @@findslash
@@N_setup_exit:
@@S_setup:
        cmp al,'S'           
         jnz short @@S_setup_exit
        call parse_num      
        call setbuttonswap
        jmp @@findslash
@@S_setup_exit:
@@C_setup:
        cmp al,'C'            
        jne short @@C_setup_exit
        mov copying_flag,1
        jmp @@findslash
@@C_setup_exit:
@@Q_setup:
        cmp al,'Q'           
        jnz short @@Q_setup_exit
        mov cs:quiet_flag,TRUE
        jmp @@findslash
@@Q_setup_exit:
        jmp short @@parm_done_exit

@@parm_err:
        mov ah,FALSE
        jmp short @@exit
@@parm_done:
        mov ah,TRUE
        jmp short @@exit

@@parm_done_exit:

if ENABLEPASTE
@@P_setup:
        cmp al,'P'            
        jne short @@P_setup_exit
        mov flag_pastebuffer,FALSE
        jmp @@findslash
@@P_setup_exit:
@@M_setup:
        cmp al,'M'           
          jnz short @@M_setup_exit
        mov al,119            
        jmp short @@X_setup1
@@M_setup_exit:
@@X_setup:
        cmp al,'X'            
        jnz short @@X_setup_exit
        call parse_num      
        cmp ax,255            
        ja @@parm_err
        cmp ax,0
        jb short @@parm_err
@@X_setup1:
        mov xor_mask,al
        inc counter_Xoption
        jmp @@findslash
@@X_setup_exit:
@@B_setup:
        cmp al,'B'           
        jnz short @@B_setup1_exit
        call parse_num        
        cmp ax,BUFFERLEN    
        ja @@parm_err        
        add ax,15           
        mov cl,4
        shr ax,cl
        mov cl,4
        shl ax,cl             
        mov cs:BUFLEN,ax
        jmp @@findslash
@@B_setup1_exit:
endif ;ENABLEPASTE

if ENABLECURSOR       
@@K_setup:
        cmp al,'K'         
          jne short @@K_setup_exit
        mov flag_arrowkeys,FALSE
        jmp @@findslash
@@K_setup_exit:
@@A_setup:
        cmp al,'A'          
        jnz short @@A_setup_exit
        call parse_num        
        cmp ax,MAX_of_A     
        ja short @@A_setup1
        cmp ax,1
        jb short @@A_setup1
        mov flag_editor,al
@@A_setup1:
        jmp @@findslash
@@A_setup_exit:
endif   ;end ENABLECURSOR 
        jmp @@findslash
@@exit:
        ret
get_opt endp

SwapButtonTable dw  123   
                db 1,3,5    
                dw 132
                db 1,5,3
                dw 213     
                db 3,1,5   
                dw 231      
                db 3,5,1     
                dw 312     
                db 5,1,3     
                dw 321      
                db 5,3,1
                dw 0
                db 0,0,0
setbuttonswap proc
        push si
        push bx
        mov si,offset SwapButtonTable
@@S_loop:
        cmp ax,word ptr [si]
         jne @@S_loopexit
        inc si
        inc si
        mov bl,byte ptr [si]
        mov button_1fix,bl
        inc si
        mov bl,byte ptr [si]
        mov button_2fix,bl
        inc si
        mov bl,byte ptr [si]
        mov button_3fix,bl
        jmp short @@S_exit
@@S_loopexit:
        add si,5
        cmp word ptr [si],0
         jnz @@S_loop
        jmp short @@S_exit1
@@S_exit:
        mov flag_swapbuttons,TRUE
@@S_exit1:
        pop bx
        pop si
        ret
setbuttonswap endp


parse_num proc
        push    bx
        mov bx,10
        mov ax,0
        mov dh,0
@@addloop:
        seges
        mov dl,[si]
        or dl,dl
        jz  short @@done
        sub dl,'0'
        jc short @@done
        cmp dl,9
        ja short @@done
        inc si
        push dx
        mul bx
        pop dx
        add ax,dx
        jmp short @@addloop
@@done:
        pop     bx
        ret
parse_num endp

if ENABLEPASTE

stringone   db 0,0
notopened   db 'No se puede abrir el archivo','$'
filehandle  dw 0
pasteseg    dw 0
DisplayXtermClasses db 'Clases de terminales',13,10
fileappend  db 128 dup (?)
SIZEfileappend equ $-fileappend
            db '$'
hundredsplace   db      0
tensplace       db      0
onesplace       db      0

convertBase10 proc near
        mov     ah,0            ;byte data
        mov     bx,100
        push    ax
        cwd
        idiv    bx
        mov     hundredsplace,al
        mov     dx,100
        mul     dx
        mov     bx,ax
        pop     ax
        sub     ax,bx
        push    ax
        mov     bx,10
        cwd
        idiv    bx
        mov    tensplace,al
        mov     dx,10
        mul     dx
        mov     bx,ax
        pop     ax
        sub     ax,bx
        mov     onesplace,al
        ret
convertBase10 endp

byteToString10 proc near

        push     bx
        push     dx
        call    convertBase10
        mov     al,hundredsplace
        cmp     al,0
        jz      @@tens
        add     al,'0'          
        mov     byte ptr ds:[di],al
        inc di
@@tens:
        mov     al,tensplace
        cmp     al,0
        jnz     @@tens_ok
        cmp     hundredsplace,0
        jz      @@ones
@@tens_ok:
        add     al,'0'          
        mov     byte ptr ds:[di],al
        inc di
@@ones:
        mov     al,onesplace
        add     al,'0'        
        mov     byte ptr ds:[di],al
        inc di
        pop     dx
        pop     bx
        ret
byteToString10 endp

showXtermClasses proc near
      XPUSHA
      call inst_tst     
       mov dx,es
       mov pasteseg,dx 
       cmp al,NOT_INSTALLED
       jne @@getclasses
         push cs
         pop es
@@getclasses:
      mov bx,offset unix_pairs
      mov al,byte ptr es:[bx]
      mov unix_pairs,al

      mov di,offset fileappend         
      mov bx,offset unix_ranges     

@@loop:
      mov al,unix_pairs
      cmp al,0
      jz @@loop_exit
      dec unix_pairs
      mov al,byte ptr es:[bx]         
      inc bx
      call byteToString10
      mov byte ptr ds:[di],'-'
      inc di
      mov al,byte ptr es:[bx]          
      inc bx
      call byteToString10
      mov byte ptr ds:[di],':'
      inc di
      jmp short @@loop
@@loop_exit:
      mov byte ptr ds:[di],13
      inc di
      mov byte ptr ds:[di],10
      inc di
      mov byte ptr ds:[di],'$'
      mov dx,offset DisplayXtermClasses
      call messageAlways
@@done:
        XPOPA
        ret
showXtermClasses endp

get_num proc
        push    bx
        mov bx,10
        mov ax,0
        mov dh,0
@@addloop:
        mov dl,byte ptr cs:[si]
        or dl,dl
        jz  short @@done
        sub dl,'0'
        jc short @@done
        cmp dl,9
        ja short @@done
        inc si
        push dx
        mul bx
        pop dx
        add ax,dx
        jmp short @@addloop
@@done:
        pop     bx
        ret
get_num endp

setXtermClasses proc near
        XPUSHA
      call inst_tst     
       mov dx,es
       mov pasteseg,dx 
       cmp al,NOT_INSTALLED
      jne @@setclasses
      jmp @@exit
@@setclasses:
        mov     al,0           
        mov     ah,3dh        
        mov     dx,offset fileappend      
        int     21h
        mov     filehandle,ax
        jnc     @@goodopen
        mov dx,offset notopened
        call messageAlways
        jmp     short @@exit         
@@goodopen:
        mov     bx,filehandle
        mov     cx,SIZEfileappend
        mov     dx,offset fileappend    
        mov     ah,3Fh
        int     21h                    
        mov     bx,ax
        mov     ax,offset fileappend
        add     bx,ax                   
        mov     byte ptr ds:[bx],0    
        mov     ah,3eh
        mov     bx,filehandle
        int     21h                     
        mov     si,offset fileappend
        mov     unix_pairs,0
        mov     bx,offset unix_ranges
        mov     es,pasteseg           
@@parsestring:                         
        call    get_num              
        cmp     ax,0
        je      @@parsestring_exit
        mov     byte ptr es:[bx],al
        inc     bx
        cmp     byte ptr cs:[si],0
        je     @@parsestring_exit
        inc     si
        call    get_num                 
        cmp     ax,0
        je      @@parsestring_exit
        mov     byte ptr es:[bx],al
        inc     bx
        inc     unix_pairs
        mov     al,unix_pairs
        cmp     al,MAX_unix_pairs
        je      @@parsestring_exit
        cmp     byte ptr cs:[si],0    
        je      @@parsestring_exit     
        inc     si                     
        jmp short @@parsestring
@@parsestring_exit:
        mov     bx,offset unix_pairs
        mov     al,unix_pairs
        mov     byte ptr es:[bx],al
@@exit:
        XPOPA
        ret
setXtermClasses endp
endif ;ENABLEPASTE

if ENABLEPASTE
openfile proc near
        push    dx
        push    ds
        push    cs
        pop     ds
        mov     al,2           
        mov     ah,3dh     
        mov     dx,si        
        int     21h
        mov     bx,ax          
        mov     ax,1           
        jnc     @@goodopen
        mov     cx,0                 
        mov     dx,si                 
        mov     ah,03ch            
        int     21h
        mov     bx,ax                
        mov     ax,1
        jnc     @@goodopen
        mov     ax,0           
@@goodopen:
        pop     ds
        pop     dx
        ret
openfile endp

seekend proc near
        push    bx
        push    cx
        push    dx
        push    es
        mov     cx,0                    
        mov     dx,0                 
        mov     al,2                  
        mov     ah,42h              
        int     21h
        pop     es
        pop     dx
        pop     cx
        pop     bx
        ret
seekend endp

appendfile proc
        push ds
      call inst_tst     
       mov pasteseg,es  
       cmp al,NOT_INSTALLED
       je @@done
@@getlocals:
      push cs
      pop ds
      mov nextpair,offset pastebuf
      mov bx,offset lastpair
      mov ax,word ptr es:[bx]
      mov lastpair,ax

      mov flag_CtrlRight,TRUE     
      mov counter_blanks,0
      mov si,offset fileappend  
      call openfile           
        mov filehandle,bx       
        cmp ax,0                
      jne @@seek
      mov dx,offset notopened
      call messageAlways
      jmp short @@done
@@seek:
      mov bx,filehandle
      call seekend
      mov       es,pasteseg             
      mov       si,offset stringone     
@@loop:
      call GetPasteBuf                  
        jc short @@@exit             
@@@loop:
        mov     byte ptr ds:[si],al
        mov     bx,filehandle
        mov     cx,1
        mov     dx,offset stringone     
        mov     ah,40h
        int     21h                  
        cmp     byte ptr ds:[si],13        
        jne @@loop
        mov     al,10     
        jmp short @@@loop
@@@exit:
        cmp     byte ptr ds:[si],10      
        mov     al,13
        jne     @@@loop                
        mov     ah,3eh
        mov     bx,filehandle
        int     21h                
@@done:
        pop ds
        ret
appendfile endp


if ENABLEFILEAPPEND
MSGfilesize db '/F:X ERROR: El nombre X tiene mas de 20 caracteres.'
else
MSGfilesize db '/F=X ERROR.'
endif ;ENABLEFILEAPPEND
            db 13,10
            db '/F=X X es la cantidad de caracteres maximos (128).'
            db 13,10,'$'

if ENABLEFILEAPPEND
ChangeTmpName proc
        push es
        push ds
        push si
        push di
        cmp al,21
        jb @@change
          mov dx,offset MSGfilesize
          call messageAlways
          stc
          jmp short @@exit
@@change:
        call inst_tst     
         mov di,offset filen
         mov si,offset fileappend
         cmp al,NOT_INSTALLED
          push cs
          pop ds
         je @@loop
          push es
          pop ds
@@loop:                                
         mov al,byte ptr cs:[si]    
         mov byte ptr ds:[di],al       
         or al,al
         jz @@loopexit
         inc si
         inc di
         jmp short @@loop
@@loopexit:
        clc
@@exit:
      pop di
      pop si
      pop ds
      pop es
      ret
ChangeTmpName endp
endif ;ENABLEFILEAPPEND
endif ;ENABLEPASTE

inst_tst proc
        push cx
        push ax
        mov ax,MOUSE_FN        
        int 16h
        cmp ax,MOUSE_FNDEC     
         jnz short @@failed
        cmp cx,IDENTCODE       
         je @@worked
@@failed:
        mov al,NOT_INSTALLED
         jmp short @@exit
@@worked:
        mov al,ALL_OK           
@@exit:
        pop cx
        mov ah,ch
        pop cx
        ret
inst_tst endp

cmp_fptr proc 
        XPUSHA
        mov si,0
        mov di,0

        rept 4
          shl ax,1
          rcl si,1
        endm
        add ax,bx
        adc si,0

        rept 4
          shl cx,1
          rcl di,1
        endm
        add cx,dx
        adc di,0

        cmp cx,ax
        jnz short @@exit
        cmp si,di
@@exit:
        XPOPA
        ret
cmp_fptr endp

      ms_hnd_ptr dw 0,0

re_activate proc
LOCAL  hndadr:DWORD = AUTO_SIZE
       push bp
       mov  bp,sp
       sub  sp,AUTO_SIZE
      PUSHR <ds,es>
      call inst_tst         
      cmp al,NOT_INSTALLED
      je short @@exit

      push bx                  
      push es
      sub bx,pastebuf-InstallMouseHandler
      mov word ptr hndadr,bx
      mov bx,es
      mov word ptr hndadr+2,bx
      call dword ptr [hndadr]       
      pop es
      pop bx
      cmp ExecExit_flag,FALSE         
       je @@exit
      push bx
      sub bx,pastebuf-flag_reinstall_exec
      mov al,flag_reinstall_exec
      mov byte ptr es:[bx],al
      pop bx
      sub bx,pastebuf-flag_reinstall_exit
      mov al,flag_reinstall_exit
      mov byte ptr es:[bx],al
      mov al,ALL_OK

@@exit:
      POPR <es,ds>
       add  sp,AUTO_SIZE
       pop bp
      ret
re_activate endp

RSTVEC macro vec,old_vec_dist
      mov bx,pcmoffs
      sub bx,old_vec_dist
      lds dx,[es:bx]
      mov ax,(25h shl 8) + vec           
      int 21h
endm

CMPVEC macro vec,new_vec_dist
      mov ax,(35h shl 8) + vec          
      int 21h                        

      mov ax,es
      mov cx,pcmseg
      mov dx,pcmoffs
      sub dx,new_vec_dist
      call cmp_fptr     
      jnz @@wrong_vec
endm

SoftMouseReset proc
      mov ax,21h        
      int 33h
      ret
SoftMouseReset endp

de_inst proc
LOCAL  pcmoffs,pcmseg:WORD,tckflg:BYTE = AUTO_SIZE
       push bp
       mov  bp,sp
       sub  sp,AUTO_SIZE
       PUSHR <ds,es>
      call inst_tst
      cmp al,ALL_OK             
        je short @@cont_de_inst
      jmp short @@goexit
@@cont_de_inst:
      mov pcmoffs,bx  
      mov ax,es
      mov pcmseg,ax  
      CMPVEC 21h,pastebuf-new_21h
      CMPVEC 16h,pastebuf-new_16h
      CMPVEC 10h,pastebuf-new_10h
      CMPVEC 08h,pastebuf-new_08h
if ENABLEFILEAPPEND
      CMPVEC 13h,pastebuf-new_13h
endif ;ENABLEFILEAPPEND

        jmp short @@de_inst

@@wrong_vec:
        mov al,WRO_VEC
@@goexit:
        jmp short @@exit

@@de_inst:

        RSTVEC 21h,pastebuf-old_21h
        RSTVEC 16h,pastebuf-old_16h
        RSTVEC 10h,pastebuf-old_10h
        RSTVEC 08h,pastebuf-old_08h
if ENABLEFILEAPPEND
        RSTVEC 13h,pastebuf-old_13h
endif ;ENABLEFILEAPPEND

        call SoftMouseReset        
        mov es,pcmseg           
        mov bx,pcmoffs            
        sub bx,pastebuf-killed_flg 
        mov byte ptr [es:bx],TRUE  
        mov bx,pcmoffs            
        sub bx,pastebuf-XPC_pspadr 
        mov es,[es:bx]            
        mov ah,49h ; free memory
        int 21h

        mov al,ALL_OK
        jmp short @@exit

@@exit:
       POPR <es,ds>
       add  sp,AUTO_SIZE
       pop bp
       ret
de_inst endp

messages proc near
        push    ax
        cmp cs:quiet_flag,TRUE
         jz short @@exit
        mov     ah,9
        int     21h
@@exit:
        pop     ax
        ret
messages endp

messageAlways proc near
        push    ax
        mov     ah,9
        int     21h
        pop     ax
        ret
messageAlways endp

init proc
      mov ax,es
      mov cs:XPC_pspadr,ax      
      mov ax,cs
      mov ds,ax
      call get_opt

      cmp cs:copying_flag,FALSE
        jz short @@no_copying
      mov dx,offset copying
      jmp @@do_exit

@@no_copying:
      mov dx,offset onsign
      call messages

@@no_onsign:
      cmp cs:how_to_flag,FALSE
        jz short @@no_onsign1
        jmp @@do_how_to_exit
@@no_onsign1:
      cmp ah,TRUE
        jz short @@cont_inst
      cmp ah,FALSE
        mov dx,offset help_str
        jz @@do_exit

if ENABLEPASTE
@@tst_showxtclass:
      cmp ah,SHOWXTCLASS
        jnz short @@tst_appendpaste
        jmp short @@do_exit_raw
@@tst_appendpaste:
      cmp ah,APPENDPASTE
        jnz short @@tst_reactivate
        mov dx,offset appendf_str
        jmp @@do_message_exit
endif ;ENABLEPASTE

@@tst_reactivate:
      cmp ah,REACTIVATE
        jnz short @@tst_de_inst
@@do_reactivate:
      call re_activate
        cmp al,NOT_INSTALLED
        jz short @@not_yet1
        mov dx,offset re_inst_str
        jmp short @@do_exit

@@tst_de_inst:
      cmp ah,DE_INSTALL
        jnz short @@cont_inst
      call de_inst
        cmp al,NOT_INSTALLED
@@not_yet1:
        mov dx,offset not_yet_str
         jz @@do_exit
        cmp al,ALL_OK
        mov dx,offset ok_de_inst
         jz @@do_exit
        mov dx,offset wro_de_inst
         jmp short @@do_exit

@@cont_inst:

      TESTCPU         

      mov ax,21h             
      int 33h
      inc ax               
        jz short @@mouse_there
      mov dx,offset nomouse_str
      call messageAlways
@@errexit:
      mov  dx,offset noins_str
      call messageAlways
      jmp short @@do_exit_raw
@@do_exit:
if ENABLEPASTE
         cmp settmpname_flag,TRUE
           je @@do_exit_raw
endif  ;ENABLEPASTE
      call messages
@@do_exit_raw:
      mov  ax,4c01h          
      int  21h

@@mouse_there:
      call inst_tst
      jnz short @@install
      cmp ExecExit_flag,FALSE  
       je @@mouse_there_exit
       jmp @@do_reactivate      
@@mouse_there_exit:

@@do_how_to_exit:
      mov dx,offset help_switches
      call messages

      cmp cs:how_to_flag,FALSE
        je @@exit02

      mov dx,offset how_to

@@do_message_exit:
      call messages

      mov  ax,4c03h         
      int  21h

@@exit02:
      mov dx,offset msgstr
      call messages
@@exit02_nomsg:
      mov  ax,4c02h         
      int  21h

@@install:

      mov ax,cs:BUFLEN         
      add ax,offset pastebuf-1 
      mov cs:EndOfBuffer,ax   

if ENABLEPASTE
      cmp cs:counter_Xoption,0  
        jnz short @@video_OK
      push ds
       mov ds,biosSEG
       cmp byte ptr [ds:49h],7   
      pop ds
       jnz short @@video_OK
      mov xor_mask,119        
@@video_OK:
endif ;ENABLEPASTE
      mov es,cs:XPC_pspadr
      mov es,[es:2ch]   
      mov ah,49h     
      int 21h

      mov ax,3516h
      int 21h          
      mov word ptr old_16h,bx
      mov word ptr cs:[old_16h+2],es

      mov ax,3510h
      int 21h
      mov word ptr cs:[old_10h],bx
      mov word ptr cs:[old_10h+2],es

      mov ax,3521h
      int 21h
      mov word ptr cs:[old_21h],bx
      mov word ptr cs:[old_21h+2],es

      mov ax,3508h
      int 21h              
      mov word ptr old_08h,bx
      mov word ptr cs:[old_08h+2],es

if ENABLEFILEAPPEND
      mov ax,3513h
      int 21h              
      mov word ptr old_13h,bx
      mov word ptr cs:[old_13h+2],es
endif ;ENABLEFILEAPPEND

if ENABLEFILEAPPEND
      mov dx,offset new_13h
      mov ax,2513h          
      int 21h
endif ;ENABLEFILEAPPEND

      mov dx,offset new_10h
      mov ax,2510h          
      int 21h

      mov dx,offset new_16h
      mov ax,2516h           
      int 21h

      mov dx,offset new_08h
      mov ax,2508h       
      int 21h

      mov dx,offset new_21h
      mov ax,2521h          
      int 21h

      call SoftMouseReset
      call InstallMouseHandler

      mov dx,offset how_to
      call messages
      mov dx,offset worked
      call messages

@@terminate:
      mov dx,cs:BUFLEN          
      add dx,offset pastebuf   
      add dx,15+256             
      shr dx,1            
      shr dx,1                
      shr dx,1             
      shr dx,1            
      mov ax,3100h
      int 21h              

init endp

copying:
       db ' ',13,10
       db '$'

if ENABLEPASTE and ENABLECURSOR
MOUSEPROG equ 'E/MOUSE'
endif ;ENABLEPASTE and ENABLECURSOR

if ENABLEPASTE and (not ENABLECURSOR)
MOUSEPROG equ 'E/MOUSE PASTE'
endif ;ENABLEPASTE and (not ENABLECURSOR)

if (not ENABLEPASTE) and ENABLECURSOR
MOUSEPROG equ 'E/MOUSE CUR'
endif ;(not ENABLEPASTE) and ENABLECURSOR

onsign db 13,10,MOUSEPROG,' version ',PVERSION,13,10
       db ' ',13,10
       db '$'

how_to:
       db 13,10
       db 'Teclas y botones         Funcion que realiza',13,10
       db '======================== ==============================',13,10
if ENABLEPASTE
       db 'Click continuo.......... Copia el texto',13,10
       db 'Doble click boton 1..... Copia el texto',13,10
       db 'Boton 2................. Pega el texto',13,10
       db 'CTRL-Boton 2............ Pega el texto y presiona enter',13,10
       db 'DSHIFT-boton 2.......... Pega textos ON/OFF',13,10
       db 'ISHIFT-CTRL-BOTON 2..... Utiliza archivos temporarios',13,10
       db 'ISHIFT-ALT-BOTON 2...... Copia en Windows',13,10
endif ;ENABLEPASTE
if ENABLECURSOR
       db 'Boton 1................. Posicion del cursor ',13,10
       db 'Boton 3................. Retorno del cache ',13,10
       db 'ALT-Mouse 2............. Replica del boton 3',13,10
       db 'ISHIFT-Boton 1.......... Cursor on/off ',13,10
       db 'ISHIFT-ALT-Boton 1...... Cambio de metodo /A1-/A3',13,10
endif ;ENABLECURSOR
       db 'CTRL-ISHIFT-DSHIFT...... Reinstala el programa',13,10
       db '$'

worked db 13,10
       db MOUSEPROG,' ha sido instalado. Utilice "',MOUSEPROG
       db ' /U" para desinstalarlo.'
       db 13,10,'$'
msgstr db 13,10
       db 'E/MOUSE ya ha sido instalado.',13,10,'$'
not_yet_str db 13,10,  MOUSEPROG,' no ha sido instalado.',13,10,'$'
ok_de_inst  db 13,10,  MOUSEPROG,' no se puede desinstalar.',13,10,'$'
re_inst_str db 13,10,  MOUSEPROG,' ha sido reactivo.',13,10,'$'
wro_de_inst db 13,10,  'Imposible de desinstalar ',MOUSEPROG,'.',13,10,'$'
nomouse_str db 13,10,7,'No hay driver presente de mouse.',13,10,'$'
noins_str   db         MOUSEPROG,' no instalado.',13,10,'$'
if not XT
wro_cpu_str db         MOUSEPROG,' requiere ordenadores 80286 o superiores.',13,10,'$'
endif

if ENABLEPASTE
appendf_str db 13,10,  'Buffer iniciado.',13,10,'$'
endif ;ENABLEPASTE

help_str db 13,10,'Opciones: /Q   : Modo silencioso.',13,10
               db '          /R   : Reactiva a CUT.',13,10
               db '          /Sabc: Swap botones (1,2,3)',13,10
;               db '         /T   : Abilita el reloj interno.',13,10
               db '         /N0   : Por defecto nulo.',13,10
               db '         /N1   : Se reinstala despues del escape.',13,10
               db '         /N2   : Se reinstala antes del escape.',13,10
               db '         /N3   : Se reinstala despues de un EXE.',13,10
if ENABLECURSOR          
               db '         /K    : Emulacion por defecto.',13,10
               db '         /A1   : Utiliza el sistema de teclas emac.',13,10
               db '         /A2   : Utiliza el metodo matrix.',13,10
               db '         /A3   : Sale del modo X.',13,10
endif   ;end ENABLECURSOR  
if ENABLEPASTE
               db '         /M    : Fuerza a video monocromo ( /X119).',13,10
               db '         /Xddd : Cambia el sistema segun el monitor (/X80).',13,10
               db '         /Bdddd: Cambia el espacio del buffer.',13,10
               db '         /P    : Utiliza el buffer para pegar.',13,10
               db '         /D=X  : Setea las terminales X en el archivo X.',13,10
               db '         /F=X  : Abre y activa el buffer en el archivo X.',13,10
               db '         /F:X  : Cambia el archivo temporario por otro X.',13,10
endif ;ENABLEPASTE
help_switches:
               db 'Informe: /C    : Muestra la informacion de copyright.',13,10
if ENABLEPASTE
               db '         /D    : Muestra las clases de Xterminals.',13,10
endif ;ENABLEPASTE
               db '         /?    : Muestra los comandos de ayuda.',13,10
               db '         /H    : Muestra el boton de asignacion.',13,10
               db '         /U    : Se desinstala de la memoria.',13,10
               db '$'

if ENABLEPASTE
if  ($-pastebuf) lt BUFFERLEN
        db  BUFFERLEN-($-pastebuf) dup (?)
endif

end_mouse equ pastebuf+BUFFERLEN
endif ;ENABLEPASTE
code ends

stck segment para stack 'stack'
  db 256 dup (?)
stck ends

end init

