#include "knes.h"
#include "header.h"

struct ines_header {
    byte sig[4];
    byte prgbanks;
    byte chrbanks;
    byte flags6;
    byte flags7;
};

#pragma rodata-name(push, "HEADER")
// should be static really, but if it is CC65 will throw a warning about
// it being never used, and I don't like warnings
struct ines_header const header = {
    { 'N', 'E', 'S', 0x1A },
    NUM_16K_PRG_BANKS,
    NUM_8K_CHR_BANKS,
    (MAPPER & 0xF) << 4 | MIRRORING,
    (MAPPER & 0xF0)
};
#pragma rodata-name(pop)
