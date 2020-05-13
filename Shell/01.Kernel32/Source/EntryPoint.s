[ORG 0x00]          
[BITS 16]           

SECTION .text

HASH1:	dw 0x0000
HASH2: 	dw 0x0000

START: 
	mov ax, 0x1000
	mov ds, ax      
    	mov es, ax

	mov ax, 0x0000
	mov ss, ax
	mov sp, 0xFFFE
	mov bp, 0xFFFE

	
	xor ax, ax

	lea bx, [START]

HASHING:
	mov dx, word [bx]
	xor ax, dx
		
	cmp bx, 0x07FC
	je FIRSTHASH
	
	add bx, 0x0004
	

	jmp HASHING

FIRSTHASH:
	mov cx, ax
	cmp word [HASH1], ax
	

	mov ax, 0x0000
	lea bx, [START+0x2]

	je SECONDHASHING
	jmp FAILCHECK
SECONDHASHING:
	mov dx, word [bx]
	xor ax, dx
	
	cmp bx, 0x07FE	
	je SECONDHASH
	
	add bx, 0x0004
	
	
	jmp SECONDHASHING

	
SECONDHASH:
	
	cmp word [HASH2], ax
	je READGDT
	jmp FAILCHECK

FAILCHECK:
	

	push FAIL
	push 3
	push 15
	call REALPRINTMESSAGE
	add sp, 6

	jmp GDTEND

READGDT:	
	push OK	
	push 3
	push 15
	call REALPRINTMESSAGE
	add sp, 6
	
	cli

	lgdt [ GDTR ]


	mov eax, 0x4001003B
	mov cr0, eax
	
		

	jmp dword 0x18: ( PROTECTEMODE - $$ + 0x10000 )
	
REALPRINTMESSAGE:
	push bp
	mov bp, sp
	
	push es
	push si
	push di
	push ax
	push cx
	push dx
	
	mov ax, 0xB800
	
	mov es, ax
	
	mov ax, word [ bp + 6 ]
	mov si, 160
	mul si
	add di, ax	
	
	mov ax, word [ bp + 4 ]
	mov si, 2
	mul si
	add di, ax

	mov si, word [ bp + 8 ]


.MESSAGELOOP:
    	mov cl, byte [ si ]
    	cmp cl, 0
    	je .MESSAGEEND

    	mov byte [ es: di ], cl
    	add si, 1
    	add di, 2

    	jmp .MESSAGELOOP

.MESSAGEEND:

	pop dx
	pop cx
	pop ax
	pop di
	pop si
	pop es
	pop bp
	
	ret


[BITS 32]
PROTECTEMODE:
	mov ax, 0x20
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

	mov ss, ax
	mov esp, 0xFFFE
	mov ebp, 0xFFFE
	
	push ( SWITCHSUCCESSMESSAGE - $$ + 0x10000)
	push 4
	push 0
	call PRINTMESSAGE
	add esp, 12
	
	jmp dword 0x18: 0x10200

PRINTMESSAGE:
	push ebp
	mov ebp, esp
	push esi
	push edi
	push eax
	push ecx
	push edx

	mov eax, dword [ ebp + 12 ]
	mov esi, 160
	mul esi
	mov edi, eax

	mov eax, dword [ ebp + 8 ]
	mov esi, 2
	mul esi
	add edi, eax

	mov esi, dword [ ebp + 16 ]
	
.MESSAGELOOP:
	mov cl, byte [ esi ]
    	cmp cl, 0
    	je .MESSAGEEND

    	mov byte [ edi + 0xB8000 ], cl
    	add esi, 1
    	add edi, 2

    	jmp .MESSAGELOOP

.MESSAGEEND:

	pop edx
	pop ecx
	pop eax
	pop edi
	pop esi
	pop ebp
	
	ret 


align 8, db 0

dw 0x0000

GDTR:
	dw GDTEND - GDT - 1 
	dd ( GDT - $$ + 0x10000 )


GDT:
	NULLDescriptor:
		dw 0x0000
		dw 0x0000
		db 0x00
		db 0x00
		db 0x00
		db 0x00

	IA_32eCODEDESCRIPTOR:
		dw 0xFFFF
		dw 0x0000       
        	db 0x00         
        	db 0x9A         	
	        db 0xAF         
	        db 0x00         
        
	IA_32eDATADESCRIPTOR:
	        dw 0xFFFF       
        	dw 0x0000       
        	db 0x00         
        	db 0x92        
        	db 0xAF         
        	db 0x00    

	CODEDESCRIPTOR:
		dw 0xFFFF
		dw 0x0000
		db 0x00
		db 0x9A
		db 0xCF
		db 0x00

	DATADESCRIPTOR:
		dw 0xFFFF
		dw 0x0000
		db 0x00
		db 0x92
		db 0xCF
		db 0x00

GDTEND:




SWITCHSUCCESSMESSAGE:
	db 'Switch to Protected Mode Succes', 0
OK:	db 'OK!!!',0
FAIL: 	db 'fail....',0
CHECKHASH:	db '1111 != ', 0
REALHASH:	db '1111 error', 0

times 512 - ( $ - $$ ) db 0x00 


