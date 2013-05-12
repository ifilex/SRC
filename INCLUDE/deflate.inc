include	clib.inc
include	iost.inc
include	string.inc
include	stdio.inc
include	malloc.inc
include	unzip.inc
include	errno.inc
include	math.inc

SEGM64K	= 0FFFFh	; malloc arg to allocate a segment

MIN_MATCH	= 3	; The minimum and maximum match lengths
MAX_MATCH	= 258

; Maximum window size = 32K. If you are really short of memory, compile
; with a smaller WSIZE but this reduces the compression ratio for files
; of size > WSIZE. WSIZE must be a power of two in the current implementation.
;
WSIZE		= 8000h
WMASK		= WSIZE-1

; Minimum amount of lookahead, except at the end of the input file.
; See deflate.c for comments about the MIN_MATCH+1.
;
MIN_LOOKAHEAD	= MAX_MATCH+MIN_MATCH+1

; In order to simplify the code, particularly on 16 bit machines, match
; distances are limited to MAX_DIST instead of WSIZE.
;
MAX_DIST	= WSIZE-MIN_LOOKAHEAD

FILE_BINARY	= 0	; internal file attribute
FILE_ASCII	= 1
FILE_UNKNOWN	= -1

METHOD_STORE	= 0	; Store method
METHOD_DEFLATE	= 8	; Deflation method
METHOD_BEST	= -1	; Use best method (deflation or store)

MEDIUM_MEM	= 1
HASH_BITS	= 14	; Number of bits used to hash strings
LIT_BUFSIZE	= 4000h

; HASH_SIZE and WSIZE must be powers of two
;
HASH_SIZE	= (1 shl HASH_BITS)
HASH_MASK	= HASH_SIZE-1
H_SHIFT  	= (HASH_BITS+MIN_MATCH-1)/MIN_MATCH

FAST		= 4	; speed options for the general purpose bit flag
SLOW		= 2

; Matches of length 3 are discarded if their distance exceeds TOO_FAR
;
TOO_FAR		= 4096
MAXSEG_64K	= 1

MAX_BITS	= 15	; All codes must not exceed MAX_BITS bits
MAX_BL_BITS	= 7	; Bit length codes must not exceed MAX_BL_BITS bits
LENGTH_CODES	= 29	; number of length codes, not counting the special
			; END_BLOCK code
LITERALS	= 256	; number of literal bytes 0..255
END_BLOCK	= 256	; end of block literal code
D_CODES		= 30	; number of distance codes
BL_CODES	= 19	; number of codes used to transfer the bit lengths

; number of Literal or Length codes, including the END_BLOCK code
;
L_CODES		= LITERALS+1+LENGTH_CODES

STORED_BLOCK	= 0	; The three kinds of block type
STATIC_TREES	= 1
DYN_TREES	= 2

DIST_BUFSIZE	= LIT_BUFSIZE

; repeat previous bit length 3-6 times (2 bits of repeat count)
;
REP_3_6		= 16
REPZ_3_10	= 17	; repeat a zero length 3-10 times (3 bits)
REPZ_11_138	= 18	; repeat a zero length 11-138 times (7 bits)

HEAP_SIZE	= 2*L_CODES+1	; maximum heap size

SMALLEST	= 1	; Index within the heap array of least
			; frequent node in the Huffman tree


; Data structure describing a single value and its code string.

CT_DATA STRUC
	ct_freq		DW ?	; frequency count
	ct_dad		DW ?	; father node in Huffman tree
CT_DATA ENDS

CT_UNION STRUC
	ct_code		DW ?	; bit string
	ct_len		DW ?	; length of bit string
CT_UNION ENDS

TREE_DESC STRUC
	td_dyn_tree	DW ?	; the dynamic tree
	td_static_tree	DW ?	; corresponding static tree or NULL
	td_extra_bits	DW ?	; extra bits for each code or NULL
	td_extra_base	DW ?	; base index for extra_bits
	td_elems	DW ?	; max number of elements in the tree
	td_max_length	DW ?	; max bit length for the codes
	td_max_code	DW ?	; largest code with non zero frequency
TREE_DESC ENDS

dconfig STRUC
	good_length	DW ?	; reduce lazy search above this match length
	max_lazy	DW ?	; do not perform lazy search above this match length
	nice_length	DW ?	; quit search above this match length
	max_chain	DW ?
dconfig ENDS

deflate_ss STRUC	; 7794 byte
	head		DW ?
	prev		DW ?
	window_size	DD ?
	block_start	DD ?
	sliding		DW ?
	ins_h		DW ?
	prev_length	DW ?
	match_start	DW ?
	str_start 	DW ?
	eofile		DW ?
	lookahead	DW ?
	max_chain_len	DW ?
	max_lazy_match	DW ?
	max_insert_len	equ max_lazy_match
	good_match	DW ?
  ifdef FULL_SEARCH
	nice_match	equ MAX_MATCH
  else
	nice_match	DW ?
  endif
	heap_len 	DW ?
	heap_max 	DW ?
	l_buf		DD ?
	d_buf		DD ?
	flag_buf	equ _bufin+1024
	last_lit	DW ?
	last_dist	DW ?
	last_flags	DW ?
	flags		DB ?
	flag_bit	DB ?
	opt_len		DD ?
	static_len	DD ?
	cmpr_bytelen	DD ?
	cmpr_lenbits	DD ?
	bi_buf		DW ?
	bi_valid	DW ?
	compr_level	DW ?
	compr_flags	DW ?
	lm_chain_length	DW ?			; longest_match()
	bt_max_code	DW ?			; build_tree()
	bt_next_node	DW ?
	st_prevlen	DW ?			; scan_tree()/send_tree()
	st_curlen	DW ?
	st_nextlen	DW ?
	st_count	DW ?
	st_max_code	DW ?
	st_max_count	DW ?
	st_min_count	DW ?
	gb_bits		DW ?			; gen_bitlen()
	gb_overflow	DW ?
	fb_opt_len	DD ?
	fb_static_ln	DD ?
	fb_buf		DD ?
	fb_stored_ln	DD ?
	fb_eof		DW ?
	cb_dist		DW ?			; compress_block()
	cb_lc		DW ?
	cb_lx		DW ?
	cb_dx		DW ?
	cb_fx		DW ?
	cb_flag		DB ?
	cb_code		DW ?
	cb_extra	DW ?
	gc_next_code	DW MAX_BITS+1 dup(?)	; gen_codes()
	d_mavailable	DW ?			; deflate()
	d_prev_match	DW ?
	d_match_length	DW ?
	dyn_ltree 	DW (2*HEAP_SIZE) 	dup(?)
	dyn_dtree	DW (2*(2*D_CODES+1)) 	dup(?)
	static_ltree	DW (2*(L_CODES+2)) 	dup(?)
	static_dtree	DW (2*(D_CODES)) 	dup(?)
	bl_tree		DW (2*(2*BL_CODES+1)) 	dup(?)
	heap		DW HEAP_SIZE		dup(?)
	depth		DB HEAP_SIZE		dup(?)
	base_length	DW LENGTH_CODES 	dup(?)
	length_code	DB MAX_MATCH-MIN_MATCH+1 dup(?)
	dist_code	DB 512 			dup(?)
	base_dist	DW D_CODES 		dup(?)
  ifdef DEBUG
	bits_sent	DD ?
	input_len	DD ?
	input_size	DD ?
  endif
deflate_ss ENDS
