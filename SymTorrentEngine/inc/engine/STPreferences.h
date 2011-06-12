/*****************************************************************************
 * Copyright (C) 2006-2007 Imre Kelényi
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

/*
 * ============================================================================
 *  Classes  : CSTPreferences			  
 *			  
 *  Part of  : SymTorrentEngine
 *  Created  : 04.02.2006 by Imre Kelényi
 * ============================================================================
 */

#ifndef SYMTORRENT_STPREFERENCES_H
#define SYMTORRENT_STPREFERENCES_H

// INCLUDES
#include <e32base.h>
#include <f32file.h>
#include <badesca.h>
#include <in_sock.h>
#include "STDefs.h"
#include "NetworkManager.h"

// FORWARD DECLARATIONS
class CKiLogger;

/**
 * Defines the functions the can be assigned to the left SoftKey in the UI.
 */
enum TSTSoftkeyMode
{
	ESTSoftkeyExit = 0,
	ESTSoftkeyHide
};

/**
 * An anumeration that defines the application's settings
 */
enum TSymTorrentSetting
{
	ESettingDownloadPath = 0,
	ESettingSTorrent,
	ESettingAccesPointId,
	ESettingAccesPointName,
	ESettingProxyHostName,
	ESettingProxyConnectionPort,
	ESettingProxyServicePort,
	ESettingIncomingConnectionsMode,
	ESettingUploadEnabled,
	ESettingIncomingPort,
	ESettingRightSoftkeyMode,
	ESettingStartupHashCheck,
	ESettingCloseConnectionAfterDownload,
	ESettingSubpieceSize
	#ifdef EKA2
	,
	ESettingTrackerServicePort,
	ESettingPieceSize,
	ESettingDHTEnabled
	#endif
};

/**
 * MSTPreferencesObserver
 *
 * Class for observing changes in the preferences
 */
class MSTPreferencesObserver
{
public:
	/**
	 * Called when one of the settings changed
	 *
	 * @param aSettingChanged the setting changed
	 */
	virtual void SettingChangedL(TSymTorrentSetting aSetting) = 0;
};

/**
 * CSTPreferences
 *
 * This class represents the settings of SymTorrent's engine.
 * It can be used to get/set the various properties of the application.
 *
 * There should be only one instance of this class (owned by the
 * singleton class CSTTorrentManager).
 */
class CSTPreferences : public CBase
{
public:		

	void ConstructL();
	
	~CSTPreferences();
	
	IMPORT_C void SetPreferencesFileL(const TDesC& aFileName);
	
	inline void AddPreferencesObserverL(MSTPreferencesObserver* aObserver);
	
	IMPORT_C void RemovePreferencesObserver(MSTPreferencesObserver* aObserver);

public: // getters
	
	/**
	 * @return the path, where the files are downloaded to
	 */
	inline const TDesC& DownloadPath() const;
	
	/**
	 * @return the name of the currently selected internet access point or KNullDesC if
	 * it's unset
	 */
	inline const TDesC& AccessPointName() const;
	
	inline TUint32 AccessPointId() const;
	
	inline const CDesCArrayFlat* STorrents() const;
	
	inline const TDesC& ProxyHostName() const;
	
	inline TInetAddr ProxyAddress() const;
	
	inline TInt ProxyServicePort() const;
	
	inline TInt ProxyConnectionPort() const;

	#ifdef EKA2
	inline TInt TrackerServicePort() const;

	inline TInt PieceSize() const;
	
	inline TBool IsDHTEnabled() const;
	#endif
	
	inline TInt IncomingPort() const;
	
	inline TBool IsUploadEnabled() const;
	
	inline CNetworkManager::TIncomingConnectionsMode IncomingConnectionsMode() const;
	
	inline TSTSoftkeyMode RightSoftkeyMode() const;
	
	inline TBool StartupHashCheck() const;
	
	inline TBool CloseConnectionAfterDownload() const;
	
	/**
	 * Subpiece size in bytes. 0 means AUTO.
	 */
	inline TInt SubpieceSize() const;
	
public: // setters

	IMPORT_C void AddSTorrentL(const TDesC& aSTorrentPath);
	
	IMPORT_C void RemoveSTorrentL(const TDesC& aSTorrentPath);
	
	IMPORT_C void SetAccessPointL(const TDesC& aAccessPointName, TUint32 aAccessPointId);
	
	/**
	 * Sets the proxy's hostname and address. 
	 * 
	 * @return the result of parsing the hostname (KErrNone on success)
	 */
	IMPORT_C TInt SetProxyAddressL(const TDesC& aHostName);
		
	IMPORT_C void SetProxyServicePort(TInt aProxyServicePort);
	
	IMPORT_C void SetProxyConnectionPort(TInt aProxyConnectionPort);
	
	IMPORT_C void SetIncomingPort(TInt aIncomingPort);

	#ifdef EKA2
	IMPORT_C void SetTrackerServicePort(TInt aTrackerServicePort);

	IMPORT_C void SetPieceSize(TInt aPieceSize);
	
	IMPORT_C void SetDHTEnabled(TBool aDHTEnabled);
	#endif
	
	IMPORT_C void SetUploadEnabled(TBool aUploadEnabled);
	
	IMPORT_C void SetIncomingConnectionsMode(CNetworkManager::TIncomingConnectionsMode aMode);
	
	IMPORT_C void SetDownloadPathL(TFileName& aDownloadPath);
	
	IMPORT_C void SetRightSoftkeyMode(TSTSoftkeyMode aMode);
	
	IMPORT_C void SetStartupHashCheck(TBool aStartupHashCheck);
	
	IMPORT_C void SetCloseConnectionAfterDownload(TBool aClose);
	
	/**
	 * Sets the preferred subpiece size. 0 means AUTO.
	 */
	IMPORT_C void SetSubpieceSize(TInt aSubpieceSize);
	
public:
	
	IMPORT_C void SaveSettingsL();
	
	/**
	 * Loads the settings from the saved settins file
	 */
	IMPORT_C void LoadSettingsL();
	
	/**
	 * Resets to default settings
	 */
	IMPORT_C void ResetSettingsL();
	
private:
	
	void SaveSettingL(RFile& aFile, const TDesC8& aSettingName, const TDesC& aSettingValue);
	
	HBufC8* ConvertToUtf8L(const TDesC& aData);
	
	HBufC* ReadAndConvertFromUtf8ToUnicodeL(RFile& aFile, TInt aLength);	
	
	TBool ReadSettingName(RFile& aFile, TDes8& aSettingName, TInt& aSettingLength);
	
	void NotifyObserversL(TSymTorrentSetting aSettingChanged);

private:

	/**
	 * Reference to the application's log
	 */
	CKiLogger* iLog;
	
	HBufC* iDownloadPath;
	
	HBufC* iAccessPointName;
	
	TUint32 iAccessPointId;
	
	TInt iProxyServicePort;
	
	TInt iProxyConnectionPort;
	
	TInt iIncomingPort;
	
	TInetAddr iProxyAddress;
	
	HBufC* iProxyHostName;
	
	TBool iStartupHashCheck;
	
	TBool iDHTEnabled;
	
	TBool iCloseConnectionAfterDownload;

	#ifdef EKA2
	TInt iTrackerServicePort;

	TInt iPieceSize;
	#endif
	
	CDesCArrayFlat* iSTorrents;
	
	CNetworkManager::TIncomingConnectionsMode iIncomingConnectionsMode;
	
	TBool iIsUploadEnabled;
	
	TSTSoftkeyMode iRightSoftkeyMode;
	
	HBufC* iPreferencesFileName;
	
	RPointerArray<MSTPreferencesObserver> iPreferencesObservers;
	
	TInt iSubpieceSize;
};


// INLINE FUNCTION IMPLEMENTATIONS

inline const TDesC& CSTPreferences::DownloadPath() const {
	return *iDownloadPath;	
}

inline const TDesC& CSTPreferences::AccessPointName() const {
	if (iAccessPointName)
		return *iAccessPointName;
	return KNullDesC;
}
	
inline TUint32 CSTPreferences::AccessPointId() const {
	return iAccessPointId;
}

inline const CDesCArrayFlat* CSTPreferences::STorrents() const {
	return iSTorrents;
}

inline const TDesC& CSTPreferences::ProxyHostName() const {
	if (iProxyHostName)
		return *iProxyHostName;
	return KNullDesC;
}

inline TInetAddr CSTPreferences::ProxyAddress() const {
	return iProxyAddress;
}
	
inline TInt CSTPreferences::ProxyServicePort() const {
	return iProxyServicePort;
}
	
inline TInt CSTPreferences::ProxyConnectionPort() const {
	return iProxyConnectionPort;
}

#ifdef EKA2
inline TInt CSTPreferences::TrackerServicePort() const {
	return iTrackerServicePort;
}

inline TInt CSTPreferences::PieceSize() const {
	return iPieceSize;
}

inline TBool CSTPreferences::IsDHTEnabled() const {
	return iDHTEnabled;
}
#endif
	
inline TBool CSTPreferences::IsUploadEnabled() const {
	return iIsUploadEnabled;
}
	
inline CNetworkManager::TIncomingConnectionsMode CSTPreferences::IncomingConnectionsMode() const {
	return iIncomingConnectionsMode;
}

inline TInt CSTPreferences::IncomingPort() const {
	return iIncomingPort;
}

inline TSTSoftkeyMode CSTPreferences::RightSoftkeyMode() const {
	return iRightSoftkeyMode;
}

inline void CSTPreferences::AddPreferencesObserverL(MSTPreferencesObserver* aObserver) {
	User::LeaveIfError(iPreferencesObservers.Append(aObserver));
}

inline TBool CSTPreferences::StartupHashCheck() const {
	return iStartupHashCheck;
}

inline TBool CSTPreferences::CloseConnectionAfterDownload() const {
	return iCloseConnectionAfterDownload;
}

inline TInt CSTPreferences::SubpieceSize() const {
	return iSubpieceSize;
}

#endif
