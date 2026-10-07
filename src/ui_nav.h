#ifndef UI_NAV_H
#define UI_NAV_H

#include <MacTypes.h>

#include "lcd_draw.h"

/* Abstract iPod input events (Stage 1 subset). */
typedef enum
{
    kEvtNone = 0,
    kEvtWheelUp,        /* counter-clockwise: highlight moves up */
    kEvtWheelDown,      /* clockwise: highlight moves down */
    kEvtSelect,
    kEvtMenu,
    kEvtPlayPause,
    kEvtNext,
    kEvtPrevious
} UIEvent;

void UI_Init(void);

/* Returns true if the LCD needs redrawing. */
Boolean UI_HandleEvent(UIEvent event);

/* Stage 7: the Now Playing screen's current mode (for tests). */
NowPlayingMode UI_NowPlayingMode(void);

/* Saves settings changed on the iPod that are still pending (the volume,
   7b; a rating, 7d); call before quitting. */
void UI_SaveSettings(void);

/* Stage 7c: the wheel's step size now (finer in the scrubber). */
short UI_WheelStepDegrees(void);

/* Timed UI work that must also happen while the wheel is held (the
   scrubber's seek once turning stops); UI_Tick does it too. True if the
   screen needs redrawing. */
Boolean UI_Idle(void);

/* True once after a rating could not be written to its song file. */
Boolean UI_TakeRatingError(void);

/* Draws the current screen into the offscreen LCD. */
void UI_Render(void);

/* The song highlighted on the iPod, or kNone if the highlighted row is not
   a song (DECISION P7: only song rows can be added to playlists). */
short UI_SelectedSong(void);

/* Rebuilds every screen on the navigation stack from the library (after a
   playlist change), keeping each highlight on the same item where it
   still exists. Does not draw. */
void UI_Refresh(void);

/* Stage 8c: the song Edit > Edit Song... acts on: the song playing on Now
   Playing (L2), else the highlighted song row; kNone if neither. */
short UI_SongToEdit(void);

/* Stage 8c (L4): call Begin before a song's details change in the
   library and End after, to stay on the same screens. Each screen is
   found again by the names of its choices: a list whose album, artist,
   genre or composer has no songs left is closed (its parent shows), and
   highlights follow their items, else stay at the same row. Now Playing
   stays. Does not draw. */
void UI_BeginLibraryEdit(void);
void UI_EndLibraryEdit(void);

/* Advances playback; returns true if the LCD needs redrawing (time or
   song changed on Now Playing, or playback stopped). Call often. */
Boolean UI_Tick(void);

#endif
