WMAXPATH  = 260
WMAXDRIVE = 3
MAXDRIVES = 27
WMAXDIR   = 256
WMAXFILE  = 256
WMAXEXT   = 256
MAXPATH	  = 80
MAXDRIVE  = 3
MAXDIR    = 66
MAXFILE   = 9
MAXEXT    = 5
SIZEWFBLK = 318

GLOBAL	fp_maskp:	DPTR
GLOBAL	fp_directory:	FPTR	; int (*fp_directory)(char *);
GLOBAL	fp_fileblock:	FPTR	; int (*fp_fileblock)(char *, wfblk *);
GLOBAL	scan_fblock:	BYTE;:[SIZE S_WFBLK]
GLOBAL	scan_curfile:	BYTE
GLOBAL	scan_curpath:	BYTE

GLOBALD	getdrv
GLOBALP	chdrv,		:WORD
GLOBALP	chdir,		:DPTR
GLOBALP	mkdir,		:DPTR
GLOBALP	rmdir,		:DPTR
GLOBALP	getcwdd,	:DPTR, :WORD
GLOBALP	fullpath,	:DPTR, :WORD
GLOBALP	scansub,	:DPTR, :DPTR, :WORD
GLOBALP	scan_files,	:DPTR
GLOBALP	scan_directory,	:WORD, :DPTR
GLOBALP	recursive,	:DWORD, :DWORD, :DWORD
