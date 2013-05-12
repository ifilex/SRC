
		PAGE	,132
CODE		SEGMENT PUBLIC'code'
		ASSUME	CS:CODE
REQ_STRUC	STRUC
LEN		DB	?
UNIT            DB      ?                       
COMMAND         DB      ?                       
STATUS          DW      ?                       
RESERVE		DB	8 DUP (?)
MEDIA		DB	?
ADDRESS         DD      ?                       
CONFIG_PTR      DD      ?                       
REQ_STRUC	ENDS
DAT_SEG_DES     STRUC                          
SEG_LIM         DW      0                       
BASE_ADRL       DW      0                       
BASE_ADRH	DB	0
                DB      0                       
                DW      0                   
DAT_SEG_DES	ENDS
                ORG     0                    
HEADER          DD      -1                     
                DW      8000H                  
                DW      OFFSET STRATEGY         
                DW      OFFSET INTERRUPT        
                DB      'EMMXXXX0'            
;Device header end
SWAP_POINTER	DW	OFFSET EMS_EXCH_PAG
PROGRAM         DB      "EMS 1.2 "
COPYRIGHT       DB      " ",13,10
PROGRAMMER      DB      "EMS",254,"Gnu GLP 2.0",13,10,"$",26
REQ_HEADADR     DD      ?                       
GDT		LABEL	BYTE
                DAT_SEG_DES <>                  
                DAT_SEG_DES <>                 
SOURCE          DAT_SEG_DES <4000H,,,93H,>      
DEST            DAT_SEG_DES <4000H,,,93H,>      
                DAT_SEG_DES <>                  
                DAT_SEG_DES <>                  
STRATEGY	PROC	FAR
		ASSUME  CS:CODE,DS:NOTHING,ES:NOTHING
                MOV     WORD PTR CS:[REQ_HEADADR],BX    
                MOV     WORD PTR CS:[REQ_HEADADR+2],ES  
		RET
STRATEGY	ENDP
INTERRUPT	PROC	FAR
		ASSUME  CS:CODE,DS:NOTHING,ES:NOTHING
		PUSHF
                PUSH    AX                      
		PUSH	BX
		PUSH	CX
		PUSH	DX
		PUSH	DI
		PUSH	SI
		PUSH	DS
		PUSH	ES
                PUSH    CS                     
		POP	DS
		ASSUME  DS:CODE
                CLD                             
                LES     DI,[REQ_HEADADR]        
		ASSUME	ES:NOTHING
		MOV	BL,ES:[DI.COMMAND]
		CMP	BL,0
		JNE	PROCESS1
		CALL	INITIALIZE
		JMP	SHORT DONE
PROCESS1:       CMP     BL,16                   
		JBE	DONE
                MOV     AX,8003H                
DONE:           OR      AX,0100H               
		MOV	ES:[DI.STATUS],AX
                POP     ES                      
		POP	DS
		POP	SI
		POP	DI
		POP	DX
		POP	CX
		POP	BX
		POP	AX
		POPF
		RET
INTERRUPT	ENDP
DRIVER_END      =       $                       
OLD_INT15H	LABEL DWORD
OLD_INT15HO     DW      ?                       
OLD_INT15HS     DW      ?                     
EXT_MEM_LIMIT   DW      0                      
OS_ENABLED      DB      1                
OS_PASS_LOW     DW      0                   
OS_PASS_HIGH 	DW	0
ALT_MAP_PTRS    DW      0                  
ALT_MAP_PTRO 	DW	0
WINDOW_SEG      DW      ?                      
WINDOW_ADDR_BASE        DD      4 DUP(0)      
EXTEND_ADRL     DW      ?                      
EXTEND_ADRH	DB	?
TOTAL_PAGES     DW      24                     
TOTAL_HANDLES   DW      127                    
INT_SAVE_SIZE   DW      15                    
PAG_OWNER_TBL   DW      ?                      
HANDLE_ARRAY    DW      ?                       
MAP_ARRAY_PTR   DW      ?                       
MOVE_BUSY_FLAG  DB      0                 
SAVED_ADDR_LOW  DW      0                    
SAVED_ADDR_HIGH DB      0                    
INT_15H		PROC	FAR
		ASSUME  CS:CODE,DS:NOTHING,ES:NOTHING
		CMP	AH,88H
		JE	INT15_F88
		JMP	CS:[OLD_INT15H]
INT15_F88:      MOV     AX,CS:EXT_MEM_LIMIT           
                CLC                                   
		RET	2
INT_15H		ENDP
EMS_CMDS        DW      OFFSET EMS_01         
                DW      OFFSET EMS_02          
                DW      OFFSET EMS_03        
                DW      OFFSET EMS_04         
                DW      OFFSET EMS_05          
                DW      OFFSET EMS_06           
                DW      OFFSET EMS_07         
                DW      OFFSET EMS_08          
                DW      OFFSET EMS_09          
                DW      OFFSET EMS_UNSP         
                DW      OFFSET EMS_UNSP       
                DW      OFFSET EMS_12         
                DW      OFFSET EMS_13          
                DW      OFFSET EMS_14           
                DW      OFFSET EMS_15           
                DW      OFFSET EMS_16           
                DW      OFFSET EMS_17           
                DW      OFFSET EMS_18           
                DW      OFFSET EMS_19      
                DW      OFFSET EMS_20          
                DW      OFFSET EMS_21       
                DW      OFFSET EMS_22        
                DW      OFFSET EMS_23          
                DW      OFFSET EMS_24           
                DW      OFFSET EMS_25          
                DW      OFFSET EMS_26           
                DW      OFFSET EMS_27          
                DW      OFFSET EMS_28           
                DW      OFFSET EMS_UNSP         
                DW      OFFSET EMS_30           
INT_67H		PROC	FAR
		ASSUME 	CS:CODE,DS:NOTHING,ES:NOTHING
		PUSH	BP
                MOV     BP,SP                 
                CLD                            
                PUSH    CX                    
                PUSH    DI                     
                PUSH    SI              
                PUSH    DS                     
                PUSH    ES                   
                PUSH    CS                   
		POP	DS
		ASSUME  DS:CODE
                CMP     AH,5DH                 
		JA	EMS_CMD_ERR
		CMP	AH,40H
		JL	EMS_CMD_ERR
                MOV     DI,OFFSET RETURN_ADDR   
                PUSH    DI                    
                PUSH    AX                    
                SUB     AH,40H               
                MOV     AL,AH                   
                XOR     AH,AH                   
                SAL     AX,1                    
                ADD     AX,OFFSET EMS_CMDS      
                MOV     DI,AX                   
                POP     AX                     
                PUSH    [DI]                    
                MOV     DI,[BP-4]             
                CALL    EMS_CHECK_HDL          
                DB      0C3h                  
RETURN_ADDR:    POP     ES                     
                POP     DS                   
                POP     SI                   
                POP     DI                  
                POP     CX                    
		POP	BP
		IRET
EMS_CMD_ERR:    MOV     AH,84H                
		JMP	SHORT RETURN_ADDR
INT_67H		ENDP
EMS_01		PROC	NEAR
		ASSUME  CS:CODE,DS:CODE,ES:NOTHING
		XOR	AX,AX
		RET
EMS_01		ENDP
EMS_02		PROC	NEAR
		ASSUME  CS:CODE,DS:CODE,ES:NOTHING
                MOV     BX,WINDOW_SEG           
		XOR	AX,AX
		RET
EMS_02		ENDP
EMS_03		PROC	NEAR
		ASSUME  CS:CODE,DS:CODE,ES:NOTHING
                XOR     DX,DX                  
		DEC	DX
                CLC                            
		CALL	EMS_13
                MOV     DX,TOTAL_PAGES         
                XOR     AX,AX                  
		RET
EMS_03		ENDP
EMS_04		PROC	NEAR
		ASSUME  CS:CODE,DS:CODE,ES:NOTHING
                OR      BX,BX                   
                JE      EMS_04_ERR             
                CALL    EMS_27                  
EMS_04_EXIT:	RET
EMS_04_ERR:     MOV     AH,89H                  
		JMP	SHORT EMS_04_EXIT
EMS_04		ENDP
EMS_05		PROC	NEAR
		ASSUME  CS:CODE,DS:CODE,ES:NOTHING
		PUSH	BX
		PUSH	DX
                JC      EMS_05_EXIT             
                CMP     AL,3              
                JA      EMS_05_ERR0           
                CMP     BX,0FFFFH            
                JNE     EMS_05_S1             
                MOV     BL,AL                
		XOR	AX,AX
                XOR     DX,DX                   
		JMP	SHORT EMS_05_S2
EMS_05_S1:      PUSH    AX                 
                CALL    EMS_LOG2PHY         
                POP     BX                    
                JC      EMS_05_EXIT            
EMS_05_S2:      CALL    EMS_EXCH_PAG          
                XOR     AX,AX                 
EMS_05_EXIT:    POP     DX                     
		POP	BX
		RET
EMS_05_ERR0:    MOV     AH,8BH                  
		JMP	SHORT EMS_05_EXIT
EMS_05		ENDP
EMS_06		PROC	NEAR
		ASSUME  CS:CODE,DS:CODE,ES:NOTHING
		PUSH	BX
		PUSH	DX
                JC      EMS_06_EXIT             
                MOV     BL,0FFH                 
                CALL    EMS_DEALLOC          
                OR      DL,DL                 
                JE      EMS_06_GOOD           
                PUSH    CS                    
		POP	ES
		ASSUME	ES:CODE
		MOV	DI,HANDLE_ARRAY
                MOV     AX,DX                   
                MOV     CX,9                  
                MUL     CX                     
		ADD	DI,AX
                XOR     AL,AL            
		REP	STOSB
EMS_06_GOOD:    XOR     AX,AX                 
EMS_06_EXIT:	POP	DX
		POP	BX
		RET
EMS_06		ENDP
EMS_07		PROC	NEAR
		ASSUME  CS:CODE,DS:CODE,ES:NOTHING
                MOV     AX,0040H                
		RET
EMS_07		ENDP
EMS_08		PROC	NEAR
		ASSUME  CS:CODE,DS:CODE,ES:NOTHING
		PUSH	BX
		PUSH	DX
                JC      EMS_08_EXIT            
                MOV     SI,MAP_ARRAY_PTR      
                MOV     CX,INT_SAVE_SIZE      
                XOR     BX,BX                  
EMS_08_L1:      ADD     SI,18                  
                CMP     WORD PTR [SI],0FFFFh   
		JNE	EMS_08_S1
                MOV     DI,SI                  
		JMP	SHORT EMS_08_S3
EMS_08_S1:      CMP     [SI],DX                
		JNE	EMS_08_S2
                MOV     AH,8DH                
		JMP	SHORT EMS_08_EXIT
EMS_08_S2:      LOOP    EMS_08_L1             
                MOV     AH,8CH                 
		JMP	SHORT EMS_08_EXIT
EMS_08_S3:      PUSH    CS                    
		POP	ES
                ASSUME ES:CODE                 
		MOV	SI,MAP_ARRAY_PTR
                MOV     [SI],DX               
		MOV	CX,9
		REP	MOVSW
                XOR     AX,AX                  
EMS_08_EXIT:    POP	DX
            	POP	BX
		RET
EMS_08		ENDP
EMS_09		PROC	NEAR
		ASSUME  CS:CODE,DS:CODE,ES:NOTHING
		PUSH	BX
		PUSH	DX
                JC      EMS_09_EXIT          
		MOV	SI,MAP_ARRAY_PTR
		MOV	CX,INT_SAVE_SIZE
EMS_09_L1:      ADD     SI,18                  
                CMP     [SI],DX              
                JE      EMS_09_S1             
		LOOP	EMS_09_L1
                MOV     AH,8EH                
		JMP	SHORT EMS_09_EXIT
EMS_09_S1:      CALL    EMS_15_1                
                MOV     WORD PTR [SI],0FFFFh  
                XOR     AX,AX                 
EMS_09_EXIT:	POP	DX
		POP	BX
		RET
EMS_09		ENDP
EMS_12		PROC	NEAR
		ASSUME  CS:CODE,DS:CODE,ES:NOTHING
                MOV     DI,HANDLE_ARRAY        
                XOR     BX,BX                  
                MOV     CX,TOTAL_HANDLES     
EMS_12_L1:      CMP     BH,[DI]                
                JE      EMS_12_S1             
                INC     BL                     
EMS_12_S1:      ADD     DI,9                   
		LOOP	SHORT EMS_12_L1
                XOR     AX,AX                 
EMS_12_EXIT:	RET
EMS_12		ENDP
EMS_13		PROC	NEAR
		ASSUME  CS:CODE,DS:CODE,ES:NOTHING
		PUSH	DI
                JC      EMS_13_EXIT            
                MOV     DI,PAG_OWNER_TBL       
		MOV	CX,TOTAL_PAGES
                XOR     BX,BX                   
EMS_13_LOOP:    CMP     DL,[DI]                 
		JNE	EMS_13_SKIP
                INC     BX                      
EMS_13_SKIP:    ADD     DI,3                   
		LOOP	EMS_13_LOOP
                XOR     AX,AX                  
EMS_13_EXIT:	POP	DI
		RET
EMS_13		ENDP
EMS_14		PROC	NEAR
		ASSUME  CS:CODE,DS:CODE,ES:NOTHING
		PUSH	DX
                XOR     DX,DX                   
                XOR     SI,SI               
EMS_14_L1:      CALL    EMS_CHECK_HDL         
                CALL    EMS_13                
                JC      EMS_14_S1              
                INC     SI                     
                MOV     ES:[DI],DX             
		MOV	ES:2[DI],BX
EMS_14_S1:      INC     DX                
                ADD     DI,4                 
                CMP     DX,TOTAL_HANDLES       
                JL      EMS_14_L1               
                MOV     BX,SI                 
                XOR     AX,AX                  
EMS_14_EXIT:	POP	DX
		RET
EMS_14		ENDP
EMS_15_TBL      DB      3               
                DW      OFFSET EMS_15_0 
		DW	OFFSET EMS_15_1
		DW	OFFSET EMS_15_2
		DW	OFFSET EMS_15_3
EMS_15		PROC	NEAR
		ASSUME  CS:CODE,DS:CODE,ES:NOTHING
		PUSH	BX
		PUSH	DX
                MOV     DS,SS:[BP-8]            
		ASSUME  DS:NOTHING
                MOV     DI,OFFSET EMS_15_TBL  
                CALL    EMS_DISPATCHER          
EMS_15_EXIT:    POP     DX                   
		POP	BX
		RET
EMS_15		ENDP
EMS_15_0	PROC	NEAR
		ASSUME  DS:NOTHING,ES:NOTHING
		PUSH	SI
		PUSH	DS
		PUSH	CS
		POP	DS
		ASSUME  DS:CODE
                MOV     SI,MAP_ARRAY_PTR        
                MOV     CX,9                   
                REP     MOVSW                  
		XOR	AH,AH
		POP	DS
		POP	SI
		RET
EMS_15_0	ENDP
EMS_15_1	PROC	NEAR
		ASSUME  DS:NOTHING,ES:NOTHING
		PUSH	SI
                XOR     BX,BX                 
                ADD     SI,2                   
EMS_15_1_L1:    MOV     AX,DS:[SI]             
		MOV	DX,DS:[SI+2]
		PUSH	BX
                CALL    EMS_EXCH_PAG            
                POP     BX                    
		ADD	SI,4
		INC	BX
                CMP     BL,3                    
                JLE     EMS_15_1_L1          
		XOR	AH,AH
EMS_15_1_EXIT:	POP	SI
		RET
EMS_15_1	ENDP
EMS_15_2	PROC	NEAR
		ASSUME  DS:NOTHING,ES:NOTHING
		PUSH	DI
                CALL    EMS_15_0                
		POP	DI
                CALL    EMS_15_1              
		XOR	AH,AH
		RET
EMS_15_2	ENDP
EMS_15_3	PROC	NEAR
		ASSUME  DS:NOTHING,ES:NOTHING
                MOV     AX,0012H                
		RET
EMS_15_3	ENDP
EMS_16_TBL      DB      2                     
                DW      OFFSET EMS_16_0        
		DW	OFFSET EMS_16_1
		DW	OFFSET EMS_16_2
EMS_16		PROC	NEAR
		ASSUME 	CS:CODE,DS:CODE,ES:NOTHING
		PUSH	BX
		PUSH	DX
                MOV     DS,SS:[BP-8]           
		ASSUME 	DS:NOTHING
                MOV     DI,OFFSET EMS_16_TBL   
                CALL    EMS_DISPATCHER          
EMS_16_EXIT:    POP     DX                 
		POP	BX
		RET
EMS_16		ENDP
EMS_16_0	PROC	NEAR
		ASSUME  DS:NOTHING,ES:NOTHING
                MOV     CX,DS:[SI]             
                MOV     ES:[DI],CX            
EMS_16_0_L1:    MOV     AX,DS:2[SI]            
                CALL    EMS_SEG2LOG           
		JC	EMS_16_0_EXIT
                MOV     ES:2[DI],BX           
                SAL     BX,1                  
                SAL     BX,1                   
		ADD	BX,CS:MAP_ARRAY_PTR
		MOV	AX,CS:2[BX]
                MOV     ES:4[DI],AX            
		MOV	AX,CS:4[BX]
                MOV     ES:6[DI],AX           
		ADD	DI,6
                INC     SI                      
		INC	SI
		LOOP	EMS_16_0_L1
		XOR	AX,AX
EMS_16_0_EXIT:	RET
EMS_16_0	ENDP
EMS_16_1	PROC	NEAR
		ASSUME  DS:NOTHING,ES:NOTHING
                MOV     CX,DS:[SI]            
EMS_16_1_L1:    MOV     BX,2[SI]                
                MOV     AX,4[SI]               
                MOV     DX,6[SI]               
                CALL    EMS_EXCH_PAG          
                ADD     SI,6                   
                LOOP    EMS_16_1_L1           
                XOR     AX,AX                  
EMS_16_1_EXIT:	RET
EMS_16_1	ENDP
EMS_16_2	PROC	NEAR
		ASSUME  DS:NOTHING,ES:NOTHING
                MOV     AX,BX                   
                MOV     AH,6                 
		MUL	AH
                ADD     AX,2                  
                XOR     AH,AH                
		RET
EMS_16_2	ENDP
EMS_17		PROC	NEAR
		ASSUME  CS:CODE,DS:CODE,ES:NOTHING
                MOV     ES,SS:[BP-8]          
		CALL	EMS_17_INTERNAL
		RET
EMS_17		ENDP
EMS_17_INTERNAL	PROC	NEAR
		ASSUME  CS:CODE,DS:CODE,ES:NOTHING
		PUSH	BX
		PUSH	DX
                JC      EMS_17_EXIT           
                CMP     AL,1                   
		JA	EMS_17_ERR1
                CMP     CX,4                    
                JA      EMS_17_ERR2            
                MOV     CH,AL                  
EMS_17_L1:      MOV     AX,ES:[SI+2]        
		OR	CH,CH
                JE      EMS_17_S3              
                CALL    EMS_SEG2LOG           
                JC      EMS_17_EXIT            
		MOV	AL,BL
EMS_17_S3:      MOV     BX,ES:[SI]           
                CLC                            
                PUSH    CX                   
                CALL    EMS_05                 
                POP     CX                    
                OR      AH,AH                  
		JNE	EMS_17_EXIT
                ADD     SI,4                   
                DEC     CL                      
		JNZ	EMS_17_L1
                XOR     AX,AX                 
EMS_17_EXIT:	POP	DX
		POP	BX
		RET
EMS_17_ERR1:    MOV     AH,8FH                 
		JMP	SHORT EMS_17_EXIT
EMS_17_ERR2:    MOV     AH,8BH                 
                JMP     SHORT EMS_17_EXIT      
EMS_17_INTERNAL	ENDP
EMS_18		PROC	NEAR
		ASSUME  CS:CODE,DS:CODE,ES:NOTHING
		PUSH	BX
		PUSH	DX
                JC      EMS_18_EXIT           
                MOV     DI,BX                  
		PUSH	BX
                CALL    EMS_13               
		MOV	CX,BX
                POP     BX                     
		SUB	BX,CX
		JG	EMS_18_INCREASE
		JL	EMS_18_REDUCE
                XOR     AH,AH                
EMS_18_EXIT:	POP	DX
		POP	BX
		RET
EMS_18_INCREASE:
		CALL	EMS_ASSIGN
		JMP	SHORT EMS_18_EXIT
EMS_18_REDUCE:  NEG     BX                      
		CALL	EMS_DEALLOC
		JMP	SHORT EMS_18_EXIT
EMS_18		ENDP
EMS_19		PROC	NEAR
		ASSUME  CS:CODE,DS:CODE
		CMP	AL,1
                JE      EMS_19_1             
                CMP     AL,2                   
                JG      EMS_19_ERR1           
EMS_19_GOOD_EXIT:
                XOR     AX,AX                 
EMS_19_EXIT:	RET
EMS_19_1:       CALL    EMS_CHECK_HDL         
		JC	EMS_19_ERR2
                CMP     BL,1                   
                JG      EMS_19_ERR3          
		JL	EMS_19_GOOD_EXIT
                MOV     AH,91H               
		JMP	SHORT EMS_19_EXIT
EMS_19_ERR1:    MOV     AH,8FH                
		JMP	SHORT EMS_19_EXIT
EMS_19_ERR2:    MOV     AH,83H                 
		JMP	SHORT EMS_19_EXIT
EMS_19_ERR3:    MOV     AH,90H                
		JMP	SHORT EMS_19_EXIT
EMS_19		ENDP
EMS_20		PROC	NEAR
		ASSUME  CS:CODE,DS:CODE,ES:NOTHING
		PUSH	BX
		PUSH	DX
                JC      EMS_20_EXIT           
                PUSH    AX                   
                MOV     BX,CS                 
                MOV     AX,DX                 
                MOV     AH,9                   
                MUL     AH                     
		MOV	DX,AX
		ADD	DX,HANDLE_ARRAY
                INC     DX                     
                POP     AX             
                OR      AL,AL                 
		JNE	EMS_20_S1
                MOV     SI,DX                 
		JMP	SHORT EMS_20_MOV
EMS_20_S1:      CMP     AL,1                   
                JNE     EMS_20_ERR1          
                MOV     DS,SS:[BP-8]            
		ASSUME  DS:NOTHING
		PUSH	SI
		PUSH	DX
                CALL    EMS_21_1             
                POP     DX                    
                POP     SI                      
                AND     AH,0FEH                
		CMP	AH,0A0H
		JNE	EMS_20_ERR2
                MOV     DI,DX                  
		PUSH	CS
		POP	ES
		ASSUME  ES:CODE
EMS_20_MOV:     MOV     CX,8                   
		REP	MOVSB
                XOR     AX,AX                  
EMS_20_EXIT:	POP	DX
		POP	BX
		RET
EMS_20_ERR1:    MOV     AH,8FH                
		JMP	SHORT EMS_20_EXIT
EMS_20_ERR2:    MOV     AH,0A1H                 
		JMP	SHORT EMS_20_EXIT
EMS_20		ENDP
EMS_21_TBL      DB      2              
                DW      OFFSET EMS_21_0 
		DW	OFFSET EMS_21_1
		DW	OFFSET EMS_21_2
EMS_21		PROC	NEAR
		ASSUME  CS:CODE,DS:CODE,ES:NOTHING
                MOV     DS,SS:[BP-8]          
		ASSUME  DS:NOTHING
		MOV	DI,OFFSET EMS_21_TBL
		CALL	EMS_DISPATCHER
EMS_21_EXIT:	RET
EMS_21		ENDP
EMS_21_0	PROC	NEAR
		ASSUME  DS:NOTHING,ES:NOTHING
		PUSH	DX
		PUSH	CS
		POP	DS
		ASSUME  DS:CODE
		MOV	SI,HANDLE_ARRAY
                XOR     AX,AX                 
                MOV     DX,AX                  
EMS_21_0_L1:    CMP     BYTE PTR [SI],0       
                JE      EMS_21_0_S1           
                MOV     ES:[DI],DX         
                INC     SI                     
                ADD     DI,2                 
                MOV     CX,8                   
		REP	MOVSB
                INC     AL                  
		JMP	SHORT EMS_21_0_S2
EMS_21_0_S1:    ADD     SI,9                  
EMS_21_0_S2:    INC     DX                    
                CMP     DX,TOTAL_HANDLES       
                JB      EMS_21_0_L1          
		POP	DX
		RET
EMS_21_0	ENDP
EMS_21_1	PROC	NEAR
		ASSUME  DS:NOTHING
		PUSH	BX
                PUSH    DS                   
		POP	ES
		ASSUME ES:NOTHING
		MOV	DI,SI
                MOV     CX,8                 
                XOR     AL,AL                 
                REPE    SCASB                 
		JE	EMS_21_1_ERR1
                MOV     CX,CS:TOTAL_HANDLES    
                PUSH    CS                     
                POP     ES                     
		ASSUME  ES:CODE
                MOV     BX,SI                  
                MOV     AX,CS:HANDLE_ARRAY     
                INC     AX                     
                XOR     DX,DX                
EMS_21_1_L1:    MOV     DI,AX                 
                MOV     SI,BX                  
		PUSH	CX
		MOV	CX,8
		REPE	CMPSB
		POP	CX
                JE      EMS_21_1_EXIT           
                ADD     AX,9                 
		INC	DX
		LOOP	EMS_21_1_L1
                MOV     AH,0A0H               
		JMP	SHORT EMS_21_1_EXIT1
EMS_21_1_EXIT:	XOR	AH,AH
EMS_21_1_EXIT1:	POP	BX
		RET
EMS_21_1_ERR1:  MOV     AH,0A1H                
		JMP	SHORT EMS_21_1_EXIT1
EMS_21_1	ENDP
EMS_21_2	PROC	NEAR
		ASSUME	DS:NOTHING
                MOV     BX,CS:TOTAL_HANDLES     
                XOR     AX,AX               
		RET
EMS_21_2	ENDP
EMS_22		PROC	NEAR
		ASSUME  CS:CODE,DS:CODE,ES:NOTHING
                JC      EMS_22_EXIT            
                MOV     DI,SI               
                MOV     ES,SS:[BP-8]         
                MOV     SI,ES:[DI]            
                MOV     SS:[BP+2],SI           
                MOV     SI,ES:[DI+2]            
		MOV	SS:[BP+4],SI
                MOV     SI,ES:[DI+5]           
                MOV     ES,ES:[DI+7]          
                MOV     CL,ES:[DI+4]           
		XOR	CH,CH
                CALL    EMS_17_INTERNAL        
EMS_22_EXIT:	RET
EMS_22		ENDP
EMS_23		PROC	NEAR
		ASSUME  CS:CODE,DS:CODE,ES:NOTHING
                JC      EMS_23_JMP_END          
                CMP     AL,2                  
		JNE	EMS_23_S1
                MOV     BX,28                   
		XOR	AH,AH
EMS_23_JMP_END:	JMP	EMS_23_EXIT
EMS_23_S1:      PUSH    AX                 
                MOV     DI,SI                
                MOV     ES,SS:[BP-8]            
                PUSH    ES                  
		PUSH	DI
		XOR	CH,CH
                MOV     CL,ES:[DI+4]           
                MOV     SI,ES:[DI+5]        
                MOV     ES,ES:[DI+7]          
		PUSH	AX
                CLC                           
                CALL    EMS_17_INTERNAL        
		OR	AH,AH
                POP     AX                    
		JNE	EMS_23_EXIT
                MOV     AX,SS:[BP+6]            
		PUSH	AX
                POPF                            
                MOV     ES,SS:[BP-10]        
                MOV     DS,SS:[BP-8]          
                MOV     SI,SS:[BP-6]           
                MOV     DI,SS:[BP-4]            
                MOV     CX,SS:[BP-2]           
		ASSUME	DS:NOTHING
                PUSH    BP                      
                MOV     BP,SS:[BP]            
                MOV     AX,0000                 
                CALL    DWORD PTR DS:[SI]       
                POP     AX                      
                PUSH    BP                   
                MOV     BP,AX                  
                POP     AX                   
                PUSHF                          
		MOV	[BP-10],ES
                MOV     [BP-8],DS              
                MOV     [BP-6],SI          
		MOV	[BP-4],DI
		MOV	[BP-2],CX
                POP     CX                     
		MOV	SS:[BP+6],CX
                MOV     SS:[BP],AX             
		PUSH	CS
		POP	DS
		ASSUME  DS:CODE
                POP     DI                     
		POP	ES
		XOR	CH,CH
                MOV     CL,ES:[DI+9]          
                MOV     SI,ES:[DI+10]        
                MOV     ES,ES:[DI+12]          
                CLC                            
                POP     AX                     
                CALL    EMS_17_INTERNAL        
EMS_23_EXIT:	RET
EMS_23		ENDP
EMS_24_SUBFUN   EQU     [BP-1]                 
EMS_24_DIR      EQU     [BP-2]                 
EMS_24_COUNT    EQU     [BP-6]                
EMS_24_SRC_PTR  EQU     [BP-10]                
EMS_24_DEST_PTR EQU     [BP-14]              
EMS_24_RET_CODE EQU     [BP-15]               
EMS_24		PROC	NEAR
		ASSUME  CS:CODE,DS:CODE,ES:NOTHING
                PUSH    BX                     
		PUSH	DX
                MOV     ES,SS:[BP-8]           
                PUSH    ES                  
		PUSH	SI
		PUSH	BP
                MOV     BP,SP                 
                SUB     SP,18                  
                MOV     EMS_24_SUBFUN,AL        
		MOV	BYTE PTR EMS_24_RET_CODE,0
                MOV     AX,ES:[SI]              
		MOV	DX,ES:[SI+2]
                CMP     DX,10H                
		JB 	EMS_24_S0
                MOV     AH,96H                 
		JMP	EMS_24_EXIT
EMS_24_S0:      MOV     WORD PTR [EMS_24_COUNT],AX     
                MOV     WORD PTR [EMS_24_COUNT+2],DX  
                MOV     CX,16384                      
		DIV	CX
                MOV     CH,AL                 
                MOV     CL,DL                 
                ADD     SI,4                  
                MOV     DI,-10                 
		CALL	EMS_CHECK_MOV
		JC	EMS_24_JMP_EXIT
                PUSH    AX                    
		PUSH	DX
                ADD     SI,7                  
                MOV     DI,-14                 
		CALL	EMS_CHECK_MOV
                POP     DI                    
		POP	BX
                JNC     EMS_24_S01              
EMS_24_JMP_EXIT:
		JMP	EMS_24_EXIT
EMS_24_S01:     MOV     BYTE PTR EMS_24_DIR,0   
                MOV     CL,BYTE PTR ES:[SI]    
                ADD     CL,BYTE PTR ES:[SI-7]  
                CMP     CL,1                   
                JE      EMS_24_MOVEDATA        
                JB      EMS_24_S2               
                MOV     CX,ES:[SI+1]          
                CMP     CX,ES:[SI-6]           
                JNE     EMS_24_MOVEDATA      
                MOV     CX,ES:[SI-2]           
                CMP     CX,ES:[SI+5]         
                JB      EMS_24_S1              
                JA      EMS_24_S02            
                MOV     CX,ES:[SI-4]           
                CMP     CX,ES:[SI+3]            
                JB      EMS_24_S1              
EMS_24_S02:     MOV     BX,AX                 
                MOV     DI,DX                 
                SUB     SI,7                  
                MOV     BYTE PTR EMS_24_DIR,1  
EMS_24_S1:      CMP     DI,ES:[SI+5]            
                JB      EMS_24_MOVEDATA         
                JA      EMS_24_OVERLAP        
                CMP     BX,ES:[SI+3]            
                JB      EMS_24_MOVEDATA        
		JMP	SHORT EMS_24_OVERLAP
EMS_24_S2:      CMP     DX,DI                   
                JB      EMS_24_S3              
                JA      EMS_24_S21             
                CMP     AX,BX                  
                JB      EMS_24_S3              
EMS_24_S21:     MOV     SI,-14                
		CALL	EMS_24_ADR_CNV
                XCHG    DI,DX                  
		XCHG	BX,AX
                MOV     SI,-10                 
		CALL	EMS_24_ADR_CNV
                MOV     BYTE PTR EMS_24_DIR,1   
EMS_24_S3:      SUB     BX,WORD PTR [EMS_24_COUNT]     
		SBB	DI,WORD PTR [EMS_24_COUNT+2]
                CMP     DX,DI                  
                JB      EMS_24_MOVEDATA        
		CMP	AX,BX
		JBE	EMS_24_MOVEDATA
EMS_24_OVERLAP: MOV     BYTE PTR EMS_24_RET_CODE,92H   
                CMP     BYTE PTR EMS_24_SUBFUN,0        
                JE      EMS_24_MOVEDATA              
                MOV     AH,97H                         
                JMP     EMS_24_EXIT                   
EMS_24_MOVEDATA:
                MOV     BX,MAP_ARRAY_PTR        
                PUSH    [BX+8]                
		PUSH	[BX+6]
                PUSH    [BX+4]                 
		PUSH	[BX+2]
		STD
                CMP     BYTE PTR EMS_24_DIR,0  
                JNE     EMS_24_MOVE0           
		CLD
                MOV     AX,ES:[SI-4]            
                MOV     DX,ES:[SI-2]                   
                MOV     WORD PTR [EMS_24_SRC_PTR],AX    
                MOV     WORD PTR [EMS_24_SRC_PTR+2],DX  
                MOV     AX,ES:[SI+3]                    
		MOV	DX,ES:[SI+5]
		MOV	WORD PTR [EMS_24_DEST_PTR],AX
		MOV	WORD PTR [EMS_24_DEST_PTR+2],DX
EMS_24_MOVE0:   XOR     CX,CX                   
EMS_24_MOVELOOP:
                MOV     ES,[BP+4]             
                MOV     BX,[BP+2]              
                MOV     DX,ES                  
                MOV     AX,CX                  
                ADD     BX,4                   
                XOR     SI,SI                  
		CALL	EMS_24_MOV_SET
                PUSH    ES                    
		PUSH	DI
                XCHG    CX,AX                   
                MOV     ES,DX                 
                ADD     BX,7                   
		MOV	SI,1
                CALL    EMS_24_MOV_SET         
                POP     SI                    
                POP     BX                     
                CMP     AX,CX            
		JA	EMS_24_MOVE1
		MOV	CX,AX
EMS_24_MOVE1:   MOV     AX,WORD PTR [EMS_24_COUNT]      
		MOV	DX,WORD PTR [EMS_24_COUNT+2]
                OR      DX,DX                        
                JNE     EMS_24_MOVE2                 
                CMP     AX,CX                       
                JA      EMS_24_MOVE2                 
		MOV	CX,AX
EMS_24_MOVE2:   SUB     AX,CX                 
		SBB	DX,0
		PUSH	CX
                PUSH    AX                      
		PUSH	DX
                CMP     BYTE PTR EMS_24_SUBFUN,1        
                MOV     DS,BX                   
		ASSUME	DS:NOTHING
		JE	EMS_24_EXCH
                REP     MOVSB                   
                JMP	SHORT EMS_24_MOVE3
EMS_24_EXCH:    MOV     BL,ES:[DI]              
		MOVSB
                CMP     BYTE PTR EMS_24_DIR,0   
                JNE     EMS_24_EXCH1          
                MOV     DS:[SI-1],BL           
		JMP	SHORT EMS_24_EXCH2
EMS_24_EXCH1:   MOV     DS:[SI+1],BL           
EMS_24_EXCH2:   LOOP    EMS_24_EXCH             
EMS_24_MOVE3:   PUSH    CS                  
		POP	DS
		ASSUME	DS:CODE
                POP     DX                     
		POP	AX
		POP	CX
                MOV     WORD PTR [EMS_24_COUNT],AX     
		MOV	WORD PTR [EMS_24_COUNT+2],DX
		OR	DX,DX
                JNE     EMS_24_MOVE4            
		OR	AX,AX
		JLE	EMS_24_LOOP_DONE
EMS_24_MOVE4:	JMP	EMS_24_MOVELOOP
EMS_24_LOOP_DONE:                              
                XOR     BX,BX                  
                POP     AX                     
                POP     DX                      
                CALL    EMS_EXCH_PAG           
                MOV     BX,1                  
                POP     AX                      
                POP     DX                    
                CALL    EMS_EXCH_PAG            
                MOV     AH,BYTE PTR EMS_24_RET_CODE    
EMS_24_EXIT:    ADD     SP,18                   
		POP	BP
		POP	SI
		POP	ES
		POP	DX
		POP	BX
		RET
EMS_24		ENDP
EMS_24_ADR_CNV	PROC	NEAR
		PUSH	AX
		PUSH	DX
                MOV     CX,16384               
		DIV	CX
                MOV     WORD PTR [BP+SI],DX     
		MOV	CL,10
		SAL	AX,CL
                MOV     WORD PTR [BP+SI+2],AX   
		POP	DX
		POP	AX
		RET
EMS_24_ADR_CNV	ENDP
EMS_24_MOV_SET	PROC	NEAR
		ASSUME	DS:CODE
		PUSH	AX
		PUSH	BX
		PUSH	DX
		MOV	DX,SI
                MOV     SI,-10                 
		OR	DX,DX
		JE	EMS_24_MOV_SET_S0
                SUB     SI,4                    
EMS_24_MOV_SET_S0:
                MOV     DH,BYTE PTR ES:[BX]    
                MOV     AX,16383               
		MOV	DI,1
                OR      DH,DH                  
                JNE     EMS_24_MOV_SET_S1       
                MOV     DI,1024             
                                                
EMS_24_MOV_SET_S1:
                CMP     BYTE PTR EMS_24_DIR,0   
		JE	EMS_24_MOV_SET_S3
                SUB     WORD PTR [BP+SI],CX     
                JAE     EMS_24_MOV_SET_S2     
                MOV     WORD PTR [BP+SI],AX     
                SUB     WORD PTR [BP+SI+2],DI   
EMS_24_MOV_SET_S2:
                MOV     CX,WORD PTR [BP+SI]     
		INC	CX
		JMP	EMS_24_MOV_SET_S5
EMS_24_MOV_SET_S3:
                ADD     WORD PTR [BP+SI],CX     
                CMP     WORD PTR [BP+SI],AX     
		JBE	EMS_24_MOV_SET_S4
EMS_24_MOV_SET_S31:
                MOV     WORD PTR [BP+SI],0      
                ADD     WORD PTR [BP+SI+2],DI   
EMS_24_MOV_SET_S4:
                MOV     CX,AX                   
		SUB	CX,WORD PTR [BP+SI]
 		INC	CX
EMS_24_MOV_SET_S5:
                OR      DH,DH                  
                JE      EMS_24_MOV_SET_CONV1   
                PUSH    CX                     
		PUSH	DX
                MOV     AL,DL                   
                MOV     DX,WORD PTR ES:[BX+1] 
                MOV     BX,WORD PTR [BP+SI+2]   
                PUSHF                           
                CLD                            
                CLC                          
                CALL    EMS_05                  
                POPF                         
		POP	DX
                POP     CX                     
                MOV     AX,WINDOW_SEG          
                OR      DL,DL                   
                JE      EMS_24_MOV_SET_S6      
                ADD     AX,1024                
EMS_24_MOV_SET_S6:
                MOV     ES,AX                   
EMS_24_MOV_SET_EXIT:
                MOV     DI,WORD PTR [BP+SI]    
		POP	DX
		POP	BX
		POP	AX
		RET
EMS_24_MOV_SET_CONV1:
		MOV	DI,WORD PTR [BP+SI+2]
		MOV	ES,DI
		JMP	SHORT EMS_24_MOV_SET_EXIT
EMS_24_MOV_SET	ENDP
EMS_CHECK_MOV 	PROC	NEAR
		ASSUME  DS:CODE,ES:NOTHING
		PUSH	BX
		PUSH	CX
		PUSH	DI
		XOR	AX,AX
                MOV     AL,CH                   
                MOV     DI,AX                  
		XOR	CH,CH
                MOV     AX,ES:[SI+5]            
                MOV     BX,ES:[SI+3]            
                MOV     DL,ES:[SI]             
                OR      DL,DL                  
                JE      EMS_CHECK_MOV_CONV     
                CMP     DL,1                    
                JE      EMS_CHECK_MOV_S1        
                MOV     AH,98H                  
		JMP	EMS_CHECK_MOV_SET_ERR
EMS_CHECK_MOV_S1:
                CMP     BX,3FFFH                
		JA	EMS_CHECK_MOV_ERR1
                MOV     DX,ES:[SI+1]           
		PUSH	CX
                CALL    EMS_CHECK_HDL           
                CALL    EMS_13                 
                POP     CX                    
                OR      AH,AH                 
                JNE     EMS_CHECK_MOV_SET_ERR  
                MOV     DX,BX                   
                MOV     BX,ES:[SI+5]         
                MOV     AX,DI                   
                ADD     AX,BX                   
                ADD     CX,ES:[SI+3]     
                DEC     CX                   
		CMP	CX,16384
                JL      EMS_CHECK_MOV_S4      
                INC     AX                     
		SUB	CX,16384
EMS_CHECK_MOV_S4:
                CMP     AX,DX                  
                JG      EMS_CHECK_MOV_ERR3   
                MOV     DX,CX                 
                XCHG    AX,DX              
EMS_CHECK_MOV_EXIT:
                CLC                         
EMS_CHECK_MOV_EXIT1:
                POP     DI                     
                MOV     [BP+DI],AX             
                MOV     [BP+DI+2],DX           
		POP	CX
		POP	BX
		RET
EMS_CHECK_MOV_CONV:
                MOV     CX,16                   
		MUL	CX
                ADD     AX,BX             
		ADC	DX,0
                ADD     AX,WORD PTR [EMS_24_COUNT]      
		ADC	DX,WORD PTR [EMS_24_COUNT+2]
                SUB     AX,1                   
		SBB	DX,0
		JMP	SHORT EMS_CHECK_MOV_EXIT
EMS_CHECK_MOV_ERR1:
                MOV     AH,95H                  
EMS_CHECK_MOV_SET_ERR:
                STC                            
		JMP	SHORT EMS_CHECK_MOV_EXIT1
EMS_CHECK_MOV_ERR3:
                MOV     AH,93H                
		JMP	SHORT EMS_CHECK_MOV_SET_ERR
EMS_CHECK_MOV ENDP
EMS_25		PROC	NEAR
		ASSUME  CS:CODE,DS:CODE,ES:NOTHING
                CMP     AL,1                   
		JB	EMS_25_S1
		JE 	EMS_25_EXIT
                MOV     AH,8FH                 
		JMP	SHORT EMS_25_EXIT1
EMS_25_S1:	MOV	AX,WINDOW_SEG
		XOR	CX,CX
EMS_25_L1:      MOV     ES:[DI],AX              
                MOV     ES:2[DI],CX         
                ADD     AX,1024               
		ADD	DI,4
                INC     CX                
		CMP	CX,3
		JLE	EMS_25_L1
EMS_25_EXIT:    MOV     WORD PTR SS:[BP-2],4   
                XOR     AX,AX                   
EMS_25_EXIT1:	RET
EMS_25		ENDP
EMS_26		PROC	NEAR
		ASSUME  CS:CODE,DS:CODE,ES:NOTHING
                CMP     OS_ENABLED,1           
                JNE     EMS_26_ERR1             
                CMP     AL,1                   
                JB      EMS_26_0                
                JA      EMS_26_ERR2           
EMS_26_1:       CALL    EMS_03                 
                JMP     SHORT EMS_26_EXIT    
EMS_26_0:	XOR	CX,CX
                MOV     WORD PTR ES:[DI],1024  
                MOV     ES:2[DI],CX           
                CALL    EMS_15_3               
		MOV	ES:4[DI],AX
                MOV     ES:6[DI],CX       
                MOV     ES:8[DI],CX         
                XOR     AX,AX                
EMS_26_EXIT:	RET
EMS_26_ERR1:    MOV     AH,0A4H                
                JMP     SHORT EMS_26_EXIT      
EMS_26_ERR2:    MOV     AH,8FH                  
		JMP	SHORT EMS_26_EXIT
EMS_26		ENDP
EMS_27		PROC	NEAR
		ASSUME  DS:CODE,ES:NOTHING
                PUSH    BX                     
                CMP     BX,TOTAL_PAGES          
                JLE     EMS_27_S1              
                MOV     AH,87H                 
		JMP	SHORT EMS_27_EXIT
EMS_27_S1:      PUSH    BX                     
                XOR     DX,DX                  
		DEC	DX
		CLC
                CALL    EMS_13               
		POP	CX
                CMP     BX,CX                   
		JL	EMS_27_ERR1
                MOV     DI,HANDLE_ARRAY        
                XOR     AX,AX                  
                MOV     DX,AX              
EMS_27_L1:      CMP     AL,[DI]                 
                JE      EMS_27_S2              
                ADD     DI,9                   
		INC	DX
                CMP     DX,TOTAL_HANDLES      
		JB	EMS_27_L1
                MOV     AH,85H                  
		JMP	SHORT EMS_27_EXIT
EMS_27_S2:      DEC     BYTE PTR [DI]          
		MOV	BX,CX
                CALL    EMS_ASSIGN              
EMS_27_EXIT:    POP     BX                     
		RET
EMS_27_ERR1:	MOV	AH,88H
		JMP	SHORT EMS_27_EXIT
EMS_27		ENDP
EMS_28_TBL      DB      8              
                DW      OFFSET EMS_28_0 
		DW	OFFSET EMS_28_1
		DW	OFFSET EMS_28_2
		DW	OFFSET EMS_28_3
		DW	OFFSET EMS_28_4
		DW	OFFSET EMS_28_3
		DW	OFFSET EMS_28_6
		DW	OFFSET EMS_28_6
		DW	OFFSET EMS_28_6
EMS_28		PROC	NEAR
		ASSUME  CS:CODE,DS:CODE,ES:NOTHING
                CMP     OS_ENABLED,1            
		JNE	EMS_28_ERR1
		MOV	DI,OFFSET EMS_28_TBL
		CALL	EMS_DISPATCHER
EMS_28_EXIT:	RET
EMS_28_ERR1:    MOV     AH,0A4H               
                JMP     SHORT EMS_28_EXIT      
EMS_28		ENDP
EMS_28_0	PROC	NEAR
		PUSH	DX
                MOV     ES,ALT_MAP_PTRS       
		MOV	DI,ALT_MAP_PTRO
		MOV	SS:[BP-10],ES
                OR      DI,DI                  
		JNE	EMS_28_0_S1
                MOV     CX,ES                 
                OR      CX,CX                  
                XOR     AX,AX                  
		JE	EMS_28_0_EXIT
EMS_28_0_S1:    PUSH    DI                      
                CALL    EMS_15_0             
                POP     DI                     
EMS_28_0_EXIT:  XOR     BL,BL                
                POP     DX                     
                RET                           
EMS_28_0	ENDP
EMS_28_1	PROC	NEAR
		PUSH	BX
		PUSH	DX
                OR      BL,BL                   
		JNE	EMS_28_1_ERR
                MOV     ALT_MAP_PTRS,ES         
		MOV	ALT_MAP_PTRO,DI
                OR      DI,DI                   
		JNE	EMS_28_1_S1
                MOV     CX,ES                  
                OR      CX,CX                  
		JE	EMS_28_1_EXIT
EMS_28_1_S1:	PUSH	DS
                MOV     SI,ES                   
                MOV     DS,SI                  
		ASSUME  DS:NOTHING
                MOV     SI,DI                  
                CALL    EMS_15_1             
		POP	DS
		ASSUME  DS:CODE
EMS_28_1_EXIT:	POP	DX
		POP	BX
		RET
EMS_28_1_ERR:   MOV     AH,9CH                 
                JMP     SHORT EMS_28_EXIT       
EMS_28_1	ENDP
EMS_28_2	PROC	NEAR
                CALL    EMS_15_3                
		MOV	DX,AX
		RET
EMS_28_2	ENDP
EMS_28_3	PROC	NEAR
                XOR     BL,BL                 
		XOR	AX,AX
		RET
EMS_28_3	ENDP
EMS_28_4	PROC	NEAR
                OR      BL,BL                   
                JNE     EMS_28_4_ERR1         
                XOR     AX,AX                 
                MOV     ALT_MAP_PTRS,AX        
		MOV	ALT_MAP_PTRO,AX
EMS_28_4_EXIT:	RET
EMS_28_4_ERR1:  MOV     AH,9CH                  
                JMP     SHORT EMS_28_4_EXIT   
EMS_28_4	ENDP
EMS_28_6	PROC	NEAR
                OR      BL,BL                  
                JNE     EMS_28_6_ERR1          
                XOR     AX,AX                  
EMS_28_6_EXIT:	RET
EMS_28_6_ERR1:  MOV     AH,9CH                 
                JMP     SHORT EMS_28_6_EXIT     
EMS_28_6	ENDP
EMS_30		PROC	NEAR
		PUSH	DX
		CMP	OS_PASS_LOW,0
		JNE	EMS_30_S1
		CMP	OS_PASS_HIGH,0
                JE      EMS_30_S2             
EMS_30_S1:      CMP     OS_PASS_LOW,BX         
		JNE	EMS_30_ERR1
		CMP	OS_PASS_HIGH,CX
		JNE	EMS_30_ERR1
		JMP	SHORT EMS_30_S3
EMS_30_S2:      PUSH    AX                      
                XOR     AH,AH                  
		INT	1AH
                MOV     AX,DX                 
                INC     CX                    
                MUL     CX                      
		MOV	OS_PASS_LOW,AX
		MOV	OS_PASS_HIGH,DX
                MOV     BX,AX                  
                MOV     SS:[BP-2],DX           
                POP     AX                     
EMS_30_S3:      CMP     AL,2                    
		JNE	EMS_30_S4
                XOR     AX,AX                   
                MOV     OS_PASS_LOW,AX        
		MOV	OS_PASS_HIGH,AX
                MOV     OS_ENABLED,1            
		JMP	SHORT EMS_30_EXIT
EMS_30_S4:      CMP     AL,1                 
		JNE	EMS_30_S5
                MOV     OS_ENABLED,0          
		JMP	SHORT EMS_30_EXIT
EMS_30_S5:      OR      AL,AL            
		JNE	EMS_30_ERR2
                MOV     OS_ENABLED,1       
EMS_30_EXIT:	XOR	AX,AX
EMS_30_EXIT1:	POP	DX
		RET
EMS_30_ERR1:    MOV     AH,0A4H                 
		JMP	SHORT EMS_30_EXIT1
EMS_30_ERR2:    MOV     AH,8FH                  
		JMP	SHORT EMS_30_EXIT1
EMS_30		ENDP
EMS_UNSP	PROC	NEAR
		ASSUME  CS:CODE,DS:CODE,ES:NOTHING
                MOV     AH,91H                  
		RET
EMS_UNSP	ENDP
EMS_SEG2LOG	PROC	NEAR
		ASSUME  CS:CODE,DS:NOTHING,ES:NOTHING
		PUSH	CX
		PUSH	DX
                XOR     BX,BX                 
		MOV	DX,CS:WINDOW_SEG
		MOV	CX,4
EMS_SEG2_L1:	CMP	AX,DX
		JE	EMS_SEG2_FND
		ADD	DX,400H
		INC	BX
		LOOP	EMS_SEG2_L1
                MOV     AH,8BH                 
		STC
		JMP	SHORT EMS_SEG2_EXIT
EMS_SEG2_FND:	CLC
EMS_SEG2_EXIT:	POP	DX
		POP	CX
		RET
EMS_SEG2LOG	ENDP
EMS_LOG2PHY	PROC	NEAR
		ASSUME  CS:CODE,DS:CODE,ES:NOTHING
		PUSH	DI
                MOV     DI,PAG_OWNER_TBL       
                MOV     CX,TOTAL_PAGES        
EMS_LOG2P_L1:   CMP     BYTE PTR [DI],DL       
		JNE	EMS_LOG2P_S1
                CMP     WORD PTR [DI+1],BX     
                JE      EMS_LOG2P_S2           
EMS_LOG2P_S1:	ADD	DI,3
		LOOP	SHORT EMS_LOG2P_L1
                MOV     AH,8AH         
		STC
		JMP	SHORT EMS_LOG2PHY_EXIT
EMS_LOG2P_S2:   SUB     DI,PAG_OWNER_TBL        
		MOV	AX,DI
                XOR     DX,DX                  
                MOV     CX,3                    
		DIV	CX
                MOV     CX,16384          
                MUL     CX                   
                ADD     AX,EXTEND_ADRL         
		ADC	DL,EXTEND_ADRH
		XOR	DH,DH
                CLC                        
EMS_LOG2PHY_EXIT:
		POP	DI
		RET
EMS_LOG2PHY	ENDP
EMS_ASSIGN	PROC	NEAR
		ASSUME  CS:CODE,DS:CODE,ES:NOTHING
                CMP     AX,TOTAL_PAGES          
		JG	EMS_ASSIGN_ERR2
                PUSH    DX                    
                PUSH    BX                     
                MOV     DX,0FFFFH          
                CLC                            
                CALL    EMS_13               
                POP     AX                    
                POP     DX                   
                CMP     BX,AX                
                JL      EMS_ASSIGN_ERR1         
                PUSH    AX                    
                CLC                             
                CALL    EMS_13                
                POP     AX                      
                MOV     DI,PAG_OWNER_TBL      
                MOV     CX,TOTAL_PAGES        
                INC     CX                    
EMS_ASSIGN_L1:  OR      AX,AX                 
                JE      EMS_ASSIGN_S2           
                CMP     BYTE PTR [DI],0FFH      
                JNE     EMS_ASSIGN_S1          
                MOV     [DI],DL                 
                MOV     [DI+1],BX              
                INC     BX                     
                DEC     AX                    
EMS_ASSIGN_S1:  ADD     DI,3                   
		LOOP	EMS_ASSIGN_L1
EMS_ASSIGN_ERR1:
                MOV     AH,88H                 
                STC                             
		JMP	SHORT EMS_ASSIGN_EXIT
EMS_ASSIGN_ERR2:
                MOV     AH,87H               
                STC                             
		JMP	SHORT EMS_ASSIGN_EXIT
EMS_ASSIGN_S2:	CLC
                XOR     AX,AX               
EMS_ASSIGN_EXIT:
		RET
EMS_ASSIGN	ENDP
EMS_DEALLOC	PROC	NEAR
		ASSUME  CS:CODE,DS:CODE,ES:NOTHING
                CMP     BL,0FFH                
		JNE	EMS_DEALLOC_S0
		XOR	BX,BX
		JMP	SHORT EMS_DEALLOC_S1
EMS_DEALLOC_S0:	XOR	BH,BH
                PUSH    BX                   
		CLC
                CALL    EMS_13                
                POP     AX                    
                SUB     BX,AX                  
EMS_DEALLOC_S1: MOV     DI,PAG_OWNER_TBL      
		MOV	CX,TOTAL_PAGES
EMS_DEALLOC_L1: CMP     [DI],DL                 
                JNE     EMS_DEALLOC_S2         
                CMP     [DI+1],BX              
                JB      EMS_DEALLOC_S2         
                MOV     BYTE PTR [DI],0FFH      
		MOV	WORD PTR [DI+1],0FFFFH
EMS_DEALLOC_S2: ADD     DI,3                 
		LOOP	EMS_DEALLOC_L1
EMS_DEALLOC_EXIT:
                XOR     AH,AH                   
		RET
EMS_DEALLOC	ENDP
EMS_DISPATCHER	PROC	NEAR
		ASSUME  CS:CODE,DS:NOTHING,ES:NOTHING
                CMP     AL,CS:[DI]                      
                JA      EMS_DIS_ERR1          
                PUSH    BX                     
                MOV     BL,AL                 
                XOR     BH,BH                 
		SHL	BX,1
                ADD     BX,DI                  
                INC     BX                     
                MOV     DI,BX                
                POP     BX                     
                PUSH    CS:[DI]                
                MOV     DI,SS:[BP-4]            
                RETN                          
EMS_DIS_ERR1:   MOV     AH,8FH                 
                STC                          
		RET
EMS_DISPATCHER	ENDP
EMS_CHECK_HDL	PROC	NEAR
		ASSUME  CS:CODE,DS:CODE,ES:NOTHING
		PUSH	DX
		PUSH	BX
                OR      DH,DH                   
		JNE	EMS_CHECK_ERROR
                XCHG    AX,DX                  
                MOV     AH,9                   
                MUL     AH                  
                MOV     BX,AX                  
                ADD     BX,HANDLE_ARRAY        
                MOV     AX,DX                   
                CMP     BYTE PTR [BX],0        
		JE	EMS_CHECK_ERROR
                CLC                             
EMS_CHECK_EXIT:	POP	BX
		POP	DX
		RET
EMS_CHECK_ERROR:
                MOV     AH,83H                 
                STC                             
		JMP	SHORT EMS_CHECK_EXIT
EMS_CHECK_HDL	ENDP
EMS_EXCH_PAG 	PROC	NEAR
		ASSUME  CS:CODE,DS:NOTHING,ES:NOTHING
		PUSH	CX
		PUSH	SI
		PUSH	DS
		PUSH	ES
                PUSH    CS                     
		POP	DS
		ASSUME 	DS:CODE,ES:CODE
                PUSH    DEST.BASE_ADRL         
                PUSH    SOURCE.BASE_ADRL       
 		MOV	CH,DEST.BASE_ADRH
 		MOV	CL,SOURCE.BASE_ADRH
 		PUSH	CX
                MOV     DI,OFFSET WINDOW_ADDR_BASE     
                MOV     SI,MAP_ARRAY_PTR       
		INC	SI
		INC	SI
                XOR     BH,BH                   
                PUSH    BX                     
                MOV     BL,MOVE_BUSY_FLAG       
                OR      BL,BL                   
                JE      EMS_EXCH_NOT_BUSY       
                DEC     BL                      
                PUSH    AX                      
		PUSH	DX
                MOV     AX,[DI+BX]              
                MOV     DL,[DI+BX+2]           
                MOV     CX,SAVED_ADDR_LOW     
                MOV     DH,SAVED_ADDR_HIGH      
                CALL    EMS_MOVE_DATA          
                MOV     MOVE_BUSY_FLAG,0       
                POP     DX                   
		POP	AX
EMS_EXCH_NOT_BUSY:
                POP     BX                      
                SAL     BX,1                   
                SAL     BX,1                   
		MOV	DH,1
                CMP     AX,[SI+BX]            
                JNE     EMS_EXCH_S1             
                CMP     DX,[SI+BX+2]           
                JE      EMS_EXCH_EXIT           
EMS_EXCH_S1:    PUSH    AX                     
		PUSH	DX
                MOV     AX,[SI+BX]              
                MOV     DX,[SI+BX+2]           
                OR      DH,DH                   
                JE      LOAD_PAGE            
                XOR     DH,DH                 
                CALL    EMS_CHK_LOCAL         
		JC 	LOAD_PAGE
STORE_PAGE:     MOV     CX,[DI+BX]           
                MOV     DH,[DI+BX+2]          
           	CALL	EMS_MOVE_DATA
LOAD_PAGE:      POP     DX                      
		POP	AX
                MOV     DH,1                   
                CALL    EMS_CHK_LOCAL           
                MOV     CX,BX                 
                INC     CX                      
                MOV     MOVE_BUSY_FLAG,CL       
                MOV     DH,1                   
                MOV     [SI+BX+2],DX           
                MOV     [SI+BX],AX              
                MOV     SAVED_ADDR_LOW,AX       
                MOV     SAVED_ADDR_HIGH,DL      
                JC      EMS_EXCH_EXIT
                MOV     CX,[DI+BX]              
		MOV	DH,[DI+BX+2]
                XCHG    AX,CX                   
                XCHG    DL,DH                   
                CALL    EMS_MOVE_DATA           
EMS_EXCH_EXIT:  MOV     MOVE_BUSY_FLAG,0        
                POP     CX                      
  		MOV	SOURCE.BASE_ADRH,CL
  		MOV	DEST.BASE_ADRH,CH
 		POP 	SOURCE.BASE_ADRL
              	POP 	DEST.BASE_ADRL
                POP     ES                     
		POP	DS
		POP	SI
		POP	CX
		RET
EMS_EXCH_PAG 	ENDP
EMS_CHK_LOCAL	PROC	NEAR
		PUSH	AX
		PUSH	BX
		PUSH	DX
		PUSH	DI
                OR      AX,AX               
                JNE     EMS_CHK_LOC0          
                OR      DL,DL                 
                JE      EMS_CHK_LOC5           
EMS_CHK_LOC0:   MOV     DI,BX                  
                XOR     BX,BX                  
                MOV     CX,4                  
EMS_CHK_LOC1:   CMP     AX,[SI+BX]             
                JNE     EMS_CHK_LOC2        
                CMP     DX,[SI+BX+2]            
                JE      EMS_CHK_LOC3           
EMS_CHK_LOC2:   ADD     BX,4                   
		LOOP	EMS_CHK_LOC1
                JMP     SHORT EMS_CHK_LOC6     
EMS_CHK_LOC3:   CMP     DI,BX                   
                JE      EMS_CHK_LOC5            
                OR      DH,DH                   
		JE	EMS_CHK_LOC4
		XCHG	DI,BX
EMS_CHK_LOC4:   MOV     BYTE PTR [SI+BX+3],1   
		XCHG	DI,BX
                MOV     BYTE PTR [SI+BX+3],0    
                MOV     CL,12
                SAL     BX,CL                   
		SAL	DI,CL
                PUSH    SI                      
                MOV     SI,BX                  
		PUSH	DS
		MOV 	AX,WINDOW_SEG
                MOV     DS,AX                   
                MOV     ES,AX                
                MOV     CX,8192                
                CLD                         
                REP     MOVSW                
		POP	DS
		POP	SI
EMS_CHK_LOC5:   STC                         
EMS_CHK_LOC_EXIT:
		POP	DI
		POP	DX
		POP	BX
		POP	AX
		RET
EMS_CHK_LOC6:   CLC                             
		JMP	SHORT EMS_CHK_LOC_EXIT
EMS_CHK_LOCAL	ENDP
EMS_MOVE_DATA	PROC	NEAR
		PUSH	CX
		PUSH	SI
		MOV	SI,CS
		MOV	ES,SI
                MOV     SI,OFFSET GDT           
                MOV     DEST.BASE_ADRL,AX       
		MOV	DEST.BASE_ADRH,DL
                MOV     SOURCE.BASE_ADRL,CX    
		MOV	SOURCE.BASE_ADRH,DH
                MOV     AH,87H                 
                MOV     CX,2000H               
                INT     15H                     
		POP	SI
		POP	CX
		RET
EMS_MOVE_DATA	ENDP
INITIALIZE1	PROC	NEAR
INIT1:		ASSUME	CS:CODE,DS:CODE,ES:CODE
		MOV	DI,PAG_OWNER_TBL
                MOV     AX,TOTAL_PAGES         
                MOV     DX,3                   
		MUL	DX
		MOV	CX,AX
                XOR     AX,AX                 
                DEC     AX                     
		REP	STOSB
                MOV     AX,TOTAL_HANDLES       
                MOV     AH,9                    
		MUL	AH
		MOV	CX,AX
		XOR	AX,AX
		REP	STOSW
                MOV     DI,HANDLE_ARRAY        
		DEC	BYTE PTR [DI]
		MOV	DI,MAP_ARRAY_PTR
                MOV     CX,INT_SAVE_SIZE        
                INC     CX                      
EMS_INIT_L2:
                MOV     WORD PTR [DI],0FFFFH   
                INC     DI                     
		INC	DI
		PUSH	CX
                MOV     CX,8                   
		REP	STOSW
		POP	CX
		LOOP	EMS_INIT_L2
		RET
INITIALIZE1	ENDP
DATA_START	=	$
VDISK_HEADER 	DB	'VDISK  V'
ERRMSG          DB      'No hay memoria',13,10,'$'
INITIALIZE	PROC	NEAR
		ASSUME  CS:CODE,DS:CODE,ES:NOTHING
                PUSH    CS                      
		POP	ES
                LDS     SI,[REQ_HEADADR]      
		ASSUME	DS:NOTHING,ES:CODE
                LDS     SI,DS:[SI.CONFIG_PTR]   
                MOV     BH,1               
                MOV     CX,80                  
                CALL    SCAN_FOR              
                OR      AH,AH                  
                JNE     CHK_FOR_VDISK         
                XOR     BH,BH                   
                CALL    SCAN_FOR              
                OR      AH,AH                  
                JNE     CHK_FOR_VDISK         
                DEC     SI                      
                MOV     CX,5                 
                XOR     AX,AX                  
                MOV     DI,10                 
INIT_LOOP1:     MOV     BL,DS:[SI]             
                INC     SI                      
                SUB     BL,'0'                 
                JB      INIT_SKIP1             
		CMP	BL,9
		JA      INIT_SKIP1
		XOR	BH,BH
                MUL     DI                     
                ADD     AX,BX                  
		LOOP	INIT_LOOP1
INIT_SKIP1:     XOR     DX,DX             
                MOV     DI,16                 
                DIV     DI                   
                OR      AX,AX                 
                JE      CHK_FOR_VDISK         
                MOV     CS:TOTAL_PAGES,AX      
CHK_FOR_VDISK:  PUSH    CS                   
		POP	DS
		ASSUME 	DS:CODE
                MOV     AX,3519H              
                INT     21H                  
                MOV     DI,12H                
		MOV	SI,OFFSET VDISK_HEADER
                MOV     CX,8                  
                XOR     BX,BX                  
                REPE    CMPSB                  
		JNE 	CHK_EXT_MEM
                MOV     SI,2CH              
                MOV     AX,ES:[SI]             
                MOV     DL,ES:[SI+2]           
                SUB     DL,10H                 
		XOR	DH,DH
                MOV     BX,16384               
		DIV	BX
                MOV     BX,AX                  
CHK_EXT_MEM:    PUSH    CS                 
		POP	ES
		ASSUME	ES:CODE
                MOV     AH,88H                
                CLC                           
                INT     15H                   
                JC      EXTEND_ERROR          
                MOV     CL,4                   
                SAR     AX,CL                 
                SUB     AX,TOTAL_PAGES         
		JL	EXTEND_ERROR
                CMP     AX,BX               
		JGE	EXTEND_MEM_OK1
EXTEND_ERROR:   JMP     DISP_ERR               
EXTEND_MEM_OK1: PUSH    CS                    
                POP     ES                     
                ASSUME  ES:CODE                
                MOV     CL,4                   
                SAL     AX,CL               
		MOV	EXT_MEM_LIMIT,AX
                MOV     CX,1024                
                MUL     CX                    
                ADD     DL,10H                 
                MOV     EXTEND_ADRL,AX      
                MOV     EXTEND_ADRH,DL         
                MOV     CX,OFFSET DATA_START  
                MOV     PAG_OWNER_TBL,CX      
                MOV     AX,TOTAL_PAGES        
                MOV     AH,3                  
                MUL     AH                    
                ADD     CX,AX                  
                MOV     HANDLE_ARRAY,CX        
                MOV     AX,TOTAL_HANDLES       
                MOV     AH,9                   
		MUL	AH
		ADD	CX,AX
                MOV     MAP_ARRAY_PTR,CX        
                MOV     AX,INT_SAVE_SIZE      
                INC     AX                     
                MOV     AH,18                  
                MUL     AH                 
                ADD     AX,CX                
                ADD     AX,15            
                MOV     CL,4                 
                SHR     AX,CL                
                MOV     BX,CS              
                ADD     AX,BX                  
                MOV     WINDOW_SEG,AX         
                PUSH    AX                   
                MOV     DX,16                  
                MUL     DX                   
		MOV	DI,OFFSET WINDOW_ADDR_BASE
		MOV	CX,4
EMS_INIT_L3:    MOV     [DI],AX                
		MOV	[DI+2],DX
                ADD     AX,16384             
		ADC	DX,0
                ADD     DI,4                    
		LOOP	EMS_INIT_L3
                POP     BX               
                ADD     BX,1000H              
                XOR     AX,AX                
		PUSH	ES
                CALL    LOAD_HEADER            
                POP     ES                     
                MOV     DX,OFFSET PROGRAM     
		MOV	AH,9
		INT	21H
		PUSH	ES
                MOV     AX,3515H               
		INT	21H
                MOV     OLD_INT15HO,BX         
		MOV	OLD_INT15HS,ES
		POP	ES
                MOV     AX,2515H               
		MOV	DX,OFFSET INT_15H
                INT     21H                   
                MOV     AX,2567H               
		MOV	DX,OFFSET INT_67H
                INT     21H                   
                JMP     INIT1                   
DISP_ERR:       MOV     DX,OFFSET ERRMSG       
                MOV     AH,9                   
		INT	21H
                MOV     AX,3000H                
		INT	21H
                XCHG    AL,AH                  
                CMP     AX,31EH                 
                MOV     AX,OFFSET DRIVER_END    
                MOV     BX,CS                   
                JB      ERROR_ABORT_SKIP        
                XOR     AX,AX                  
ERROR_ABORT_SKIP:
                CALL    LOAD_HEADER             
                MOV     AX,8002                
                RET                            
LOAD_HEADER	PROC	NEAR
                LES     DI,[REQ_HEADADR]              
                MOV     WORD PTR ES:[DI.ADDRESS],AX     
                MOV     WORD PTR ES:[DI.ADDRESS+2],BX  
		RET
LOAD_HEADER	ENDP
SCAN_FOR	PROC	NEAR
                XOR     AH,AH                   
SCAN_LOOP1:     LODSB                          
                OR      BH,BH                
                JE      SCAN_SKIP1            
                CMP     AL,20H                
		JLE	SCAN_SKIP4
		JMP	SHORT SCAN_SKIP2
SCAN_SKIP1:     CMP     AL,20H                  
		JG 	SCAN_SKIP4
SCAN_SKIP2:     CMP     AL,13             
		JE	SCAN_SKIP3
                CMP     AL,10                 
		JE	SCAN_SKIP3
		LOOP	SCAN_LOOP1
SCAN_SKIP3:     INC     AH                    
SCAN_SKIP4:	RET
SCAN_FOR	ENDP
END_OF_CODE	=	$
INITIALIZE	ENDP
CODE		ENDS
		END
