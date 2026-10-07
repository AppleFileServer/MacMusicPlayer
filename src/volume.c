#include "volume.h"

/* SdVolume (low memory 0x0260) holds the level in its low three bits; the
   hardware takes it from VIA port A bits 0-2 (Mac Plus). The VIA's base
   address is in low memory at 0x01D4; port A's register is at offset
   0x1E00 (register 15, 512 bytes apart). */
#define kSdVolume       0x0260
#define kVIA            0x01D4
#define kVBufA          0x1E00

short Volume_Get(void)
{
    return *(volatile unsigned char *)kSdVolume & kVolumeMax;
}

void Volume_Set(short level)
{
    volatile unsigned char *sdVolume = (volatile unsigned char *)kSdVolume;
    unsigned char *via = *(unsigned char **)kVIA;
    unsigned char bits;

    if (level < 0)
        level = 0;
    if (level > kVolumeMax)
        level = kVolumeMax;
    bits = (unsigned char)level;

    *sdVolume = (*sdVolume & ~kVolumeMax) | bits;

    /* Port A also carries other lines (the floppy head select among them,
       changed by the disk driver at interrupt time), so only bits 0-2 are
       changed, each step a single read-modify-write instruction, which an
       interrupt can't split. */
    __asm__ volatile (
        "andi.b #0xF8,0x1E00(%0)\n\t"
        "or.b   %1,0x1E00(%0)"
        : : "a" (via), "d" (bits) : "cc", "memory");
}
