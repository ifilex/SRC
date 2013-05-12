_A_NORMAL	equ	00h
_A_RDONLY	equ	01h
_A_HIDDEN	equ	02h
_A_SYSTEM	equ	04h
_A_SUBDIR	equ	10h
_A_ARCH		equ	20h

SEEK_SET    	equ	0
SEEK_CUR    	equ	1
SEEK_END    	equ	2

IO_SEARCHCASE	equ	0001h
IO_SEARCHHEX    equ	0002h
IO_SEARCHSET	equ	0004h
IO_SEARCHCUR	equ	0008h
IO_SEARCHMASK	equ	000Fh
IO_SEARCHSUB	equ	0020h

IO_LINEBUF	equ	0040h	; Line buffer
IO_STRINGB	equ	0080h	; String buffer
IO_CRYPT	equ	0100h	; Crypted file
IO_USEUPD	equ	0200h	; Progress
IO_UPDTOTAL	equ	0400h	; Add Read/Write size to total
IO_USECRC	equ	0800h	; Update CRC on read/write
IO_USEBITS	equ	1000h	; Align bits on read/write
IO_CLIPBOARD	equ	2000h	; Flush to clipboard
IO_MEMREAD	equ	4000h	; View memory area
IO_ERROR	equ	8000h	; Write fault

S_IOST		STRUC
ios_bp		dd ?
ios_i		dw ?
ios_c		dw ?
ios_size	dw ?
ios_flag	dw ?
ios_file	dw ?
ios_l		dw ?
ios_bb		dd ?
ios_total	dd ?
ios_offset	dd ?
S_IOST		ENDS

externdef	STDI:		dword
externdef	STDO:		dword
externdef	crctab:		dword
externdef	oupdate: 	FPTR
externdef	tinfo:		word

OSTDI		equ offset STDI
OSTDO		equ offset STDO

oupdcrc		proto
otell		proto
owrite		proto
oopenst		proto
oopencl		proto

oread		proto
oputc		proto
ogetc		proto

oclose		proto _CType :dword
oinitst		proto _CType :dword, :word
ofreest		proto _CType :dword
continuesearch	proto dist
oflush		proto dist
ofread		proto dist
oungetc		proto dist

ogetl	 	proto _CType :dword, :dword, :word
ogets	 	proto _CType
oopen 		proto _CType :dword, :word
oopens 		proto _CType :dword, :word
iopens 		proto _CType :dword, :word
close 		proto _CType :word
ogetouth 	proto _CType :dword
openfile 	proto _CType :dword, :word, :word
ogetouth 	proto _CType :dword
oseek 		proto _CType :dword, :word
oseekl		proto _CType :dword, :word
ocopy		proto _CType :dword
osearch		proto _CType :word, :dword, :dword, :word, :word
cmdsearch	proto _CType :dword

ugetc		proto dist
uputc		proto _CType :word
