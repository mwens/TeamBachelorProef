$NOLIST
$nomod51
$INCLUDE (c:/reg832.pdf)
$LIST

spi_counter		equ	0037h;

stack_init	equ	07fh				;beginadres programma
send1		equ	0030h				;8 bit doorsturen/ontvangen
send2		equ	0031h				
receive1	equ	0032h
receive2	equ	0033h
fase		equ	0034h

geheugen	equ	0021h
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

		SETB 	ea				;enable all interrupts
		setb	et0
		setb	tr0
		
		setb	p2.4
		setb	p2.6
		clr	p2.7
		
		lcall	init_uart

main:		jb	uart_flag,controlleer
		sjmp	main


;uart

controlleer:	clr	uart_flag
		mov	a,fase
		jz	init
		subb	a,#1h
		jz	sync
		subb	a,#2h
		jz	run



init:		;plaats het tel commando in receive2 en verzend via SPI
		clr	p2.7
		ljmp	clock
		
sync:		;plaats het sync commando in receive2 en verzend via SPI
		setb	p2.7
		ljmp	clock
		
run:		cpl	p2.6
		ljmp	main

clock:		clr	p2.4
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
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
		ljmp	main



$INCLUDE (c:/aduc800_mideA.inc)
$INCLUDE (UART.inc)

end
