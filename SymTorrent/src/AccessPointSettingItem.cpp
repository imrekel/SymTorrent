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

#include "AccessPointSettingItem.h"
#include <aknlists.h> 
#include <avkon.hrh>
#include <aknPopup.h>

CAccessPointSettingItem::CAccessPointSettingItem(TInt aIdentifier, TInt32 &aDefaultAp, TDes& aApName) :
	CAknTextSettingItem(aIdentifier, aApName), iDefaultAp(aDefaultAp), iApName(aApName)
	{}

void CAccessPointSettingItem::LoadL() 
{ 
	CreateApNameFromId(); 	
	CAknTextSettingItem::LoadL();
}

const TDesC& CAccessPointSettingItem::SettingTextL( ) 
{
	return iApName;
}

void CAccessPointSettingItem::CreateApNameFromId()
{
	HBufC* name = NULL;
	TRAPD(err, name = GetIapNameL(iDefaultAp));
	_LIT(KLitNoAccessPoint, "Always ask");
	if (err)
	{
		iApName = KLitNoAccessPoint;
		iDefaultAp = 0;
	}
	else
		iApName.Copy(*name);
		
	delete name;
}

void CAccessPointSettingItem::EditItemL(TBool /*aCalledFromMenu*/)
{
	if (AskIapIdL(iDefaultAp, ETrue))
	{
		CreateApNameFromId();
		LoadL();
	}
}

// !!!! MUST BE REVISED !!!!
HBufC* CAccessPointSettingItem::GetIapNameL( TUint32 aIapId )
	{
	TBool foundAP( EFalse );
    RArray<TUint32> idArray;
	CleanupClosePushL( idArray );
    CDesCArrayFlat* namesArray = new ( ELeave ) CDesCArrayFlat( 5 );
	CleanupStack::PushL( namesArray );
	GetIapNamesAndIdsL( idArray, *namesArray );

	HBufC* apName = NULL;
	for (TInt i = 0; i < idArray.Count(); i++)
		{
		if (idArray[i] == aIapId)
			{
			apName = (*namesArray)[i].AllocLC();
			foundAP = ETrue;
			break;
			}
		}
	if ( !foundAP )
		{
		User::Leave(KErrNotFound);		// Name was not found
		}


	CleanupStack::Pop();				// apName
    CleanupStack::PopAndDestroy( 2 );	// commsDb and view
	return apName;
	}

// !!!! MUST BE REVISED !!!!
void CAccessPointSettingItem::GetIapNamesAndIdsL( RArray<TUint32>& aIds, CDesC16Array& aNames )
	{
	CCommsDatabase* commsDb = CCommsDatabase::NewL();
	CleanupStack::PushL( commsDb );
    
	CCommsDbTableView* view = commsDb->OpenTableLC( TPtrC( IAP ) );
    TInt res = view->GotoFirstRecord();

    while (res == KErrNone)
		{
		TBuf<KCommsDbSvrMaxFieldLength> name;
		TUint32 id;
        view->ReadTextL(TPtrC(COMMDB_NAME), name);
        view->ReadUintL(TPtrC(COMMDB_ID), id);

	    aIds.Insert(id, 0);
		aNames.InsertL(0, name);

        res = view->GotoNextRecord();
		}
	CleanupStack::PopAndDestroy(2);
	}

	
TBool CAccessPointSettingItem::AskIapIdL(TInt32& aIap, TBool aAddAlwaysAsk)
{
	TBuf<200> buf;
	return AskIapIdL(aIap, buf, aAddAlwaysAsk);
}


// !!!! MUST BE REVISED !!!!
TBool CAccessPointSettingItem::AskIapIdL(TInt32& aIap, TDes& aAccesPointName, TBool aAddAlwaysAsk)
    {
    RArray<TUint32> idArray;
	CleanupClosePushL(idArray);

    CDesCArrayFlat* namesArray = new (ELeave) CDesCArrayFlat(5);
	CleanupStack::PushL( namesArray );

	// get access points
	GetIapNamesAndIdsL(idArray, *namesArray);
	
	if (aAddAlwaysAsk)
	{
		// add always ask
		idArray.Insert((TUint32)-1, 0);
		namesArray->InsertL(0, _L("Always ask"));
	}	

    CEikTextListBox* list = new (ELeave) CAknSinglePopupMenuStyleListBox;
    CleanupStack::PushL(list);
    
    CAknPopupList* popupList = CAknPopupList::NewL(list, R_AVKON_SOFTKEYS_OK_CANCEL,AknPopupLayouts::EMenuWindow);
    CleanupStack::PushL(popupList);
    
    // initialize listbox.
    list->ConstructL(popupList, CEikListBox::ELeftDownInViewRect);
    list->CreateScrollBarFrameL(ETrue);
    list->ScrollBarFrame()->SetScrollBarVisibilityL(CEikScrollBarFrame::EOff, CEikScrollBarFrame::EAuto);

    // Set listitems.
    CTextListBoxModel* model = list->Model();
    model->SetItemTextArray(namesArray);
    model->SetOwnershipType(ELbmDoesNotOwnItemArray);
    
    // Set title
    popupList->SetTitleL(_L("Select connection"));

    // Show popup list.
    TBool changed = popupList->ExecuteLD();
	CleanupStack::Pop();							// popuplist

	if (changed)
	{
		aIap = (TUint32)idArray[list->CurrentItemIndex()];
		TPtrC iapName = (*namesArray)[list->CurrentItemIndex()];
		if (iapName.Length() <= aAccesPointName.MaxLength())
			aAccesPointName = (*namesArray)[list->CurrentItemIndex()];
	}
	CleanupStack::PopAndDestroy(3);					// list, namesArray, idArray
	return changed;
    }
