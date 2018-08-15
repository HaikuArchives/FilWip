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

#include <ControlLook.h>
#include <Debug.h>
#include <View.h>
#include <Application.h>
#include <Box.h>
#include <GridView.h>
#include <Screen.h>
#include <LayoutBuilder.h>
#include <ListView.h>
#include <ScrollView.h>
#include <CheckBox.h>
#include <List.h>
#include <StringView.h>
#include <Button.h>
#include <PopUpMenu.h>
#include <MenuField.h>
#include <MenuItem.h>

#include "PrefsWindow.h"
#include "Constants.h"
#include "PrefsView.h"
#include "Preferences.h"


const char * const rv_spaceFreeStr = "Disk space freed",
	*const rv_nFilesDeletedStr = "Number of files, folders deleted",
	*const rv_timeTakenStr = "Time taken to clean-up",
	*const it_autoCheckStartStr = "Check items at startup",
	*const it_autoCheckLiveStr = "Check items live (if monitoring)",
	*const it_unCheckAfterDelStr = "Un-check items after clean-up",
	*const lo_syncStr = "Make loopers " B_UTF8_OPEN_QUOTE "sync" B_UTF8_CLOSE_QUOTE " to disk",
	*const lo_safeStr = "Safe check (recommended)",
	*const lo_monitorStr = "Live monitoring",
	*const lo_priorityFieldStr = "Eraser priority:",
	*const lo_item1Str = "Low priority [5]",
	*const lo_item2Str = "Normal priority [10]",
	*const lo_item3Str = "Display priority [15]",
	*const lo_item4Str = "Urgent display priority [20]",
	*const lo_item5Str = "Low custom priority [30]",
	*const lo_item6Str = "Normal custom priority [35]",
	*const lo_item7Str = "High custom priority [40]",
	*const lo_capacityFieldStr = "Port capacity:",
	*const rm_treeStr = "Tree state",
	*const rm_winPosStr = "Window positions and sizes",
	*const rm_itemsStr = "Clean-up items (changeable by Items options)",
	*const pv_asyncLoadStr = "Load plugins asynchronously",
	*const pv_debugStr = "Debug mode",
	*const pv_checkInstallStr = "Check install path existence",
	*const pv_recurseStr = "Allow recursion into sub-directories",
	*const ms_confirmStr = "Confirm before clean-up",
	*const ms_registerStr = "Register file-types at startup",
	*const ms_quitAppStr = "Quit after clean-up";

/*============================================================================================================*/

PrefsWindow::PrefsWindow()
	: BWindow (BRect (0, 0, 240, 200), "Preferences", B_TITLED_WINDOW, B_NOT_ZOOMABLE | B_NOT_RESIZABLE | B_AUTO_UPDATE_SIZE_LIMITS,
				B_CURRENT_WORKSPACE),
		fPreviousSelection(0)
{
	PRINT (("PrefsWindow::PrefsWindow ()\n"));

	SetFeel (B_MODAL_APP_WINDOW_FEEL);

	saveBtn = new BButton ("Preferences:saveBtn", "Save", new BMessage (M_SAVE_PREFS));

	cancelBtn = new BButton ("Preferences:cancelBtn", "Cancel", new BMessage (M_CLOSE_PREFS));

	BBox *fLabelBoxStartUp = new BBox("SettingsContainerBox2");
	fSettingsBoxStartUp = new BView("SettingsBoxStartUp",0);
	
	BLayoutBuilder::Group<>(fSettingsBoxStartUp, B_VERTICAL,0)
		.SetInsets(B_USE_WINDOW_SPACING)
				.Add(lo_monitorChk = new BCheckBox("Live monitoring"))
				.Add(it_autoCheckStartChk = new BCheckBox("Select as needed"))
		.End();

	fLabelBoxStartUp->AddChild(fSettingsBoxStartUp);
	fLabelBoxStartUp->SetLabel("At start-up");

	BBox *fLabelBoxBeforeCleanUP = new BBox("SettingsContainerBox2");
	fSettingsBoxBeforeCleanUP = new BView("SettingsBoxStartUp",0);

	BLayoutBuilder::Group<>(fSettingsBoxBeforeCleanUP, B_VERTICAL)
		.SetInsets(B_USE_WINDOW_SPACING)
				.Add(ms_confirmChk = new BCheckBox("Ask confirmation"))
		.End();

	fLabelBoxBeforeCleanUP->AddChild(fSettingsBoxBeforeCleanUP);
	fLabelBoxBeforeCleanUP->SetLabel("Before clean up");

	BBox *fLabelBoxAfterCleanUP = new BBox("SettingsContainerBox2");
	fSettingsBoxAfterCleanUP = new BView("SettingsBoxStartUp",0);
	BLayoutBuilder::Group<>(fSettingsBoxAfterCleanUP, B_VERTICAL,0)
		.SetInsets(B_USE_WINDOW_SPACING)
				.Add(showReportChk = new BCheckBox("Show report"))
				.Add(it_unCheckAfterDelChk = new BCheckBox("Deselect all"))
				.Add(ms_quitAppChk = new BCheckBox("Quit"))
		.End();

	fLabelBoxAfterCleanUP->AddChild(fSettingsBoxAfterCleanUP);
	fLabelBoxAfterCleanUP->SetLabel("After clean up");

	BLayoutBuilder::Group<>(this)
		.SetInsets(B_USE_WINDOW_SPACING)
			.AddGroup(B_VERTICAL, B_USE_DEFAULT_SPACING)
				.Add(fLabelBoxStartUp)
				.Add(fLabelBoxBeforeCleanUP)
				.Add(fLabelBoxAfterCleanUP)
				.AddGroup(B_HORIZONTAL, B_USE_DEFAULT_SPACING)
					.AddStrut(be_control_look->DefaultItemSpacing() * 5)
					.AddGlue()
					.Add(cancelBtn)
					.Add(saveBtn)
				.End()
	.End();

	/* Center window on-screen */
	BRect screen_rect (BScreen().Frame());
	MoveTo (screen_rect.Width() / 2 - Frame().Width() / 2, screen_rect.Height() / 2 - Frame().Height() / 2);
	
	/* Check if there is a position in prefs, if so move to that point */
	bool mustRestore;
	if (prefs.FindBool ("rm_winPos", &mustRestore) == B_OK)
	{
		if (mustRestore == true)
		{
			BPoint wndPt;
			if (prefs.FindPoint ("prefwnd_point", &wndPt) == B_OK)
				MoveTo (wndPt);
		}
	}
	Load();
}

/*============================================================================================================*/

void PrefsWindow::Show ()
{
	BWindow::Show();
}

/*============================================================================================================*/

void PrefsWindow::Quit ()
{
	prefs.SetPoint ("prefwnd_point", Frame().LeftTop());
	be_app_messenger.SendMessage (M_CLOSE_PREFS);
	BWindow::Quit();
}

/*============================================================================================================*/

void PrefsWindow::MessageReceived (BMessage *message)
{
	switch (message->what)
	{
		case M_SAVE_PREFS:
		{
			Save();
			prefs.SetPoint ("prefwnd_point", Frame().LeftTop());
			prefs.WriteSettings();
			/* no "break" continue and close this window */
		}
		case M_CLOSE_PREFS:
		{
			Quit();
			break;
		}
	}
	BWindow::MessageReceived (message);
}


/*============================================================================================================*/

void PrefsWindow::Save ()
{
	PRINT (("PrefsWindow::Save ()\n"));

	// StartUp
	prefs.SetBool ("lo_monitor", IsChecked (lo_monitorChk));
	prefs.SetBool ("it_autoCheckStart", IsChecked (it_autoCheckStartChk));
	// Before
	prefs.SetBool ("ms_confirm", IsChecked (ms_confirmChk));
	// After Clean up
	// show report
	
	bool reportActive = prefs.FindBoolDef ("rv_spaceFree", true)
						|| prefs.FindBoolDef ("rv_nFilesDeleted", true)
						|| prefs.FindBoolDef ("rv_timeTaken", true);

	if (IsChecked (showReportChk) != reportActive) {
		prefs.SetBool ("rv_spaceFree", IsChecked (showReportChk));
		prefs.SetBool ("rv_nFilesDeleted", IsChecked (showReportChk));
		prefs.SetBool ("rv_timeTaken", IsChecked (showReportChk));
	}
	prefs.SetBool ("it_unCheckAfterDel", IsChecked (it_unCheckAfterDelChk));
	prefs.SetBool ("ms_quitApp", IsChecked (ms_quitAppChk));
}

/*============================================================================================================*/

void PrefsWindow::Load ()
{
	PRINT (("PrefsWindow::LoadViewReport ()\n"));

	// StartUp
	lo_monitorChk->SetValue (CheckBoxValue (prefs.FindBoolDef ("lo_monitor", true)));
	it_autoCheckStartChk->SetValue (CheckBoxValue (prefs.FindBoolDef ("it_autoCheckStart", false)));
	// Before
	ms_confirmChk->SetValue (CheckBoxValue (prefs.FindBoolDef ("ms_confirm", true)));
	// After Clean up
	// show report
	it_unCheckAfterDelChk->SetValue (CheckBoxValue (prefs.FindBoolDef ("it_unCheckAfterDel", false)));
	ms_quitAppChk->SetValue (CheckBoxValue (prefs.FindBoolDef ("ms_quitApp", false)));
	showReportChk->SetValue (CheckBoxValue (prefs.FindBoolDef ("rv_spaceFree", true)
										|| prefs.FindBoolDef ("rv_nFilesDeleted", true)
										|| prefs.FindBoolDef ("rv_timeTaken", true)));
}


/*============================================================================================================*/

bool PrefsWindow::IsChecked (BCheckBox *chkBox) const
{
	/* Convert checkbox's value as a bool */
	if (chkBox->Value() == B_CONTROL_ON)
		return true;
	else
		return false;
}

/*============================================================================================================*/

int32 PrefsWindow::CheckBoxValue (bool value) const
{
	/* Convert a bool value as a checkbox value*/
	if (value == true)
		return B_CONTROL_ON;
	else
		return B_CONTROL_OFF;
}

/*============================================================================================================*/
