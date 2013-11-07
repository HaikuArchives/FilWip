/*
 * FilWip
 * Copyright (c) 2001 Ramshankar
 *
 * This license does not apply to source code written by third parties included with
 * and/or used by this software. This includes third party modules or any other form
 * of source code provided by third parties or by an unknown source.
 *
 * You are allowed to  use parts of  this code in  your own program(s) provided  the
 * program  is  freeware  and  you  give  credit to  the  author in  your  program's
 * interactive mode (ie viewable by the end-user). You are not allowed to distribute
 * the code in any other language other than the language it was  originally written
 * in. Any alterations made  to the original  code must be  mentioned explicitly and
 * the author must be explicitly disclaimed of any responsibility.
 *
 * If you wish to  use the source code  or parts of  it thereof  in a  commercial or
 * shareware  application, the author  receives a free  version of  your program and
 * contact the author to negotiate  a fee for your  use of the author's source code.
 * Distributing  modified  versions  of  the source  code  is  allowed provided  you
 * explicitly disclaim the author and claim that the distribution is not an original
 * distribution from the author.
 * 
 * Original code by:
 * Ramshankar
 *
 */

#include "TreeView.h"

/*============================================================================================================*/

TreeView::TreeView (float left, float top, const char *name, rgb_color backColor, BMessage *expanded,
			BMessage *collapsed, BMessage *midway, int8 index)
	: BView (BRect (left, top, left + 13, top + 13), name, B_FOLLOW_LEFT, B_WILL_DRAW | B_SUBPIXEL_PRECISE),
		collapsedMsg (collapsed),
		expandedMsg (expanded),
		midwayMsg (midway)
{
	indexID = index;
	SetViewColor (backColor);


	/* This index helps our caller to make their superitem implementation completely generic
		and not hardcoded */
	collapsedMsg->AddInt8 ("item_index", indexID);
	expandedMsg->AddInt8 ("item_index", indexID);
	midwayMsg->AddInt8 ("item_index", indexID);
	
	
	/* Load all the bitmaps from the resource file - No error checking for speed */
	BResources *appRes = be_app->AppResources();
	size_t bmpSize;
	BMessage msg;
	char *buf;

	buf = (char*)appRes->LoadResource ('BBMP', "Image:Collapsed", &bmpSize);
	msg.Unflatten (buf);
	collapsedBitmap = new BBitmap (&msg);

	buf = (char*)appRes->LoadResource ('BBMP', "Image:MidWay", &bmpSize);
	msg.Unflatten (buf);
	midwayBitmap = new BBitmap (&msg);

	buf = (char*)appRes->LoadResource ('BBMP', "Image:Expanded", &bmpSize);
	msg.Unflatten (buf);
	expandedBitmap = new BBitmap (&msg);


	/* Initially set the state as collapsed */
	isExpanded = false;
	isMidWay = false;
}

/*============================================================================================================*/

TreeView::~TreeView ()
{
	delete collapsedBitmap;
	delete midwayBitmap;
	delete expandedBitmap;
	delete collapsedMsg;
	delete midwayMsg;
	delete expandedMsg;
}

/*============================================================================================================*/

void TreeView::Draw (BRect updateRect)
{
	/* All-in-all drawing function (in transparent-alpha mode) */
	SetHighColor (ViewColor());
	FillRect (Bounds());
	
	SetDrawingMode (B_OP_ALPHA);
	if (isMidWay == true)
		DrawBitmapAsync (midwayBitmap);
	else	
		DrawBitmapAsync (isExpanded == false ? collapsedBitmap : expandedBitmap);	
	
	BView::Draw (updateRect);
}

/*============================================================================================================*/

void TreeView::MouseDown (BPoint point)
{
	isMidWay = true;
	Invalidate (Bounds());
	
	NotifyStatus();
	BView::MouseDown (point);
}

/*============================================================================================================*/

void TreeView::MouseUp (BPoint point)
{
	isExpanded = !isExpanded;
	isMidWay = false;
	Invalidate (Bounds());
	NotifyStatus();
	
	BView::MouseUp (point);
}

/*============================================================================================================*/

void TreeView::MouseMoved (BPoint point, uint32 status, const BMessage *dragMessage)
{
	/* Handle the mouse when its moving */
	if (Window()->IsActive() == false)
		return;

	BPoint pt;
	uint32 buttons;
	GetMouse (&pt, &buttons, false);
	
	/* Exit function if no mouse button is held down */
	if (!(buttons != 0))
		return;
	
	if (status == B_ENTERED_VIEW)
		isMidWay = true;
	else if (status == B_EXITED_VIEW)
		isMidWay = false;
	
	if (status == B_EXITED_VIEW || status == B_ENTERED_VIEW)
	{
		Invalidate (Bounds());
		NotifyStatus();
	}
	
	BView::MouseMoved (point, status, dragMessage);
}

/*============================================================================================================*/

void TreeView::NotifyStatus ()
{
	/* Tell our parent window about the new status... */
	if (isMidWay == true)
		Window()->PostMessage (midwayMsg);
	else if (isExpanded == true)
		Window()->PostMessage (expandedMsg);
	else if (isExpanded == false)
		Window()->PostMessage (collapsedMsg);
}

/*============================================================================================================*/

bool TreeView::IsExpanded ()
{
	return isExpanded;
}

/*============================================================================================================*/

void TreeView::SetStatus (bool expanded)
{
	isExpanded = expanded;
	Invalidate (Bounds());
}

/*============================================================================================================*/
