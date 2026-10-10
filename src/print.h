#ifndef PRINT_H
#define PRINT_H

#include <MacTypes.h>

/* 1.1 (docs/plans/release-1.1.md, Part C): File > Page Setup... and
   File > Print Library..., through the Printing Manager exactly as
   Inside Macintosh describes it, to whichever printer is chosen in the
   Chooser. This file is the "Printing" code segment (N11-24, the segment
   map in resources/segments.map): the caller unloads it after each call
   (UnloadSeg), so it takes no memory when nothing is being printed. */

void Print_PageSetup(void);

/* Asks which lists to print (Songs, Albums, Artists), then prints them.
   The caller pauses the music first (N11-17). */
void Print_Library(void);

#endif
