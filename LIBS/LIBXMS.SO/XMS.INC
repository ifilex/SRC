; include file for the Free-DOS XMS driver
; Copyright 2001, Martin Str”mberg.
; Copyright (c) 1995, Till Gerken
 
INFO_STR                equ     'Version para 128 MB.'
DRIVER_VERSION          equ     '1.0'
INTERFACE_VERSION       equ     '3.3'
INTERFACE_VER		=	300h

XMS_START               =       1088    

CMD_INIT                =       0      

STATUS_OK               =       0100h   
STATUS_BAD              =       8000h  

VDISK_IDSTR		equ	"VDISK"
VDISK_IDLEN		=	5
VDISK_IDOFS		=	12h

XMS_NOT_IMPLEMENTED	=	80h
XMS_VDISK_DETECTED	=	81h
XMS_A20_FAILURE		=	82h
XMS_DRIVER_FAILURE	=	8eh
XMS_DRIVER_FATAL	=	8fh
XMS_HMA_NOT_THERE	=	90h
XMS_HMA_IN_USE		=	91h
XMS_HMAREQ_TOO_SMALL	=	92h
XMS_HMA_NOT_USED	=	93h
XMS_A20_STILL_ENABLED	=	94h
XMS_ALREADY_ALLOCATED	=	0a0h
XMS_NO_HANDLE_LEFT	=	0a1h
XMS_INVALID_HANDLE	=	0a2h
XMS_INVALID_SOURCE_HANDLE =	0a3h
XMS_INVALID_SOURCE_OFFSET =	0a4h
XMS_INVALID_DESTINATION_HANDLE=	0a5h
XMS_INVALID_DESTINATION_OFFSET=	0a6h
XMS_INVALID_LENGTH	=	0a7h
XMS_OVERLAP		=	0a8h
XMS_PARITY_ERROR	=	0a9h
XMS_BLOCK_NOT_LOCKED	=	0aah
XMS_BLOCK_LOCKED	=	0abh
XMS_LOCK_COUNT_OVERFLOW	=	0ach
XMS_LOCK_FAILED		=	0adh
XMS_ONLY_SMALLER_UMB	=	0b0h
XMS_NO_UMB_AVAILABLE	=	0b1h
XMS_UMB_SEGMENT_NR_INVALID =	0b2h

struc	request_hdr
  req_size      db      ?               
  unit_id       db      ?             
  cmd           db      ?             
  status        dw      ?             
  rsvd          db      8 dup (?)     
ends	request_hdr

struc	init_strc
  init_hdr	db	size request_hdr dup (?)
  units         db      ?              
  end_addr      dd      ?              
  cmd_line      dd      ?            
ends	init_strc

struc	desc
  limit         dw      0ffffh         
  base0_15      dw      ?       
  base16_23     db      ?          
  flags         db      93h          
  reserved	db	0
ends	desc

struc	xms_handle
  xbase         dw      ?               
  xsize         dw      ?             
  used          db      ?             
  locks         db      ?            
ends	xms_handle

struc	xms_move_strc
  len           dd      ?              
  src_handle    dw      ?            
  src_offset    dd      ?              
  dest_handle   dw      ?              
  dest_offset   dd      ?              
ends	xms_move_strc
