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

#include <Message.h>
#include <Window.h>
#include <Bitmap.h>

#include "Constants.h"
#include "ImageButton.h"
#include "FilWip.h"

/*============================================================================================================*/

ImageButton::ImageButton (const char *name, BBitmap *bitmap, BMessage *message,
						const rgb_color bgColor)
	: BView (BRect (0, 0, 23, 23), name, B_FOLLOW_RIGHT, B_WILL_DRAW | B_SUBPIXEL_PRECISE),
	clickBitmap (bitmap),
	clickMessage (message),
	backgroundColor (bgColor)
{
	SetViewColor (backgroundColor);
}

/*============================================================================================================*/

ImageButton::~ImageButton ()
{
	/* OK, were done with these pointers, delete them */
	delete clickMessage;
	delete clickBitmap;
}

/*============================================================================================================*/

void ImageButton::Draw (BRect updateRect)
{
	/* Erase entire bounds */
	MovePenTo (0, 0);
	SetHighColor (backgroundColor);
	StrokeRect (updateRect);	

	/* Draw the picture */
	SetDrawingMode (B_OP_ALPHA);
	MovePenTo (4, 4);
	DrawBitmapAsync (clickBitmap);
	SetDrawingMode (B_OP_COPY);
	
	BView::Draw (updateRect);
}

/*============================================================================================================*/

void ImageButton::MouseMoved (BPoint point, uint32 status, const BMessage *dragInfo)
{
	/* Handle the mouse, very carefully done to suit the BubbleHelper
		-- DON'T ALTER -- */
	if (Window()->IsActive() == false)
	{
		BView::MouseMoved (point, status, dragInfo);
		return;
	}

	BPoint pt;
	uint32 buttons;
	GetMouse (&pt, &buttons, false);
	
	if (status == B_ENTERED_VIEW)
	{
		firstClick = false;
		lastClick = false;
		
		if (buttons != 0)
			PushButton ();
	}
	
	if (status == B_INSIDE_VIEW)
	{
		if (buttons == 0)
		{
			MovePenTo (0, 0);
			DrawOutsideEdge ();
			DrawShinyEdge (false);
		}
	}
	
	if (status == B_EXITED_VIEW)
		Invalidate (Bounds());
	
	BView::MouseMoved (point, status, dragInfo);
}

/*============================================================================================================*/

void ImageButton::DrawOutsideEdge ()
{
	SetHighColor (BeLightenedShadow);
	StrokeRect (Bounds());
}

/*============================================================================================================*/

void ImageButton::DrawShinyEdge (bool isPressing)
{
	/* Do interface details (annoying) */
	SetHighColor (BeLightedEdge);
	BRect bounds (Bounds());
	if (isPressing == false)
	{
		StrokeLine (BPoint (0, 0), BPoint (bounds.right, 0));
		StrokeLine (BPoint (0, 0), BPoint (0, bounds.bottom));
	}
	else
	{
		StrokeLine (BPoint (bounds.right, bounds.top), BPoint (bounds.right, bounds.bottom));
		StrokeLine (BPoint (0, bounds.bottom), BPoint (bounds.right, bounds.bottom));
	}
}

/*============================================================================================================*/

void ImageButton::MouseDown (BPoint point)
{
	/* Render a pushed button */
	PushButton ();

	BView::MouseDown (point);
}

/*============================================================================================================*/

void ImageButton::MouseUp (BPoint point)
{
	/* Basically check whether to PostMessage() or not */
	Draw (Bounds());
	DrawOutsideEdge ();
	DrawShinyEdge (false);
	
	if (Bounds().Contains (point) == true)
	{
		/* Check if its the first click on the window */
		if (firstClick == false)
		{
			Invalidate (Bounds());
			Window()->PostMessage (clickMessage);
		}
		else
		{
			MouseMoved (point, B_ENTERED_VIEW, NULL);
		}

		if (lastClick == false)
			MouseMoved (point, B_ENTERED_VIEW, NULL);
	}

	BView::MouseUp (point);
}

/*============================================================================================================*/

void ImageButton::MessageReceived (BMessage *message)
{
	switch (message->what)
	{
		case M_JUST_GOT_FOCUS:
		{
			bool state;
			message->FindBool ("Has Focus", &state);
			if (state == true)
				firstClick = true;
			else
				lastClick = true;
			
			/* Window got out of focus, thus force a re-draw */
			if (lastClick == true)
				Draw (Bounds());
			
			break;
		}
	}

	BView::MessageReceived (message);
}

/*============================================================================================================*/

void ImageButton::PushButton ()
{
	/* Give a pushed button effect */
	if (firstClick == true)
		return;
	
	DrawOutsideEdge ();
	DrawShinyEdge (true);

	/* Erase view (leave border alone) then re-draw */
	SetHighColor(backgroundColor);
	FillRect(Bounds().InsetBySelf (1, 1));

	/* Draw with transparency */
	SetDrawingMode (B_OP_ALPHA);
	MovePenTo (5, 5);
	DrawBitmapAsync (clickBitmap);
	SetDrawingMode (B_OP_COPY);
}

/*============================================================================================================*/
