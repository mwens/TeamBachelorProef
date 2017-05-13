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
counter_in	equ	0051h
msg_pointer_l	equ	0052h
msg_pointer_h	equ	0053h

sending		bit	0			;aan het zenden=1 of aan het ontvangen=0
laatste_byte	bit	1			;wordt gezet wanneer 1e byte gezonden ontvangen => dan weten we dat er nog een byte
uart_flag	bit	2			; gezonden/ontvangen moet worden.
uart_in_mode	bit	3			; ':' start commando => mode = 1 (CR eindigd,mode = 0)
string		bit	4
sending_msg	bit	5
byte1		bit	6
versturen	bit	7
sampled		bit	8
uart_in_data	bit	9

Interrupt	bit	p2.5
CS		bit	p2.3		;actief laag
Din		bit	p2.2		;MOSI
clk		bit	p2.1
Dout		bit	p2.0		;MISO

		org	0000h	
		ljmp	begin
		org	0023h				;UART interrupt
		ljmp	uart_int
		
begin:		mov	pllcon,#0b			;zet 0 in dpllcon => clockf = 16.77MHz
		mov	sp,#stack_init			;stackpointer laten wijzen naar beginadres programma
		clr	byte1				;0e byte als = 0 of 1e byte als = 1
		mov	spi_counter,#00h		;welke bit in de byte
		
		SETB 	ea				;enable all interrupts
		setb	et0
		setb	tr0
		
		clr	clk
		setb	CS
		clr	Din
		
		mov	r3,#00000000b
		mov	r4,#00000000b
		
		lcall	init_uart

main:		jb	uart_flag,controlleer
		sjmp	main
		;jb	Interrupt,main
		;ljmp	verzend


;verander zenddata

controlleer:	clr	uart_flag
		mov	a,fase
		jz	init
		dec	a
		jz	sync
		dec	a
		jz	send
		dec	a
		jz	runp

;verander zenddata

init:		;write configuration
		mov	r0,#11000100b
		mov	r1,#10001011b
		ljmp	verzend
		
sync:		;read configuration
		mov	r0,#01000000b
		mov	r1,#00000000b
		ljmp	verzend
		
send:		;write data
		mov	r0,#10000011b
		mov	r1,#00001111b
		ljmp	verzend
		
runp:		;read data
		mov	r0,#00000000b
		mov	r1,#00000000b
		ljmp	verzend


;

verzend:	clr	CS

verzend_spi:	lcall	wait
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
		jz	bit3
		dec	a
		jz	bit4
		dec	a
		jz	bit5
		dec	a
		jz	bit6
		dec	a
		jz	bit7

bit0:		clr	clk
		MOV	c,22h.7
		ljmp	clock
		
bit1:		clr	clk
		MOV	c,22h.6
		ljmp	clock
		
bit2:		clr	clk
		MOV	c,22h.5
		ljmp	clock
		
bit3:		clr	clk
		MOV	c,22h.4
		ljmp	clock
		
bit4:		clr	clk
		MOV	c,22h.3
		ljmp	clock
				
bit5:		clr	clk
		MOV	c,22h.2
		ljmp	clock
		
bit6:		clr	clk
		MOV	c,22h.1


clock:		mov	Din,c
		inc	spi_counter
		clr	c
		mov	c,Dout
		mov	a,r3
		rlc	a
		mov	r3,a
		mov	a,r4
		rlc	a
		mov	r4,a
		lcall	wait
		setb	clk
		lcall	wait
		ljmp	verzend_spi
		
bit7:		clr	clk
		MOV	c,22h.0
		mov	Din,c
		mov	spi_counter,#0h
		clr	c
		mov	c,Dout
		mov	a,r3
		rlc	a
		mov	r3,a
		mov	a,r4
		rlc	a
		mov	r4,a
		lcall	wait
		setb	clk
		lcall	wait
		jb	byte1,einde_spi
		setb	byte1
		ljmp	verzend_spi

wait:		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		RET

einde_spi:	clr	byte1
		clr	clk
		lcall	wait
		setb	CS
		lcall	wait
		lcall	uart_in
		ljmp	main


$INCLUDE (c:/aduc800_mideA.inc)
$INCLUDE (uart.inc)
end
