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
 * Humdinger
 */

#include <Roster.h>
#include <Application.h>
#include <TextView.h>
#include <Box.h>
#include <Button.h>
#include <CheckBox.h>
#include <LayoutBuilder.h>
#include <Screen.h>
#include <Resources.h>
#include <Alert.h>
#include <String.h>
#include <FilePanel.h>
#include <NodeInfo.h>
#include <MenuField.h>
#include <PopUpMenu.h>
#include <MenuItem.h>
#include <Volume.h>
#include <VolumeRoster.h>
#include <ScrollView.h>
#include <Debug.h>
#include <StatusBar.h>
#include <Beep.h>
#include <FindDirectory.h>
#include <ControlLook.h>
#include <IconUtils.h>
#include <TranslationUtils.h>


#include <ctype.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#include "MainWindow.h"
#include "Constants.h"
#include "Preferences.h"
#include "FileLooper.h"
#include "PluginParser.h"
#include "InfoStrView.h"
#include "FilWip.h"
#include "EraserLooper.h"
#include "ElementListView.h"
#include <ColumnTypes.h>
#include "CheckBoxWithStringColumn.h"
using namespace std;

#define B_TRANSLATE(x) x

bool
SelectItems(BRow *row, void *data)
{
	BField *field = row->GetField(ROW_FIELD_ENTRIES_STRING_WITH_CHECKBOX);
	CheckBoxWithStringField* checkBoxStringField = static_cast<CheckBoxWithStringField*>(field);
	BColumnListView* columnListView = static_cast<BColumnListView*>(data);
	checkBoxStringField->SetMarked(true);
	columnListView->InvalidateRow(row);
	return true;
}


bool
SelectSmartItems(BRow *row, void *data)
{
	BField *field = row->GetField(ROW_FIELD_ENTRIES_STRING_WITH_CHECKBOX);
	BField *fieldSize = row->GetField(ROW_FIELD_BYTES_COUNTS);

	CheckBoxWithStringField* checkBoxStringField = static_cast<CheckBoxWithStringField*>(field);
	BSizeField* sizeField = static_cast<BSizeField*>(fieldSize);

	bool selected = sizeField != NULL && sizeField->Size() > 0;
	BColumnListView* columnListView = static_cast<BColumnListView*>(data);
	checkBoxStringField->SetMarked(selected);
	columnListView->InvalidateRow(row);
	return true;
}


bool
DeselectItems(BRow *row, void *data)
{
	BField *field = row->GetField(ROW_FIELD_ENTRIES_STRING_WITH_CHECKBOX);
	CheckBoxWithStringField* checkBoxStringField = static_cast<CheckBoxWithStringField*>(field);
	BColumnListView* columnListView = static_cast<BColumnListView*>(data);
	checkBoxStringField->SetMarked(false);
	columnListView->InvalidateRow(row);
	return true;
}


class MainWindow *pWnd;

/*============================================================================================================*/

MainWindow::MainWindow ()
	: BWindow (BRect(240, 160, 700, 536), AppTitle, B_TITLED_WINDOW, B_CURRENT_WORKSPACE)
{
	PRINT (("MainWindow::MainWindow ()\n"));
	
	/* Initialise some pointers & then some */
	pWnd = this;
	savePanel = NULL;
	messenger = new BMessenger (this);
	asyncLoadAllowed = true;
	liveMonitoring = prefs.FindBoolDef ("lo_monitor", true);
	debugMode = prefs.FindBoolDef ("pv_debug", false);
	allowRecurse = prefs.FindBoolDef ("pv_recurse", true);

	/* Initialize BDirectory for Presets & Docs (if found) */
	BPath presetsPath;
	if (find_directory(B_USER_SETTINGS_DIRECTORY, &presetsPath) != B_OK)
		return;

	presetsFolder = BDirectory(presetsPath.Path());
	presetsPath.Append("FilWip");

	if (!presetsFolder.Contains(presetsPath.Path()))
		presetsFolder.CreateDirectory(presetsPath.Path(), NULL);

	presetsPath.Append("Presets");

	if (!presetsFolder.Contains(presetsPath.Path()))
		presetsFolder.CreateDirectory(presetsPath.Path(), NULL);

	presetsFolder.SetTo(presetsPath.Path());

	app_info appInfo;
	be_app->GetAppInfo (&appInfo);
	
	BEntry appEntry (&appInfo.ref);
	appEntry.GetParent (&appEntry);

	BPath docsPath (&appEntry);
	if (docsPath.Append ("Docs/") == B_OK)
		docsFolder.SetTo (docsPath.Path());

	/* Move window to the center of the screen & set size limits (default position, size) */
	BRect screen_rect (BScreen().Frame());
	float minH, maxH, minV, maxV;

	MoveTo (screen_rect.Width() / 2 - Frame().Width() / 2, screen_rect.Height() / 2 - Frame().Height() / 2);
	GetSizeLimits (&minH, &maxH, &minV, &maxV);
	SetSizeLimits (Frame().Width(), maxH, Frame().Height(), maxV);

	/* Restore window position? Do this now itself so we get our controls sized right */
	bool windowPosition = true;
	if (prefs.FindBool ("rm_winPos", &windowPosition) == B_OK)
		if (windowPosition == true)
			RestoreWindowPosition();

	/* Begin drawing more controls */
	boxView = new BBox("MainWindow:BBox");

	statusBar = new BStatusBar ("statusBar");
	statusBar->Hide();

	// Menu Bar
	BMenuBar* menuBar = new BMenuBar("MenuBar");
	BMenu* menu;
	menu = new BMenu("FilWip");
	menuBar->AddItem(menu);
	menu->AddItem(new BMenuItem(B_TRANSLATE("Preferences..."), new BMessage(M_PREFS), ','));
	menu->AddSeparatorItem();
	menu->AddItem(new BMenuItem(B_TRANSLATE("Help"), new BMessage(M_HELP)));
	menu->AddItem(new BMenuItem(B_TRANSLATE("About FilWip..."), new BMessage(B_ABOUT_REQUESTED)));
	menu->AddSeparatorItem();
	menu->AddItem(new BMenuItem(B_TRANSLATE("Quit"), new BMessage(B_QUIT_REQUESTED), 'Q'));

	menu = new BMenu(B_TRANSLATE("Selection"));
	menu->AddItem(new BMenuItem(B_TRANSLATE("Select all"), new BMessage(M_SELECT_ALL), 'A'));
	menu->AddItem(new BMenuItem(B_TRANSLATE("Deselect all"), new BMessage(M_DESELECT_ALL), 'D'));
	menu->AddItem(new BMenuItem(B_TRANSLATE("Select as needed"), new BMessage(M_SMART_SELECT), 'M'));
	menu->AddSeparatorItem();
	menu->AddItem(new BMenuItem(B_TRANSLATE("Save as preset"), new BMessage(M_SAVE_PRESET), 'S'));
	menuBar->AddItem(menu);

	// Tool Bar
	mainToolBar = new BToolBar(B_HORIZONTAL);

	mainToolBar->AddAction(new BMessage(M_SELECT_ALL), this, ResVectorToBitmap("MARKED_CHECKBOX"),"Select all (Alt-A)","",false);
	mainToolBar->AddAction(new BMessage(M_DESELECT_ALL), this, ResVectorToBitmap("EMPTY_CHECKBOX"),"Deselect all (Alt-D)","",false);
	mainToolBar->AddAction(new BMessage(M_SMART_SELECT),this, ResVectorToBitmap("SMART_CHECKBOX"), "Select as needed (Alt-M)","",false);
	mainToolBar->GroupLayout()->AddItem(BSpaceLayoutItem::CreateHorizontalStrut(B_USE_HALF_ITEM_SPACING));
	mainToolBar->AddSeparator();
	mainToolBar->GroupLayout()->AddItem(BSpaceLayoutItem::CreateHorizontalStrut(B_USE_HALF_ITEM_SPACING));
	mainToolBar->AddAction(new BMessage(M_SAVE_PRESET), this, ResVectorToBitmap("SAVE"), "Save preset (Alt-S)","",false);
	mainToolBar->AddAction(new BMessage(M_PREFS),this, ResVectorToBitmap("PREFERENCES"),"Preferences","",false);

	mainToolBar->AddAction(new BMessage(M_HELP), this, ResVectorToBitmap("HELP"),"Help (F1)","",false);
//	mainToolBar->AddAction(new BMessage(B_ABOUT_REQUESTED),this, ResVectorToBitmap("ABOUT"),"About","",false);
//	mainToolBar->AddAction(new BMessage(M_PREVIEW),this,
//		previewButtonBitmap /*ResVectorToBitmap("PREVIEW") */,"Preview (Alt-P)","",false);


	/* Create tooltips for all the toolbar buttons */
	/*
	toolTip->SetHelp (presetField, "Load preset options");
	toolTip->SetHelp (cleanUp, "Begin the erasing process");
	*/

	mainToolBar->AddGlue();
	/* "Select As needed" won't work if there are no infostrings */
	/* TODO FIX THIS
	if (liveMonitoring == false)
		smartSelectButton->Hide();
	*/
	/*
	static const float spacing = be_control_look->DefaultLabelSpacing();
	BGroupLayout *boxLayout = BLayoutBuilder::Group<>(B_HORIZONTAL)
		.SetInsets(B_USE_WINDOW_INSETS, B_USE_WINDOW_INSETS,
			B_USE_WINDOW_INSETS, B_USE_WINDOW_INSETS)
		.Add(fElementListView = new ElementListView("MainWindow:ElementListView"))
		.Add(mainToolBar);
	boxView->AddChild(boxLayout->View()); */

	/* Draw the button, busyview and presetField AFTER resizing the boxView */
	cleanUp = new BButton ("MainWindow:CleanUp", "Clean up", new BMessage (M_CLEANUP));
	cleanUp->MakeDefault (true);

	presetPopup = new BPopUpMenu ("");
	presetField = new BMenuField ("MainWindow:PresetPopUp",
						"Presets:", (BMenu*)presetPopup);

	/* Add shortcuts */
	AddShortcut ('s', B_COMMAND_KEY, new BMessage (M_SAVE_PRESET));
	AddShortcut ('p', B_COMMAND_KEY, new BMessage (M_PREVIEW));
	AddShortcut ('a', B_COMMAND_KEY, new BMessage (M_SELECT_ALL));
	AddShortcut ('d', B_COMMAND_KEY, new BMessage (M_DESELECT_ALL));	
	AddShortcut ('m', B_COMMAND_KEY, new BMessage (M_SMART_SELECT));

	/* List presets */
	ListPresets ();
	

	/* Read the port capacity for our FileLooper threads (member since we spawn FileLoopers from many
		places in the code) */
	if (prefs.FindInt16 ("lo_capacity", &looperPortCapacity) != B_OK)
		looperPortCapacity = 1000;
	
	/* Launch our EraserLooper */
	int8 eraserPriority;
	if (prefs.FindInt8 ("lo_priority", &eraserPriority) != B_OK)
		eraserPriority = B_NORMAL_PRIORITY;
	
	eraserLooper = new EraserLooper (eraserPriority, statusBar);

	BLayoutBuilder::Group<>(this, B_VERTICAL, 5)
		.SetInsets(0, 0, 0, 0)
		.Add(menuBar)
		.Add(mainToolBar)
		.AddGroup(B_VERTICAL)
		.SetInsets(B_USE_WINDOW_INSETS, 0, B_USE_WINDOW_INSETS, B_USE_WINDOW_INSETS)
			.Add(fElementListView = new ElementListView("MainWindow:ElementListView"), 100)
			.AddGroup(B_HORIZONTAL, 0)
				//.SetInsets(0, 3, B_USE_ITEM_SPACING, 0)
				.Add(presetField,1)
				.Add(statusBar, 10)
				.AddGlue()
				.Add(cleanUp,1)
			.End()
		.End()
		.End();
}

/*============================================================================================================*/

MainWindow::~MainWindow ()
{
	PRINT (("MainWindow::~MainWindow ()\n"));

	/* Fill the "prefs" message with the options */	
	FillPreset ((BMessage*)&prefs);
	
	/* Save window position into prefs */	
	SaveWindowPosition ();

	/* Delete un-attached objects, don't delete the fileLooper BList, as
		each looper receives a B_QUIT_REQUESTED message and deletes itself */
	delete messenger;
	
	PluginContainerItem *cItem (NULL);
	do
	{
		cItem = (PluginContainerItem*)containerItems.RemoveItem ((int32)0);
		if (cItem != NULL)
			delete cItem;
	} while (cItem != NULL);
	
}

/*============================================================================================================*/

void MainWindow::Show ()
{
	PRINT (("MainWindow::Show()\n"));

	/* Check for the plugins folder and act accordingly */	
	BPath pluginsPath;
	if (find_directory (B_USER_SETTINGS_DIRECTORY, &pluginsPath) != B_OK)
		return;

	pluginsPath.Append("FilWip");
	if (pluginsPath.Append ("Plugins/") == B_OK)
		pluginsFolder.SetTo (pluginsPath.Path());
	
	BEntry pluginEntry;
	pluginsFolder.GetEntry (&pluginEntry);
	
	if (pluginEntry.Exists() == false || pluginEntry.IsDirectory() == false)
	{
		TellUserNoPlugins();
		return;
	}

	bool asyncLoad = prefs.FindBoolDef ("pv_asyncLoad", false);

	/* Set isProcessingPlugins to true so that FrameResized does not call RecalcScrollBar while 
		ParseAndSetupUI is doing its stuff, After Show() message loop starts then PostMessage to process
		the plugins and generate GUI at the end of which that function will reset isProcessingPlugins
		to false (Very efficient and fast if we go by this method of using MessageReceived) */
	isProcessingPlugins = true;
	fElementListView->MakeFocus (true);		/* Put focus on this view so keyboard works (arrow key scrolling) */

	/* If asynchronous plugin load mode, then show the window, then load plugins through MessageReceived() */
	if (asyncLoad == true && asyncLoadAllowed == true)
	{
		PRINT ((" >> load mode: asynchronous\n"));
		
		BWindow::Show();
		PostMessage (M_PARSE_AND_SETUP_UI);
	}
	else
	{
		PRINT ((" >> load mode: synchronous\n"));

		/* In sychronous mode, load everything then finally show window (start message loop) */
		ParseAndSetupUI();
		
		/* Check prefs if we want to open the preview window now */
		//if (prefs.FindBool ("show_preview") == true && isModeGUI == true)
		//	be_app_messenger.SendMessage (M_PREVIEW);
		
		BWindow::Show();
	}
}

/*============================================================================================================*/

void MainWindow::ParsePlugins (BDirectory pluginFolder)
{
	PRINT (("MainWindow::ParsePlugins (BDirectory)\n"));

	/* This function parses all the plugins using the PluginParser class */
	BEntry entry;

	bool checkInstall = prefs.FindBoolDef ("pv_checkInstall", true);
	
	while (pluginFolder.GetNextEntry (&entry, true) != B_ENTRY_NOT_FOUND)
	{
		PluginContainerItem *cItem = new PluginContainerItem();
		
		BPath filePath;
		entry.GetPath (&filePath);
		
		PluginParser parseMaster (filePath.Path());
		if (parseMaster.DoesFileExist() == false)
			continue;
		
		parseMaster.FillContainerItem (cItem);

		/* Don't add items that have CHECK_INSTALL paths that don't exist + prefs must have this check ON */
		if (strlen (cItem->checkPath.String()) > 0L && checkInstall == true)
		{
			BEntry checkPath (cItem->checkPath.String(), false);
			if (checkPath.Exists() == false)
				continue;
		}

		if (cItem->isLinear == true)
		{
			AddLinearItem (cItem, (char*)filePath.Leaf());
		}
		else
		{
			AddHierarchialItem (cItem,(char*)filePath.Leaf());
		}
		
		containerItems.AddItem ((void*)cItem);
	}
}

/*============================================================================================================*/

void MainWindow::TellUserNoPlugins () const
{
	PRINT (("MainWindow::TellUserNoPlugins ()\n"));

	/* OK either the plugins folder is messed up or is empty, tell user the error and commit suicide */
	BAlert *alert = new BAlert ("Error", B_UTF8_OPEN_QUOTE "Plugins" B_UTF8_CLOSE_QUOTE
							" directory not found...", "All right", NULL, NULL,
							B_WIDTH_AS_USUAL, B_EVEN_SPACING, B_STOP_ALERT);
	alert->Go();
	be_app->PostMessage (B_QUIT_REQUESTED);
}

/*============================================================================================================*/

void MainWindow::RestoreWindowPosition ()
{
	PRINT (("MainWindow::RestoreWindowPosition ()\n"));

	/* Move & resize the window as per prefs
		Later add checks for seeing if window is off-screen etc */
	BRect rect;
	if (prefs.FindRect ("mainwnd_frame", &rect) == B_OK)
	{
		MoveTo (rect.LeftTop());
		ResizeTo (rect.Width(), rect.Height() - 4.0);

		PRINT ((" >> prefs->main_wnd_frame=%g  Action=LOAD\n", Frame().Height() - 4.0));
	}
}

/*============================================================================================================*/

void MainWindow::SaveWindowPosition () const
{
	PRINT (("MainWindow::SaveWindowPosition ()\n"));

	/* Remove previous setting and save current window size, position */
	prefs.SetRect ("mainwnd_frame", Frame());

	PRINT ((" >> prefs->main_wnd_frame=%g  Action=SAVE\n", Frame().Height()));
}

/*============================================================================================================*/

void MainWindow::LoadTreeState (BMessage *prefsMessage)
{
	PRINT (("MainWindow::LoadTreeState (BMessage*)\n"));
	
	/* Collapse/Expand tree structure from "message" */
	fElementListView->LoadTreeState(prefsMessage);
}

/*============================================================================================================*/

status_t MainWindow::OpenPreset (entry_ref *ref, bool guiMode, bool addToList)
{
	PRINT (("MainWindow::OpenPreset (entry_ref*, bool, bool)\n"));

	/* Self defined error codes: */
	BString errors[] =
	{
		/* 0 */		"Your file isn't real! It doesn't exists!",
		/* 1 */		"Don't you know I can't handle directories!!",
		/* 2 */		"How about giving me something I can handle?",
		/* 3 */		"Incompatible preset version",
		/* 4 */		"Unknown error - What rubbish WAS that!?!"
	};
	
	int32 error_code (-1);
	if (ref != NULL)
	{
		BMessage presetMsg;
		BFile file;
		if (file.SetTo (ref, B_READ_ONLY) != B_OK)
		{
			error_code = 0;
		}
		else
		{	
			if (file.IsFile() == true)
			{
				if (presetMsg.Unflatten (&file) == B_OK)
				{
					float veriCode;
					if (presetMsg.FindFloat ("presetVerifyCode", &veriCode) == B_OK)
					{
						if (veriCode == PresetVerifyCode)
						{
							LoadPreset (&presetMsg);

							/* Add to our preset list if its valid */
							if (addToList == true)
							{
								BMessage *msg = new BMessage (M_OPEN_PRESET);
								msg->AddBool ("Already In List", true);
								msg->AddRef ("FileRef", ref);
								
								/* Add separator item if its the first custom preset */
								if (presetPopup->CountItems() == 1L)
									presetPopup->AddSeparatorItem();
								
								BMenuItem *item = new BMenuItem (ref->name, msg);
								presetPopup->AddItem (item);
								item->SetMarked (true);
							}

							return B_OK;
						}
						else
						{
							error_code = 3;
						}
					}
					else
					{
						error_code = 2;
					}
				}
				else
				{
					error_code = 2;
				}
			}
			else
			{
				if (file.IsDirectory() == true)
					error_code = 1;
				else
					error_code = 4;
			}
		}
	}
	
	
	/* Popup the error according to the mode */
	beep();
	if (guiMode == true)
	{
		BAlert *errorAlert = new BAlert ("Error", errors[error_code].String(),
							"OK", NULL, NULL, B_WIDTH_AS_USUAL,
							B_EVEN_SPACING, B_WARNING_ALERT);
		errorAlert->Go ();
	}
	else
	{
		cerr << endl << errors[error_code].String() << endl;
		cerr.flush ();
	}
	
	return B_ERROR;
}

/*============================================================================================================*/

void MainWindow::LoadPreset (BMessage *presetMessage)
{
	PRINT (("MainWindow::LoadPreset (BMessage*)\n"));

	BMessage message;
	if (presetMessage->FindMessage ("plugin_settings", &message) == B_OK)
	{
		fElementListView->LoadPreset(&message);
	}
}

/*============================================================================================================*/

void MainWindow::Quit ()
{
	PRINT (("MainWindow::Quit ()\n"));

	if (eraserLooper)
	{
		if (eraserLooper->isRunning == true)
		{
			suspend_thread (eraserLooper->Thread());
			int32 warning;
			warning = (new BAlert ("Warning", "Clean-up process is in progress, force it to stop?",
						"Don't force", "Force", NULL, B_WIDTH_AS_USUAL, B_EVEN_SPACING, B_WARNING_ALERT))->Go();
			
			if (warning == 1)
				eraserLooper->stopErasing = true;
			else
			{
				resume_thread (eraserLooper->Thread());
				return;
			}
		}

		eraserLooper->PostMessage (B_QUIT_REQUESTED);
	}

	/* Store the tree-view like structure */
	FillTreeState (&prefs);

	/* Store Columns disposition */
	BMessage columnViewState('clms');
	fElementListView->SaveState(&columnViewState);

	prefs.RemoveName ("columns_states");
	prefs.AddMessage ("columns_states", &columnViewState);

	/* We're done, time to go */
	be_app_messenger.SendMessage (B_QUIT_REQUESTED);
	return BWindow::Quit();
}

/*============================================================================================================*/

void MainWindow::WindowActivated (bool state)
{
	PRINT (("MainWindow::WindowActivated (bool)\n"));
}

/*============================================================================================================*/

void MainWindow::FrameResized (float newWidth, float newHeight)
{
	PRINT (("MainWindow::FrameResized (float,float)\n"));

	BWindow::FrameResized(newWidth, newHeight);
}

/*============================================================================================================*/

void MainWindow::MessageReceived (BMessage *message)
{
	switch (message->what)
	{
		case M_OVERVIEW_STATS:
		{
			if (isModeGUI == false)
				break;

			bool first_scan = false;
			int32 looperID = message->FindInt32 ("looper_id");
			int64 bytesCounted = message->FindInt64 ("bytes_counted");
			int64 entriesCounted = message->FindInt64 ("entries_counted");
			BRow *row;
			status_t status = message->FindPointer ("row_pointer", (void**)&row);
			if (status != B_OK) {
				PRINT (("M_OVERVIEW_STATS empty row_pointer\n"));
				break;
			};
			/*
			BString buf = "[";
			buf << entriesCounted << (entriesCounted == 1 ? " item, " : " items, ");
			buf << GetByteSizeString (bytesCounted) << "]";
			
			*/
			BIntegerField* itemsField = (BIntegerField*)(row->GetField(ROW_FIELD_ENTRIES_COUNTS));
			if (itemsField->Value() != entriesCounted) {
				itemsField->SetValue(entriesCounted );
			}
			BSizeField* sizeField = (BSizeField*)(row->GetField(ROW_FIELD_BYTES_COUNTS));
			if (sizeField->Size() != bytesCounted) {
				sizeField->SetSize(bytesCounted);
			}
			fElementListView->UpdateRow(row);

			/* Make sure we HAVE a "default" item. This item will NOT be there if the settings
				file is deleted. In which case we cannot use IsMarked() on a NULL item. */
			if (presetPopup->ItemAt(0L))
			{
				/* Don't check items if a custom preset is loaded */
				if (presetPopup->ItemAt(0L)->IsMarked() == true)
				{
					/* Check if it is the first time we are getting this, then check if prefs wants to check
						item at startup (ie first time)  */
					if (message->FindBool ("first_scan", &first_scan) == B_OK)
					{
						if (prefs.FindBoolDef ("it_autoCheckStart", false) == true)
						{
							if (first_scan == true)
							{
								bool firstMark = (entriesCounted > 0);	
								((CheckBoxWithStringField*)checkBoxesFields.ItemAtFast(looperID))->SetMarked(firstMark);
							}
						}
						else if (prefs.FindBoolDef ("it_autoCheckLive", false) == true)
						{
							/* Do live check even if it is the first time! */
								bool firstMark = (entriesCounted > 0);	
								((CheckBoxWithStringField*)checkBoxesFields.ItemAtFast(looperID))->SetMarked(firstMark);
						}
					}
					else
					{
						/* Live checking/unchecking (if prefs wants) */
						if (prefs.FindBoolDef ("it_autoCheckLive", false) == true)
						{
								bool firstMark = (entriesCounted > 0);		
								((CheckBoxWithStringField*)checkBoxesFields.ItemAtFast(looperID))->SetMarked(firstMark);
						}
					}
				}
			}
					
			break;
		}
		
		/* Startup message call to parse plugins and setup interface */
		case M_PARSE_AND_SETUP_UI:
		{
			ParseAndSetupUI ();
			break;
		}
		
		/* These are handled by our BApp object */
		case B_ABOUT_REQUESTED: case M_PREVIEW: case M_PREFS:
		{
			be_app_messenger.SendMessage (message);
			break;
		}

		/* Call Help */
		case M_HELP:
		{
			BPath helpPath (&docsFolder, "Index.html");
			BEntry helpFile (helpPath.Path(), true);

			if (helpFile.Exists() == false)
			{
				BAlert *errAlert = new BAlert ("Error", "Couldn't locate the help files!",
									"OK", NULL, NULL, B_WIDTH_AS_USUAL, B_STOP_ALERT);
				errAlert->Go();
			}
			else
			{
				char *help_url = const_cast<char*>(helpPath.Path());
				be_roster->Launch ("application/x-vnd.Be.URL.http", 1, &help_url);
			}

			break;
		}
		
		case B_UPDATE_STATUS_BAR:
		{
			/* Initially received from eraser */
			UpdateIfNeeded();
			statusBar->SetMaxValue (message->FindInt32 ("max"));
			statusBar->SetText ("Deleting...");
			message->RemoveName ("max");
			message->SendReply ('dumb');
			break;
		}
		
		/* The clean-up process begins! */
		case M_CLEANUP:
		{
			/* If there are no plugins at all, quit the app */
			if (CheckIfPluginsExist ("No plugins to work with" B_UTF8_ELLIPSIS) == false)
				return;

			/* Check if any plugins are checked or not */
			if (CountOptions() == 0)
			{
				TellUserNoOptions();
				break;
			}
			
			/* Discover gui mode, if silent mode, don't popup confirmations or reports etc. */
			bool mode = false;
			if (message->FindBool ("is_guimode", &mode) == B_OK)
				isModeGUI = mode;
			else
				isModeGUI = true;
			
			/* Check if user wants us to confirm if so confirm */
			if (prefs.FindBoolDef ("ms_confirm", true) == true && isModeGUI == true)
				if (ConfirmCleanUp() == false)
					break;
			
			/* Important we reset these stat variables */
			if (eraserLooper->Lock())
			{
				eraserLooper->ResetStatVariables();
				eraserLooper->Unlock();
			}
			
			presetField->Hide();
			statusBar->Show();
			statusBar->Reset();
			statusBar->SetText ("Preparing to delete");

			bigtime_t start, end;
			start = system_time();
			
			int32 looperCount = fileLoopers.CountItems();
			for (int32 i = 0; i < looperCount; i++)
			{
				FileLooper *fileLooper = (FileLooper*)fileLoopers.ItemAtFast(i);
				if ((((CheckBoxWithStringField*)checkBoxesFields.ItemAtFast(i))->IsMarked())
					&& (fileLooper->Lock() == true))
				{
					fileLooper->BeginAddOperation();
					fileLooper->Unlock();
				}
				UpdateIfNeeded();
			}
			
			/* Seems our time will always be longer as we time things like Lock() calls & other
				calls like INSIDE BeginAddOperation() LockLoopers etc. So our time isn't very accurate */
			end = system_time();
			
			UpdateIfNeeded();
			BMessage delMessage (M_ERASE_LIST);
			delMessage.AddDouble ("prepare_time", (double)(end - start));
			delMessage.AddBool ("is_guimode", isModeGUI);

			ResetLoopers (true);
			eraserLooper->PostMessage (&delMessage);

			break;
		}

		case M_RESET_LOOPERS:
		{
			ResetLoopers (false);
			break;
		}
				
		case M_WIPE_DONE:
		{
			/* Called from be_app (got there from EraserLooper) */
			if (presetField->IsHidden() == true)
			{
				statusBar->Hide();
				presetField->Show();
			}

			/* Must we uncheck all boxes after clean-up ? Check prefs and do accordingly */
			if (prefs.FindBoolDef ("it_unCheckAfterDel", false) == true)
				PostMessage (M_DESELECT_ALL);
			
			break;
		}
		
		/* Read a preset file from disk */
		case M_OPEN_PRESET:
		{
			entry_ref ref;
			if (message->FindRef ("FileRef", &ref) == B_OK)
			{
				bool mode = true;
				bool add = false;
				bool doubt = false;
				message->FindBool ("Is GUI Mode", &mode);
				message->FindBool ("Already In List", &add);
				if (message->FindBool ("Doubtful If In List", &doubt) == B_OK)
				{
					BMenuItem *theItem = NULL;
					if (doubt == true)
					{
						entry_ref ref2;
						for (int32 i = 0; i < presetPopup->CountItems(); i++)
						{
							theItem = presetPopup->ItemAt (i);
							if (theItem->Message() == NULL)
								continue;

							if (theItem->Message()->FindRef ("FileRef", &ref2) == B_OK)
							{
								if (ref2 == ref)
								{
									add = true;
									break;
								}
							}
						}

						if (add == true)
							theItem->SetMarked (true);
					}
				}
				
				OpenPreset (&ref, mode, !add);
			}
			
			break;
		}

		/* User wants to save a preset */
		case M_SAVE_PRESET:
		{
			if (CheckIfPluginsExist ("No plugins to save" B_UTF8_ELLIPSIS) == false)
				return;


			/* Setup stuff to perform a save */
			CreateSavePanel ();
			savePanel->SetSaveText ("MyPreset");
			savePanel->SetPanelDirectory (&presetsFolder);
			savePanel->Show ();
			break;
		}
		
		/* User has confirmed the save */
		case B_SAVE_REQUESTED:
		{
			entry_ref entryRef;
			BPath filePath;
			BEntry fileEntry;
			const char *fileName (NULL);

			DeleteSavePanel ();
			if ((message->FindRef ("directory", &entryRef) == B_OK) &&
				(message->FindString ("name", &fileName) == B_OK))
			{
	    		if (fileEntry.SetTo (&entryRef, true) == B_OK)
	    		{
		    		fileEntry.GetPath (&filePath);
					filePath.Append (fileName);
					fileEntry.SetTo (filePath.Path());
					fileEntry.GetRef (&entryRef);
					SavePreset (&entryRef);
				}
			}
			
			break;
		}
		
		/* Select/Deselect all the checkboxes */
		case M_SELECT_ALL:
		{
			if (CheckIfPluginsExist ("No plugins to work with" B_UTF8_ELLIPSIS) == false)
				return;
			fElementListView->FullListDoForEach(SelectItems, fElementListView);
			break;
		}
		case M_DESELECT_ALL:
		{
			if (CheckIfPluginsExist ("No plugins to work with" B_UTF8_ELLIPSIS) == false)
				return;
			fElementListView->FullListDoForEach(DeselectItems, fElementListView);
			break;
		}
		/* Smart select -- Select 'cleanable' options | works only when overview has finished */
		case M_SMART_SELECT:
		{
			if (CheckIfPluginsExist ("No plugins to work with" B_UTF8_ELLIPSIS) == false)
				return;
			fElementListView->FullListDoForEach(SelectSmartItems, fElementListView);
			break;
		}
	}
	
	BWindow::MessageReceived (message);
}



/*============================================================================================================*/

status_t MainWindow::SavePreset (entry_ref *saveRef)
{
	PRINT (("MainWindow::SavePreset (entry_ref*)\n"));

	/* Save the preset into a file */
	if (saveRef != NULL)
	{
		BMessage presetMsg;
		FillPreset (&presetMsg);
		BFile file (saveRef, B_WRITE_ONLY | B_CREATE_FILE);


		/* Add a verification code so that when we load we know its a
			genuine preset file and not some other settings file with
			flattened BMessages */
		presetMsg.RemoveName ("presetVerifyCode");
		presetMsg.AddFloat ("presetVerifyCode", PresetVerifyCode);
		
		if (presetMsg.Flatten (&file) == B_OK)
		{
			/* Claim the file as ours :) */
			BNode node (saveRef);
			BNodeInfo nodeInfo (&node);
			nodeInfo.SetType (PresetSignature);
			
			
			/* Add this file to the preset menu */
			BMessage *msg = new BMessage (M_OPEN_PRESET);
			msg->AddRef ("FileRef", saveRef);
			msg->AddBool ("Is GUI Mode", true);
			msg->AddBool ("Already In List", true);
			

			/* Add the preset item only if it is already not in the list, check
				the ref for this */
			BMenuItem *item = new BMenuItem (saveRef->name, msg);
			item->SetMarked (true);
			bool alreadyExists = false;
			for (int i = 0; i < presetPopup->CountItems(); i++)
			{
				if (strcmp (item->Label(), presetPopup->ItemAt(i)->Label()) == 0)
				{
					entry_ref ref1;
					entry_ref ref2;
					item->Message()->FindRef ("FileRef", &ref1);
					presetPopup->ItemAt(i)->Message()->FindRef ("FileRef", &ref2);
					if (ref1 == ref2)
						alreadyExists = true;
				}
			}
			
			if (alreadyExists == false)
			{
				/* Add a separator after "Default" preset if needed */
				if (presetPopup->CountItems() == 1)
					presetPopup->AddSeparatorItem();
					
				presetPopup->AddItem (item);
			}
			else
			{
				delete item;
			}
			
			return B_OK;
		}
	}


	/* An error occurred while saving */
	beep();
	BAlert *errorAlert = new BAlert ("Error", "Unable to save file", "Damn!",
					NULL, NULL, B_WIDTH_AS_USUAL, B_EVEN_SPACING,
					B_WARNING_ALERT);
	errorAlert->Go ();

	return B_ERROR;
}

/*============================================================================================================*/

void MainWindow::FillTreeState (BMessage *prefsMessage) const
{
	PRINT (("MainWindow::FillTreeState (BMessage*)\n"));

	/* Fill "prefMessage" with the bool values of tree view for all hierarchial items */
	fElementListView->SaveTreeState(prefsMessage);
}

/*============================================================================================================*/

void MainWindow::FillPreset (BMessage *presetMessage) const
{
	PRINT (("MainWindow::FillPreset (BMessage*)\n"));

	/* Fill the given "presetMessage" with the current plugin settings 
		Right now the items are referenced using the index of checkBoxes BList. No names are used*/
	BString settingName;
	BMessage *message = new BMessage ();


	/* Loop and add the settings to "message" for all plugins */
	/*
	for (int32 i = 0; i < checkBoxes.CountItems(); i++)
	{
		settingName << i;
		message->AddInt8 (settingName.String(), (int8)((BCheckBox*)checkBoxes.ItemAtFast(i))->Value());
		settingName.SetTo ("");
	} */
	fElementListView->SavePreset(message);
	
	presetMessage->RemoveName ("plugin_settings");
	presetMessage->AddMessage ("plugin_settings", message);
	delete message;
}

/*============================================================================================================*/

void MainWindow::CreateSavePanel ()
{
	/* Create a new panel, if it doesn't already exist */
	entry_ref ref;
	BEntry entry;
	presetsFolder.GetEntry (&entry);
	entry.GetRef (&ref);

	if (savePanel == NULL)
		savePanel = new BFilePanel (B_SAVE_PANEL, messenger, &ref, B_FILE_NODE, false);
}

/*============================================================================================================*/

void MainWindow::DeleteSavePanel ()
{
	/* Delete the save panel, if it exists */
	if (savePanel != NULL)
		delete savePanel;

	savePanel = NULL;
}

/*============================================================================================================*/

int MainWindow::CountOptions () const
{
	/* Return the number of options selected by the user */
	int cnt (0);
	for (int32 i = 0; i < checkBoxesFields.CountItems(); i++)
		if (((CheckBoxWithStringField*)checkBoxesFields.ItemAtFast(i))->IsMarked())
			cnt++;

	
	return cnt;
}

/*============================================================================================================*/

void MainWindow::TellUserNoOptions ()
{
	PRINT (("MainWindow::TellUserNoOptions ()\n"));

	/* User hasn't selected any options, so... */
	static int nWarns (0);
	static BString warning[] =
	{
		"Select one or more options for the clean-up process",
		"I said... Select at least one checkbox before the clean-up process",
		"Look... For the last time!\n\nSELECT SOME OPTION!!",
		"That's it! I've had enough!"
	};

	BAlert *cantAlert;
	if (isModeGUI == true)
	{
		cantAlert = new BAlert ("Can't clean-Up", warning[nWarns].String(),
						"OK", NULL, NULL, B_WIDTH_AS_USUAL, B_EVEN_SPACING,
						B_STOP_ALERT);
		cantAlert->Go();
	}
	else
	{
		cerr << endl << warning[nWarns].String() << endl;
		cerr.flush();
		nWarns = 3;
	}

	if (nWarns >= 3)
		be_app_messenger.SendMessage (B_QUIT_REQUESTED);
	else
		nWarns ++;
}

/*============================================================================================================*/

void MainWindow::ListPresets ()
{
	PRINT (("MainWindow::ListPresets ()\n"));

	/* Scan presets folder for presets and fill the menufield */
	BEntry entry;
	entry_ref ref;
	
	presetsFolder.GetEntry (&entry);

	
	/* First add the "Default" entry */
	BEntry settingsEntry (prefs.PrefFilePath());
	entry_ref settings_ref;
	settingsEntry.GetRef (&settings_ref);

	
	BMessage *defMsg = new BMessage (M_OPEN_PRESET);
	defMsg->AddRef ("FileRef", &settings_ref);
	defMsg->AddBool ("Already In List", true);
	defMsg->AddBool ("Is GUI Mode", true);

	BMenuItem *defaultItem = new BMenuItem ("Default", defMsg);
	if (settingsEntry.Exists() == true)
	{
		presetPopup->AddItem (defaultItem);	
		defaultItem->SetMarked (true);
	}
	else
	{
		delete defaultItem;
	}

	/* Check if the folder exists and if it has any files */
	if (entry.Exists() == false || presetsFolder.CountEntries() == 0)
		return;

	presetPopup->AddSeparatorItem();
	
	
	/* Begin the scan... later need to make this whole function asynchronous */
	while (presetsFolder.GetNextEntry (&entry, true) != B_ENTRY_NOT_FOUND)
	{
		entry.GetRef (&ref);
		BMessage *msg = new BMessage (M_OPEN_PRESET);
		msg->AddRef ("FileRef", &ref);
		msg->AddBool ("Already In List", true);
		msg->AddBool ("Is GUI Mode", true);

		BMenuItem *item = new BMenuItem (ref.name, msg);
		presetPopup->AddItem (item);
	}
}

/*============================================================================================================*/

bool MainWindow::GetGUIModeFromMessage (BMessage *message) const
{
	bool mode;

	/* Make the default GUI mode as true using this check */
	if (message->FindBool ("GUI Mode", &mode) != B_OK)
		mode = true;
		
	return mode;
}

/*============================================================================================================*/

void MainWindow::AddLinearItem (PluginContainerItem *item, char *fileName)
{
	PRINT (("MainWindow::AddLinearItem (PluginContainerItem*, float, BView*, char*)\n"));

	/* This function adds a linear item (a simple checkbox - bstringview combination) and adds
		them to the corresponding BLists */
	BMessage *msg = new BMessage (M_CHECKBOX_CHANGED);
	msg->AddInt8 ("checkbox_index", (int8)checkBoxesFields.CountItems());
	// TODO pass message to CheckBoxWithStringField

	InfoStrView *linearInfo = new InfoStrView ("MainWindow:InfoView",
						liveMonitoring ? "[0 files, 0 bytes]" : "", B_WILL_DRAW);

	PluginItem *sItem = (PluginItem*)item->subItems.FirstItem ();
	BString looperName = "_";
	looperName << sItem->itemName.String();
	
	/* Set path for double-click opening of folder (TODO: pass sItem->isFolder for better path recognition) */
	linearInfo->SetPath (sItem->itemPath.String());

	BRow *row = new BRow();
	int32 i = 0;
	CheckBoxWithStringField* checkBoxField;
	row->SetField(checkBoxField = new CheckBoxWithStringField(((PluginItem*)item->subItems.ItemAt(0L))->itemName.String()), i++);
	row->SetField(new BIntegerField(0), i++);
	row->SetField(new BSizeField(0), i++);

	fElementListView->AddRow(row);

	/* Now initialize and run the FileLooper objects for each linear item */
	PRINT ((" >> spawing_looper: %s\t\tport_capacity: %ld\n", looperName.String(), looperPortCapacity));
	FileLooper *looper = new FileLooper (eraserLooper, sItem->itemPath.String(), looperName.String(),
								B_NORMAL_PRIORITY, true, sItem->isFolder, sItem->recurse && allowRecurse,
								debugMode, fileName, fileLoopers.CountItems(), row, looperPortCapacity);
	if (sItem->excludeFilePath != "")
		looper->ExcludeFileName ((char*)sItem->excludeFilePath.String());
	else if (sItem->includeMimeType != "")
		looper->IncludeMimeType ((char*)sItem->includeMimeType.String());

	checkBoxesFields.AddItem ((void*)checkBoxField);
	fileLoopers.AddItem ((void*)looper);
}

/*============================================================================================================*/

void MainWindow::AddHierarchialItem (PluginContainerItem *item, char *fileName)
{
	PRINT (("MainWindow::AddHierarchialItem (PluginContainerItem*, float, BView*, int, char*)\n"));
	
	BRow *row = new BRow();

	CheckBoxWithStringField* checkBoxString;
	row->SetField(checkBoxString = new CheckBoxWithStringField(item->name.String(), false), 0);
	//row->SetField(new BIntegerField(0), i++); // TOTAL
	//row->SetField(new BSizeField(0), i++);		//TOTAL
	fElementListView->AddRow(row);

	AddSubItems (item, row, fileName);
	// UpdateIfNeeded();
}

/*============================================================================================================*/

void MainWindow::AddSubItems (PluginContainerItem *item, BRow *parentRow, char *fileName)
{
	PRINT (("MainWindow::AddSubItems (PluginContainerItem*, BView*, char*)\n"));

	InfoStrView *itemInfo (NULL);


	/* Loop and add subitems of "item" We dont care to lock views because this is meant to be
		called from a function called from MessageReceived */
	int32 count = item->subItems.CountItems();
	for (int32 i = 0; i < count; i++)
	{
		PluginItem *pluginSubItem;
		pluginSubItem = (PluginItem*)item->subItems.ItemAtFast (i);
		
		BMessage *msg = new BMessage (M_CHECKBOX_CHANGED);
		msg->AddInt8 ("checkbox_index", (int8)checkBoxesFields.CountItems());
		// TODO pass message to CheckBoxWithStringField
		itemInfo = new InfoStrView (pluginSubItem->itemName.String(), liveMonitoring ? "[0 files, 0 bytes]" : "",
							B_WILL_DRAW);

		PluginItem *sItem = (PluginItem*)item->subItems.ItemAt (i);
		BString looperName = "_";
		looperName << sItem->itemName.String();
		itemInfo->SetPath (sItem->itemPath.String());
		
		BRow *row = new BRow();
		int32 index = 0;
		CheckBoxWithStringField* checkBoxField;
		row->SetField(checkBoxField = new CheckBoxWithStringField(pluginSubItem->itemName.String()), index++);
		row->SetField(new BIntegerField(0), index++); // TOTAL
		row->SetField(new BSizeField(0), index++);		//TOTAL

		fElementListView->AddRow(row, parentRow);

		/* Spawn our looper for the sub-item now */
		PRINT ((" >> spawning_looper: %s\t\tport_capacity: %ld\n", looperName.String(), looperPortCapacity));
		FileLooper *looper = new FileLooper (eraserLooper, sItem->itemPath.String(), looperName.String(),
									B_NORMAL_PRIORITY, true, sItem->isFolder, sItem->recurse && allowRecurse,
									debugMode, fileName, fileLoopers.CountItems(), row, looperPortCapacity);
		
		if (sItem->excludeFilePath != "")
			looper->ExcludeFileName ((char*)sItem->excludeFilePath.String());
		else if (sItem->includeMimeType != "")
			looper->IncludeMimeType ((char*)sItem->includeMimeType.String());
		
		checkBoxesFields.AddItem ((void*)checkBoxField);
		fileLoopers.AddItem ((void*)looper);
	}
}

/*============================================================================================================*/

void MainWindow::ParseAndSetupUI ()
{
	PRINT (("MainWindow::ParseAndSetupUI ()\n"));

	/* This message should be called by PostMessage() ie from MessageReceived so that the
		Window's looper is locked */
	ParsePlugins (pluginsFolder);
	

	/* Check if our threads have spawned properly */
	if (fileLoopers.CountItems() != checkBoxesFields.CountItems())
	{
		BAlert *fatal = new BAlert ("Error", "FilWip could not spawn all the required threads. This"
				" is a fatal error and would be not be wise to continue...", "Quit", NULL, NULL,
				B_WIDTH_AS_USUAL, B_EVEN_SPACING, B_STOP_ALERT);
		fatal->Go();

		be_app->PostMessage (B_QUIT_REQUESTED);
		return;
	}

	/* Check if we must restore the tree-view structure and do accordingly */
	bool loadTree = true;
	if (prefs.FindBool ("rm_tree", &loadTree) == B_OK)
		if (loadTree == true)
			LoadTreeState ((BMessage*)&prefs);

	BMessage columnsMessage;
	if (prefs.FindMessage ("columns_states", &columnsMessage) == B_OK)
		fElementListView->LoadState(&columnsMessage);

	/* Now reset this flag so that FrameResized from here on can call RecalcScrollBar
		since we are done accessing the scrollbar and now recalculate it */
	isProcessingPlugins = false;

	/* Restore clean-up options? */
	bool loadChkBoxes = true;
	if (prefs.FindBool ("rm_items", &loadChkBoxes) == B_OK)
		if (loadChkBoxes == true)
			LoadPreset ((BMessage*)&prefs);
	
	
	/* Fill overview stringviews? */
	if (liveMonitoring == true)
	{
		int32 looperCount = fileLoopers.CountItems();
		for (int32 i = 0; i < looperCount; i++)
			((FileLooper*)fileLoopers.ItemAtFast(i))->PostMessage (M_BEGIN_OVERVIEW);
	}
	else
	{
		/* Don't show [0 files, 0 bytes] if live monitor is switched off, instead hide infostrviews */
		int32 infoStrCount = infoViews.CountItems();
		for (int32 i = 0; i < infoStrCount; i++)
			((InfoStrView*)infoViews.ItemAtFast(i))->Hide();
	}
}

/*============================================================================================================*/

BBitmap* MainWindow::ResourceBitmap (const char *resourceName) const
{
	PRINT (("MainWindow::ResourceBitmap (const char*)\n"));

	/* Allocate bitmaps from resource, freed using delete elsewhere */
	char *buf (NULL);
	size_t bmpSize;
	BMessage msg;

	buf = (char*)be_app->AppResources()->LoadResource ('BBMP', resourceName, &bmpSize);
	if (!buf)
	{
		char errString[60];
		sprintf (errString, "error loading application resource: NAME=\"%s\" TYPE='BBMP'", resourceName);
		debugger (errString);
	}

	msg.Unflatten (buf);
	return new BBitmap (&msg);
}


BBitmap *MainWindow::ResVectorToBitmap(const char *resName)
{
	BResources res;
	size_t size;
	app_info appInfo;

	be_app->GetAppInfo(&appInfo);
	BFile appFile(&appInfo.ref, B_READ_ONLY);
	res.SetTo(&appFile);
	BBitmap *aBmp = NULL;
	const uint8* iconData = (const uint8*) res.LoadResource('VICN', resName, &size);

	if (size > 0 ) {
		aBmp = new BBitmap (BRect(0,0,20,20), 0, B_RGBA32);
		status_t result = BIconUtils::GetVectorIcon(iconData, size, aBmp);
		if (result != B_OK) {
			delete aBmp;
			aBmp = NULL;
		}
	}
	return aBmp;
}

/*============================================================================================================*/

bool MainWindow::ConfirmCleanUp () const
{
	PRINT (("MainWindow::ConfirmCleanUp ()\n"));
	
	/* This functions pops an alert and returns true if the user clicks OK else returns false */
	int32 buttonIndex;
	BAlert *confirm;
	BTextView *vw;
	
	/* Setup the confirm pop-up box */
	confirm = new BAlert ("Confirmation", "Warning\n\nThis clean-up process is irreversible!"
							" Once the data has been removed it cannot be recovered.\n\n"
							"Do you wish to begin the clean-up process?\n", "Cancel","Clean-up",
							NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT);
	confirm->SetShortcut (0, B_ESCAPE);
	
	/* Add some colours and font effects to the heading of the pop-up box */
	vw = confirm->TextView ();
	vw->SetStylable (true);
	
	BFont font (be_plain_font);
	font.SetFace (B_BOLD_FACE);
	rgb_color alertColor = {198, 0, 0, 255};
	vw->SetFontAndColor (0, 7, &font, B_FONT_ALL, &alertColor);

	/* Popup a scary warning :) */
	buttonIndex = confirm->Go ();
	
	/* Return true if user is brave enough to proceed ;) else false */
	return buttonIndex == 1 ? true : false;
}

/*============================================================================================================*/

bool MainWindow::CheckIfPluginsExist (const char* errorStr) const
{
	PRINT (("MainWindow::CheckIfPluginsExist (const char*)\n"));

	if (fElementListView->CountRows() == 0)
	{
		BAlert *fatal = new BAlert ("Error", errorStr, "Quit", NULL, NULL,
							B_WIDTH_AS_USUAL, B_EVEN_SPACING, B_STOP_ALERT);
		fatal->Go();
		be_app_messenger.SendMessage (B_QUIT_REQUESTED);
		return false;
	}

	return true;
}
			
/*============================================================================================================*/

void MainWindow::ResetLoopers (bool monitorNodes)
{
	PRINT (("MainWindow::ResetLoopers (bool)\n"));

	if (liveMonitoring == false)
		return;
	
	/* Reset loopers so they stop/start monitoring nodes */
	for (int32 i = 0; i < fileLoopers.CountItems(); i++)
	{
		FileLooper *fileLooper = ((FileLooper*)fileLoopers.ItemAtFast(i));
		if (fileLooper->Lock())
		{
			fileLooper->ignoreChanges = monitorNodes;
			fileLooper->Unlock();

			if (monitorNodes == false)
				fileLooper->PostMessage (M_BEGIN_OVERVIEW);
		}
	}
}

/*============================================================================================================*/
