init_uart:	lcall 	initsio			;init baudrategenerator (t3) op 9600 baud (pllcon wordt gebruikt)
		setb	es			;UART interupt on
		clr	uart_in_mode	
		mov	uart_in_counter,#0h	
		clr	uart_flag
		clr	error
		mov	uart_out_counter,#0h
		RET
		
		
;UART 

uart_int:	push	psw
		push	acc
		mov	a,R0
		push	acc
		
		
;Deze functie controleerd of hij aan het ontvangen was bij een UART interrupt. Als hij nog niet begonnen is met lezen en ":" wordt gegeven,
;dan zal hij de uart_in_mode bit op 1 zetten, zodat hij volgende keer weet dat hij aan het inlezen is. Indien hij een ";" ontvangt zet hij deze
;bit terug op 0 zodat hij volgende keer weet dat hij geen commando meer aan het inlezen is.

uart_rec:	jnb	RI, uart_trans2 	;Received?
		mov	R0,SBUF			;lees ontvangen character	
		
		jb	uart_in_mode,uart_lees	;als in mode 1, lees ; mode 0: check op startcommando ':'
		cjne	R0,#':',uart_trans2	;als geen start => negeren
		mov	uart_in_counter,#0h
		setb	uart_in_mode

uart_trans2:	ljmp	uart_trans

uart_error2:	ljmp	uart_error_tmp

;Met de uart_in_counter wordt bepaald in welke buffer het ingelezen symbool gestoken moet worden. Indien er 4 symbolen en vervolgens een CR wordt ontvangen,
;zal de uart_flag op 1 gezet worden, zodat het hoofdprogramma weet dat er een geldig commando is ontvangen en dit verwerkt moet worden. De uart_in_mode bit wordt
;ook gecleared, aangezien we niet meer moeten ontvangen hierna. De uart wacht opnieuw op het startsymbool ":"
		
uart_lees:	mov	a,uart_in_counter
		cjne	a,#0h,uart_lees1	;counter op 0 => in buf1
		mov	uart_in1,R0
		inc	uart_in_counter
		ljmp	uart_trans	
uart_lees1:	cjne	a,#1h,uart_lees2	;counter op 1 => in buf2
		mov	uart_in2,R0
		inc	uart_in_counter
		ljmp	uart_trans
uart_lees2:	cjne	a,#2h,uart_lees3	;counter op 2 => in buf3
		mov	uart_in3,R0
		inc	uart_in_counter
		ljmp	uart_trans
uart_lees3:	cjne	a,#3h,uart_lees4	;counter op 3 => in buf4
		mov	uart_in4,R0
		inc	uart_in_counter
		ljmp	uart_trans
uart_lees4:	cjne	R0,#CR,uart_lees_error	;CR => commando checken en uitvoeren
		setb	uart_flag
		clr	uart_in_mode
		sjmp	uart_init
uart_lees_error:mov	uart_in_counter, #0h	; Reset UART In vlag/counter
		clr	uart_in_mode
		ljmp	uart_error_tmp	


; <= COMMANDO's =>
; :init<cr> = tel alle matrixen
; :sync<cr> = synchroniseer klokken
; :send<cr> string <cr> = versturen string
; :runp<cr>
; Elk commando krijgt ook een eigen fasenummer, dat in de variabele "fase" wordt gestoken, zodat het hoofdprogramma weet welk commando werd ingegeven.
		
uart_init:	mov	a,uart_in1
		cjne	a,#'i',uart_sync		;init
		mov	a,uart_in2
		cjne	a,#'n',uart_error_tmp
		mov	a,uart_in3
		cjne	a,#'i',uart_error_tmp
		mov	a,uart_in4
		cjne	a,#'t',uart_error_tmp
		mov	fase,#0h
		ljmp	print_fase

uart_sync:	mov	a,uart_in1
		cjne	a,#'s',uart_run			;sync
		mov	a,uart_in2
		cjne	a,#'y',uart_send
		mov	a,uart_in3
		cjne	a,#'n',uart_error_tmp
		mov	a,uart_in4
		cjne	a,#'c',uart_error_tmp
		mov	fase,#1h
		ljmp	print_fase
		
uart_send:	mov	a,uart_in2
		cjne	a,#'e',uart_error_tmp		;send
		mov	a,uart_in3
		cjne	a,#'n',uart_error_tmp
		mov	a,uart_in4
		cjne	a,#'d',uart_error_tmp
		mov	fase,#2h
		setb	string
		
lees_string:	;hier wordt de string ingelezen		
		clr	string
		ljmp	print_fase

uart_run:	mov	a,uart_in1
		cjne	a,#'r',uart_error_tmp		;runp
		mov	a,uart_in2
		cjne	a,#'u',uart_error_tmp
		mov	a,uart_in3
		cjne	a,#'n',uart_error_tmp
		mov	a,uart_in4
		cjne	a,#'p',uart_error_tmp
		mov	fase,#3h
		ljmp	print_fase



uart_error_tmp: ljmp	uart_error_msg
		
; <Start Transmit>		
uart_trans:	jnb	TI, uart_ret			;SBUF fully transmitted?
		jb	error,uart_trans_msg
		jb	uart_in_data,uart_data
		sjmp	uart_ret


;Wanneer een foutief commando wordt ontvangen, zal er via uart een error-signaal teruggestuurd worden naar de computer.
uart_error_msg:	mov	dptr,#uart_input_error
		mov 	msg_pointer_h,dph	;sla counter op
		mov 	msg_pointer_l,dpl
		setb	error

uart_trans_msg: mov 	dph, msg_pointer_h	;laad counter in dptr
		mov 	dpl, msg_pointer_l
		mov	a,#0h
		movc	a,@a+dptr
		inc	dptr			;inc counter
		mov 	msg_pointer_h,dph	;sla counter op
		mov 	msg_pointer_l,dpl
		jz	einde
		mov	SBUF,a			;schrijf character;
		sjmp	uart_ret
		
einde:		clr	error
		sjmp	uart_ret


print_fase:	;mov	a,fase
		;lcall	batrans			;cijfer omzetten in overeenkomstig ascii karakter
		;mov	r0,a
		sjmp	uart_ret

byte_trans:	mov	SBUF,r0			;schrijf karacter;	
		sjmp	uart_ret		

; <UART interrupt end>
uart_ret:	pop	acc
		mov	R0,a
		pop	acc
		pop	psw
		clr	TI
		clr	RI
		reti
		
; <== Einde Interrupt ==>

;terugsturen binnengekregen data

uart_in:	push	acc
		mov	a,r0
		push	acc
		push	psw
		setb	uart_in_data

uart_data:	inc	uart_out_counter
		mov	a,uart_out_counter
		dec	a
		jz	in1
		dec	a
		jz	in2
		dec	a
		jz	in3
		mov	uart_out_counter,#0h
		clr	uart_in_data		
		sjmp	in4

in1:		mov	a,r4
		anl	a,#11110000b
		rl	a
		rl	a
		rl	a
		rl	a
		lcall	batrans
		mov	r0,a
		mov	SBUF,r0			;schrijf karacter;
			
		pop	psw
		pop	acc
		mov	r0,a
		pop	acc
		ret
		
in2:		mov	a,r4
		anl	a,#00001111b
		lcall	batrans
		mov	r0,a
		ljmp	byte_trans
		
in3:		mov	a,r3
		anl	a,#11110000b
		rl	a
		rl	a
		rl	a
		rl	a
		lcall	batrans
		mov	r0,a
		ljmp	byte_trans
		
in4:		mov	a,r3
		anl	a,#00001111b
		lcall	batrans
		mov	r0,a
		ljmp	byte_trans






; <== VASTE STRINGS ==>
uart_input_error: 	db CR,CR,"-----------------------------------------------------------------",CR
			db "| ERROR! Type :init<cr> of :sync<cr> of :send<cr> of :runp<cr>! |"
			db CR,"-----------------------------------------------------------------",CR,CR,0