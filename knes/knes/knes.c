#include "knes.h"

void __fastcall__ wait_vblank(void)
{
    byte old_count = NMI_COUNT();
    while(old_count == NMI_COUNT());
}    

void __fastcall__ poll_vblank(void)
{
    // read once first to make sure the flag is initially clear (we don't want
    // to return in the middle of a frame)
    byte dummy = PPU.status;
    while(!(PPU.status & 0x80));
}

extern byte __fastcall__ _read_joy0(void);
extern byte __fastcall__ _read_joy1(void);

byte __fastcall__ read_joy(byte n)
{
    return n == 0 ? _read_joy0() : _read_joy1();
}

void __fastcall__ enable_interrupts(void)
{
    asm("cli");
}

void __fastcall__ disable_interrupts(void)
{
    asm("sei");
}
