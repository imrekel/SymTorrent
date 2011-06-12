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

#include "FileSelectionSettingItem.h"
#include <aknlists.h> 
#include <avkon.hrh>
#include <aknPopup.h>
#include <caknfileselectiondialog.h>
#include <caknmemoryselectiondialog.h> 


CFileSelectionSettingItem::CFileSelectionSettingItem(TInt aIdentifier, TFileName& aFileName, TCommonDialogType aDialogType) :
	CAknTextSettingItem(aIdentifier, aFileName),
	iFileName(aFileName),
	iDialogType(aDialogType)
{		
}

void CFileSelectionSettingItem::LoadL() 
{ 
	CAknTextSettingItem::LoadL();
}

const TDesC& CFileSelectionSettingItem::SettingTextL( ) 
{
	return iFileName;
}

void CFileSelectionSettingItem::EditItemL(TBool /*aCalledFromMenu*/)
{
	// Select memory
	CAknMemorySelectionDialog* memSelectionDialog = CAknMemorySelectionDialog::NewL
		(ECFDDialogTypeNormal, /*aShowUnavailableDrives*/EFalse);
	CleanupStack::PushL(memSelectionDialog);

	CAknMemorySelectionDialog::TMemory mem(CAknMemorySelectionDialog::EPhoneMemory);

	TInt ret = memSelectionDialog->ExecuteL(mem);
	CleanupStack::PopAndDestroy(memSelectionDialog);
	if (!ret) return;

	CAknFileSelectionDialog* fileSelectionDialog = 
		CAknFileSelectionDialog::NewL(iDialogType);	
	CleanupStack::PushL(fileSelectionDialog);
	
	fileSelectionDialog->SetTitleL(_L("Download files to"));
	fileSelectionDialog->SetLeftSoftkeyFileL(_L("Select"));	
	
	if (mem == CAknMemorySelectionDialog::EMemoryCard)	
		iOwnedFileName = _L("E:\\");
	else
		iOwnedFileName = _L("C:\\");

	// launch file select dialog
	TBool result = fileSelectionDialog->ExecuteL(iOwnedFileName);
	CleanupStack::PopAndDestroy(); // fileSelectionDialog
	
	if (result)
		iFileName = iOwnedFileName;
}

