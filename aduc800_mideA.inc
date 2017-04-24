;******************************************************************************
; Hier kan de gebruiker instellen welke versie van het aduc800 bord er gebruikt
; wordt. V1.1 zal communiceren via een seriële poort, V2.2 via USB
; Dit is ook belangrijk voor het lezen van de schakelaars.  Deze zitten op 
; andere poortpinnen bij de twee versies! (zie hardware documentatie)
; Instelling versie ADuC832 bord = V1.1: verander in commentaar voor V2.0 (i2c)
;aduc832_v1_1  set   1               ;versie 1.1, LCD op P0.2-P0.7
;******************************************************************************
; Instelling versie ADuC832 bord = V2.0: verander in commentaar voor V1.1 (P0)
aduc832_v2_0  set  1               ;versie 2.0, LCD via pcf8574
;******************************************************************************


;******************************************************************************
; Hier definieren we welke modules uit de aduc800_mide file we effectief nodig
; hebben voor onze toepassing. Door een module te definieren via het 
; SET commando is deze modulenaam gekend voor de assembler en kunnen de nodige
; modules aan de hand van deze modulenaam conditioneel worden geassembleerd.
; Indien de gebruiker bepaalde modules niet nodig heeft moet de definitie in
; commentaar worden gezet door er een ';' voor te plaatsen.
; Hierdoor zal er ook minder codegeheugen worden gebruikt door de aduc800_mide
; file. De hoeveelheid code per module wordt telkens aangegeven achter de 
; definities.
;******************************************************************************
aduc_lcd    set        1    ;benodigd codegeheugen = v1.1: 02bfh of 703d bytes
                            ;benodigd codegeheugen = v2.0: 045ch of 1116d bytes
                            ;dit is inclusief met de benodigde aduc_i2c driver 
aduc_i2c    set        1    ;benodigd codegeheugen = 0168h of 360d bytes
aduc_sio    set        1    ;benodigd codegeheugen = 0160h of 352d bytes
aduc_math   set        1    ;benodigd codegeheugen = 0d9ah of 3482d bytes!
aduc_adc    set        1    ;benodigd codegeheugen = 00bch of 188d bytes
aduc_key    set        1    ;benodigd codegeheugen = 0192h of 402d bytes

ifdef    aduc_key
;De volgende definities geven aan of de 8 individuele schakelaars van het 
;aduc832 bord (de dipswitches, druktoetsen of extern aangesloten schakel-
;contacten) moeten worden gelezen, of dat er een 4x4 matrixkeyboard moet 
;worden gelezen dat is verbonden met het aduc832 bord.   
;Bij aduc832v1.1 zijn de 4 druktoetsen parallel geschakeld met de dipswitch 
;en de externe connectorpinnen van H2 waar er een extern 4x4 matrixkeybord 
;kan worden aangesloten. Het is duidelijk dat bij deze versie V1.1 ofwel een
;definitie moet gemaakt worden van het symbool matrix, ofwel van het symbool
;dipswitch daar deze niet tegelijk kunnen worden gelezen.
;Bij aduc832v2.0 zijn de 4 druktoetsen met P3 verbonden, terwijl de 
;dipswitch en de externe connectorpinnen van H0 parallel zijn verbonden.
;Bij deze versie kunnen beide dus wel tegelijk worden gelezen en kunnen beide 
;definities wel gemaakt worden.
matrix        set      1    ;als er een matrixkeyboard moet worden gelezen
                            ;anders commentaar van maken!
dipswitch     set      1    ;als de individuele schakelaars moeten worden 
                            ;gelezen, anders commentaar van maken!
;******************************************************************************
;Declaraties van de flags voor de aduc_key driver.
;Deze flags worden gebruikt in de routines voor het lezen van een matrix-
;keybord nl:key_init, keyscan, keydown, keyedge,toets_flank,toets_ingedrukt
;en bij de routines voor het lezen van 8 individuele schakelaars:
;switch_init en read_switch.
keyflags    equ        20h      ;dit moet in bit adresseerbaar gebied liggen
                                ;bereik = 20h-2fh!
keyvalid    bit        keyflags.0  ;flag die aangeeft dat KEY 
                                   ;een geldige waarde bevat
keymode     bit        keyflags.1  ;flag voor intern gebruik, 
                                   ;die de detectie mode aangeeft
;declaratie van de read_switch flags
swvalid     bit        keyflags.2  ;flag die aangeeft dat er actuele
                                   ;ontdenderde schakelaars zijn
negvalid    bit        keyflags.3  ;flag die aangeeft dat er negatieve
                                   ;flanken zijn
posvalid    bit        keyflags.4  ;flag die aangeeft dat er positieve
                                   ;flanken zijn
ifdef    matrix        ;matrix keyboard
;Declaraties van variabelen (9 bytes) voor de aduc_key driver.
;Deze worden gebruikt in de routines voor het lezen van een matrixkeybord
;nl:key_init,Keyscan, Keydown,Keyedge, toets_flank en toets_ingedrukt.
;Deze kunnen door de gebruiker aangepast worden aan de noden van zijn
;toepassing.
keymap      equ        30h      ;default startadres van de benodigde 9 byte 
                                ;geheugenbuffer
map01ptr    equ        keymap   ;intern startadres van de 8 map bytes die de 
                                ;debounce en de flankdetectie samples stockeren
                                ;de eerste 4 bytes dienen voor debouncing
map23ptr    equ        keymap+4 ;de volgende 4 bytes dienen voor flankdetectie
key         equ        keymap+8 ;adres van de variabele die de ingedrukte 
                                ;toetswaarde aangeeft 
                                ;bij een geldige keyvalid flag ('1')
endif    ;einde matrix keyboard

ifdef    dipswitch
;Declaraties van variabelen (6 bytes) voor het lezen en debouncen van 8 
;individuele schakelaars nl: de switch_init en read_switch routine.
;Het startadres kan door de gebruiker worden aangepast aan de noden van de 
;toepassing 
switchmap    equ        40h     ;startadres voor de 6 bytes intern geheugen
switch1      equ        switchmap+0 ;adres nieuwste sample schakelaars
switch2      equ        switchmap+1 ;adres vorige sample schakelaars
debounce     equ        switchmap+2 ;adres ontdenderde schakelaars
closed       equ        switchmap+3 ;adres flankdetectie flipflop
negedge      equ        switchmap+4 ;adres negatieve flankdetectie
posedge      equ        switchmap+5 ;adres positieve flankdetectie
endif    ;einde dipswitch
;******************************************************************************
endif    ;einde aduc_key


ifdef    aduc_lcd                   ;LCD driver is nodig indien gedefinieerd

ifdef    aduc832_v2_0               ;bij v2.0 is de i2c driver nodig ook al
aduc_i2c    set        1            ;heeft de gebruiker deze niet gedefinieerd
endif                               ;dus doen we dit hier nog eens!
;******************************************************************************

;******************************************************************************
;
; ADuc_lcd.inc 
; Is een include file voor het gebruik van een op een Hitachi HD44870 gebaseerd
; (of compatibel) lcd scherm op het ADuC800 bord. 
; Het bord wordt gebruikt met een 1*16 of een 2*16 karakter
; display. De basisroutines zijn ook bruikbaar voor schermen met meerdere
; lijnen en meer chars per lijn. (de gebruiker moet dan wel zelf zorgen voor 
; het plaatsen van de cursor op een correcte plaats).
; De routines op het hoogste niveau gaan er van uit dat we een 2*16 karakter 
; display gebruiken.
; Parameters worden doorgegeven via de accu, b register, dptr of r0.
; De routines vernietigen geen registers. Alle timing wordt automatisch 
; aangepast aan de controller clockfrequentie (aan de waarden in het pllcon 
; register). De routines zijn geoptimaliseerd naar de aansluitingen op een 
; MGM/[dp] ADuC800 bord. Dit in de versie ADuC832V1.1 of de versie ADuC832V2.0
; 
; Versie ADuC832v1.1:
; Hier is het LCD verbonden met pinnen van poort0
; DB4=P0.4, DB5=P0.5, DB6=P0.6, DB7=P0.7, E=P0.2, RS=P0.3
; De 2 overblijvende klemmen van poort 0 (P0.0 en P0.1) worden niet aangepast.
;
; Versie ADuC832v2.2:
; Hier is het LCD via een I2C port expander PCF8574 verbonden met de controller
; Via de definitie van het symbool aduc832_v1_1 of aduc832_v2_0 kan er 
; conditioneel code worden geassembleerd voor één van de twee versies:
; aduc832_v1_1    set 1 => ADuC832v1.1 (poort0) = default instelling
; aduc832_v2_0  set    1 => ADuC832v2.0 (IIC-bus) 
; 
;
; GESCHREVEN OP 02/1994 DOOR MARC ROGGEMANS als lcd.drv
; -aangepast door Roggemans Marc (MGM) op 11/2004 voor ADuC800 kaart als 
;  aduclcd0.inc
; -aangepast door Roggemans Marc (MGM) op 11/11/2004
;  aduclcd1.inc
;    -00dh en 00ah worden door outchar verwerkt
;    -bij de init wordt het display anders opgestart
;
; -door [dp] op 22/11/2013 voorzien van extra commentaar met voorbeelden 
;  van gebruik en uitgebreid  met :
;  * outcharlcd drukt ook ASCII code 0-7 (Build gedefinieerde karakters) af
;  * barlcd routine voor het afdrukken van een 'bargraph' display
;  * hexbuf2lcd  routine voor afdrukken van interne registers voor 
;    debug doeleinden
;  * build_adr routine om vanaf een bepaald intern cgram adres (0-7) 
;    user codes aan te maken
;  * conditioneel assembleren voor twee versies van het ADuC832 bord
;  * bug uit outhnibd en outhnibc ivm aansturing p0.0 en p0.1
;
; Beschikbare routines:
; initlcd, outcharlcd, outniblcd, outbytelcd, dispdptrlcd, outmsgalcd, 
; regtolcd, outd, outhnibd, outc, outhnibc, hexbuf2lcd, barlcd, build, 
; build_adr, delay2ms, delay60us 
;******************************************************************************

ifdef    aduc832_v1_1    ;V1.1 gebruikt P0 voor LCD sturing
lcdport       EQU    p0                   ;SELECTIE POORT 0
e             bit    lcdport.2            ;enable
rs            bit    lcdport.3            ;register select
endif    ;einde aduc832_v1_1

ifdef    aduc832_v2_0    ;V2.0 gebruikt een I2C port expander voor LCD sturing
lcdport       EQU    b                    ;b wordt gebruikt als dummy poort
e             bit    lcdport.1            ;enable
rs            bit    lcdport.0            ;register select
backlight     bit    lcdport.6            ;bit waar backlight op zit
buzzer        bit    lcdport.7            ;bit waar buzzer op zit
endif    ;einde aduc832_v2_0

;Hier worden de codes aangegeven die naar het LCD moeten worden gestuurd voor 
;het uitvoeren van de overeenkomstige commando's.
cleardisp     EQU    001H                 ;LEEG MAKEN VAN HET SCHERM
cursathom     EQU    002H                 ;CURSOR OP DE EERSTE PLAATS ZETTEN

entrymode     EQU    00000110B            ;CURSOR NAAR RECHTS, SCHERM VAST
displayon     EQU    00000001B            ;HOMEN EN CLEAR CURSOR
displayof     EQU    00001000B            ;DISPLAY OFF, CURSOR OFF
cursoroff     EQU    00001100B            ;DISPLAY ON ZONDER CURSOR
cursoronb     EQU    00001111B            ;CURSOR ON AND BLINK
cursoronn     EQU    00001110B            ;CURSOR ON NO BLINK
functions     EQU    00101000B            ;INTERFACE LENGTE EN KARAKTER FONT
cgram         EQU    01000000B            ;SELECTIE KARAKTER GENERATOR RAM
ddram         EQU    10000000B            ;SELECTIE DATA DISPLAY RAM

;******************************************************************************
;
; initlcd
; Zal het scherm resetten en initialiseren voor gebruik via een 4 bit bus op
; poort0 of via i2c. De routine zet het scherm klaar voor normaal gebruik: 
; ingave van links naar rechts zonder display shift, normale karakterfont. 
; De cursor staat aan als een knipperend lijntje onderaan de regel. 
; Na initialisering wordt het scherm leeg gemaakt.
;
; registergebruik: geen
;
;******************************************************************************

initlcd:      push   acc           ;registers op stack plaatsen
              push   psw
              push   b
ifdef  aduc832_v1_1    
              mov    lcdport,#003h ;poort in uitgangstoestand zetten
                                   ;let op:twee laagste bits worden
                                   ;niet gebruikt
endif

ifdef    aduc832_v2_0
              lcall  i2cinit              ;LCD via een I2C bus
              mov    lcdport,#11111101b   ;poort in uitgangstoestand zetten
                                          ;buzzer en backlight  op 1 gezet
              lcall  porttolcd            ;data uit b naar het scherm 
endif
  
              mov    a,#010h       ;bij opstarten van Vcc > dan 30ms wachten
initlcd1:     lcall  delay2ms
              djnz   acc,initlcd1
              mov    a,#00110000b  ;volgende drie schrijfbeurten zijn 
              lcall  outhnibc      ;beschreven in de HD44870 datasheet  
              lcall  delay2ms      ;om het display altijd opgestart
              mov    a,#00110000b  ;te krijgen.
              lcall  outhnibc
              lcall  delay2ms
              mov    a,#00110000b
              lcall  outhnibc
              lcall  delay2ms
              mov    a,#00100000b  ;instellen 4 bit interface gebruiken
              lcall  outhnibc      ;hoge nibble naar het controleregister
              lcall  delay2ms      ;niet nodig volgens datasheet
              mov    a,#functions  ;4 bit interface, twee lijnen 5*8 dots
              lcall  outc          ;naar display
              lcall  delay2ms      ;is iets te lang volgens datasheet
              mov    a,#cursoronb  ;display on cursor on and blink
              lcall  outc
              lcall  delay2ms      ;ook deze delay is iets te lang
              mov    a,#displayon  ;clear display and home cursor
              lcall  outc
              lcall  delay2ms      ;deze wachttijd is bijna juist
              mov    a,#entrymode  ;display klaar zetten normaal gebruik
              lcall  outc
              lcall  delay2ms      ;wachten weer veel te lang
              pop    b
              pop    psw           ;registers terug in orde zetten
              pop    acc
              ret                  ;einde van de initialisatie

;******************************************************************************
;
; outcharlcd  
; Zal de ascii code in de accu naar het lcd scherm sturen om af te drukken. 
; De codes 00h tot 07h komen overeen met een door de build routine aangemaakte
; user karakters. De codes groter dan 08h en kleiner dan 20h worden als 
; controlekarakters bekeken en kunnen door de gebruiker gewoon worden opgenomen
; in de ascii strings die moeten worden weergegeven op het LCD.
; De controlekarakters komen overeen met volgende acties:
;  08h,09h,0bh,0eh-01fh   =nog niet in gebruik
;      00ah   =cursor op eerste char van tweede lijn plaatsen 
;              (line feed+ carriage return!)
;      00ch   =form feed : scherm leeg maken en cursor op locatie 000h
;      00dh   =cursor in de home positie (eerste char van de eerste lijn)
;      011h   =cursor on, blink mode
;      012h   =cursor on, no blink mode
;      013h   =cursor off
;  80h-0ffh   =cursor plaatsen op een DDRAM adres. Waneer bit7 is geset(80h)
;              worden de 7 lagere bits geinterpreteerd als het DDRAM adres 
;              van het LCD
;              |00|01|02|03|04|05|06|07|08|09|0A|0B|0C|0D|0E|0F|
;              |40|41|42|43|44|45|46|47|48|49|4A|4B|4C|4D|4E|4F|   
; Een code bv. C5H zal de cursor dus positioneren op de tweede lijn 
; positie 6 (C5H-80H=45H)              
;
; Deze routine vernietigt geen registers
;
;******************************************************************************

; we gaan eerst na of het een ascii code betreft uit de afdrukbare ASCII set.
; Als dat zo is,(00h-07h of 20h-07fh) dan mag de data direct naar het 
; dataregister van het scherm.
; Indien de code >80H dan stelt dit een DDRAM adres voor en moeten we het als 
; controlebyte doorsturen naar het LCD. 
; Betreft het een controle karakter (08h-20H), dan is een andere aanpak nodig 
; en moeten we de code interpreteren en het gepaste overeenkomende commando 
; naar de LCD sturen.

outcharlcd:   push   acc           ;registers op stack
              push   psw
              cjne   a,#080h,outcharlcd1  ;hogere of gelijke code is adres
outcharlcd1:  jnc    outcharlcd9   ;als geen carry dan is de code een address
              cjne   a,#08h,outcharlcd2_0  ;lagere code is afdrukbare user char
outcharlcd2_0:jc     outcharlcd8   ;bij cy dan code van userchar naar display
              cjne   a,#020h,outcharlcd2  ;hogere of gelijke code is afdrukbaar
outcharlcd2:  jnc    outcharlcd8   ;als geen carry dan ascii code naar display

; De code in de accu is geen adres of geen geldig afdrukbaar karakter.
; Als het een commando is, moeten we het uitvoeren. 
; Indien niet, gewoon terug zonder actie!

              cjne   a,#00ch,outcharlcd3  ;geen form feed, dan volgende testen
              mov    a,#cleardisp         ;"scherm leegmaken" commando
              sjmp   outcharlcd9          ;afsluiten
              
outcharlcd3:  cjne   a,#011h,outcharlcd4  ;geen cursor on blink
              mov    a,#cursoronb         ;"cursoron met blink"commando
              sjmp   outcharlcd9 

outcharlcd4:  cjne   a,#012h,outcharlcd5  ;geen cursor on no blink
              mov    a,#cursoronn         ;"cursoron no blink" commando
              sjmp   outcharlcd9

outcharlcd5:  cjne   a,#013h,outcharlcd6  ;geen cursoroff
              mov    a,#cursoroff         ;"cursor uitschakelen"commando
              sjmp   outcharlcd9

outcharlcd6:  cjne   a,#00dh,outcharlcd7  ;geen 'cursor home '
              mov    a,#cursathom         ;"cursor in home positie"commando
              sjmp   outcharlcd9

outcharlcd7:  cjne   a,#00ah,outcharlcde  ;geen line feed, terug zonder actie
              mov    a,#0c0h             ;DDRAM adres:1ste plaats 2de lijn 
              sjmp   outcharlcd9          
              

; Dit label wordt uitgevoerd als de code in de accumulator een  ascii code
; is van een afdrukbaar karakter, deze code mag dan naar het data register.

outcharlcd8:  lcall  outd          ;display het karakter op het LCD
              sjmp   outcharlcde   ;einde routine

; De volgende instructies worden uitgevoerd als de hoogste bit van de code in 
; de accu geset is. Dit geeft immers aan dat de 7 laagste bits een adres 
; voorstellen. De code in de accu is dan meteen ook een commando om de cursor 
; op dat adres te zetten.

outcharlcd9:  lcall  outc          ;klaar
outcharlcde:  pop    psw           ;registers herstellen
              pop    acc
              ret

;******************************************************************************
;
; outniblcd     
; Zal de lage nibble omzetten in een ascii code en die afdrukken
; op het lcd scherm.
;
; de routine vernietigt geen registers
;
;******************************************************************************

outniblcd:    push   acc           ;registers op stack zetten
              push   psw
              anl    a,#00fh       ;laagste 4 bits overhouden
              add    a,#030h       ;omvormen naar ascii codes 0-9
              cjne   a,#03ah,outniblcd1   ;als groter dan dit nog 7 bijtellen
outniblcd1:   jc     outniblcde    ;als carry dan omzetting klaar
              add    a,#007h       ;omvormen naar hoofdletter
outniblcde:   lcall  outd          ;naar het scherm
              pop    psw
              pop    acc
              ret

;******************************************************************************
;
; outbytelcd    
; Zal de inhoud van de accu als hex getal afdrukken op het lcd scherm.
;
; de routine vernietigt geen registers
;
;******************************************************************************

outbytelcd:   push   acc           ;registers op de stack zetten
              push   psw
              swap   a             ;bitjes omruilen
              lcall  outniblcd
              swap   a             ;nog eens
              lcall  outniblcd
              pop    psw
              pop    acc
              ret

;******************************************************************************
;
; dispdptrlcd   
; Zal de inhoud van de dptr als hex getal afdrukken op het lcd scherm
;
; de routine vernietigt geen registers
;
;******************************************************************************

dispdptrlcd:  push   acc           ;registers bewaren op stack
              push   psw
              mov    a,dph         ;hoge deel eerst naar het scherm
              lcall  outbytelcd
              mov    a,dpl         ;dan het lage deel
              lcall  outbytelcd
              pop    psw
              pop    acc
              ret

;******************************************************************************
;
; outmsgalcd  
; Zal een string ascii codes op het scherm afdrukken. Het startadres
; van de string staat in de dptr. De string wordt afgesloten met een 000h.
; De routine gebruikt outcharlcd voor het sturen van karakters naar het LCD
; scherm. De zelf aangemaakte user karakters 00h-07h (via de build routine) en
; de positionnering van de cursor en cursorcontrole kan in de ascii string
; opgenomen worden met de pseudo ascii codes (zie outcharlcd).
;
; de routine vernietigt geen registers
;
;******************************************************************************

outmsgalcd:   push   acc           ;registers op de stack
              push   psw
              push   dph           ;ook de data pointer
              push   dpl

;We lezen karakters uit code memory tot de 000h code gevonden wordt.

outmsgalcd1:  clr    a             ;waarde zit in onze weg
              movc   a,@a+dptr     ;waarde lezen
              jz     outmsgalcde   ;als 0 dan naar einde springen

; de code is niet 0 dus versturen met outchar

              lcall  outcharlcd    ;weg er mee
              inc    dptr          ;naar volgende ascii code wijzen
              ljmp   outmsgalcd1   ;lus sluiten

outmsgalcde:  pop    dpl           ;registers herstellen
              pop    dph
              pop    psw
              pop    acc
              ret

;******************************************************************************
;
; regtolcd    
; Is een subroutine die de ASCII gegevens uit een interne buffer, die een copie
; is van de tekst die op het volledige scherm moet komen, naar de lcd stuurt. 
; De routine gaat er van uit dat er op het scherm maar 16 karakters per lijn 
; beschikbaar zijn. De string ascii codes in het geheugen moet ook 32 karakters
; bevatten. De routine verwacht in r0 het startadres van de buffer.
;
; de routine vernietigt geen registers
;
;******************************************************************************


regtolcd:     push   acc           ;registers op de stack
              push   psw
              mov    a,r0
              push   acc
              mov    a,r1
              push   acc
              mov    a,#80h          ;dit is het eerste adres op de eerstelijn
              lcall  outcharlcd    ;hiermee plaatsen we de cursor op de juiste
                                   ;plaats
              mov    r1,#000h      ;werkt als loopcounter
regtolcd1:    mov    a,@r0         ;karakter lezen
              lcall  outcharlcd    ;weg er mee
              inc    r0
              inc    r1
              cjne   r1,#016d,regtolcd1   ;als deze waarde 16 is moet een
                                          ;nieuwe lijn genomen worden
              mov    a,#(80h+40h)  ;dit is het eerste adres op de tweede lijn
              lcall  outcharlcd    ;hiermee plaatsen we de cursor op de juiste
                                   ;plaats
              
regtolcd2:    mov    a,@r0         ;karakter lezen
              lcall  outcharlcd    ;weg er mee
              inc    r0
              inc    r1
              cjne   r1,#032d,regtolcd2   ;als deze waarde 32 is stoppen
              pop    acc
              mov    r1,a
              pop    acc
              mov    r0,a
              pop    psw           ;registers herstellen
              pop    acc
              ret

;******************************************************************************
;
; outd 
; Is een routine die naar het data register van het display een
; volledige byte stuurt.
; Deze routine kan oa. nuttig gebruikt worden om karakters met code 0a0h - offh 
; (de aziatische karakterset en de Griekse symbolen) of de user gedefinieerde
; karakters (via build routine) met code 00h-07h af te drukken op het lcdscherm
; Vb:  mov a,#0E0H  ; code van het alfa symbool (zie karakterset LCD)
;      lcall  outd  ; druk alfa af op de plaats van de cursor
; 
; De routine vernietigt geen registers.
;
;******************************************************************************

outd:         push   acc           ;registers op stack zetten
              push   psw
              lcall  outhnibd      ;hoogste vier bits verzenden
ifdef     aduc832_v1_1
              lcall  delay60us     ;niet echt nodig
endif
              swap   a             ;bitjes omruilen
              lcall  outhnibd      ;naar het scherm sturen
              
ifdef    aduc832_v1_1              
              lcall  delay60us     ;altijd goed
endif              
              
              pop    psw           ;registers terug herstellen
              pop    acc
              ret

;******************************************************************************
;
; outhnibd 
; Stuurt de hoogste 4 bits van de accu naar het data register van het
; lcd scherm.
; Een databyte naar het lcd scherm sturen gebeurt in twee maal via deze routine.
; de routine vernietigt geen registers.
;
;******************************************************************************
ifdef     aduc832_v1_1
outhnibd:     push   acc           ;registers op stack plaatsen
              push   psw
              setb   rs            ;controle lijn in orde
              clr    e             ;deselectie display
              mov     c,acc.7      ;4 meest beduidende bits naar
              mov     lcdport.7,c  ;de lcd poort sturen en van bit1 en bit0
              mov     c,acc.6      ;afblijven!
              mov     lcdport.6,c
              mov     c,acc.5
              mov     lcdport.5,c
              mov     c,acc.4
              mov     lcdport.4,c
              setb   e             ;display enabelen
              nop                  ;450ns is normaal gezien voldoende
              clr    e             ;display deselecteren
              pop    psw           ;registers herstellen
              pop    acc
              ret
endif

ifdef    aduc832_v2_0
outhnibd:     push   acc           ;registers op stack plaatsen
              push   psw
              push   b             ;werkregister
              lcall  lcdtoport     ;lees de expander naar b register
              setb   rs            ;controle lijn in orde
              clr    e             ;deselectie display
              anl    a,#0f0h       ;hoogste 4 bits afzonderen
              rr     a             ;bits op juiste plaats zetten
              rr     a             ;voor b reg
              anl    b,#11000011b  ;bits laag maken, worden vervangen door data in a
              orl    b,a           ;bij elkaar voegen
              lcall  porttolcd     ;naar expander              
              setb   e             ;display enabelen
              lcall  porttolcd
              clr    e             ;display deselecteren
              lcall  porttolcd
              pop    b
              pop    psw           ;registers herstellen
              pop    acc
              ret
endif
;******************************************************************************
;
; outc 
; Is een routine die naar het controleregister van het display een
; volledige byte (commando) stuurt.
; De routine vernietigt geen registers.
;
;******************************************************************************

outc:         push   acc           ;registers op stack zetten
              push   psw
              lcall  outhnibc      ;hoogste vier bits verzenden
              lcall  delay2ms      ;niet echt nodig
              swap   a             ;bitjes omruilen
              lcall  outhnibc      ;naar het scherm sturen
              lcall  delay2ms      ;altijd goed voor traagste commando
              pop    psw           ;registers terug herstellen
              pop    acc
              ret

;******************************************************************************
;
; outhnibc 
; Deze routine stuurt de hoge 4 bits van de accu naar het controle register van 
; het lcd scherm.
; Een controlebyte naar het lcd scherm sturen gebeurt in twee maal via deze routine.
; de routine vernietigt geen registers.
;
;******************************************************************************
ifdef     aduc832_v1_1
outhnibc:     push   acc           ;registers op stack plaatsen
              push   psw
              clr    rs            ;controle lijn in orde
              clr    e             ;deselectie display
              mov     c,acc.7      ;4 meest beduidende bits naar
              mov     lcdport.7,c  ;de lcd poort sturen en van bit1 en bit0
              mov     c,acc.6      ;afblijven!
              mov     lcdport.6,c
              mov     c,acc.5
              mov     lcdport.5,c
              mov     c,acc.4
              mov     lcdport.4,c
              setb   e             ;display enabelen
              nop                  ;450ns is normaal gezien voldoende
              clr    e             ;display deselecteren
              pop    psw           ;registers herstellen
              pop    acc
              ret
endif

ifdef    aduc832_v2_0
outhnibc:     push   acc           ;registers op stack plaatsen
              push   psw
              push   b             ;werkregister
              lcall  lcdtoport     ;lees de expander naar b register
              clr    rs            ;controle lijn in orde
              clr    e             ;deselectie display
              anl    a,#0f0h       ;hoogste 4 bits afzonderen
              rr     a             ;bits op juiste plaats zetten
              rr     a             ;voor b reg
              anl    b,#11000011b  ;bits laag maken, worden vervangen door data in a
              orl    b,a           ;bij elkaar voegen
              lcall  porttolcd     ;naar expnder              
              setb   e             ;display enabelen
              lcall  porttolcd
              clr    e             ;display deselecteren
              lcall  porttolcd
              pop    b
              pop    psw           ;registers herstellen
              pop    acc
              ret
endif

;******************************************************************************
; hexbuf2lcd  
; Deze debug routine laat toe om op het scherm een interne geheugen-
; buffer van 4 bytes af te drukken in hexadecimaal formaat.
; Dit is vooral handig in een debug fase van een userprogramma.
; Het startadres van de buffer wordt doorgegeven via R0.
; De buffer wordt met de nodige info afgedrukt op de twee lijnen van de LCD
; zoals hieronder met een voorbeeld wordt aangegeven.
; 
; ADR| HEX DATA  |
; 42 |AF 2D 23 5A|
;
; Deze routine vernietigt geen registers
;******************************************************************************

hexbuf2lcd:   push   acc
              push   b
              push   psw
              push   dpl
              push   dph
              mov     a,r0
              push   acc
              mov    dptr,#hexbuftxt ;eerst de infotekst schrijven
              lcall   outmsgalcd
              mov    a,r0            ;neem pointerwaarde
              lcall  outbytelcd      ;byte afdrukken als startadres 
              mov    a,#' '          ;spatie afdrukken
              lcall  outcharlcd
              mov    a,#'|'          ;pipe afdrukken
              lcall  outcharlcd
              mov    b,#4            ;bytecounter=4
hexbuf2lcd1:  mov    a,@r0           ;hex byte ophalen
              lcall  outbytelcd      ;byte naar buiten
              mov    a,#' '          ;neem whitespace char
              lcall  outcharlcd       ;druk af
              inc    r0              ;volgende byte
              djnz   b,hexbuf2lcd    ;4 bytes op een lijn
;Hex.bytes staan nu op lcd, nu nog pipe erachter zetten
              mov    a,#0cfh         ;adres laatste char
              lcall   outcharlcd
              mov    a,#'|'          ;neem pipe
              lcall  outcharlcd      ;ascii code afdrukken
hexbuf2lcde:  pop    acc
              mov   r0,a
              pop   dph
              pop   dpl
              pop   psw
              pop   b
              pop   acc
              ret
                        ;'0123456789abcdef'
hexbuftxt:    db 13h,80h,'ADR| HEX DATA  |',0c0h,0


;******************************************************************************
; barlcd 
; Deze routine vormt een horizontale bargraph op het LCD scherm
; De startlocatie (00h-0fh of 40h-4fh) van de bargraph wordt aangegeven door 
; de inhoud van A
; Het actuele aantal af te drukken bar's "|" wordt aangegeven door de inhoud 
; van B. Het maximum aantal bar's dat moet kunnen afgedrukt worden, wordt 
; aangegeven door R0. Via build moeten eerst de nodige spaciale char's worden 
; aangemaakt in de LCD CGRAM.
; Deze speciale chars moeten op locaties 0 tot 4 zitten.
; vb:    mov  dptr,#barchars   ;maak speciale chars aan
;       lcall build
;       mov     r0,#40      ;we willen maximum 40 bars gebruiken als bargraph
;       mov     a, #42h     ;de startlocatie is 42h
;       mov  b, #20         ;de bargraph is tot de helft gevuld
;       lcall barlcd        ;geef de bargraph weer op het dislpay
;
; Deze routine vernietigt geen registers.
;******************************************************************************

barlcd:       push   dpl
              mov     dpl,r0
              push   dpl
              push   acc            ;bewaar startlocatie
              push   psw            ;bewaar status
              push   b              ;bewaar aantal bar's
              mov    b,a            ;bewaar startlocatie
;Bereken het aantal karakters (5x8) die maximum nodig zijn voor de bargraph
              mov    a,r0           ;neem maximum aantal bar's
              push   b              ;bewaar startadres
              mov    b,#5           ;max aantal bar's/ aantal bars per blokje
              div    ab             ;bereken dus het aantal nodige blokjes
              mov    r0,a           ;initaliseer hiermee de karaktercounter
              mov    a,b            ;neem restbars
              jz     barlcd_1       ;als nul dan verder
              inc    r0             ;als rest niet nul,an 1 char meer nemen
barlcd_1:     pop    acc            ;startadres terugnemen in ACCU
              orl    a,#10000000b   ;maak er een set DDRAM address commando van
              lcall  outcharlcd     ;plaats de cursor op het gevraagde adres
;We berekenen het aantal volle 5x8 blokjes en eventueel de rest
              mov    a,#13h         ;cursor off
              lcall  outcharlcd     ;stuur naar LCD
              pop    b              ;aantal af te drukken bar's terugnemen
              push   b              ;en weer bewaren
              mov    a,b            ;bereken aantal volle blokjes, en de rest
              jz     barlcd3        ;als gevraagde aantal bar's nul is: wissen
              mov    b,#5           ;5 bars per karakter= maximum
              div    ab             ;A=aantal volle blokjes, B=rest
              push   b              ;bewaar rest
              jz     barlcd2        ;geen volle blokjes: dan rest afdrukken
              mov    b,a            ;blokjescounter=aantal volle blokjes

;Druk de volle 5x8 blokjes af van de bargraph
              mov    a,#4           ;neem code voor vol blokje
barlcd1:      lcall  outd           ;stuur vol blokje naar het scherm
              dec    r0             ;verminder karaktersteller
              djnz   b,barlcd1      ;doe dit voor aantal=blokjescounter

;Druk het onvolledige blokje (rest) af
barlcd2:      pop    acc            ;haal rest weer op
              jz     barlcd3        ;is de rest nul, dan gedaan
              dec    a              ;converteer rest naar code voor de 
                                    ;overeenkomende bar's           
              lcall  outd           ;druk resterende bar's af
              dec    r0             ;verminder karakterteller

;Test of we aan het einde van de bargraph zitten..
barlcd3:      cjne   r0,#0,barlcd4  ;karakterteller niet nul: spaces afdrukken
              sjmp   barlcde        ;als de karakterteller nul is, dan klaar!

;Spaties afdrukken tot einde bargraph om vorige bargraph te overschrijven!
barlcd4:      mov    a,#' '         ;neem spatie
              lcall  outd           ;en druk af
              dec    r0             ;karakterteller verminderen
              sjmp   barlcd3        ;druk spaties af tot einde bargraph 

barlcde:      pop    b              ;haal aantal afgedrukte bar's weer
              pop    psw            ;haal status weer
              pop    acc            ;haal startlocatie weer
              pop    dpl
              mov    r0,dpl
              pop    dpl
              ret



;******************************************************************************
; barchars 
; Dit zijn de 5 char's die nodig zijn voor de routine barlcd die een
; bargraph vormt op het LCD scherm. Deze char's moeten eerst door de gebruiker 
; via de build routine in de CGRAM van de LCD module worden geplaatst, en 
; kunnen pas daarna worden gebruikt.
; Indien de gebruiker zelf nog andere speciale char's nodig heeft, kunnen deze 
; achter deze 5 char's worden geplaatst via de build_adr routine 
; (max. 3 extra char's). 
; De gebruiker moet er op letten dat de laatste byte van de laatste char wordt 
; opgeslagen met MSB=1.
;******************************************************************************

barchars:     db 00010000b    ;1 vertikale bar links in pixelruimte 5x8
              db 00010000b
              db 00010000b
              db 00010000b
              db 00010000b
              db 00010000b
              db 00010000b
              db 00010000b 

              db 00011000b    ;2 vertikale bars links in pixelruimte 5x8
              db 00011000b
              db 00011000b
              db 00011000b
              db 00011000b
              db 00011000b
              db 00011000b
              db 00011000b 

              db 00011100b    ;3 vertikale bars links in pixelruimte 5x8
              db 00011100b
              db 00011100b
              db 00011100b
              db 00011100b
              db 00011100b
              db 00011100b
              db 00011100b 

              db 00011110b    ;4 vertikale bars links in pixelruimte 5x8
              db 00011110b
              db 00011110b
              db 00011110b
              db 00011110b
              db 00011110b
              db 00011110b
              db 00011110b 

              db 00011111b    ;volledig zwarte pixelruimte 5x8
              db 00011111b
              db 00011111b
              db 00011111b
              db 00011111b
              db 00011111b
              db 00011111b
bareinde:     db 10011111b    ;opgelet dit is de laatste byte, hier is MSB=1!

;******************************************************************************
;
; build
; Is een routine die de karaktergenerator RAM (CGRAM) vanaf het startadres 00h 
; zal laden met de bitpatronen voor maximaal 8 karakters, die dan daarna 
; overeen komen met de ASCII codes 00h-07h. 
; De routine gebruikt een door de gebruiker aangemaakte bitmap op het adres 
; aangegeven in DPTR. Deze bitmap wordt afgesloten door in de laatste byte van 
; de laatste aan te maken karakter bit7 op  1 te zetten. 
; Na het schrijven van de bitmap in CGRAM wordt terug de data display RAM 
; (DDRAM) geselecteerd en ingesteld op adres 00H
;
; De routine vernietigt geen registers.
;
; Vb.  mov dptr,#eigenkarakter   ; wijs naar de bitmap met eigen karakters
;      lcall  build              ; en maak ze aan in de CGRAM vanaf positie 0
;      mov    a,#00h             ; we gaan zelfgemaakte karakter 0 afdrukken
;      lcall  outcharlcd         ; druk de karakter af
;
;eigenkarakter:
;      db 00001110b
;      db 00001010b
;      db 00001110b
;      db 00000100b
;      db 00011111b
;      db 00000100b
;      db 00001010b
;      db 10010001b              ;laatste byte van onze enige karakter(msb=1)
;******************************************************************************

build:        push   acc           ;bewaar de gebruikte registers
              push   psw
              mov     a,#CGRAM     ;Zet adres van de CGRAM op 0
              lcall  build_adr
              pop    psw
              pop    acc
              ret                  ;terug
              
;******************************************************************************              
; build_adr
; Is een routine die de karaktergenerator RAM (CGRAM) vanaf het adres aan-
; gegeven in de ACCU zal laden met de bitpatronen voor maximaal 8 karakters, 
; die overeen komen met de ASCII codes 00h-07h. 
; De routine gebruikt een door de gebruiker aangemaakte bitmap op het adres 
; aangegeven in DPTR. Deze bitmap wordt afgesloten door in de laatste byte van 
; de laatste aan te maken karakter bit7 op  1 te zetten. 
; Na het schrijven van de bitmap in CGRAM wordt terug de data display RAM 
; (DDRAM) geselecteerd en ingesteld op adres 00H
;
; De routine vernietigt geen registers.
;
; Vb.  mov  a,#05h              ; maak userchar op de zesde locatie in CGRAM
;      mov dptr,#eigenkarakters ; wijs naar de bitmap met eigen karakters
;      lcall  build_adr         ; en maak ze aan in de CGRAM vanaf positie 05h
;      mov    a,#05h            ; we gaan zelfgemaakte karakter 5 afdrukken
;      lcall  outcharlcd        ; druk de karakter af
;
;eigenkarakters:
;      db 00001110b
;      db 00001010b
;      db 00001110b
;       db 00000100b
;      db 00011111b
;      db 00000100b
;      db 00001010b
;      db 10010001b             ;de laatste byte van onze enige karakter    
;******************************************************************************          
build_adr:      push   acc
              push   psw
              push   dph
              push   dpl
              orl    a,#01000000b  ;maak er een adrescommando van
build1:       lcall  outc          ;NAAR SCHERM VERSTUREN
build3:       mov    a,#000h       ;ACCU OP NUL ZETTEN
              movc   a,@a+dptr     ;EERSTE CODE UIT TABEL LEZEN
              lcall  outd          ;NAAR KARAKTER GENERATOR RAM STUREN
              jb     acc.7,build2  ;ALS BIT 1 DAN EINDE
              inc    dptr          ;NAAR VOLGENDE LOKATIE WIJZEN
              sjmp   build3
build2:       mov    a,#DDRAM      ;zet adres DATA DISPLAY RAM op 0
              lcall  outc          ;NAAR KONTROLE REGISTER STUREN
              pop    dpl
              pop    dph
              pop    psw
              pop    acc
              ret
              
;******************************************************************************
;
; delay2ms 
; Is een vertragingsroutine van 2ms.
;
; De routine vernietigt geen registers.
;
;******************************************************************************

delay2ms:     push   acc           ;registers op de stack zetten
              push   psw
              mov    a,#032d       ;aantal keer 60us wachten
delay2ms1:    lcall  delay60us
              djnz   acc,delay2ms1
              pop    psw           ;registers herstellen
              pop    acc
              ret

;******************************************************************************
;
; delay60us.
; Deze routine genereert een delay van 60us als vertraging in de LCD driver.
; De tijdsvertraging klopt bij de hoogste kloksnelheid van de
; controller. Bij de traagste snelheid wordt de routine overgeslagen.
;
; de routine vernietigt geen registers.
;
;******************************************************************************

delay60us:    push   acc           ;registers op de stack plaatsen
              push   psw
              mov    a,pllcon      ;pllcon inlezen
              anl    a,#00000111b  ;beduidende bits overhouden

; Hoe kleiner de waarde in de pllcon, hoe sneller de controller loopt.
; door wat bitbangen maken we er een bruikbare waarde van.

              cjne   a,#00000111,delay60us2  ;als traagste snelheid stoppen
              pop    acc           ;registers herstellen
              pop    psw
              ret

delay60us2:   mov    a,#25d        ;loopcounter laden
delay60us1:   djnz   acc,delay60us1 ;telkens twee cycles tijd verliezen
              pop    psw
              pop    acc
              ret
              
endif        ;einde van aduc_lcd




ifdef aduc_i2c              
;******************************************************************************
;
; Dit is een I2C driver voor een ADuC8xx microcontroller.
; De driver bevat alle routines om de ADuC als single master te gebruiken.
; Als I/O worden de specifieke I2C pinnen sdata en sclock gebruikt. 
; 
; Door de hardwarematige beperking dat de fysische pinnen van de sclock-
; en sdatalijn niet kunnen gelezen worden als ze als output worden gebruikt 
; is multimaster werking met arbittrage en clockstretching niet mogelijk. 
;
; Origineel geschreven door Roggemans Marc (MGM) op 03/2005 v0
; Toevoegingen van high level routines door [dp] op 01/2014
;
; Beschikbare routines:
; i2cinit      :klaar zetten van de i2c bus voor gebruik
; i2cstart     :verzenden van een start conditie
; i2cstop      :verzenden van een stop conditie
; i2coutbyte   :verzenden van 1 byte vanuit de accu, er wordt een negende klok
;               verzonden, in de carry wordt de ack van de slave weergegeven
; i2cinbyteack :leest een byte van de i2c bus en genereert een ack op de
;               negende klokpuls
; i2cinbytenack :leest een byte van de i2c bus en genereert een nack op de 
;               negende klokpuls
;
; Toevoeging [dp]
; i2csenddata  :adresseert en stuurt de inhoud van een geheugenbuffer naar 
;               een slavedevice
; i2crcvdata   :adresseert en ontvangt één of meerdere databytes van een slave
;               en plaatst deze in een geheugenbuffer
;******************************************************************************
i2cpcf8574		equ		01000000b		;port expander basisadres gewone versie
i2cpcf8574A		equ		01110000b		;port expander basisadres A versie
;pcf8574		set		1				;bij gewone portexpander met adres 40h
pcf8574A		set		1				;bij A versie portexpander met adres 70h

;******************************************************************************
; i2cinit       
; Dit is een subroutine die de i2c bus van de ADuC als master instelt
;******************************************************************************
i2cinit:        setb    mdo             ;data moet hoog wakker worden
                setb    mco             ;klok moet hoog wakker worden
                setb    mde             ;data wordt als output geschakeld
                setb    i2cm            ;master mode inschakelen
                ret
;******************************************************************************
; i2cstart      
; Dit is een subroutine die de start conditie op de pinnen plaatst
;******************************************************************************
i2cstart:       setb    mco             ;zeker zijn dat de klok hoog is
                clr     mdo             ;data moet laag worden
                setb    mde             ;data pin enable
                clr     mco             ;klok laag maken
                ret
;******************************************************************************
; i2cstop       
; Dit is een subroutine die de stop conditie opwekt.
;******************************************************************************
i2cstop:        clr     mdo             ;data moet laag worden
                setb    mde             ;data mag naar buiten
                setb    mco             ;nu moet de klok omhoog
                setb    mdo             ;nu data pin hoog maken
                ret                

;******************************************************************************
; i2coutbyte    
; Deze routine stuurt één databyte (MSbit eerst) naar een I2C slave device.
; De routine vernietigt geen registers.
; input : De te versturen byte wordt doorgegeven in ACCU
; output: De carry zal de waarde van de ack bit bevatten na het
;         uitvoeren van de routine.
;******************************************************************************
i2coutbyte:     push    acc             ;worden effe op stack bewaard
                push    b
                mov     b,#008h         ;loopcounter
i2coutbyte1:    rlc     a               ;bit in de carry plaatsen
                mov     mdo,c           ;carry naar de mdo pin
                setb    mde             ;pin als output activeren
                setb    mco             ;klokpukls hoog
                clr     mco             ;klokpuls laag
                djnz    b,i2coutbyte1   ;volledige byte verzenden
                clr     mde             ;iic data pin als input schakelen
                setb    mco             ;klokpuls hoog
                mov     c,mdi           ;data inlezen in de carry
                clr     mco             ;klokpuls terug laag
                pop     b
                pop     acc             ;registers herstellen
                ret

;******************************************************************************
; i2cinbyteack  
; Deze routine zal in de accu een byte inlezen, msb eerst.
; De routine vernietigt geen registers. De routine zal als
; negende bit een ack verzenden.
;******************************************************************************
i2cinbyteack:   push    b               ;deze registers worden gebruikt
                push    psw
                mov     b,#008h         ;wordt gebruikt als loopcounter
i2cinbytea1:    clr     mde             ;iic data pin als input schakelen
                setb    mco             ;klokpuls hoog
                mov     c,mdi           ;data inlezen in de carry
                clr     mco             ;klokpuls terug laag
                rlc     a               ;in accu shiften
                djnz    b,i2cinbytea1   ;volledige byte ontvangen
                clr     c               ;ack verzenden
                mov     mdo,c           ;carry naar de mdo pin
                setb    mde             ;pin als output activeren
                setb    mco             ;klokpukls hoog
                clr     mco             ;klokpuls laag
                pop     psw             ;registers herstellen
                pop     b
                ret

;******************************************************************************
; i2cinbytenack 
; Deze routine zal in de accu een byte inlezen, msb eerst.
; De routine vernietigt geen registers. De routine zal als
; negende bit een nack verzenden.
;******************************************************************************
i2cinbytenack:  push    b               ;deze registers worden gebruikt
                push    psw
                mov     b,#008h         ;wordt gebruikt als loopcounter
i2cinbyten1:    clr     mde             ;iic data pin als input schakelen
                setb    mco             ;klokpuls hoog
                mov     c,mdi           ;data inlezen in de carry
                clr     mco             ;klokpuls terug laag
                rlc     a               ;in accu shiften
                djnz    b,i2cinbyten1   ;volledige byte ontvangen
                setb    c               ;nack verzenden
                mov     mdo,c           ;carry naar de mdo pin
                setb    mde             ;pin als output activeren
                setb    mco             ;klokpukls hoog
                clr     mco             ;klokpuls laag
                pop     psw             ;registers herstellen
                pop     b
                ret        

;******************************************************************************
; i2csenddata 
; Adresseert en stuurt één of meerdere bytes over de I2C bus naar een 
; slave device op de bus. Het 7 bit slave-adres, met de Write aanduiding "0" 
; op bitpositie 0 wordt doorgegeven via dph. Bij 10 bit adressering (indien het
; aan te spreken I2C device dit vereist) zal het 7 bit adres in dph '11110xx' 
; zijn met de write aanduiding '0'. Er worden hier maar twee nuttige adresbits
; doorgegeven. De volgende 8 bits (LSB) van het 10bit adres moeten in dit geval 
; worden doorgegeven via dpl. Bij 7 bit adressering wordt dpl niet gebruikt.
;
; Het te versturen datablok moet in inwendig geheugen staan en R0 is een 
; pointer naar deze data.  .

; Via B wordt aangegeven hoeveel bytes moeten worden verzonden uit het datablok
; input : dph,dpl = slave adres, 7bits (dph) of 10bits (+dpl) met Write (bit=0)
;         B       = aantal te versturen bytes (minimum 1)
;         R0      = startadres van te versturen datablok 
;                   
; output: cy=0 :Verzenden is ok en R0 = startadres van de verstuurde data
;         cy=1 :Wanneer er geen ack bit komt van de slave wordt de Cy bit 
;               geset bij return. R0 wijst in dit geval naar de laatste 
;               verzonden byte waarvoor er geen ack kwam.
; 
; Deze routine vernietigt niets...
;******************************************************************************
i2csenddata:        push   acc           ;registers bewaren
                    push   psw
                    mov    a,r0
                    push   acc           ;r0 op stack
                    
                    lcall  i2cstart      ;genereer startconditie
                    mov    a,dph         ;neem adresbyte 
                    lcall  i2coutbyte    ;en verstuur naar slave
                    jc     i2csenderror  ;indien geen ack, fout!
                    mov    a,dph         ;7bit of 10bit adresbyte
                    anl    a,#11111001b  ;maskeer de adresbits bij 10bit
                    cjne   a,#0f0h,i2csenddata1 ;10 bit adressering?(11110xx0)
;10 bit adressering!
                    mov    a,dpl         ;neem low byte van het 10bit adres
                    lcall  i2coutbyte    ;en verstuur naar slave
                    jc     i2csenderror  ;indien geen ack, fout!
;7 bit adressering!
i2csenddata1:       mov    a,@r0         ;haal databyte uit geheugenbuffer
                    lcall  i2coutbyte    ;en verstuur naar slave
                    jc     i2csenderror  ;indien geen ack, fout!
                    inc    r0            ;volgende byte
                    djnz   b,i2csenddata1 ;zijn alle bytes verzonden?
                    lcall  i2cstop        ;klaar, stuur een stop conditie
                    pop    acc    
                    mov    r0,a          ;r0 wijst naar start van data
                    pop    psw                
                    clr    c             ;alles is ok!
                    pop    acc
                    ret
;fout: geen ack gekregen bij het versturen van het adres of data                    
i2csenderror:       lcall  i2cstop      ;stopconditie genereren
                    pop    acc          ;dummy, r0 niet herstellen
                    pop    psw
                    setb   c            ;terug met foutmelding
                    pop    acc
                    ret
                    
;******************************************************************************
; i2crcvdata 
; Adresseert en ontvangt één of meerdere bytes over de I2C bus van een 
; slave device op de bus. Het 7 bit slave-adres, met de Read aanduiding "1" 
; op bitpositie 0 wordt doorgegeven via dph. Bij 10 bit adressering (indien het
; aan te spreken I2C device dit vereist) zal het 7 bit adres in dph '11110xx' 
; zijn met de Read aanduiding '1'. Er worden hier maar twee nuttige adresbits
; doorgegeven. De volgende 8 bits (LSB) van het 10bit adres moeten in dit geval 
; worden doorgegeven via dpl. Bij 7 bit adressering wordt dpl niet gebruikt.
;
; Het te ontvangen datablok zal in inwendig geheugen geplaatst worden en R0 
; is een pointer naar de start van dit ontvangen datablok. 
; 
; Via B wordt aangegeven hoeveel bytes moeten worden ontvangen
; input : dph,dpl = slave adres, 7bits (dph) of 10bits (+dpl) met Read (bit=1)
;         B       = aantal te ontvangen bytes (minimum 1)
;         R0      = startadres waar de ontvangen data wordt opgeslagen 
;
; output: cy=0 :Verzenden is ok en R0 = startadres van de ontvangen data
;         cy=1 :Wanneer er geen ack bit komt van de slave bij het versturen van
;               het adres, wordt de CY bit geset bij return. 
;               R0 = startadres van de buffer voor de te ontvangen data
;
; Deze routine vernietigt verder niets...
;******************************************************************************
i2crcvdata:         push    acc             ;registers op stack
                    push    psw
                    mov     a,r0            ;r0 op stack
                    push    acc
                    
                    lcall   i2cstart        ;genereer startconditie
                    mov     a,dph           ;neem adresbyte 
                    lcall   i2coutbyte      ;en verstuur naar slave
                    jc      i2crcverror     ;indien geen ack, fout!
                    mov     a,dph           ;7bit of 10bit adresbyte
                    anl     a,#11111001b    ;maskeer de adresbits bij 10bit
                    cjne    a,#0f1h,i2crcvdata1 ;10 bit adressering?(11110xx1)
;10 bit adressering!
                    mov     a,dpl           ;neem low byte van het 10bit adres
                    lcall   i2coutbyte      ;en verstuur naar slave
                    jc      i2crcverror     ;indien geen ack, fout!
;7 bit adressering!    
                    
i2crcvdata1:        djnz    b,i2crcvdata2   ;niet nul, byte lezen met ack!
                    lcall   i2cinbytenack   ;de laatste byte lezen met nack!
                    mov     @r0,a           ;en deze ook bewaren
                    sjmp    i2crcvdata3
i2crcvdata2:        lcall   i2cinbyteack    ;lees byte van slave en geef ack
                    mov     @r0,a           ;bewaar de data in buffer
                    inc     r0              ;pointer verhogen
                    sjmp    i2crcvdata1      ;volgende byte ontvangen
                    
i2crcvdata3:        lcall   i2cstop         ;klaar, stuur een stopconditie
                    pop     acc    
                    mov     r0,a            ;r0 wijst naar start van data
                    pop     psw                
                    clr     c               ;alles is ok!
                    pop     acc
                    ret
;fout: geen ack gekregen bij het versturen van het adres
i2crcverror:        lcall    i2cstop        ;stopconditie genereren
                    pop        acc          ;dummy,r0 is nog niet veranderd
                    pop        psw
                    setb    c               ;terug met foutmelding
                    pop        acc
                    ret


;******************************************************************************
;
; porttolcd          
; Dit is een subroutine die de inhoud van het b register naar het lcd
; scherm stuurt via de port expander.
;
; Gebruikt geen registers
;
;******************************************************************************

porttolcd:    push   acc           ;geen registers gebruiken
              push   psw           ;trekken ons niks aan van ack bit
              lcall  i2cstart      ;vertrokken
ifdef    pcf8574			  
              mov    a,#01000000b  ;schrijven naar expander
endif
ifdef    pcf8574A			  
              mov    a,#01110000b  ;schrijven naar expander
endif
              lcall  i2coutbyte    ;weg er mee
              mov    a,b           ;data verzenden
              lcall  i2coutbyte
              lcall  i2cstop       ;transactie sluiten
              pop    psw
              pop    acc
              ret

;******************************************************************************
;
; lcdtoport          
; Dit is een routine die de waarde van de expander uitleest naar het b register 
; Op die manier kunnen we onthouden wat de waarde van de LCD pinnen is voor het 
; aansturen van de buzzer en backlight.
;
; output: b register
;
;******************************************************************************

lcdtoport:    push   acc           ;geen registers gebruiken
              push   psw           ;trekken ons niks aan van ack bit
              lcall  i2cstart      ;vertrokken
ifdef    pcf8574			  
              mov    a,#01000000b  ;schrijven naar expander
endif
ifdef    pcf8574A			  
              mov    a,#01110001b  ;schrijven naar expander
endif
              lcall  i2coutbyte    ;weg er mee
              lcall  i2cinbytenack ;data inlezen
              lcall  i2cstop
              mov    b,a
              pop    psw
              pop    acc
              ret

;******************************************************************************
;
; lcdlighton         
; Dit is een subroutine die de backlight inschakeld
;
; Gebruikt geen registers
;
;******************************************************************************

lcdlighton:   push   b
              lcall  lcdtoport     ;expander lezen
              clr    b.6           ;licht aan doen
              lcall  porttolcd     ;expander schrijven
              pop    b
              ret


;******************************************************************************
;
; lcdlightoff        
; Dit is een subroutine die de backlight uitschakeld
;
; Gebruikt geen registers
;
;******************************************************************************

lcdlightoff:  push   b
              lcall  lcdtoport     ;expander lezen
              setb   b.6           ;licht uit doen
              lcall  porttolcd     ;expander schrijven
              pop    b
              ret

;******************************************************************************
;
; lcdbuzon           
; Dit is een subroutine die de buzzer inschakeld
;
; Gebruikt geen registers
;
;******************************************************************************

lcdbuzon:     push   b
              lcall  lcdtoport     ;expander lezen
              clr    b.7           ;buzzer aan doen
              lcall  porttolcd     ;expander schrijven
              pop    b
              ret

;******************************************************************************
;
; lcdbuzoff          
; Dit is een subroutine die de buzzer uitschakeld
;
; Gebruikt geen registers
;
;******************************************************************************

lcdbuzoff:    push   b
              lcall  lcdtoport     ;expander lezen
              setb   b.7           ;buzzer aan doen
              lcall  porttolcd     ;expander schrijven
              pop    b
              ret
              
endif    ;einde aduc_i2c          
;******************************************************************************



ifdef    aduc_sio    ;driver voor de seriële poort (UART)           
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
; ADuCsio.inc 
; Is een include file voor de ADuC832 microcontroller.
; De file laat toe om de seriele poort te gebruiken en biedt de standaard
; monitorroutines zoals die beschikbaar zijn in de MGM monitors voor 8051.
; De file wordt achteraan in de .asm file ge-included. De belangrijkste
; afwijking van de standaard routines is de initsio subroutine.
; Die laat toe om 9600 baud in te stellen, maar zal rekening houden met de PLL
; waarde. Wil je de initsio routine aanroepen, dan moet de PLL reeds geladen
; zijn.
;
; Geschreven door Roggemans Marc (MGM) op 11/2004 v0
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

blank         equ    020h          ;ascii code voor een blank
xoff          equ    013h          ;ascii code xoff caracter (ctrl s)
ff            equ    00ch          ;ascii code voor nieuw blad
cr            equ    00dh          ;ascii code voor een wagenterugloop
lf            equ    00ah          ;ascii code voor volgende lijn
backsp        equ    008h          ;ascii code voor het bs karakter
beep          equ    007h          ;ascii code voor een beep

wissen:       db     backsp,blank,backsp,000h    ;string wissen karakter
                                                        ;van het scherm

strtbuf       equ     054h         ;start adres van de buffer
endbuf        equ     07fh         ;laatst gebruikte locatie (kan cr bevatten)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
; initsio zal de seriele poort klaar zetten voor gebruik. De baudrate is 9600.
; De routine maakt gebruik van de pllcon waarde om te bepalen hoe de timer 3
; als baud rate generator ingesteld moet worden. Indien de instelling niet
; mogelijk is wordt de carry geset. In het andere geval wordt de carry laag
; gemaakt. De routine is pedagogisch geschreven en kan dus korter
;
; de routine gebruikt de psw
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

initsio:      push   acc           ;registers op de stack zetten
              mov    a,pllcon      ;we lezen het pllcon SFR uit
              anl    a,#007h       ;alleen de laagste 3 bits tellen
              cjne   a,#006h,initsio1   ;005h is maximaal toegelaten
initsio1:     jnc    initsioe      ;geen carry dan is waarde in pllcon >=6

; de waarde in pllcon is bruikbaar. We gebruiken de waarde om de inhoud van
; t3con te bepalen. t3fd is altijd 02dh voor 9600 baud.

              mov    t3fd,#02dh    ;zie databoek t3 als baud rate generator
              cpl    a             ;bitbangen om laten uit te komen
              clr    c             ;verder doen
              subb   a,#002h       ;laagste 3 bits zijn 0k
              anl    a,#007h       ;hoogste 5 bits op 0
              add    a,#080h       ;nu alles in orde
              mov    t3con,a       ;baud rate klaar
              mov    scon,#01010000b ;UART initialiseren
              clr    c             ;aangeven init ok (overbodige instructie)
              pop    acc
              ret

initsioe:     setb   c             ;aangeven initialisatie niet mogelijk
              pop    acc
              ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
; OUTCHAR IS EEN SUBROUTINE DIE EEN KARAKTER VIA DE SERIELE INTERFACE NAAR
; BUITEN STUURT. DE ROUTINE GEBRUIKT DE ACCU ALS INPUT. DE ROUTINE GAAT NA DAT
; DE TRANSMITTER LEEG IS ALVORENS HET KARAKTER TE VERSTUREN.
;
; de routine gebruikt geen registers.
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

OUTCHAR:      MOV    SBUF,A        ;KARAKTER VERZENDEN
OUTCHAR1:     JNB    TI,OUTCHAR1   ;WACHT TOT ZENDER BESCHIKBAAR
              CLR    TI            ;LAAG MAKEN VAN DE BIT
              RET

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
; OUTNIB VERZEND DE LAAGSTE VIER BITS NAAR DE SERIELE POORT ONDER DE VORM VAN
; EEN ASCII CODE. DE ACCU EN DE VLAGGEN WORDEN DOOR DE ROUTINE GEBRUIKT.
;
; de routine gebruikt geen registers.
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

OUTNIB:       push   acc           ;registers op de stack
              push   psw
              ANL    A,#00FH       ;AFZONDEREN
              LCALL  BATRANS       ;OMZETTEN IN ASCII CODE
              LCALL  OUTCHAR       ;VERSTUREN
              LCALL  XONXOFF       ;FLOW CONTROL
              pop    psw           ;registers herstellen
              pop    acc
              RET

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
; OUTBYTE ZET DE HEX INHOUD VAN DE ACCU OP DE SERIELE POORT. ALS FLOW CONTROL
; WORDT XONXOFF GEBRUIKT.
;
; de routine gebruikt geen registers.
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

OUTBYTE:      push   acc           ;registers op de stack zetten
              push   psw
              PUSH   ACC           ;BACK UP NEMEN VAN DE ACCU
              SWAP   A             ;EERST HOOGSTE DEEL
              LCALL  OUTNIB        ;NIBBLE NAAR HET SCHERM
              POP    ACC           ;HERSTELLEN VAN DE ACCU
              LCALL  OUTNIB        ;NIBBLE NAAR HET SCHERM
              pop    psw           ;registers herstellen
              pop    acc
              RET

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
; DISPDPTR ZET DE HEX INHOUD VAN DE DPTR OP HET SCHERM OP DE HUIDIGE PLAATS
; VAN DE CURSOR.
;
; de routine gebruikt geen registers.
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

DISPDPTR:     push   acc           ;registers op de stack zetten
              push   psw
              MOV    A,DPH         ;EERST HOOGSTE DEEL OP SCHERM
              LCALL  OUTBYTE       ;BYTE IN ASCII NAAR HET SCHERM
              MOV    A,DPL         ;MINST BEDUIDENDE DEEL VERZENDEN
              LCALL  OUTBYTE       ;WEG ER MEE
              pop    psw           ;registers herstellen
              pop    acc
              RET                  ;EINDE

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
; OUTMSGA IS EEN SUBROUTINE, DIE DE ASCII CODES VANAF HET ADRES IN DE DPTR,
; TOT AAN DE CODE 00H, OP HET SCHERM ZET. HET BERICHT STAAT IN CODE MEMORY.
;
; de routine gebruikt geen registers.
; 
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

OUTMSGA:      push   acc           ;alles op de stack zetten
              push   psw
              push   dpl
              push   dph
outmsga2:     CLR    A             ;ACCU LEEG MAKEN
              MOVC   A,@A+DPTR     ;EERSTE CODE OPHALEN
              JZ     OUTMSGA1      ;ALS NUL DAN EINDE
              LCALL  OUTCHAR       ;KARAKTER NAAR HET SCHERM
              LCALL  XONXOFF       ;SOFTWARE HANDSHAKE
              INC    DPTR          ;DPTR AANPASSEN
              SJMP   OUTMSGA2      ;HERHAAL TOT EINDE
OUTMSGA1:     pop    dph           ;alles herstellen
              pop    dpl
              pop    psw
              pop    acc
              RET                  ;EINDE ROUTINE

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
; XONXOFF LAAT HET GEBRUIK VAN SOFTWARE HANDSHAKE TOE. DE ROUTINE GAAT NA DAT
; ER EEN KARAKTER ONTVANGEN WERD. ALS DIT HET XOFF KARAKTER IS BLIJFT DE
; ROUTINE WACHTEN OP EEN ANDER WILLEKEURIG KARAKTER. DIT LAATSTE WORDT NIET
; VERDER DOORGEGEVEN.
;
; de routine gebruikt geen registers.
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

XONXOFF:      push   acc           ;registers op de stack zetten
              push   psw
              JBC    RI,XONXOFF1   ;GA NA KARAKTER ONTVANGEN
XONXOFF2:     pop    psw           ;registers herstellen
              pop    acc
              RET                  ;ANDERS EINDE
XONXOFF1:     MOV    A,SBUF        ;ZO JA, KARAKTER LADEN IN ACCU
              CJNE   A,#XOFF,XONXOFF2   ;ALS NIET XOFF DAN VERDER
XONXOFF3:     JBC    RI,XONXOFF2   ;WACHTEN OP WILLEKEURIG KARAKTER
              SJMP   XONXOFF3      ;EN DAN RIBBE EN DE BIE

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
; INCHAR BLIJFT WACHTEN TOTDAT ER EEN KARAKTER ONTVANGEN WERD DOOR DE SERIELE
; INTERFACE. HET KARAKTER WORDT DOORGEGEVEN IN DE ACCUMULATOR.
;
; de routine gebruikt de accu.
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

INCHAR:       JNB    RI,INCHAR     ;BLIJF WACHTEN OP HET KARAKTER
              CLR    RI            ;BIT LAAG MAKEN
              MOV    A,SBUF        ;KARAKTER IN DE ACCU
              RET

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
; INBYTE LEEST TWEE ASCII CODES BINNEN EN VORMT ZE OM TOT EEN HEX BYTE.
; ALS OUTPUT WORDT DE ACCU GEBRUIKT.
; ALS HET DE TWEE ASCII CODES NIET TOT EEN BYTE OMGEZET KUNNEN WORDEN, ZAL
; DE CARRY OP 1 GEZET WORDEN.
;
; De routine gebruikt de accu EN PSW.
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

INBYTE:       PUSH     B
              LCALL  INCHAR         ;EERSTE KARAKTER INLEZEN
              LCALL  LOWUPTR
              LCALL  ASCBINTRANS    ;OMZETTEN NAAR HEX GETAL
              JC     INBYTE1        ;BIJ ERROR AKTIE ONDERNEMEN
              SWAP   A              ;IS HOOGSTE NIBBLE
              MOV    B,A            ;ACCU TIJDELIJK BEWAREN
              LCALL  INCHAR         ;TWEEDE KARAKTER OPHALEN
              LCALL  LOWUPTR
              LCALL  ASCBINTRANS    ;OMZETTEN NAAR BIN GETAL
              JC     INBYTE1        ;BIJ ERROR ....
              ORL    A,B            ;BIJDE GETALLEN SAMEN VOEGEN
INBYTE1:      POP     B
              RET

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
; INBUFA LEEST EEN ASCII BUFFER BINNEN VIA DE SERIELE POORT. DE ROUTINE LAAT
; TOE OM FOUTIEVE INGAVEN TE VERBETEREN. HIERVOOR KAN DE BACKSPACE TOETS
; GEBRUIKT WORDEN. HET STARTADRES VAN DE BUFFER IS STRTBUF. DE LAATSE BYTE
; HEEFT HET ADRES ENDBUF. ALS DE BUFFER TE VOL, OF TE LEEG, GEMAAKT WORDT,
; LAAT DE CPU EEN BEEP HOREN. DE ROUTINE GEBRUIKT DE ACCU EN R0. CONTROLE
; KARAKTERS WORDEN NIET VERWERKT. DE ROUTINE GEEFT OOK EEN ECHO NAAR HET
; SCHERM. ZE GEBRUIKT DE SUBROUTINES INCHAR, OUTCHAR EN ASCCTR. INBUFA DOET
; OOK DE OMVORMING NAAR ASCII 0 KARAKTERS. Door de waarden van strtbuf en endbuf
; aan te passen kan de gebruiker zelf de lengte en de plaats van de buffer
; bepalen.
;
; De routine gebruikt  r0 .
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

INBUFA:       push   acc            ;registers bewaren
              push   psw
              MOV    R0,#STRTBUF    ;START ADRES VAN DE BUFFER LADEN
INBUFA2:      LCALL  INCHAR         ;KARAKTER LEZEN
              ANL    A,#07FH        ;KARAKTERS OMVORMEN TOT ASCII 0
; ALVORENS EEN ECHO NAAR HET SCHERM TE VERSTUREN, MOET NAGEGAAN WORDEN DAT HET
; BEKOMEN KARAKTER AAN DE TE VERWACHTEN KARAKTERS VOLDOET. DE ENIGE TOEGELATEN
; CONTROLEKARAKTERS ZIJN 'CR' EN 'BACKSPACE'. VOOR AL DE ANDERE KARAKTERS
; WORDT EEN BEEP NAAR HET SCHERM GESTUURD. DE CONTROLE WORDT DOOR ASCCTR UIT-
; GEVOERD.
              LCALL  LOWUPTR              ;WEL EFFE IN CAPS TE ZETTEN
              CJNE   A,#CR,INBUFA1        ;CR INGEGEVEN?
              MOV    @R0,A                ;IN GEHEUGEN ZETTEN
              pop    psw                  ;registers herstellen
              pop    acc
              RET                         ;EINDE VAN DE ROUTINE
              
INBUFA1:      CJNE   A,#BACKSP,INBUFA6    ;NAGAAN BACK SPACE INDRUKKEN
              CJNE   R0,#STRTBUF,INBUFA3  ;STAAT DE BUFFER ONDERAAN,DAN BEEP
INBUFA4:      MOV    A,#BEEP
INBUFA7:      LCALL  OUTCHAR
              SJMP   INBUFA2              ;NIEUW KARAKTER OPHALEN
              
INBUFA3:      DEC    R0                   ;ENTRY UIT BUFFER HALEN
              MOV    DPTR,#wissen         ;WIS KARAKTER DATA NAAR SCHERM
              LCALL  OUTMSGA
              SJMP   INBUFA2              ;WACHT OP VOLGENDE INGAVE
INBUFA6:      LCALL  ASCCTR               ;ALS CARRY DAN NIET GELDIG
              JC     INBUFA4              ;BEEP NAAR SCHERM
              CJNE   R0,#ENDBUF-1,INBUFA5 ;BUFFER OP HET EINDE DAN ERROR
              SJMP   INBUFA4              ;BEEP NAAR HET SCHERM
INBUFA5:      MOV    @R0,A                ;KARAKTER IN DE BUFFER
              INC    R0                   ;BUFFER POINTER AANPASSEN
              SJMP   INBUFA7              ;KARAKTER IN BUFFER, VOLGENDE ?

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
; BDELETE HAALT ALLE blank KARAKTERS UIT de inputbuffer
;
; De routine gebruikt niets.
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

BDELETE:      PUSH     ACC
              PUSH     PSW
              MOV     A,R0                  ;BEWAAR REGISTERS
              PUSH     ACC
              MOV     A,R1
              PUSH     ACC
              MOV    R0,#STRTBUF          ;START ADRES IN R0
              MOV    R1,#STRTBUF          ;START ADRES IN R1
BDELETE1:     MOV    A,@R0                ;LEES KARAKTER
              CJNE   A,#BLANK,BDELETE2    ;KARAKTER GEEN BLANKO,DAN BDELETE2
BDELETE3:     INC    R0                   ;VOLGENDE KARAKTER OPHALEN
              CJNE   R0,#ENDBUF+1,BDELETE1 ;ALS NIET EINDE DAN VERDER
              POP     ACC                  ;REGISTERS HERSTELLEN
              MOV     R1,A
              POP     ACC
              MOV     R0,A
              POP     PSW
              POP     ACC
              RET                         ;EINDE VAN DE ROUTINE
BDELETE2:     MOV    @R1,A                ;KARAKTER NAAR BUFFER STUREN
              INC    R1
              SJMP   BDELETE3

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
; ASCCTR MOET NAGAAN DAT HET KARAKTER IN DE ACCU BIJ DE STANDAARD KARAKTERS
; HOORT. ENKEL DE KONTROLEKARAKTERS CR EN BP ZIJN TOEGELATEN. DE ACCU EN DE
; VLAGGEN WORDEN GEBRUIKT. DE KARAKTERS < 020H EN > DAN 07EH WORDEN ALS FOUT
; AANZIEN.
;
; De routine gebruikt de accu en de psw.
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

ASCCTR:       CJNE   A,#020H,ASCCTR1      ;NAGAAN KARAKTER KLEINER DAN
ASCCTR1:      JC     ASCCTR3              ;CARRY, DAN ERROR
              CJNE   A,#07FH,ASCCTR2      ;GROTER DAN 7EH
ASCCTR2:      JNC    ASCCTR3              ;GEEN CARRY DAN ERROR
              CLR    C                    ;AANGEVEN GEEN FOUT
              RET
ASCCTR3:      SETB   C                    ;AANGEVEN FOUT TOESTAND
              RET

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
; LOWUPTR ZET HET KARAKTER IN DE ACCU OM VAN LOWER NAAR UPPER CASE. DE ROUTINE
; GEBRUIKT DE VLAGGEN EN DE ACCU. DE HEX WAARDEN 61H TOT EN MET 7AH MOETEN
; OMGEZET WORDEN NAAR 41H TOT EN MET 5AH. DE ANDERE GETALLEN MOGEN NIET
; AANGEPAST WORDEN.
;
; De routine gebruikt de accu en psw .
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

LOWUPTR:      CJNE   A,#07BH,LOWUPTR1     ;BOVENSTE GRENS
LOWUPTR1:     JNC    LOWUPTR2             ;GEEN CARRY DAN GEEN OMZETTING
              CJNE   A,#061H,LOWUPTR3     ;ONDERSTE GRENS NAGAAN
LOWUPTR3:     JC     LOWUPTR2             ;CARRY DAN GEEN OMZETTING
              SUBB   A,#020H              ;OMZETTING
LOWUPTR2:     RET                         ;EINDE OMZETTING

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
; BATRANS DOET DE OMZETTING VAN EEN HEX NIBBLE IN DE ACCU NAAR DE PASSENDE
; ASCII CODE.
;
; De routine gebruikt de accu .
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

BATRANS:      PUSH   PSW
              CJNE   A,#00AH,BATRANS1     ;NAGAAN 30 OF 37 MOETEN BIJ OPTELLEN
BATRANS1:     JC     BATRANS2             ;CARRY=1 DAN SLECHTS 30
              ADD    A,#007H              ;ANDERS 30+7=37
BATRANS2:     ADD    A,#030H              ;30 BIJ OPTELLEN
              POP     PSW
              RET

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
; ASCBINTRANS ZET HET ASCII KARAKTER IN DE ACCU OM IN EEN BINAIR GETAL. DE
; ROUTINE KAN ENKEL DE KARAKTERS TUSSEN 0-9 EN A-F VERWERKEN. IN HET ANDERE
; GEVAL WORDT DE CARRY GEZET.
;
; De routine gebruikt de accu en de psw.
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

ASCBINTRANS:  CLR    C                    ;CARRY LAAG MAKEN VOOR BEREKENINGEN
              SUBB   A,#030H              ;NAGAAN GETAL BINNEN GRENZEN
              JC     ASCBINTRANS1         ;ALS C=1 DAN FOUT
              CJNE   A,#00AH,ASCBINTRANS3 ;GETAL TUSSEN 0 EN 9 ?
ASCBINTRANS3: JC     ASCBINTRANS2         ;ALS CARRY DAN ALLES OK
              SUBB   A,#007H              ;VERDER MET DE OMZETTING
              JC     ASCBINTRANS1         ;ALS CARRY DAN ERROR
              CJNE   A,#010H,ASCBINTRANS4 ;CONTROLE OP TE GROTE GETALLEN
ASCBINTRANS4: CPL    C                    ;CARRY COMPLEMENTEREN
              JC     ASCBINTRANS1         ;ALS CARRY DAN FOUT IN HET GETAL
ASCBINTRANS2: CLR    C                    ;CARRY LAAG MAKEN
ASCBINTRANS1: RET

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
; ASCII1 ZET HET ASCII KARAKTER @R0 OM IN EEN HEX GETAL IN DE ACCU.
; DE ROUTINE VERANDERD OOK DE VLAGGEN.
;
; De routine gebruikt de accu en de psw.
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

ASCII1:       MOV    A,@R0                ;KARAKTER OPHALEN
              LCALL  ASCBINTRANS          ;OMZETTING ASCII NAAR BINAIR
              RET                         ;EINDE OMZETTING

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
; ASCII2 ZET TWEE ASCII KARAKTERS @R0 EN @(R0+1) OM NAAR EEN HEX KARAKTER
; IN DE ACCU. R0 WORDT MET EEN VERHOOGD. R1 WORDT AANGEPAST. ALS OP HET EINDE
; VAN DE ROUTINE DE CARRY GESET IS, DAN IS DE OMZETTING NIET MOGELIJK.
;
; De routine gebruikt de accu, r0.
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

ASCII2:       PUSH   B
              LCALL  ASCII1               ;OMZETTEN
              JC     ASCII21              ;CARRY 1 DAN OMZETTING NIET MOGELIJK
              SWAP   A                    ;NIBBLE VAN PLAATS VERANDEREN
              MOV    B,A                 ;TIJDELIJK WEG STOPPEN
              INC    R0                   ;NAAR VOLGENDE KARAKTER WIJZEN
              LCALL  ASCII1               ;KARAKTER OMZETTEN
              JC     ASCII21               ;WEG ALS C=1
              ORL    A,B                  ;GETALLEN BIJ ELKAAR VOEGEN
ASCII21:      POP     B
              RET

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
; ASCII4 ZET DE 4 ASCII KARAKTERS @R0 TOT @[R0+3] OM IN EEN HEX GETAL IN DE
; DATAPOINTER. ALS DE OMZETTING NIET MOGELIJK IS WORDT DE CARRY GEZET.
; R0 WORDT MET DRIE VERHOOGD, EN DE ACCU WORDT AANGEPAST,ALSOOK R1. ALS OP HET
; EINDE VAN DE ROUTINE DE CARRY GESET IS, DAN WAS EEN OMZETTING NIET MOGELIJK.
;
; De routine gebruikt de accu, r0, r1 en de psw.
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

ASCII4:       LCALL  ASCII2               ;OMZETTEN EERSTE TWEE ASCII BYTES
              JC     ASCII41              ;IN GEVAL VAN ERROR DAN WEG
              MOV    DPH,A                ;HEX GETAL IN DE DPTRH
              INC    R0                   ;WIIJST NU NAAR VOLGENDE GROEP VAN 2
              LCALL  ASCII2               ;OMZETTEN VOLGENDE TWEE BYTES
              JC     ASCII41              ;BIJ FOUT DAN WEG
              MOV    DPL,A                ;IN DPTR LADEN
ASCII41:      RET

endif        ;einde van aduc_sio
;*******************************************************************************              


ifdef        aduc_math    ;rekenkundige routines              
;*******************************************************************************
; aduc_math
; Rekenkundige routines voor 8051 compatibele (ook aduc832) microcontrollers
; De routines zijn gebaseerd op het verwerken van de getallen in twee 32 bit 
; software accumulatoren ACC0 en ACC1. De accumulatoren hebben elk hun tekenbit
; waarmee het teken wordt bepaald bij de rekenkundige bewerkingen.  
; Deze accumulatoren bestaan uit R0 tot R7
; zoals aangegeven:
;
;    msb 31  24 23  16 15   8 7    0 lsb 
;        ---------------------------
; ACC0  |  R3  |  R2  |  R1  |  R0  |    tekenbit van ACC0 = f0  errorbit  = CY
;        ---------------------------                             statusbit = f0
;        ---------------------------
; ACC1  |  R7  |  R6  |  R5  |  R4  |    tekenbit van ACC1 = f1
;        ---------------------------
; Bij 8bit bewerkingen zullen steeds de minst beduidende bytes (R0 en R4) worden
; gebruikt, bij 16bit bewerkingen de minst beduidende woorden (R1,R0 en R5,R4).
; De userbits F0 en F1 worden gebruikt als tekenbits, of als statusbits bij
; return en moeten hiervoor worden gereserveerd. 
; De CY bit wordt gebruikt als errorbit bij return.
; Er zijn routines voorzien voor bewerkingen op signed (2' s complement) 
; en unsigned getallen.
;
; Beschikbare routines:
;
; Conversie routines:
; bcdhex8, hexbcd8, bcdhex16, hexbcd16, hexbcd16_u, magsig8, magsig8acc1, 
; magsig16, magsig16acc1, magsig32, magsig32acc1, sigmag8acc0, sigmag8acc1, 
; sigmag16acc0, sigmag16acc1, sigmag16acc1h, sigmag32acc0, sigmag32acc1 
;
; Som:
; s_add16, add16, s_add32, add32
;
; Verschil:
; s_sub16, sub16, s_sub32, sub32
;
; Vermenigvuldigen:
; mul16, mul16acc1, s_mul16, s_mul16acc1, s_mul816, mul816, s_mac16 
;
; Delen:
; s_div8, div8, s_div16, div16, s_div32, div32, s_muldiv, muldiv
;
; Vemenigvuldig met factor:
; mul16_pi, mul16_i_pi,mul16_sqrt2, mul16_i_sqrt2, mul16_10, mul16_100 
;
; Vierkantswortel:
; sqrt32, s_sqrt32
;
; Schuiven:
; shiftright32, shiftleft32, s_shiftright32, s_shiftleft32 
;
; Vergelijken:
; s_cmp16, cmp16, s_cmp32, cmp32
;
; Tabel lookup:
; table_lu
;
; Gonemetrische functies:
; Cordic = sinus,cosinus
;
;*******************************************************************************

;*******************************************************************************
;
; bcdhex8     (45,77us @16.777216MHz)
; Deze routine zal een positief of negatief 8bit bcd getal in acc0 (R0) omzetten 
; naar een 2's complement hex getal.
; Bij een positief getal (F0=0) zal een bcdgetal 00h tot 99h worden omgezet naar
; een waarde 00h tot 63h.
; Bij een negatief getal (F0=1) zal een bcdgetal 00h tot 99h worden omgezet naar 
; een waarde 00h,ffh tot 9dh  (9dh is de 2's complement voorstelling van -63h). 
; input:      R0 = waarde tussen 00h en 99h.
;             F0 = tekenbit, 1=negatief, 0=positief.
; output:     R0 = 2's complement waarde  
;             cy=1 bij out of range van r0
;             cy=0 bij conversie ok
; vernietigt: niets
;*******************************************************************************
bcdhex8:      push   acc                  ;registers op de stack zetten
              push   psw
              push   b
                    
              mov    a,r0                 ;neem te converteren waarde
              mov    b,a
              anl    a,#0fh
              cjne   a,#0ah,bcdhex81      ;test low nibble
bcdhex81:     jnc    bcdhex82             ;nibble te groot = error
              mov    a,b
              anl    a,#0f0h
              cjne   a,#0a0h,bcdhex811    ;test high nibble
bcdhex811:    jnc    bcdhex82             ;nibble te groot = error
              
              mov    a,r0
              anl    a,#0f0h              ;maskeer hoogste bits
              swap   a
              mov    b,#10                ;bereken de waarde van de  4 msb's
              mul    ab
              mov    b,a                  ;bewaar dit product even
              mov    a,r0
              anl    a,#00fh
              mov    r0,a
              mov    a,b
              add    a,r0                 ;eenheden er bijtellen
              mov    r0,a
              
              clr    f1                   ;tweede tekenbit = positief 
              lcall  magsig8              ;omzetten naar 2's complement
              
              pop    b
              pop    psw
              clr    cy                   ;geen fout
              pop    acc
              ret
              
bcdhex82:     pop    b
              pop    psw
              setb   cy                   ;fout! r0 out of range
              pop    acc
              ret

;*******************************************************************************
;
; hexbcd8     (38,4us @16.777216MHz)
; Deze routine zal een 8bit 2's complement getal in acc0 (R0) omzetten 
; naar een 8bit bcd getal.
; Bij een positief getal zal een waarde 00h tot 63h worden omgezet naar een 
; bcdgetal van 00h tot +99h.
; Bij een negatief getal zal een waarde 00h,ffh tot 9dh  
; (9dh is de 2's complement voorstelling van -63h) worden omgezet naar een 
; bcdgetal 00h tot -99h 
; 
; input:      R0 = 2's complement waarde tussen 9dh..ffh,0,01h..63h.
;             
; output:     R0 = bcd getal tussen -99h en 99h
;              F0 = tekenbit, 1=negatief, 0=positief. 
;             cy=1 bij out of range van r0
;             cy=0 bij conversie ok
; vernietigt: niets
;*******************************************************************************
hexbcd8:        push    acc             ;registers bewaren
                push    psw
                push    b
                mov     a,r0            ;neem 2's complement getal en test het
                cjne    a,#64h,hexbcd81 ;bereik van het getal.
hexbcd81:       jc      hexbcd83        ;als  9dh> getal >63h dan out of range
                cjne    a,#9dh,hexbcd82
hexbcd82:       jnc     hexbcd83        ;geen carry, dan ok
                pop     b
                pop     psw             ;jammer maar helaas fout...
                setb    c
                pop     acc
                ret
hexbcd83:       lcall    sigmag8acc0    ;2's compl. naar abs. waarde + teken f0
                mov     a,r0            ;zet getal om naar 00h..99h en klaar...
                mov     b,#10
                div     ab
                swap    a
                add     a,b
                mov     r0,a
                pop     b               ;registers herstellen
                pop     acc             ;psw van stack halen                
                mov     c,f0            ;we moeten de f0 flag als teken houden
                mov     acc.5,c
                mov     psw,a           ;f0 flag in psw is correct
                clr     c               ;status: conversie ok
                pop     acc
                ret


              
;*******************************************************************************
;
; bcdhex16    (86,6us @16.777216MHz)
; Deze routine zal een positief of negatief 16 bit bcd getal in acc0 (R1,R0)
; omzetten naar een 2's complement hex getal.
; Bij een positief getal (F0=0) zal een bcdgetal 0000h tot 9999h worden omgezet  
; naareen waarde 0000h tot 270fh.
; Bij een negatief getal (F0=1) zal een bcdgetal 0000h tot -9999h worden omgezet
; naar een waarde 0000h,ffffh tot d8f1h (d8f1h is de 2's complement voorstelling 
; van -270fh). 
; input:      R1,R0 = waarde tussen 0000h en 9999h.  (R1=msb,R0=lsb)
;             F0 = tekenbit, 1=negatief, 0=positief.
; output:     R1,R0 = 2's complement waarde   
;             cy=1 bij out of range van r1,r0
;             cy=0 bij conversie ok
; vernietigt: niets
;*******************************************************************************
bcdhex16:     push   acc                  ;registers op de stack zetten
              push   psw
              push   b
              
              mov    a,r0                  ;low byte van te converteren waarde
              mov    b,a
              anl    a,#0fh
              cjne   a,#0ah,bcdhex161      ;test low nibble
bcdhex161:    jnc    bcdhex162             ;nibble te groot = error
              mov    a,b
              anl    a,#0f0h
              cjne   a,#0a0h,bcdhex1611    ;test high nibble
bcdhex1611:   jnc    bcdhex162             ;nibble te groot = error
              mov    a,r1                  ;high byte van te converteren waarde
              mov    b,a
              anl    a,#0fh
              cjne   a,#0ah,bcdhex1612      ;test low nibble
bcdhex1612:   jnc    bcdhex162             ;nibble te groot = error
              mov    a,b
              anl    a,#0f0h
              cjne   a,#0a0h,bcdhex1613    ;test high nibble
bcdhex1613:   jnc    bcdhex162             ;nibble te groot = error

              mov    a,r0                  ;neem low byte voor conversie
              anl    a,#00fh               ;eenheden afzonderen
              push   acc                   ;en even bewaren

              mov    a,r0
              swap   a
              anl    a,#00fh              ;tientallen
              mov    b,#00ah              ;maal tien
              mul    ab                   ;
              pop    b                    ;neem eenheden
              add    a,b                  ;tel bij tientallen
              push   acc                  ;weer bewaren

              mov    a,r1                 ;neem high byte voor conversie 
              anl    a,#0f0h
              swap   a
              push   acc                  ;bewaar de duizendtallen

              mov    a,r1                  ;neem high byte voor conversie
              anl    a,#00fh              ;zonder de honderdtallen af
              mov    b,#100d              ;en maal honderd
              mul    ab
              mov    r1,b
              mov    r0,a

              pop    acc                  ;neem de duizendtallen en 
              mov    b,#10d               ;vermenigvuldig met *10*100=*1000
              mul    ab                   ;resultaat zit nog helemaal in accu 
              mov    b,#100d              ;volgende stap *100
              mul    ab
              add    a,r0                 ;tel honderdtallen bij de
              mov    r0,a                 ;duizendtallen
              mov    a,r1
              addc   a,b
              mov    r1,a

              pop    acc                  ;neem de tientallen en eenheden
              add    a,r0                 ;en tel bij de honderdtallen en
              mov    r0,a                 ;duizendtallen
              mov    a,#000h
              addc   a,r1
              mov    r1,a                 ;omzetting klaar

              clr    f1                   ;tweede tekenbit = positief 
              lcall  magsig16             ;omzetten naar 2's complement
              
              pop    b
              pop    psw
              clr    cy                   ;geen fout
              pop    acc
              ret
              
bcdhex162:    pop    b
              pop    psw
              setb   cy                  ;fout! r1,r0 out of range
              pop    acc
              ret

;*******************************************************************************
;
; hexbcd16    (1,415ms @16.777216MHz)
; Deze routine zal een 16 bit 2's complement getal in acc0 (r1,r0) omzetten 
; naar een 24bit bcd getal.
; Bij een positief getal zal een waarde 0000h tot 7fffh worden omgezet naar een 
; bcdgetal van 000000h tot +032767h.
; Bij een negatief getal zal een waarde 0000h,ffffh tot 8000h worden omgezet 
; naar een bcdgetal 000000h tot -032768h 
; 
; input:      r1,r0 = 16 bit 2's complement getal
;             
; output:     r2,r1,r0 = acc0 = bcd getal tussen -032768 en +032767
;              F0 = tekenbit, 1=negatief, 0=positief. 
;             
; vernietigt: niets
;*******************************************************************************
;
; hexbcd16_u    (1,405ms @16.777216MHz)
; Deze routine zal een 16 bit unsigned getal in acc0 (r1,r0) omzetten 
; naar een 24bit bcd getal.
; Dit positief getal met een waarde 0000h tot ffffh zal worden omgezet naar een 
; bcdgetal van 000000h tot 065535h.
; 
; input:      r1,r0 = 16 bit getal (absolute waarde)
;             
; output:     r2,r1,r0 = acc0 = 24 bit bcd getal tussen 0 en 065535h
;                           
; vernietigt: niets
;*******************************************************************************
hexbcd16:       lcall   sigmag16acc0    ;2's compl. naar abs. waarde +teken f0
hexbcd16_u:     push   acc             ;registers bewaren 
                push   psw
                push   b
                push   dpl
                
                mov    b,#005h         ;loopcounter voor 5 digits
hexbcd161:      mov    r4,#0ah         ;deler '10d' in acc1 laden
                mov    r5,#00h         ;omzetten naar 10-delig talstelsel 
                lcall  div16           ;deling uitvoeren, Q=r1,r0  rest=r5,r4
                mov    a,r4            ;rest is steeds kleiner dan 10 (msb=0)
                push   acc             ;dit als resultaatbyte op stack zetten
                                        ;dus eerst eenheden, dan tientallen
                                        ;honderdtallen, duizendtallen en
                                        ;tienduizendtallen
                djnz   b,hexbcd161      ;quotient nogmaals delen
                
;nu alle resultaatbytes weer van stack halen en combineren

                pop    acc              ;tienduizendtallen van stack halen
                mov    r2,a             ;ms digit in outputregister stoppen
                pop    acc              ;duizendtallen ophalen
                swap   a                ;en combineren met de
                mov    dpl,a
                pop    acc              ;honderdtallen van de stack
                add    a,dpl
                mov    r1,a             ;volgende 2 digits in outputregister
                pop    acc              ;tientallen van de stack halen
                swap   a
                mov    dpl,a
                pop    acc              ;en combineren met de eenheden
                add    a,dpl
                mov    r0,a             ;laatste 2 digits in outputregisters

                pop    dpl              ;registers herstellen
                pop    b
                pop    psw              ;we krijgen onze tekenbit terug            
                pop    acc
                ret

;*******************************************************************************
; magsig8        (13,6us @16.777216MHz)
; 8-Bit absolute waarde/ tekenBits omzetting naar 2's Complement waarde
; Outputbereik getallen: -128 tot  +127 (80h..ffh,0,01h..7fh)
; input:    r0 = acc0 absolute waarde (0..80h)
;           Bits f0 & f1 = tekenbits van acc0 en acc1
;           (f0 en f1 zijn 1 als negatief)
;
; output:   r0 = 2's complement
;
; vernietigt: niets
;*******************************************************************************

magsig8:       push   acc                ;bewaar gebruikte registers
               push   psw
               jb     f0, magsig8b       ;test eerste tekenbit
               jb     f1, magsig8a       ;test tweede tekenbit
               sjmp   magsig8c

magsig8b:      jnb    f1, magsig8a
               sjmp   magsig8c

magsig8a:      mov    a,r0               ;als r0 negatief, neem abs waarde
               cpl    a                  ;complementeer abs waarde 
               inc    a                  ;r0 = complement(r0)+1
               mov    r0,a               ;bewaar in 2's complement
magsig8c:      pop    psw
               pop    acc
               ret                        ;klaar
               
;*******************************************************************************
; magsig8acc1        (13,6us @16.777216MHz)
; 8-Bit absolute waarde/ tekenBits omzetting naar 2's Complement waarde
; Outputbereik getallen: -128 tot  +127 (80h..ffh,0,01h..7fh)
; input:    r4 = acc1 absolute waarde (0..80h)
;           Bits f0 & f1 = tekenbits van acc0 en acc1
;           (f0 en f1 zijn 1 als negatief)
;
; output:   r4 = 2's complement
;
; vernietigt: niets
;*******************************************************************************

magsig8acc1:   push    acc                ;bewaar gebruikte registers
               push    psw
               jb      f0, magsig8acc1b   ;test eerste tekenbit
               jb      f1, magsig8acc1a   ;test tweede tekenbit
               sjmp    magsig8c

magsig8acc1b:  jnb     f1, magsig8acc1a
               sjmp    magsig8acc1c

magsig8acc1a:  mov     a,r4              ;als r4 negatief, neem abs waarde
               cpl     a                 ;complementeer abs waarde 
               inc     a                 ;r4 = complement(r0)+1
               mov     r4,a              ;bewaar in 2's complement
magsig8acc1c:  pop     psw
               pop     acc
               ret                       ;klaar

;*******************************************************************************
; magsig16        (16,6us @16.777216MHz)
; 16-Bit absolute waarde/ tekenBits omzetting naar 2's Complement waarde
; Outputbereik getallen: -32768 tot  +32767 (8000h..ffff,0,1..7fffh)
; input:    r1,r0 = acc0 absolute waarde (0..8000h)(r1=msb)
;           Bits f0 & f1 = tekenbits van acc0 en acc1
;           (f0 en f1 zijn 1 als negatief)
;
; output:   r1,r0 = 2's complement
;
; vernietigt: niets
;*******************************************************************************
magsig16:      push    acc              ;bewaar gebruikte registers
               push    psw
               jb      f0,magsig16b     ;test eerste tekenbit
               jb      f1,magsig16a     ;test tweede tekenbit
               sjmp    magsig16c

magsig16b:     jnb     f1,magsig16a
               sjmp    magsig16c

magsig16a:     mov     a,r0             ;als het een negatief getal is
               cpl     a                ;complementeer het dan
               add     a, #1            ;en tel er 1 bij
               mov     r0,a 
               mov     a,r1             ;neem volgende byte
               cpl     a                ;complementeer en tel carry erbij
               addc    a,#0
               mov     r1,a
magsig16c:     pop     psw
               pop     acc
               ret
               
;*******************************************************************************
; magsig16acc1        (16,6us @16.777216MHz)
; 16-Bit absolute waarde/ tekenBits omzetting naar 2's Complement waarde
; Outputbereik getallen: -32768 tot  +32767 (8000h..ffff,0,1..7fffh)
; input:    r5,r4 = acc1 absolute waarde (0..8000h) (r1=msb)
;           Bits f0 & f1 = tekenbits van acc0 en acc1
;           (f0 en f1 zijn 1 als negatief)
;
; output:   r5,r4 = 2's complement
;
; vernietigt: niets
;*******************************************************************************
magsig16acc1:   push    acc                 ;bewaar gebruikte registers
                push    psw
                jb      f0,magsig16acc1b    ;test eerste tekenbit
                jb      f1,magsig16acc1a    ;test tweede tekenbit
                sjmp    magsig16acc1c

magsig16acc1b:  jnb     f1,magsig16acc1a
                sjmp    magsig16acc1c

magsig16acc1a:  mov     a,r4                ;als het een negatief getal is
                cpl     a                   ;complementeer het dan
                add     a,#1                ;en tel er 1 bij
                mov     r4,a 
                mov     a,r5                ;neem volgende byte
                cpl     a                   ;complementeer en tel carry erbij
                addc    a,#0
                mov     r5,a
magsig16acc1c:  pop     psw
                pop     acc
                ret

;*******************************************************************************
; magsig32        (22,2us @16.777216MHz)
; 32-Bit absolute waarde/ tekenBits omzetting naar 2's Complement waarde
; Outputbereik getallen: -2147483648 tot +2147483647 
;                  (80000000h..ffffffffh,0,00000001h..7fffffffh)
; input:    r3,r2,r1,r0 = acc0 absolute waarde (0..80000000h)(r3=msb)
;           Bits f0 & f1 = tekenbits van acc0 en acc1
;           (f0 en f1 zijn 1 als negatief)
;
; output:   r3,r2,r1,r0 = 2's complement
;
; vernietigt: niets
;*******************************************************************************
magsig32:      push    acc              ;bewaar gebruikte registers
               push    psw
               jb      f0,magsig32b     ;test eerste tekenbit
               jb      f1,magsig32a     ;test tweede tekenbit
               sjmp    magsig32c

magsig32b:     jnb     f1,magsig32a
               sjmp    magsig32c

magsig32a:     mov     a,r0             ;als het een negatief getal is
               cpl     a                 ;complementeer het dan
               add     a,#1             ;en tel er 1 bij
               mov     r0,a 
               mov     a,r1             ;neem volgende byte
               cpl     a                 ;complementeer en tel carry erbij
               addc    a,#0
               mov     r1,a
               mov     a,r2            ;neem volgende byte
               cpl     a                ;complementeer en tel carry erbij
               addc    a,#0
               mov     r2,a
               mov     a,r3            ;neem volgende byte
               cpl     a                ;complementeer en tel carry erbij
               addc    a,#0
               mov     r3,a
               
magsig32c:     pop     psw
               pop     acc
               ret



;*******************************************************************************
; magsig32acc1        (22,2us @16.777216MHz)
; 32-Bit absolute waarde/ tekenBits omzetting naar 2's Complement waarde
; Outputbereik getallen: -2147483648 tot +2147483647 
;                  (80000000h..ffffffffh,0,00000001h..7fffffffh)
; input:    r7,r6,r5,r4 = acc1 absolute waarde (0..80000000h) (r7=msb)
;           Bits f0 & f1 = tekenbits van acc1 high en acc1 low
;           (f0 en f1 zijn 1 als negatief)
;
; output:   r7,r6,r5,r4 = 2's complement
;
; vernietigt: niets
;*******************************************************************************
magsig32acc1:  push    acc               ;bewaar gebruikte registers
               push    psw
               jb      f0,magsig32acc1b  ;test eerste tekenbit
               jb      f1,magsig32acc1a  ;test tweede tekenbit
               sjmp    magsig32acc1c

magsig32acc1b: jnb     f1,magsig32acc1a
               sjmp    magsig32acc1c

magsig32acc1a: mov     a,r4             ;als het een negatief getal is
               cpl     a                 ;complementeer het dan
               add     a,#1             ;en tel er 1 bij
               mov     r4,a 
               mov     a,r5             ;neem volgende byte
               cpl     a                 ;complementeer en tel carry erbij
               addc    a,#0
               mov     r5,a
               mov     a,r6             ;neem volgende byte
               cpl     a                 ;complementeer en tel carry erbij
               addc    a,#0
               mov     r6,a
               mov     a,r7             ;neem volgende byte
               cpl     a                ;complementeer en tel carry erbij
               addc    a,#0
               mov     r7,a
               
magsig32acc1c: pop     psw
               pop     acc
               ret


;*******************************************************************************
; sigmag8acc0        (13,0us @16.777216MHz)
; 8-Bit signed (2's Complement) omzetting naar magnitude / Sign Bit Conversion
; Outputbereik getallen: 00h..80h, f0 als tekenbit 
; input:     r0 = 2's complement byte in acc0
;                 80h..ffh,0,01h..7fh     (-128d tot  +127d)
; output:    r0 = absolute waarde
;            Bit f0 = tekenbit (f0=1 als acc0 negatief is)
;
; vernietigt: niets
;*******************************************************************************

sigmag8acc0:    push    acc                ;bewaar registers
                push    psw                
                mov     a,r0               ;neem lsb van acc0
                jb      acc.7,sigmag8acc0a ;kijk of het negatief is
                pop     psw
                clr     f0                 ;nee, positief...klaar
                pop     acc
                ret         
sigmag8acc0a:   cpl     a                   ;negatief, bepaal absolute waarde
                inc     a                   ;een erbij
                mov     r0,a               ;bewaar
                pop     psw
                setb    f0                 ;f0=1 als negatief
                pop     acc
                ret
                
;*******************************************************************************
; sigmag8acc1        (13,0us @16.777216MHz)
; 8-Bit signed (2's Complement) omzetting naar magnitude / Sign Bit Conversion
; Outputbereik getallen: 00h..80h, f1 als tekenbit 
; input:     r4 = 2's complement byte in acc1
;            80h..ffh,0,01h..7fh     (-128d tot  +127d)
; output:    r4 = absolute waarde
;            Bit f1 = tekenbit (f1=1 als acc0 negatief is)
;
; vernietigt: niets
;*******************************************************************************

sigmag8acc1:    push    acc                ;bewaar registers
                push    psw
                mov     a,r4               ;neem lsb van acc0
                jb      acc.7,sigmag8acc1a ;kijk of het negatief is
                pop     psw
                clr     f1                 ;nee, positief...klaar
                pop     acc
                ret         
sigmag8acc1a:   cpl     a                   ;negatief, bepaal absolute waarde
                inc     a                   ;een erbij
                mov     r4,a               ;bewaar
                pop     psw
                setb    f1                 ;f0=1 als negatief
                pop     acc
                ret

;*******************************************************************************
; sigmag16acc0        (16,6us @16.777216MHz)
; 16-Bit signed (2's Complement) omzetting naar magnitude / Sign Bit Conversion
; Outputbereik getallen: 0000h..8000h, f0 als tekenbit  
; input:     r1,r0 = 2's complement word in acc0
;            8000h..ffff,0,1..7fffh (-32768 tot  +32767)
; output:    r1,r0 = absolute waarde
;            Bit f0 = tekenbit (f0=1 als acc0 negatief is)
;
; vernietigt: niets
;*******************************************************************************

sigmag16acc0:   push    acc                ;bewaar registers
                push    psw
                mov     a,r1               ; neem msb van acc0
                jb      acc.7,sigmag16acc0a ;kijk of het negatief is
                pop     psw
                clr     f0                 ;f0=0 als positief
                pop     acc
                ret                        ;klaar

sigmag16acc0a:  mov     a,r0               ;getal is negatief
                cpl     a                  ;complementeren
                add     a,#1               ;en een bijtellen
                mov     r0,a 
                mov     a,r1           
                cpl     a                  ;complement eer volgende byte
                addc    a,#0               ;carry bij volgende byte tellen
                mov     r1,a
                pop     psw
                setb    f0                 ;f0=1 als negatief
                pop     acc
                ret
               

;*******************************************************************************
; sigmag16acc1        (16,6us @16.777216MHz)
; 16-Bit signed (2's Complement) omzetting naar magnitude / Sign Bit Conversion
; Outputbereik getallen: 0000h..8000h, f1 als tekenbit 
; input:     r5,r4 = 2's complement word in acc1
;            8000h..ffff,0,1..7fffh (-32768 tot  +32767)
; output:    r5,r4 = absolute waarde
;            Bit f1 = tekenbit (f1=1 als acc1 negatief is)
;
; vernietigt: niets
;*******************************************************************************

sigmag16acc1:   push    acc                ;bewaar registers
                push    psw
                mov     a,r5               ;neem msb van acc1
                jb      acc.7,sigmag16acc1a ;kijk of het negatief is
                pop     psw
                clr     f1                 ;f1=0 als positief
                pop    acc
                ret                        ;klaar

sigmag16acc1a:  mov     a,r4               ;getal is negatief
                cpl     a                   ;complementeren
                add     a,#1               ;en een bijtellen
                mov     r4,a 
                mov     a,r5           
                cpl     a                  ;complementeer volgende byte
                addc    a,#0               ;carry bij volgende byte tellen
                mov     r5,a
                pop     psw
                setb    f1                ;f1=1 als negatief
                pop     acc
                ret
               

;*******************************************************************************
; sigmag16acc1h        (16,6us @16.777216MHz)
; 16-Bit signed (2's Complement) omzetting naar magnitude / Sign Bit Conversion
; Outputbereik getallen: 0000h..8000h, f0 als tekenbit
; input:     r7,r6 = 2's complement word in acc1 high word
;            8000h..ffff,0,1..7fffh (-32768 tot  +32767)
; output:    r7,r6 = absolute waarde
;            Bit f0 = tekenbit (f0=1 als acc1 high word negatief is)
;
; vernietigt: niets
;*******************************************************************************

sigmag16acc1h:  push    acc                 ;bewaar registers
                push    psw
                mov     a,r7                ; neem msb van acc1 high
                jb      acc.7,sigmag16acc1ha ;kijk of het negatief is
                pop     psw
                clr     f0                  ;f0=0 als positief
                pop     acc
                ret                         ;klaar

sigmag16acc1ha: mov     a,r6               ;getal is negatief
                cpl     a                   ;complementeren
                add     a,#1               ;en een bijtellen
                mov     r6,a 
                mov     a,r7           
                cpl     a                  ;complementeer volgende byte
                addc    a,#0               ;carry bij volgende byte tellen
                mov     r7,a
                pop     psw
                setb    f0                ;f0=1 als negatief
                pop     acc
                ret


;*******************************************************************************
; sigmag32acc0        (22,2us @16.777216MHz)
; 32-Bit signed (2's Complement) omzetting naar magnitude / Sign Bit Conversion
; Outputbereik getallen: 00000000h..80000000h, f0 als tekenbit
;                   
; input:     r3,r2,r1,r0 = 2's complement longword in acc0
;            80000000h..ffffffffh,0,00000001h..7fffffffh
;            (-2147483648 tot +2147483647)
; output:    r3,r2,r1,r0 = absolute waarde
;            Bit f0 = tekenbit (f0=1 als acc0 negatief is)
;
; vernietigt: niets
;*******************************************************************************

sigmag32acc0:   push    acc                ;bewaar registers
                push    psw
                mov     a,r3               ;neem msb van acc0
                jb      acc.7,sigmag32acc0a  ;kijk of het getal negatief is
                pop     psw                ;nee, positief
                clr     f0                  
                pop     acc
                ret                        ;klaar

sigmag32acc0a:  mov     a,r0               ;getal is negatief
                cpl     a                  ;complementeren
                add     a,#1               ;en een bijtellen
                mov     r0,a
                mov     a,r1               ;neem volgende byte
                cpl     a                  ;complementeer volgende byte
                addc    a,#0
                mov     r1,a
                mov     a,r2               ;neem volgende byte
                cpl     a                  ;complementeer volgende byte
                addc    a,#0
                mov     r2,a
                mov     a,r3               ;neem volgende byte
                cpl     a                  ;complementeer volgende byte
                addc    a,#0
                mov     r3,a
                pop     psw
                setb    f0                 ;negatief: f0=1
                pop     acc
                ret                        ;klaar

;*******************************************************************************
; sigmag32acc1        (22,2us @16.777216MHz)
; 32-Bit signed (2's Complement) omzetting naar magnitude / Sign Bit Conversion
; Outputbereik getallen: 00000000h..80000000h, f1 als tekenbit
;                   
; input:     r7,r6,r5,r4 = 2's complement longword in acc1
;            80000000h..ffffffffh,0,00000001h..7fffffffh
;            (-2147483648 tot +2147483647)
; output:    r7,r6,r5,r4 = absolute waarde
;            Bit f1 = tekenbit (f1=1 als acc1 negatief is)
;
; vernietigt: niets
;*******************************************************************************

sigmag32acc1:   push    acc                ;bewaar registers
                push    psw
                mov     a,r7               ;neem msb van acc0
                jb      acc.7,sigmag32acc1a  ;kijk of het getal negatief is
                pop     psw                ;nee, positief
                clr     f1                  
                pop     acc
                ret                        ;klaar

sigmag32acc1a:  mov     a,r4               ;getal is negatief
                cpl     a                   ;complementeren
                add     a,#1               ;en een bijtellen
                mov     r4,a
                mov     a,r5               ;neem volgende byte
                cpl     a                  ;complementeer volgende byte
                addc    a,#0
                mov     r5,a
                mov     a,r6               ;neem volgende byte
                cpl     a                  ;complementeer volgende byte
                addc    a,#0
                mov     r6,a
                mov     a,r7               ;neem volgende byte
                cpl     a                  ;complementeer volgende byte
                addc    a,#0
                mov     r7,a
                pop     psw
                setb    f1                 ;negatief: f0=1
                pop     acc
                ret                        ;klaar


;*******************************************************************************
; s_add16        (19,35us @16.777216MHz)
; 16-Bit Signed (2's Complement) optelling (acc0=acc0+acc1)
; Inputbereik getallen: -32768 tot  +32767 (8000h..ffff,0,1..7fffh)
; input:     r1,r0 = acc0
;            r5,r4 = acc1
;
; output:    r3,r2,r1,r0 = acc0 = acc0+acc1  met teken
;            CY=0 :het resultaat zit binnen het 16bit bereik
;                  gebruik van r3,r2 is niet nodig
;            CY=1 :als het resultaat groter werd dan 16bit (buiten bereik) dan,
;                  meer beduidende bytes (r3,r2) van het resultaat ook gebruiken
;                   
; vernietigt:  niets
;*******************************************************************************
s_add16:       push    acc            ;bewaar gebruikte registers
               push    psw
               
               mov     a,r0           ;tel de soft accumulators op
               add     a,r4           ;low byte eerst
               mov     r0,a           ;resultaat bewaren
               mov     a,r1           ;nu de high bytes 
               addc    a,r5           ;optellen met eventuele overdracht
               mov     r1,a           ;16 bit resultaat is klaar
               
               mov     f0,c
               mov     c,ov          ;bewaar overflow flag voor sign extention
               mov     f1,c
               mov     c,f0           ;herstel carrybit
               
               mov     a,#00h
               addc    a,#00h
               mov     r2,a            ;24bit resultaat klaar
               
               jnb     f1,s_add163     ;bij overflow, sign extention toepassen
               
               mov     a,r2            ;ja, sign extention toepassen!
               jnb     acc.0,s_add161  ;als lsb van r2=0, dan r3,r2=0000h
               mov     r2,#0ffh        ;als lsb van r2=1, dan r3,r2=ffffh
               mov     r3,#0ffh
               sjmp    s_add162
s_add161:      mov     r2,#00h
               mov     r3,#00h               
s_add162:      pop     psw
               setb    c               ;aangeven aan de gebruiker dat het
               pop     acc             ;resultaat groter is dan 16bits! 
               ret
               
s_add163:      pop     psw             ;binnen 16bit bereik!geen overflow
               clr     c
               pop     acc
               ret
               
;*******************************************************************************
; add16        (13,6us @16.777216MHz)
; 16-Bit Unsigned optelling (acc0 = acc0+acc1)
; Inputbereik getallen: 0 tot  +65535 (0000h...ffffh)
; input:     r1,r0 = acc0
;            r5,r4 = acc1
;
; output:    (r2),r1,r0 = acc0 = acc0+acc1  
;            indien het resultaat groter wordt dan 16 bit, r2 als msb gebruiken
;            CY=0 :het resultaat zit binnen het 16bit bereik
;                  gebruik van meer beduidende byte (r2=0) is niet nodig
;            CY=1 :het resultaat is groter dan 16bit (buiten bereik) dus ook de
;                  meer beduidende byte (r2) van het resultaat gebruiken
;                          
; vernietigt:  niets
;*******************************************************************************

add16:         push    acc            ;bewaar gebruikte registers
               push    psw
               mov     a,r0           ;tel de soft accumulators op
               add     a,r4           ;low byte eerst
               mov     r0,a           ;resultaat bewaren
               mov     a,r1           ;nu de high bytes 
               addc    a,r5           ;optellen met eventuele overdracht
               mov     r1,a           ;16 bit resultaat is klaar
               jnc     add161          ;geen 24 bit resultaat
               
               mov     r2,#01h         ;24bit resultaat klaar
               pop     psw
               setb    c               ;aangeven aan de gebruiker dat het 
               pop     acc             ;resultaat 24 bits groot is
               ret
               
add161:        mov     r2,#00h
               pop     psw
               clr     c               ;aangeven aan de gebruiker dat het 
               pop     acc             ;resultaat 16 bits groot is
               ret

;*******************************************************************************
; s_add32        (28,0us @16.777216MHz)
; 32-Bit Signed (2's Complement) optelling (acc0=acc0+acc1)
; Inputbereik getallen: -2147483648 tot +2147483647 
;                      (80000000h..ffffffffh,0,00000001h..7fffffffh)
; input:     r3,r2,r1,r0 = acc0
;            r7,r6,r5,r4 = acc1
;
; output:    (r4),r3,r2,r1,r0 = acc0 = acc0+acc1  met teken
;            CY=0 :het resultaat zit binnen het 32bit bereik
;                  gebruik van een meer beduidende byte (r4=0) is niet nodig
;            CY=1 :als het resultaat groter werd dan 32bit (buiten bereik) dan,  
;                  de meer beduidende byte (r4) van het resultaat ook gebruiken
;                   
; vernietigt:  niets
;*******************************************************************************
s_add32:       push    acc             ;bewaar gebruikte registers
               push    psw
               
               mov     a,r0           ;tel de soft accumulators op
               add     a,r4           ;low byte eerst
               mov     r0,a           ;resultaat bewaren
               mov     a,r1           ;nu de hogere bytes 
               addc    a,r5           ;optellen met eventuele overdracht
               mov     r1,a           ;16 bit resultaat is klaar
               
               mov     a,r2           ;derde bytes optellen
               addc    a,r6           ;optellen met eventuele overdracht
               mov     r2,a           ;24bit resultaat is klaar
               mov     a,r3           ;nu de ms bytes 
               addc    a,r7           ;optellen met eventuele overdracht
               mov     r3,a           ;32 bit resultaat is klaar
               
               mov     f0,c
               mov     c,ov           ;bewaar overflow flag voor sign extention
               mov     f1,c
               mov     c,f0           ;herstel carrybit
               
               mov     a,#00h
               addc    a,#00h
               mov     r4,a            ;40bit resultaat klaar
               
               jnb     f1,s_add323     ;als overflow, sign extention toepassen
               mov     a,r4            ;ja, sign extention toepassen!
               jnb     acc.0,s_add321  ;als lsb van r4=0, dan r4=00h
               mov     r4,#0ffh        ;als lsb van r4=1, dan r4=ffh
               sjmp    s_add322
s_add321:      mov     r4,#00h
s_add322:      pop     psw
               setb    c               ;aangeven aan de gebruiker dat het
               pop     acc             ;resultaat groter is dan 32bits! 
               ret
               
s_add323:      pop     psw             ;binnen 32bit bereik!
               clr     c
               pop     acc
               ret
               
               
;*******************************************************************************
; add32        (18,0us @16.777216MHz)
; 32-Bit Unsigned optelling (acc0 = acc0+acc1)
; Inputbereik getallen: 0 tot  +4294967295  (00000000h...ffffffffh)
; input:     r3,r2,r1,r0 = acc0
;            r7,r6,r5,r4 = acc1
;
; output:    (r4),r3,r2,r1,r0 = acc0 = acc0+acc1  
;            indien het resultaat groter wordt dan 32 bit, r4 als msb gebruiken
;            CY=0 :het resultaat zit binnen het 32bit bereik
;                  gebruik van meer beduidende byte (r4=0) is niet nodig
;            CY=1 :het resultaat is groter dan 32bit (buiten bereik) dus ook de
;                  meer beduidende byte (r4) van het resultaat gebruiken
;                          
; vernietigt:  niets
;*******************************************************************************
add32:         push    acc             ;bewaar gebruikte registers
               push    psw
               mov     a,r0           ;tel de soft accumulators op
               add     a,r4           ;low byte eerst
               mov     r0,a           ;resultaat bewaren
               mov     a,r1           ;nu de hogere bytes 
               addc    a,r5           ;optellen met eventuele overdracht
               mov     r1,a           ;16 bit resultaat is klaar
               
               mov     a,r2           ;derde bytes optellen
               addc    a,r6           ;optellen met eventuele overdracht
               mov     r2,a           ;24bit resultaat is klaar
               mov     a,r3           ;nu de ms bytes 
               addc    a,r7           ;optellen met eventuele overdracht
               mov     r3,a           ;32 bit resultaat is klaar
               
               jnc     add321          ;geen 40 bit resultaat
               
               mov     r4,#01h         ;40bit resultaat klaar
               pop     psw
               setb    c               ;aangeven aan de gebruiker dat het 
               pop     acc             ;resultaat 40 bits groot is
               ret
               
add321:        mov     r4,#00h
               pop     psw
               clr     c               ;aangeven aan de gebruiker dat het 
               pop     acc             ;resultaat 32 bits groot is
               ret


;*******************************************************************************
; s_sub16        (24,4us @16.777216MHz)
; 16-Bit Signed (2's Complement) verschil (acc0=acc0-acc1)
; Inputbereik getallen: -32768 tot  +32767 (8000h..ffff,0,1..7fffh)
; input:     r1,r0 = acc0
;            r5,r4 = acc1
;
; output:    r3,r2,r1,r0 = acc0 = acc0-acc1  met teken
;            CY=0 :het resultaat zit binnen het 16bit bereik
;                  gebruik van r3,r2 is niet nodig
;            CY=1 :als het resultaat groter werd dan 16bit (buiten bereik) dan,
;                  meer beduidende bytes (r3,r2) van het resultaat ook gebruiken
;                   
; vernietigt:  niets
;*******************************************************************************
s_sub16:        push    acc             ;bewaar gebruikte registers
                push    psw
;maak 2's complement van acc1, dan kunnen we acc0 en acc1 optellen!    

                mov   	a,r4            ;verander het teken van acc1
                cpl    	a               ;door het 2's complement te nemen
                add   	a,#1            
                mov    	r4,a            
                mov    	a,r5            
                cpl    	a               
                addc	a,#0
                mov    	r5,a
;acc0+acc1
				mov		a,r0
				add		a,r4
				mov		r0,a
				mov		a,r1
				addc	a,r5
				mov		r1,a	
                     
                mov    f0,c
                mov    c,ov          ;bewaar overflow flag voor sign extention
                mov    f1,c
                mov    c,f0            ;herstel carrybit
               
                mov    a,#00h
                addc   a,#00h
                mov    r2,a            ;24bit resultaat klaar
               
                jnb    f1,s_sub163     ;als overflow;sign extention toepassen
                mov    a,r2            ;ja, sign extention toepassen!
                jnb    acc.0,s_sub161  ;als lsb van r2=0, dan r3,r2=0000h
                mov    r2,#0ffh        ;als lsb van r2=1, dan r3,r2=ffffh
                mov    r3,#0ffh
                sjmp   s_sub162
s_sub161:       mov    r2,#00h
                mov    r3,#00h               
s_sub162:       pop    psw
                setb   c               ;aangeven aan de gebruiker dat het
                pop    acc             ;resultaat groter is dan 16bits! 
                ret
               
s_sub163:       pop    psw             ;binnen 16bit bereik!geen overflow
                clr    c
                pop    acc
                ret

;*******************************************************************************
; sub16            (14,4us @16.777216MHz)
; 16-Bit Unsigned verschil (acc0 = acc0-acc1)
; Inputbereik getallen: 0 tot  +65535 (0000h...ffffh)
; input:     r1,r0 = acc0
;            r5,r4 = acc1
;
; output:    (r2),r1,r0 = acc0 = acc0-acc1  
;            bij een underflow r2 op ffh zetten
;            CY=0 :het resultaat zit binnen het 16bit bereik
;                  gebruik van meer beduidende byte (r2) is niet nodig
;            CY=1 :underflow, het resultaat is negatief (buiten bereik) 
;                  de meer beduidende byte (r2) van het resultaat staat op ffh
;                          
; vernietigt:  niets
;*******************************************************************************

sub16:          push    acc             ;bewaar gebruikte registers
                push    psw
                clr     c
                mov     a,r0            ;tel de soft accumulators op
                subb    a,r4            ;low byte eerst
                mov     r0,a            ;resultaat bewaren
                mov     a,r1            ;nu de high bytes 
                subb    a,r5            ;verschil met eventuele overdracht
                mov    r1,a             ;16 bit resultaat is klaar
                jnc    sub161           ;geen 24 bit resultaat
               
                mov     r2,#0ffh        ;24bit resultaat klaar
                pop     psw
                setb    c               ;aangeven aan de gebruiker dat het 
                pop     acc             ;resultaat 24 bits groot is
                ret
               
sub161:         mov     r2,#00h
                pop     psw
                clr     c               ;aangeven aan de gebruiker dat het 
                pop     acc             ;resultaat 16 bits groot is
                ret

;*******************************************************************************
; s_sub32        (27,2us @16.777216MHz)
; 32-Bit Signed (2's Complement) verschil (acc0=acc0+acc1)
; Inputbereik getallen: -2147483648 tot +2147483647 
;                       (80000000h..ffffffffh,0,00000001h..7fffffffh)
; input:     r3,r2,r1,r0 = acc0
;            r7,r6,r5,r4 = acc1
;
; output:    (r4),r3,r2,r1,r0 = acc0 = acc0-acc1  met teken
;            CY=0 :het resultaat zit binnen het 32bit bereik
;                  gebruik van r4 is niet nodig
;            CY=1 :als het resultaat groter werd dan 32bit (buiten bereik) dan,  
;                  de meer beduidende byte (r4) van het resultaat ook gebruiken
;                   
; vernietigt:  niets
;*******************************************************************************
s_sub32:        push   acc             ;bewaar gebruikte registers
                push   psw
               
;maak 2's complement van acc1, dan kunnen we acc0 en acc1 optellen!    

                mov    	a,r4            ;verander het teken van acc1
                cpl    	a               ;door het 2's complement te nemen
                add    	a,#1            
                mov    	r4,a            
                mov    	a,r5            
                cpl    	a               
                addc	a,#0
                mov    	r5,a
                mov    	a,r6           
                cpl    	a               
                addc   	a,#0           
                mov    	r6,a
                mov    	a,r7           
                cpl    	a               
                addc   	a,#0
                mov    	r7,a           
;acc0+acc1
				mov		a,r0
				add		a,r4
				mov		r0,a
				mov		a,r1
				addc	a,r5
				mov		r1,a
				mov		a,r2
				addc	a,r6
				mov		r2,a
				mov		a,r3
				addc	a,r7
				mov		r3,a 
                               
                mov   f0,c
                mov   c,ov            ;bewaar overflow flag voor sign extention
                mov   f1,c
                mov   c,f0            ;herstel carrybit
               
                mov   a,#00h
                addc  a,#00h
                mov   r4,a            ;40bit resultaat klaar
               
                jnb  f1,s_sub323      ;bij overflow, sign extention toepassen
                
                mov   a,r4            ;ja, sign extention toepassen!
                jnb   acc.0,s_sub321  ;als lsb van r4=0, dan r4=00h
                mov   r4,#0ffh        ;als lsb van r4=1, dan r4=ffh
                sjmp   s_sub322
s_sub321:       mov   r4,#00h
s_sub322:       pop   psw
                setb  c                ;aangeven aan de gebruiker dat het
                pop   acc               ;resultaat groter is dan 32bits! 
                ret
               
s_sub323:       pop   psw              ;binnen 32bit bereik!geen overflow
                clr   c
                pop   acc
                ret
               
               
;*******************************************************************************
; sub32        (18,7us @16.777216MHz)
; 32-Bit Unsigned verschil (acc0 = acc0-acc1)
; Inputbereik getallen: 0 tot  +4294967295  (00000000h...ffffffffh)
; input:     r3,r2,r1,r0 = acc0
;            r7,r6,r5,r4 = acc1
;
; output:    (r4),r3,r2,r1,r0 = acc0 = acc0-acc1  
;            indien het resultaat groter wordt dan 32 bit, r4 als msb gebruiken
;            CY=0 :het resultaat zit binnen het 32bit bereik
;                  gebruik van meer beduidende byte (r4) is niet nodig
;            CY=1 :het resultaat is groter dan 32bit (buiten bereik) dus ook de
;                  meer beduidende byte (r4) van het resultaat gebruiken
;                          
; vernietigt:  niets
;*******************************************************************************
sub32:         push    acc            ;bewaar gebruikte registers
               push    psw
               clr       c
               mov     a,r0           ;maak verschil van de soft accumulators 
               subb    a,r4           ;low byte eerst
               mov     r0,a           ;resultaat bewaren
               mov     a,r1           ;nu de hogere bytes 
               subb    a,r5           ;verschil met eventuele overdracht
               mov     r1,a           ;16 bit resultaat is klaar
               
               mov     a,r2           ;derde bytes optellen
               subb    a,r6           ;verschil met eventuele overdracht
               mov     r2,a           ;24bit resultaat is klaar
               mov     a,r3           ;nu de ms bytes 
               subb    a,r7           ;verschil met eventuele overdracht
               mov     r3,a           ;32 bit resultaat is klaar
               
               jnc     sub321         ;geen 40 bit resultaat
               
               mov     r4,#0ffh       ;40bit resultaat klaar
               pop     psw
               setb    c               ;aangeven aan de gebruiker dat het 
               pop     acc             ;resultaat 40 bits groot is
               ret
               
sub321:        mov     r4,#00h
               pop     psw
               clr     c               ;aangeven aan de gebruiker dat het 
               pop     acc             ;resultaat 32 bits groot is
               ret



;*******************************************************************************
; mul16        (71,2us @16.777216MHz)
; 16-Bit Unsigned product (acc0 = acc0 x acc1) naar 32bit resultaat
; Inputbereik getallen: 0 tot  +65535 (0000h...ffffh)
; input:     r1,r0 = acc0
;            r5,r4 = acc1
;
; output:    r3,r2,r1,r0 = acc0 = acc0 x acc1  
;            0 tot  +4294836225  (00000000h...fffe0001h)
;                          
; vernietigt:  niets
;*******************************************************************************
mul16:          push    acc                ;bewaar de gebruikte registers
                push    psw
                push    b
                push    dpl
                push    dph

;we bewaren de tussenresultaten in 3 gealloceerde bytes  stackgeheugen!

                mov     dpl,r0          ;verplaats r0 en r1 naar datapointer
                mov     dph,r1
                mov     r0,sp           ;r0 = tussenresultaatpointer 
                inc     r0              ;pointer naar eerste vrije plaats
                mov     r1,sp           ;stackpointer bewaren tot einde 
                mov     a,sp            ;plaats alloceren
                add     a,#3            ;3 bytes nodig
                mov     sp,a            ;stackpointer verzetten
                
                mov     a,dpl            ;acc0 lsb
                mov     b,r4            ;acc1 lsb
                mul     ab
                mov     @r0,a            ;bewaar lsb resultaat
                inc     r0
                mov     @r0,b            ;bewaar tussenresultaat
                
                mov     a,dph            ;acc0 msb
                mov     b,r4            ;acc1 lsb
                mul     ab
                
                add     a,@r0            ;tel lsb bij tussenresultaat
                mov     @r0,a            ;bewaar als tussenresultaat
                mov     a,b              ;tel eventuele carry bij msb
                addc    a,#0
                inc     r0               ;bewaar als tussenresultaat
                mov     @r0,a
                                
                mov     a,dpl            ;acc0 lsb
                mov     b,r5             ;acc1 msb
                mul     ab
                dec     r0                 
                add     a,@r0            ;tel lsb bij tussenresultaat
                mov     @r0,a            ;en bewaar weer
                inc     r0
                mov     a,b              ;tel msb bij tussenresultaat
                addc    a,@r0
                mov     @r0,a            ;en bewaar weer
                mov     f0,c             ;bewaar de carry van de laatste addc
                
                mov     a,dph            ;acc0 msb
                mov     b,r5             ;acc1 msb
                mul     AB
                add     a,@r0            ;tel lsb bij tussenresultaat
                mov     r2,a
                mov     a,b
                addc    a,#0
                
                mov     c,f0             ;tel opgeslagen carry hier ook nog bij
                addc    a,#0
                
                mov     r3,a         ;product is klaar, outputregisters vullen
                dec     r0
                mov     dph,@r0
                dec     r0
                mov     dpl,@r0
                
                mov     sp,r1            ;zet stackpointer terug
                mov     r1,dph
                mov     r0,dpl
                
                pop     dph              ;herstel de bewaarde registers
                pop     dpl
                pop     b
                pop     psw
                pop     acc
                ret



;*******************************************************************************
; mul16acc1        (71,2us @16.777216MHz)
; 16-Bit Unsigned product (acc1 = acc1low x acc1high) naar 32bit resultaat
; Inputbereik getallen: 0 tot  +65535 (0000h...ffffh)
; input:     r5,r4 = acc1 low
;            r7,r6 = acc1 high
;
; output:    r7,r6,r5,r4 = acc1 = acc1(low) x acc1(high)  
;            0 tot  +4294836225  (00000000h...fffe0001h)
;                         
; vernietigt:  niets  
;*******************************************************************************
mul16acc1:      push    acc              ;bewaar de gebruikte registers
                push    psw
                push    b
                push    dpl
                push    dph

;we bewaren de tussenresultaten in 3 gealloceerde bytes  stackgeheugen!

                mov     dpl,r0           ;verplaats r0 en r1 naar datapointer
                mov     dph,r1           ;om ze als pointer te gebruiken
                
                mov     r0,sp            ;r0 = tussenresultaatpointer 
                inc     r0               ;pointer naar eerste vrije plaats
                mov     r1,sp            ;stackpointer bewaren tot einde 
                mov     a,sp             ;plaats alloceren
                add     a,#3             ;3 bytes nodig
                mov     sp,a             ;stackpointer verzetten
                
                mov     a,r4             ;acc1 low lsb
                mov     b,r6             ;acc1 high lsb
                mul     ab
                mov     @r0,a            ;bewaar lsb tussenresultaat
                inc     r0
                mov     @r0,b            ;bewaar msb tussenresultaat
                
                mov     a,r5             ;acc1 low msb
                mov     b,r6             ;acc1 high lsb
                mul     ab
                
                add     a,@r0            ;tel lsb bij msb tussenresultaat
                mov     @r0,a            ;bewaar als tussenresultaat
                mov     a,b              ;tel eventuele carry bij msb
                addc    a,#0
                inc     r0               ;bewaar als tussenresultaat
                mov     @r0,a
    
                mov     a,r4             ;acc1 low lsb
                mov     b,r7             ;acc1 high msb
                mul     ab
                dec     r0
                add     a,@r0            ;tel lsb bij tussenresultaat
                mov     @r0,a            ;en bewaar weer
                inc     r0
                mov     a,b              ;tel msb bij tussenresultaat
                addc    a,@r0
                mov     @r0,a            ;en bewaar weer
                mov     f0,c             ;bewaar de carry van de laatste addc
                
                mov     a,r5             ;acc1 low msb
                mov     b,r7             ;acc1 high msb
                mul     AB
                
                add     a,@r0            ;tel lsb bij tussenresultaat
                mov     r6,a             ;deze byte van product is al klaar
                mov     a,b
                addc    a,#0
                mov     c,f0            ;tel opgeslagen carry hier ook nog bij
                addc    a,#0
                
                mov     r7,a          ;product is klaar, outputregisters vullen
                dec     r0
                mov     a,@r0
                mov     r5,a
                dec     r0
                mov     a,@r0
                mov     r4,a             ;r7,r6,r5,en r4 zijn klaar 
                
                mov     sp,r1            ;zet stackpointer terug
                mov     r1,dph
                mov     r0,dpl
                
                pop     dph              ;herstel de bewaarde registers
                pop     dpl
                pop     b
                pop     psw
                pop     acc
                ret
                
;*******************************************************************************
; s_mul16        (121,1us @16.777216MHz)
; 16-Bit signed product (acc0 = acc0 x acc1) naar 32bit signed resultaat
; Inputbereik getallen: 8000h..ffffh,0,0001h..7fffh  (-32768 tot  +32767) 
; input:     r1,r0 = acc0
;            r5,r4 = acc1
;
; output:    r3,r2,r1,r0 = acc0 = acc0 x acc1      
;                                  -1073709056 tot +1073741824
;                                  (c0008000h..ffffffffh,0,00000001h..40000000)
;                          
; vernietigt:  niets
;*******************************************************************************
s_mul16:        lcall    sigmag16acc0    ;acc0 = absolute waarde + teken f0
                lcall    sigmag16acc1    ;acc1 = absolute waarde + teken f1
                lcall    mul16           ;maak product
                lcall    magsig32        ;maak 2's complement formaat
                ret                      ;klaar

;*******************************************************************************
; s_mul16acc1        (121,1us @16.777216MHz)
; 16-Bit signed product (acc1 = acc1high x acc1low) naar 32bit signed resultaat
; Inputbereik getallen: 8000h..ffffh,0,0001h..7fffh  (-32768 tot  +32767) 
; input:     r5,r4 = acc1 low
;            r7,r6 = acc1 high
;
; output:    r7,r6,r5,r4 = acc1 = acc1(low word) x acc1(high word)  
;                                  -1073709056 tot +1073741824
;                                  (c0008000h..ffffffffh,0,00000001h..40000000)                          
; vernietigt:  niets
;*******************************************************************************
s_mul16acc1:    lcall    sigmag16acc1    ;acc1 = absolute waarde + teken f1
                lcall    sigmag16acc1h   ;acc1 high = absolute waarde + teken f0
                lcall    mul16acc1       ;maak product
                lcall    magsig32acc1    ;maak 2's complement formaat
                ret                      ;klaar
 
;*******************************************************************************
; s_mul816        (83,5us @16.777216MHz)
; 8bit x 16-Bit signed product (acc0 = acc1 x acc0) naar 24bit signed resultaat
; Inputbereik getallen: acc0: 8000h..ffffh,0,0001h..7fffh (-32768d tot  +32767d) 
;                        acc1: 80h..ffh,0,01h..7fh         (-128d tot  +127d)
; input:     r1,r0 = acc0
;               r4 = acc1
;
; output:    r3,r2,r1,r0 = acc0 = acc0 x acc1  (24 bit resultaat, r3=00h)
;                                  -4194176 tot +4194304 
;                                  (c00080h..ffffffh,0,000001h..400000h)                          
; vernietigt:  niets
;******************************************************************************* 
s_mul816:       lcall    sigmag16acc0    ;acc0 = absolute waarde + teken f0
                lcall    sigmag8acc1     ;acc1 = absolute waarde + teken f1
                lcall    mul816          ;8bit x 16 bit product
                lcall    magsig32        ;abs. waarde naar 2's complement
                ret
                
;*******************************************************************************
; mul816        (29,4us @16.777216MHz)
; 8bit x 16-Bit Unsigned product (acc0 = acc1 x acc0) naar 24bit resultaat
; Inputbereik getallen: acc0: 0 tot 65535 (0000h...ffffh)
;                       acc1: 0 tot 255     (00h...ffh)
; input:     r1,r0 = acc0
;               r4 = acc1
;
; output:    r3,r2,r1,r0 = acc0 = acc0 x acc1  (24 bit resultaat,r3=00h)
;                           0..16711425 (000000h..feff01h)
;                          
; vernietigt:  niets
;*******************************************************************************                
mul816:         push    acc             ;bewaar registers
                push    b
                push    psw
                mov     a,r4            ;neem acc1
                mov     b,r0            ;neem lsb van acc0
                mul     ab              ;maak product
                mov     r0,a            ;bewaar lsb van het product is klaar
                push    b               ;bewaar lsb van het product
                mov     a,r4            ;neem acc1
                mov     b,r1            ;neem msb can acc0
                mul     ab              ;maak product
                mov     r2,b            ;bewaar msb van product even
                mov     b,a             ;verplaats lsb van product
                pop     acc             ;neem msb van vorig product
                add     a,b             ;en tel bij lsb van laatste product
                mov     r1,a            ;de middenste byte is klaar
                mov     a,r2            ;tel nu de eventuele
                addc    a,#0            ;carry bij de msb
                mov     r2,a            ;en bewaar, de msb is klaar
                mov     r3,#0
                pop     psw
                pop     b
                pop     acc
                ret
 
;*******************************************************************************
; s_mac16        (145,1us @16.777216MHz)
; 16-Bit signed product naar 32bit signed resultaat plus signed optelling 
; met 32bit resultaat in acc0,  acc0 = (acc1low x acc1high) + acc0
; Inputbereik getallen: 8000h..ffffh,0,0001h..7fffh  (-32768 tot  +32767) 
; input:     r7,r6 = acc1 (high word)
;            r5,r4 = acc1 (low  word)
;
; output:    r7,r6,r5,r4 = acc1 = product acc1(low) x acc1(high)
;            r3,r2,r1,r0 = acc0 = acc1 +acc0
;                          -2147483648 tot +2147483647 
;                          (80000000h..ffffffffh,0,00000001h..7fffffffh)  
;            CY=0 :het resultaat zit binnen het 32bit bereik
;            CY=1 :het resultaat is groter dan 32bit (buiten bereik) 
;                          
; vernietigt:  niets
;******************************************************************************* 
s_mac16:        lcall    s_mul16acc1     ;product van acc1 low en acc1 high
                push    acc
                push    psw
                mov     a,r4            ;tel r7,r6,r5,r4 op bij
                add     a,r0            ;het getal in r3,r2,r1,r0
                mov     r0,a
                mov     a,r5
                addc    a,r1
                mov     r1,a
                mov     a,r6
                addc    a,r2
                mov     r2,a
                mov     a,r7
                addc    a,r3
                mov     r3,a
                jb      ov,s_mac161
                pop     psw
                clr     c                ;ok, 32bit som binnen bereik
                pop     acc
                ret
s_mac161:       pop     psw
                setb    c                ;fout! 32bit som buiten bereik
                pop     acc
                ret
 
 
;*******************************************************************************
; s_div8        (78us @16.777216MHz)
; signed  8-Bit (acc0) door 8-Bit (acc1) deling naar 8-Bit Quotient en 8bit rest 
; Inputbereik getallen: 80h..ffh,0,01h..7fh  (-128 tot  +127)  
; input:    r0 = acc0 deeltal (2's compl)
;           r4 = acc1 deler   (2's compl)
;
; output:   r0 = quotient van deling acc0/acc1 (2's compl)
;           r4 = rest (2's compl)
;           cy=1 bij acc1=0 (deling door nul)
;            cy=0 in andere gevallen
;
; vernietigt: niets
;*******************************************************************************
s_div8:         lcall   sigmag8acc0      ;2's comp naar abs.waarde en teken f0
                lcall   sigmag8acc1      ;2's comp naar abs.waarde en teken f1
                lcall   div8
                jc      s_div81          ;bij deling door nul, gedaan!
                lcall   magsig8          ;abs.waarde en teken naar 2's Compl
                lcall   magsig8acc1      ;rest ook omzetten naar 2's compl
s_div81:        ret
                
;*******************************************************************************
; div8        (19,4us @16.777216MHz)
; 8-Bit (acc0) door 8-Bit (acc1) deling naar 8-Bit Quotient en 8bit rest
; absolute waarde
; Inputbereik getallen: 00h..ffh  (0 tot 255) 
; input:    r0 = acc0 deeltal
;           r4 = acc1 deler
;
; output:   r0 = quotient van deling acc0/acc1
;           r4 = rest 
;           cy=1 bij acc1=0 (deling door nul)
;            cy=0 in andere gevallen
;          
; vernietigt: niets
;*******************************************************************************
div8:           push    acc             ;bewaar registers
                push    b
                push    psw
                mov     a,r4            ;neem deler
                jz      div81           ;deling door nul?
                mov     a,r0            ;nee, neem deeltal
                mov     b,r4            ;en deler
                div     ab              ;en maak deling
                mov     r0,a            ;bewaar quotient in acc0
                mov     r4,b            ;bewaar rest in acc1
                pop     psw             ;herstel registers
                pop     b                
                pop     acc
                clr     c                ;geen deling door nul
                ret
                
div81:          pop     psw              ;fout, deling door nul!
                pop     b
                pop     acc
                setb    c
                ret


;*******************************************************************************
; s_div16        (469,1us @16.777216MHz)
; 16Bit (acc0) door 16Bit (acc1) deling naar 16Bit Quotient en 16bit rest
; 2's Complement Format
; Inputbereik getallen: 8000h..ffffh,0,0001h..7fffh  (-32768 tot  +32767) 
; input:    r1, r0 = acc0 deeltal (2's compl)
;           r5, r4 = acc1 deler   (2's compl)
;
; output:   r1, r0 = quotient van deling acc0/acc1 (2's compl)
;           r5, r4 = rest
;           cy=1 bij acc1=0 (deling door nul)
;            cy=0 in andere gevallen
;
; vernietigt: niets
;*******************************************************************************
s_div16:        push    acc
                push    psw
                mov     a,r5            ;neem msb van deler 
                orl     a,r4            ;combineer met lsb van deler
                jnz     s_div161        ;deler is ok als dit niet nul is
                pop     psw             ;anders fout, deling door nul!
                pop     acc
                setb    C               ;fout, deling door nul!
                ret

s_div161:       lcall   sigmag16acc0    ;2's comp naar abs. waarde en teken f0
                lcall   sigmag16acc1    ;2's comp naar abs. waarde en teken f1
                lcall   div16
                lcall   magsig16        ;abs.waarde en f0 en f1 naar 2's Compl
                lcall   magsig16acc1    ;rest ook omzetten naar 2's compl
                pop     psw
                clr     C               ;delig ok
                pop     acc
                ret                     ;klaar
                
;*******************************************************************************
; div16            (405,1us @16.777216MHz)
; 16-Bit / 16-Bit naar 16-Bit Quotient & Rest absolute waarde deling
; Inputbereik getallen: 0000h..ffffh  (0 tot 65535d) 
; input:    r1, r0 = Deeltal (acc0)
;           r5, r4 = Deler   (acc1)
;
; output:   r1, r0 = quotient van de deling acc0/acc1
;           r5, r4 = rest van de deling acc0/acc1
;            cy=1 bij deling door nul (acc1=0)
;            cy=0 bij normale deling
;
; vernietigt: niets
;*******************************************************************************

div16:          push    acc            ;bewaar de registers waar we mee werken
                push    psw
;check voor deling door nul
                mov     a,r4           ;neem lsb deler
                orl     a,r5           ;combineer met msb van deler
                jnz     div160         ;niet nul, dan verder werken
                pop     psw            ;als nul, dikke snul                
                setb    c              ;terug met errorstatus
                pop     acc
                ret
                
div160:         push    b              ;registers verder bewaren
                mov     a,r7
                push    acc            ;push r7 op stack
                mov     a,r6
                push    acc            ;push r6 op stack
                mov     a,r3
                push    acc            ;push r3 op stack
                mov     a,r2
                push    acc            ;push r2 op stack
                push    dph
                push    dpl
                
                mov     r2,#0          ;tijdelijk 16 bit quotient op nul zetten
                mov     r3,#0
                mov     r7,#0          ;gedeeltelijke 16 bit rest op nul zetten
                mov     r6,#0
                mov     b,#16          ;initiele lusteller op 16
                
;om zo weinig mogelijk de delerlus (div162) te moeten doorlopen aligneren we het
;deeltal met zijn meest significante bit op de msb van het deeltal en we tellen 
;het aantal significante bits af in de lusteller b
                
                mov     a,r1            ;neem de msbyte van het deeltal
div161:         jb      acc.7,div162    ;test of hoogste bit significant is
                clr     c               ;nee, het is een nul, deeltal opschuiven
                mov     a,r0            ;lsb van deeltal nemen
                rlc     a                ;en opschuiven
                mov     r0,a
                mov     a,r1            ;msb van deeltal nemen
                rlc     a
                mov     r1,a
                djnz    b,div161        ;volgende bit en lusteller decrementeren 
;lusteller=0, dus deeltal is nul, het resultaat is dus gekend acc0=0 en acc1=0
                mov     r4,#0
                mov     r5,#0
                sjmp    div164          ;klaar!
                
;deeltal is nu msb gealigneerd en klaar voor deling

div162:            clr     C            ;start met carry op nul
                mov     a,r0            ;schuif deeltal naar links
                rlc     a               
                mov     r0,a
                mov     a,r1
                rlc     a
                mov     r1,a
                mov     a,r6            ;schuif carry van deeltal in de 
                rlc     a               ;gedeeltelijke rest
                mov     r6,a
                mov     a,r7
                rlc     a
                mov     r7,a
;trek de deler van de gedeeltelijke rest af en bewaar tijdelijk verschil            
                mov     a,r6
                clr     C
                subb    a,r4
                mov     dpl,a
                mov     a,r7
                subb    a,r5
                mov     dph,a
                jc      div163
                mov     r7,dph          ;als carry nul, dan tijdelijk verschil
                mov     r6,dpl          ;in gedeeltelijke rest stoppen
                
div163:         cpl     C               ;geinverteerde cy in tijdelijk quotient
                mov     a,r2            ;binnenschuiven
                rlc     a
                mov     r2,a
                mov     a,r3
                rlc     a
                mov     r3,a            ;tijdelijk quotient is aangepast
                djnz    b,div162        ;doe zolang lusteller niet nul is
                
                mov     a,r3            ;acc0 vullen met 16 bit quotient 
                mov     r1,a            
                mov     a,r2
                mov     r0,a
                mov     a,r7            ;acc1 vullen met de 16 bit rest
                mov     r5,a
                mov     a,r6
                mov     r4,a
                
div164:         pop     dpl             ;registers herstellen
                pop     dph
                pop     acc
                mov     r2,a            ;r2 van stack halen
                pop     acc
                mov     r3,a            ;r3 van stack halen
                pop     acc
                mov     r6,a            ;r6 van stack halen
                pop     acc
                mov     r7,a            ;r7 van stack halen
                pop     b
                pop     psw
                clr     c               ;deling ok
                pop     acc
                ret
                

;*******************************************************************************
; s_div32        (2,15ms @16.777216MHz)
; 32Bit (acc0) door 33Bit (acc1) deling naar 32Bit Quotient en 32bit rest
; 2's Complement Format
; Inputbereik getallen: -2147483648 tot +2147483647 
;                       (80000000h..ffffffffh,0,00000001h..7fffffffh)  
; 
; input:    r3,r2,r1,r0 = acc0 deeltal (2's compl)
;           r7,r6,r5,r4 = acc1 deler   (2's compl)
;
; output:   r3,r2,r1, r0 = quotient van deling acc0/acc1 (2's compl)
;           r7,r6,r5, r4 = rest
;           cy=1 bij acc1=0 (deling door nul)
;            cy=0 in andere gevallen
;
; vernietigt: niets
;*******************************************************************************
s_div32:        push    acc
                push    psw
                mov     a,r7            ;neem msb van deler 
                orl     a,r6            ;combineer met lsb's van deler
                orl     a,r5
                orl     a,r4
                jnz     s_div321        ;deler is ok als dit niet nul is
                pop     psw                ;anders fout, deling door nul!
                pop     acc
                setb    C                ;fout, deling door nul!
                ret

s_div321:       lcall   sigmag32acc0    ;2's comp naar abs. waarde en teken f0
                lcall   sigmag32acc1    ;2's comp naar abs. waarde en teken f1
                lcall   div32            ;deel 32bit/32bit
                lcall   magsig32        ;abs.waarde en f0 en f1 naar 2's Compl
                lcall    magsig32acc1    ;rest ook omzetten naar 2's compl
                pop     psw
                clr     C                ;delig ok
                pop     acc
                ret                        ;klaar
                

;*******************************************************************************
; div32        (2,09ms @16.777216MHz)
; 32-Bit / 32-Bit naar 32-Bit Quotient & 32-bit Rest (absolute waarde deling)
; Inputbereik getallen: 00000000h..ffffffffh  (0 tot +4294967296d) 
; input:    r3,r2,r1, r0 = Deeltal (acc0)
;           r7,r6,r5, r4 = Deler   (acc1)
;
; output:   r3,r2,r1, r0 = quotient van de deling acc0/acc1
;           r7,r6,r5, r4 = rest van de deling acc0/acc1
;            cy=1 bij deling door nul (acc1=0)
;            cy=0 bij normale deling
;
; vernietigt: niets
;*******************************************************************************
div32:          push    acc             ;bewaar de registers waar we mee werken
                push    psw
;check voor deling door nul
                mov     a,r7            ;neem msb van deler 
                orl     a,r6            ;combineer met lsb's van deler
                orl     a,r5
                orl     a,r4
                jnz     div320          ;niet nul, dan verder werken
                pop     psw             ;als nul, dikke snul                
                setb    c               ;terug met errorstatus
                pop     acc
                ret
                
div320:         push    b                ;registers verder bewaren
                push    dph
                push    dpl
                
                mov     b,#32            ;initiele lusteller op 32
                
;om zo weinig mogelijk de delerlus (div322) te moeten doorlopen aligneren we het
;deeltal met zijn meest significante bit op de msb van het deeltal en we tellen 
;het aantal significante bits af in de lusteller b

                mov     a,r3            ;neem de msbyte van het deeltal
div320a:        jb     acc.7,div320b    ;test of hoogste bit significant is
                clr     c               ;nee, het is een nul, deeltal opschuiven
                mov     a,r0            ;lsb van deeltal nemen
                rlc     a               ;en opschuiven
                mov     r0,a
                mov     a,r1           ;volgende byte van deeltal nemen
                rlc     a
                mov     r1,a
                mov     a,r2           ;volgende byte van deeltal nemen
                rlc     a
                mov     r2,a
                mov     a,r3           ;lsb van deeltal nemen
                rlc     a
                mov     r3,a
                djnz    b,div320a       ;volgende bit en lusteller decrementeren 
;lusteller=0, dus deeltal is nul, het resultaat is dus gekend acc0=0 en acc1=0
                mov     r7,#0           ;rest is dan ook nul
                mov     r6,#0
                mov     r5,#0
                mov     r4,#0
                ljmp    div324           ;klaar!
                
;Alloceer 12 bytes voor lokale variabelen op de stack.
;In de vier laagste bytes wordt de deler opgeslagen, de vier hogere bytes zijn
;het tijdelijk verschil (X=rest-deler) en de vier hoogste bytes is het quotient.
;We maken twee pointers vrij die wijzen naar de deler en het tijdelijk verschil
;en in dpl bewaren we het adres van de lage byte van het quotient.
                
div320b:        
                mov     dph,sp          ;stackpointer opslaan voor na de deling
                
                mov     a,sp            ;we gaan nu de stackpointer verzetten
                add     a,#12           ;met 12 plaatsen
                mov     sp,a            ;nieuwe stackpointer klaar
                
                mov     a,r1            ;bewaar r1 op nieuwe stack
                push    acc
                mov     a,r0            ;bewaar r0 op nieuwe stack
                push    acc
                
                mov     r0,dph          ;pointer maken naar de lage byte van
                inc     r0              ;de deler
                mov     a,dph           ;pointer maken naar de lage byte van 
                add     a,#5            ;het tijdelijk verschil X
                mov     r1,a            ;pointer naar tijdelijk verschil
                mov     a,dph           ;adres berekenen van lage byte van het
                add     a,#9            ;quotient
                mov     dpl,a           ;opslaan voor gebruik als pointer
                
;Eenmalig de variabelen op stack initaliseren                
;Tijdelijk quotient op nul zetten

                xch     a,dpl            ;lsb adres van quotient in accu
                xch     a,r1             ;r1=lsb adres van quotient, a=X
                push    b                ;loopteller deling bewaren
                mov     b,#4
div320c:        mov     @r1,#0           ;maak quotient nul
                inc     r1
                djnz    b,div320c
                pop     b                ;loopteller deling ophalen
                dec     r1
                dec     r1
                dec     r1
                dec     r1               ;pointer weer op startadres quotient 
                xch     a,r1             ;weer omwisselen
                xch     a,dpl
                
;deler in r7,r6,r5,r4 in lokale variabele op stack zetten
                mov     a,r4
                mov     @r0,a
                inc     r0
                mov     a,r5
                mov     @r0,a
                inc     r0
                mov     a,r6
                mov     @r0,a
                inc     r0
                mov     a,r7
                mov     @r0,a
                dec     r0             ;pointer herstellen naar lsb van deler
                dec     r0
                dec     r0
                
                mov     r7,#0          ;gedeeltelijke 32 bit rest op nul zetten
                mov     r6,#0
                mov     r5,#0
                mov     r4,#0
                
                
;deeltal is reeds msb gealigneerd en klaar voor deling
div322:         clr     C               ;start met carry op nul

                pop     acc             ;low byte r0 van deeltal weer van stack
                rlc     a
                push    acc             ;r0 van deeltal weer op stack
                dec     sp              ;wijs naar r1 van deeltal op stack
                pop     acc             ;r1 van deeltal van stack
                rlc     a
                push    acc             ;en weer bewaren
                inc     sp              ;stackpointer weer corrigeren
                                
                mov     a,r2
                rlc     a
                mov     r2,a
                mov     a,r3
                rlc     a
                mov     r3,a            ;deeltal is 1 plaats verschoven
                
                mov     a,r4            ;schuif carry van deeltal in de 
                rlc     a               ;gedeeltelijke rest r7..r4
                mov     r4,a
                mov     a,r5
                rlc     a
                mov     r5,a
                mov     a,r6
                rlc     a
                mov     r6,a
                mov     a,r7
                rlc     a
                mov     r7,a            ;tijdelijke rest is verschoven
                
;trek de deler van de gedeeltelijke rest af en bewaar tijdelijk verschil X    
                
                clr     C               ;de deler zit in gealloceerd geheugen!!
                mov     a,r4            ;neem lsb van gedeeltelijke rest
                subb    a,@r0           ;trek er de lsb deler af
                mov     @r1,a           ;en bewaar in tijdelijk verschil
                inc     r0              ;pointers aanpassen
                inc     r1
                mov     a,r5            ;volgende byte van gedeeltelijke rest
                subb    a,@r0           ;trek er volgende byte deler af
                mov     @r1,a           ;en bewaar in tijdelijk verschil
                inc     r0              ;pointers aanpassen
                inc     r1
                mov     a,r6            ;volgende byte van gedeeltelijke rest
                subb    a,@r0           ;trek er volgende byte deler af
                mov     @r1,a           ;en bewaar in tijdelijk verschil
                inc     r0              ;pointers aanpassen
                inc     r1
                mov     a,r7            ;msb van gedeeltelijke rest
                subb    a,@r0           ;trek er msb deler af
                mov     @r1,a           ;en bewaar in tijdelijk verschil
                dec     r0              ;pointers herstellen naar lsb's
                dec     r0
                dec     r0
                dec     r1
                dec     r1
                dec     r1
                
                jc      div323          ;test of er een carry werd gegenereerd
                mov     a,@r1           ;nee, dan het tijdelijk verschil
                mov     r4,a            ;in de gedeeltelijke rest stoppen
                inc     r1
                mov     a,@r1
                mov     r5,a
                inc     r1
                mov     a,@r1
                mov     r6,a
                inc     r1
                mov     a,@r1
                mov     r7,a
                dec     r1              ;met pointer weer naar lsb van 
                dec     r1              ;het tijdelijk verschil wijzen
                dec     r1
                
                
div323:         cpl     C               ;/cy in tijdelijk quotient schuiven
                xch     a,dpl           ;lsb adres van quotient in accu
                xch     a,r1            ;r1=lsb adres van quotient, a=X 
                push    acc            ;pointer naar verschil X even bewaren
                mov     a,@r1           ;neem lsb van qoutient
                rlc     a               ;geinverteerde cy binnenschuiven
                mov     @r1,a
                inc     r1              ;volgende byte
                mov     a,@r1
                rlc     a
                mov     @r1,a
                inc     r1              ;volgende byte
                mov     a,@r1
                rlc     a
                mov     @r1,a
                inc     r1              ;volgende byte
                mov     a,@r1
                rlc     a
                mov     @r1,a           ;tijdelijk quotient is aangepast
                dec     r1
                dec     r1
                dec     r1              ;pointer verzetten naar lsb quotient
                pop     acc             ;pointer naar verschil X ophalen
                xch     a,r1            ;r1=pointer naar verschil
                xch     a,dpl           ;dpl=adres lsb quotient
                
                djnz    b,div322        ;doe zolang lusteller niet nul is
                
;De deling is klaar, nu nog de output registers vullen met het resultaat
;In r7,r6,r5,r4 staat reeds de rest, dus enkel het quotient nog verplaatsen

                mov     r0,sp           ;pointer maken naar msb quotient
                dec     r0
                dec     r0
                mov     a,@r0           ;msb van quotient eerst nemen
                mov     r3,a            ;en verplaatsen
                dec     r0              ;volgende byte
                mov     a,@r0            
                mov     r2,a
                dec     r0              ;volgende byte
                mov     a,@r0            
                mov     r1,a
                dec     r0              ;volgende byte
                mov     a,@r0            
                mov     r0,a            ;laatste byte verplaatst!
                
                mov     sp,dph          ;plaats van de lokale variabelen 
                                         ;weer vrijgeven
                
                            
div324:         pop     dpl             ;registers herstellen
                pop     dph
                pop     b
                pop     psw
                clr     c               ;normale deling
                pop     acc
                ret
                
;*******************************************************************************
; s_muldiv        (2,08ms @16.777216MHz)
; 16-Bit signed product acc1(low word) x acc1(high word) naar 32bit signed 
; resultaat  gevolgd door 32bit signed deling door 32 bit getal acc0 met als
; resultaat een 32bit quotient in acc0 en 32bit rest in acc1
;
; Inputbereik getallen: 8000h..ffffh,0,0001h..7fffh  (-32768 tot  +32767) 
;
; input:     r5,r4 = acc1 (low word product)
;            r7,r6 = acc1 (high word product)
;             r3,r2,r1,r0 = acc0  32bit deler    
;    
; output:    r3,r2,r1,r0 = acc0 = quotient 
;            r7,r6,r5,r4 = acc1 = rest
;                          -2147483648 tot +2147483647 
;                          (80000000h..ffffffffh,0,00000001h..7fffffffh)  
;             cy=1 bij deling door nul (acc0=0)
;             cy=0 bij normale deling
;                                      
; vernietigt:  niets
;*******************************************************************************                 
s_muldiv:       push    acc
                push    psw
                mov     a,r0            ;neem msb van deler 
                orl     a,r1            ;combineer met lsb's van deler
                orl     a,r2
                orl     a,r3
                jnz     s_muldiv1       ;deler is ok als dit niet nul is
                pop     psw             ;anders fout, deling door nul!
                pop     acc
                setb    C               ;fout, deling door nul!
                ret

s_muldiv1:      lcall    sigmag16acc1    ;acc1 low naar absolute waarde
                lcall    sigmag16acc1h   ;acc1 high naar absolute waarde
                lcall    muldiv          ;maak product en daarna de deling
                lcall    magsig32        ;quotient naar 2's complement
                lcall    magsig32acc1    ;rest naar 2's complement
                pop     psw
                clr     c                ;deling is ok
                pop    acc
                ret
                
;*******************************************************************************
; muldiv        (2,0ms @16.777216MHz)
; 16-Bit absolute waarde product acc1(low word) x acc1(high word) naar 32bit 
; absolute waarde resultaat  gevolgd door een 32bit absolute waarde deling door
; acc0 als deler met als resultaat een 32bit quotient in acc0 en 
; 32bit rest in acc1.
; acc1(low word) x acc1(high word) / acc0 = acc0(Quotient) en acc1(rest)
;
; Inputbereik getallen: 0000h..ffffh (0 tot 65535d) 
;
; input:     r5,r4 = acc1 (low word product)
;            r7,r6 = acc1 (high word product)
;             r3,r2,r1,r0 = acc0  32bit deler    
;    
; output:    r3,r2,r1,r0 = acc0 = quotient 
;            r7,r6,r5,r4 = acc1 = rest
;                          0 tot  +4294967296  (00000000h...ffffffffh) 
;             cy=1 bij deling door nul (acc0=0)
;             cy=0 bij normale deling
;                                      
; vernietigt:  niets
;*******************************************************************************                
muldiv:         push    acc
                push    psw
                mov     a,r0            ;neem msb van deler 
                orl     a,r1            ;combineer met lsb's van deler
                orl     a,r2
                orl     a,r3
                jnz     muldiv1         ;deler is ok als dit niet nul is
                pop     psw             ;anders fout, deling door nul!
                pop     acc
                setb    C               ;fout, deling door nul!
                ret
    
muldiv1:        lcall    mul16acc1      ;16 bit product maken
                mov     a,r4            ;registers verplaatsen voor deling
                push    acc
                mov     a,r5
                push    acc
                mov     a,r6
                push    acc
                mov     a,r7
                push    acc             ;product staat nu op stack
                
                mov     a,r0            ;deler in acc1 stoppen
                mov     r4,a
                mov     a,r1
                mov     r5,a
                mov     a,r2
                mov     r6,a
                mov     a,r3
                mov     r7,a            ;deler zit volledig in acc1
                
                pop     acc             ;deeltal in acc0 stoppen
                mov     r3,a
                pop     acc
                mov     r2,a
                pop     acc
                mov     r1,a
                pop     acc
                mov     r0,a            ;deeltal zit volledig in acc0
                lcall   div32           ;maak deling
                
                pop     psw            ;herstel registers
                pop     acc
                clr     C              ;resultaat ok!
                ret
                
;*******************************************************************************
; mul16_pi        (257,1us @16.777216MHz)    
; acc0 = acc0 x pi            
; 16 bit absolute waarde product van acc0 low (r1,r0) met pi=3.1416            
; Inputbereik getallen: 0000h..ffffh (0 tot 65535d) 
;
; input:     r1,r0 = acc0 (low word)
;                
; output:    r3,r2,r1,r0 = acc0 = acc0 x pi (0 tot 03243ch) r3=00h         
;                                      
; vernietigt:  niets
;*******************************************************************************                
mul16_pi:       push    acc            ;registers bewaren
                push    psw
                mov     a,r4
                push    acc
                mov     a,r5
                push    acc
                mov     r5,#0c9h       ;acc1 laden met pi x 2^14 
                mov     r4,#0fH
                lcall   mul16           ;acc0 = acc0 x acc1
                mov     r4,#14          ;14 plaatsen naar rechts schuiven
                lcall   shiftright32    ;om te delen door 2^14
                pop     acc
                mov     r5,a
                pop     acc
                mov     r4,a
                pop     psw
                pop     acc
                ret

;*******************************************************************************
; mul16_i_pi        (279,1us @16.777216MHz)
; acc0 = acc0 x 1/pi    
; 16 bit absolute waarde product acc0 low (r1,r0) met (1/pi)=0,3183            
; Inputbereik getallen: 0000h..ffffh (0 tot 65535d) 
;
; input:     r1,r0 = acc0 (low word)
;                
; output:    r3,r2,r1,r0 = acc0 = acc0 x 1/pi (0 tot +20860d) (0000h tot 517ch)          
;                                      
; vernietigt:  niets
;*******************************************************************************
mul16_i_pi:     push    acc             ;registers bewaren
                push    psw
                mov     a,r4
                push    acc
                mov     a,r5
                push    acc
                mov     r5,#051h        ;acc1 laden met 1/pi x 2^16 
                mov     r4,#07dH
                lcall    mul16          ;acc0 = acc0 x acc1
                mov     r4,#16          ;16 plaatsen naar rechts schuiven
                lcall   shiftright32    ;om te delen door 2^16
                pop     acc
                mov     r5,a
                pop     acc
                mov     r4,a
                pop     psw
                pop     acc
                ret

;*******************************************************************************
; mul16_sqrt2        (268,1us @16.777216MHz)
; acc0 = acc0 x sqrt2
; Vermenigvuldigt acc0 low (r1,r0) met sqrt2 (2^1/2) =1,1442        
; Inputbereik getallen: 0000h..ffffh (0 tot 65535d) 
;
; input:     r1,r0 = acc0 (low word)
;                
; output:    r3,r2,r1,r0 = acc0 = acc0 x sqrt2
;                                 0 tot  +74985  (000000h … 0124e9ch)           
;                                      
; vernietigt:  niets
;*******************************************************************************
mul16_sqrt2:    push    acc             ;registers bewaren
                push    psw
                mov     a,r4
                push    acc
                mov     a,r5
                push    acc
                mov     r5,#0b5h        ;acc1 laden met sqrt2x2^15 
                mov     r4,#005H
                lcall   mul16           ;acc0 = acc0 x acc1
                mov     r4,#15          ;15 plaatsen naar rechts schuiven
                lcall   shiftright32    ;om te delen door 2^15
                pop     acc
                mov     r5,a
                pop     acc
                mov     r4,a
                pop     psw
                pop     acc
                ret



;*******************************************************************************
; mul16_i_sqrt2            (279,1us @16.777216MHz)
; acc0 = acc0 x 1/sqrt2    
; Vermenigvuldigt acc0 low (r1,r0) met 1/sqrt2=0,7071        
; Inputbereik getallen: 0000h..ffffh (0 tot 65535d) 
;
; input:     r1,r0 = acc0 (low word)
;                
; output:    r3,r2,r1,r0 = acc0 = acc0 x 1/sqrt2 (0 tot 46340d)  (0000h..b504h)         
;                                      
; vernietigt:  niets
;*******************************************************************************            
mul16_i_sqrt2:  push    acc             ;registers bewaren
                push    psw
                mov     a,r4
                push    acc
                mov     a,r5
                push    acc
                mov     r5,#0b5h        ;acc1 laden met 1/sqrt2 x2^16
                mov     r4,#005H
                lcall   mul16           ;acc0 = acc0 x acc1
                mov     r4,#16          ;16 plaatsen naar rechts schuiven
                lcall   shiftright32    ;om te delen door 2^16
                pop     acc
                mov     r5,a
                pop     acc
                mov     r4,a
                pop     psw
                pop     acc
                ret 

;*******************************************************************************
; mul16_10        (98,7us @16.777216MHz)
; acc0 = acc0 x 10
; Vermenigvuldigt acc0 low (r1,r0) met 10        
; Inputbereik getallen: 0000h..ffffh (0 tot 65535d) 
;
; input:     r1,r0 = acc0 (low word)
;                
; output:    r3,r2,r1,r0 = acc0 = acc0 x 10   (r3=00h)        
;                                 0 tot  655350 (000000h..09fff6h)      
; vernietigt:  niets
;*******************************************************************************
mul16_10:       push    acc             ;registers bewaren
                push    psw
                mov     a,r4
                push    acc
                mov     r4,#1           ;acc0 x2
                lcall   shiftleft32     ;acc0 = acc0 x 2
                mov     a,r3            ;bewaar tussenresultaat op stack
                push    acc
                mov     a,r2
                push    acc
                mov     a,r1
                push    acc
                mov     a,r0
                push    acc             ;acc0 op stack
                mov     r4,#2           ;acc0 x4, in totaal dus x8!
                lcall   shiftleft32     ;acc0 =acc0 x4
                pop     acc             ;haal tussenresultaat van stack en
                add     a,r0            ;tel bij huidige acc0
                mov     r0,a
                pop     acc
                addc    a,r1
                mov     r1,a
                pop     acc
                addc    a,r2
                mov     r2,a
                pop     acc
                addc    a,r3
                mov     r3,a            ;acc0 =acc0x2 + acc0x8 = acc0x10
                
                
                pop     acc
                mov     r4,a
                pop     psw
                pop     acc
                ret 
                
;*******************************************************************************
; mul16_100        (169,1us @16.777216MHz)
; acc0 = acc0 x 100
; Vermenigvuldigt acc0 low (r1,r0) met 100        
; Inputbereik getallen: 0000h..ffffh (0 tot 65535d) 
;
; input:     r1,r0 = acc0 (low word)
;                
; output:    r3,r2,r1,r0 = acc0 = acc0 x 100  (r3=00h)         
;                                 0 tot  6553500 (000000h..63ff9ch)     
; vernietigt:  niets
;*******************************************************************************
mul16_100:      push    acc             ;registers bewaren
                push    psw
                mov     a,r4
                push    acc
                mov     r4,#2           ;acc0 x4
                lcall   shiftleft32     ;acc0 = acc0 x 4
                mov     a,r3            ;bewaar tussenresultaat op stack
                push    acc
                mov     a,r2
                push    acc
                mov     a,r1
                push    acc
                mov     a,r0
                push    acc             ;acc0 op stack
                
                mov     r4,#3           ;acc0 x8, in totaal dus x32!
                lcall   shiftleft32     ;acc0 =acc0 x8
                mov     a,r3            ;bewaar tussenresultaat op stack
                push    acc
                mov     a,r2
                push    acc
                mov     a,r1
                push    acc
                mov     a,r0
                push    acc             ;acc0 op stack
                
                mov     r4,#1           ;acc0 x2, in totaal dus x64!
                lcall   shiftleft32     ;acc0 =acc0 x2
                pop     acc             ;haal tussenresultaat van stack en
                add     a,r0            ;tel bij huidige acc0
                mov     r0,a
                pop     acc
                addc    a,r1
                mov     r1,a
                pop     acc
                addc    a,r2
                mov     r2,a
                pop     acc
                addc    a,r3
                mov     r3,a            ;acc0 =acc0x64 + acc0x32 = acc0x96
                
                pop     acc             ;haal tussenresultaat van stack en
                add     a,r0            ;tel bij huidige acc0
                mov     r0,a
                pop     acc
                addc    a,r1
                mov     r1,a
                pop     acc
                addc    a,r2
                mov     r2,a
                pop     acc
                addc    a,r3
                mov     r3,a            ;acc0 =acc0x96+ acc0x4 = acc0x100
                
                pop     acc
                mov     r4,a
                pop     psw
                pop     acc
                ret 

;*******************************************************************************
; sqrt32        (2,99ms @16.777216MHz)
; acc0 = acc0 ^(1/2)    
; Deze routine neemt de vierkantswortel van de absolute 32 bit waarde van acc0.        
; Inputbereik getal in acc0:  00000000h..ffffffffh (0 tot  +4294967295d) 
;
; input:     r3,r2,r1,r0 = acc0 
;                
; output:    r3,r2,r1,r0 = acc0^(1/2)     r3=r2=00h      
;                                      
; vernietigt:  niets
;******************************************************************************* 
sqrt32:         push    acc              ;bewaren van de gebruikte registers
                push    b
                push    psw
                push    dpl
                push    dph
                mov     a,r4
                push    acc
                mov     a,r5
                push    acc
                mov     a,r6
                push    acc
                mov     a,r7
                push    acc
                
                mov     r4,#0           ;acc1 op nul zetten
                mov     r5,#0
                mov     r6,#0
                mov     r7,#0
                mov     dpl,#0          ;wortel op nul zetten
                mov     dph,#0
                mov     b,#16           ;lusteller op 16 zetten
;hier start het algorithme voor het berekenen van de wortel                
sqrt320:        push    b               ;lusteller wortel even bewaren
                mov     b,#2            ;we schuiven acc0 2 bitposities naar
sqrt321:        clr     c               ;links, de twee msb's komen zo 
                mov     a,r0            ;stelselmatig in acc1 terecht waar ze
                rlc     a                ;getest worden om een wortelbit te
                mov     r0,a            ;berekenen, acc0 geraakt stilaan vol
                mov     a,r1            ;met nullen!
                rlc     a
                mov     r1,a
                mov     a,r2
                rlc     a
                mov     r2,a
                mov     a,r3
                rlc     a
                mov     r3,a            ;acco is nu een bitpositie opgeschoven
                
                mov     a,r4            ;msb van acco in lsb van acc1 schuiven
                rlc     a
                mov     r4,a
                mov     a,r5
                rlc     a
                mov     r5,a
                mov     a,r6
                rlc     a
                mov     r6,a
                mov     a,r7
                rlc     a
                mov     r7,a            ;acc1 is nu ook 1 positie opgeschoven
                djnz    b,sqrt321       ;doe dit nog eens: 2 bits opschuiven!
                
                pop     b               ;lusteller wortel weer terugnemen
                
                setb    c               ;we schuiven nu een 1 binnen op de lsb
                mov     a,dpl           ;van de voorlopige wortel
                rlc     a
                mov     dpl,a
                mov     a,dph
                rlc     a
                mov     dph,a           ;wortel is 1 bitpositie opgeschoven
                
;kwadraat van de voorlopige wortel berekenen om te vergelijken met acc1
                mov     a,r0            ;de inhoud van acc0 en acc1 even 
                push    acc             ;bewaren voor het berekenen van het
                mov     a,r1            ;kwadraat van de voorlopige wortel
                push    acc             ;in acc1 en voor het vergelijken van
                mov     a,r2            ;dit kwadraat met de huidige inhoud 
                push    acc             ;van acc1
                mov     a,r3
                push    acc
                mov     a,r4
                push    acc
                mov     a,r5
                push    acc
                mov     a,r6
                push    acc
                mov     a,r7
                push    acc             ;acc0 en acc1 zijn opgeslagen
                
                mov     r4,dpl          ;voorlopige wortel in acc1 low
                mov     r5,dph
                mov     r6,dpl          ;voorlopige wortel in acc1 high
                mov     r7,dph
                lcall    mul16acc1      ;maak kwadraat in acc1 (r7,r6,r5,r4)
                
                pop     acc             ;haal huidige acc1 weer op en stop even
                mov     r3,a            ;in acc0 om te kunnen vergelijken met
                pop     acc             ;het kwadraat van de voorlopige
                mov     r2,a            ;wortel die nu in acc1 zit.
                pop     acc             ;hierbij vermindert de stackpointer met
                mov     r1,a            ;vier plaatsen en wijst hij naar acc0 op
                pop     acc             ;de stack...
                mov     r0,a            ;huidige acc1 zit nu volledig in acc0
                
                clr     c               ;we gaan nu vergelijken of de huidige
                mov     a,r0            ;acc1 groter of gelijk is aan het 
                subb    a,r4            ;kwadraat van de voorlopige wortel
                mov     a,r1            ;in acc1.
                subb    a,r5            ;We doen dit door de carry van het
                mov     a,r2            ;verschil acc0-acc1 te evalueren.
                subb    a,r6
                mov     a,r3
                subb    a,r7            ;als acc1>= kwadraat, dan: geen carry
                jnc     sqrt323         ;en de laatst ingeschoven 1 van de
                                        ;wortel behouden
sqrt322:        mov     a,dpl          ;We verwerpen de laatst ingeschoven 1 in
                anl     a,#0feh         ;de wortel! kwadraat is te groot!
                mov     dpl,a          ;wortel aanpassen
                
sqrt323:        mov     a,sp           ;verzet de stackpointer weer naar boven
                add     a,#4           ;zodat de huidige acc1 die nog op stack
                mov     sp,a           ;opnieuw kan worden opgehaald
                
                pop     acc            ;haal acc1 weer op
                mov     r7,a
                pop     acc
                mov     r6,a
                pop     acc
                mov     r5,a
                pop     acc
                mov     r4,a           ;huidige acc1 weer hersteld
                
                pop     acc            ;nu nog acc0 van stack halen
                mov     r3,a
                pop     acc
                mov     r2,a
                pop     acc
                mov     r1,a
                pop     acc
                mov     r0,a           ;acc0 is helemaal hersteld
                
sqrt3240:       djnz    b,sqrt324      ;blijf herhalen voor alle bit duo's
                sjmp    sqrt325
sqrt324:        ljmp    sqrt320        ;uit acc0
                
sqrt325:        mov     r1,dph         ;voorlopige wortel is de vierkants-
                mov     r0,dpl         ;wortel van acc0!
                
                pop     acc            ;herstel de gebruikte registers
                mov     r7,a
                pop     acc
                mov     r6,a
                pop     acc
                mov     r5,a
                pop     acc
                mov     r4,a           ;acc1 is hersteld
                pop     dph
                pop     dpl
                pop     psw
                pop     b
                pop     acc
                
                ret                    ;eindelijk klaar...

;*******************************************************************************
; s_sqrt32        (2,99ms @16.777216MHz)
; acc0 = acc0 ^(1/2)    
; Deze routine neemt de vierkantswortel van de signed 32 bit waarde van acc0.        
; Inputbereik getal in acc0: 80000000h..ffffffffh,0,00000001h..7fffffffh
;                           (-2147483648 tot +2147483647) 
; input:     r3,r2,r1,r0 = acc0 
;                
; output:    r3,r2,r1,r0 = acc0^(1/2)     r3=r2=00h      
;            CY = 1 als acc0 negatief is                       
; vernietigt:  niets
;******************************************************************************* 
s_sqrt32:       push    acc
                push    psw
                mov     a,r3
                jnb     acc.7,s_sqrt321  ;als positief dan verder werken...
                pop     psw
                setb    c                ;als negatief dan error
                pop     acc
                ret
s_sqrt321:      pop     psw
                pop     acc
                lcall   sqrt32            ;wortel trekken
                clr     c                 ;wortel ok
                ret                       ;klaar
 
;*******************************************************************************
; shiftright32        (359,1us met N=32, 25,94us met N=1  @16.777216MHz)
; Schuift de 32 bit inhoud van acc0 N plaatsen naar rechts.
; (logical shift)             
; Inputbereik getallen: 00000000h..ffffffffh (0 tot  +4294967296d) 
;
; input:     r3,r2,r1,r0 = acc0 
;                     r4 = N  (te schuiven aantal posities  0<N<32) 
;                
; output:    r3,r2,r1,r0 = acc0 = acc0 >>N           
;            CY=1 als N>=32                          
; vernietigt:  niets
;******************************************************************************* 
shiftright32:   push    acc             ;bewaren van de registers
                push    psw
                mov     a,r4            ;neem aantal shifts
                cjne    a,#33,shiftright1 ;test waarde van N
shiftright1:    jc      shiftright2
                pop     psw             ;als 32 of groter dan buiten bereik!
                setb    c               ;N buiten bereik
                pop     acc
                ret
                
shiftright2:    push    b               ;ok, N is kleiner dan 32
                mov     b,r4            ;lusteller initialiseren
shiftright3:    clr     c               ;nullen binnenschuiven in msb
                mov     a,r3            ;start bij msb
                rrc     a
                mov     r3,a
                mov     a,r2
                rrc     a
                mov     r2,a
                mov     a,r1
                rrc     a
                mov     r1,a
                mov     a,r0
                rrc     a
                mov     r0,a            ;32bits zijn 1 plaats verschoven
                djnz    b,shiftright3   ;herhaal N maal
                pop     b               ;registers herstellen
                pop     psw                
                clr     c               ;N is binnen bereik
                pop     acc
                ret 
 
;*******************************************************************************
; shiftleft32        (359,1us met N=32, 25,94us met N=1  @16.777216MHz)
; Schuift de 32 bit inhoud van acc0 N plaatsen naar links.
; (logical shift)             
; Inputbereik getallen: 00000000h..ffffffffh (0 tot  +4294967296d) 
;
; input:     r3,r2,r1,r0 = acc0 
;                     r4 = N  (te schuiven aantal posities  0<N<32) 
;                
; output:    r3,r2,r1,r0 = acc0 = acc0 <<N           
;                                      
; vernietigt:  niets
;*******************************************************************************
shiftleft32:    push    acc             ;bewaren van de registers
                push    psw
                mov     a,r4            ;neem aantal shifts
                cjne    a,#33,shiftleft1;test waarde van N
shiftleft1:     jc      shiftleft2
                pop     psw             ;als 32 of groter dan buiten bereik!
                setb    c               ;N buiten bereik
                pop     acc
                ret
                
shiftleft2:     push    b               ;ok, N is kleiner dan 32
                mov     b,r4            ;lusteller initialiseren
shiftleft3:     clr     c                ;nullen binnenschuiven in lsb
                mov     a,r0            ;start bij lsb
                rlc     a
                mov     r0,a
                mov     a,r1
                rlc     a
                mov     r1,a
                mov     a,r2
                rlc     a
                mov     r2,a
                mov     a,r3
                rlc     a
                mov     r3,a            ;32bits zijn 1 plaats verschoven
                djnz    b,shiftleft3    ;herhaal N maal
                pop     b               ;registers herstellen
                pop     psw
                clr     c               ;N is binnen bereik
                pop     acc
                ret
 
;*******************************************************************************
; s_shiftright32        (353,1us met N=31, 30,34us met N=1  @16.777216MHz)
; (arithmetic shift)
; Schuift de signed (2's compl.) 32 bit inhoud van acc0 N plaatsen naar rechts.
; De tekenbit wordt als bit binnengeschoven langs links, het getal verandert dus
; niet van teken!             
; Inputbereik getallen: 80000000h..ffffffffh,0,00000001h..7fffffffh
;                       (-2147483648 tot +2147483647)
;
; input:     r3,r2,r1,r0 = acc0 
;                     r4 = N  (te schuiven aantal posities  0<N<31) 
;                
; output:    r3,r2,r1,r0 = acc0 = acc0 >>N           
;            CY=1 als N>=31                          
; vernietigt:  niets
;******************************************************************************* 
s_shiftright32: push    acc             ;bewaren van de registers
                push    psw
                mov     a,r4            ;neem aantal shifts
                cjne    a,#32,s_shiftright1 ;test waarde van N
s_shiftright1:  jc      s_shiftright2
                pop     psw             ;als 32 of groter dan buiten bereik!
                setb    c               ;N buiten bereik
                pop     acc
                ret
                
s_shiftright2:  push    b               ;ok, N is kleiner dan 32
                mov     b,r4            ;lusteller initialiseren
                mov     a,r3
                jb      acc.7,s_shiftright20
                clr     f0              ;geef aan dat getal positief is
                sjmp    s_shiftright3
s_shiftright20: setb    f0              ;geef aan dat getal negatief is    
                
s_shiftright3:  mov     c,f0            ;teken binnenschuiven in msb
                mov     a,r3            ;start bij msb
                rrc     a
                mov     r3,a
                mov     a,r2
                rrc     a
                mov     r2,a
                mov     a,r1
                rrc     a
                mov     r1,a
                mov     a,r0
                rrc     a
                mov     r0,a            ;32bits zijn 1 plaats verschoven
                djnz    b,s_shiftright3 ;herhaal N maal
                pop     b               ;registers herstellen
                pop     psw
                clr     c               ;N is binnen bereik
                pop     acc
                ret 
 
;*******************************************************************************
; s_shiftleft32        (393,1us met N=31, 27,2us met N=1  @16.777216MHz)
; (arithmetic shift)
; Schuift de signed (2's compl.) 32 bit inhoud van acc0 N plaatsen naar links.
; De tekenbit wordt als bit behouden op de msb, het getal verandert dus
; niet van teken! Er worden langs rechts nullen binnengeschoven.            
; Inputbereik getallen: 80000000h..ffffffffh,0,00000001h..7fffffffh
;                       (-2147483648 tot +2147483647)
;
; input:     r3,r2,r1,r0 = acc0 
;                     r4 = N  (te schuiven aantal posities  0<N<31) 
;                
; output:    r3,r2,r1,r0 = acc0 = acc0 >>N           
;            CY=1 als N>=31                          
; vernietigt:  niets
;******************************************************************************* 
s_shiftleft32:  push    acc             ;bewaren van de registers
                push    psw
                mov     a,r4            ;neem aantal shifts
                cjne    a,#32,s_shiftleft1;test waarde van N
s_shiftleft1:   jc      s_shiftleft2
                pop     psw             ;als 32 of groter dan buiten bereik!
                setb    c               ;N buiten bereik
                pop     acc
                ret
                
s_shiftleft2:   push    b               ;ok, N is kleiner dan 32
                mov     b,r4            ;lusteller initialiseren
                
                
                
s_shiftleft3:   clr     c                ;nullen binnenschuiven in lsb
                mov     a,r0             ;start bij lsb
                rlc     a
                mov     r0,a
                mov     a,r1
                rlc     a
                mov     r1,a
                mov     a,r2
                rlc     a
                mov     r2,a
                mov     a,r3
                rlc     a
                mov     acc.7,c          ;tekenbit behouden!
                mov     r3,a             ;32bits zijn 1 plaats verschoven
                djnz    b,s_shiftleft3    ;herhaal N maal
                pop     b                ;registers herstellen
                pop     psw
                clr     c                ;N is binnen bereik
                pop     acc
                ret   
 
;*******************************************************************************
; s_cmp16    (29,5us @16.777216MHz)
; Vergelijkt de 2's complement inhoud van acc0 (low) en acc1 (low).
;              
; Inputbereik getallen: 8000h..ffffh,0,0001h..7fffh (-32768d,0,+32767d) 
;
; input:     r1,r0 = acc0 
;            r5,r4 = acc1         
;                
; output:    f0=0,cy=1  als acc0 < acc1
;             f0=0,cy=0  als acc0 > acc1
;             f0=1 cy=0    als acc1 = acc0             
;                                      
; vernietigt:  niets
;*******************************************************************************
s_cmp16:        push    acc             ;registers bewaren op stack
                push    psw
                mov     a,r0
                push    acc
                mov     a,r1
                push    acc
                
                mov     a,r1            ;neem msb van acc0
                xrl     a,r5            ;check gelijk aan tekenbit van acc1
                jnb     acc.7,s_cmp162  ;bij gelijk teken, verschil maken
                mov     a,r1            ;bij verschillend teken kijken wie er
                                        ;negatief is
                jb      acc.7,s_cmp161  ;als r1.7=1, dan is acc0 negatief
                                        ;en dus acc1 positief. Dus acc0<acc1
  
;als r1.7=0, dan is acc0 positief en dus acc1 negatief. Dus acc0>acc1                                        
;acc0>acc1
s_cmp160:       pop     acc
                mov     r1,a
                pop     acc
                mov     r0,a
                pop     psw             ;cy=0, dus acc0 is positief en dus
                clr     f0              ;groter dan acc1 die negatief is.
                clr     c               ;geef dit aan in de outputstatus
                pop     acc
                ret
;acc0<acc1
s_cmp161:       pop     acc
                mov     r1,a
                pop     acc
                mov     r0,a
                pop     psw             ;cy=1, acc0 is negatief en acc1 is dus
                setb    c               ;groter dan acc0
                clr     f0              ;geef dit aan in de outputstatus
                pop     acc
                ret
;gelijk teken, maak verschil
s_cmp162:       clr     c               ;teken is gelijk maak verschil 
                mov     a,r0            ;acc0-acc1
                subb    a,r4        
                mov     r0,a
                mov     a,r1
                subb    a,r5
                mov     r1,a
                jb      acc.7,s_cmp161  ;als r1.7=1 (underflow) acc0<acc1
                orl     a,r0            ;kijk of acc0>acc1 of acc0=acc1
                jz      s_cmp163        ;acc0=acc1
                sjmp    s_cmp160        ;acc0>acc1
;acc0=acc1                
s_cmp163:       pop     acc
                mov     r1,a
                pop     acc
                mov     r0,a
                pop     psw             ;acc0=acc1
                clr     c               ;geef dit aan in de outputstatus
                setb    f0
                pop     acc
                ret
                
;*******************************************************************************
; cmp16            (19,9us @16.777216MHz)
; Vergelijkt de unsigned inhoud van acc0 (low) en acc1 (low).
;              
; Inputbereikt getallen: 0000h..ffffh (0,+65535d) 
;
; input:     r1,r0 = acc0 
;            r5,r4 = acc1         
;                
; output:    f0=0,cy=1  als acc0 < acc1
;             f0=0,cy=0  als acc0 > acc1
;             f0=1 cy=0    als acc1 = acc0             
;                                      
; vernietigt:  niets
;*******************************************************************************                
cmp16:          push    acc             ;bewaar registers
                push    b
                push    psw
                
                clr     c               ;vergelijk acc0 met acc1
                mov     a,r1            ;eerst de msb's
                mov     b,r5            ;test r1-r5
                cjne    a,b,cmp161      ;niet gelijk, kijk wie grootst is
                
                mov     a,r0            ;msb's zijn gelijk, vergelijk lsb's
                mov     b,r4            ;test r0-r4
                cjne    a,b,cmp161      ;niet gelijk, kijk wie grootst is
;acc0=acc1
                pop     psw
                setb    f0
                clr     c
                pop     b
                pop     acc
                ret
cmp161:         jc      cmp162          ;cy=1 indien acc0<acc1
;acc0>acc1
                pop     psw
                clr     f0
                clr     c
                pop     b
                pop     acc
                ret
;acc0<acc1
cmp162:         pop     psw
                setb    c
                clr     f0
                pop     b
                pop     acc
                ret
                
;*******************************************************************************
; s_cmp32        (44,3us @16.777216MHz)
; Vergelijkt de 2's complement inhoud van acc0  en acc1.
;              
; Inputbereik getallen: 80000000h..ffffffffh,0,00000001h..7fffffffh 
;                         -2147483648 tot +2147483647 
; input:     r1,r0 = acc0 
;            r5,r4 = acc1         
;                
; output:    f0=0,cy=1  als acc0 < acc1
;             f0=0,cy=0  als acc0 > acc1
;             f0=1 cy=0    als acc1 = acc0             
;                                      
; vernietigt:  niets
;*******************************************************************************
s_cmp32:        push    acc             ;registers bewaren op stack
                push    psw
                mov     a,r0
                push    acc
                mov     a,r1
                push    acc
                mov     a,r2
                push    acc
                mov     a,r3
                push    acc
                
                mov     a,r3            ;neem msb van acc0
                xrl     a,r7            ;check gelijk aan tekenbit van acc1
                mov     c,acc.7         ;cy=0 bij gelijk teken
                jnc     s_cmp322        ;bij gelijk teken, verschil maken
                mov     a,r3            ;bij verschillend teken kijken wie er
                mov     c,acc.7         ;negatief is
                jc      s_cmp321        ;als cy=1 dan is acc0 negatief en dus 
                                        ;acc1 positief. Dus acc1>acc0
;acc0>acc1
s_cmp320:       pop     acc
                mov     r3,a
                pop     acc
                mov     r2,a
                pop     acc
                mov     r1,a
                pop     acc
                mov     r0,a
                pop     psw             ;cy=0, dus acc0 is positief en dus
                clr     f0              ;groter dan acc1 die negatief is.
                clr     c               ;geef dit aan in de outputstatus
                pop     acc
                ret
;acc0<acc1
s_cmp321:       pop     acc
                mov     r3,a
                pop     acc
                mov     r2,a
                pop     acc
                mov     r1,a
                pop     acc
                mov     r0,a
                pop     psw             ;cy=1, acc0 is negatief en acc1 is dus
                setb    c               ;groter dan acc0
                clr     f0              ;geef dit aan in de outputstatus
                pop     acc
                ret
;gelijk teken, maak verschil
s_cmp322:       clr     c               ;teken is gelijk maak verschil 
                mov     a,r0            ;acc0-acc1
                subb    a,r4        
                mov     r0,a
                mov     a,r1
                subb    a,r5
                mov     r1,a
                mov     a,r2
                subb    a,r6
                mov     r2,a
                mov     a,r3
                subb    a,r7
                mov     r3,a
                jb      acc.7,s_cmp321  ;neem hoogste bit (underflow)acc0<acc1
                orl     a,r2            ;geen cy, acc0>=acc1
                orl     a,r1
                orl     a,r0            ;kijk of acc0>acc1 of acc0=acc1
                jz      s_cmp323        ;acc0=acc1
                sjmp    s_cmp320        ;acc0>acc1
;acc0=acc1                
s_cmp323:       pop     acc
                mov     r3,a
                pop     acc
                mov     r2,a
                pop     acc
                mov     r1,a
                pop     acc
                mov     r0,a
                pop     psw             ;acc0=acc1
                clr     c               ;geef dit aan in de outputstatus
                setb    f0
                pop     acc
                ret
                
;*******************************************************************************
; cmp32        (27,14us @16.777216MHz)
; Vergelijkt de unsigned inhoud van acc0 en acc1.
;              
; Inputbereik getallen: 00000000h..ffffffffh (0 tot +4294967296d) 
;
; input:     r3,r2,r1,r0 = acc0 
;            r7,r6,r5,r4 = acc1         
;                
; output:    f0=0,cy=1  als acc0 < acc1
;            f0=0,cy=0  als acc0 > acc1
;            f0=1 cy=0    als acc1 = acc0             
;                                      
; vernietigt:  niets
;*******************************************************************************                
cmp32:          push    acc             ;bewaar registers
                push    b
                push    psw
                
                clr     c               ;vergelijk acc0 met acc1
                mov     a,r3            ;eerst de msb's, dan volgende bytes
                mov     b,r7            ;test r3-r7
                cjne    a,b,cmp321      ;niet gelijk, kijk wie grootst is
                
                mov     a,r2            ;msb's zijn gelijk, vergelijk volgende
                mov     b,r6            ;bytes, test r2-r6
                cjne    a,b,cmp321      ;niet gelijk, kijk wie grootst is
                
                mov     a,r1            ;msb's zijn gelijk, vergelijk volgende
                mov     b,r5            ;bytes,  test r1-r5
                cjne    a,b,cmp321      ;niet gelijk, kijk wie grootst is
                
                mov     a,r0            ;msb's zijn gelijk, vergelijk volgende
                mov     b,r4            ;bytes,  test r0-r4
                cjne    a,b,cmp321      ;niet gelijk, kijk wie grootst is
;acc0=acc1
                pop     psw
                setb    f0
                clr     c
                pop     b
                pop     acc
                ret
cmp321:         jc      cmp322          ;cy=1 indien acc0<acc1
;acc0>acc1, indien cy=0
                pop     psw
                clr     f0
                clr     c
                pop     b
                pop     acc
                ret
;acc0<acc1
cmp322:         pop     psw
                setb    c
                clr     f0
                pop     b
                pop     acc
                ret
                
                
;*******************************************************************************
; table_lu        (42,2us @16.777216MHz worst case)
; Tabel lookup routine voor tabel met byte (8bit) of word (16bit) items.
; Indien word items, dan moeten deze steeds met de lsbyte op het laagste
; adres zitten.
;              
; Inputbereik getallen:0000h..ffffh (0 tot +65535d) 
;
; input:     dptr  =  startadres van de tabel (0000h..ffffh)
;            r1,r0 = acc0 low = index van het gewenste item (0000h..ffffh)
;      flags f1,f0 = 00 = tabel in codegeheugen, items op 8bit basis
;                    01 = tabel in codegeheugen, items op 16bit basis
;                    10 = tabel in datageheugen, items op 8bit basis
;                    11 = tabel in datageheugen, items op 16bit basis
; output:    r5,r4 = gewenste 8bit (r4)  of 16bit (r5,r4) waarde uit de tabel
;                cy = 0  lookup is ok
;                cy = 1  startadres+index groter dan 16bits (overflow)=error!
; vernietigt:  niets
;*******************************************************************************                
table_lu:       push    acc             ;bewaren registers
                push    psw
                push    dpl
                push    dph
                
                mov     c,f0            ;byte of word items?
                jc     table_lu2        ;als cy=1, dan tabel met word items
;tabel met byte items
                mov     a,dpl           ;datapointer plus index
                add     a,r0
                mov     dpl,a
                mov     a,dph
                addc    a,r1
                mov     dph,a
                jc      table_lu_10     ;indien carry dan overflow!
                mov     c,f1            ;code of datageheugen?
                jc      table_lu1       ;als cy=1, dan datageheugen

;bytetabel in codegeheugen
                mov     a,#0
                movc    a,@a+dptr       ;haal byte op uit codegeheugen
                mov     r0,a            ;en stop in acc0
                mov     r1,#0
                ljmp    table_lu4       ;en klaar
                
;bytetabel in datageheugen
table_lu1:      movx    a,@dptr         ;haal byte op uit datageheugen
                mov     r0,a            ;en stop in acc0
                mov     r1,#0
                ljmp    table_lu4       ;en klaar
                
table_lu_10:    pop     dph            ;registers herstellen met errorstatus
                pop     dpl
                pop     psw
                setb    c              ;foutstatus teruggeven!
                pop     acc
                ret
                
;tabel met word items
table_lu2:      mov     a,r0            ;index op stack
                push    acc
                mov     a,r1
                push    acc
                
                clr      c              ;index maal 2 (word items)
                mov     a,r0
                rlc     a
                mov     r0,a
                mov     a,r1
                rlc     a
                mov     r1,a
                
                mov     a,dpl           ;datapointer plus index
                add     a,r0
                mov     dpl,a
                mov     a,dph
                addc    a,r1
                mov     dph,a
                
                pop     acc             ;index weer van stack
                mov     r1,a
                pop     acc
                mov     r0,a
                
                jc      table_lu_10     ;indien carry dan overflow!
                mov     c,f1            ;code of datageheugen?
                jc      table_lu3       ;als cy=1, dan datageheugen
;wordtabel in codegeheugen
                mov     a,#0
                movc    a,@a+dptr       ;haal lsb op uit codegeheugen
                mov     r0,a            ;lsb in acc0 stoppen
                mov     a,dpl           ;datapointer incrementeren voor msb
                add     a,#1
                mov     dpl,a
                mov     a,dph
                addc    a,#0
                mov     dph,a
                mov     a,#0
                movc    a,@a+dptr        ;haal msb op uit codegeheugen
                mov     r1,a            ;msb in acc0 stoppen
                sjmp    table_lu4        ;en klaar
;wordtabel    in datageheugen
table_lu3:      movx    a,@dptr         ;haal lsb op uit codegeheugen
                mov     r0,a            ;lsb in acc0 stoppen
                mov     a,dpl           ;datapointer incrementeren voor msb
                add     a,#1
                mov     dpl,a
                mov     a,dph
                addc    a,#0
                mov     dph,a
                mov     a,#0
                movx    a,@dptr         ;haal msb op uit codegeheugen
                mov     r1,a            ;msb in acc0 stoppen
                                        ;en klaar        
                
table_lu4:      pop     dph             ;registers herstellen zonder error
                pop     dpl
                pop     psw
                pop     acc
                ret
                
                
        
;*******************************************************************************
; Cordic        (2,11ms @16.777216MHz)
; Deze routine berekent de sinus en de cosinus van de in acc0 low als signed
; waarde (2's complement) opgegeven hoek. 
; De hoek (in rad.) moet vermenigvuldigd worden met een schaalfactor 16384.
; Het inputhoekbereik gaat van -pi/2 tot pi/2.
; Dwz. een hoek van pi/2 wordt gegeven als pi/2 * 16384 =25735 (6487h) 
;      een hoek van -pi/2 wordt gegeven als -pi/2*16384 =-25736 (9b78h)
; De outputparameters zijn de sinus en cosinus van de opgegeven hoek die ook 
; als geschaalde getallen ter beschikking komen.
; Het outputbereik is hier: -1 (c000h)  tot  +1 (3fffh).
;
; input:    r1,r0 = acc0(low)  = hoek (in rad.) geschaald met 16384          
;
; output:   r1,r0 = acc0(low)  = cosinus van de hoek * 16384
;             r3,r2 = acc0(high) = sinus van de hoek * 16384
;            cy = 0 : hoek binnen bereik
;            cy = 1 : error, hoek buiten bereik
;            nauwkeurigheid: 12 msbits
; vernietigt:  niets
;*******************************************************************************                
cordic:         push    acc                ;bewaar de gebruikte registers
                push    psw
                push    dph
                push    dpl
                push    b
                
                mov     a,r7
                push    acc
                mov     a,r6
                push    acc
                mov     a,r5
                push    acc
                mov     a,r4
                push    acc
                
;Check of de doorgegeven hoek niet kleiner is dan -pi/2 radialen (9b78h) 
;en niet groter dan pi/2 radialen (6478h) 
                mov     r4,#87h         ;plaats 6487h (+pi/2) in acc1
                mov     r5,#64h
                lcall   cmp16           ;vergelijk de hoeken (acc0-acc1)
                jb      f0,cordic_plus90;gelijk aan +pi/2, resultaat gekend!
                jnc     cordic000       ;bij cy=0 is acc0>acc1, dus mogelijk fout
                sjmp    cordic00        ;hoek ok, bereken de sinus en cosinus...
                
cordic000:      clr     p2.0
                mov     r4,#78h         ;plaats 9b78h in acc1
                mov     r5,#9bh
                lcall   cmp16           ;vergelijk de hoeken (acc0-acc1)
                jb      f0,cordic_min90 ;bij gelijk aan -pi/2, resultaat gekend!
                jc      cordic_error    ;bij cy=1 is acc0<acc1, dus fout
                sjmp    cordic00        ;hoek ok, bereken de sinus en cosinus...
                
cordic_error:   clr     p2.1
                ljmp    cordic40        ;bij fout, terug met foutstatus!
                
;Hoek is -pi/2 resultaat gekend
cordic_min90:   mov     r3,#0c0h        ;sinus is c000h = -1
                mov     r2,#000h
                mov     r1,#000h        ;cosinus is 0000h = 0
                mov     r0,#000h
                clr     c
                ljmp    cordic40        ;en klaar
;hoek is +pi/2 resultaat gekend
cordic_plus90:  mov     r3,#03fh        ;sinus is 3fffh = +1
                mov     r2,#0ffh    
                mov     r1,#000h        ;cosinus is 0000 = 0
                mov     r0,#000h
                clr     c
                ljmp    cordic40        ;en klaar

cordic00:       mov     r4,#00h         ;plaats 0000h in acc1
                mov     r5,#00h
                lcall   cmp16            ;vergelijk de hoeken
                jb     f0,cordic_nul    ;bij gelijk aan 0, resultaat gekend!
                sjmp   cordic0          ;bereken de sinus en cosinus...
;hoek is 0, resultaat gekend
cordic_nul:     clr     p2.7
                mov     r3,#000h        ;sinus is 0000h = 0
                mov     r2,#000h    
                mov     r1,#03fh        ;cosinus is 3fff = +1
                mov     r0,#0ffh
                clr     c
                ljmp    cordic40        ;en klaar
                
;De hoek zit binnen bereik, bepaal sin en cos.
cordic0:        mov     a,r0            ;bewaar de lsb hoek op stack
                push    acc
                mov     a,r1            ;bewaar de msb hoek op stack
                push    acc
                
                mov     r3,#00h         ;y=0
                mov     r2,#00h
                mov     r1,#026h        ;x = K * 2^14   
                mov     r0,#0ddh        ;K=0.607252935
                
                mov     b,#00h          ;loopteller=0
                
                mov     dptr,#cordic_atan    ;wijs naar de atan tabel
                
cordic1:        mov     a,r3            ;sry=y
                mov     r7,a
                mov     a,r2
                mov     r6,a
                mov     a,r1            ;srx=x
                mov     r5,a
                mov     a,r0
                mov     r4,a
                
                lcall   cordic5          ;srx en sry >> loopteller
                                
                pop     acc
                jb      acc.7,cordic3    ;als negatief, hoek vergroten

;De hoek is positief; nieuwe x en y berekenen en de hoek verkleinen
                clr     c
                mov     a,r0            ;bereken x=x-sry
                subb    a,r6            ;dit stelt de voorlopige cosinus voor
                mov     r0,a
                mov     a,r1
                subb    a,r7
                mov     r1,a
                
                mov     a,r2            ;bereken y=y+srx
                add     a,r4            ;dit stelt de voorlopige sinus voor
                mov     r2,a
                mov     a,r3
                addc    a,r5
                mov     r3,a
                
;De hoek verkleinen met de waarde uit de lookuptabel en weer bewaren
                mov    a,#0            ;haal lsb atan waarde uit de lookuptabel
                movc    a,@a+dptr        
                inc     dptr
                mov     r4,a            ;bewaar in srx die verloren mag gaan
                pop     acc             ;haal lsb van de hoek van stack
                clr     c               ;verklein de hoek
                subb    a,r4
                push    acc             ;en bewaar de lsb van de hoek op stack
                inc     sp              ;wijs weer naar msb van de hoek
                
                mov     a,#0            ;haal msb atan waarde uit de lookuptabel
                movc    a,@a+dptr
                inc     dptr            ;wijs naar volgende tabelwaarde
                mov     r5,a            ;bewaar in srx die verloren mag gaan
                pop     acc             ;haal msb van de hoek van stack
                subb    a,r5            ;verklein de hoek
                push    acc             ;en bewaar de msb van de hoek op stack
                
;loopteller incrementeren en testen op einde van de iteratie
cordic2:        inc     b               ;loopteller++
                mov     a,#14           ;iteratie 14 maal uitgevoerd?
                xrl     a,b             ;test op gelijkheid
                jz      cordic4         ;ja klaar!
                ljmp    cordic1         ;nee, nog even verder...
                
;De hoek is negatief; nieuwe x en y berekenen en de hoek vergroten
cordic3:        mov     a,r0            ;bereken x=x+sry
                add     a,r6            ;dit stelt de voorlopige cosinus voor
                mov     r0,a
                mov     a,r1
                addc    a,r7
                mov     r1,a
                
                clr     c
                mov     a,r2            ;bereken y=y-srx
                subb    a,r4            ;dit stelt de voorlopige sinus voor
                mov     r2,a
                mov     a,r3
                subb    a,r5
                mov     r3,a
;De hoek vergroten met de waarde uit de lookuptabel en weer bewaren
                mov     a,#0            ;haal lsb atan waarde uit de lookuptabel
                movc    a,@a+dptr
                inc     dptr
                mov     r4,a            ;bewaar in srx die verloren mag gaan
                pop     acc             ;haal lsb van de hoek van stack
                add     a,r4            ;vergroot de hoek
                push    acc             ;en bewaar de lsb van de hoek op stack
                inc     sp              ;wijs weer naar msb van de hoek
                
                mov     a,#0            ;haal msb atan waarde uit de lookuptabel
                movc    a,@a+dptr
                inc     dptr            ;wijs naar volgende tabelwaarde
                mov     r5,a            ;bewaar in srx die verloren mag gaan
                pop     acc             ;haal msb van de hoek van stack
                addc    a,r5            ;vergroot de hoek
                push    acc             ;en bewaar de msb van de hoek op stack
                
                sjmp    cordic2
                
cordic4:        pop     acc             ;dummy hoek van stack
                pop     acc             ;dummy hoek van stack
                clr     c               ;hoek is ok, geen foutstatus!
                
cordic40:       pop     acc
                mov     r4,a
                pop     acc
                mov     r5,a
                pop     acc
                mov     r6,a
                pop     acc
                mov     r7,a
                pop     b
                pop     dpl
                pop     dph
                jc      cordic41        ;foutstatus:hoek buiten bereik!!
                pop     psw
                pop     acc
                ret
                
cordic41:       pop     psw
                setb    c
                pop     acc
                ret
;Deze routine zal de 16 bit schuifregisters srx en sry verschuiven naar rechts
;over het aantal posities aangegeven in het B register.
cordic5:        push    b               ;loopteller bewaren
                mov     a,#0            ;test of loopteller=0
                xrl     a,b
                jz      cordic52        ;klaar
                
                mov     a,r5            ;test teken van srx
                jb      acc.7,cordic5a  ;als negatief: 1 binnenschuiven
                clr     f0              ;geef aan dat srx positief is
                sjmp    cordic50
cordic5a:       setb    f0
cordic50:       mov     c,f0            ;schuif srx>>1
                mov     a,r5            ;eerst de msb
                rrc     a
                mov     r5,a
                mov     a,r4            ;dan de lsb
                rrc     a
                mov     r4,a
                djnz    b,cordic50      ;herhaal tot loopteller nul is
                pop     b               ;haal de loopteller van stack
                push    b               ;en bewaar hem weer
                mov     a,r7            ;test teken van sry
                jb      acc.7,cordic5b  ;als negatief: 1 binnenschuiven
                clr     f0              ;geef aan dat sry positief is
                sjmp    cordic51
cordic5b:       setb    f0
cordic51:       mov     c,f0            ;schuif sry>>1
                mov     a,r7            ;eerst de msb
                rrc     a
                mov     r7,a
                mov     a,r6            ;dan de lsb
                rrc     a
                mov     r6,a
                djnz    b,cordic51      ;herhaal tot loopteller nul is
                
cordic52:       pop     b               ;haal de loopteller van stack                
                ret

;Lookuptabel met de hoekwaarden Z=atan(1/2^i) en dit voor i=0 tot 14
;Alle tabelwaarden zijn vermenigvuldigd met de schaalfactor 16384.
;De 16bit waarden zijn gestockeerd met de lsbyte op de laagste adreslocatie
cordic_atan:    dw    04332h                ;i=0     1/2^i=1
                dw    0ac1dh                ;i=1    1/2^i=1/2
                dw    0ad0fh                ;i=2    1/2^i=1/4
                dw    0f507h                ;i=3    1/2^i=1/8
                dw    0fe03h                ;i=4    1/2^i=1/16
                dw    0ff01h                ;i=5    1/2^i=1/32
                dw    0ff00h                ;i=6    1/2^i=1/64
                dw    07f00h                ;i=7    1/2^i=1/128
                dw    03f00h                ;i=8    1/2^i=1/256
                dw    01f00h                ;i=9    1/2^i=1/512
                dw    00f00h                ;i=10    1/2^i=1/1024
                dw    00700h                ;i=11    1/2^i=1/2048
                dw    00300h                ;i=12    1/2^i=1/4096
                dw    00100h                ;i=13    1/2^i=1/8192
                dw    00000h                ;i=14    1/2^i=1/16384
                              
endif        ;einde van aduc_math
;******************************************************************************

ifdef        aduc_adc    ;A/D conversie routines              
;*******************************************************************************
; ADuCADC_mide         
; Geschreven door [dp] op 5/1/2014
; Deze file bevat routines voor het bedienen van de ADConverter in de ADuC832.
; De routine  'adc_init' zal bij het gebruik van de ADC steeds eerst eenmalig
; moeten uitgevoerd worden. De routine zal de ADC aanschakelen en de referentie-
; bron instellen.
; De routine  'adc_single' zal de gebruiker in staat stellen een AD conversie te
; doen van een instelbaar kanaal, met een instelbare resolutie en dit in een 
; unsigned of signed (2's complement) uitgangsformaat.
;
;*******************************************************************************

;*******************************************************************************
; adc_init
; Deze routine initialiseert de ADC in een ADuC832 voor 
; -  4 track&hold clockperiodes
; -  clockdivider 4  (4us ADC conversietijd @16.777216MHz) 
; -  softwarematig starten van een ADconversie (via adc_single)
; De gebruiker kan via R0 een interne (2,5V) of externe referentie instellen.
; De externe referentie is op het MGM/[dp] ADuC bord meestal ingesteld op 5V.
;
; input: R0 =  0  : Interne referentiebron
;        R0 <> 0  : Externe referentiebron
;
; Deze routine vernietigt niets
;*******************************************************************************
adc_init:       push    acc                 ;registers op stack                
                push    psw
                mov     adccon1,#11011100b  ;init ADC met externe ref
                mov     a,r0                ;welke referentiebron?
                jnz     adc_init1           ;als niet nul,dan ok
                mov     adccon1,#10011100b  ;init ADC met interne ref
adc_init1:      pop     psw
                pop     acc
                ret
                
;*******************************************************************************
; adc_single
; Deze routine doet een AD conversie van een instelbaar kanaal, met een instel-
; bare resolutie van 8, 10 of 12 bits en dit met een unsigned of signed 
; (2's complement) formaat van de meetwaarde.
; _____________________________________________________________
; resolutie  |     unsigned      |           signed            |
; -----------|---0V------Vref----|---0V------Vref/2-----Vref---|
;            | R1  R0   R1  R0   | R1  R0    R1  R0    R1  R0  |
; 8 bit      | 00h 00h  00h ffh  | ffh 80h   00h 00h   00h 7fh |       
; 10 bit     | 00h 00h  03h ffh  | feh 00h   00h 00h   01h ffh |
; 12 bit     | 00h 00h  0fh ffh  | f8h 00h   00h 00h   07h ffh |
; _____________________________________________________________
;
; De gemeten waarde wordt doorgegeven aan de gebruiker via de registers R1,R0.
; Hierbij stelt R1 de MSB en R0 de LSB voor zoals aangegeven hierboven.
; De ingangsparameter wordt doorgegeven via R0.
;
; input:    
;             7    6    5    4    3    2    1    0
;           _______________________________________
;       R0 |  x |s/U |res1|res0|ch3 |ch2 |ch1 |ch0 |
;           --------------------------------------- 
;       R0 = kanaalnummer (ch3-0) : 0000b-0111b = kanaal 0 tot 7
;                                         1000b = temp.sensor
;       R0 = resolutie (res1,0)   : 00b =8 bits,   01b = 10 bits
;                                   10b = 12 bits, 11b = 12 bits
;       R0 = (un)signed (s/U)     : 0 = unsigned   1= signed (2's complement) 
;                                   
; output: R1,R0 = R1 MSbyte, R0 LSbyte
;         cy=0  bij conversie ok
;         cy=1  bij foutief kanaalnummer (geen conversie gedaan)
; 
; Deze routine vernietigt niets
;*******************************************************************************
adc_single:     push    acc                 ;registers op stack
                push    psw
                push    b
                mov     a,r0                ;neem inputparameter
                anl     a,#0fh              ;neem kanaalnummer
                cjne    a,#9h,adc_single1   ;controleer kanaalnummer
adc_single1:    jnc    adc_single21        ;als geen carry, dan fout!
                mov     adccon2,a           ;geef kanaalnummer aan ADC
                setb    sconv               ;start een meting
                mov     a,r0                ;neem parameter terug
                mov     c,acc.6
                mov     f0,c                ;bewaar format bit
                jb      sconv,$             ;wacht hier tot conversie klaar is
                mov     b,adcdatah          ;neem high nibble van resultaat
                jb      acc.4,adc_single3   ;10 bit resultaat gevraagd
                jb      acc.5,adc_single4   ;12 bit resultaat gevraagd
;*******************************************************************************                
;8 bit resultaat gevraagd
;*******************************************************************************
                mov     a,adcdatal           ;neem low byte van resultaat
                swap    a                    ;swap nibbles
                anl     a,#0fh               ;maskeer low nibble van resultaat
                xch     a,b                  ;verwissel met high nibble resultaat
                anl     a,#0fh               ;maskeer kanaalnummer
                swap    a
                orl     a,b                  ;voeg samen met low nibble
                jb      f0,adc_single2       ;check formaat
;8 bit unsigned formaat
                mov     r1,#00h              ;unsigned
                mov     r0,a
                sjmp    adc_single42
;8 bit signed formaat                
adc_single2:    jb      acc.7,adc_single20    ;test bereik >=80h
                add     a,#80h                ;negatief getal van maken
                mov     r0,a
                mov     r1,#0ffh
                sjmp    adc_single42
adc_single20:   clr     c
                subb    a,#80h                ;positief deel verkleinen
                mov     r0,a
                mov     r1,#00h
                sjmp    adc_single42
                
adc_single21:   ljmp    adc_single43
;*******************************************************************************                
;10 bit resultaat gevraagd
;*******************************************************************************
adc_single3:    mov    a,b            ;neem high nibble resultaat
                anl    a,#00000011b   ;twee onderste bits houden en
                rr     a              ;naar msbits roteren
                rr     a
                mov    b,a            ;even bewaren
                mov    a,adcdatal     ;neem low byte van resultaat
                rr     a              ;twee plaatsen opschuiven
                rr     a
                anl    a,#00111111b   ;zes onderste bits houden en
                orl    a,b            ;combineren met bits uit high nibble
                mov    b,a            ;low byte even bewaren
                mov    a,adcdatah     ;neem high nibble
                anl    a,#00001100b   ;twee msbits overhouden
                rr     a               ;en op hun plaats schuiven
                rr     a
                jb     f0,adc_single30 ;check formaat
;10 bit unsigned formaat
                mov    r1,a            ;unsigned
                mov    r0,b
                sjmp   adc_single42
;10 bit signed formaat 
adc_single30:   jb     acc.1,adc_single31  ;test bereik >= 200h
                add    a,#0feh             ;negatief getal van maken
                mov    r1,a
                mov    r0,b
                sjmp   adc_single42
adc_single31:   clr    c                    
                subb   a,#02h              ;positief deel verkleinen
                mov    r1,a
                mov    r0,b
                sjmp   adc_single42
                
;*******************************************************************************                
;12 bit resultaat gevraagd    
;*******************************************************************************            
adc_single4:    mov    a,b                   ;neem high nibble resultaat
                anl    a,#0fh                ;maskeer kanaalnummer
                jb     f0,adc_single40       ;check formaat
;12 bit unsigned formaat                
                mov    r0,adcdatal           ;unsigned
                mov    r1,a
                sjmp   adc_single42
;12 bit signed formaat
adc_single40:   jb     acc.3,adc_single41    ;test bereik >= 800h
                add    a,#0f8h               ;negatief getal van maken
                mov    r1,a
                mov    r0,adcdatal
                sjmp   adc_single42
adc_single41:   clr    c
                subb   a,#08h                ;positief deel verkleinen
                mov    r1,a
                mov    r0,adcdatal
adc_single42:   pop    b
                pop    psw
                clr    c
                pop    acc
                ret
                
adc_single43:   pop    b
                pop    psw
                setb   c
                pop    acc
                ret
                              
endif        ;einde van aduc_adc
;*******************************************************************************
              
ifdef    aduc_key    ;matrix keyboard scanner en lezen/debouncen schakelaars 
;******************************************************************************
; ADuCKEY_mide.inc
; Deze routines zorgen voor het scannen van een standaard 4x4 matrix-keyboardje
; dat op volgende manier is aangesloten op het aduc800 bord v1.1 of v2.0.
;
;     V2.0      V1.1
;                           |   |   |   |
;     P0.0      P0.0|-R1----1---2---3---A---
;                   |       |   |   |   |
;     P0.1      P0.1|-R2----4---5---6---B---
;                   |       |   |   |   |
;     P0.2      P3.2|-R3----7---8---9---C---
;                   |       |   |   |   |
;     P0.3      P3.3|-R4----*---0---#---D---
;                   |       |   |   |   | 
;     P0.4      P3.4|-K1----|   |   |   |
;                   |           |   |   |
;     P0.5      P3.5|-K2--------|   |   |
;                   |               |   |
;     P0.6      P3.6|-K3------------|   |
;                   |                   |
;     P0.7      P3.7|-K4----------------|
;                         
; Afhankelijk van de gedefinieerde versie zullen de juiste pinnen worden
; gebruikt.
; Het blokschema van het gevolgde algorithme is hieronder aangegeven.
; Het bestaat uit een debounce circuit gevolgd door een flankdetector.
; Het blokschema is hier afgebeeld voor één bit (kolomlijn) die gelezen wordt
; en als ingang wordt aangeboden.
;
;                                  ___                 ___
;                     ____________|   |  |------------|   |
;              ____  |   ____     |   |  |   ____     |   |
;             |    | |  |    |    |&  |__|__|    |    |&  |
;  KOLOM  ----|D  Q|_|__|D  Q|----|___|     |D  Q|---O|___|---- OUT
; '1'= toets  | cl |    | cl |              | cl |
;  ingedrukt  |____|    |____|              |____|
;                |         |                   |
;      CLOCK     |         |                   |
;         _______|_________|___________________|
;
;            DEBOUNCE CIRCUIT              POS. FLANKDETECTIE
;
;              MAP0      MAP1                 MAP2         MAP3
;
; In werkelijkheid gaat het over 4 bits (kolomlijnen) die telkens tegelijk
; worden gelezen voor elke van de 4 scantoestanden van de rij-lijnen.
; De flip-flops zijn softwarematig bitmaps waar de toestand van de kolomlijnen
; en de afgeleide (doorgeschoven) toestanden worden bewaard.
; Zo zijn er dus 4 maps van 4x4 bits die worden georganiseerd als 2x4 bytes.
; Map0 (uitgangen 1ste flip-flop) en map1 (uitgangen 2de flip-flop) worden
; samen in dezelfde 4 bytes ( 1byte per scanrij) gestockeerd.
; Ook map2 (uitgangen 3de flip-flop) en map3 (uitgangen 'OUT') worden samen
; opgeslagen. In map3 zitten de toetsen opgeslagen waarbij er een flank werd
; gedetecteerd (bit = 1 bij de flank bij het indrukken van een toets) 
; 
;          MAP0    MAP1          MAP2    MAP3
;         _______________       _______________
;        |_|_|_|_|_|_|_|_|     |_|_|_|_|_|_|_|_|  <--RIJ LIJN 1
;        |_|_|_|_|_|_|_|_|     |_|_|_|_|_|_|_|_|  <--RIJ LIJN 2
;        |_|_|_|_|_|_|_|_|     |_|_|_|_|_|_|_|_|  <--RIJ LIJN 3
;        |_|_|_|_|_|_|_|_|     |_|_|_|_|_|_|_|_|  <--RIJ LIJN 4
;
;         ^ ^ ^ ^ ^ ^ ^ ^       ^ ^ ^ ^ ^ ^ ^ ^
;         | | | | | | | |       | | | | | | | |
;        K K K K K K K K       K K K K K K K K 
;         O O O O O O O O       O O O O O O O O
;         L L L L L L L L       L L L L L L L L
;         1 2 3 4 1 2 3 4       1 2 3 4 1 2 3 4
;
; Het programma algorithme loopt van achter in de figuur naar voor, waarbij 
; eerst de logische functies uitgevoerd worden en dan pas de schuifregister-
; acties. Het schuiven van de schuifregisters vertaald zich in het copieren 
; van bitmaps van voor naar achter.
; (map2->map3, map1->map2, map0->map1 en de kolominputs->map0)
;
; De gebruiker moet eerst de key_init routine uitvoeren om de nodige bitmaps
; en variabelen: key, keyvalid flag en keymode flag te initialiseren.
; Bij het periodisch uitvoeren (vb. om de 20ms) van de keyscan routine worden 
; de toetsen binnengelezen en gedebounced. Na het uitvoeren van de keyscan-
; routine kan de gebruiker de routine  key_edge of key_pressed uitvoeren.
; Deze routines geven via de keyvalid flag aan dat er al dan niet: een toets
; pas is ingedrukt (key_edge routine) of ingedrukt wordt gehouden (key_pressed 
; routine). De variabele key kan dan door de gebruiker worden gelezen en geeft
; deze toetswaarde weer.
; 
;
;
;******************************************************************************


;******************************************************************************
; key_init
; Dit is de initialisatieroutine voor de keyboardscanner. De keyboardscanner
; gebruikt 9 bytes intern datageheugen, deze worden hier op 0 gezet.
; input: niets
; output: De bitmaps voor de verschillende scanlijnen worden op nul gezet 
;         (map0,map1,map2,map3 =0) 
;         key = 0 , keyvalid flag = 0, keymode flag = 0 (keyedge mode)
; Deze routine vernietigt niets
;******************************************************************************

Key_init:       push    acc          ;bewaar registers
                push    psw
                mov     a,r0        
                push    acc
                mov     a,r1
                push    acc
                mov     r0,#map01ptr ;pointer naar debounce en flankdetector maps
                mov     r1,#9        ;9 bytes in de maps
Keyinit1:       mov     @r0,#0       ;clear byte in map
                inc     r0           ;volgende byte nemen
                djnz    r1,Keyinit1  ;zijn we al klaar, nee: dan volgende nemen
                clr     keyvalid     ;ja, geldige toets-flag afzetten.
                clr     keymode      ;keybord leesmode op flankdetectie zetten
                mov     key,#0       ;default vertrekwaarde KEY
                pop     acc          ;herstel registers
                mov     r1,a
                pop     acc
                mov     r0,a
                pop     psw
                pop     acc
                ret                    ;terug naar caller
;******************************************************************************


;******************************************************************************
; toets_flank
; Deze routine detecteert de indrukflank van een toets.
; Een geldige toetswaarde wordt gestockeerd in 'key' en aangegeven door
; de 'keyvalid' flag op '1'. Deze routine moet periodisch worden uitgevoerd 
; door de gebruiker (bv. om de 20ms), dit kan door deze routine op te roepen 
; vanuit een timerinterruptroutine (aanbevolen) of in een hoofdprogrammalus die
; minder dan 20ms duurt. 
; input : niets
; output: keyvalid flag = 0 : geen geldige toetswaarde in key
;         keyvalid flag = 1 : er is een geldige toetswaarde in key gestockeerd
;         key : ascii code van de ingdrukte toets (zie keyvalid flag)
;
; Deze routine vernietigt geen registers.
;******************************************************************************
Toets_flank:    push    acc    
                push    psw
                lcall   Keyscan
                lcall   Keyedge
                pop     psw
                pop     acc
                ret

;******************************************************************************
; toets_ingedrukt
; Deze routine detecteert een ingedrukte toets.
; Een geldige toetswaarde wordt gestockeerd in 'key' en aangegeven door
; de 'keyvalid' flag op '1'. Deze routine moet periodisch worden uitgevoerd 
; door de gebruiker (bv. om de 20ms), dit kan door deze routine op te roepen 
; vanuit een timerinterruptroutine (aanbevolen) of in een hoofdprogrammalus die
; minder dan 20ms duurt. 
; input : niets
; output: keyvalid flag = 0 : geen geldige toetswaarde in key
;         keyvalid flag = 1 : er is een geldige toetswaarde in key gestockeerd
;         key : ascii code van de ingdrukte toets (zie keyvalid flag)
;
; Deze routine vernietigt geen registers.
;******************************************************************************
Toets_ingedrukt:push    acc
                push    psw
                lcall   Keyscan
                lcall   Keydown
                pop     psw
                pop     acc
                ret


;******************************************************************************
; Keyscan  (wordt opgeroepen door de toets_flank en toets_ingedrukt routines)
; Deze routine zorgt voor het scannen van de verschillende rij-lijnen, het
; resultaat  van ingedrukte toetsen komt dan binnen op de kolomlijnen.
; De Keyscan routine werkt met verschillende bitmaps die bijhouden wat de
; vorige toestand was van een toets. Deze worden hier verwerkt en doorgeschoven 
; zoals aangegeven in het blokschema van het algorithme.
; input : niets
; Output: update van de bitmaps die de toetstoestand en de recente geschiedenis
;         ervan weergeven
;
; Deze routine vernietigt geen registers
;******************************************************************************

Keyscan:        push    acc            ;gebruikte registers op stack
                push    psw
                push    b
                mov     a,r0
                push    acc
                mov     a,r1
                push    acc
                mov     a,r2
                push    acc
                mov     a,r6
                push    acc
                
                mov     r6,#11111110b  ;scanpatroon laden (0 circulatie)
                mov     b,#4           ;scanline counter (4 scanlijnen=rijen)
                mov     r0,#map01ptr   ;pointer naar debounce maps
                mov     r1,#map23ptr   ;pointer naar flankdetectiemaps

Scanloop:        
ifdef        aduc832_v1_1    ;keyboard scanlijnen zitten op twee poorten
                mov     a,r6           ;splits het scanpatroon op voor p0 en p3
                mov     c,acc.0        ;alleen twee bits van poort0 aansturen
                mov     p0.0,c
                mov     c,acc.1
                mov     p0.1,c
                orl     a,#00000011b ;P3.0 en P3.1 zijn Txd en Rxd -> afblijven
                mov     p3,a         ;stuur de scanlijnen naar P3
endif

ifdef        aduc832_v2_0    ;keyboardscanlijnen zitten alleen op poort0
                mov     p0,r6        ;scanline activeren met nul niveau op 
                                      ;een rij-lijn
endif 

; Eerst de flipflops implementeren.
; Dit kan eenvoudig door de bitmaps door te schuiven en een nieuwe entry
; te lezen van de kolomlijnen
                mov     a,@r0         ;4 bits uit de debouncemaps laden 
                                      ;( MAP0=bit 7..4,MAP1=bit 3..0 )
                mov     r2,a          ;MAP0 en MAP1 even bewaren
                swap    a             ;MAP0 en MAP1 omwisselen
                anl     a,r2          ;and functie tussen MAP0 en MAP1
                anl     a,#11110000b  ;maskeren lsb's
                mov     r2,a          ;nieuwe map2 even bewaren
                mov     a,@r1        ;ophalen oude map2 en map3
                anl     a,#00001111b  ;oude map2 weggooien
                orl     a,r2          ;samenvoegen oude MAP3 en nieuwe map2
                mov     @r1,a         ;bewaren nieuwe MAP2 en oude MAP3
                mov     a,@r0         ;MAP1 en MAP0 weer nemen
                swap    a             ;MAP0 naar MAP1 schuiven en omgekeerd
                anl     a,#00001111b  ;nieuwe MAP1 bijhouden
                mov     r2,a          ;even bewaren in register
                
                
ifdef        aduc832_v1_1    ;keyboard kolomlijnen zitten op poort3
                mov     a,p3          ;nieuwe entry (4 msb's) lezen van de 
                                      ;poort3 kolomlijnen
endif                            
ifdef        aduc832_v2_0    ;keyboardkolom lijnen zitten op poort0
                mov     a,p0         ;nieuwe entry (4 msb's) lezen van de 
                                     ;poort0 kolomlijnen
endif
                cpl     a             ;ingedrukte toets -> een logische '1'
                anl     a,#11110000b  ;scanlijnen (rijen) wegmaskeren
                orl     a,r2          ;nieuwe entry samenvoegen met nieuwe MAP1
                mov     @r0,a         ;en bewaren als nieuwe map0 en map1
                                
; De flipflops (doorschuiven van maps) zijn nu uitgevoerd.
; Nu volgen de logische functies uit het blokschema

                mov     a,@r0        ;4 bits uit de debouncemaps laden 
                                     ;( MAP0=bit 7..4,MAP1=bit 3..0 )
                mov     r2,a         ;MAP0 en MAP1 even bewaren
                swap    a            ;MAP0 en MAP1 omwisselen
                anl     a,r2         ;and functie tussen MAP0 en MAP1
                anl     a,#00001111b ;low nibble houden
                mov     r2,a         ;resultaat bewaren 
                mov     a,@r1        ;4 bits uit de flankdetectiemaps laden
                                     ;(MAP2=bit7..4,MAP3=bit3..0)
                cpl     a            ;MAP2 inverteren( +edge=toets ingedrukt)
                swap    a            ;map2 naar low nibble schuiven
                anl     a,r2         ;combineren met output vorige and poort
                anl     a,#00001111b ;low nibble voor MAP3 overhouden
                mov     r2,a         ;even bewaren
                mov     a,@r1        ;map2 en map3 weer ophalen
                anl     a,#11110000b ;map2 overhouden
                orl     a,r2         ;en combineren met nieuwe map3
                mov     @r1,a        ;en weer wegschrijven in map2 en map3
                
; We gaan nu de volgende lijn scannen

                mov     a,r6         ;scanpatroon nemen
                rl      a            ;'0' opschuiven=volgende scanlijn activeren
                mov     r6,a         ;scanpatroon bewaren

                inc     r0           ;MAP0 en MAP1 pointer eentje verder zetten
                inc     r1           ;MAP2 en MAP3 pointer eentje verder zetten
                djnz    b,Scanloop   ;4 lijnen scannen, en daarna gedaan
                pop     acc          ;herstel registers
                mov     r6,a
                pop     acc
                mov     r2,a
                pop     acc
                mov     r1,a
                pop     acc
                mov     r0,a
                pop     b
                pop     psw
                pop     acc
                ret                    ;terug naar caller
;******************************************************************************


;******************************************************************************
; Keydown, Keyedge (twee entry's)
; (wordt opgeroepen door de toets_flank en toets_ingedrukt routines)
;
; Deze routine zal uit de opgeslagen bitpatronen in de bitmaps de ingedrukte
; toets bepalen. De Keyscan routine moet steeds eerst worden uitgevoerd.
; Er zijn twee detectie-modes: 
; flankdetectie mode:   In deze mode zal 'indrukflank' van een toets worden 
;                       aangegeven. Nadat de gebruiker de toetswaarde heeft 
;                       gelezen en de 'keyvalid' flag heeft gereset zal pas
;                       wanneer de toets wordt losgelaten en opnieuw wordt 
;                       ingedrukt deze opnieuw worden aangegeven.    
; toets ingedrukt mode: In deze mode zal zolang een toets wordt ingedrukt 
;                       deze telkens,na het lezen en resetten van de 'keyvalid'
;                       flag door de gebruiker, opnieuw worden aangegeven.
; 
; Keyedge is de entry om na te gaan of een toets pas is ingedrukt 
; (flankdetectie mode).
; keydown is de entry om na te gaan of een toets continue is ingedrukt
; (toets ingedrukt mode). 
;
; De gedetecteerde toetspositie wordt door deze routine via een lookup tabel
; omgezet naar een toetswaarde. De lookuptabel kan door de gebruiker worden
; aangepast in functie van de toepassing. De default tabel met ascii codes
; geldt voor een standaard 4x4 keypad.
;
; De gedetecteerde toetswaarde wordt door deze routine achtergelaten in de
; variabele 'Key', en de 'keyvalid' flag geeft aan dat er geldige toetswaarde
; in 'key' aanwezig is. De gebruiker moet in zijn toepassing na het lezen van 
; de toetswaarde uit 'key' de 'keyvalid' flag zelf weer resetten.
;
; input : de door de Keyscan routine aangepaste bitmaps
; output: key = gedetecteerde toets
;         keyvalid flag =1 : er zit een geldige waarde in key
;         keyvalid flag =0 : er is geen nieuwe toetswaarde
;
; Deze routine vernietigt geen registers
;******************************************************************************

;'Toets ingedrukt mode' enty van de routine
Keydown:        push    acc            ;registers bewaren
                push    psw
                push    b
                push    dpl
                push    dph
                mov     a,r0
                push    acc
                mov     a,r1
                push    acc
                mov     a,r2
                push    acc
                mov     r0,#map01ptr   ;pointer naar debounce maps
                setb    keymode        ;'toets ingedrukt'mode (keymode =1)
                sjmp    Key1           ;continue

;'Flankdetectiemode' entry van de routine
Keyedge:        push    acc            ;registers bewaren
                push    psw
                push    b
                push    dpl
                push    dph
                mov     a,r0
                push    acc
                mov     a,r1
                push    acc
                mov     a,r2
                push    acc
                mov     r0,#map23ptr  ;pointer naar flankdetectie maps
Key1:           mov     r2,#4         ;rijteller laden
Rijloop:        mov     r1,#4         ;kolomteller laden
                mov     a,@r0         ;map entry ophalen
                jnb     keymode,Key2  ;als flank mode dan verder...
                mov     b,a           ;nee, dan map0 en map1 even bewaren
                swap    a             ;map1 en map0 omwisselen
                anl     a,b           ;map0 en map1 'and'en, of debouncen
Key2:           swap    a             ;map 1 en map0 verwisselen
                rlc     a             ;kolombit testen
Kolomloop:      jc      toetsin       ;als carry dan toets gedrukt
                rlc    a              ;nee, dan verder roteren
                djnz    r1,Kolomloop  ;test alle kolommen
                inc     r0            ;rijpointer in maps verder zetten
                djnz    r2,Rijloop    ;test alle rijen
                sjmp    nokey         ;alles getest en geen entry

;Er is een toets gedrukt, we gaan nu bepalen waar de overeenkomende code staat
;in de toetstabel. Dit doen we aan de hand van de rij- en kolom pointers.
;De offset in de toetstabel berekenen we als volgt: offset=(rij-1)x4+(kolom-1)   

toetsin:        dec     r2            ;rij-1
                dec     r1            ;kolom-1
                mov     a,r2          ;neem rij-1
                rl      a             ;(rij-1)x2
                rl      a              ;(rij-1)x4
                add     a,r1          ;offset=(rij-1)x4 + (kolom-1)
                mov     dptr,#toetstabel ;startadres van de toetstabel
                movc    a,@a+dptr     ;haal toetscode op startadres+offset
                mov     key,a         ;stop toetscode in de variabele key
                setb    keyvalid      ;geldige toets is gevonden
nokey:          clr     keymode        ;default in flankdetectie-mode
                pop     acc            ;herstellen registers
                mov     r2,a
                pop     acc
                mov     r1,a
                pop     acc
                mov     r0,a
                pop     dph
                pop     dpl
                pop     b
                pop     psw
                pop     acc
                ret                    ;terug naar caller
;******************************************************************************
;Toetstabel voor een standaard 4x4 keyboard.
;De gebruiker kan hier andere waarden voor de toetscodes ingeven

toetstabel:     db    '*'    ;kolom1@rij4 toets
                db    '0'    ;kolom2@rij4 toets
                db    '#'    ;kolom3@rij4 toets
                db    'D'    ;kolom4@rij4 toets
                        
                db    '7'    ;kolom1@rij3 toets
                db    '8'    ;kolom2@rij3 toets
                db    '9'    ;kolom3@rij3 toets
                db    'C'    ;kolom4@rij3 toets

                db    '4'    ;kolom1@rij2 toets
                db    '5'    ;kolom2@rij2 toets
                db    '6'    ;kolom3@rij2 toets
                db    'B'    ;kolom4@rij2 toets

                db    '1'    ;kolom1@rij1 toets
                db    '2'    ;kolom2@rij1 toets
                db    '3'    ;kolom3@rij1 toets
                db    'A'    ;kolom4@rij1 toets
              
;******************************************************************************
; switch_init
; Dit is de initialisatieroutine voor 'read_switch' de switchdebouncer/scanner. 
; Deze gebruikt 6 bytes intern datageheugen, deze worden hier op 0 gezet.
; input : niets
; output: De 6 bytes voor de debouncer en flankdetectoren worden op 0 gezet.
;         Deze routine reset ook de 'swvalid','negvalid' en 'posvalid' flag.
;         
; Deze routine vernietigt niets
;******************************************************************************

switch_init:    push    acc
                push    psw
                push    b
                mov     a,r0         ;bewaar registers
                push    acc
                
                mov     r0,#switch1  ;pointer naar debounce&flankdetectorbytes
                mov     b,#6         ;6 bytes met bitmaps
swinit:         mov     @r0,#0       ;clear byte in map
                inc     r0           ;volgende byte nemen
                djnz    b,swinit     ;zijn we al klaar, nee: dan volgende nemen
                clr     swvalid      ;ja, flags afzetten.
                clr     negvalid
                clr     posvalid
                
                pop     acc
                mov     r0,a
                pop     b
                pop     psw
                pop     acc
                ret                   ;terug naar caller

;******************************************************************************
; read_switch
; Dit is de routine voor het lezen, debouncen en de +/- flankdetecties van de
; 8 DIP schakelaars (of de druktoetsen) tegelijk. 
; Deze routine werkt volgens onderstaand blokschema.
;
;                                  ___                    ___
;                     ____________| 1 |   ---------*-----| 2 |
;              ____  |   ____     |   |  |   ____  |     |   |
;SWitch |\    |    | |  |    |    |&  |__*__|    | |     |&  |
;     --| o---|D  Q|_*__|D  Q|----|___|     |D  Q|-|--*-o|___|---- negedge
;Px.y   |/    | cl |    | cl |              | cl | |  |   ___  (negvalid-flag)
;             |____|    |____|              |____|  ----o| 3 |
;                |         |                   |      |  |   |
;      CLOCK     |         |                   |      |  |&  | (posvalid-flag)
;         _______*_________*___________________|       --|___|---- posedge
;
;            DEBOUNCE CIRCUIT                   FLANKDETECTIES
;                              (swvalid-flag)
;            switch1   switch2    debounce      closed 
;
; In werkelijkheid gaat het ipv. over 1 bit, zoals in de figuur,over 8 bits 
; die telkens tegelijk worden gelezen (8 schakelaars).
; De flip-flops zijn softwarematig bytes waar de toestand van de schakelaars
; en de vorige (doorgeschoven) toestanden in worden bewaard.
; Zo zijn er 6 bytes nodig voor de drie flipflops en de 2 edge detectoren.
; Switch1 (uitgangen 1ste flip-flop) en switch2 (uitgangen 2de flip-flop)
; vormen met de eerste and poort de debouncer voor de 8 schakelaars.
; De byte 'closed' (uitgang 3de flipflop) vormt samen met de tweede and-
; poort een detector voor negatieve flanken (negedge) en met de derde andpoort
; een detector voor positieve flanken (posedge).
; 
; input : de schakelaars wordt gelezen ifv. de versie van het aduc800 bord
;         bij aduc832v1.1 : 
;         als f0=0 of f0=1 :altijd de schakelaars op p3/p0
;         bij aduc832v2.0 : 
;         als f0=0 dan: schakelaars op p0
;          als f0=1 dan: 4 MSB schakelaars op p3, p3.3-p3.0 worden gemaskeerd
;
; output: De 5 bytes met bitmaps voor de debouncer en flankdetectoren worden 
;         geactualiseerd. 
;         debounce: een byte waarin een bit op '1' de actueel ingedrukte 
;                  schakelaars weergeeft
;         swvalid: een flag die aangeeft dat 'debounce' geactualiseerd is.
;         negedge: een byte waarin een bit op '1' aangeeft dat er voor die 
;                  schakelaar een negetieve flank werd gedetecteerd.
;         negvalid:een flag die aangeeft dat er een nieuwe negatieve flank 
;                  werd gedetecteerd op een schakelaar.
;         posedge: een byte waarin een bit op '1' aangeeft dat er voor die 
;                  schakelaar een positieve flank werd gedetecteerd.
;         posvalid:een flag die aangeeft dat er een nieuwe positieve flank
;                  werd gedetecteerd op een schakelaar.
;
;
; De gebruiker moet eerst de switch_init routine uitvoeren om de nodige bytes
; en flags te initialiseren.
; Bij het periodisch uitvoeren (vb. om de 20ms) van de read_switch routine 
; worden de schakelaars binnengelezen en gedebounced. Na het uitvoeren van de 
; read_switch routine kan de gebruiker de flags 'swvalid', 'negvalid' en
; 'posvalid' checken en de bijhorende bytes 'debounce', 'negedge' en 'posedge' 
; lezen. De gebruiker zal na het lezen van debounce, negedge of posedge de 
; bijhorende flag resetten.  
; 
; Deze routine vernietigt niets
;******************************************************************************                
read_switch:    push    acc            ;registers bewaren
                push    psw
                push    b
;Flipflops uitvoeren is bytes doorschuiven                
                mov     closed,debounce;uitgang eerste andpoort naar 3de FF
                mov     switch2,switch1;uitgang 1ste FF naar 2de FF
                
ifdef        aduc832_v1_1    ;schakelaars zitten op twee poorten
                clr      a
                mov     c,p0.0        ;lees laagste schakelaars
                mov     acc.0,c        
                mov     c,p0.1
                mov     acc.1,c
                mov     b,a            ;even bewaren
                mov     a,p3           ;lees 6 hoogste schakelaars
                anl     a,#11111100b   ;maskeer rxd en txd
                orl     a,b            ;laagste schakelaars bijvoegen
endif        ;einde aduc832_v1_1

ifdef        aduc832_v2_0        ;schakelaars zitten op poort0
                jb      f0,read_port3  ;indien flag=1, dan de druktoetsen lezen
                mov     a,p0           ;lees de schakelaars
                sjmp    lowlogic
read_port3:     mov     a,p3           ;lees druktoetsen
                orl     a,#0fh         ;en maskeer de 4 lsb's
endif        ;einde aduc832_2_0

lowlogic:       cpl     a              ;schakelaars complementeren
                mov     switch1,a      ;en sample opslaan

;Flipflops zijn klaar, nu de logische schakelingen (and poorten)

                mov     a,switch1      ;neem laatste sample
                anl     a,switch2      ;and met vorige sample,-> ontdender 
                mov     debounce,a    ;en bewaar ontdenderde toestand
                setb    swvalid       ;nieuwe schakelaartoestand opgeslagen
;negatieve flankdetectie 
                mov     a,closed      ;neem 3de FF
                cpl     a             ;complementeer
                anl     a,debounce    ;en combineer met signaal uit and poort
                mov     negedge,a     ;bewaar negatieve flanken
                jz      read_switch1  ;spring als geen negatieve flanken
                setb    negvalid      ;ja, negatieve flank: status updaten
                sjmp    read_switch2  ;negatieve flank, dus er is geen positieve
;positieve flankdetectie
read_switch1:   mov     a,debounce    ;neem uitgang and poort
                cpl     a             ;complementeer
                anl     a,closed      ;combineer met 3de FF
                mov     posedge,a    ;bewaar positieve flanken
                jz      read_switch2 ;spring als geen positieve flanken
                setb    posvalid     ;ja, positieve flank: status updaten
read_switch2:   pop     b            ;registers herstellen
                pop     psw
                pop     acc
                ret

endif    ;einde van aduc_key              
              
              
