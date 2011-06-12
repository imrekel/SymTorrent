/*
 * MutablePointerArrayIterator.h
 *
 *  Created on: 2010.12.13.
 *      Author: kenley
 */

#ifndef MUTABLEPOINTERARRAYITERATOR_H_
#define MUTABLEPOINTERARRAYITERATOR_H_

#include <e32base.h>

template <class T>
class RMutablePointerArrayIterator
{
public:
	
	RMutablePointerArrayIterator();
	
	void OpenL(RPointerArray<T>& aArray);
	
	void Close();
	
	/**
	 * Returns the next unprocessed item. NULL if no more unprocessed items left.
	 */
	T* NextL();
	
	TBool HasNext();
	
	/**
	 * Removes the given item from the array.
	 * 
	 * @return KErrNone if the item was found and removed
	 *         KErrNotFound if the item was not found
	 * 
	 * It should be pointed out that the items can also be removed from the array directly.
	 */
	TInt Remove(T* aItem);
	
private:
	
	T* GetUnprocessedItem();
	
private:
	
	RPointerArray<T>* iArray;
	
	/**
	 * Pointers to the items of the array that has already been processed
	 */
	RPointerArray<T> iProcessedItems;
	
	/**
	 * Possible next item
	 */
	T* iNextCandidate;
	
};

// METHOD DEFINITIONS

template <class T>
RMutablePointerArrayIterator<T>::RMutablePointerArrayIterator()
 : iNextCandidate(NULL)
{
}

template <class T>
void RMutablePointerArrayIterator<T>::Close()
{
	iProcessedItems.Close();
}

template <class T>
void RMutablePointerArrayIterator<T>::OpenL(RPointerArray<T>& aArray)
{
	iArray = &aArray;
}

template <class T>
T* RMutablePointerArrayIterator<T>::NextL()
{
	T* next = iNextCandidate;

	if (iNextCandidate) // check if item still in array
	{
		TBool found = EFalse;
		for (TInt i=0; i<iArray->Count(); i++)
		{
			if ((*iArray)[i] == iNextCandidate)
			{
				found = ETrue;
				break;
			}
		}
		
		if (!found)
			next = NULL;
		
		iNextCandidate = NULL;
	}
	
	if (!next)
		next = GetUnprocessedItem();
	
	if (next)
	{
		iProcessedItems.Append(next);
	}
	
	return next;
}

template <class T>
T* RMutablePointerArrayIterator<T>::GetUnprocessedItem()
{
	for (TInt i=0; i<iArray->Count(); i++)
	{
		TBool alreadyProcessed = EFalse;
		
		for (TInt j=0; j<iProcessedItems.Count(); j++)
		{
			if ((*iArray)[i] == iProcessedItems[j])
			{
				alreadyProcessed = ETrue;
				break;
			}
		}
		
		if (!alreadyProcessed)
		{
			return (*iArray)[i];
		}
	}
	
	return NULL;
}

template <class T>
TBool RMutablePointerArrayIterator<T>::HasNext()
{
	iNextCandidate = GetUnprocessedItem();
	
	return (iNextCandidate != NULL);
}

template <class T>
TInt RMutablePointerArrayIterator<T>::Remove(T* aItem)
{
	if (iNextCandidate == aItem)
		iNextCandidate = NULL;
	
	TInt index = iArray->Find(aItem);
	if (index >= 0)
	{
		iArray->Remove(index);
		return KErrNone;
	}
	
	return KErrNotFound;
}



#endif /* MUTABLEPOINTERARRAYITERATOR_H_ */
