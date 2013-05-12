SEEK_CUR	= 1
SEEK_END	= 2
SEEK_SET	= 0

_NSTREAM_	= 10
_MAXIOBUF       = 4000h
_INTIOBUF	= 1000h
_MINIOBUF	= 0200h

_IOFBF          = 0000h
_IOREAD		= 0001h
_IOWRT		= 0002h
_IONBF		= 0004h
_IOMYBUF	= 0008h
_IOEOF  	= 0010h
_IOERR  	= 0020h
_IOSTRG		= 0040h
_IOLBF		= 0040h
_IORW		= 0080h
_IOYOURBUF	= 0100h
_IOSETVBUF      = 0400h
_IOFEOF		= 0800h
_IOFLRTN	= 1000h
_IOCTRLZ        = 2000h
_IOCOMMIT	= 4000h

S_FILE		STRUC
iob_bp		PD ?
iob_cnt		DW ?
iob_base	PD ?
iob_flag	DW ?
iob_file	DW ?
iob_bufsize	DW ?
iob_charbuf	DW ?
S_FILE		ENDS

GLOBAL	_bufin:	BYTE

GLOBALP	fopen,		:DPTR, :DPTR
GLOBALP	fclose,		:DPTR
GLOBALP	fgetc,		:DPTR
GLOBALC	printf,		:DPTR, :DPTR
GLOBALC	sprintf,	:DPTR, :DPTR, :DPTR
GLOBALC	fprintf,	:DPTR, :DPTR, :DPTR
GLOBALP	vsprintf,	:DPTR, :DPTR, :DPTR
GLOBALP	fread,		:DPTR, :WORD, :WORD, :DPTR
GLOBALP	fwrite,		:DPTR, :WORD, :WORD, :DPTR
GLOBALP	fseek,		:DPTR, :DWORD, :WORD
GLOBALP	fgets,		:DPTR, :WORD, :DPTR
GLOBALP	fputs,		:DPTR, :DPTR
GLOBALP	ftell,		:DPTR

GLOBALP	perror,		:DPTR
GLOBALP	fflush,		:DPTR
GLOBALP	_output,	:DPTR, :DPTR, :DPTR
GLOBALP	_flush,		:DPTR
GLOBALP	_flsbuf,	:WORD, :DPTR
GLOBALP	_freebuf,	:DPTR
GLOBALP	_filebuf,	:DPTR
GLOBALP	_openfile,	:DPTR
GLOBALP	_getst
GLOBALP	_ftbuf,		:WORD, :DPTR
GLOBALP	_stbuf,		:DPTR
GLOBALP	_getbuf,	:DWORD
GLOBALP	setvbuf,	:DPTR, :DPTR, :WORD, :WORD

GLOBALD	@sprintf
GLOBALN	@vsbufin
GLOBALD	@printf

