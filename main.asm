$NOLIST
$nomod51
$INCLUDE (c:/reg832.pdf)
$LIST


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


		org	0000h				;start programma
		sjmp	start	
		org	0023h				;UART interrupt
		ljmp	uart_int
		org	003Bh				;SPI interrupt
		ljmp	read_or_write
		
		
		
start:		mov	pllcon,#0b			;zet 0 in dpllcon => clockf = 16.77MHz
		mov	sp,#stack_init			;stackpointer laten wijzen naar beginadres programma
		mov	adccon1,#10001000b		;MCLK Divider = 8
		mov	adccon2,#00000111b		;select channel 7
		
		setb	ea				;enable all Interrupt sources
		lcall	init_uart
		lcall	init_spi
		
		
		
		
main:		jb	uart_flag,controleer
		sjmp	main



controleer:	clr	uart_flag
		clr	c
		mov	a,fase
		subb	a,#1h
		JC	init
		subb	a,#1h
		JC	sync
		subb	a,#1h
		JC	send
		subb	a,#1h
		JC	run



init:		;plaats het tel commando in receive2 en verzend via SPI
		cpl	p2.0
		ljmp	main
		
sync:		;plaats het sync commando in receive2 en verzend via SPI
		cpl	p2.1
		ljmp	main
		                                             
send:		;plaats de lengte van de string in receive2 en verzend via SPI
		;verstuur vervolgens elke byte van de string via SPI (eerst plaatsen in receive2)
		cpl	p2.2
		ljmp	main
		
run:		;plaats het run commando in receive2 en verzend via SPI
		cpl	p2.3
		ljmp	main
		
		
				
$INCLUDE (c:/aduc800_mideA.inc)
$INCLUDE (UART.inc)			;DEZE FILE GEBRUIKT GEHEUGENLOCATIES 0045h TOT EN MET 0049h EN BIT 2+3+4
$INCLUDE (spi.inc)			;DEZE FILE GEBRUIKT GEHEUGENLOCATIES 0030h TOT EN MET 0034h EN BIT 0+1
;$INCLUDE (lettertype.inc)		
end