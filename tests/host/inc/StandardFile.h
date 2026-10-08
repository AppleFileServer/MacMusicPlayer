#pragma once
#include "MacTypes.h"
#include "Dialogs.h"
#include "Files.h"
enum { putDlgID = -3999, getDlgID = -4000, putSave = 1, putCancel = 2,
       putName = 10 };
typedef struct { Boolean good, copy; OSType fType; short vRefNum, version;
                 Str63 fName; } SFReply;
typedef OSType SFTypeList[4];
typedef pascal short (*DlgHookProcPtr)(short item, DialogPtr d);
typedef pascal Boolean (*FileFilterProcPtr)(CInfoPBPtr pb);
void SFPPutFile(Point where, ConstStringPtr prompt, ConstStringPtr origName,
                DlgHookProcPtr hook, SFReply *reply, short dlgID,
                ModalFilterProcPtr filter);
void SFPGetFile(Point where, ConstStringPtr prompt, FileFilterProcPtr fileFilter,
                short numTypes, const OSType *typeList, DlgHookProcPtr hook,
                SFReply *reply, short dlgID, ModalFilterProcPtr filter);
short LMGetSFSaveDisk(void);
long LMGetCurDirStore(void);
