;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; This is the headerfile containing one or more functions 
; Note that all variables used in the functions must be defined here
; We must also have PSECT statement at the begining  
; ORG is used to speciafy where the program must be assembled
; See this for more details: 
; https://docs.google.com/presentation/d/1ncyPaBJeA2svo2R948PHlVa4i-EYXyIp/edit?usp=sharing&ouid=116265364704461279990&rtpof=true&sd=true
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
   
    
PSECT absdata,abs,ovrld        ; Do not change
ORG	    0x100
    
;---------------------
; Definitions & Variables
;---------------------    
what_button EQU		20h    ; this is used in the main and the header
 

;-------------------------------------
; Call Functions
;------------------------------------- 




