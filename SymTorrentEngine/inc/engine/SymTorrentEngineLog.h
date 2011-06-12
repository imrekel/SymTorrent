#ifndef SYMTORRENTENGINELOG_H_
#define SYMTORRENTENGINELOG_H_

#include "SymTorrentEngineLogConf.h"

#include "KiLogger.h"
#include "KiLogManager.h"
#include "STDefs.h"

#define STLOG CKiLogManager::Instance()->GetLoggerL(TUid::Uid(SYMTORRENT_ENGINE_UID))
#define LOG CKiLogManager::Instance()->GetLoggerL(TUid::Uid(SYMTORRENT_ENGINE_UID))

#endif /*SYMTORRENTENGINELOG_H_*/
