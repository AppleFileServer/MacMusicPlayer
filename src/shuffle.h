#ifndef SHUFFLE_H
#define SHUFFLE_H

#include <MacTypes.h>

/* Stage 9c: shuffle orders (settings.h's kShuffleOff, kShuffleSongs,
   kShuffleAlbums) for lists of library song indexes. */

/* Fisher-Yates over songs[first..n-1]. */
void Shuffle_Range(short *songs, short first, short n);

/* Orders songs (n of them) for shuffle mode: Songs puts songs[start]
   first if songRow (T3), else all random; Albums puts songs[start]'s
   album first if songRow, the albums in random order, each in track
   order. Off, and a list whose song to start with is an audiobook (S-2),
   keep their order. Returns the index to start playing at. */
short Shuffle_Order(short *songs, short n, short start, Boolean songRow,
                    short mode);

#endif
