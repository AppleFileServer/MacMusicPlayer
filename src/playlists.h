#ifndef PLAYLISTS_H
#define PLAYLISTS_H

#include <MacTypes.h>
#include <Menus.h>

/* The Playlists menu (MENU 131): "New Playlist...", "Rename
   Playlist..." (Stage 8e-4), "Delete Playlist..." (Stage 8e-3), "Remove
   from Playlist" (Stage 8d-4, L7), then, below a divider, one "Add to
   <name>" item per playlist (DECISION P6). Songs are added and removed only from
   here, never from the iPod screen. */

/* Rebuilds the playlist items and sets what is enabled. Call before
   MenuSelect/MenuKey. hasLibrary: a library is set. */
void Playlists_AdjustMenu(MenuHandle menu, Boolean hasLibrary);

/* Handles a choice from the Playlists menu. Returns true if the library's
   playlists changed (the caller then refreshes and redraws the iPod). */
Boolean Playlists_HandleMenu(short item);

#endif
