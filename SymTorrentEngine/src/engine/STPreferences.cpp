/*****************************************************************************
 * Copyright (C) 2006 Imre Kelényi
 *-------------------------------------------------------------------
 * This file is part of SymTorrent
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
 *  Name     : CSTPreferences from STPreferences.cpp
 *  Part of  : SymTorrent Engine
 *  Created  : 31.01.2006 by Imre Kelényi
 * ============================================================================
 */

// INCLUDE FILES
#include "STPreferences.h"
#include "SymTorrentEngineLog.h"
#include <badesca.h>
#include <f32file.h>
#include <utf.h>

_LIT8(KLit8Colon, ":");
_LIT8(KLit8EqualSign, "=");
_LIT8(KLit8EndLine, "\r\n");

// the maximum length of the setting names is 50 characters!
const TInt KMaxSettingNameLength = 50;

_LIT8(KSettingDownloadPath, "downloadpath");
_LIT8(KSettingSTorrent, "storrent");
_LIT8(KSettingAccesPointId, "iapid");
_LIT8(KSettingAccesPointName, "iapname");
_LIT8(KSettingProxyHostName, "proxy_hostname");
_LIT8(KSettingProxyConnectionPort, "proxy_connectionport");
_LIT8(KSettingProxyServicePort, "proxy_serviceport");
_LIT8(KSettingIncomingConnectionsMode, "incoming_connections_mode");
_LIT8(KSettingUploadEnabled, "upload_enabled");
_LIT8(KSettingIncomingPort, "incomingport");
_LIT8(KSettingStartupHashCheck, "startup_hash_check");
_LIT8(KSettingCloseConnectionAfterDownload, "close_connection_after_download");
_LIT8(KSettingSubpieceSize, "subpiece_size");
#ifdef EKA2
_LIT8(KSettingTrackerServicePort, "tracker_serviceport");
_LIT8(KSettingPieceSize, "piecesize");
_LIT8(KSettingDHTEnabled, "dht_enabled");
#endif
_LIT8(KSettingRightSoftkeyMode, "right_softkey_mode");

// ================= MEMBER FUNCTIONS =======================

void CSTPreferences::ConstructL()
{
	#ifdef LOG_TO_FILE
	iLog = LOGMGR->GetLoggerL(TUid::Uid(SYMTORRENT_ENGINE_UID));
	#endif
	
	iSTorrents = new (ELeave) CDesCArrayFlat(5);
	
	ResetSettingsL();
	LoadSettingsL();	
}
	
CSTPreferences::~CSTPreferences()
{
	iPreferencesObservers.Reset();
	delete iDownloadPath;
	delete iAccessPointName;
	delete iSTorrents;
	delete iProxyHostName;
	delete iPreferencesFileName;
}

EXPORT_C void CSTPreferences::SetPreferencesFileL(const TDesC& aFileName)
{
	delete iPreferencesFileName;
	iPreferencesFileName = 0;
	
	iPreferencesFileName = aFileName.AllocL();
}

EXPORT_C void CSTPreferences::ResetSettingsL()
{
	delete iDownloadPath;
	iDownloadPath = NULL;
	#ifdef __WINS__
		iDownloadPath = _L("\\system\\apps\\symtorrent\\download\\").AllocL();
	#else
		iDownloadPath = _L("E:\\BitTorrent\\").AllocL();
	#endif
	
	delete iAccessPointName;
	iAccessPointName = NULL;
	iAccessPointName = KNullDesC().AllocL();
	
	iAccessPointId = 0;

	iSTorrents->Reset();
	
	_LIT(KLocalhost, "127.0.0.1");
	SetProxyAddressL(KLocalhost);
//	SetProxyAddressL(_L("81.182.243.156"));
	
	SetProxyConnectionPort(10000);
	SetProxyServicePort(10001);
	
	SetIncomingPort(6881);
	
	SetStartupHashCheck(EFalse); // hash checking could take much time therefore it's disabled by default

	#ifdef EKA2
	SetTrackerServicePort(10002);
	SetPieceSize(64);
	#ifdef USE_DHT
		SetDHTEnabled(ETrue);
	#else
		SetDHTEnabled(EFalse);
	#endif
	#endif
	
	SetUploadEnabled(ETrue);
	SetIncomingConnectionsMode(CNetworkManager::EEnabledWithoutProxy);
	SetSubpieceSize(0);
	
	SetRightSoftkeyMode(ESTSoftkeyExit);
	
	SetCloseConnectionAfterDownload(EFalse);
}

EXPORT_C void CSTPreferences::LoadSettingsL()
{
	if (iPreferencesFileName)
	{
		LWRITELN(iLog, _L("Loading settings"));
		RFs fs;
		User::LeaveIfError(fs.Connect());
		CleanupClosePushL(fs);
		
		RFile file;
		if (file.Open(fs, *iPreferencesFileName, EFileRead|EFileShareReadersOnly) == KErrNone)
		{
			LWRITELN(iLog, _L("Settings file opened"));
			CleanupClosePushL(file);
			TBuf8<KMaxSettingNameLength> settingName;
			TInt settingLength;
			
			while (ReadSettingName(file, settingName, settingLength))
			{			
				if (settingName == KSettingDownloadPath)
				{
					HBufC* downloadPath = ReadAndConvertFromUtf8ToUnicodeL(file, settingLength);
					
					if (downloadPath)
					{
						delete iDownloadPath;
						iDownloadPath = downloadPath;
					}
				}
				else
				
				if (settingName == KSettingAccesPointId)
				{
					HBufC* iapIdBuf = ReadAndConvertFromUtf8ToUnicodeL(file, settingLength);
					
					if (iapIdBuf)
					{
						TLex lex(*iapIdBuf);
						lex.Mark();
						
						TInt iapId = 0;
						if (lex.Val(iapId) == KErrNone)
							iAccessPointId = iapId;
					}
					
					delete iapIdBuf;
				}
				else
				
				if (settingName == KSettingAccesPointName)
				{
					HBufC* iapName = ReadAndConvertFromUtf8ToUnicodeL(file, settingLength);
					
					if (iapName)
					{
						delete iAccessPointName;
						iAccessPointName = iapName;
					}
				}
				else
				
				if (settingName == KSettingRightSoftkeyMode)
				{
					HBufC* rsmBuf = ReadAndConvertFromUtf8ToUnicodeL(file, settingLength);
					
					if (rsmBuf)
					{
						TLex lex(*rsmBuf);
						lex.Mark();
						
						TInt rsm = 0;
						if (lex.Val(rsm) == KErrNone)
							SetRightSoftkeyMode(TSTSoftkeyMode(rsm));
					}
					
					delete rsmBuf;
				}
				else
				
				if (settingName == KSettingSTorrent)
				{
					HBufC* sTorrent = ReadAndConvertFromUtf8ToUnicodeL(file, settingLength);
					CleanupStack::PushL(sTorrent);
					
					if (sTorrent)
						iSTorrents->AppendL(*sTorrent);
					
					CleanupStack::PopAndDestroy(); // sTorrent
				}
				
				else
					if (settingName == KSettingProxyServicePort)
					{
						HBufC* servicePortBuf = ReadAndConvertFromUtf8ToUnicodeL(file, settingLength);
						
						if (servicePortBuf)
						{
							TLex lex(*servicePortBuf);
							lex.Mark();
							
							TInt servicePort = 0;
							if (lex.Val(servicePort) == KErrNone)
								SetProxyServicePort(servicePort);
						}
						
						delete servicePortBuf;
					}
					else
					
					if (settingName == KSettingProxyConnectionPort)
					{
						HBufC* connPortBuf = ReadAndConvertFromUtf8ToUnicodeL(file, settingLength);
						
						if (connPortBuf)
						{
							TLex lex(*connPortBuf);
							lex.Mark();
							
							TInt connPort = 0;
							if (lex.Val(connPort) == KErrNone)
								SetProxyConnectionPort(connPort);
						}
						
						delete connPortBuf;
					}
					else
					
					if (settingName == KSettingProxyHostName)
					{
						HBufC* hostname = ReadAndConvertFromUtf8ToUnicodeL(file, settingLength);
				
						if (hostname)
						{
							CleanupStack::PushL(hostname);
							SetProxyAddressL(*hostname);
							CleanupStack::PopAndDestroy(hostname);
						}
					}
					else
					
					if (settingName == KSettingIncomingConnectionsMode)
					{
						HBufC* icmBuf = ReadAndConvertFromUtf8ToUnicodeL(file, settingLength);
						
						if (icmBuf)
						{
							TLex lex(*icmBuf);
							lex.Mark();
							
							TInt icm = 0;
							if (lex.Val(icm) == KErrNone)
								SetIncomingConnectionsMode(CNetworkManager::TIncomingConnectionsMode(icm));
						}
						
						delete icmBuf;
					}
					else	
					
					if (settingName == KSettingUploadEnabled)
					{
						HBufC* upBuf = ReadAndConvertFromUtf8ToUnicodeL(file, settingLength);
						
						if (upBuf)
						{
							TLex lex(*upBuf);
							lex.Mark();
							
							TInt uploadEnabled = 0;
							if (lex.Val(uploadEnabled) == KErrNone)
								if (uploadEnabled)
									SetUploadEnabled(ETrue);
								else
									SetUploadEnabled(EFalse);
						}
						
						delete upBuf;
					}		
					else											
					
					if (settingName == KSettingStartupHashCheck)
					{
						HBufC* hcBuf = ReadAndConvertFromUtf8ToUnicodeL(file, settingLength);
						CleanupStack::PushL(hcBuf);
						
						if (hcBuf)
						{
							TLex lex(*hcBuf);
							lex.Mark();
							
							TInt hashCheck = 0;
							if (lex.Val(hashCheck) == KErrNone)
								if (hashCheck)
									SetStartupHashCheck(ETrue);
								else
									SetStartupHashCheck(EFalse);
						}
						
						CleanupStack::PopAndDestroy(); // hcBuf
					}		
					else
						
					if (settingName == KSettingCloseConnectionAfterDownload)
					{
						HBufC* hcBuf = ReadAndConvertFromUtf8ToUnicodeL(file, settingLength);
						CleanupStack::PushL(hcBuf);
						
						//TODO test before releasing
						
						/*if (hcBuf)
						{
							TLex lex(*hcBuf);
							lex.Mark();
							
							TInt closeConn = 0;
							if (lex.Val(closeConn) == KErrNone)
								if (closeConn)
									SetCloseConnectionAfterDownload(ETrue);
								else
									SetCloseConnectionAfterDownload(EFalse);
						}*/
						
						CleanupStack::PopAndDestroy(); // hcBuf
					}		
					else
						
					if (settingName == KSettingSubpieceSize)
					{													
						HBufC* buf = ReadAndConvertFromUtf8ToUnicodeL(file, settingLength);
											
						if (buf)
						{
							TLex lex(*buf);
							lex.Mark();
							
							TInt sps = 0;
							if (lex.Val(sps) == KErrNone)
								SetSubpieceSize(sps);
						}
						
						delete buf;
					}
					else
					
					#ifdef EKA2					
					if (settingName == KSettingTrackerServicePort)
					{
						HBufC* trackerServicePortBuf = ReadAndConvertFromUtf8ToUnicodeL(file, settingLength);
						
						if (trackerServicePortBuf)
						{
							TLex lex(*trackerServicePortBuf);
							lex.Mark();
							
							TInt trackerServicePort = 0;
							if (lex.Val(trackerServicePort) == KErrNone)
								SetTrackerServicePort(trackerServicePort);
						}
						
						delete trackerServicePortBuf;
					}
					else
						
					if (settingName == KSettingDHTEnabled)
					{
						HBufC* buf = ReadAndConvertFromUtf8ToUnicodeL(file, settingLength);
						CleanupStack::PushL(buf);
						#ifdef USE_DHT																					
							if (buf)
							{
								TLex lex(*buf);
								lex.Mark();
								
								TInt dhtEnabled = 0;
								if (lex.Val(dhtEnabled) == KErrNone)
									if (dhtEnabled)
										SetDHTEnabled(ETrue);
									else
										SetDHTEnabled(EFalse);
							}
							
							
						#else
							SetDHTEnabled(EFalse);
						#endif
							
						CleanupStack::PopAndDestroy(); // buf
					}		
					else

					if (settingName == KSettingPieceSize)
					{
						HBufC* psBuf = ReadAndConvertFromUtf8ToUnicodeL(file, settingLength);
						
						if (psBuf)
						{
							TLex lex(*psBuf);
							lex.Mark();
							
							TInt ps = 0;
							if (lex.Val(ps) == KErrNone)
								SetPieceSize(ps);
						}
						
						delete psBuf;
					}
					else
					#endif

					if (settingName == KSettingIncomingPort)
					{
						HBufC* portBuf = ReadAndConvertFromUtf8ToUnicodeL(file, settingLength);
						
						if (portBuf)
						{
							TLex lex(*portBuf);
							lex.Mark();
							
							TInt port = 0;
							if (lex.Val(port) == KErrNone)
								SetIncomingPort(port);
						}
						
						delete portBuf;
					}
					else
					{
						HBufC* unknownSetting = ReadAndConvertFromUtf8ToUnicodeL(file, settingLength);
						delete unknownSetting;
					}
			}
			
			CleanupStack::PopAndDestroy(); // file
		}
		else
			LWRITELN(iLog, _L("Failed to open settings file"));
		
		CleanupStack::PopAndDestroy(); // fs
	}
}

TBool CSTPreferences::ReadSettingName(RFile& aFile, TDes8& aSettingName, TInt& aSettingLength)
{
	aSettingName.SetLength(0);
	
	TBuf8<KMaxSettingNameLength + 10> buffer;
	
	if (aFile.Read(buffer) != KErrNone)
		return EFalse;
	
	TInt filePos = 0;
	if (aFile.Seek(ESeekCurrent, filePos) != KErrNone)
		return EFalse;
	
	TLex8 lex(buffer);
	lex.SkipSpaceAndMark();
	
	while (lex.Peek())
	{
		if (lex.Peek() == ':')
		{
			aSettingName = lex.MarkedToken();
			LWRITELN(iLog, aSettingName);
			lex.Inc();
			break;
		}
		
		lex.Inc();
	}
	
	if (lex.Val(aSettingLength) != KErrNone)
		return EFalse;
	
	//iLog->WriteLineL(aSettingLength);

	while (lex.Peek())
	{
		if (lex.Peek() == '=')
			break;
		
		lex.Inc();
	}
	
	if (lex.Peek() != '=')
		return EFalse;
	
	lex.Inc();
	
	TInt offset = filePos - (buffer.Length() - lex.Offset());
	if (aFile.Seek(ESeekStart, offset) != KErrNone)
		return EFalse;
	
	return ETrue;
}

EXPORT_C void CSTPreferences::SaveSettingsL()
{
	if (iPreferencesFileName)
	{
		LWRITELN(iLog, _L("Saving settings"));
		RFs fs;
		User::LeaveIfError(fs.Connect());
		CleanupClosePushL(fs);
		
		fs.MkDirAll(*iPreferencesFileName);
		
		RFile file;
		if (file.Replace(fs, *iPreferencesFileName, EFileWrite) == KErrNone)
		{
			LWRITELN(iLog, _L("Settings file replaced"));
			CleanupClosePushL(file);
			
			// DownloadPath
			SaveSettingL(file, KSettingDownloadPath, *iDownloadPath);
			
			// AccessPointName
			SaveSettingL(file, KSettingAccesPointName, *iAccessPointName);
			
			// AccessPointId
			{
				TBuf<16> accesPointId;
				accesPointId.Num(TInt(iAccessPointId));
				SaveSettingL(file, KSettingAccesPointId, accesPointId);	
			}
			
			// Right softkey mode	
			{
				TBuf<16> rsmBuf;
				rsmBuf.Num(TInt(iRightSoftkeyMode));
				SaveSettingL(file, KSettingRightSoftkeyMode, rsmBuf);	
			}
			
			// IncomingPort
			{
				TBuf<16> portBuf;
				portBuf.Num(IncomingPort());
				SaveSettingL(file, KSettingIncomingPort, portBuf);	
			}	
			
			#ifdef EKA2
			// TrackerServicePort
			{
				TBuf<16> trackerBuf;
				trackerBuf.Num(TrackerServicePort());
				SaveSettingL(file, KSettingTrackerServicePort, trackerBuf);	
			}

			// PieceSize
			{
				TBuf<16> pieceBuf;
				pieceBuf.Num(PieceSize());
				SaveSettingL(file, KSettingPieceSize, pieceBuf);	
			}
			
			// DHTEnabled
			{
				if (IsDHTEnabled())
					SaveSettingL(file, KSettingDHTEnabled, _L("1"));	
				else
					SaveSettingL(file, KSettingDHTEnabled, _L("0"));
			}
			#endif
			
			// CloseConnectionAfterDownload
			{
				if (CloseConnectionAfterDownload())
						SaveSettingL(file, KSettingCloseConnectionAfterDownload, _L("1"));	
					else
						SaveSettingL(file, KSettingCloseConnectionAfterDownload, _L("0"));					
			}
			
			// Subpiece size	
			{
				TBuf<16> spsBuf;
				spsBuf.Num(iSubpieceSize);
				SaveSettingL(file, KSettingSubpieceSize, spsBuf);	
			}
				
			// ProxyServicePort
			{
				TBuf<16> servicePortBuf;
				servicePortBuf.Num(iProxyServicePort);
				SaveSettingL(file, KSettingProxyServicePort, servicePortBuf);	
			}
			
			// ProxyConnectionPort
			{
				TBuf<16> connPortBuf;
				connPortBuf.Num(iProxyConnectionPort);
				SaveSettingL(file, KSettingProxyConnectionPort, connPortBuf);	
			}
			
			// ProxyHostName
			SaveSettingL(file, KSettingProxyHostName, ProxyHostName());
			
			// IncomingConnectionsMode
			{
				TBuf<16> icmBuf;
				icmBuf.Num(TInt(iIncomingConnectionsMode));
				SaveSettingL(file, KSettingIncomingConnectionsMode, icmBuf);	
			}
			
			// IsUploadEnabled
			{
				if (iIsUploadEnabled)
					SaveSettingL(file, KSettingUploadEnabled, _L("1"));	
				else
					SaveSettingL(file, KSettingUploadEnabled, _L("0"));
			}
			
			// StartupHashCheck
			{
				if (StartupHashCheck())
					SaveSettingL(file, KSettingStartupHashCheck, _L("1"));	
				else
					SaveSettingL(file, KSettingStartupHashCheck, _L("0"));
			}		

					
			for (TInt i=0; i<iSTorrents->Count(); i++)
				SaveSettingL(file, KSettingSTorrent, (*iSTorrents)[i]);
			
			file.Write(KLit8EndLine);
			CleanupStack::PopAndDestroy(); // file
		}
		else
			LWRITELN(iLog, _L("Failed to replace settings file"));
		
		CleanupStack::PopAndDestroy(); // fs	
	}
}

void CSTPreferences::SaveSettingL(RFile& aFile, const TDesC8& aSettingName, const TDesC& aSettingValue)
{
	aFile.Write(aSettingName);
	aFile.Write(KLit8Colon);
	
	
	HBufC8* encodedSettingValue = ConvertToUtf8L(aSettingValue);
	if (encodedSettingValue)
	{
		CleanupStack::PushL(encodedSettingValue);
		
		TBuf8<16> settingLength;
		settingLength.Num(encodedSettingValue->Length());
		aFile.Write(settingLength);
		
		aFile.Write(KLit8EqualSign);
		aFile.Write(*encodedSettingValue);
		CleanupStack::PopAndDestroy(); // encodedSettingValue
	}
	else
	{
		_LIT8(KLit8Unset, "0=");
		aFile.Write(KLit8Unset);
	}
	
	aFile.Write(KLit8EndLine);
}

HBufC8* CSTPreferences::ConvertToUtf8L(const TDesC& aData)
{	
	HBufC8* encodedData = HBufC8::NewLC(aData.Length() * 3);
	
	TPtr8 encodedDataPtr = encodedData->Des();
	if (CnvUtfConverter::ConvertFromUnicodeToUtf8(encodedDataPtr, aData) != 0)
	{
		CleanupStack::PopAndDestroy(); // encodedData
		return NULL;
	}
	
	CleanupStack::Pop(); // encodedData
		
	return encodedData;
}

HBufC* CSTPreferences::ReadAndConvertFromUtf8ToUnicodeL(RFile& aFile, TInt aLength)
{
	HBufC8* utfBuffer = HBufC8::NewLC(aLength);
	TPtr8 utfBufferPtr = utfBuffer->Des();
	
	if ((aFile.Read(utfBufferPtr, aLength) != KErrNone) ||
		(utfBuffer->Length() != aLength))
	{
		CleanupStack::PopAndDestroy(); // utfBuffer
		return NULL;
	}			
	
	HBufC* unicodeBuffer = HBufC::NewLC(aLength);
	TPtr unicodeBufferPtr = unicodeBuffer->Des();
	
	if (CnvUtfConverter::ConvertToUnicodeFromUtf8(unicodeBufferPtr, *utfBuffer) != KErrNone)
	{
		CleanupStack::PopAndDestroy(); // unicodeBuffer
		CleanupStack::PopAndDestroy(); // utfBuffer
		return NULL;
	}
	
	CleanupStack::Pop(); // unicodeBuffer
	CleanupStack::PopAndDestroy(); // utfBuffer

	return unicodeBuffer;
}

EXPORT_C TInt CSTPreferences::SetProxyAddressL(const TDesC& aHostName)
{
	TInt res = iProxyAddress.Input(aHostName);
	
	if (res == KErrNone)
	{
		delete iProxyHostName;
		iProxyHostName = NULL;
		iProxyHostName = aHostName.AllocL();
		
		NotifyObserversL(ESettingProxyHostName);
	}
	
	return res;
}

EXPORT_C void CSTPreferences::SetUploadEnabled(TBool aUploadEnabled)
{
	iIsUploadEnabled = aUploadEnabled;
	NotifyObserversL(ESettingUploadEnabled);
}

EXPORT_C void CSTPreferences::SetDHTEnabled(TBool aDHTEnabled)
{
	iDHTEnabled = aDHTEnabled;
	NotifyObserversL(ESettingDHTEnabled);
}
 
EXPORT_C void CSTPreferences::SetIncomingConnectionsMode(CNetworkManager::TIncomingConnectionsMode aMode)
{
	iIncomingConnectionsMode = aMode;
	NotifyObserversL(ESettingIncomingConnectionsMode);
}

EXPORT_C void CSTPreferences::SetDownloadPathL(TFileName& aDownloadPath)
{
	delete iDownloadPath;
	iDownloadPath = 0;
	iDownloadPath = aDownloadPath.AllocL();
	NotifyObserversL(ESettingDownloadPath);
}

EXPORT_C void CSTPreferences::SetProxyServicePort(TInt aProxyServicePort) {
	iProxyServicePort = aProxyServicePort;
	NotifyObserversL(ESettingProxyServicePort);
}
	
EXPORT_C void CSTPreferences::SetProxyConnectionPort(TInt aProxyConnectionPort) {
	iProxyConnectionPort = aProxyConnectionPort;
	NotifyObserversL(ESettingProxyConnectionPort);
}

EXPORT_C void CSTPreferences::SetIncomingPort(TInt aIncomingPort) {	
		iIncomingPort = aIncomingPort;
		NotifyObserversL(ESettingIncomingPort);
}

#ifdef EKA2
EXPORT_C void CSTPreferences::SetTrackerServicePort(TInt aTrackerServicePort) {
	iTrackerServicePort=aTrackerServicePort;
	NotifyObserversL(ESettingTrackerServicePort);
}

EXPORT_C void CSTPreferences::SetPieceSize(TInt aPieceSize) {
	iPieceSize = aPieceSize;
	NotifyObserversL(ESettingPieceSize);
}
#endif

EXPORT_C void CSTPreferences::SetRightSoftkeyMode(TSTSoftkeyMode aMode) {
	iRightSoftkeyMode = aMode;
	NotifyObserversL(ESettingRightSoftkeyMode);
}

EXPORT_C void CSTPreferences::SetStartupHashCheck(TBool aHashCheck) {
	iStartupHashCheck = aHashCheck;
	NotifyObserversL(ESettingStartupHashCheck);
}

EXPORT_C void CSTPreferences::SetCloseConnectionAfterDownload(TBool aClose) {
	iCloseConnectionAfterDownload = aClose;
	NotifyObserversL(ESettingCloseConnectionAfterDownload);
}

EXPORT_C void CSTPreferences::RemovePreferencesObserver(MSTPreferencesObserver* aObserver)
{
	TInt index = iPreferencesObservers.Find(aObserver);
	
	if (index >= 0)
		iPreferencesObservers.Remove(index);
}

EXPORT_C void CSTPreferences::SetAccessPointL(const TDesC& aAccessPointName, TUint32 aAccessPointId) 
{
	delete iAccessPointName;
	iAccessPointName = 0;
	iAccessPointName = aAccessPointName.AllocL();
	
	iAccessPointId = aAccessPointId;
	
	NotifyObserversL(ESettingAccesPointId);
	NotifyObserversL(ESettingAccesPointName);
}

EXPORT_C void CSTPreferences::AddSTorrentL(const TDesC& aSTorrentPath)
{
	TInt dummy = 0;
	
	if (iSTorrents->Find(aSTorrentPath, dummy) == 0) // the storrent is alreasy in the list
		return;
	
	iSTorrents->AppendL(aSTorrentPath);
	
	NotifyObserversL(ESettingSTorrent);
}

EXPORT_C void CSTPreferences::RemoveSTorrentL(const TDesC& aSTorrentPath)
{
	TInt pos = 0;
	
	if (iSTorrents->Find(aSTorrentPath, pos) != 0) // the storrent is not found
		return;
	
	iSTorrents->Delete(pos);
	
	NotifyObserversL(ESettingSTorrent);	
}

void CSTPreferences::NotifyObserversL(TSymTorrentSetting aSettingChanged)
{
	for (TInt i=0; i<iPreferencesObservers.Count(); i++)
		iPreferencesObservers[i]->SettingChangedL(aSettingChanged);
}

EXPORT_C void CSTPreferences::SetSubpieceSize(TInt aSubpieceSize)
{
	iSubpieceSize = aSubpieceSize;
	NotifyObserversL(ESettingSubpieceSize);
}
