/*
 * TouchScrollableRichTextEditor.cpp
 *
 *  Created on: 2010.08.19.
 *      Author: kenley
 */

#include "TouchScrollableRichTextEditor.h"

const TInt KScrollTolerance = 10;

CTouchScrollableRichTextEditor::CTouchScrollableRichTextEditor(const TGulBorder& aBorder)
 : CEikRichTextEditor(aBorder)
{
}

void CTouchScrollableRichTextEditor::HandlePointerEventL(const TPointerEvent& aPointerEvent)
{
	ClearSelectionL(); 
	
	if (aPointerEvent.iType == TPointerEvent::EButton1Down)
	{
		iMovePoint = aPointerEvent.iPosition;
	}
	else if (aPointerEvent.iType == TPointerEvent::EDrag)
	{
		TInt dist = iMovePoint.iY - aPointerEvent.iPosition.iY;
		
		if (dist > KScrollTolerance)
		{
			iMovePoint = aPointerEvent.iPosition;
			while (dist > 0)
			{
				MoveDisplayL(TCursorPosition::EFLineDown);
				dist -= KScrollTolerance;
			}
		}
		else if (dist < -KScrollTolerance)
		{
			iMovePoint = aPointerEvent.iPosition;
			while (dist < 0)
			{
				MoveDisplayL(TCursorPosition::EFLineUp);
				dist += KScrollTolerance;
			}
		}
		
	   DrawNow();
	}
}
