#include <Quickdraw.h>

#include "shuffle.h"
#include "library.h"
#include "settings.h"

/* Stage 9c: putting a list in Shuffle order; also used by playback to
   reshuffle each time round with Repeat All (9d, T5). */

static short RandomBelow(short n)
{
    return (short)((unsigned short)Random() % n);
}

/* Fisher-Yates over songs[first..n-1]. */
void Shuffle_Range(short *songs, short first, short n)
{
    short i, j, t;

    for (i = n - 1; i > first; i--)
    {
        j = first + RandomBelow(i - first + 1);
        t = songs[i];
        songs[i] = songs[j];
        songs[j] = t;
    }
}

static short gShuffleOut[kMaxSongs + 1];
static short gAlbumOrder[kMaxSongs + 1];

/* Shuffle Albums: the albums in random order, the chosen song's album
   first (T3), each album's songs in track order. Songs without an album
   are one group. Returns where the chosen song is (0 for none). */
static short ShuffleAlbums(short *songs, short n, short chosen)
{
    short albums = 0, i, j, k, a, out = 0, start = 0, s;
    Boolean seen;

    for (i = 0; i < n; i++)
    {
        a = Library_SongAlbum(songs[i]);
        seen = false;
        for (j = 0; j < albums; j++)
            if (gAlbumOrder[j] == a)
                seen = true;
        if (!seen)
            gAlbumOrder[albums++] = a;
    }
    Shuffle_Range(gAlbumOrder, 0, albums);
    if (chosen != kNone)
        for (j = 0; j < albums; j++)
            if (gAlbumOrder[j] == Library_SongAlbum(chosen))
            {
                gAlbumOrder[j] = gAlbumOrder[0];
                gAlbumOrder[0] = Library_SongAlbum(chosen);
                break;
            }

    for (j = 0; j < albums; j++)
    {
        k = out;
        for (i = 0; i < n; i++)
        {
            if (Library_SongAlbum(songs[i]) != gAlbumOrder[j])
                continue;
            /* Insert in track order (DECISION D13, as an album plays). */
            s = songs[i];
            for (a = out; a > k && Library_SongTrack(gShuffleOut[a - 1])
                                   > Library_SongTrack(s); a--)
                gShuffleOut[a] = gShuffleOut[a - 1];
            gShuffleOut[a] = s;
            out++;
        }
    }
    for (i = 0; i < n; i++)
    {
        songs[i] = gShuffleOut[i];
        if (songs[i] == chosen)
            start = i;
    }
    return start;
}

short Shuffle_Order(short *songs, short n, short start, Boolean songRow,
                    short mode)
{
    short t;

    if (n <= 1 || mode == kShuffleOff
        || Library_SongIsAudiobook(songs[start]))
        return start;
    if (mode == kShuffleAlbums)
        return ShuffleAlbums(songs, n, songRow ? songs[start] : kNone);

    if (!songRow)
    {
        Shuffle_Range(songs, 0, n);
        return 0;
    }
    t = songs[0];
    songs[0] = songs[start];
    songs[start] = t;
    Shuffle_Range(songs, 1, n);
    return 0;
}
