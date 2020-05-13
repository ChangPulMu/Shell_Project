[ORG 0x00]
[BITS 16]

SECTION .text

jmp 0x07C0:START

TOTALSECTORCOUNT: dw 0x02
KERNEL32SECTORCOUNT: dw 0x02

SECTORNUMBER: 	db 0x02
HEADNUMBER:   	db 0x00
TRACKNUMBER:	db 0x00	
HOUR: 	db 0x00
MIN:	db 0x00
SEC:	db 0x00


START:
    	mov ax, 0x07C0
    	mov ds, ax
   	mov ax, 0xB800
    	mov es, ax
	
	mov ax, 0x0000
	mov ss, ax
	mov sp, 0xFFFE
	mov bp, 0xFFFE

	mov ah, 0x02
	int 0x1A
	
	mov [HOUR], ch
	mov [MIN], cl
	mov [SEC], dh
	
	mov cl, 4     		
	xor ax, ax

	mov ah, [HOUR]
	mov al, ah
	
	shr ah, cl
	and al, 0x0F
	
	or ax, 0x3030
	
	mov [TIME], ah
	mov [TIME+1], al

	xor ax, ax

	mov ah, [MIN]
	mov al, ah
	
	shr ah, cl
	and al, 0x0F
	
	or ax, 0x3030
	
	mov [TIME+3], ah
	mov [TIME+4], al

	xor ax, ax

	mov ah, [SEC]
	mov al, ah
	
	shr ah, cl
	and al, 0x0F
	
	or ax, 0x3030
	
	mov [TIME+6], ah
	mov [TIME+7], al	
	
	mov dl, 0
    	mov si, 0
    
.SCREENCLEARLOOP:
  	mov byte [ es: si ], 0
   	mov byte [ es: si + 1 ], 0x0A

    	add si, 2
    	cmp si, 80 * 25 * 2

    	jl .SCREENCLEARLOOP

	push MESSAGE1
	push 0
	push 0
	call PRINTMESSAGE
	add sp, 6

	push IMAGELOADMESSAGE
	push 2
	push 0
	call PRINTMESSAGE
	add sp, 6

	push TIMEMESSAGE
	push 1
	push 0
	call PRINTMESSAGE
	add sp, 6
	
	push TIME
	push 1
	push 13
	call PRINTMESSAGE
	add sp, 6
	
	push CHECK
	push 3
	push 0
	call PRINTMESSAGE
	add sp, 6
	
RESETDISK:
	mov ax, 0
	mov dl, 0
	int 0x13

	jc HANDLEDISKERROR
		
	mov si, 0x1000

	mov es, si
	mov bx, 0x0000

	mov di, word [ TOTALSECTORCOUNT ]



READDATA:
	cmp di, 0
	je READEND
	sub di, 0x1

	
	mov ah, 0x02
	mov al, 0x1
	mov ch, byte [ TRACKNUMBER ]
	mov cl, byte [ SECTORNUMBER ]	
	mov dh, byte [ HEADNUMBER ]
	mov dl, 0x00
	int 0x13
	jc HANDLEDISKERROR
	

	add si, 0x0020
	mov es, si
	
	mov al, byte [ SECTORNUMBER ] 
	add al, 0x01

	mov byte [ SECTORNUMBER ], al
	cmp al, 19
	jl READDATA

	xor byte [ HEADNUMBER ], 0x01
	mov byte [ SECTORNUMBER ], 0x01

	cmp byte [ HEADNUMBER ], 0x00
	jne READDATA
	
	add byte [ TRACKNUMBER ], 0x01
	jmp READDATA

READEND:
	
	push LOADINGCOMPLETEMESSAGE
	push 2
	push 20
	call PRINTMESSAGE
	add sp, 6
	
	jmp 0x1000:0x04

HANDLEDISKERROR:

	push DISKERRORMESSAGE
	push 1
	push 20
	call PRINTMESSAGE
	
	jmp $
	
PRINTMESSAGE:
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


MESSAGE1:    db 'group7 OS Boot Loader Start~!!', 0

DISKERRORMESSAGE:	db 'disk error!!', 0
IMAGELOADMESSAGE:	db 'image loading....', 0
LOADINGCOMPLETEMESSAGE:	db 'complete!!', 0
CHECK:			db 'check loading...',0

TIME: 	db '00:00:00', 0

TIMEMESSAGE: db 'Current Time: ' ,0
	


	times 510 - ( $ - $$ )    db    0x00

	db 0x55
	db 0xAA	


