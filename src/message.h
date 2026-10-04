#ifndef MESSAGE_H
#define MESSAGE_H

#include <MacTypes.h>
#include <Dialogs.h>

/* Appends Pascal string s to dest (a Str255), truncating at 255. */
void PStrAppend(Str255 dest, ConstStringPtr s);

/* Shows text in the general message alert (ALRT 130); a non-zero err is
   added as " (error n)". */
void Message_Show(ConstStringPtr text, OSErr err);

/* Puts up a window showing text ("Importing...") with the watch cursor,
   for work that keeps the user waiting; Message_EndWorking removes it.
   Returns nil if the window could not be made (the work goes ahead). */
DialogPtr Message_BeginWorking(ConstStringPtr text);
void Message_EndWorking(DialogPtr window);

#endif
