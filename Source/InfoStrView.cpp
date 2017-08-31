/*
 * FilWip
 * Copyright (c) 2002 Ramshankar
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
 * Modified by:
 * :Puck Meerburg
 */

#include <Window.h>
#include <Message.h>
#include <Roster.h>
#include <Messenger.h>
#include <Path.h>

#include <stdlib.h>
#include <string>
#include <string.h>

#include "InfoStrView.h"

/*============================================================================================================*/

InfoStrView::InfoStrView (BRect bounds, const char *name, const char *text,
				uint32 resizeFlags, uint32 flags)
	: BStringView (bounds, name, text, resizeFlags, flags),
		itemPath (NULL),
		trackerSignature ("application/x-vnd.Be-TRAK")
{
	SetAlignment (B_ALIGN_RIGHT);
}

/*============================================================================================================*/

InfoStrView::~InfoStrView()
{
	DeAllocPath();
}

/*============================================================================================================*/

void InfoStrView::DeAllocPath ()
{
	if (itemPath != NULL)
	{
		delete[] itemPath;
		itemPath = NULL;
	}
}

/*============================================================================================================*/

void InfoStrView::AllocPath (const char *str)
{
	itemPath = new char[strlen(str) + 1];
	strcpy (itemPath, str);
}

/*============================================================================================================*/

void InfoStrView::SetPath (const char *path)
{
	/* Store the given directory path in "itemPath". If the path is that of a file, it normalizes the path
		and gets its parent folder path and stores it in "itemPath"
		TODO: Accept a bool parameter "isFolder" to overcome the problem specified in the "else" part */
	DeAllocPath();
		
	BEntry entry (path, false);				/* Question: Must we traverse the link here ?? */
	if (entry.Exists() == true)
	{
		if (entry.IsFile() || entry.IsSymLink())
		{
			BEntry parentDir;
			entry.GetParent (&parentDir);
			
			BPath dirPath;
			parentDir.GetPath (&dirPath);
			AllocPath (dirPath.Path());
		}
		else								/* Means it's a directory */
		{
			AllocPath (path);
		}
	}
	else									/* Problem: Assume its a file NOT a directory */
	{
		BPath parentPath;
		BPath dirPath (path);
		dirPath.GetParent (&parentPath);
		
		AllocPath (parentPath.Path());
	}
}

/*============================================================================================================*/

const char *InfoStrView::Path ()
{
	return const_cast<const char*>(itemPath);
}

/*============================================================================================================*/

void InfoStrView::MouseDown (BPoint where)
{
	/* Detect double-click */
	BMessage* msg = Window()->CurrentMessage();
	int32 clicks = msg->FindInt32 ("clicks");
	int32 button = msg->FindInt32 ("buttons");
	static BPoint previousPoint = where;

	if ((button == lastButton) && (clicks > 1))
		clickCount++;
	else
		clickCount = 1;

	lastButton = button;
	
	/* Make sure the two clicks are clicked close to eachother (5 pixel tolerance) */
	if (clickCount >= 2 && button == B_PRIMARY_MOUSE_BUTTON && itemPath != NULL
		&& ((where.y >= previousPoint.y - 5 && where.y <= previousPoint.y + 5)
			&& where.x >= previousPoint.x - 5 && where.x <= previousPoint.x + 5))
	{
		DrawOpenAnimation (CalcAnimationRect (where));
		OpenFolder (itemPath);
	}

	previousPoint = where;
	
	BStringView::MouseDown (where);
}

/*============================================================================================================*/

void InfoStrView::OpenFolder (const char *path) const
{
	/* This function opens a folder entry_ref through Tracker */
	BEntry entry (path, true);
	entry_ref refToDir;
	entry.GetRef (&refToDir);
	
	if (entry.Exists() == true)
	{
		BMessage trakMsg (B_REFS_RECEIVED);
		trakMsg.AddRef ("refs", &refToDir);

		/* Just check if tracker is running */
		if (be_roster->IsRunning (trackerSignature) == true)
			BMessenger(trackerSignature).SendMessage (&trakMsg);
	}
}

/*=============================================================================================================*/

void InfoStrView::DrawOpenAnimation (BRect rect)
{
	/* Hacked from OpenTracker code (some minor modifs) */
	SetDrawingMode (B_OP_INVERT);

	BRect box1 (rect);
	box1.InsetBy (rect.Width() / 2 - 2, rect.Height() / 2 - 2);
	BRect box2 (box1);

	for (int32 index = 0L; index < 5; index++)
	{
		box2 = box1;
		box2.InsetBy (-2, -2);
		StrokeRect (box1, B_MIXED_COLORS);
		Sync();
		StrokeRect (box2, B_MIXED_COLORS);
		Sync();
		snooze (10000);
		StrokeRect (box1, B_MIXED_COLORS);
		StrokeRect (box2, B_MIXED_COLORS);
		Sync();
		box1 = box2;
	}

	SetDrawingMode (B_OP_OVER);
}

/*=============================================================================================================*/

BRect InfoStrView::CalcAnimationRect (BPoint point) const
{
	BRect rect;
	rect.left = point.x;
	rect.top = point.y;
	rect.right = rect.left;
	rect.bottom = rect.top;
	return rect;
}

/*=============================================================================================================*/
