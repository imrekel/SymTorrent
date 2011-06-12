#ifndef KINETWORKLOG_H_
#define KINETWORKLOG_H_

#include "KiNetworkLogConf.h"

#include "KiLogManager.h"
#include "KiLogger.h"
#include "STDefs.h"

#define STLOG CKiLogManager::Instance()->GetLoggerL(TUid::Uid(SYMTORRENT_ENGINE_UID))

#define LOG CKiLogManager::Instance()->GetLoggerL(TUid::Uid(SYMTORRENT_ENGINE_UID))

#endif /*KINETWORKLOG_H_*/
