#ifndef MESSAGE_H
#define MESSAGE_H

#include <MacTypes.h>

/* Appends Pascal string s to dest (a Str255), truncating at 255. */
void PStrAppend(Str255 dest, ConstStringPtr s);

/* Shows text in the general message alert (ALRT 130); a non-zero err is
   added as " (error n)". */
void Message_Show(ConstStringPtr text, OSErr err);

#endif
