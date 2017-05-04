$NOLIST
$nomod51
$INCLUDE (c:/reg832.pdf)
$LIST

spi_counter	equ	0037h;

stack_init	equ	07fh				;beginadres programma
send1		equ	0030h				;8 bit doorsturen/ontvangen
send2		equ	0031h				
receive1	equ	0032h
receive2	equ	0033h
fase		equ	0034h

uart_in1	equ	0045h			;uart buff in
uart_in2	equ	0046h			
uart_in3	equ	0047h			
uart_in4	equ	0048h			
uart_in_counter	equ	0049h			;uart buff current size
uart_out_counter equ	0050h

sending		bit	0			;aan het zenden=1 of aan het ontvangen=0
laatste_byte	bit	1			;wordt gezet wanneer 1e byte gezonden ontvangen => dan weten we dat er nog een byte gezonden/ontvangen moet worden.
uart_flag	bit	2
uart_in_mode	bit	3			; ':' start commando => mode = 1 (CR eindigd,mode = 0)
string		bit	4
error		bit	5
byte1		bit	6
versturen	bit	7
sampled		bit	8



		org	0000h	
		ljmp	begin
		org	0023h				;UART interrupt
		ljmp	uart_int
		
;/**
; * FUNCTION_PURPOSE: This file set up spi in master mode with 
; * p2.4 clock	=> actief hoog!
; * P2.6 Slave Select	=> actief laag!
; * P2.7(MOSI) serial output	=> actief hoog!
; */		
		
begin:		mov	pllcon,#0b			;zet 0 in dpllcon => clockf = 16.77MHz
		mov	sp,#stack_init			;stackpointer laten wijzen naar beginadres programma
		
		clr	byte1				;0e byte als = 0 of 1e byte als = 1
		mov	spi_counter,#00h		;welke bit in de byte

		SETB 	ea				;enable all interrupts
		setb	et0
		setb	tr0
		
		clr	p2.4
		setb	p2.6
		clr	p2.7
		
		lcall	init_uart

main:		jb	uart_flag,controlleer
		sjmp	main


;uart

controlleer:	clr	uart_flag
		mov	a,fase
		jz	init
		dec	a
		jz	sync
		dec	a
		jz	send
		dec	a
		jz	runp



init:		;uit shutdown mode
		mov	r0,#00001100b
		mov	r1,#00000001b
		ljmp	verzend
		
sync:		;scan limit inladen
		mov	r0,#00001011b
		mov	r1,#00000111b
		ljmp	verzend
		
send:		;intensity aanpassen
		mov	r0,#00001010b
		mov	r1,#00001111b
		ljmp	verzend
		
runp:		;leds aanpassen aanpassen
		mov	r0,#00000010b
		mov	r1,#00001111b
		ljmp	verzend
		

;spi

verzend:	clr	p2.6

verzend_spi:	NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		jb	byte1,spi_byte1
		mov	22h,r0
		sjmp	start_spi
		
		
spi_byte1:	mov	22h,r1

start_spi:	mov	a,spi_counter
		jz	bit0
		dec	a
		jz	bit1
		dec	a
		jz	bit2
		dec	a
		jz	bit32
		dec	a
		jz	bit42
		dec	a
		jz	bit52
		dec	a
		jz	bit62
		dec	a
		jz	bit72
		
bit32:		ljmp	bit3
bit42:		ljmp	bit4
bit52:		ljmp	bit5		
bit62:		ljmp	bit6
bit72:		ljmp	bit7

bit0:		MOV	c,22h.0
		mov	p2.7,c
		inc	spi_counter
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		setb	p2.4
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		clr	p2.4
		ljmp	verzend_spi
		
bit1:		MOV	c,22h.1
		mov	p2.7,c
		inc	spi_counter
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		setb	p2.4
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		clr	p2.4
		ljmp	verzend_spi
		
bit2:		MOV	c,22h.2
		mov	p2.7,c
		inc	spi_counter
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		setb	p2.4
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		clr	p2.4
		ljmp	verzend_spi
		
bit3:		MOV	c,22h.3
		mov	p2.7,c
		inc	spi_counter
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		setb	p2.4
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		clr	p2.4
		ljmp	verzend_spi
		
bit4:		MOV	c,22h.4
		mov	p2.7,c
		inc	spi_counter
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		setb	p2.4
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		clr	p2.4
		ljmp	verzend_spi
				
bit5:		MOV	c,22h.5
		mov	p2.7,c
		inc	spi_counter
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		setb	p2.4
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		clr	p2.4
		ljmp	verzend_spi
		
bit6:		MOV	c,22h.6
		mov	p2.7,c
		inc	spi_counter
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		setb	p2.4
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		clr	p2.4
		ljmp	verzend_spi
		
bit7:		MOV	c,22h.7
		mov	p2.7,c
		mov	spi_counter,#0h
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		setb	p2.4
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		clr	p2.4
		jb	byte1,einde_spi
		setb	byte1
		ljmp	verzend_spi


einde_spi:	clr	byte1
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		setb	p2.6
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		ljmp	main


$INCLUDE (c:/aduc800_mideA.inc)
$INCLUDE (UART.inc)

end
