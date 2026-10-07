#ifndef SONGDIALOG_H
#define SONGDIALOG_H

#include <MacTypes.h>

#include "song_format.h"

/* The song details window (DLOG/DITL 131), shared by File > Import
   Song... and Edit > Edit Song... (Stage 8c, L3): everything that can be
   set on import can be revised later. */

typedef struct
{
    Str63 text[kSongTagCount];      /* title, artist, album, genre, composer */
    short track;
    Boolean audiobook;
} SongDetails;

/* Shows the window filled with *details, with fileName and the song's
   length in seconds at the top and okTitle on the default button
   ("Import" or "Save"). Returns true with the new details if that button
   is clicked (after the checks: 63 characters a field, a title, a track
   of 0-9999), false on Cancel. */
Boolean SongDialog_Run(ConstStringPtr okTitle, ConstStringPtr fileName,
                       long seconds, SongDetails *details);

#endif
