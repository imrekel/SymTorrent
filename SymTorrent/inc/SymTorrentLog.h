#ifndef SYMTORRENTLOG_H_
#define SYMTORRENTLOG_H_

#include "SymTorrentLogConf.h"

#include "KiLogger.h"
#include "KiLogManager.h"

#define STLOG CKiLogManager::Instance()->GetLoggerL(TUid::Uid(SYMTORRENT_ENGINE_UID))
#define LOG CKiLogManager::Instance()->GetLoggerL(TUid::Uid(SYMTORRENT_ENGINE_UID))

#endif /*SYMTORRENTLOG_H_*/
