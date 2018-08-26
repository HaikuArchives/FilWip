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
 * Modified by:
 * :Puck Meerburg
 */

#include <Debug.h>
#include <Path.h>
#include <StatusBar.h>
#include <List.h>
#include <Entry.h>
#include <Message.h>
#include <Alert.h>
#include <String.h>
#include <TextView.h>
#include <Application.h>

#include <stdlib.h>
#include <iostream>

#include "Constants.h"
#include "EraserLooper.h"
#include "Preferences.h"

/*============================================================================================================*/

EraserLooper::EraserLooper (int32 priority, BStatusBar *progressBar)
	: BLooper ("The Eraser", priority, B_LOOPER_PORT_DEFAULT_CAPACITY),
		stopErasing (false),
		isRunning (false),
		statusBar (progressBar)
{
	Run();
}

/*============================================================================================================*/

EraserLooper::~EraserLooper ()
{
	int32 itemCount = entryList.CountItems();
	for (int32 i = 0L; i < itemCount; i++)
	{
		BEntry *entry = reinterpret_cast<BEntry*>(entryList.ItemAtFast(i));
		if (entry)
			delete entry;
	}
}

/*============================================================================================================*/

void EraserLooper::EraseList (bool guiMode)
{
	/* The core of the entire project ;-P */
	isRunning = true;
	int32 counter = 0L;
	int32 max = entryList.CountItems();
	
	BWindow *wnd = statusBar->Window();
	BString buf = "0 of ";
	buf << max;

	BMessage initMessage (B_UPDATE_STATUS_BAR), reply;
	initMessage.AddString ("text", "");
	initMessage.AddString ("trailing_text", buf.String());
	initMessage.AddInt32 ("max", max);
	
	BMessenger wndMessenger (wnd);
	wndMessenger.SendMessage (&initMessage, &reply);
	
	/* Use AddFloat here, not inside loop */
	BMessage updateMsg (B_UPDATE_STATUS_BAR);
	updateMsg.AddFloat ("delta", 1.0f);

	bigtime_t start, end;
	start = system_time();

	for (;;)
	{
		const char *path = (const char*)entryList.RemoveItem ((int32)0);
		
		if (!path)
			break;
		
		/* Construct BEntry (don't traverse if it is a link) */
		BEntry entry (path, false);
		entry.Remove();
		delete[] path;
		
		char updateStr[35];
		sprintf (updateStr, "%ld of %ld", ++counter,  max);

		updateMsg.RemoveName ("trailing_text");
		updateMsg.AddString ("trailing_text", updateStr);
		
		if (!wnd || stopErasing == true)
			return;
		
		wnd->PostMessage (&updateMsg, statusBar);
	}

	end = system_time();
	wnd->PostMessage (M_RESET_LOOPERS);		/* Make loopers monitor files again */
	
	timeTaken += (end - start);
	timeTaken /= 1000000.0;

	entryList.MakeEmpty();

	if (prefs.FindBoolDef ("lo_sync", false) == true)
		system ("sync &");
	
	wnd->PostMessage (M_WIPE_DONE);
	
	/* Prepare report */
	/* We use "char" instead of a BString because while appending float values to a BString, 2 decimal
		precision is used, we want more :) */
	const char *indentStr = "          ";
	BString reportStr;
	if (guiMode)
		reportStr = "Report\n\n";
	else
		reportStr = "------\nReport\n------\n\n";
	
	char timeTakenStr [100];
	sprintf (timeTakenStr, "• Time taken:\n%s%g seconds", indentStr, timeTaken);
	
	char spaceFreedStr [100];
	sprintf (spaceFreedStr, "• Disk space freed:\n%s%s", indentStr, GetByteSizeString(totalBytes).String());
	
	char itemsCleanedStr [100];
	sprintf (itemsCleanedStr, "• Items erased:\n%s%li folders\n%s%li files", indentStr, totalFolders, indentStr,
			totalFiles);

	/* Use our own FindBoolDef (so that it passes the default value when pref is not found) */
	bool showSpace = prefs.FindBoolDef ("rv_spaceFree", true);
	bool nFilesDel = prefs.FindBoolDef ("rv_nFilesDeleted", true);
	bool timeTaken = prefs.FindBoolDef ("rv_timeTaken", true);
	
	if (showSpace || nFilesDel || timeTaken)
	{
		
		if (showSpace)
			reportStr << spaceFreedStr << "\n";

		if (nFilesDel)
			reportStr << itemsCleanedStr << "\n";
	
		if (timeTaken)
			reportStr << timeTakenStr << "\n";
	
		/* Show report */
		if (guiMode)
		{
			BFont font (be_plain_font);
			font.SetFace (B_BOLD_FACE);

			BAlert *report = new BAlert ("Report", reportStr.String(), "OK", NULL, NULL,
									B_WIDTH_AS_USUAL, B_EVEN_SPACING, B_INFO_ALERT);
			BTextView *vw = report->TextView();
			vw->SetStylable (true);
			rgb_color alertColor = {0, 0, 198};
			vw->SetFontAndColor (0, 6, &font, B_FONT_ALL, &alertColor);
	
			report->SetShortcut (0L, B_ESCAPE);
			report->Go(NULL);
		}
		else
		{
			std::cout << reportStr.String() << std::endl;
			std::cout.flush ();
		}
	}
	isRunning = false;
	
	/* Must we quit after clean-up? */
	if (prefs.FindBoolDef ("ms_quitApp", false) == true || guiMode == false)
		be_app->PostMessage (B_QUIT_REQUESTED);
}

/*============================================================================================================*/

void EraserLooper::MessageReceived (BMessage *message)
{
	switch (message->what)
	{
		case M_ERASE_LIST:
		{
			message->FindDouble ("prepare_time", &timeTaken);

			bool guiMode = true;
			if (message->FindBool ("is_guimode", &guiMode) != B_OK)
				guiMode = true;

			if (stopErasing == false)
				EraseList (guiMode);
			break;
		}
				
		default:
			BLooper::MessageReceived (message);
			break;
	}
}

/*============================================================================================================*/

BString EraserLooper::GetByteSizeString (int64 v) const
{
	/* Hacked from BeShare with minor changes -- many thanks to Jeremy Freisner,
		don't bother about holding calculated constants for 1024*1024.. etc. as it will be
		optimized by the (GCC) compiler  */
	char buf[256];
	if (v > (1024LL * 1024LL * 1024LL * 1024LL))
		sprintf (buf, "%.2f TiB", ((double)v) / (1024LL * 1024LL * 1024LL * 1024LL));
	else if (v > (1024LL * 1024LL * 1024LL))
		sprintf(buf, "%.2f GiB", ((double)v)/(1024LL * 1024LL * 1024LL));
	else if (v > (1024LL * 1024LL))
		sprintf(buf, "%.2f MiB", ((double)v) / (1024LL * 1024LL));
	else if (v > (1024LL))
		sprintf(buf, "%.2f KiB", ((double)v) / 1024LL);
	else
		sprintf(buf, "%Li bytes", v);
	
	BString str;
	str << buf;
	return str;
}

/*============================================================================================================*/

void EraserLooper::ResetStatVariables ()
{
	totalFiles = totalFolders = totalBytes = 0;
}

/*============================================================================================================*/
