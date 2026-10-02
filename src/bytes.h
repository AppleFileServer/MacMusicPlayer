#ifndef BYTES_H
#define BYTES_H

/* Big-endian integers in persistent data (song files, prefs), read and
   written a byte at a time: never through casts to short/long pointers,
   which would fault on the 68000 at odd addresses and depend on struct
   layout. */

static inline unsigned short Get16(const unsigned char *p)
{
    return (unsigned short)((p[0] << 8) | p[1]);
}

static inline unsigned long Get32(const unsigned char *p)
{
    return ((unsigned long)p[0] << 24) | ((unsigned long)p[1] << 16)
         | ((unsigned long)p[2] << 8) | p[3];
}

static inline void Put16(unsigned char *p, unsigned short v)
{
    p[0] = (unsigned char)(v >> 8);
    p[1] = (unsigned char)v;
}

static inline void Put32(unsigned char *p, unsigned long v)
{
    p[0] = (unsigned char)(v >> 24);
    p[1] = (unsigned char)(v >> 16);
    p[2] = (unsigned char)(v >> 8);
    p[3] = (unsigned char)v;
}

#endif
