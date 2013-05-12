ZIPHEADERID	= 4B50h	; two-byte zip lead-in
ZIPLOCALID	= 0403h	; signature local file
ZIPCENTRALID	= 0201h	; signature central file
ZIPDIGITALID	= 0505h	; digital signature
ZIPENDSENTRID	= 0605h	; signature end central

_A_ENCRYPTED	= 0200h
_A_EXTLOCHD	= 0400h

S_HUFT		STRUC
huft_e	 	DB ?
huft_b	 	DB ?
huft_n		DW ?
huft_seg	DW ?
S_HUFT		ENDS

S_LZIP		STRUC	; Local file header: 30 byte
lz_pkzip	DW ?	; local file header signature
lz_zipid	DW ?
lz_version	DW ?	; version needed to extract
lz_flag		DW ?	; general purpose bit flag
lz_method	DW ?	; compression method
lz_time		DW ?	; last mod file time
lz_date		DW ?	; last mod file date
lz_crc		DD ?	; crc-32
lz_csize	DD ?	; compressed size
lz_fsize	DD ?	; uncompressed size
lz_fnsize	DW ?	; file name length
lz_extsize	DW ?	; extra field length
S_LZIP		ENDS

S_CZIP		STRUC	; Central directory structure: 46 byte
cz_pkzip	DW ?	; central file header signature
cz_zipid	DW ?
cz_version_made	DW ?	; version made by
cz_version_need	DW ?	; version needed to extract
cz_bitflag	DW ?	; general purpose bit flag
cz_method	DW ?	; compression method
cz_time		DW ?	; last mod file time
cz_date		DW ?	; last mod file date
cz_crc		DD ?	; crc-32
cz_csize	DD ?	; compressed size
cz_fsize	DD ?	; uncompressed size
cz_fnsize	DW ?	; file name length
cz_extsize	DW ?	; extra field length
cz_cmtsize	DW ?	; file comment length
cz_disk		DW ?	; disk number start
cz_int_attrib	DW ?	; internal file attributes
cz_ext_attrib	DW ?	; external file attributes
cz_ext_attribdx DW ?
cz_off_local	DD ?	; relative offset of local header
S_CZIP		ENDS

S_ZEND		STRUC	; End of central directory record: 22 byte
ze_pkzip	DW ?	; end of central dir signature
ze_zipid	DW ?
ze_disk_cur	DW ?	; number of this disk
ze_disk_one	DW ?	; number at start of the central directory
ze_entry_cur	DW ?	; central directory on this disk
ze_entry_dir	DW ?	; total entries in the central directory
ze_size_cent	DD ?	; size central directory
ze_off_cent	DD ?	; offset central directory
ze_comment_size	DW ?	; .ZIP file comment length
S_ZEND		ENDS

externdef	zip_attrib:	WORD
externdef	zip_local: 	S_LZIP
externdef	zip_central: 	S_CZIP
externdef	zip_endcent: 	S_ZEND

unzip		proto dist
zip_inflate 	proto dist
zip_explode 	proto dist pascal
zip_deflate	proto dist pascal level:WORD
zip_copylocal	proto dist pascal exact_match:BYTE
zip_copycentral	proto dist pascal loffset:DWORD, lsize:DWORD, exact_match:BYTE
