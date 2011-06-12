/*
 * TouchScrollableRichTextEditor.h
 *
 *  Created on: 2010.08.19.
 *      Author: kenley
 */

#ifndef TOUCHSCROLLABLERICHTEXTEDITOR_H_
#define TOUCHSCROLLABLERICHTEXTEDITOR_H_

#include <eikrted.h>

class CTouchScrollableRichTextEditor : public CEikRichTextEditor
{
public:
	
	CTouchScrollableRichTextEditor(const TGulBorder& aBorder);
	
private:
	
	void HandlePointerEventL(const TPointerEvent& aPointerEvent);
	
private:
	
	TPoint 					iMovePoint;	
};


#endif /* TOUCHSCROLLABLERICHTEXTEDITOR_H_ */
