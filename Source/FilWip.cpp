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
 */

#include <AboutWindow.h>
#include <Debug.h>
#include <String.h>
#include <Entry.h>
#include <Mime.h>


#include "FilWip.h"
#include "DataBits.h"
#include "Constants.h"
#include "MainWindow.h"
#include "AdvancedPrefsWindow.h"
#include "PrefsWindow.h"
#include "Preferences.h"

FilWip *filWip;

/*============================================================================================================*/

FilWip::FilWip ()
	: BApplication (AppSignature),
	mainWnd (NULL),
	aboutWnd (NULL),
	prefsWnd (NULL),
	advancedPrefsWnd (NULL),
	forgetWindow (false)
{
	PRINT (("FilWip::FilWip ()\n"));

	if (prefs.FindBoolDef ("ms_register", false) == true)
		RegisterMimeType();
	
	mainWnd = new MainWindow();
}

/*============================================================================================================*/

void FilWip::ReadyToRun ()
{
	mainWnd->isModeGUI = true;
	mainWnd->Show();
}

/*============================================================================================================*/

void FilWip::MessageReceived (BMessage *message)
{
	switch (message->what)
	{
		case B_ABOUT_REQUESTED:
		{
			AboutRequested();
			break;
		}
		
		case M_PREFS:
		{
			if (prefsWnd)
				prefsWnd->Activate();
			else
			{
				prefsWnd = new PrefsWindow();
				prefsWnd->Show();
			}

			break;
		}
		
		case M_CLOSE_PREFS:
		{
			prefsWnd = NULL;
			break;
		}


		case M_ADVANCED_PREFS:
		{
			if (advancedPrefsWnd)
				advancedPrefsWnd->Activate();
			else
			{
				advancedPrefsWnd = new AdvancedPrefsWindow();
				advancedPrefsWnd->Show();
			}

			break;
		}
		case M_CLOSE_ADVANCED_PREFS:
		{
			advancedPrefsWnd = NULL;
			break;
		}
		default:
			BApplication::MessageReceived (message);
	}
}

/*============================================================================================================*/

void FilWip::ArgvReceived (int32 argc, char **argv)
{
	PRINT (("FilWip::ArgvReceived (int32, char**)\n"));
	
	/* Handle arguments sent to us from the command-line */
	if (argc >= 3)
	{
		if (strcasecmp (argv[1], "-preset") == 0)
		{
			entry_ref ref;
			if ((get_ref_for_path (argv[2], &ref) != B_OK) || !mainWnd)
				return;
			
			BMessage argvMsg (M_OPEN_PRESET);
			argvMsg.AddRef ("FileRef", &ref);
			argvMsg.AddBool ("Is GUI Mode", false);
			argvMsg.AddBool ("Doubtful If In List", true);
			
			mainWnd->asyncLoadAllowed = false;
			mainWnd->PostMessage (&argvMsg);
			
			if (argc >= 4 && strcasecmp (argv[3], "-silent") == 0)
				SilentMode();
		}
	}
	
	if (argc >= 2 && strcasecmp (argv[1], "-silent") == 0)
		SilentMode();
}

/*============================================================================================================*/

void FilWip::SilentMode ()
{
	PRINT (("FilWip::SilentMode ()\n"));
	
	if (mainWnd->Lock())
	{
		mainWnd->isModeGUI = false;
		mainWnd->Hide();
		mainWnd->Unlock();
		
		BMessage msg (M_CLEANUP);
		msg.AddBool ("is_guimode", false);
		mainWnd->PostMessage (&msg);
	}
}

/*============================================================================================================*/

void FilWip::RefsReceived (BMessage *message)
{
	PRINT (("FilWip::RefsReceived (BMessage)\n"));
	
	/* Handle arguments sent to us from the Tracker interface */
	uint32 type;
	int32 count;
    entry_ref ref;
	
	message->GetInfo ("refs", &type, &count);
	if (type != B_REF_TYPE)
		return;
	
	/* Do NOT call "OpenPreset()" of mainWnd here, we want to do the loading asynchronously without blocking
		this thread (in case we	show BAlerts on file read errors).	Instead simply pass a message with details */

	if ((message->FindRef ("refs", 0, &ref) == B_OK) && (mainWnd != NULL))
	{
		BMessage refMessage (M_OPEN_PRESET);
		refMessage.AddRef ("FileRef", &ref);
		refMessage.AddBool ("Is GUI Mode", true);
		refMessage.AddBool ("Doubtful If In List", true);

		mainWnd->PostMessage (&refMessage);
	}
}

/*============================================================================================================*/

void FilWip::AboutRequested ()
{
	/* Implement AboutRequested to show about even when external apps like "hey" ask for it */

	BAboutWindow* aboutWin = new BAboutWindow("FilWip", AppSignature);
	aboutWin->AddDescription("A manager to clean-up temporary files");
	const char* extraCopyrights[] = {
		"2013 Puckipedia",
		"2015 Sergei Reznikov, Humdinger",
		"2018 Humdinger, Janus",
		NULL
	};
	const char* authors[] = {
			"Ramshankar (original author)",
			"Humdinger",
			"Janus",
			"Robert Mercer",
			"Puckipedia",
			"Sergei Reznikov",
			NULL
	};
	aboutWin->AddCopyright(2002, "Ramshankar", extraCopyrights);
	aboutWin->AddAuthors(authors);
	aboutWin->Show();
}

/*============================================================================================================*/

void FilWip::RegisterMimeType ()
{
	PRINT (("FilWip::RegisterMimeType ()\n"));

	/* Register the preset file type if prefs allows us */
	BMimeType fileType ("application/x-vnd.FilWip-Preset");

	/* Set file type details  & its icon from "DataBits.h" */
	fileType.SetShortDescription ("FilWip preset");
	fileType.SetLongDescription ("FilWip preset");
	fileType.SetPreferredApp (AppSignature, B_OPEN);
	fileType.SetIcon (kVectorIconBits, sizeof(kVectorIconBits));

	if (fileType.IsInstalled() == false)
		fileType.Install();

}

/*============================================================================================================*/

int main (int argc, char *argv[])
{
	/* The world infamous main! */
	filWip = new FilWip();
	filWip->Run();

	delete filWip;
	return B_OK;
}

/*============================================================================================================*/
