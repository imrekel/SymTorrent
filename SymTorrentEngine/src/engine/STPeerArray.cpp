#include "STPeerArray.h"
#include "STPeer.h"
#include "STBencode.h"

RSTPeerArray::RSTPeerArray()
 : iPrimaryPeerCount(0), iLocalPeerCount(0)
{
}

RSTPeerArray::~RSTPeerArray()
{
	iPeers.ResetAndDestroy();
}

void RSTPeerArray::Close()
{
	iPeers.ResetAndDestroy();
}

void RSTPeerArray::Remove(TInt aIndex) 
{
	if (iPeers[aIndex]->IsLocal())
		iLocalPeerCount--;
	else
		iPrimaryPeerCount--;
	
	iPeers.Remove(aIndex);
}

TInt RSTPeerArray::Append(CSTPeer* aPeer)
{
	TInt res = iPeers.Append(aPeer);
	
	if (res == KErrNone)
	{
		if (aPeer->IsLocal())
			iLocalPeerCount++;
		else
			iPrimaryPeerCount++;
	}
	
	return res;
}

void RSTPeerArray::AppendL(CSTPeer* aPeer)
{
	User::LeaveIfError(Append(aPeer));
}

TInt RSTPeerArray::Insert(CSTPeer* aPeer, TInt aPos)
{
	TInt res = iPeers.Insert(aPeer, aPos);
		
	if (res == KErrNone)
	{
		if (aPeer->IsLocal())
			iLocalPeerCount++;
		else
			iPrimaryPeerCount++;
	}
	
	return res;
}

void RSTPeerArray::InsertL(CSTPeer* aPeer, TInt aPos)
{
	User::LeaveIfError(Insert(aPeer, aPos));
}

CSTBencodedList* RSTPeerArray::CreateBencodeL(TInt aMaxPeerCount)
{
	if (iPeers.Count() == 0)
		return NULL;
	
	CSTBencodedList* peerList = new (ELeave) CSTBencodedList;
	CleanupStack::PushL(peerList);
	
	// first try to add peers that has already been connected
	for (TInt i=0; i<iPeers.Count(); i++)
	{
		if (aMaxPeerCount && peerList->Count() >= aMaxPeerCount)
			break;
		
		if (iPeers[i]->SuccessfullyConnected())
			peerList->AppendL(CreatePeerInfoBencodeL(*iPeers[i]));	
	}
	
	// add peers that has not been connected yet
	for (TInt i=0; i<iPeers.Count(); i++)
	{
		if (aMaxPeerCount && peerList->Count() >= aMaxPeerCount)
			break;
		
		if (!iPeers[i]->SuccessfullyConnected())
			peerList->AppendL(CreatePeerInfoBencodeL(*iPeers[i]));	
	}
	
	CleanupStack::Pop(peerList);
	return peerList;
}

CSTBencodedList* RSTPeerArray::CreatePeerInfoBencodeL(CSTPeer& aPeer)
{
	CSTBencodedList* peerInfo = new (ELeave) CSTBencodedList;
	CleanupStack::PushL(peerInfo);
		
	TInetAddr peerAddr(aPeer.Address());
	
	CSTBencodedInteger* addrBencode = new (ELeave) CSTBencodedInteger(peerAddr.Address());
	CleanupStack::PushL(addrBencode);
	peerInfo->AppendL(addrBencode);
	CleanupStack::Pop(addrBencode);
	
	CSTBencodedInteger* portBencode = new (ELeave) CSTBencodedInteger(peerAddr.Port());
	CleanupStack::PushL(portBencode);
	peerInfo->AppendL(portBencode);
	CleanupStack::Pop(portBencode);
	
	CleanupStack::Pop(peerInfo);
			
	return peerInfo;
			
}
