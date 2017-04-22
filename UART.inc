init_uart:	lcall 	initsio			;init baudrategenerator (t3) op 9600 baud (pllcon wordt gebruikt)
		setb	es			;UART interupt on
		clr	uart_in_mode	
		mov	uart_in_counter,#0h	
		clr	p2.5
		setb	p2.6
		setb	p2.7
		clr	uart_flag
		clr	error
		mov	uart_out_counter,#0h
		RET
		
		
;UART 

uart_int:	push	psw
		push	acc
		mov	a,R0
		push	acc
		cpl	p2.5
		
uart_rec:	jnb	RI, uart_trans2 	;Received?
		mov	R0,SBUF			;lees ontvangen character;	
		
		jb	uart_in_mode,uart_lees	;als in mode 1, lees ; mode 0: check op startcommando ':'
		cjne	R0,#':',uart_trans2	;als geen start => negeren
		mov	uart_in_counter,#0h
		setb	uart_in_mode
		ljmp	uart_trans

uart_trans2:	ljmp	uart_trans
		
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
uart_lees4:	cjne	R0,#CR,uart_error	;CR => commando checken en uitvoeren
		setb	uart_flag
		clr	uart_in_mode


; <= COMMANDO's =>
; :init<cr> = tel alle matrixen
; :sync<cr> = synchroniseer klokken
; :send<cr> string <cr> = versturen string
; :runp<cr>

; DIRECT BYTE NIET COMPAREN (enkel register of acc)		
uart_init:	cpl	p2.6
		mov	a,uart_in1
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
		
;<= afhandelen eindstates receiving =>	
uart_error:	clr	p2.7
		setb	error

		inc	uart_out_counter
		mov	a,uart_out_counter
		mov	r0,#'e'
		subb	a,#1
		jc	error_trans
		mov	r0,#'r'
		subb	a,#1
		jc	error_trans
		mov	r0,#'r'
		subb	a,#1
		jc	error_trans
		mov	r0,#'o'
		subb	a,#1
		jc	error_trans
		mov	r0,#'r'
		subb	a,#1
		jc	error_trans
		mov	r0,#'!'
		mov	uart_out_counter,#0h
		clr	error

error_trans:	mov	SBUF,r0			;schrijf character;	
		sjmp	uart_ret

print_fase:	mov	SBUF,fase		;schrijf character;	
		sjmp	uart_ret
		
uart_trans:	jnb	TI, uart_ret			;End Transmission?
		jb	error,uart_error

uart_ret:	pop	acc
		mov	R0,a
		pop	acc
		pop	psw
		clr	TI
		clr	RI
		reti
