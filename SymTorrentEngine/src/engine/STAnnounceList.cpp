/*****************************************************************************
 * Copyright (C) 2006,2007 Imre Kelényi
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

#include "STAnnounceList.h"
#include <badesca.h>
#include <e32math.h>

CSTAnnounceList::~CSTAnnounceList()
{
	Reset();
}

void CSTAnnounceList::Reset()
{
	iTiers.ResetAndDestroy();
}

EXPORT_C TInt CSTAnnounceList::AddressCount() const
{
	TInt count = 0;
	
	for (TInt i=0; i<iTiers.Count(); i++)
		count += iTiers[i]->MdcaCount();
		
	return count;
}

void CSTAnnounceList::AddAddressL(TInt aTierIndex, const TDesC8& aAddress)
{
	while (aTierIndex >= iTiers.Count())
	{
		CDesC8ArraySeg* tier = new (ELeave) CDesC8ArraySeg(3);
		CleanupStack::PushL(tier);
		User::LeaveIfError(iTiers.Append(tier));
		CleanupStack::Pop(); // tier
	}
	
	iTiers[aTierIndex]->AppendL(aAddress);
}

void CSTAnnounceList::ShuffleL()
{
	TTime now;
	now.HomeTime();
	TInt64 seed = now.Int64();

	for (TInt i=0; i<iTiers.Count(); i++)
	{
		TInt addressCount = iTiers[i]->MdcaCount();
			
		if (addressCount > 1)
			for (TInt j=0; j<addressCount; j++)
			{
				TInt pos = TInt(Math::Rand(seed)) % (addressCount-1);

				HBufC8* address = (*iTiers[i])[j].AllocLC();
				iTiers[i]->Delete(j);
				iTiers[i]->InsertL(pos, *address);
				CleanupStack::PopAndDestroy(); // address
			}
	}
}

void CSTAnnounceList::PromoteActiveAddressL()
{
	HBufC8* address = (*iTiers[iActiveTierIndex])[iActiveTrackerIndex].AllocLC();
	iTiers[iActiveTierIndex]->Delete(iActiveTrackerIndex);
	iTiers[iActiveTierIndex]->InsertL(0, *address);
	CleanupStack::PopAndDestroy(); // address
	
	iActiveTrackerIndex = 0;
}

void CSTAnnounceList::MoveToNextAddress()
{
	if (iActiveTrackerIndex < (iTiers[iActiveTierIndex]->MdcaCount()-1))
		iActiveTrackerIndex++;
	else
	{
		iActiveTrackerIndex = 0;
		
		if (iActiveTierIndex < (iTiers.Count() - 1))
			iActiveTierIndex++;
		else
			iActiveTierIndex = 0;
	}
}

EXPORT_C CDesC8Array* CSTAnnounceList::GetAllAddressesL() const
{
	CDesC8ArraySeg* addresses = new (ELeave) CDesC8ArraySeg(5);
	CleanupStack::PushL(addresses);
		
	for (TInt i=0; i<iTiers.Count(); i++)
	{
		TInt addressCount = iTiers[i]->MdcaCount();
		for (TInt j=0; j<addressCount; j++)
		{
			addresses->AppendL((*iTiers[i])[j]);
		}
	}
	
	addresses->Sort();
	
	CleanupStack::Pop(); // addresses
	
	return addresses;
}

void CSTAnnounceList::RemoveEmptyTiers()
{
	for (TInt i=0; i<iTiers.Count(); i++)
	{
		if (iTiers[i]->MdcaCount() == 0)
		{
			iTiers.Remove(i);
			i--;
		}
	}
}
