; Inline compare case - uses DX

cmpaxi	macro
	mov dx,ax
	sub al,'A'
	sub ah,'A'
	cmp al,'Z' - 'A' + 1
	sbb al,al
	cmp ah,'Z' - 'A' + 1
	sbb ah,ah
	and ax,2020h
	xor ax,dx
	cmp ah,al
	endm

memcpy 		proto dist pascal :dword, :dword, :word
memmove 	proto dist pascal :dword, :dword, :word
memset 		proto dist pascal :dword, :word, :word
memzero 	proto dist pascal :dword, :word
memxchg 	proto dist pascal :dword, :dword, :word
strlen 		proto dist pascal :dword
strcpy 		proto dist pascal :dword, :dword
strncpy 	proto dist pascal :dword, :dword, :word
strcat 		proto dist pascal :dword, :dword
strncat 	proto dist pascal :dword, :dword, :word
strcmp 		proto dist pascal :dword, :dword
strncmp 	proto dist pascal :dword, :dword, :word
stricmp 	proto dist pascal :dword, :dword
strnicmp 	proto dist pascal :dword, :dword, :word
strchr 		proto dist pascal :dword, :word
strrchr 	proto dist pascal :dword, :word
strstr 		proto dist pascal :dword, :dword
strlwr 		proto dist pascal :dword
strrev 		proto dist pascal :dword
strupr 		proto dist pascal :dword
strcmpi		proto
strstart 	proto dist pascal :dword
strtrim 	proto dist pascal :dword
strfn 		proto dist pascal :dword
strpath		proto dist pascal :dword
strfcat 	proto dist pascal :dword, :dword, :dword
argvext 	proto dist pascal :dword, :dword
strtol 		proto dist pascal :dword
cmpwarg 	proto dist pascal :dword, :dword
cmpwargs 	proto dist pascal :dword, :dword
strxchg 	proto dist pascal :dword, :dword, :dword, :word
dostounix 	proto dist pascal :dword
unixtodos 	proto dist pascal :dword
atohex 		proto dist pascal :dword
hextoa 		proto dist pascal :dword
strext 		proto dist pascal :dword
setfext 	proto dist pascal :dword, :dword
addext 		proto dist
ssistart	proto 		; [si++] if _SPACE
ssitoend	proto 		; [si++] if _UPPER/_LOWER/_DIGIT and '_'
ftolower	proto dist
