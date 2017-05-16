$NOLIST
$nomod51
$INCLUDE (c:/reg832.pdf)
$LIST


stack_init	equ	07fh				;beginadres programma

		org	0000h				;start programma
		sjmp	start	
		
start:		mov	pllcon,#0b			;zet 0 in dpllcon => clockf = 16.77MHz
		mov	sp,#stack_init			;stackpointer laten wijzen naar beginadres programma

		mov	spicon,#00111010b		;2 LSB => f_osc/8 = "10
		setb	p3.4
		
main:		clr	p3.4
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		mov	spidat,#11111111b
wacht00:	jnb	ISPI,wacht00
		mov	spidat,#11111110b
wacht0:		jnb	ISPI,wacht0
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		setb	p3.4
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		clr	p3.4
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		mov	spidat,#00001100b
wacht:		jnb	ISPI,wacht
		mov	spidat,#00000001b
wacht2:		jnb	ISPI,wacht2
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		setb	p3.4
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		clr	p3.4
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		mov	spidat,#00001011b
wacht3:		jnb	ISPI,wacht3
		mov	spidat,#00000111b
wacht4:		jnb	ISPI,wacht4
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		setb	p3.4
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		clr	p3.4
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		mov	spidat,#00001010b
wacht5:		jnb	ISPI,wacht5
		mov	spidat,#00001111b
wacht6:		jnb	ISPI,wacht6
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		setb	p3.4
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		clr	p3.4
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		mov	spidat,#00000011b
wacht7:		jnb	ISPI,wacht7
		mov	spidat,#01010101b
wacht8:		jnb	ISPI,wacht8
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		setb	p3.4
wacht9:		sjmp	wacht9
		
$INCLUDE (c:/aduc800_mideA.inc)
;$INCLUDE (UART.inc)			;DEZE FILE GEBRUIKT GEHEUGENLOCATIES 0045h TOT EN MET 0050h EN BIT 2+3+4+5
;$INCLUDE (spi.inc)			;DEZE FILE GEBRUIKT GEHEUGENLOCATIES 0030h TOT EN MET 0034h EN BIT 0+1
;$INCLUDE (lettertype.inc)		
end