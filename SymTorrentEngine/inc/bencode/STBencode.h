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

/*
 * ============================================================================
 *  Classes  : CSTBencode
 *             CSTBencodedInteger
 *             CSTBencodedString
 *             CSTBencodedList
 *             CSTBencodeDictionary
 *             CSTBencodeDictionaryEntry
 *			  
 *  Part of  : SymTorrent
 *  Created  : 31.01.2006 by Imre Kelényi
 * ============================================================================
 */

#ifndef SYMTORRENT_STBENCODE_H
#define SYMTORRENT_STBENCODE_H

// INCLUDES
#include <e32base.h>
//#include "STLogger.h"
//#include "STDefs.h"

enum TBencodeType
{
	EBencodedString = 0,
	EBencodedInteger,
	EBencodedList,
	EBencodedDictionary
};

/**
 * CSTBencode
 *
 * Abstract base class for bencoded data classes.
 * It has a static method for parsing bencoded byte streams.
 */
class CSTBencode : public CBase
{
public:

	virtual TBencodeType Type() = 0;
	
	/**
	 * Parses a bencoded byte stream. It only decodes the first
	 * bencoded element of the stream (of course, if the element
	 * is a list or dictionary, it parses the contained elements too)!
	 *
	 * @return on succces a newly created bencoded data class,
	 * otherwise NULL
	 */
	IMPORT_C static CSTBencode* ParseL(const TDesC8& aBuffer);
	
	/**
	 * Writes the content of the object to the log
	 */
	IMPORT_C virtual void ToLogL(TInt aIndentation = 0);
	
	/**
	 * @return a newly created buffer containing the bencoded data
	 */
	virtual HBufC8* BencodeL() const = 0;
};


/**
 * CSTBencodedInteger
 */
class CSTBencodedInteger : public CSTBencode
{
public:

	IMPORT_C CSTBencodedInteger(TInt64 aValue);

public: // from CSTBencode

	IMPORT_C TBencodeType Type();
	
	IMPORT_C void ToLogL(TInt aIndentation = 0);
	
	IMPORT_C HBufC8* BencodeL() const;
	
public:

	TInt64 iValue;
};


/**
 * CSTBencodedString
 */
class CSTBencodedString : public CSTBencode
{
public:

	IMPORT_C static CSTBencodedString* NewL(const TDesC8& aDes);
	
	IMPORT_C static CSTBencodedString* NewLC(const TDesC8& aDes);		

	IMPORT_C ~CSTBencodedString();
		
	IMPORT_C void SetValueL(const TDesC8& aDes);
	
	inline const TDesC8& Value() const;
	
	IMPORT_C void ToLogL(TInt aIndentation = 0);
	
public: // from CSTBencode

	IMPORT_C TBencodeType Type();
	
	IMPORT_C HBufC8* BencodeL() const;	
	
private: // Constructors

	CSTBencodedString() {};

	void ConstructL(const TDesC8& aDes);
	
private:

	HBufC8* iBuffer;
};


/**
 * CSTBencodedList
 */
class CSTBencodedList : public CSTBencode
{
public:

	IMPORT_C ~CSTBencodedList();
		
	IMPORT_C void AppendL(CSTBencode* aItem);
	
	inline TInt Count() const;
	
	inline CSTBencode* Item(TInt aIndex);
	
	IMPORT_C void ToLogL(TInt aIndentation = 0);
	
public: // from CSTBencode
	
	IMPORT_C TBencodeType Type();
	
	IMPORT_C HBufC8* BencodeL() const;
	
private:

	RPointerArray<CSTBencode> iItems;
};


/**
 * CSTBencodedDictionaryEntry
 */
class CSTBencodedDictionaryEntry : public CBase
{
public:

	/**
	 * Constructor. Takes ownership of the supplied key and value.
	 */
	IMPORT_C CSTBencodedDictionaryEntry(	CSTBencodedString* aKey,
											CSTBencode*	aValue);
								
	IMPORT_C ~CSTBencodedDictionaryEntry();
								
	inline CSTBencode* Value();
	
	inline CSTBencodedString* Key();
	
	/**
	 * Used for sorting.
	 */
	IMPORT_C static TInt Compare(
		const CSTBencodedDictionaryEntry& aFirst, 
		const CSTBencodedDictionaryEntry& aSecond);		

private:

	CSTBencodedString* 	iKey;
	
	CSTBencode*			iValue;
};


/**
 * CSTBencodedDictionary
 */
class CSTBencodedDictionary : public CSTBencode
{
public:

	IMPORT_C ~CSTBencodedDictionary();
		
	IMPORT_C void AddEntryL(CSTBencodedDictionaryEntry* aEntry);
	
	/**
	 * Important! Do NOT push aKey and aValue to the cleanup stack!
	 */
	IMPORT_C void AddEntryL(CSTBencodedString* aKey, CSTBencode* aValue);
	
	IMPORT_C void AddEntryL(const TDesC8& aKey, CSTBencode* aValue);
	
	inline TInt Count() const;
	
	inline CSTBencodedDictionaryEntry* Entry(TInt aIndex);
	
	IMPORT_C CSTBencode* EntryValue(const TDesC8& aKey);
	
	IMPORT_C CSTBencode* EntryValue(CSTBencodedString* aKey);
	
	IMPORT_C void ToLogL(TInt aIndentation = 0);
	
public: // from CSTBencode
	
	IMPORT_C TBencodeType Type();
	
	IMPORT_C HBufC8* BencodeL() const;
	
private:

	RPointerArray<CSTBencodedDictionaryEntry> iEntries;
};


// INLINE FUNCTION IMPLEMENTATIONS

inline TInt CSTBencodedList::Count() const {
	return iItems.Count();	
}

inline CSTBencode* CSTBencodedList::Item(TInt aIndex) {
	return iItems[aIndex];	
}

inline const TDesC8& CSTBencodedString::Value() const {
	return *iBuffer;
}

inline CSTBencode* CSTBencodedDictionaryEntry::Value() {
	return iValue;
}
	
inline CSTBencodedString* CSTBencodedDictionaryEntry::Key(){
	return iKey;
}

inline TInt CSTBencodedDictionary::Count() const {
	return iEntries.Count();
}
	
inline CSTBencodedDictionaryEntry* CSTBencodedDictionary::Entry(TInt aIndex) {
	return iEntries[aIndex];
}


#endif
