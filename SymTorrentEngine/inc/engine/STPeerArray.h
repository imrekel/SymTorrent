#ifndef CSTPEERARRAY_H_
#define CSTPEERARRAY_H_

#include <e32base.h>
#include "STPeer.h"

class CSTBencodedList;

/**
 * RSTPeerArray
 * 
 * Holds the peers of a torrent 
 */
class RSTPeerArray
{
public:
	
	RSTPeerArray();
	
	virtual ~RSTPeerArray();
	
	inline void ResetAndDestroy();
	
	inline TInt Count() const;
	
	inline TInt LocalPeerCount() const;
	
	inline TInt PrimaryPeerCount() const;
	
	inline CSTPeer* operator[](TInt aIndex);
	
	inline TInt Find(CSTPeer* aPeer);
	
	void Close();
	
	void Remove(TInt aIndex);
	
	TInt Append(CSTPeer* aPeer);
	
	void AppendL(CSTPeer* aPeer);
	
	TInt Insert(CSTPeer* aPeer, TInt aPos);
	
	void InsertL(CSTPeer* aPeer, TInt aPos);
	
	/**
	 * Creates a bencoded list from the peers.
	 * The caller takes ownership of the returned object.
	 * 
	 * returns NULL if there are no peers
	 * 
	 * @param aMaxPeerCount is the maximum number of peers that are returned, 0 means no limit
	 */
	CSTBencodedList* CreateBencodeL(TInt aMaxPeerCount);
	
private:
	
	CSTBencodedList* RSTPeerArray::CreatePeerInfoBencodeL(CSTPeer& aPeer);
	
private:
	
	RPointerArray<CSTPeer> iPeers;
	
	TInt iPrimaryPeerCount;
	TInt iLocalPeerCount;
};

inline void RSTPeerArray::ResetAndDestroy() {
	iPeers.ResetAndDestroy();
}

inline TInt RSTPeerArray::Count() const {
	return iPeers.Count();
}

inline TInt RSTPeerArray::LocalPeerCount() const {
	return iLocalPeerCount;
}

inline TInt RSTPeerArray::PrimaryPeerCount() const {
	return iPrimaryPeerCount;
}

inline CSTPeer* RSTPeerArray::operator[](TInt aIndex) {
	return iPeers[aIndex];
}

inline TInt RSTPeerArray::Find(CSTPeer* aPeer) {
	return iPeers.Find(aPeer);
}

#endif /*CSTPEERARRAY_H_*/
