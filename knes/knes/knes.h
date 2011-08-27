#ifndef KNES_H_INCLUDED
#define KNES_H_INCLUDED

typedef unsigned char   u8_t;
typedef unsigned short  u16_t;
typedef unsigned long   u32_t;
typedef   signed char   s8_t;
typedef   signed short  s16_t;
typedef   signed long   s32_t;

typedef u8_t    byte;
typedef u16_t   word;
typedef u32_t   dword;

// This variable is increased in the NMI handler.
extern volatile byte _nmi_count;
#pragma zpsym( "_nmi_count" );
#define NMI_COUNT() _nmi_count

// Thanks to blargg for the idea, validation was removed
//   though because CC65 throws a warning when it is used.
// Also note that the resulting value is not a byte,
//   which might lead to performance penalties in some cases,
//   so cast to byte when necessary.
// TODO: Cast to byte, as this can't handle more than 8 bits
//   anyways
#define __B(n)      ( (n       & 0x01) |\
                      (n >>  2 & 0x02) |\
                      (n >>  4 & 0x04) |\
                      (n >>  6 & 0x08) |\
                      (n >>  8 & 0x10) |\
                      (n >> 10 & 0x20) |\
                      (n >> 12 & 0x40) |\
                      (n >> 14 & 0x80) )

// Convert a binary number, e.g. _B(10101100).
#define _B(a)       ( __B( 0##a ) )

// Casts parameter to a byte pointer.
#define _P(a)       ( ( byte* )(a) )
// Returns a reference to a byte at the specified address.
#define _M(a)       ( *_P( a ) )

// TODO: Verify that the result from these is a byte, if not, cast
#define LOBYTE(a)   ( (a) & 0xFF )
#define HIBYTE(a)   ( ( (a) >> 8 ) & 0xFF )

#define ABS(a)      ( (a) >= 0 ? (a) : -(a) )

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

enum PPUStatus {
    SPR_OVERFLOW   = _B(100000),
    SPR0_HIT       = _B(1000000),
    VBLANK_STARTED = _B(10000000)
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

// TODO: Enumerate APU sweep/envelope/etc options.

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
#define    CPU         ( *( struct _CPU volatile * )0x4000 )

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
#define    PPU         ( *( struct _PPU volatile * )0x2000 )

struct ObjAttr {
    byte y;
    byte tile;
    byte attrib;
    byte x;
};
#define    OAM         ( ( struct ObjAttr * )0x200)

#define PPU_ADDR(a)                             \
{                                               \
    PPU.addr = HIBYTE( a );                     \
    PPU.addr = LOBYTE( a );                     \
}

#define OAM_DMA()                               \
{                                               \
    PPU.oam_addr = 0;                           \
    CPU.oam_dma = HIBYTE( ( word )OAM );        \
}

#define PPU_SCROLL( x, y )                      \
{                                               \
    PPU.scroll = ( x );                         \
    PPU.scroll = ( y );                         \
}

#define START_VIRTUANES_PROFILING()             \
{                                               \
    _M( 0x401E ) = 0;                           \
}

#define END_VIRTUANES_PROFILING()               \
{                                               \
    _M( 0x401F ) = 0;                           \
}

// Start NintendulatorDX cycle counting timer
#define START_TIMER( timer )                    \
{                                               \
    _M( 0x4020 | ( timer & 0xF ) ) = 0;         \
}

// Stop NintendulatorDX cycle counting timer
#define STOP_TIMER( timer )                     \
{                                               \
    _M( 0x4030 | ( timer & 0xF ) ) = 0;         \
}

typedef void irq_handler( void );

// Wait for vblank by checking a flag set by the NMI routine.
extern void __fastcall__ wait_vblank( void );
// Wait for vblank by polling the PPU status register (misses frames occasionally).
extern void __fastcall__ poll_vblank( void );
// Read joystick state.
extern byte __fastcall__ read_joy( byte n );
// Set the IRQ handler.
// OBS: You have to be very careful if you decide to program the IRQ handler in C.
extern void __fastcall__ set_irq_handler( irq_handler *irq );
// Set the NMI handler.
extern void __fastcall__ set_nmi_handler( irq_handler *nmi );
// Enable interrupts (CLI).
extern void __fastcall__ enable_interrupts( void );
// Enable interrupts (SEI).
extern void __fastcall__ disable_interrupts( void );

#endif //!KNES_H_INCLUDED
