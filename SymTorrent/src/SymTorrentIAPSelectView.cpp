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

// INCLUDE FILES
#include <aknviewappui.h>
#include <avkon.hrh>
#include <MobileAgent.rsg>
#include <commdb.h>
#include "SymTorrentIAPSelectView.h"
#include "SymTorrentIAPSelectContainer.h"

#include "STGlobal.h"

#include "STSocketManager.h"


// ================= MEMBER FUNCTIONS =======================


void CSymTorrentIAPSelectView::ConstructL()
{
    BaseConstructL( R_SYMTORRENT_IAPSELECTVIEW );	
}

// ---------------------------------------------------------
// CSymTorrentIAPSelectView::~CSymTorrentIAPSelectView()
// destructor
// ---------------------------------------------------------
//
CSymTorrentIAPSelectView::~CSymTorrentIAPSelectView()
{
    if ( iContainer )
    {
		AppUi()->RemoveFromViewStack( *this, iContainer );
    }

    delete iContainer;
    iIAPIDs.Close();
}

// ---------------------------------------------------------
// TUid CSymTorrentIAPSelectView::Id()
//
// ---------------------------------------------------------
//
TUid CSymTorrentIAPSelectView::Id() const
{
    return KIAPSelectViewId;
}

// ---------------------------------------------------------
// CSymTorrentIAPSelectView::HandleCommandL(TInt aCommand)
// takes care of view command handling
// ---------------------------------------------------------
//
void CSymTorrentIAPSelectView::HandleCommandL(TInt aCommand)
{   
    switch ( aCommand )
    {	
    	case EAknSoftkeySelect:	
        case EAknSoftkeyOk:
        {        
        	if (iContainer->CurrentItemIndex() >= 0)
				DOCUMENT->SetIAPIDL(iIAPIDs[iContainer->CurrentItemIndex()]);
			AppUi()->ActivateLocalViewL(KMainViewId);
			break;
        }

		case EAknSoftkeyCancel:	
        case EAknSoftkeyBack:
        {
			AppUi()->ActivateLocalViewL(KMainViewId);
			break;
        }

        default:
        {		
			break;
        }
    }
}

// ---------------------------------------------------------
// CSymTorrentIAPSelectView::HandleClientRectChange()
// ---------------------------------------------------------
//
void CSymTorrentIAPSelectView::HandleClientRectChange()
{
    if ( iContainer )
    {
        iContainer->SetRect( ClientRect() );
    }
}

// ---------------------------------------------------------
// CSymTorrentIAPSelectView::DoActivateL(...)
// 
// ---------------------------------------------------------
//
void CSymTorrentIAPSelectView::DoActivateL(
   const TVwsViewId& /*aPrevViewId*/,TUid /*aCustomMessageId*/,
   const TDesC8& /*aCustomMessage*/)
{
	APPUI->SetActiveView(this);
	
    if (!iContainer)
    {
        iContainer = new (ELeave) CSymTorrentIAPSelectContainer;
        iContainer->SetMopParent(this);
        iContainer->ConstructL( ClientRect() );
        
        GetGPRSIAPsFromIAPTableL();                
        
        AppUi()->AddToStackL( *this, iContainer );
	}
}

// ---------------------------------------------------------
// CSymTorrentIAPSelectView::DoDeactivate()
// 
// ---------------------------------------------------------
//
void CSymTorrentIAPSelectView::DoDeactivate()
{
    if ( iContainer )
    {
        AppUi()->RemoveFromViewStack( *this, iContainer );
        
        delete iContainer;
    	iContainer = NULL;
    }
    APPUI->SetPrevView(this);
}


void CSymTorrentIAPSelectView::GetGPRSIAPsFromIAPTableL()
{ 
	TBuf<52> iapfromtable;
	TBuf<53> listItem;
	TInt err = KErrNone;
	
	CCommsDatabase* commsDB = CCommsDatabase::NewL(EDatabaseTypeIAP);
	CleanupStack::PushL(commsDB);

	// Open IAP table using the GPRS as the bearer.
	CCommsDbTableView* gprsTable = commsDB->OpenIAPTableViewMatchingBearerSetLC(ECommDbBearerGPRS,
		ECommDbConnectionDirectionOutgoing);
	// Point to the first entry
	User::LeaveIfError(gprsTable->GotoFirstRecord());
	gprsTable->ReadTextL(TPtrC(COMMDB_NAME), iapfromtable);
	listItem.SetLength(0);
	listItem.Append(_L("\t"));
	listItem.Append(iapfromtable);
	iContainer->AppendItemL(listItem);	
	
	TUint32 id;
	gprsTable->ReadUintL(TPtrC(COMMDB_ID), id);
	User::LeaveIfError(iIAPIDs.Append(id));

	while (err = gprsTable->GotoNextRecord(), err == KErrNone)
	{
		gprsTable->ReadTextL(TPtrC(COMMDB_NAME), iapfromtable);
		listItem.SetLength(0);
		listItem.Append(_L("\t"));
		listItem.Append(iapfromtable);
		iContainer->AppendItemL(listItem);
			
		TUint32 id;
		gprsTable->ReadUintL(TPtrC(COMMDB_ID), id);
		User::LeaveIfError(iIAPIDs.Append(id));
	}

	CleanupStack::PopAndDestroy(); // gprsTable
	CleanupStack::PopAndDestroy(); // commsDB
}


// End of File

