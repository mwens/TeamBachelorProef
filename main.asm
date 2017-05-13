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
counter_in	equ	0051h
msg_pointer_l	equ	0052h
msg_pointer_h	equ	0053h
tekst_counter_l equ	0054h
tekst_counter_h equ	0055h
tekst_counter   equ	0056h


sending		bit	0			;aan het zenden=1 of aan het ontvangen=0
laatste_byte	bit	1			;wordt gezet wanneer 1e byte gezonden ontvangen => dan weten we dat er nog een 
uart_flag	bit	2			;byte gezonden/ontvangen moet worden.
uart_in_mode	bit	3			; ':' start commando => mode = 1 (CR eindigd,mode = 0)
string		bit	4
sending_msg	bit	4


		org	0000h				;start programma
		sjmp	start	
		org	000BH				;timer0_int
		ljmp	timer0_int
		org	0023h				;UART interrupt
		ljmp	uart_int
		org	003Bh				;SPI interrupt
		ljmp	read_or_write
		
start:		mov	pllcon,#0b			;zet 0 in dpllcon => clockf = 16.77MHz
		mov	sp,#stack_init			;stackpointer laten wijzen naar beginadres programma
		mov	adccon1,#10001000b		;MCLK Divider = 8
		mov	adccon2,#00000111b		;select channel 7
		
		setb	ea				;enable all Interrupt sources
		setb	et0
		
		orl	tmod,#00000010
		mov	th0,#0FFh
		clr	tr1
		setb	tr0
		
		
		lcall	init_uart
		lcall	init_spi
		
main:		jb	uart_flag,controlleer
		sjmp	main

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
		mov	send1,#00001100b
		mov	send2,#00000001b
		lcall	schrijf_data
		ljmp	main
		
sync:		;scan limit inladen
		mov	send1,#00001011b
		mov	send2,#00000111b
		lcall	schrijf_data
		ljmp	main
		                                             
send:		;intensity aanpassen
		mov	send1,#00001010b
		mov	send2,#00001111b
		lcall	schrijf_data
		ljmp	main
		
runp:		;laat tekst zien
		mov	dptr,#tekst
		mov	tekst_counter_l,dpl
		mov	tekst_counter_h,dph
		mov	tekst_counter,#0h
		
schrijf:	mov	dpl,tekst_counter_l
		mov	dph,tekst_counter_h
		mov	a,#0h
		mov	send1,#00000000b		;Eerste kolom
		movc	a,@a+dptr
		mov	send2,a
		lcall	schrijf_data
		mov	a,#1h				;Tweede kolom
		mov	send1,#00000001b
		movc	a,@a+dptr
		mov	send2,a
		lcall	schrijf_data
		mov	a,#2h				;Derde kolom
		mov	send1,#00000010b
		movc	a,@a+dptr
		mov	send2,a
		lcall	schrijf_data
		mov	a,#3h				;Vierde kolom
		mov	send1,#00000011b
		movc	a,@a+dptr
		mov	send2,a
		lcall	schrijf_data
		mov	a,#4h				;Vijfde kolom
		mov	send1,#00000100b
		movc	a,@a+dptr
		mov	send2,a
		lcall	schrijf_data
		mov	a,#5h				;Zesde kolom
		mov	send1,#00000101b
		movc	a,@a+dptr
		mov	send2,a
		lcall	schrijf_data
		mov	a,#6h				;Zevende kolom
		mov	send1,#00000110b
		movc	a,@a+dptr
		mov	send2,a
		lcall	schrijf_data
		mov	a,#7h				;Achtste kolom
		mov	send1,#00000111b
		movc	a,@a+dptr
		mov	send2,a
		lcall	schrijf_data
wacht:		cjne	timer_counter,#00010000b,wacht
		mov	timer_counter,#00000000b
		inc	dptr				;Lichtkrant =>  kolom opschuiven
		mov	tekst_counter_l,dpl
		mov	tekst_counter_h,dph
		inc	tekst_counter
		cjne	tekst_counter,#26h,schrijf	;lengte string - 7
		ljmp	main
		
timer0_int:	inc	timer_counter
		RETI

		
tekst:
    db      01111111b     ; .*******
    db      00000010b     ; ......*.
    db      00000100b     ; .....*..
    db      00000010b     ; ......*.
    db      01111111b     ; .*******	
    db      00000000b         
    db      00100000b     ; ..*.....
    db      01010100b     ; .*.*.*..
    db      01010100b     ; .*.*.*..
    db      01010100b     ; .*.*.*..
    db      01111000b     ; .****...	
    db      00000000b        
    db      00000100b     ; .....*..
    db      00111111b     ; ..******
    db      01000100b     ; .*...*..
    db      01000000b     ; .*......
    db      00100000b     ; ..*.....
    db      00000000b     
    db      00000100b     ; .....*..
    db      00111111b     ; ..******
    db      01000100b     ; .*...*..
    db      01000000b     ; .*......
    db      00100000b     ; ..*.....
    db      00000000b     
    db      01111111b     ; .*******
    db      00001000b     ; ....*...
    db      00000100b     ; .....*..
    db      00000100b     ; .....*..
    db      01111000b     ; .****...
    db      00000000b     
    db      01000100b     ; .*...*..
    db      01111101b     ; .*****.*
    db      01000000b     ; .*......
    db      00000000b     
    db      00100000b     ; ..*.....
    db      01010100b     ; .*.*.*..
    db      01010100b     ; .*.*.*..
    db      01010100b     ; .*.*.*..
    db      01111000b     ; .****...	    
    db      00000000b     
    db      01001000b     ; .*..*...
    db      01010100b     ; .*.*.*..
    db      01010100b     ; .*.*.*..
    db      01010100b     ; .*.*.*..
    db      00100000b     ; ..*.....
    
        
     
$INCLUDE (c:/aduc800_mideA.inc)
$INCLUDE (UART.inc)			;DEZE FILE GEBRUIKT GEHEUGENLOCATIES 0045h TOT EN MET 0050h EN BIT 2+3+4+5
$INCLUDE (spi.inc)			;DEZE FILE GEBRUIKT GEHEUGENLOCATIES 0030h TOT EN MET 0034h EN BIT 0+1
;$INCLUDE (lettertype.inc)		
end