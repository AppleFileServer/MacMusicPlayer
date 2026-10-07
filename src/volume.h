#ifndef VOLUME_H
#define VOLUME_H

#include <MacTypes.h>

/* Stage 7b (N1, N2): the Mac Plus speaker volume, 0-7: the system volume,
   changed directly (as the Sound Manager's GetSoundVol/SetSoundVol do;
   Retro68 has no glue for them). PRAM is not changed. */
#define kVolumeMax  7

short Volume_Get(void);
void Volume_Set(short level);

#endif
