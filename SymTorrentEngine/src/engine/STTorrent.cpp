/*****************************************************************************
 * Copyright (C) 2006-2008 Imre Kelényi
 *-------------------------------------------------------------------
 * This file is part of SymTorrentEngine
 *
 * SymTorrent is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * SymTorrent is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Symella; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *****************************************************************************/

/* ============================================================================ 
 *  Name     : CSTTorrent from STTorrent.cpp
 *  Part of  : SymTorrent Engine
 *  Created  : 07.02.2006 by Imre Kelényi
 * ============================================================================
 */

// INCLUDE FILES 
#include "STTorrent.h"
#include "STTorrentManager.h"
#include "STGlobal.h"
#include "STBencode.h"
#include "STPreferences.h"
#include "STFile.h"
#include "STTrackerConnection.h"
#include "STPeer.h"
#include "STPiece.h"
#include "STBitField.h"
#include "STFileManager.h"
#include "STAnnounceList.h"
#include "SymTorrentEngineLog.h"
//#include "STPieceAccess.h"
#include <f32file.h>
#include <in_sock.h>
#include <e32math.h>
#include <utf.h>
#include "hash.h"
#include "MeasurementLog.h"
#include "FailedPieceCollector.h"

#ifdef USE_DHT
#include "BitTorrentDHT.h"
#include "CompactConverters.h"
#include "Key.h"

using namespace NKademlia;
#endif

const TInt KMaxTrackerFailures = 3;
const TInt KMaxStoredPeers = 200;
const TInt KMaxStoredLocalPeers = 20;
const TInt KDownloadingPiecesCheckInterval = 10;
const TInt KPeerReconnectCheckInterval = 30;

_LIT8(KLitBitfield, "bitfield");
_LIT8(KLitToDownloadBitfield, "to-dl-bitfield");
_LIT8(KLitPath, "path");
_LIT8(KLitPieceStatus, "piece-status");
_LIT8(KLitPeers, "peers");

// ================= MEMBER FUNCTIONS =======================

CSTTorrent::CSTTorrent(CSTTorrentManager* aTorrentMgr) 
 : iValid(EFalse),
   iTorrentMgr(aTorrentMgr),   
   iStatusInfo(ENotSpecified),
   iClosingState(ETorrentNotClosing),
   iStatusInfoDelay(-1)
{
}

 
void CSTTorrent::ConstructL()
{
	#ifdef LOG_TO_FILE
	iLog = LOGMGR->GetLoggerL(TUid::Uid(SYMTORRENT_ENGINE_UID));
	#endif
		
	iFileManager = CSTFileManager::NewL(*this, iTorrentMgr->Fs());
	
	iTrackerManager = new (ELeave) CTrackerManager(*this);
	iTrackerManager->ConstructL();
	iTrackerManager->SetObserver(this);
	
	//iPieceAccess = CSTPieceAccess::NewL(*iFileManager);
	
	TTime now;
	now.HomeTime();
	iRandomSeed = now.Int64();
	
	//iAnnounceList = new (ELeave) CSTAnnounceList;
	
	iMeasurementLog = CMeasurementLog::NewL();
}


CSTTorrent::~CSTTorrent() 
{
	delete iMeasurementLog;
	
	delete iTrackerManager;
	delete iHashChecker;
	iDisconnectedPeers.Reset();
	delete iName;
	delete iPath;
	delete iComment;
	delete iCreatedBy;
	delete iStatusText;
	iFiles.ResetAndDestroy();
	iPeers.ResetAndDestroy();
	iPieces.ResetAndDestroy();
	delete iBitField;
	delete iToDownloadBitField;
	delete iLocalDownloadBitField;
	delete iLocalNetBitField;
	iDownloadingPieces.Reset();
	//delete iPieceAccess;
	delete iFileManager;
	delete iSavedTorrent;
	delete iFailReason;
}

EXPORT_C TInt CSTTorrent::LoadL(const RFile& aTorrentFile)
{
	TInt fileSize;
	User::LeaveIfError(aTorrentFile.Size(fileSize));
	HBufC8* torrentBuf = HBufC8::NewLC(fileSize);
	TPtr8 torrentBufPtr(torrentBuf->Des());
	User::LeaveIfError(aTorrentFile.Read(torrentBufPtr));
	
	CSTBencode* item = CSTBencode::ParseL(*torrentBuf);
	
	if (item)
	{
	    CleanupStack::PushL(item);
	    item->ToLogL();
	    // iLog->WriteLineL(_L("*/*/*/*/*/*/**/*/*/*/"));
	    // HBufC8* bencoded = item->BencodeL();
	    // iLog->WriteLineL(*bencoded);
	    // delete bencoded;
	    // iLog->WriteLineL(_L("*/*/*/*/*/*/**/*/*/*/"));
	    TInt result = ReadFromBencodedDataL(item);
	    
	    CleanupStack::PopAndDestroy(); // item	    
	    
	    if (result == KErrNone)
	    {
	    	// creating .storrent
			HBufC* sTorrentName = 
				HBufC::NewLC(iName->Length() + Preferences()->DownloadPath().Length() + 30);
				
			TPtr stPtr(sTorrentName->Des());	
			stPtr.Append(Preferences()->DownloadPath());
			stPtr.Append(_L("_"));
			if (iName)
				stPtr.Append(*iName);
			else
			{
				TBuf<20> infoHash16;
				infoHash16.Copy(iInfoHash);
				stPtr.Append(infoHash16);
			}
			stPtr.Append(_L(".storrent"));
			
			iTorrentMgr->Fs().MkDirAll(*sTorrentName);
			
			RFile stFile;
			TInt result = 
				stFile.Replace(iTorrentMgr->Fs(), *sTorrentName, EFileWrite|EFileShareExclusive);
				
			if (result == KErrNone)
			{
				CleanupClosePushL(stFile);
				
				TBuf8<10> lengthBuf;
				lengthBuf.Num(torrentBuf->Length());
				while (lengthBuf.Length() < 10) // fixed 10 decimals length
					lengthBuf.Insert(0, _L8("0"));
				stFile.Write(lengthBuf);
				stFile.Write(*torrentBuf);
				
				CleanupStack::PopAndDestroy(); // stFile
				CleanupStack::Pop(); // sTorrentName
				
				iSavedTorrent = sTorrentName;
			}
			else				
				CleanupStack::PopAndDestroy(); // sTorrentName    			
	    	
			CalculateFileFragmentsL();
	    	iValid = ETrue;
	    }
	    
	    CleanupStack::PopAndDestroy(); // torrentBuf
	    
	    return result;
	}
	else
	{
		CleanupStack::PopAndDestroy(); // torrentBuf
		
		LWRITELN(iLog, _L("Parsing failed."));
	    return KErrParsingFailed;
	}
}

EXPORT_C TInt CSTTorrent::LoadL(const TDesC& aTorrentFileName)
{
	RFile torrentFile;
	TInt result = torrentFile.Open(iTorrentMgr->Fs(), aTorrentFileName, 
		EFileRead|EFileShareReadersOnly);

	if (result != KErrNone)
	{
		LWRITELN(iLog, _L("Failed to open torrent file"));
		return KErrGeneral;
	}

	CleanupClosePushL(torrentFile);
	result = LoadL(torrentFile);
	CleanupStack::PopAndDestroy(); // torrentFile
	return result;
}

EXPORT_C TInt CSTTorrent::LoadSTorrentL(const TDesC& aSTorrentFileName)
{
	TInt openResult = KErrGeneral;
	RFile stFile;
	TInt res = 
		stFile.Open(iTorrentMgr->Fs(), aSTorrentFileName, EFileRead|EFileShareReadersOnly);
		
	if (res == KErrNone)
	{
		CleanupClosePushL(stFile);
		while (1)
		{
			TBuf8<10> lengthBuf;
			stFile.Read(lengthBuf);
			TInt torrentLength;
			TLex8 lex(lengthBuf);
			if (lex.Val(torrentLength) != KErrNone)
				break;
			
			HBufC8* torrentBuf = HBufC8::NewLC(torrentLength);
			TPtr8 torrentPtr(torrentBuf->Des());
			if (stFile.Read(torrentPtr) != KErrNone)
			{
				CleanupStack::PopAndDestroy(); // torrentBuf
				break;
			}
			
			CSTBencode* torrentBencode = CSTBencode::ParseL(*torrentBuf);
			CleanupStack::PopAndDestroy();	// torrentBuf
			if (!torrentBencode)
				break;
			
			CleanupStack::PushL(torrentBencode);
			
			
			lengthBuf.SetLength(0);
			stFile.Read(torrentLength + 10, lengthBuf, 10);
			TInt stateLength;
			TLex8 lex2(lengthBuf);
			if (lex2.Val(stateLength) != KErrNone)
			{
				CleanupStack::PopAndDestroy(); // torrentBencode
				break;
			}
			
			HBufC8* stateBuf = HBufC8::NewLC(stateLength);
			TPtr8 statePtr(stateBuf->Des());
			if (stFile.Read(statePtr) != KErrNone)
			{
				CleanupStack::PopAndDestroy(); // stateBuf
				CleanupStack::PopAndDestroy(); // torrentBencode
				break;
			}
			
			
			CSTBencode* stateBencode = CSTBencode::ParseL(*stateBuf);
			if (stateBencode)
			{
				CleanupStack::PushL(stateBencode);
				
				if (stateBencode->Type() == EBencodedDictionary)
				{
					CSTBencodedDictionary* state = 
						static_cast<CSTBencodedDictionary*>(stateBencode);
						
					CSTBencode* bencode = state->EntryValue(KLitPath);
					if (bencode && bencode->Type() == EBencodedString)
					{
						CSTBencodedString* path = 
							static_cast<CSTBencodedString*>(bencode);
						
						HBufC* path16 = HBufC::NewLC(path->Value().Length());
						TPtr path16Ptr(path16->Des());
						path16Ptr.Copy(path->Value());
						TInt result = ReadFromBencodedDataL(torrentBencode, *path16);
						CleanupStack::PopAndDestroy(); // path16
						
						if (result == KErrNone)
						{
							CalculateFileFragmentsL();						
							
							// parse bitfield
							bencode = state->EntryValue(KLitBitfield);
							if (bencode && bencode->Type() == EBencodedString)
								iBitField->SetL(static_cast<CSTBencodedString*>(bencode)->Value());
							
							// parse toDownloadBitfield
							bencode = state->EntryValue(KLitToDownloadBitfield);
							if (bencode && bencode->Type() == EBencodedString)
								iToDownloadBitField->SetL(static_cast<CSTBencodedString*>(bencode)->Value());
							
							// detect skipped files
							for (TInt i=0; i<iPieces.Count(); i++)
							{
								if (!iPieces[i]->IsDownloaded() && !iToDownloadBitField->IsBitSet(i))
								{
									for (TInt j=0; j<iFiles.Count(); j++)
									{
										if (iPieces[i]->HasFile(iFiles[j]))
											iFiles[j]->SetSkipped();
									}
									
								}
							}
							
							// setting pieces to downloaded state according to the bitfield
							TInt pieceCount = iPieces.Count();
							for (TInt i=0; i<pieceCount; i++)
							{
								if (iBitField->IsBitSet(i))
								{
								//	if (iPieces[i]->CheckHashL())
								//	{
										PieceDownloadedL(iPieces[i]);
										iPieces[i]->SetDownloaded();
										UpdateBytesDownloadedL(iPieces[i]->TotalSize(), EFalse);
								//	}
								//	else
								//		iLog->WriteLineL(_L("[Torrent] Loading piece failed (hash failed)"));
								}		
							}
							
							// parse piece status
							bencode = state->EntryValue(KLitPieceStatus);
							if (bencode && bencode->Type() == EBencodedList)
							{
								CSTBencodedList* pieceStatus = static_cast<CSTBencodedList*>(bencode);
								
								if (pieceStatus->Count() == iPieces.Count())
								{
									for (TInt i=0; i<iPieces.Count(); i++)
									{
										TInt downloadedSize = 
											static_cast<CSTBencodedInteger*>(pieceStatus->Item(i))->iValue;
											
										if ((downloadedSize > 0) && (downloadedSize < iPieces[i]->TotalSize()))
										{
											iPieces[i]->SetDownloadedSize(downloadedSize);
											UpdateBytesDownloadedL(downloadedSize, EFalse);
										}
									}
								}
							}
							
							// peers
							bencode = state->EntryValue(KLitPeers);
							if (bencode && bencode->Type() == EBencodedList)
							{
								CSTBencodedList* peerList = static_cast<CSTBencodedList*>(bencode);
								
								for (TInt i=0; i<peerList->Count(); i++)
								{
									CSTBencodedList* peerInfo =
										static_cast<CSTBencodedList*>(peerList->Item(i));
									
									TInt64 addr = 
										static_cast<CSTBencodedInteger*>(peerInfo->Item(0))->iValue;
									TInt64 port = 
										static_cast<CSTBencodedInteger*>(peerInfo->Item(1))->iValue;
									
									TInetAddr inetAddr((TUint32)addr, (TUint)port);
									AddPeerL(inetAddr);
								}
							}
							
							iBytesPerSecond = 0;
														
							iValid = ETrue;
							openResult = KErrNone;
							
							iSavedTorrent = aSTorrentFileName.AllocL();
						}	
					}
				}
				
				CleanupStack::PopAndDestroy(); // stateBencode
			}
			
			CleanupStack::PopAndDestroy(); // stateBuf
			CleanupStack::PopAndDestroy(); // torrentBencode			
			break;
		}
		CleanupStack::PopAndDestroy(); // stFile
	}
	
	if (openResult == KErrNone)
	{
		CalculateBytesToDownload();	
		CheckDownloadCompleteL();
	}
	
	return openResult;
}

EXPORT_C void CSTTorrent::StartL()
{
	HLWRITELN(iLog, _L("[Torrent] StartL begin"));
	if (!iActive && iValid)
	{
		iActive = ETrue;
		iEndGame = EFalse; // start with endgame turned off
		
		iEllapsedTime = 0;
		
		for (TInt i=0; i<iPeers.Count(); i++)
			iPeers[i]->SetReconnectTime(0);	
		
		iFailed = EFalse;
		iTrackerManager->AnnounceL(ETrackerEventStarted);
	}
	HLWRITELN(iLog, _L("[Torrent] StartL end"));
}

#ifdef USE_DHT
void CSTTorrent::DHTAnnounceL()
{
	if ((!iDHTAnnouncing) && (iTorrentMgr->DHT()->IsRunning()))
	{
		iLog->WriteLineL(_L("[Torrent] DHT announcing"));
		
		iDHTAnnouncing = ETrue;
		
		CKey* key = CKey::NewL(TBinDataPtr(iInfoHash.Ptr(), 20));
		CleanupStack::PushL(key);
		
		TBuf8<6> value;
		CreateCompactAddress(0, iTorrentMgr->DHT()->Port(), value);
		
		iTorrentMgr->DHT()->PutGetValueL(*key, TBinDataPtr(value.Ptr(), 6), this);
		
		CleanupStack::PopAndDestroy(); // key
	}	
}
#endif

EXPORT_C void CSTTorrent::StopL()
{
	HLWRITELN(iLog, _L("[Torrent] StopL begin"));
	
	if (iActive)
	{
		for (TInt i=0; i<iPeers.Count(); i++)
		{
			iPeers[i]->DisconnectL();
		}
		
		iAvarageBytesPerSecond = 0;
		iBytesPerSecond = 0;
		
		if (!iFailed && iTorrentMgr->NetworkManager()->IsNetworkConnectionStarted(0))
			iTrackerManager->AnnounceL(ETrackerEventStopped);
		
		iEndGame = EFalse;
		iActive = EFalse;
		
		iTorrentMgr->NotifyTorrentObserverL(this, ESTEventTorrentStopped);
	}
	
	HLWRITELN(iLog, _L("[Torrent] StopL end"));
}


EXPORT_C void CSTTorrent::CloseL(TBool aDeleteIncompleteFiles)
{
	HLWRITELN(iLog, _L("[Torrent] CloseL begin"));
	
	if (iActive)
	{		
		StopL();
		iClosingState = ETorrentClosed;
	}
	else
		iClosingState = ETorrentClosed;
	
	if (aDeleteIncompleteFiles)
	{
		for (TInt i=0; i<iFiles.Count(); i++)
		{
			if (!iFiles[i]->IsDownloaded())
				iTorrentMgr->Fs().Delete(iFiles[i]->Path());
		}
	}
	
	SetStatusInfoL(ETorrentRemoving);
	
	HLWRITELN(iLog, _L("[Torrent] CloseL end"));
}

EXPORT_C void CSTTorrent::SaveStateL()
{
	HLWRITELN(iLog, _L("[Torrent] SaveStateL begin"));
	if (iSavedTorrent)
	{
		RFile stFile;
		TInt res =
			stFile.Open(iTorrentMgr->Fs(), *iSavedTorrent, EFileWrite|EFileShareExclusive);
			
		if (res == KErrNone)
		{
			CleanupClosePushL(stFile);
			while(1)
			{
				TBuf8<10> lengthBuf;
				if (stFile.Read(lengthBuf) != KErrNone) 
					break;
				
				TLex8 lex(lengthBuf);
				TInt length;
				if (lex.Val(length) != KErrNone)
					break;
				
				if (stFile.SetSize(length + 10) != KErrNone)
					break;
				
				TInt endPos = length + 10;
				if (stFile.Seek(ESeekStart, endPos) != KErrNone)
					break;
				
				CSTBencodedDictionary* state = new (ELeave) CSTBencodedDictionary;
				CleanupStack::PushL(state);
				
				{
					CSTBencodedString* bitfield = 
						CSTBencodedString::NewL(iBitField->Data());
					state->AddEntryL(KLitBitfield, bitfield);
				}
				
				{
					CSTBencodedString* toDownloadBitfield = 
						CSTBencodedString::NewL(iToDownloadBitField->Data());
					state->AddEntryL(KLitToDownloadBitfield, toDownloadBitfield);
				}
				
				// download path
				{					
					HBufC8* encodedPath = HBufC8::NewLC(iPath->Length() * 3);
					TPtr8 encodedPathPtr = encodedPath->Des();
					if (CnvUtfConverter::ConvertFromUnicodeToUtf8(encodedPathPtr, *iPath) != 0)
					{						
						CleanupStack::PopAndDestroy(); // encodedPath
						CleanupStack::PopAndDestroy(); // state
						break;
					}
					
					CSTBencodedString* path = 
						CSTBencodedString::NewL(*encodedPath);
					CleanupStack::PopAndDestroy(); // encodedPath
					state->AddEntryL(KLitPath, path);
				}
				
				// Piece status
				{
					if (!IsComplete())
					{
						CSTBencodedList* pieceStatus = new (ELeave) CSTBencodedList;
						CleanupStack::PushL(pieceStatus);
						
						for (TInt i=0; i<iPieces.Count(); i++)
						{
							CSTBencodedInteger* downloadedSize = NULL;
							
							if (iPieces[i]->IsDownloaded())
								downloadedSize = new CSTBencodedInteger(-1);
							else
								downloadedSize = new CSTBencodedInteger(iPieces[i]->DownloadedSize());
							
							pieceStatus->AppendL(downloadedSize);
						}
						
						CleanupStack::Pop(pieceStatus);
						state->AddEntryL(KLitPieceStatus, pieceStatus);
					}
				}
				
				// peers
				if (!IsComplete())
				{
					CSTBencodedList* peerList = iPeers.CreateBencodeL(50);
					if (peerList)
						state->AddEntryL(KLitPeers, peerList);
				}
			
				HBufC8* stateBencoded = state->BencodeL();
				TBuf8<10> stateLengthBuf;
				stateLengthBuf.Num(stateBencoded->Length());
				while (stateLengthBuf.Length() < 10) // fixed 10 decimals length
					stateLengthBuf.Insert(0, _L8("0"));
				stFile.Write(stateLengthBuf);
				stFile.Write(*stateBencoded);
				delete stateBencoded;
				
				CleanupStack::PopAndDestroy(); // state
				
				iTorrentMgr->Preferences()->AddSTorrentL(*iSavedTorrent);
				break;	
			}	
			
			CleanupStack::PopAndDestroy(); // stFile
		}
	}
	
	HLWRITELN(iLog, _L("[Torrent] SaveStateL end"));
}

TInt CSTTorrent::ReadFromBencodedDataL(CSTBencode* aBencodedTorrent, const TDesC& aPath)
{
	if (aBencodedTorrent->Type() != EBencodedDictionary)
		return KErrParsingFailed;

	CSTBencodedDictionary* torrent =
		static_cast<CSTBencodedDictionary*>(aBencodedTorrent);
							
	CSTBencode* value = NULL;
	
	// announce
	{
		
		_LIT8(KLitAnnounce, "announce");
		value = torrent->EntryValue(KLitAnnounce);
			
		if (!value || (value->Type() != EBencodedString))
			return KErrParsingFailed;
	
		iTrackerManager->AddTrackerL(
			static_cast<CSTBencodedString*>(value)->Value());			
	}
	
	// announce-list
	{
		_LIT8(KLitAnnounceList, "announce-list");
		value = torrent->EntryValue(KLitAnnounceList);
			
		if (value && (value->Type() == EBencodedList))
		{		
			CSTBencodedList* announceList =
				static_cast<CSTBencodedList*>(value);
				
			for (TInt i=0; i<announceList->Count(); i++)
			{
				if (announceList->Item(i)->Type() == EBencodedList)
				{
					CSTBencodedList* tier = 
						static_cast<CSTBencodedList*>(announceList->Item(i));
						
					for (TInt j=0; j<tier->Count(); j++)
					{
						if (tier->Item(j)->Type() == EBencodedString)
						{
							iTrackerManager->AddTrackerL(
								static_cast<CSTBencodedString*>(tier->Item(j))->Value());
						}						
					}
					
				}
			}
		}
		
		// there must be at least one address in the tracker manager (from the "announce"
		// or the "announce-list" fields)
		if (iTrackerManager->TrackerCount() == 0)
			return KErrParsingFailed;			
	}

#ifdef USE_DHT
	// nodes (for the DHT)
	{
		_LIT8(KLitNodes, "nodes");
		value = torrent->EntryValue(KLitNodes);
			
		if (value && (value->Type() == EBencodedList))
		{
			CSTBencodedList* nodesList =
				static_cast<CSTBencodedList*>(value);
			
			for (TInt i=0; i<nodesList->Count(); i++)
			{
				if (nodesList->Item(i)->Type() == EBencodedList)
				{
					CSTBencodedList* nodeInfo = 
						static_cast<CSTBencodedList*>(nodesList->Item(i));
						
					if ((nodeInfo->Count() == 2) &&
						(nodeInfo->Item(0)->Type() == EBencodedString) &&
						(nodeInfo->Item(1)->Type() == EBencodedInteger))
					{
						int port = static_cast<CSTBencodedInteger*>(nodeInfo->Item(1))->iValue;
						
						TInetAddr address;
						TPtrC8 addrBuf8 =
							static_cast<CSTBencodedString*>(nodeInfo->Item(0))->Value();
							
						HBufC* addrBuf16 = HBufC::NewLC(addrBuf8.Length());
						TPtr addrBuf16Ptr(addrBuf16->Des());
						addrBuf16Ptr.Copy(addrBuf8);
						if (address.Input(*addrBuf16) == KErrNone)
						{
							iTorrentMgr->DHT()->AddNodeL(TUDPAddress(address.Address(), port));
						}
						
						CleanupStack::PopAndDestroy(); // addrBuf16
					}
				}
			}
		}
	}
#endif
	
	// info
	{
		_LIT8(KLitInfo, "info");
		value = torrent->EntryValue(KLitInfo);
			
		if (!value || (value->Type() != EBencodedDictionary))
			return KErrParsingFailed;
	
		CSTBencodedDictionary* info =
			static_cast<CSTBencodedDictionary*>(value);
			
		HBufC8* infoBencoded = info->BencodeL();
		CleanupStack::PushL(infoBencoded);
		CSHA1* sha1 = CSHA1::NewL();
		CleanupStack::PushL(sha1);
		iInfoHash.Copy(sha1->Hash(*infoBencoded));
		CleanupStack::PopAndDestroy(); // sha1
		CleanupStack::PopAndDestroy(); // infoBencoded				

		// info/piece length
		{
			_LIT8(KLitPieceLength, "piece length");
			value = info->EntryValue(KLitPieceLength);
				
			if (!value || (value->Type() != EBencodedInteger))
				return KErrParsingFailed;
			
			iPieceLength = static_cast<CSTBencodedInteger*>(value)->iValue;
		}
		
		// file / files
		{
			_LIT8(KLitName, "name");
			_LIT8(KLitLength, "length");
			
			_LIT8(KLitFiles, "files");
			value = info->EntryValue(KLitFiles);
				
			if (value) // multi-file torrent
			{
				if (value->Type() != EBencodedList)
					return KErrParsingFailed;
				
				CSTBencodedList* files = 
					static_cast<CSTBencodedList*>(value);
				
				// info/name
				{					
					value = info->EntryValue(KLitName);
						
					if (!value || (value->Type() != EBencodedString))
						return KErrParsingFailed;
					
					TPtrC8 name8 = static_cast<CSTBencodedString*>(value)->Value();			
					iName = HBufC::NewL(name8.Length());
					TPtr namePtr(iName->Des());
					namePtr.Copy(name8);
					
					if (aPath == KNullDesC)
					{
						iPath = HBufC::NewL(iName->Length() + Preferences()->DownloadPath().Length() + 1);
						TPtr pathPtr(iPath->Des());
						pathPtr.Copy(Preferences()->DownloadPath());
						pathPtr.Append(*iName);
						pathPtr.Append(_L("\\"));
					}
					else
						iPath = aPath.AllocL();

				}
				
				_LIT8(KLitPath, "path");
				
				for (TInt i=0; i < files->Count(); i++)
				{
					CSTBencode* value = files->Item(i);
					if (value->Type() != EBencodedDictionary)
						return KErrParsingFailed;
					
					CSTBencodedDictionary* file = 
						static_cast<CSTBencodedDictionary*>(value); // bencoded dictionary describing the file
						
					value = file->EntryValue(KLitPath);
					if (!value || (value->Type() != EBencodedList))
						return KErrParsingFailed;
					
					CSTBencodedList* path = 
						static_cast<CSTBencodedList*>(value); // bencoded list describing the path of the file
						
					TInt pathLength = 0;
					TInt j = 0;										
					for(j = 0; j<path->Count(); j++) // calculates the full length of the path
					{
						CSTBencode* value = path->Item(j);
						if (value->Type() != EBencodedString)
							return KErrParsingFailed;
						
						pathLength += 
							static_cast<CSTBencodedString*>(value)->Value().Length() + 1;						
					}
					
					HBufC8* pathBuf = HBufC8::NewLC(pathLength);
					TPtr8 pathBufPtr(pathBuf->Des());
					
					for(j = 0; j<path->Count(); j++)
					{
						CSTBencode* value = path->Item(j);
						pathBufPtr.Append(
							static_cast<CSTBencodedString*>(value)->Value());
							
						if (j != (path->Count() - 1))
							pathBufPtr.Append(_L8("\\"));
					}
					
					value = file->EntryValue(KLitLength); // length of the file
					if (!value || (value->Type() != EBencodedInteger))
						return KErrParsingFailed;
					
					TInt64 length = static_cast<CSTBencodedInteger*>(value)->iValue;
					AddFileL(*pathBuf, length);
					
					iTotalBytesLeft += length;
						
					CleanupStack::PopAndDestroy(); // pathBuf
				}
			}
			else // single-file torrent
			{
				TInt64 length = 0;
				
				// info/length
				{					
					value = info->EntryValue(KLitLength);
						
					if (!value || (value->Type() != EBencodedInteger))
						return KErrParsingFailed;
					
					length = static_cast<CSTBencodedInteger*>(value)->iValue;
					
					iTotalBytesLeft = length;					
				}
				
				// info/name
				{					
					value = info->EntryValue(KLitName);
						
					if (!value || (value->Type() != EBencodedString))
						return KErrParsingFailed;
					
					TPtrC8 name8 = static_cast<CSTBencodedString*>(value)->Value();			
					iName = HBufC::NewL(name8.Length());
					TPtr namePtr(iName->Des());
					namePtr.Copy(name8);

					LWRITE(iLog, _L("[Torrent] Name: "));
					LWRITELN(iLog, Name());
					
					if (aPath == KNullDesC)
					{
						iPath = HBufC::NewL(Preferences()->DownloadPath().Length());
						TPtr pathPtr(iPath->Des());
						pathPtr.Copy(Preferences()->DownloadPath());
					}	
					else
						iPath = aPath.AllocL();

					AddFileL(static_cast<CSTBencodedString*>(value)->Value(), length);
				}				
			}
		}

		LWRITE(iLog, _L("[Torrent] Size: "));
		LWRITELN(iLog, Size());
		LWRITE(iLog, _L("[Torrent] Path: "));
		LWRITELN(iLog, Path());

		// info/pieces
		{
			_LIT8(KLitPieces, "pieces");
			value = info->EntryValue(KLitPieces);
			
			if (!value || (value->Type() != EBencodedString))
				return KErrParsingFailed;
			
			TPtrC8 pieces = 
				static_cast<CSTBencodedString*>(value)->Value();
			
			if ((pieces.Length() % 20) != 0)
				return KErrParsingFailed;
			
			TInt64 processedLength = 0;
			TInt pieceCount = pieces.Length() / 20;
			for (TInt i=0; i<pieceCount; i++)
			{
				//iLog->WriteL(_L("Piece: "));
				//iLog->WriteLineL(i);
				TBuf8<20> hash = pieces.Mid(i*20, 20);
				//iLog->WriteLineL(hash);

				CSTPiece* piece = NULL;

				if (i != pieceCount - 1)
				{
					processedLength += iPieceLength;
					piece = new (ELeave) CSTPiece(*this, hash, iPieceLength);
				}
				else
				{
					piece = new (ELeave) CSTPiece(*this, hash, Size() - processedLength);
					LWRITE(iLog, _L("[Torrent] Last piece length: "));
					LWRITELN(iLog, Size() - processedLength);
				}
								
				CleanupStack::PushL(piece);
				//iLog->WriteLineL(_L("Piece created"));
				User::LeaveIfError(iPieces.Append(piece));
				//iLog->WriteLineL(_L("Piece appended"));
				CleanupStack::Pop(); // piece
				//iLog->WriteLineL(_L("Piece popped"));
			}
			
			LWRITE(iLog, _L("[Torrent] Number of pieces: "));
			LWRITELN(iLog, iPieces.Count());
			
			iBitField = new (ELeave) CSTBitField;
			iBitField->ConstructL(iPieces.Count());
			
			iToDownloadBitField = new (ELeave) CSTBitField;
			iToDownloadBitField->ConstructL(iPieces.Count(), ETrue);
			
			iLocalDownloadBitField = new (ELeave) CSTBitField;
			iLocalDownloadBitField->ConstructL(iPieces.Count());
			
			iLocalNetBitField = new (ELeave) CSTBitField;
			iLocalNetBitField->ConstructL(iPieces.Count());
		}
	}
	
	CalculateBytesToDownload();
	
	// comment
	{
		_LIT8(KLitComment, "comment");
		value = torrent->EntryValue(KLitComment);
			
		if (value && (value->Type() == EBencodedString))
			iComment = static_cast<CSTBencodedString*>(value)->Value().AllocL();
	}
	
	// created by
	{
		_LIT8(KLitCreatedBy, "created by");
		value = torrent->EntryValue(KLitCreatedBy);
			
		if (value && (value->Type() != EBencodedString))
			iCreatedBy = static_cast<CSTBencodedString*>(value)->Value().AllocL();
	}
	
	// creation date
	{
		_LIT8(KLitCreationDate, "creation date");
		value = torrent->EntryValue(KLitCreationDate);
			
		if (value && (value->Type() != EBencodedInteger))
		{
			TDateTime epoch(1970, EJanuary, 1, 0, 0, 0, 0); // standard unix epoch format
			iCreationDate = 
				TTime(epoch) + 
				TTimeIntervalSeconds(static_cast<CSTBencodedInteger*>(value)->iValue);
		}
	}
	
	return KErrNone;
}


void CSTTorrent::AddFileL(const TDesC& aRelativePath, TInt64 aSize)
{	
	if (!iPath) User::Panic(_L("TorrentPathNotSet"), 0);
	
	HBufC* filePath  = HBufC::NewLC(iPath->Length() +
									aRelativePath.Length());
									
	TPtr filePathPtr(filePath->Des());
	filePathPtr.Copy(*iPath);	
	filePathPtr.Append(aRelativePath);
					
	CSTFile* file = CSTFile::NewLC(*filePath, aSize);
	User::LeaveIfError(iFiles.Append(file));
	CleanupStack::Pop(); // file
	
	CleanupStack::PopAndDestroy(); // filePath
}


void CSTTorrent::AddFileL(const TDesC8& aRelativePath, TInt64 aSize)
{
	HBufC* path16 = HBufC::NewLC(aRelativePath.Length());
	TPtr path16Ptr(path16->Des());
	path16Ptr.Copy(aRelativePath);
	
	AddFileL(*path16, aSize);
	
	CleanupStack::PopAndDestroy(); // fileName16
}


/*void CSTTorrent::ProcessTrackerResponseL(CSTBencode* aResponse)
{
	HLWRITELN(iLog, _L("[Torrent] ProcessTrackerResponseL begin"));
	
	LWRITELN(iLog, _L("[Announce] Response received"));
	
	CSTTorrentManager* torrentMgr = iTorrentMgr;
	
	TInetAddr localAddress;
	TInt getAddressRes = torrentMgr->NetworkManager()->GetLocalAddress(0, localAddress);
	
	//aResponse->ToLogL();
	
	if (aResponse->Type() != EBencodedDictionary)
	{
		LWRITELN(iLog, _L("[Announce] Invalid response (not a bencoded dictionary)"));
		return;
	}
	
	CSTBencodedDictionary* response = 
		static_cast<CSTBencodedDictionary*>(aResponse);
		
	CSTBencode* value = NULL;
	
	// interval
	{
		_LIT8(KLitInterval, "interval");
		value = response->EntryValue(KLitInterval);
		
		if (value && (value->Type() == EBencodedInteger))
		{
			TInt interval = static_cast<CSTBencodedInteger*>(value)->iValue;
			
			LWRITE(iLog, _L("[Announce] Interval parsed: "));
			LWRITELN(iLog, interval);
			
			if ((interval > KDefaultTrackerRequestInterval) && (interval <= 3600))			
				iTrackerRequestInterval = interval;
			else
				iTrackerRequestInterval = KDefaultTrackerRequestInterval;				
		}
		else
			iTrackerRequestInterval = KDefaultTrackerRequestInterval;	
	}

	// failure reason
	{
		_LIT8(KLitFailureReason, "failure reason");
		value = response->EntryValue(KLitFailureReason);
		
		if (value && (value->Type() == EBencodedString))
		{
			LWRITE(iLog, _L("[Announce] Request failed, reason: "));
			LWRITELN(iLog, static_cast<CSTBencodedString*>(value)->Value());
		}
	}
	
	// complete
	{		
	}
	
	// incomplete
	{		
	}
	
	// peers
	{
		LWRITE(iLog, _L("[Torrent] Local peer id: "));
		LWRITELN(iLog, torrentMgr->PeerId());
		
		_LIT8(KLitIp, "ip");
		_LIT8(KLitPeerId, "peer id");
		_LIT8(KLitPort, "port");
		
		_LIT8(KLitPeers, "peers");
		value = response->EntryValue(KLitPeers);
		
		if (value && (value->Type() == EBencodedList)) // normal tracker response
		{
			CSTBencodedList* peers = 
				static_cast<CSTBencodedList*>(value);
			
			LWRITE(iLog, _L("[Announce] Number of peers received: "));
			LWRITELN(iLog, peers->Count());
				
			for (TInt i=0; i<peers->Count(); i++)
			{
				value = peers->Item(i);
				
				if (value->Type() != EBencodedDictionary)
					return;
				
				CSTBencodedDictionary* bencodedPeer = 
					static_cast<CSTBencodedDictionary*>(value);
					
				// peer id
					value = bencodedPeer->EntryValue(KLitPeerId);
					if (!value || (value->Type() != EBencodedString))
						continue;
					TPtrC8 peerId = static_cast<CSTBencodedString*>(value)->Value();
					
					LWRITE(iLog, _L("[Announce] Processing peer id: "));
					LWRITELN(iLog, peerId);
					if (peerId.Length() != 20)
						continue;
					
					if (peerId.CompareF(torrentMgr->PeerId()) == 0) // got back our own address
					{
						LWRITELN(iLog, _L("[Announce] Got own local address from tracker (peer ID is the same), throwing it away..."));
						continue;
					}
				
				// ip
					value = bencodedPeer->EntryValue(KLitIp);
					if (!value || (value->Type() != EBencodedString))
						continue;
					TPtrC8 ip = static_cast<CSTBencodedString*>(value)->Value();
					
					HBufC* ip16 = HBufC::NewLC(ip.Length());
					TPtr ip16ptr(ip16->Des());
					ip16ptr.Copy(ip);
					
					LWRITE(iLog, _L("[Announce] Peer address received: "));
					LWRITE(iLog, ip);
					LWRITE(iLog, _L(":"));
					
					TInetAddr address;				
					if (address.Input(*ip16) != KErrNone)
					{
						CleanupStack::PopAndDestroy(); // ip16
						continue;						
					}									
					
					CleanupStack::Pop(); // ip16						
				
				// port
					value = bencodedPeer->EntryValue(KLitPort);
					if (!value || (value->Type() != EBencodedInteger))
						continue;
					address.SetPort(static_cast<CSTBencodedInteger*>(value)->iValue);
					
					LWRITE(iLog, static_cast<CSTBencodedInteger*>(value)->iValue);
					
					if (getAddressRes == KErrNone)
					{
						if (address == localAddress)
						{
							LWRITELN(iLog, _L("[Announce] Got own local address from tracker, throwing it away..."));
							continue;
						}						
					}								
					
				CSTPeer* peer = CSTPeer::NewLC(address, peerId, iPieces.Count());
				if (AddPeerL(peer) != KErrNone)
				{
					LWRITELN(iLog, _L(" NOT added"));
					CleanupStack::PopAndDestroy(); // peer
				}				
				else
				{
					LWRITELN(iLog, _L(" ADDED"));
					CleanupStack::Pop(); // peer
				}					
			}
		}
		else

		if (value && (value->Type() == EBencodedString)) // likely a compact response
		{
			CSTBencodedString* peers = 
				static_cast<CSTBencodedString*>(value);

			if ((peers->Value().Length() % 6) == 0)
			{
				TPtrC8 ips = peers->Value();
				for (TInt i = 0; i < ips.Length()/6; i++)
				{
					TInt pos = i * 6;
					TBuf<25> addressBuffer;
					_LIT(KLitIpFormat, "%u.%u.%u.%u");
					addressBuffer.Format(KLitIpFormat, TUint(ips[pos]), TUint(ips[pos+1]), 
						TUint(ips[pos+2]), TUint(ips[pos+3]));

					TInetAddr address;				
					if (address.Input(addressBuffer) != KErrNone)
						continue;

					address.SetPort(TUint((ips[pos+4] << 8) + ips[pos+5]));
					
					TBuf<32> ipBuf;
					address.Output(ipBuf);
					LWRITE(iLog, _L("[Announce] Peer address received: "));
					LWRITE(iLog, ipBuf);
					LWRITE(iLog, _L(":"));
					LWRITE(iLog, address.Port());
					
					if (getAddressRes == KErrNone)
					{
						if (address == localAddress)
						{						
							LWRITELN(iLog, _L(" NOT ADDED (Got own local address from tracker, throwing it away...)"));
							continue;
						}
					}

					CSTPeer* peer = CSTPeer::NewLC(address, iPieces.Count());
					if (AddPeerL(peer) != KErrNone)
					{
						LWRITELN(iLog, _L(" NOT added"));
						CleanupStack::PopAndDestroy(); // peer
					}
						
					else
					{
						LWRITELN(iLog, _L(" ADDED"));
						CleanupStack::Pop(); // peer
					}
						
				}
			}
			else
				LWRITELN(iLog, _L("[Announce] Compact response invalid (length cannot be devided by 6 without remainder)"));
		}
		else
			LWRITELN(iLog, _L("[Announce] No peers list / peers list invalid"));
	}	
	LWRITELN(iLog, _L("[Announce] Tracker response procesed"));
	
	HLWRITELN(iLog, _L("[Torrent] ProcessTrackerResponseL end"));
}*/


void CSTTorrent::OnTimerL()
{
	HLWRITELN(iLog, _L("[Torrent] OnTimerL begin"));
	
	iEllapsedTime++;		
	
	if (iValid)
	{
		if (iStatusInfoDelay > 0)
			iStatusInfoDelay--;
		else if (iStatusInfoDelay == 0)
		{
			SetStatusInfoL(ENotSpecified);
			iStatusInfoDelay = -1;
		}
		
		TInt activeConnectionCount = 0;
		TInt activeLocalConnectionCount = 0;
		
		TInt i;
		for (i=0; i<iPeers.Count(); i++)
		{
			CSTPeer* peer = iPeers[i];
			
			peer->OnTimerL(this, iEllapsedTime);
			
			if (peer->IsDeletable())
			{
				HLWRITELN(iLog, _L("[Torrent] OnTimerL - deleting peer"));
				
				for (TInt j=0; j<PieceCount(); j++)	
					if (peer->BitField()->IsBitSet(j))
						DecreaseNumberOfPeersHavingPiece(j);
					
				delete peer;
				iPeers.Remove(i);
				i--;
				
				HLWRITELN(iLog, _L("[Torrent] OnTimerL - deleting peer done"));				
				continue;
			}
		}
		
		// moving recently disconnected peers to the end of the peer list
		// thus these peers won't be reconnected too soon
		for (i=0; i<iDisconnectedPeers.Count(); i++)
		{
			TInt index = iPeers.Find(iDisconnectedPeers[i]);

			if (index >= 0)
			{
				HLWRITELN(iLog, _L("[Torrent] OnTimerL - Moving disconnected peer"));
				
				iPeers.Remove(index);
				CleanupStack::PushL(iDisconnectedPeers[i]);
				User::LeaveIfError(iPeers.Append(iDisconnectedPeers[i]));
				CleanupStack::Pop(); // aPeer
				
				HLWRITELN(iLog, _L("[Torrent] OnTimerL - Moving disconnected peer done"));
			}	
		}
		iDisconnectedPeers.Reset();		
			
		if (iActive)
		{
			if (!iComplete)
			{
				// check if there is still hope to get some peers
				if (iPeers.Count() == 0)
				{
					TInt announcingTrackerCount = iTrackerManager->GetAnnouncingTrackerCount();
					
					if (iTorrentMgr->NetworkManager()->IsNetworkConnectionStarted(0) && 
						announcingTrackerCount == 0)
					{
						ReportCriticalFaultL(ENoPeersLeft);
					}
					else
					{
						SetStatusInfoL(ETrackerConnecting);
					}
				}
				
				// calculating download speed
				{				
					TReal lastSecondAvarage = iAvarageBytesPerSecond;				
					iAvarageBytesPerSecond = (0.9 * iAvarageBytesPerSecond) + (0.1 * iBytesPerSecond);
					iBytesPerSecond = 0;

					if (!IsComplete() && (iAvarageBytesPerSecond != lastSecondAvarage))
					{
						HLWRITELN(iLog, _L("[Torrent] OnTimerL - Download speed changed notify"));
						
						iTorrentMgr->NotifyTorrentObserverL(this, ESTEventDownloadSpeedChanged);
						
						HLWRITELN(iLog, _L("[Torrent] OnTimerL - Download speed changed notify done"));
					}
				}
				
				// checking whether end game mode can be enabled
				if (!EndGame() && !iTorrentMgr->IsEndGameDisabled())
				{
					if (IsEndGameConditionsMet())
						EnableEndGameL();
				}	
			}
			
			/*if (((iEllapsedTime % iTrackerRequestInterval) == 0) ||
			    (iPeers.Count() == 0) && ((iEllapsedTime % iTrackerRequestInterval) > 300))
				AnnounceL();*/						
			
			// calculating upload speed
			{
				TReal lastSecondAvarage = iAvarageUploadedBytesPerSecond;				
				iAvarageUploadedBytesPerSecond = (0.9 * iAvarageUploadedBytesPerSecond) + (0.1 * iUploadedBytesPerSecond);
				iUploadedBytesPerSecond = 0;

				if (iAvarageUploadedBytesPerSecond != lastSecondAvarage)
				{
					HLWRITELN(iLog, _L("[Torrent] OnTimerL - Upload speed changed notify"));
					
					iTorrentMgr->NotifyTorrentObserverL(this, ESTEventUploadSpeedChanged);
					
					HLWRITELN(iLog, _L("[Torrent] OnTimerL - Upload speed changed notify end"));
				}
			}
	
			HLWRITELN(iLog, _L("[Torrent] OnTimerL - Peer cycle begin"));
			for (TInt i=0; i<iPeers.Count(); i++)
			{
				CSTPeer* peer = iPeers[i];
				
				if (peer->State() != EPeerNotConnected)
				{
					activeConnectionCount++;
				}
				else if (!iComplete)
				{
					if (activeConnectionCount < KMaxPeerConnectionCount ||
						peer->IsPushPeer()) // connecting peers
					{							
						if (peer->ReconnectTime() == 0)
						{
							peer->ConnectL(*this, iTorrentMgr);
							if (peer->IsLocal())
								activeLocalConnectionCount++;
							else
								activeConnectionCount++;							
						}			
					}
				}
			}
			HLWRITELN(iLog, _L("[Torrent] OnTimerL - Peer cycle end"));
			
			if ((activeLocalConnectionCount > 0) ||(iEllapsedTime % KPeerReconnectCheckInterval) == 0)
			{
				HLWRITELN(iLog, _L("[Torrent] OnTimerL - Checking reconnect time"));
				
				for (TInt i=0; i<iPeers.Count(); i++)
				{
					if (iPeers[i]->ReconnectTime() <= iEllapsedTime)
						iPeers[i]->SetReconnectTime(0);
				}
				
				HLWRITELN(iLog, _L("[Torrent] OnTimerL - Checking reconnect time done"));
			}
		}											
			//TODO should be removed?
			if ((iEllapsedTime % KDownloadingPiecesCheckInterval) == 0)
			{
				HLWRITELN(iLog, _L("[Torrent] OnTimerL - DL piece check"));
				
				//iLog->WriteLineL(_L("[Torrent] Searching for lost pieces"));
				for (TInt i=0; i<iDownloadingPieces.Count(); i++)
				{
					TBool foundDownloadingPeer = EFalse;
					for (TInt j=0; j<iPeers.Count(); j++)
						if (iPeers[j]->PiecesToDownload())
						{
							for (TInt k=0; k<iPeers[j]->PiecesToDownload()->Count(); k++)
							{									
								CSTPiece* pieceToDownload = (*iPeers[j]->PiecesToDownload())[k]->iPiece;
								if (pieceToDownload == iDownloadingPieces[i])
								{
									foundDownloadingPeer = ETrue;
									break;
								}
							}							
						}
						
					if (!foundDownloadingPeer)
					{
						LWRITE(iLog, _L("[Torrent] Found lost downloading piece: "));
						LWRITELN(iLog, iDownloadingPieces[i]->Index());
						iDownloadingPieces.Remove(i);
						i--;
					}					
				}
				
				HLWRITELN(iLog, _L("[Torrent] OnTimerL - DL piece check end"));
			}			
	}
	
	HLWRITELN(iLog, _L("[Torrent] OnTimerL end"));
}

EXPORT_C CSTPeer* CSTTorrent::GetPeer(const TSockAddr& aAddress)
{
	for (TInt i=0; i<iPeers.Count(); i++)
		if (iPeers[i]->Address() == aAddress)
			return iPeers[i];
	
	return NULL;
}

CSTPeer* CSTTorrent::GetPeer(const TDesC8& aPeerId)
{
	for (TInt i=0; i<iPeers.Count(); i++)
		if (iPeers[i]->PeerId() == aPeerId)
			return iPeers[i];
	
	return NULL;
}

EXPORT_C TInt CSTTorrent::AddPeerL(CSTPeer* aPeer)
{
	HLWRITELN(iLog, _L("[Torrent] AddPeerL"));
	
	if (iPeers.Count() >= KMaxStoredPeers)
		return KErrOverflow;
	
	for (TInt i=0; i<iPeers.Count(); i++)
	{
		if (((aPeer->PeerId() != KNullDesC8) && (iPeers[i]->PeerId() == aPeer->PeerId()) && (iPeers[i]->IsLocal() == aPeer->IsLocal())) || 
			((iPeers[i]->IsLocal() == aPeer->IsLocal()) && (iPeers[i]->Address() == aPeer->Address())))
		{
			return KErrAlreadyExists;
		}		
	}
	
	iPeers.InsertL(aPeer, 0);
	
	//iLog->WriteLineL(_L("Peer added"));

	return KErrNone;	
}

EXPORT_C TInt CSTTorrent::AddPeerL(const TInetAddr& aAddress)
{
	#ifdef LOG_TO_FILE
	TBuf<50> ipBuf;
	aAddress.Output(ipBuf);
	ipBuf.Append(_L(":"));
	ipBuf.AppendNum(aAddress.Port());
	iLog->WriteL(_L("[Torrent] Adding peer "));
	iLog->WriteL(ipBuf);
	#endif

	if (iPeers.Count() >= KMaxStoredPeers)
	{	
		LWRITELN(iLog, _L(" FAILED (maximum number of peers reached)"));		
		return KErrOverflow;
	}
		
	
	for (TInt i=0; i<iPeers.Count(); i++)
	{
		if (iPeers[i]->Address() == aAddress)
		{
			LWRITELN(iLog, _L(" FAILED (peer is already added)"));
			return KErrGeneral;
		}			
	}
	
	CSTPeer* peer = CSTPeer::NewLC(aAddress, iPieces.Count());
	iPeers.InsertL(peer, 0);
	CleanupStack::Pop(); // peer
	
	LWRITELN(iLog, _L(" OK"));
	
	return KErrNone;
}

EXPORT_C TInt CSTTorrent::AddLocalPeerL(const TSockAddr& aAddr)
{
	#ifdef LOG_TO_FILE
	iLog->WriteL(_L("[Torrent] Adding local peer "));
	
	if (iTorrentMgr->NetworkManager()->Connection(1).Type() == CNetworkConnection::ERConnectionBased)
	{
		TBuf<128> addressBuf;
		TInetAddr remoteAddress(aAddr);
				
		remoteAddress.Output(addressBuf);
		addressBuf.Append(_L(":"));
		addressBuf.AppendNum(remoteAddress.Port());
		
		iLog->WriteLineL(addressBuf);
	}
	else
	{
		iLog->WriteL(aAddr.Port());
	}
	#endif
	
	if (iPeers.LocalPeerCount() >= KMaxStoredLocalPeers)
	{
		LWRITELN(iLog, _L(" FAILED (maximum number of local peers reached)"));
		return KErrOverflow;
	}
	
	for (TInt i=0; i<iPeers.Count(); i++)
	{
		// if IP based connection is used, only compare the address
		if (iTorrentMgr->NetworkManager()->Connection(1).Type() == CNetworkConnection::ERConnectionBased)
		{
			if ((iPeers[i]->IsLocal()) && 
				(static_cast<TInetAddr>(iPeers[i]->Address()).Address() == static_cast<TInetAddr>(aAddr).Address()))
			{
				LWRITELN(iLog, _L(" FAILED (peer is already added)"));
				return KErrGeneral;
			}	
		}
		else
		{
			if ((iPeers[i]->IsLocal()) && (iPeers[i]->Address() == aAddr))
			{
				LWRITELN(iLog, _L(" FAILED (peer is already added)"));
				return KErrGeneral;
			}	
		}
	}

	CSTPeer* peer = CSTPeer::NewLC(aAddr, iPieces.Count(), ETrue);	
	iPeers.InsertL(peer, 0);
	CleanupStack::Pop(); // peer
	
	LWRITELN(iLog, _L(" OK"));
	
	// broadcasting local peer address (peer exchange)
	if (iTorrentMgr->IsLocalCooperationEnabled() && (!iTorrentMgr->IsLocalConnMasterSlaveMode()))
	{
		TInetAddr addr(aAddr);
		
		for (TInt i=0; i<iPeers.Count(); i++)
		{
			if (iPeers[i]->IsLocal() && (iPeers[i]->State() == EPeerPwConnected))
			{
				if (iPeers[i]->Address() != aAddr)
					iPeers[i]->Connection()->SendLocalPeerMessageL(addr.Address(), addr.Port());
			}
		}
	}
	
	return KErrNone;
}

EXPORT_C void CSTTorrent::AddPushPeerL(TInetAddr aAddress)
{
	CSTPeer* pushPeer = CSTPeer::NewLC(aAddress, ETrue);
	iPeers.InsertL(pushPeer, 0);
	CleanupStack::Pop(); // pushPeer	
}

void CSTTorrent::PeerDisconnectedL(CSTPeer* aPeer, TBool aPeerWireConnected)
{
	HLWRITELN(iLog, _L("[Torrent] PeerDisconnectedL begin"));
	
	if (aPeer->PiecesToDownload())
	{
		//LWRITE(iLog, _L("[Torrent] Releasing pieces: "));
		for (TInt i=0; i<aPeer->PiecesToDownload()->Count(); i++)
		{			
			CSTPiece* pieceToDownload = (*aPeer->PiecesToDownload())[i]->iPiece;
			if (pieceToDownload)
			{
			/*	#ifdef LOG_TO_FILE
					if (i != 0)
						iLog->WriteL(_L(", "));
					iLog->WriteL(pieceToDownload->Index());
				#endif*/
				
				RemovePieceFromDownloading(pieceToDownload);								
			}			
		}		
		//LLINE(iLog);
	}
	
/*#ifdef LOG_TO_FILE
	iLog->WriteL(_L("[Torrent] Downloading pieces: "));
	for (TInt i=0; i<iDownloadingPieces.Count(); i++)
	{
		iLog->WriteL(iDownloadingPieces[i]->Index());
		iLog->WriteL(_L(", "));
	}
	iLog->WriteLineL();
	
	iLog->WriteL(_L("[Torrent] Connection count: "));
	iLog->WriteLineL(iConnectionCount);
	
	for (TInt i=0; i<iPeers.Count(); i++)
	{
		if (iPeers[i]->Connection())
		{
			iLog->WriteL(_L("[Torrent] Peer pieces: "));
			for (TInt j=0; j<iPeers[i]->Connection()->PiecesToDownload().Count(); j++)
			{
				iLog->WriteL(iPeers[i]->Connection()->PiecesToDownload()[j]->iPiece->Index());
				iLog->WriteL(_L(", "));
			}
			iLog->WriteLineL();
		}
	}	
#endif*/
	
	if (aPeerWireConnected)
	{
		if (aPeer->IsLocal())
			iLocalConnectionCount--;
		else
			iConnectionCount--;
		
		iTorrentMgr->NotifyTorrentObserverL(this, ESTEventConnectionsChanged);
	}
	
	for (TInt i=0; i<iPeers.Count(); i++)
	{
		if (iPeers[i]->State() == EPeerPwConnected)
			iPeers[i]->Connection()->IssueDownloadL();
	}
	
	HLWRITELN(iLog, _L("[Torrent] PeerDisconnectedL - before IsPushPeer check"));
	
	// we hold the disconnected peers in an array until the next OnTimerL call when they will be rearranged
	// to the end of the peers list (so the client won't reconnect them too soon)
	if (!aPeer->IsPushPeer())
		User::LeaveIfError(iDisconnectedPeers.Append(aPeer));
	
	HLWRITELN(iLog, _L("[Torrent] PeerDisconnectedL end"));
}

TBool CSTTorrent::HasTimeoutlessPeer()
{
	for (TInt i=0; i<iPeers.Count(); i++)
		if (!iPeers[i]->HadRequestTimeout())
			return ETrue;
		
	return EFalse;
}

TBool CSTTorrent::IsEndGameConditionsMet()
{
	return (((iPiecesLeftToDownloadCount <= iDownloadingPieces.Count()) && (iDownloadingPieces.Count() < 30)) 
	        || (iPiecesLeftToDownloadCount < 20));
}

CSTPiece* CSTTorrent::GetPieceToDownloadL(CSTPeer* aPeer)
{
	HLWRITELN(iLog, _L("[Torrent] GetPieceToDownloadL begin"));
	
	if (IsComplete())
		return NULL;

	CSTPiece* pieceToDownload = NULL;
	
	CSTBitField* downloadable = iBitField->CloneL();
	CleanupStack::PushL(downloadable);	
	downloadable->BitwiseNot();
	downloadable->BitwiseAnd(iToDownloadBitField);
	downloadable->BitwiseAnd(aPeer->BitField());
	
	TInt peersHavePiece = 0xFFFF;
	TInt numberOfPieces = 0;
	
	// enters end game mode if all remaining pieces have been requested
	//if (!EndGame() && ((iPieces.Count() - iDownloadedPieceCount) <= iDownloadingPieces.Count()) && (iDownloadingPieces.Count() < 20))
	//	EnableEndGameL();
	
	// END GAME CANNOT BE ENABLED HERE!
	
	if (EndGame()) // End Game mode
	{
		for (TInt j=0; j<iPieces.Count(); j++)
		{
			if ((downloadable->IsBitSet(j)) && (iDownloadingPieces.Find(iPieces[j]) < 0))
			{
				if (aPeer->FailedPieceCollector().IsDenied(*iPieces[j])) // skip piece if it is denied
					continue;
				
				TBool alreadyDownloading = EFalse;
				for (TInt i=0; i<aPeer->PiecesToDownload()->Count(); i++)
				{
					if (((*aPeer->PiecesToDownload())[i])->Piece() == iPieces[j])
					{
						alreadyDownloading = ETrue;
						break;
					}
				}
				
				if (!alreadyDownloading)
				{
					pieceToDownload	= iPieces[j];
					break;
				}
			}
		}
	}
	else // Normal mode
	{
		for (TInt i=0; i<iPieces.Count(); i++)
		{
			if (downloadable->IsBitSet(i) && (iDownloadingPieces.Find(iPieces[i]) < 0))
			{
				if (aPeer->FailedPieceCollector().IsDenied(*iPieces[i])) // skip piece if it is denied
					continue;
				
				if (iPieces[i]->NumberOfPeersHaveThis() < peersHavePiece)
				{
					peersHavePiece = iPieces[i]->NumberOfPeersHaveThis();
					
					//pieceToDownload = iPieces[i];
					numberOfPieces = 1;
					//break;
				}
				else
					if (iPieces[i]->NumberOfPeersHaveThis() == peersHavePiece)
						numberOfPieces++;
			}	
		}
		
		TInt index = 0;	
		if (numberOfPieces > 0)
			index = Math::Rand(iRandomSeed) % numberOfPieces;
		
		for (TInt j=0; j<iPieces.Count(); j++)
		{
			if ((downloadable->IsBitSet(j)) && (iDownloadingPieces.Find(iPieces[j]) < 0) && (iPieces[j]->NumberOfPeersHaveThis() == peersHavePiece))
			{								
				if (index == 0)
				{
					User::LeaveIfError(iDownloadingPieces.Append(iPieces[j]));
					
					pieceToDownload	= iPieces[j];
					break;
				}
				else
					index--;																
			}		
		}
	}
				
	CleanupStack::PopAndDestroy(); // downloadable
	
	HLWRITELN(iLog, _L("[Torrent] GetPieceToDownloadL end"));
	
	return pieceToDownload;
}


void CSTTorrent::RemovePieceFromDownloading(CSTPiece* aPiece)
{
	TInt index = iDownloadingPieces.Find(aPiece);
	if (index >= 0)
		iDownloadingPieces.Remove(index);
	
	/*TInt egIndex = iEndGamePiecesInTransit.Find(aPiece);
	if (egIndex >= 0)
		iEndGamePiecesInTransit.Remove(egIndex);*/
}

void CSTTorrent::PieceDownloadedL(CSTPiece* aPiece, TBool aNotifyTorrentObserver)
{
	PieceDownloadedL(aPiece, NULL, aNotifyTorrentObserver);
}

void CSTTorrent::PieceDownloadedL(CSTPiece* aPiece, CSTPeer* aPeer, TBool aNotifyTorrentObserver)
{
	HLWRITELN(iLog, _L("[Torrent] PieceDownloaded begin"));
	
	RemovePieceFromDownloading(aPiece);

	iBitField->SetBit(aPiece->Index());
	if (aPeer && (aPeer->IsLocal()))
		iLocalDownloadBitField->SetBit(aPiece->Index());

	iDownloadedPieceCount++;
	iPiecesLeftToDownloadCount--;
	
	if (aNotifyTorrentObserver)
		iTorrentMgr->NotifyTorrentObserverL(this, ESTEventPieceDownloaded);
	
	CheckDownloadCompleteL();
	
	if (iActive)
	{
		for (TInt i=0; i<iPeers.Count(); i++)
			iPeers[i]->NotifyThatClientHavePiece(aPiece->Index());
	}
	
	HLWRITELN(iLog, _L("[Torrent] PieceDownloaded end"));
}

void CSTTorrent::CheckDownloadCompleteL()
{
	HLWRITELN(iLog, _L("[Torrent] CheckDownloadCompleteL begin"));
	
	CSTBitField* bitfield = iBitField->CloneL();
	CleanupStack::PushL(bitfield);
	bitfield->BitwiseAnd(iToDownloadBitField);
	
	if (!iComplete)
	{
		// only set complete if the download selection has some files (not all files are skipped)
		if ((*bitfield == *iToDownloadBitField) && (iBytesToDownload != 0))
		{
			iComplete = ETrue;
			LWRITELN(iLog, _L("[Torrent] DOWNLOAD COMPLETE (All selected pieces have been received)"));
			iFileManager->CloseAll();
			
			iAvarageBytesPerSecond = 0;

			// in case of loading a "saved torrent", connecting to the tracker is not needed
			if (iActive)
			{
				if (Preferences()->CloseConnectionAfterDownload() && (!iToDownloadBitField->IsNull()))
				{
					StopL();
					//iTorrentMgr->CloseNetworkIfNoTorrentsActiveL();
				}				
				else
				{
					// announce only if the full torrent has been downloaded (no files are skipped)
					if (*iToDownloadBitField == *iBitField)
						iTrackerManager->AnnounceL(ETrackerEventCompleted);					
				}	
			}
			
			iTorrentMgr->NotifyTorrentObserverL(this, ESTEventDownloadedBytesChanged);
		}
	}
	else
	{
		if (*bitfield != *iToDownloadBitField)
		{
			iComplete = EFalse;
		}
	}
		
	CleanupStack::PopAndDestroy(); // bitfield
	
	HLWRITELN(iLog, _L("[Torrent] CheckDownloadCompleteL end"));
}


void CSTTorrent::PieceHashFailedL(CSTPiece* aPiece)
{
	RemovePieceFromDownloading(aPiece);
	
	UpdateBytesDownloadedL(-aPiece->TotalSize());
}


void CSTTorrent::CalculateFileFragmentsL()
{
	TInt pieceIndex = 0;
	CSTPiece* piece = iPieces[pieceIndex];
	TInt piecePos = 0;

	for (TInt i=0; i<iFiles.Count(); i++)
	{
		CSTFile* file = iFiles[i];

		if (TUint(piecePos + file->Size()) <= TUint(piece->TotalSize()))
		{
			piece->AddFileFragmentL(*file, 0, file->Size());
			piecePos += file->Size();

			if (TUint(piecePos + file->Size()) == TUint(piece->TotalSize()))
			{
				if ( (i < (iFiles.Count()-1)) && (iFiles[i+1]->Size() > 0))
				{
					piece = iPieces[++pieceIndex];
					piecePos = 0;
				}			
			}
		}
		else
		{
			TInt filePos = 0;

			while (TUint(filePos) < file->Size())
			{
				if (TUint(file->Size() - filePos + piecePos) > TUint(piece->TotalSize()))
				{
					piece->AddFileFragmentL(*file, filePos, piece->TotalSize() - piecePos);
					filePos += piece->TotalSize() - piecePos;

					piece = iPieces[++pieceIndex];
					piecePos = 0;
				}
				else
				{
					piece->AddFileFragmentL(*file, filePos, file->Size() - filePos);
					piecePos += file->Size() - filePos;

					if (piecePos == piece->TotalSize())
					{
						if (pieceIndex < (iPieces.Count() - 1))
						{
							piece = iPieces[++pieceIndex];
							piecePos = 0;
						}
					}

					break;
				}
			}
		}
	}
}

void CSTTorrent::UpdateBytesDownloadedL(TInt aBytesDownloaded, TBool aNotifyObserver)
{
	HLWRITELN(iLog, _L("[Torrent] UpdateBytesDownloadedL begin"));
	
	iTotalBytesDownloaded += aBytesDownloaded;
	iTotalBytesLeft -= aBytesDownloaded;
	iDownloadBytesLeft -= aBytesDownloaded;

	iDownloadPercent = (iBytesToDownload - iDownloadBytesLeft) / (iBytesToDownload / 100.0);

	if (aNotifyObserver)
	{
		iBytesPerSecond += aBytesDownloaded;
		if (iBytesPerSecond < 0)
			iBytesPerSecond = 0;
		
		iTorrentMgr->IncBytesDownloadedL(aBytesDownloaded);
		iTorrentMgr->NotifyTorrentObserverL(this, ESTEventDownloadedBytesChanged);
	}
	
	HLWRITELN(iLog, _L("[Torrent] UpdateBytesDownloadedL end"));
}

void CSTTorrent::ResetDownloadsL(TBool aNotifyObserver)
{
	iDownloadedPieceCount = 0;
	UpdateBytesDownloadedL(-BytesDownloaded(), aNotifyObserver);
	iComplete = EFalse;
}

void CSTTorrent::UpdateBytesUploadedL(TInt aBytesUploaded, TBool aNotifyObserver)
{
	HLWRITELN(iLog, _L("[Torrent] UpdateBytesUploadedL begin"));
	
	iBytesUploaded += aBytesUploaded;
	iUploadedBytesPerSecond += aBytesUploaded;

	if (aNotifyObserver)
	{
		iTorrentMgr->IncBytesUploadedL(aBytesUploaded);
		iTorrentMgr->NotifyTorrentObserverL(this, ESTEventUploadedBytesChanged);
	}
	
	HLWRITELN(iLog, _L("[Torrent] UpdateBytesUploadedL end"));
}

void CSTTorrent::IncreaseConnectionCountL()
{
	HLWRITELN(iLog, _L("[Torrent] IncreaseConnectionCountL begin"));
	
	iConnectionCount++;
	iTorrentMgr->NotifyTorrentObserverL(this, ESTEventConnectionsChanged);
	
	HLWRITELN(iLog, _L("[Torrent] IncreaseConnectionCountL end"));
}

void CSTTorrent::IncreaseLocalConnectionCountL()
{
	iLocalConnectionCount++;
	iTorrentMgr->NotifyTorrentObserverL(this, ESTEventConnectionsChanged);
}

EXPORT_C void CSTTorrent::SetFailedL()
{
	LWRITELN(iLog, _L("[Torrent] FAILED"));
	iFailed = ETrue;
		
	StopL();
	
	//delete iFailReason;
	//iFailReason = NULL;
	//iFailReason = aReason.AllocL();
	
	SetStatusInfoL(ETorrentFailed);
}

void CSTTorrent::IncreaseNumberOfPeersHavePiece(TInt aPieceIndex)
{
	iPieces[aPieceIndex]->IncNumberOfPeersHaveThis();
}
	
void CSTTorrent::DecreaseNumberOfPeersHavingPiece(TInt aPieceIndex)
{
	iPieces[aPieceIndex]->DecNumberOfPeersHaveThis();	
}

void CSTTorrent::ReportProxyConnectionL(TBool /*aProxyConnectionSucceeded*/)
{
	//iTrackerManager->AnnounceL();
}

EXPORT_C void CSTTorrent::DeletePeer(const TSockAddr& aAddress)
{
	HLWRITELN(iLog, _L("[Torrent] DeletePeer begin"));
	
	for (TInt i=0; i<iPeers.Count(); i++)
	{
		if (iPeers[i]->Address() == aAddress)
		{
			delete iPeers[i];
			iPeers.Remove(i);
			break;
		}
	}
	
	HLWRITELN(iLog, _L("[Torrent] DeletePeer end"));
}

EXPORT_C TBool CSTTorrent::HasEnoughDiskSpaceToDownload()
{	
	if (iPath)
	{
		TInt drive = EDriveC;
		
		if (iPath->Length() > 0)
		{
			RFs session; 
        	TInt status = session.Connect(); 
        	if (status != KErrNone) 
                return EFalse; 
			
			if (TChar((*iPath)[0]).GetUpperCase() == 'C')
				drive = EDriveC;
			else
				if (TChar((*iPath)[0]).GetUpperCase() == 'E')
					drive = EDriveE;
				else
				{
					TBuf<256> sessionPath;
					if (session.SessionPath(sessionPath) == KErrNone)
					{
						if (sessionPath.Length() > 0)
						{				
							if (TChar(sessionPath[0]).GetUpperCase() == 'E')
								drive = EDriveE;
						}
					}
				}
			
			TVolumeInfo volumeInfo;	
			TInt retVal = session.Volume( volumeInfo, drive );
			session.Close();
			 
        	if (retVal == KErrNone) 
        	{
        		if (volumeInfo.iFree >= (iBytesToDownload - iTotalBytesDownloaded))
        			return ETrue;
        		else
        			return EFalse;
        	}         
		}		
	}
	
	return ETrue;
}

void CSTTorrent::SetStatusInfoL(TSTTorrentStatusInfo aStatus) 
{
	if (iStatusInfo != aStatus)
	{
		iStatusInfo = aStatus;
		
		iTorrentMgr->NotifyTorrentObserverL(this, ESTEventStatusInfoChanged);
		
		if ((iStatusInfo == ETrackerFailed) || (iStatusInfo == ETrackerSuccessful) || (iStatusInfo == ETrackerConnecting))
		{
			iStatusInfoDelay = 3;
		}
	}
}

void CSTTorrent::EnableEndGameL()
{
	HLWRITELN(iLog, _L("[Torrent] EnableEndGameL begin"));
	
	TBuf<100> buf;
	buf.Format(_L("[Torrent] Entering End Game mode. Remaining pieces: %d Downloading pieces: %d"),(iPieces.Count() - iDownloadedPieceCount), iDownloadingPieces.Count());
	LWRITELN(iLog, buf);
	
	iEndGame = ETrue;
	
	// cancel all actieve piece requests, to have a fresh start for the end game mode
	for (TInt i=0; i<iPeers.Count(); i++)
	{
		if (iPeers[i]->Connection())
			iPeers[i]->Connection()->CancelAllPieceRequestsL();
	}
	
	iDownloadingPieces.Reset();
	
	HLWRITELN(iLog, _L("[Torrent] EnableEndGameL end"));
}

void CSTTorrent::DisableEndGameL()
{
	if (iEndGame)
	{
		LWRITELN(iLog, _L("[Torrent] Exiting End Game mode."));
		
		iEndGame = EFalse;
		
		iDownloadingPieces.Reset();
		//iEndGamePiecesInTransit.Reset();
		
		// cancel all actieve piece requests
		for (TInt i=0; i<iPeers.Count(); i++)
		{
			if (iPeers[i]->Connection())
				iPeers[i]->Connection()->CancelAllPieceRequestsL();
		}				
	}	
}

void CSTTorrent::EndGamePieceReceivedL(CSTPiece* aPiece, CSTPeer* aPeer)
{
	if (iDownloadingPieces.Find(aPiece) < 0)
	{
		TBuf<100> buf;
		buf.Format(_L("[Torrent] Piece %d added to End Game pieces"), aPiece->Index());
		LWRITELN(iLog, buf);
		
		User::LeaveIfError(iDownloadingPieces.Append(aPiece));
		for (TInt i=0; i<iPeers.Count(); i++)
		{
			if (iPeers[i] != aPeer)
				iPeers[i]->CancelPieceRequestL(aPiece);
		}
	}
}

EXPORT_C void CSTTorrent::CheckHashL(CAknProgressDialog* aProgressDialog)
{
	HLWRITELN(iLog, _L("[Torrent] CheckHashL begin"));
	
	if (!iHashChecker)
	{
		iHashChecker = new (ELeave) CSTHashChecker(this, aProgressDialog, this);
		CActiveScheduler::Add(iHashChecker);
		iHashChecker->StartL();
	}
	
	HLWRITELN(iLog, _L("[Torrent] CheckHashL end"));
}

void CSTTorrent::HashCheckFinishedL(CSTHashChecker* /*aHashChecker*/)
{
	HLWRITELN(iLog, _L("[Torrent] HashCheckFinishedL begin"));
	
	delete iHashChecker;
	iHashChecker = 0;
	
	iTorrentMgr->NotifyTorrentObserverL(this, ESTEventDownloadedBytesChanged);
	
	HLWRITELN(iLog, _L("[Torrent] HashCheckFinishedL end"));
}

void CSTTorrent::DeletePrimaryPeerL(const TDesC8& aPeerId)
{
	HLWRITELN(iLog, _L("[Torrent] DeletePrimaryPeerL begin"));
	
	for (TInt i=0; i<iPeers.Count(); i++)
	{
		if ((!iPeers[i]->IsLocal()) && (iPeers[i]->PeerId() == aPeerId))
		{
			iPeers[i]->DeleteL();
		}
	}
	
	HLWRITELN(iLog, _L("[Torrent] DeletePrimaryPeerL end"));
}

void CSTTorrent::HaveLocalNetPiecesL(const TDesC8& aBitFieldDes, const CSTPeer& aPeer)
{
	CSTBitField* bitfield = new (ELeave) CSTBitField;
	CleanupStack::PushL(bitfield);
	bitfield->ConstructL(PieceCount());
	bitfield->SetL(aBitFieldDes);
	
	iLocalNetBitField->BitwiseOr(bitfield);
	
	if (iTorrentMgr->IsLocalConnMasterSlaveMode() && 
		(iTorrentMgr->LocalConnectionMSType() == CSTTorrentManager::EMaster))
	{
		for (TInt i=0; i<iPeers.Count(); i++)
		{
			if (iPeers[i]->IsLocal() && (iPeers[i]->State() == EPeerPwConnected))
			{
				if (iPeers[i] != &aPeer)
					iPeers[i]->Connection()->SendLocalBitfieldMessageL(aBitFieldDes);
			}
		}
	}
	
	CleanupStack::PopAndDestroy(); // bitfield
}

void CSTTorrent::HaveLocalNetPieceL(TInt aPieceIndex, const CSTPeer& aPeer)
{
	iLocalNetBitField->SetBit(aPieceIndex);
	
	if (iTorrentMgr->IsLocalConnMasterSlaveMode() && 
		(iTorrentMgr->LocalConnectionMSType() == CSTTorrentManager::EMaster))
	{
		for (TInt i=0; i<iPeers.Count(); i++)
		{
			if (iPeers[i]->IsLocal() && (iPeers[i]->State() == EPeerPwConnected))
			{
				if ((!iPeers[i]->BitField()->IsBitSet(aPieceIndex)) && (iPeers[i] != &aPeer))
					iPeers[i]->Connection()->SendLocalHaveMessageL(aPieceIndex);
			}
		}
	}
}

void CSTTorrent::BroadcastLocalPeerL(TUint32 aAddress, TUint aPort, const CSTPeer& aException)
{
	// broadcast peer only if not using master-slave mode
	if (iTorrentMgr->IsLocalCooperationEnabled() && (!iTorrentMgr->IsLocalConnMasterSlaveMode()))
	{
		for (TInt i=0; i<iPeers.Count(); i++)
		{
			if (iPeers[i]->IsLocal() && (iPeers[i]->State() == EPeerPwConnected))
			{
				if (iPeers[i] != &aException)
					iPeers[i]->Connection()->SendLocalPeerMessageL(aAddress, aPort);
			}
		}
	}
}

void CSTTorrent::GetLocalPeersL(RPointerArray<CSTPeer>& aPeers)
{
	for (TInt i=0; i<iPeers.Count(); i++)
	{
		if (iPeers[i]->IsLocal())
			aPeers.AppendL(iPeers[i]);
	}
}

EXPORT_C void CSTTorrent::SetAllFilesSkippedL(TBool aSkipped)
{
	for (TInt i=0; i<iPieces.Count(); i++)
		if (aSkipped)
			iToDownloadBitField->UnsetBit(i);
		else
			iToDownloadBitField->SetBit(i);
	
	for (TInt j=0; j<iFiles.Count(); j++)
		iFiles[j]->SetSkipped(aSkipped);
	
	CheckDownloadCompleteL();
	CalculateBytesToDownload();
	
	if (EndGame() && !IsEndGameConditionsMet())		
		DisableEndGameL();
	
	iTorrentMgr->NotifyTorrentObserverL(this, ESTEventDownloadedBytesChanged);
}

EXPORT_C void CSTTorrent::SetFileSkippedL(TInt aFileIndex, TBool aSkipped)
{
	CSTFile* file = iFiles[aFileIndex];
	file->SetSkipped(aSkipped);
	
	for (TInt i=0; i<iPieces.Count(); i++)
	{
		if (iPieces[i]->HasFile(file))
		{
			if (aSkipped)
			{
				if (iPieces[i]->IsAllFilesSkipped())
				{
					iToDownloadBitField->UnsetBit(i);
					
					// cancel active download(s)
					for (TInt j=0; j<iPeers.Count(); j++)
					{
						iPeers[j]->CancelPieceRequestL(iPieces[i]);
					}
					
					RemovePieceFromDownloading(iPieces[i]);
				}
			}			
			else
			{
				iToDownloadBitField->SetBit(i);
			}			
		}
	}
	
	CheckDownloadCompleteL();
	CalculateBytesToDownload();
	
	if (EndGame() && !IsEndGameConditionsMet())		
		DisableEndGameL();
	
	iTorrentMgr->NotifyTorrentObserverL(this, ESTEventDownloadedBytesChanged);
}

void CSTTorrent::CalculateBytesToDownload()
{
	iBytesToDownload = 0;
	iDownloadBytesLeft = 0;
	
	iPiecesLeftToDownloadCount = 0;
	
	for (TInt i=0; i<iPieces.Count(); i++)
	{
		if (iToDownloadBitField->IsBitSet(i))
		{
			if (!iPieces[i]->IsDownloaded())
				iPiecesLeftToDownloadCount++;
			
			iBytesToDownload += iPieces[i]->TotalSize();
			iDownloadBytesLeft += (iPieces[i]->TotalSize() - iPieces[i]->DownloadedSize());
		}			
	}
	
	iDownloadPercent = (iBytesToDownload - iDownloadBytesLeft) / (iBytesToDownload / 100.0);
	
/*	iLog->WriteL(_L("Size in bytes: "));
	iLog->WriteLineL(Size());
	iLog->WriteL(_L("Bytes downloaded: "));
	iLog->WriteLineL(iTotalBytesDownloaded);
	iLog->WriteL(_L("Bytes to download: "));
	iLog->WriteLineL(iBytesToDownload);
	iLog->WriteL(_L("iToDownloadBitfield: "));
	iLog->WriteLineL(iToDownloadBitField->Data());*/
}

void CSTTorrent::ReportCriticalFaultL(TSTCriticalFaultType aFault)
{
	SetFailedL();
	
	if (iTorrentMgr->EngineEventObserver())
		iTorrentMgr->EngineEventObserver()->HandleTorrentCriticalFaultL(this, aFault);			
}

void CSTTorrent::AnnouncedToAllTrackersL(TTrackerConnectionEvent aEvent)
{
	if (aEvent == ETrackerEventStopped)
	{
		
	}
}

EXPORT_C TInt CSTTorrent::SkipTooLargeFiles()
{
	TInt skippedCount = 0;
	for (TInt i=0; i<iFiles.Count(); i++)
	{
		if (iFiles[i]->Size() >= 4000000000) // 4GB, FAT32 limit
		{
			SetFileSkippedL(i, ETrue);
			skippedCount++;
		}
	}
	
	return skippedCount;
}

#ifdef USE_DHT
void CSTTorrent::ValueFoundL(const CKey& /*aKey*/, const TBinDataPtr& aValue)
{		
	if (aValue.iLen == 6)
	{
		iLog->WriteLineL(_L("[Torrent][DHT] Got peer"));
		
		iPeersFromDHTCount++;
		
		TUDPAddress address = CreateUDPAddress(TPtrC8(aValue.iPtr, aValue.iLen));
		
		TInetAddr addr;
		addr.SetAddress(address.iAddress);
		addr.SetPort(address.iPort);
		
		AddPeerL(addr);
	}
	else
		iLog->WriteLineL(_L("[Torrent][DHT] Wrong peer data"));
}

void CSTTorrent::GetValueFinishedL(const CKey& /*aKey*/)
{
	iLog->WriteLineL(_L("[Torrent][DHT] Announce finished"));
	iDHTAnnouncing = EFalse;
}
#endif
