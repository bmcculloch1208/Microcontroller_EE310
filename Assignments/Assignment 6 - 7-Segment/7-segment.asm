;---------------------
; Title: 7-Segent Display Counter
;---------------------
; Program Details:
; The purpose of this program is to increment or decrement, or set the
; the 7 segment display to 0 when button 1 and/or 2 is pressed.
    
; Inputs: RB0, RB1
; Outputs: RD0-6
; Setup: The Curiosity Board

; Date: March 24, 2026
; File Dependencies / Libraries: It is required to include the
;   MyConfigFile.inc and CallFunction.inc in the Header Folder
; Compiler: pic-as, v3.10
; Author: Brandon McCulloch
; Versions:
;       V1.0: Original

    
PROCESSOR 18F47K42
#include "MyConfigFile.inc"
#include <xc.inc>
#include "CallFunction.inc"
    
    
    
PSECT absdata,abs,ovrld	    ; Do not change
    
	
current_value	equ	0x20	;Define register location names
REG30		equ     0x30
REG31		equ     0x31
REG32		equ	0x32
		
First_loop	equ 1	    ; defining  number of times to loop (about 600ms)
Second_loop	equ 1
Third_loop	equ 1
	    
seg0  EQU 0x40	    ; reserve memory locations right next to each other for
seg1  EQU 0x41	    ; each segment 0 through F
seg2  EQU 0x42
seg3  EQU 0x43
seg4  EQU 0x44
seg5  EQU 0x45
seg6  EQU 0x46
seg7  EQU 0x47
seg8  EQU 0x48
seg9  EQU 0x49
segA  EQU 0x4A
segB  EQU 0x4B
segC  EQU 0x4C
segD  EQU 0x4D
segE  EQU 0x4E
segF  EQU 0x4F
	
ORG	0x20	    ; start program at 0x20

_setup:
    clrf	current_value	    ; clear current value
    clrf	WREG		    ; clear WREG
    RCALL	_setupPortD	    ; Call setupPortD function, then return
    RCALL	_setupPortB	    ; Call setupPortD function, then return
    LFSR	1, 0x40		    ; Load pointer 1 with 0x40
    MOVLW	0x3F		    ; Load seg0 to segF with their corresponding
    MOVWF	seg0		    ; binary representation
    MOVLW	0x06		    
    MOVWF	seg1
    MOVLW	0x5B
    MOVWF	seg2
    MOVLW	0x4F
    MOVWF	seg3
    MOVLW	0x66
    MOVWF	seg4
    MOVLW	0x6D
    MOVWF	seg5
    MOVLW	0x7D
    MOVWF	seg6
    MOVLW	0x07
    MOVWF	seg7
    MOVLW	0x7F
    MOVWF	seg8
    MOVLW	0x67
    MOVWF	seg9
    MOVLW	0x77
    MOVWF	segA
    MOVLW	0x7C
    MOVWF	segB
    MOVLW	0x39
    MOVWF	segC
    MOVLW	0x5E
    MOVWF	segD
    MOVLW	0x79
    MOVWF	segE
    MOVLW	0x71
    MOVWF	segF
    GOTO	beginning	    ; start at the beginning of reading values
	
_setupPortD:			    ; Initializes portD
    BANKSEL	PORTD ;
    CLRF	PORTD ;Init PORTD
    BANKSEL	LATD ;Data Latch
    CLRF	LATD ;
    BANKSEL	ANSELD ;
    CLRF	ANSELD ;digital I/O
    BANKSEL	TRISD ;
    MOVLW	0b00000000 ;Set RD[7:1] as outputs
    MOVWF	TRISD ;and set RD0 as ouput
    RETURN
    
_setupPortB:			    ; Initializes portB
    BANKSEL	PORTB ;
    CLRF	PORTB ;Init PORTB
    BANKSEL	LATB ;Data Latch
    CLRF	LATB ;
    BANKSEL	ANSELB ;
    CLRF	ANSELB ;digital I/O
    BANKSEL	TRISB ;
    MOVLW	0b00000011 ;
    MOVWF	TRISB ;
    RETURN
	
	
beginning:			    ; Start of reading values loop
	CALL	output		    ; Call output function to display result.
	BTFSC	PORTB, 0	    ; Check if button A is pressed, skip next line if "0"
	GOTO	buttonCheck1	    ; Goto buttonCheck1 function
	BTFSC	PORTB, 1	    ; Check if button B is pressed, skip next line if "0"
	GOTO	buttonCheck2	    ; Goto buttonCheck2 function
	GOTO	beginning	    ; return to the beginning

bothPressed:			    ; If both buttons are pressed, set the value to 0
	MOVLW	0x0		    ; and then return to beginning
	LFSR	1, 0x40
	CALL	loopDelay
	GOTO	beginning
	
buttonCheck1:			    ; Button A is pressed, if button B is also
	BTFSC	PORTB, 1	    ; pressed, call the bothPressed function above
	GOTO	bothPressed	    ; otherwise go to checkF
	GOTO	checkF
	
buttonCheck2:			    ; Button B is pressed, if button A is also pressed,
	BTFSC	PORTB, 0	    ; call bothPressed function above. otherwise go to check0
	GOTO	bothPressed
	GOTO	check0
	
checkF:				    ; Check to see if pointer1 is pointing to location of segF (0x4F)
	MOVLW	0x4F		    ; If so skip the next line which is increment 1. Otherwise
	CPFSEQ	FSR1L		    ; set pointer 1 to seg0 location (0x40), then delay and finally
	GOTO	increment	    ; return to beginning
	LFSR	1, 0x40
	CALL	loopDelay
	GOTO	beginning

check0:				    ; Check to see if pointer1 is pointing to location seg0 (0x40)
	MOVLW	0x40		    ; If so skip the next line which is decrement 1, Otherwise
	CPFSEQ	FSR1L		    ; set pointer 1 to segF location  (0x4F), then delay and finally
	GOTO	decrement	    ; return to beginning
	LFSR	1, 0x4F
	CALL	loopDelay
	GOTO	beginning

increment:			    ; Function to increment pointer1 location by 1, then delay and
	INCF	FSR1L, 1	    ; return to beginning
	CALL	loopDelay
	GOTO	beginning
	
decrement:			    ; Function to decrement pointer1 location by 1, then delay and
	DECF	FSR1L, 1	    ; return to beginning
	CALL	loopDelay
	GOTO	beginning
	
loopDelay:			    ; move number of loop values into each register
	MOVLW	First_loop	    
	MOVWF	REG30
	MOVLW	Second_loop
	MOVWF	REG31
	MOVLW	Third_loop
	MOVWF	REG32
	
_loop1:				    ; loopDelay function. Loops through three loops to get around
	DECF	REG30, 1	    ; 600ms of delay between reads.
	BNZ	_loop1
	MOVLW	First_loop
	MOVWF	REG30
	DECF	REG31, 1
	BNZ	_loop1
	MOVLW	Second_loop
	MOVWF	REG31
	DECF	REG32, 1
	BNZ	_loop1
	MOVLW	Third_loop
	MOVWF	REG32
	RETURN
	
output:				    ; copies the value pointer1 is pointing to into WREG
	MOVF	INDF1, W	    ; then moves to D pins to output value on 7-segment
	MOVWF	LATD
	RETURN			    ; return back to beginning