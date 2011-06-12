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

/* ============================================================================
 *  Name     : CSTBencode from STBencode.cpp
 *  Part of  : SymTorrent Engine
 *  Created  : 31.01.2006 by Imre Kelényi
 * ============================================================================
 */

// INCLUDE FILES
#include "STBencode.h"
#include "STSimpleStack.h"

enum TBencodeParserState
{
	EParsingInProgress = 0,	
	EParsingFailed,
	EParsingSucceeded	
};

// ================= MEMBER FUNCTIONS =======================

EXPORT_C void CSTBencode::ToLogL(TInt /*aIndentation*/)
{
//	for (TInt i=0; i<aIndentation; i++)
//		LOG->WriteL(_L("  "));
}


EXPORT_C CSTBencode* CSTBencode::ParseL(const TDesC8& aBuffer)
{
	TBencodeParserState parserState = EParsingInProgress;	
	TLex8 lex(aBuffer);	
	TChar c;
	TBool itemParsed = EFalse; 	// state flag, used to indicate that a bencoded 
								// item has been parsed
								
	//TBool keyOnStack = EFalse; // used to indicate that there is a dictionary entry key on the stack
								
	CSTSimpleStack<CSTBencode>* parserStack = 
		new (ELeave) CSTSimpleStack<CSTBencode>;
	CleanupStack::PushL(parserStack);
	
	while (	(c = lex.Peek()) && 
			(parserState == EParsingInProgress))
	{		
		// bencoded string		
		if (c.IsDigit())
		{
			TInt stringLength = 0;
			if ((lex.Val(stringLength) == KErrNone) && (stringLength >= 0))
			{
				while (c && (c != ':'))				
					c = lex.Get();
					
				if (!c || ((lex.Offset() + stringLength) > aBuffer.Length()))
				{
					parserState = EParsingFailed;
					continue;
				}
								
				lex.Mark();
				lex.Inc(stringLength);
				
				CSTBencodedString* string = CSTBencodedString::NewLC(lex.MarkedToken());
				parserStack->PushL(string);
				CleanupStack::Pop();							
				
				c = lex.Peek();
				
				itemParsed = ETrue;
			}
			else
			{							
				parserState = EParsingFailed;			
				continue;
			}
		}
		else
		
		// bencoded integer
		if (c == 'i')
		{
			lex.Inc();
			
			TInt64 value = 0;
			if (lex.Val(value) == KErrNone)
			{
				c = lex.Peek();
				
				while (c && (c != 'e'))
				{
					lex.Inc();
					c = lex.Peek();					
				}
				
				if (c == 'e')
				{
					
					CSTBencodedInteger* bencodedInteger = 
						new (ELeave) CSTBencodedInteger(value);
					
					CleanupStack::PushL(bencodedInteger);
					parserStack->PushL(bencodedInteger);
					CleanupStack::Pop(); // bencodedInteger
				}					
				
			}
			else
			{							
				parserState = EParsingFailed;			
				continue;
			}
			
		}
		else
		
		// bencoded list
		if (c == 'l')
		{
			lex.Inc();
			
			CSTBencodedList* bencodedList = 
				new (ELeave) CSTBencodedList;
			CleanupStack::PushL(bencodedList);
			parserStack->PushL(bencodedList);
			CleanupStack::Pop(); // bencodedList
		}
		else
		
		// bencoded dictionary
		if (c == 'd')
		{
			lex.Inc();
			
			CSTBencodedDictionary* bencodedDictionary = 
				new (ELeave) CSTBencodedDictionary;
			CleanupStack::PushL(bencodedDictionary);
			parserStack->PushL(bencodedDictionary);
			CleanupStack::Pop(); // bencodedDictionary			
		}
		else
		
		// end of bencoded integer/list/dictionary
		if (c == 'e')
		{
			lex.Inc();
			itemParsed = ETrue;			
		}
		else
			parserState = EParsingFailed;
		
		if (itemParsed)
		{
			itemParsed = EFalse;
			
			if (parserStack->Count() == 1)
				parserState = EParsingSucceeded;
			else
			{
				switch (parserStack->FromTop(1)->Type())
				{
					case EBencodedList:
					{
						CSTBencode* item = parserStack->Pop();
						CleanupStack::PushL(item);
						static_cast<CSTBencodedList*>(parserStack->Top())->AppendL(item);
						CleanupStack::Pop(); // item						
					}
					break;
					
					case EBencodedString:
					{
						if (parserStack->FromTop(2)->Type() == EBencodedDictionary)
						{
							CSTBencode* value = parserStack->Pop();
							CSTBencodedString* key = 
								static_cast<CSTBencodedString*>(parserStack->Pop());
						
							CleanupStack::PushL(value);
							CleanupStack::PushL(key);
						
							static_cast<CSTBencodedDictionary*>(parserStack->Top())->AddEntryL(key, value);
						
							CleanupStack::Pop(); // key
							CleanupStack::Pop(); // value							
						}
						else
							parserState = EParsingFailed;												
					}
					break;
					
					case EBencodedDictionary:
						break;
						
					default:
					{
						parserState = EParsingFailed;												
					}
					break;									
				}
			}
		} // if (itemparsed)
	} // while
	
	CSTBencode* parsedItem = NULL;
	if (parserState == EParsingSucceeded)
		parsedItem = parserStack->Pop();		
	
	CleanupStack::PopAndDestroy(); // parserStack
	
	return parsedItem;
}
