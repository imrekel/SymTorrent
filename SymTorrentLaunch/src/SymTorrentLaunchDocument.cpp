/*
============================================================================
 Name        : CSymTorrentLaunchDocument from SymTorrentLaunchDocument.h
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : CSymTorrentLaunchDocument implementation
============================================================================
*/

// INCLUDE FILES
#include "SymTorrentLaunchDocument.h"
#include "SymTorrentLaunchAppui.h"
#include <utf.h>
#include <APGCLI.H>
#include <e32math.h>

_LIT(KTempTorrentFileLocation,"C:\\Data\\BitTorrent\\");
_LIT(KTempTorrentFileName,"C:\\Data\\BitTorrent\\MyTorrent%d.torrent");

// sendmessage through file constant(s)
_LIT(KNotifyFile, "C:\\Data\\BitTorrent\\Notify\\STNotify.nt");
_LIT(KNotifyFileMod, "C:\\Data\\BitTorrent\\Notify\\STNotifyMod.nt");
_LIT(KMessageFile, "C:\\Data\\BitTorrent\\Notify\\STNotifyMessage%d.mnt");

// ================= MEMBER FUNCTIONS =======================

// constructor
CSymTorrentLaunchDocument::CSymTorrentLaunchDocument(CEikApplication& aApp)
: CAknDocument(aApp)    
    {
    }
    
// destructor
CSymTorrentLaunchDocument::~CSymTorrentLaunchDocument()
    {
    }

// EPOC default constructor can leave.
void CSymTorrentLaunchDocument::ConstructL()
    {
    }

// Two-phased constructor.
CSymTorrentLaunchDocument* CSymTorrentLaunchDocument::NewL(
        CEikApplication& aApp)     // CSymTorrentLaunchApp reference
    {
    CSymTorrentLaunchDocument* self = new (ELeave) CSymTorrentLaunchDocument( aApp );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop();

    return self;
    }
    
// ----------------------------------------------------
// CSymTorrentLaunchDocument::CreateAppUiL()
// constructs CSymTorrentLaunchAppUi
// ----------------------------------------------------
//
CEikAppUi* CSymTorrentLaunchDocument::CreateAppUiL()
    {
    return new (ELeave) CSymTorrentLaunchAppUi;
    }

// This function calls, when a torrent file is select in the filebrowser, ...
void CSymTorrentLaunchDocument::OpenFileL(CFileStore*& /*aFileStore*/, RFile& aFile)
	{	
		TFileName filename;
		aFile.FullName(filename);
		
		TFileName tempFilename;
		tempFilename.Copy(filename);		
		tempFilename.LowerCase();
		
		// If the torrent file is in a Private directory than we copy it to C:\BitTorrent
		if (tempFilename.FindF(_L("private")) != KErrNotFound || tempFilename.FindF(_L("system")) != KErrNotFound)
		{			
			RFs fileServer;
			RFile targetFile, tempFile;
			TInt remaining, nextpiece, copied, i;

			// connect to fileserver
			User::LeaveIfError(fileServer.Connect());
			// create directory
			fileServer.MkDirAll(KTempTorrentFileLocation);

			// copy the file content to a new file
			// if the new filename already exist than generate a newer one
			TBuf<200> newFileName;
			TInt error;
			i=0;
			newFileName.Format(KTempTorrentFileName,i);
			error=tempFile.Open(fileServer,newFileName,EFileWrite);
			while (error != KErrNotFound)
			{
				tempFile.Close();
				i++;
				newFileName.Format(KTempTorrentFileName,i);
				error=tempFile.Open(fileServer,newFileName,EFileWrite);				
			}				
				
			// create the new torrent file
			TInt err = targetFile.Replace(fileServer,newFileName,EFileWrite);
			if (err !=KErrNone) return;

			// copy the content to the new file
			HBufC8* copybuf = HBufC8::NewL(4096);
			TPtr8 pcopybuf = copybuf->Des();

			copied = 0;
			err = aFile.Size(remaining);
			if (err !=KErrNone) return;

			while (remaining>0)
			{
				if (remaining>4096)
					nextpiece = 4096;
				else
					nextpiece = remaining;

				aFile.Read(copied, pcopybuf, nextpiece);
				targetFile.Write(copied, pcopybuf, nextpiece);

				copied	  += nextpiece;
				remaining -= nextpiece;
			}
			delete copybuf;
			targetFile.Close();
			fileServer.Close();

			// calls SymTorrent
			HandleRecognizeL(newFileName);
		}
		else
		{
			// calls SymTorrent
			HandleRecognizeL(filename);
		}		
	}
	
// Switchs to SymTorrent
void CSymTorrentLaunchDocument::HandleRecognizeL(TFileName aFilename)
{	
	// check if SymTorrent is already running
	TApaTaskList taskList(CEikonEnv::Static()->WsSession());
	
	TApaTask task = taskList.FindApp(_L("SymTorrent"));
	
	if (!task.Exists())
	{		
		RApaLsSession appArcSession;
		TThreadId thread_id = 0;
		HBufC * param  ;
		    
		param = HBufC::NewLC( aFilename.Length() );
		param->Des().Copy(aFilename);

		User::LeaveIfError(appArcSession.Connect()); // connect to AppArc server
		appArcSession.StartDocument( *param, TUid::Uid( 0xA0001751 ), thread_id );
		appArcSession.Close();

		CleanupStack::PopAndDestroy();
	}
	else
	{						
		// send the filename utf8 encoded
		HBufC8* encodedFilename = HBufC8::NewLC(aFilename.Length() * 3);
		
		TPtr8 encodedFilenamePtr = encodedFilename->Des();
		if (CnvUtfConverter::ConvertFromUnicodeToUtf8(encodedFilenamePtr, aFilename) == 0)
		{		
			RFs fileServer;			
			RFile tempFile, existFile;
			User::LeaveIfError(fileServer.Connect());			

			TBuf<200> filename;

			/// initialize seed
			TTime time;
			time.HomeTime();

			TInt64 smallRandSeed=time.Int64();

			TInt number = Math::Rand(smallRandSeed);
			filename.Format(KMessageFile,number);
			
			fileServer.MkDirAll(filename);			
						
			// creating random file, if the random file exist, we generate a new one
			TInt error=existFile.Open(fileServer,filename,EFileWrite);
			while (error != KErrNotFound)
			{
				existFile.Close();
				number = Math::Rand(smallRandSeed);	
				filename.Format(KMessageFile,number);							
				error=existFile.Open(fileServer,filename,EFileWrite);				
			}						
			
			if (tempFile.Replace(fileServer,filename,EFileWrite)==KErrNone)
			{
				tempFile.Write(*encodedFilename);
				tempFile.Flush();
				tempFile.Close();
			}
		
			fileServer.Rename(KNotifyFile,KNotifyFileMod);	
			fileServer.Close();						
		}
		
		CleanupStack::PopAndDestroy(); // encodedFilename
		
		task.BringToForeground();
	}
	
	CEikAppUi* appUi = CEikonEnv::Static()->EikAppUi();
	if (appUi)
		appUi->HandleCommandL(EEikCmdExit);	
}
