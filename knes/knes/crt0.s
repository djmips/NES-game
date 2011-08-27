.export   _init, _exit, _set_irq_handler
.import   _main

.export   __STARTUP__ : absolute = 1        ; Mark as startup
.import   __RAM_START__, __RAM_SIZE__       ; Linker generated

.import   __ZEROPAGE_LOAD__, __ZEROPAGE_RUN__, __ZEROPAGE_SIZE__

.import    copydata, zerobss, initlib, donelib

.include  "zeropage.inc"

.zeropage
.export __nmi_count
__nmi_count:       .res 1

user_irq:          .res 2
user_nmi:          .res 2

.segment  "STARTUP"
_init:
    sei
    cld
    
    ldx #0
    stx $2000
    stx $2001
    stx user_irq
    stx user_irq+1
    stx user_nmi
    stx user_nmi+1
    
    dex
    txs
    
    ; wait for the PPU to stabilize
    bit $2002
:
    bit $2002
    bpl :-
:
    bit $2002
    bpl :-
    
    ; disable frame interrupts
    lda #$80
    sta $4017
    lda $4015 ; clear frame interrupt flag if it happens to be on
    
    ; hide all sprites
    ldx #0
    lda #$FF
:
    sta $200,x
    dex
    bne :-
    
    ; TODO: clear PPU memory and init other NES related registers
    ;       to known values
    
    ; copy initialized zero page data over from __ZEROPAGE_LOAD__ to
    ; __ZEROPAGE_RUN__    
    lda #<__ZEROPAGE_SIZE__
    bne :+
    lda #>__ZEROPAGE_SIZE__
    beq no_zp_data
:
    ldx #0
copy_zp:
    lda __ZEROPAGE_LOAD__,x
    sta __ZEROPAGE_RUN__,x
    inx
    cpx #<__ZEROPAGE_SIZE__
    bne copy_zp
    
no_zp_data:
    
    ; ---
    
    lda #<(__RAM_START__ + __RAM_SIZE__)
    sta sp
    lda #>(__RAM_START__ + __RAM_SIZE__)
    sta sp+1
    
    jsr zerobss ; clear BSS segment
    jsr copydata ; copy initialized data from ROM to RAM
    jsr initlib ; run constructors
    
    jsr _main
    ; if main returns, reset
    
_exit:
    jsr donelib ; run destructors
    jmp _init ; reset

.macro generate_irq_handler ptr
    pha
    lda ptr+1
    bne have_user_irq
    lda ptr
    beq no_user_irq
have_user_irq:
    txa
    pha
    tya
    pha
    jsr trampoline
    pla
    tay
    pla
    tax
no_user_irq:
    pla
    rti
    
trampoline:
    jmp (ptr)
.endmacro
    
.proc nmi
    bit $2002 ; acknowledge the NMI
    inc __nmi_count
    generate_irq_handler user_nmi
.endproc
    
.proc irq
    generate_irq_handler user_irq
.endproc
    
.export _set_irq_handler
.proc _set_irq_handler
    sta user_irq
    stx user_irq+1
    rts
.endproc

.export _set_nmi_handler    
.proc _set_nmi_handler
    sta user_nmi
    stx user_nmi+1
    rts
.endproc
          
.segment "VECTORS"
    .word nmi
    .word _init
    .word irq
