#ifndef KNES_H_INCLUDED
#define KNES_H_INCLUDED

typedef unsigned char   byte;
typedef unsigned short  word;
typedef unsigned long   dword;

// this variable is increased in the NMI handler
extern volatile byte _nmi_count;
#pragma zpsym("_nmi_count");
#define NMI_COUNT() _nmi_count

// thanks to blargg for the idea, validation was removed
//   though because CC65 throws a warning when it is used
// also note that the resulting value is not a byte,
//   which might lead to performance penalties in some cases,
//   so cast to byte when necessary
#define __B(n)      ((n       & 0x01) |\
                     (n >>  2 & 0x02) |\
                     (n >>  4 & 0x04) |\
                     (n >>  6 & 0x08) |\
                     (n >>  8 & 0x10) |\
                     (n >> 10 & 0x20) |\
                     (n >> 12 & 0x40) |\
                     (n >> 14 & 0x80))

// convert a binary number, e.g. _B(10101100)
#define _B(a)       (__B(0##a))

// casts parameter to a byte pointer
#define _P(a)       ((byte volatile *)(a))
// returns byte at the specified address
#define _M(a)       (*_P(a))

#define LOBYTE(a)   ((a) & 0xFF)
#define HIBYTE(a)   (((a) >> 8) & 0xFF)

#define ABS(a)      ((a) >= 0 ? (a) : -(a))

enum PPUCtrl {
    BASE_NT0    = _B(00),
    BASE_NT1    = _B(01),
    BASE_NT2    = _B(10),
    BASE_NT3    = _B(11),
    ADDRINC_1   = _B(000),
    ADDRINC_32  = _B(100),
    SPR_CHR0    = _B(0000),
    SPR_CHR1    = _B(1000),
    BG_CHR0     = _B(00000),
    BG_CHR1     = _B(10000),
    SPR_8X8     = _B(000000),
    SPR_8X16    = _B(100000),
    NMI_OFF     = _B(00000000),
    NMI_ON      = _B(10000000)
};

enum PPUMask {
    GRAYSCL_OFF = _B(0),
    GRAYSCL_ON  = _B(1),
    BGCLIP_ON   = _B(00),
    BGCLIP_OFF  = _B(10),
    SPRCLIP_ON  = _B(000),
    SPRCLIP_OFF = _B(100),
    BGREND_OFF  = _B(0000),
    BGREND_ON   = _B(1000),
    SPRREND_OFF  = _B(00000),
    SPRREND_ON   = _B(10000),
    TINTRED_OFF  = _B(000000),
    TINTRED_ON   = _B(100000),
    TINTGRN_OFF  = _B(0000000),
    TINTGRN_ON   = _B(1000000),
    TINTBLU_OFF  = _B(00000000),
    TINTBLU_ON   = _B(10000000)
};

enum JoyState {
    JOY_A        = _B(1),
    JOY_B        = _B(10),
    JOY_SELECT   = _B(100),
    JOY_START    = _B(1000),
    JOY_UP       = _B(10000),
    JOY_DOWN     = _B(100000),
    JOY_LEFT     = _B(1000000),
    JOY_RIGHT    = _B(10000000)
};

enum SprAttrib {
    SPR_PAL0     = _B(00),
    SPR_PAL1     = _B(01),
    SPR_PAL2     = _B(10),
    SPR_PAL3     = _B(11),
    SPR_INFRNTBG = _B(000000),
    SPR_BEHINDBG = _B(100000),
    SPRFLIPH_OFF = _B(0000000),
    SPRFLIPH_ON  = _B(1000000),
    SPRFLIPV_OFF = _B(00000000),
    SPRFLIPV_ON  = _B(10000000),
};

// TODO: enumerate APU sweep/envelope/etc options

struct _APUsquare {
    byte vol;
    byte sweep;
    byte lo;
    byte hi;
};

struct _APUtriangle {
    byte linear;
    byte _dummy;
    byte lo;
    byte hi;
};

struct _APUnoise {
    byte vol;
    byte _dummy;
    byte lo;
    byte hi;
};

struct _APUdmc {
    byte freq;
    byte raw;
    byte start;
    byte len;
};

struct _APU {
    struct _APUsquare sq1;
    struct _APUsquare sq2;
    struct _APUtriangle tri;
    struct _APUnoise noise;
    struct _APUdmc dmc;
};

struct _CPU {
    struct _APU apu;
    byte oam_dma;
    byte apu_chn;
    byte joy1;
    byte joy2;
};
#define    CPU         (*(struct _CPU volatile *)0x4000)

struct _PPU {
    byte ctrl;
    byte mask;
    byte const status;
    byte oam_addr;
    byte oam_data;
    byte scroll;
    byte addr;
    byte data;
};
#define    PPU         (*(struct _PPU volatile *)0x2000)

struct ObjAttr {
    byte y;
    byte tile;
    byte attrib;
    byte x;
};
#define    OAM         ((struct ObjAttr *)0x200)

#define PPU_ADDR(a)       { PPU.addr = HIBYTE(a); PPU.addr = LOBYTE(a); }
#define OAM_DMA()         { CPU.oam_dma = HIBYTE((word)OAM); }
#define PPU_SCROLL(x,y)   { PPU.scroll = (x); PPU.scroll = (y); }

#define START_VIRTUANES_PROFILING()   { _M(0x401E) = 0; }
#define END_VIRTUANES_PROFILING()     { _M(0x401F) = 0; }

typedef void irq_handler(void);

// wait for vblank by checking a flag set by the NMI routine
extern void __fastcall__ wait_vblank(void);
// wait for vblank by polling the PPU status register (misses frames occasionally)
extern void __fastcall__ poll_vblank(void);
// read joystick state
extern byte __fastcall__ read_joy(byte n);
// set the irq handler
// OBS: you have to be very careful if you decide to program the irq handler in C
extern void __fastcall__ set_irq_handler(irq_handler *irq);
// set the nmi handler
extern void __fastcall__ set_nmi_handler(irq_handler *nmi);
// enable interrupts (CLI)
extern void __fastcall__ enable_interrupts(void);
// enable interrupts (SEI)
extern void __fastcall__ disable_interrupts(void);

#endif //!KNES_H_INCLUDED
