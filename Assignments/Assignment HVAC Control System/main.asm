//-----------------------------
// Title: HVAC Control System
//-----------------------------
// Purpose: Enable and disable the hot air blower or cooling fan based upon
// a given desired temperature and actual temperature.
// Dependencies: MyConfigFile.inc
// Compiler: v6.30
// Author: Brandon McCulloch
// OUTPUTS: PORTD.1 (Heating System/LED0) and PORTD.2 (Cooling System/LED1)
// INPUTS: measuredTemp (Temperature Sensor) refTemp (Keypad)
// Versions:
//  	V1.0: 03/08/26
//-----------------------------

;----------------
; PROGRAM INPUTS
;----------------
;The DEFINE directive is used to create macros or symbolic names for values.
;It is more flexible and can be used to define complex expressions or sequences of instructions.
;It is processed by the preprocessor before the assembly begins.

PROCESSOR 18F47K42
#include <xc.inc>
#include "MyConfigFile.inc"

	    
#define  measuredTempInput	15 ; this is the input value
#define  refTempInput		15 ; this is the input value
#define	 SUBTR			10

;---------------------
; Definitions
;---------------------
#define SWITCH    LATD,2  
#define LED0      PORTD,1
#define LED1	  PORTD,2
    
 
;---------------------
; Program Constants
;---------------------
; The EQU (Equals) directive is used to assign a constant value to a symbolic name or label.
; It is simpler and is typically used for straightforward assignments.
;It directly substitutes the defined value into the code during the assembly process.

	    
REG10		equ   10h	; in HEX
REG11		equ   11h
REG01		equ   1h
		
refTemp		equ   0x20	; Define data memory locations for variables  
measuredTemp	equ   0x21
contReg		equ   0x22
tempReg		equ   0x23
incrementReg	equ   0x24

refOnes		equ   0x60
refTens		equ   0x61
refHundeds	equ   0x62

measOnes	equ   0x70
measTens	equ   0x71
measHundreds	equ   0x72
	
PSECT absdata,abs,ovrld
	
	    
	    ORG	    0x20	    
	    MOVLW   0x0C	;Initialize PORTD pins with desired inputs/outputs
	    MOVWF   TRISD
	    MOVWF   PORTD
	    
	    MOVLW   refTempInput    ;Move input values intomeasured and reference temp registers
	    MOVWF   refTemp, 0
	    MOVLW   measuredTempInput
	    MOVWF   measuredTemp, 0
	    
	    ORG	    0x40
BACK:
	    GOTO    CONVANDHEAT	    ; Immediately check if any values are negative.
	    ORG	    0x60
CONTINUE:   
	    ; If measured temperature is greater than reference. Perform cooling actions. Move to HEAT if not.
	    MOVF    refTemp, 0, 0
	    CPFSGT  measuredTemp, 0
	    GOTO    HEAT
	    MOVLW   0x02
	    MOVWF   contReg, 0
	    GOTO    LED_TOGGLE
	    
	    ORG	    0x100
HEAT:
	    ; If measured temperature is less than reference. Perform heating action. Move to NEUTRAL if not.
	    CPFSLT  measuredTemp, 0
	    GOTO    NEUTRAL
	    MOVLW   0x01
	    MOVWF   contReg, 0
	    GOTO    LED_TOGGLE
	    
	    ORG	    0x120
	    ; Disable cooling and heating system if measured temperature equal each other
NEUTRAL:
	    MOVLW   0x00
	    MOVWF   contReg, 0
	    GOTO    LED_TOGGLE
	    
	    ORG	    0x150
	    ; Enable and disable desired pins in PORTD. Then move to MEAS_LOOP
LED_TOGGLE:
	    MOVFF   contReg, PORTD
	    GOTO    MEAS_LOOP

	    
	    ; Checks if a vaue is negative by checking the most significant bit,
	    ; and points NEGATIVE; otherwise, just return to continue
	    ORG	    0x230
CONVANDHEAT:
	    MOVFF   measuredTemp, tempReg
	    BTFSC   tempReg, 7, 0
	    GOTO    NEGATIVES
	    GOTO    CONTINUE
	    
	    ; Obtains tens place value of the measured temp value with subtracting by 10.
	    ; Stores the value into register 11 and moves onto negative ten number check.
	    ORG	    0x260
MEAS_LOOP:  
	    MOVLW   10
	    CPFSGT  tempReg, 0
	    GOTO    negTenCheckMeas
	    SUBWF   tempReg, 1, 0
	    INCF    REG11, 1, 0
	    GOTO    MEAS_LOOP
	    
	    ; Obtains ten's place value of the reference temp value with subtracting by 10.
	    ; Stores the value into register 10 and moves onto checking if value is 10
	    ORG	    0x290
REF_LOOP:  
	    MOVLW   10
	    MOVFF   refTemp, tempReg
	    
	    ORG	    0x310
REPEAT_LOOP:
	    CPFSGT  tempReg, 0
	    GOTO    tenCheckRef
	    SUBWF   tempReg, 1, 0
	    INCF    REG10, 1, 0
	    GOTO    REPEAT_LOOP
	    
	    ORG	    0x330
	    ; Checks if the measured value is 10 after performing tens subtraction. If so, increment register 11 once more.
negTenCheckMeas:
	    CPFSEQ  tempReg, 0
	    GOTO    MEAS_DONE
	    SUBWF   tempReg, 1, 0
	    INCF    REG11, 1, 0
	    GOTO    MEAS_LOOP
	    
	    ; Checks if the reference value is 10 after performing tens subtraction. If so, increment register 10 once more
	    ORG	    0x360
tenCheckRef:
	    CPFSEQ  tempReg, 0
	    GOTO    REF_DONE
	    SUBWF   tempReg, 1, 0
	    INCF    REG10, 1, 0
	    GOTO    REF_DONE
	    
	    ; Decimal conversion complete for measured value, move the results into appropriate registers.
	    ORG	    0x390
MEAS_DONE:
	    MOVFF   REG11, measTens
	    MOVFF   tempReg, measOnes
	    GOTO    REF_LOOP
	    
	    ; Decimal conversion complete for referenced value, move the results into appropriate registers.
	    ORG	    0x410
REF_DONE:
	    MOVFF   REG10, refTens
	    MOVFF   tempReg, refOnes
	    GOTO    COMPLETE 
	    
	    ; Performs 2's complement to represent the positive form of the value.  
	    ORG	    0x430
NEGATIVES:
	    NEGF    tempReg, 0
	    MOVLW   0x01
	    MOVWF   contReg, 0	    ;Enable PORTD to HEATER
	    MOVWF   PORTD, 0
	    GOTO    MEAS_LOOP
	    ; Program complete. End the program.
COMPLETE:
	    ORG	    0x460
	    END
	
   
	