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

#include <ScrollBar.h>
#include <Window.h>
#include <Message.h>

#include "Constants.h"
#include "ItemsView.h"

/*============================================================================================================*/

ItemsView::ItemsView (BRect frame, const char *name, uint32 resizeMask, uint32 flags)
	: BView (frame, name, resizeMask, flags)
{
}

/*============================================================================================================*/

void ItemsView::KeyDown (const char *bytes, int32 numBytes)
{
	/* Handle movement within the view using the keyboard */
	float smallStep, largeStep;
	BScrollBar* vertScrollBar = ScrollBar (B_VERTICAL);
	if (vertScrollBar == NULL || numBytes != 1)
		return;
	
	
	/* The reason why we get the scrollbar to scroll, and not scroll the view directly, is that going through
		the scrollbar makes sure a scroll is not 'attempted' when a scroll cannot be made. We need not add
		checks for this. */
	vertScrollBar->GetSteps (&smallStep, &largeStep);
	switch (bytes[0])
	{
		case B_FUNCTION_KEY:
		{
			BMessage *msg = Window()->CurrentMessage();
			if (msg != NULL)
			{
				int32 key;
				msg->FindInt32 ("key", &key);
				if (key == B_F1_KEY)
					Window()->PostMessage (M_HELP);
			}
			break;
		}
		
		case B_DOWN_ARROW: case B_RIGHT_ARROW:
			vertScrollBar->SetValue (vertScrollBar->Value() + smallStep);
			break;
			
		case B_UP_ARROW: case B_LEFT_ARROW:
			vertScrollBar->SetValue (vertScrollBar->Value() - smallStep);
			break;
			
		case B_PAGE_DOWN:
			vertScrollBar->SetValue (vertScrollBar->Value() + 3 * largeStep);
			break;
		
		case B_PAGE_UP:
			vertScrollBar->SetValue (vertScrollBar->Value() - 3 * largeStep);
			break;
			
		case B_HOME:
			vertScrollBar->SetValue (0);
			break;
		
		case B_END:
			float min, max;
			vertScrollBar->GetRange (&min, &max);
			vertScrollBar->SetValue (max);
			break;
		
		default:
			BView::KeyDown (bytes, numBytes);
	}
}

/*============================================================================================================*/
