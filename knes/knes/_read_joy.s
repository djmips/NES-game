; this is heavily based on blargg's "DMC-fortified controller read routine"
; see http://nesdev.parodius.com/bbs/viewtopic.php?t=4124

.export __read_joy0, __read_joy1

.zeropage
temp:       .res 1
temp2:      .res 1
temp3:      .res 1

.code

; Reads controller into A and temp.
; Unreliable if DMC is playing.
; Preserved: X, Y
; Time: 153 clocks
.macro m_read_joy_fast n
    ; Strobe controller
    lda #1          ; 2
    sta $4016       ; 4
    lda #0          ; 2
    sta $4016       ; 4
    
    ; Read 8 bits
    lda #$80        ; 2
    sta <temp       ; 3
:   lda $4016+n     ; *4
    
    ; Merge bits 0 and 1 into carry. Normal
    ; controllers use bit 0, and Famicom
    ; external controllers use bit 1.
    and #$03        ; *2
    cmp #$01        ; *2
    
    ror <temp       ; *5
    bcc :-          ; *3
                    ; -1
    lda <temp       ; 3
    rts             ; 6
.endmacro

; Reads controller into A.
; Reliable even if DMC is playing.
; Preserved: X, Y
; Time: ~660 clocks
.macro m_read_joy n
    jsr n
    sta <temp3
    jsr n
    pha
    jsr n
    sta <temp2
    jsr n
    
    ; All combinations of one controller
    ; change and one DMC DMA corruption
    ; leave at least two matching readings,
    ; and never just the first and last
    ; matching. No more than one DMC DMA
    ; corruption can occur.
    
                    ; X--X can't occur
    pla
    cmp <temp3
    beq :+          ; XX--
    cmp <temp
    beq :+          ; -X-X
    
    lda <temp2      ; X-X-
                    ; -XX-
                    ; --XX
:   cmp #0
    rts
.endmacro

.align 32

read_joy_fast0:
    m_read_joy_fast 0
read_joy_fast1:
    m_read_joy_fast 1

__read_joy0:
    m_read_joy read_joy_fast0
__read_joy1:
    m_read_joy read_joy_fast1
