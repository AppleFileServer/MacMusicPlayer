#ifndef LIBSETUP_H
#define LIBSETUP_H

#include <MacTypes.h>

/* Startup: finds the remembered iPod Library (asking for one if there is
   none, or it cannot be found) and scans it. Never fails: without a
   library the iPod shows empty lists. */
void LibSetup_Startup(void);

/* File > Choose Library...: asks for a library and rescans. Returns true
   if the library changed (the UI must then be reset). */
Boolean LibSetup_ChooseLibrary(void);

#endif
