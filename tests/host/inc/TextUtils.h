#pragma once
#include "MacTypes.h"
Boolean EqualString(ConstStringPtr a, ConstStringPtr b, Boolean caseSens,
                    Boolean diacSens);
void NumToString(long n, StringPtr s);
void StringToNum(ConstStringPtr s, long *n);
