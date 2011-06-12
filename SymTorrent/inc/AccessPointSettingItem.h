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

#ifndef ACCESSPOINTSETTINGITEM_H__
#define ACCESSPOINTSETTINGITEM_H__

#include <aknsettingitemlist.h>
#include <commdb.h>

class CAccessPointSettingItem : public CAknTextSettingItem
{
	public:
		/**
		 * the length aApName must be at least KCommsDbSvrMaxFieldLength long
		 */
		CAccessPointSettingItem(TInt aIdentifier, TInt32 &aDefaultAp, TDes& aApName);
		virtual void EditItemL(TBool aCalledFromMenu);
		
		TBool AskIapIdL(TInt32& aIap, TBool aAddAllwaysAsk = EFalse);
		TBool AskIapIdL(TInt32& aIap, TDes& aAccesPointName, TBool aAddAllwaysAsk = EFalse);
		
		void LoadL();
		
	protected:
		
		const TDesC& SettingTextL() ;

	private:
	
		void CreateApNameFromId();
		HBufC* GetIapNameL(TUint32 aIapId);
		void GetIapNamesAndIdsL( RArray<TUint32>& aIds, CDesC16Array& aNames ); 

		TInt32& iDefaultAp;
		TDes& iApName;
};

#endif
