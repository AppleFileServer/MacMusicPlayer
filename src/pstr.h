#ifndef PSTR_H
#define PSTR_H

#include <MacTypes.h>

/* A Pascal string literal typed as the Toolbox expects (unsigned).
   P("Hello") = "\pHello"; adjacent literals may be joined: the compiler
   computes the length byte over the whole string. */
#define P(s)    ((ConstStringPtr)("\p" s))

#endif
