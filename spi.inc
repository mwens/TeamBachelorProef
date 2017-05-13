init_spi:	mov	spicon,#00111010b		;2 LSB => f_osc/8 = "10"
		
		setb	ea				;enable all interrupts
		orl	IEIP2,#00010001b		;set priority spi high
							;enable SPI serial port interrupt
							
		clr	sending
		clr	laatste_byte
		
		
		mov	send1,#11000000b		;write configuration
		mov	send2,#00000000b
		lcall	schrijf_data			;DIT MOET NOG VERSTUURT WORDEN NAAR DE MAX3100
		
		
		mov	send1,#10000010b		;write data bovenste byte
		mov	send2,#00000000b
		mov	receive1,#00000000b
		mov	receive2,#00000000b
		RET	
		
		
;Deze interrupt past geen registerbanken aan

read_or_write:	jnb	sending,leesnogeens2		;interruptroutine
		jnb	laatste_byte,stuurnogeens2
		clr	laatste_byte			;er zijn reeds 2 bytes gezonden
		clr	sending
		RETI

stuurnogeens2:	lcall	stuurnogeens
		RETI

			
stuurnogeens:	setb	laatste_byte
		mov	send1,send2		
		
schrijf_data:	setb	sending				;sending=1 => aan het zenden
		mov	spidat,send1			;hierkomtdewaarde = het te versturen vak
		RET					;Er worden automatisch 8 clockslagen naar buiten gestuurd.
							;Op het einde wordt de ISPI op 1 gezet.


leesnogeens2:	lcall	leesnogeens
		RETI
							
leesnogeens:	mov	receive2,receive1
		
lees_data:	mov	receive1,spidat
		RET
