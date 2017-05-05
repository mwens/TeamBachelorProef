init_uart:	lcall 	initsio			;init baudrategenerator (t3) op 9600 baud (pllcon wordt gebruikt)
		setb	es			;UART interupt on
		clr	uart_in_mode	
		mov	uart_in_counter,#0h	
		clr	uart_flag
		clr	error
		clr	uart_in_data
		mov	uart_out_counter,#0h
		mov	counter_in,#0h
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

uart_error2:	ljmp	uart_error

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
		ljmp	uart_error	


; <= COMMANDO's =>
; :init<cr> = tel alle matrixen
; :sync<cr> = synchroniseer klokken
; :send<cr> string <cr> = versturen string
; :runp<cr>
; Elk commando krijgt ook een eigen fasenummer, dat in de variabele "fase" wordt gestoken, zodat het hoofdprogramma weet welk commando werd ingegeven.
		
uart_init:	mov	a,uart_in1
		cjne	a,#'i',uart_sync		;init
		mov	a,uart_in2
		cjne	a,#'n',uart_error
		mov	a,uart_in3
		cjne	a,#'i',uart_error
		mov	a,uart_in4
		cjne	a,#'t',uart_error
		mov	fase,#0h
		ljmp	print_fase

uart_sync:	mov	a,uart_in1
		cjne	a,#'s',uart_run			;sync
		mov	a,uart_in2
		cjne	a,#'y',uart_send
		mov	a,uart_in3
		cjne	a,#'n',uart_error
		mov	a,uart_in4
		cjne	a,#'c',uart_error
		mov	fase,#1h
		ljmp	print_fase
		
uart_send:	mov	a,uart_in2
		cjne	a,#'e',uart_error		;send
		mov	a,uart_in3
		cjne	a,#'n',uart_error
		mov	a,uart_in4
		cjne	a,#'d',uart_error
		mov	fase,#2h
		setb	string
		
lees_string:	;hier wordt de string ingelezen		
		clr	string
		ljmp	print_fase

uart_run:	mov	a,uart_in1
		cjne	a,#'r',uart_error		;runp
		mov	a,uart_in2
		cjne	a,#'u',uart_error
		mov	a,uart_in3
		cjne	a,#'n',uart_error
		mov	a,uart_in4
		cjne	a,#'p',uart_error
		mov	fase,#3h
		ljmp	print_fase
		
;Wanneer een foutief commando wordt ontvangen, zal er via uart een error-signaal teruggestuurd worden naar de computer.

uart_error:	setb	error

		inc	uart_out_counter
		mov	a,uart_out_counter
		mov	r0,#'e'
		dec	a
		jz	error_trans
		mov	r0,#'r'
		dec	a
		jz	error_trans
		mov	r0,#'r'
		dec	a
		jz	error_trans
		mov	r0,#'o'
		dec	a
		jz	error_trans
		mov	r0,#'r'
		dec	a
		jz	error_trans
		mov	r0,#'!'
		
		mov	uart_out_counter,#0h
		clr	error

error_trans:	mov	SBUF,r0			;schrijf karacter;	
		sjmp	uart_ret

print_fase:	;mov	a,fase
		;add	a,#30h			;cijfer omzetten in overeenkomstig ascii karakter
		;mov	SBUF,a			;schrijf karakter;	
		sjmp	uart_ret
		
uart_trans:	jnb	TI, uart_ret		;End Transmission?
		jb	error,uart_error
		jb	uart_in_data,uart_in

uart_ret:	pop	acc
		mov	R0,a
		pop	acc
		pop	psw
		clr	TI
		clr	RI
		reti


;terugsturen binnengekregen data

uart_in:	setb	uart_in_data
		inc	uart_out_counter
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
		ret
		
in2:		mov	a,r4
		anl	a,#00001111b
		lcall	batrans
		mov	r0,a
		ljmp	error_trans
		
in3:		mov	a,r3
		anl	a,#11110000b
		rl	a
		rl	a
		rl	a
		rl	a
		lcall	batrans
		mov	r0,a
		ljmp	error_trans
		
in4:		mov	a,r3
		anl	a,#00001111b
		lcall	batrans
		mov	r0,a
		ljmp	error_trans
