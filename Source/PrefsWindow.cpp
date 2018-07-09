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

#include <Debug.h>
#include <View.h>
#include <Application.h>
#include <Box.h>
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
	: BWindow (BRect (0, 0, 440, 270-60), "Preferences", B_TITLED_WINDOW, B_NOT_ZOOMABLE | B_NOT_RESIZABLE | B_AUTO_UPDATE_SIZE_LIMITS,
				B_CURRENT_WORKSPACE),
		checkBoxWidth (30.0f)
{
	PRINT (("PrefsWindow::PrefsWindow ()\n"));

	SetFeel (B_MODAL_APP_WINDOW_FEEL);

	/* Construct basic outline controls */
	backView = new BView ("Preferences:backView", B_WILL_DRAW);

	optionsListView = new BListView ("Preferences:optionsView",
		B_SINGLE_SELECTION_LIST);
	optionsListView->SetSelectionMessage (new BMessage (M_PREF_OPTION_CHANGED));

	scrollView = new BScrollView ("Preferences:scrollView", optionsListView,
		B_FRAME_EVENTS | B_WILL_DRAW, false, true);

	/* Fill-in the corresponding description strings */
	const char *descStrings[] = 
	{
		"Details to be shown after each clean-up",				// Report
		"Automatic checking options",							// Items
		"Loopers maintain list files before clean-up",			// Loopers
		"Options to restore on startup",						// Remember
		"Plugin options (needs app restart to take effect)",	// Plugins
		"Some miscellaneous options"							// Miscellaneous
	};

	saveBtn = new BButton ("Preferences:saveBtn", "Save", new BMessage (M_SAVE_PREFS));
	
	cancelBtn = new BButton ("Preferences:cancelBtn", "Cancel", new BMessage (M_CLOSE_PREFS));
	
	/* Below is generic code to render panels (makes addition of panels easy) */
	RenderFunc reportFuncPtr = &PrefsWindow::MakeViewReport;
	RenderFunc itemsFuncPtr = &PrefsWindow::MakeViewItems;
	RenderFunc loopersFuncPtr = &PrefsWindow::MakeViewLoopers;
	RenderFunc rememberFuncPtr = &PrefsWindow::MakeViewRemember;
	RenderFunc pluginsFuncPtr = &PrefsWindow::MakeViewPlugins;
	RenderFunc miscFuncPtr = &PrefsWindow::MakeViewMiscellaneous;

	funcList.AddItem ((void*)&reportFuncPtr);
	funcList.AddItem ((void*)&itemsFuncPtr);
	funcList.AddItem ((void*)&loopersFuncPtr);
	funcList.AddItem ((void*)&rememberFuncPtr);
	funcList.AddItem ((void*)&pluginsFuncPtr);
	funcList.AddItem ((void*)&miscFuncPtr);

	/* We need to allocate save pointers on the heap as we will be needing them OUTSIDE this function.
		NOTE: I haven't found a way to "delete" this new SLFunc pointers. Are they deletable as they point
			to functions in memory? So at the moment, they aren't deleted */
	SLFunc *reportSavePtr = new SLFunc;
	SLFunc *itemsSavePtr = new SLFunc;
	SLFunc *loopersSavePtr = new SLFunc;
	SLFunc *rememberSavePtr = new SLFunc;
	SLFunc *pluginsSavePtr = new SLFunc;
	SLFunc *miscSavePtr = new SLFunc;
	
	*reportSavePtr = &PrefsWindow::SaveViewReport;
	*itemsSavePtr = &PrefsWindow::SaveViewItems;
	*loopersSavePtr = &PrefsWindow::SaveViewLoopers;
	*rememberSavePtr = &PrefsWindow::SaveViewRemember;
	*pluginsSavePtr = &PrefsWindow::SaveViewPlugins;
	*miscSavePtr = &PrefsWindow::SaveViewMiscellaneous;
	
	saveList.AddItem ((void*)reportSavePtr);
	saveList.AddItem ((void*)itemsSavePtr);
	saveList.AddItem ((void*)loopersSavePtr);
	saveList.AddItem ((void*)rememberSavePtr);
	saveList.AddItem ((void*)pluginsSavePtr);
	saveList.AddItem ((void*)miscSavePtr);
	
	/* Build the load function list. Allocate on heap. The loadlist will become 'invalid' outside this
		function as the below pointers will disappear after this function (Same applies to funcList) */
	SLFunc reportLoadPtr = &PrefsWindow::LoadViewReport;
	SLFunc itemsLoadPtr = &PrefsWindow::LoadViewItems;
	SLFunc loopersLoadPtr = &PrefsWindow::LoadViewLoopers;
	SLFunc rememberLoadPtr = &PrefsWindow::LoadViewRemember;
	SLFunc pluginsLoadPtr = &PrefsWindow::LoadViewPlugins;
	SLFunc miscLoadPtr = &PrefsWindow::LoadViewMiscellaneous;
	
	loadList.AddItem ((void*)&reportLoadPtr);
	loadList.AddItem ((void*)&itemsLoadPtr);
	loadList.AddItem ((void*)&loopersLoadPtr);
	loadList.AddItem ((void*)&rememberLoadPtr);
	loadList.AddItem ((void*)&pluginsLoadPtr);
	loadList.AddItem ((void*)&miscLoadPtr);
	
	BBox *fLabelBox = new BBox("SettingsContainerBox2");
	fSettingsContainerBox = new BView("SettingsContainerBox",0);
	fSettingsContainerBox->SetLayout(new BCardLayout());
	fLabelBox->AddChild(fSettingsContainerBox);

	int32 panelCount = funcList.CountItems();
	for (int32 i = 0; i < panelCount; i++)
	{
		RenderFunc func = *(reinterpret_cast<RenderFunc*>(funcList.ItemAtFast(i)));
		SLFunc loadFunc = *(reinterpret_cast<SLFunc*>(loadList.ItemAtFast(i)));

		PrefsView *vw = ConstructPrefsView (descStrings[i], func, loadFunc);
		viewList.AddItem ((void*)vw);
		((BCardLayout*) fSettingsContainerBox->GetLayout())->AddView(vw);
	}

	BLayoutBuilder::Group<>(this)
		.AddGroup(B_HORIZONTAL, B_USE_DEFAULT_SPACING)
			.Add(scrollView)
			.AddGroup(B_VERTICAL, B_USE_DEFAULT_SPACING)
				.Add(fLabelBox)
				.AddGroup(B_HORIZONTAL, B_USE_DEFAULT_SPACING)
					.AddGlue()
					.Add(cancelBtn)
					.Add(saveBtn)
				.End()
			.End()
		.SetInsets(B_USE_WINDOW_SPACING)
	.End();

	/* Fill-in the preferences panel */	

	AddOptionsToListView(optionsListView, new BStringItem ("Report"));
	AddOptionsToListView(optionsListView, new BStringItem ("Items"));
	AddOptionsToListView(optionsListView, new BStringItem ("Loopers"));
	AddOptionsToListView(optionsListView, new BStringItem ("Remember"));
	AddOptionsToListView(optionsListView, new BStringItem ("Plugins"));
	AddOptionsToListView(optionsListView, new BStringItem ("Miscellaneous"));

	optionsListView->Select (0L, false);
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

}

/*============================================================================================================*/

void PrefsWindow::Show ()
{
	optionsListView->MakeFocus(true);
	BWindow::Show();
}

/*============================================================================================================*/

void PrefsWindow::Quit ()
{
	be_app_messenger.SendMessage (M_CLOSE_PREFS);
	BWindow::Quit();
}

/*============================================================================================================*/

void PrefsWindow::MessageReceived (BMessage *message)
{
	switch (message->what)
	{
		case M_PREF_OPTION_CHANGED:
		{
			int32 selectedItem = optionsListView->CurrentSelection();
			if (selectedItem >= 0L && selectedItem < funcList.CountItems())
			{
				((BCardLayout*) fSettingsContainerBox->GetLayout())->SetVisibleItem(selectedItem);
			} 
//			else
//			{
				/* Find previously selected item (stored in currentView) and make that the selected view
					This happens when user tries to deselect an item in the list */
/*				int32 previousSelection (0L), listCount (viewList.CountItems());
				for (int32 i = 0L; i < listCount; i++)
					if (currentView == reinterpret_cast<PrefsView*>(viewList.ItemAtFast(i)))
					{
						previousSelection = i;
						break;
					}
				optionsListView->Select (previousSelection);
			} */
			
			break;
		}
		
		case M_SAVE_PREFS:
		{
			/* Basically call the save hook of PrefsView */
			for (int32 i = 0L; i < funcList.CountItems(); i++)
			{
				/*	SLFunc saveFunc = *(reinterpret_cast<SLFunc*>(saveList.ItemAtFast(i)));
					(this->*saveFunc)();				Neat line below ;-P
				*/
				(this->**(reinterpret_cast<SLFunc*>(saveList.ItemAtFast(i))))();
			}
			
			prefs.WriteSettings();
			prefs.SetPoint ("prefwnd_point", Frame().LeftTop());
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

PrefsView* PrefsWindow::ConstructPrefsView (const char *desc, RenderFunc func, SLFunc slfunc)
{
	PrefsView *vw = new PrefsView (desc);

	/* Call render function */	
	(this->*func)(vw);

	/* Call load function */	
	(this->*slfunc)();
	vw->GroupLayout()->AddItem(BSpaceLayoutItem::CreateGlue());
	return vw;
}

/*============================================================================================================*/

void PrefsWindow::MakeViewReport (BView *vw)
{
	PRINT (("PrefsWindow::MakeViewReport (BView*)\n"));
	
	rv_spaceFreeChk = new BCheckBox ("Preferences:rv_spaceFreeChk", rv_spaceFreeStr, NULL);

	rv_nFilesDeletedChk = new BCheckBox ("Preferences:rv_nFilesDeletedChk",	rv_nFilesDeletedStr, NULL);

	rv_timeTakenChk = new BCheckBox ("Preferences:rv_timeTakenChk",	rv_timeTakenStr, NULL);

	vw->AddChild (rv_spaceFreeChk);
	vw->AddChild (rv_nFilesDeletedChk);
	vw->AddChild (rv_timeTakenChk);
}

/*============================================================================================================*/

void PrefsWindow::SaveViewReport ()
{
	PRINT (("PrefsWindow::SaveViewReport ()\n"));

	prefs.SetBool ("rv_spaceFree", IsChecked (rv_spaceFreeChk));
	prefs.SetBool ("rv_nFilesDeleted", IsChecked (rv_nFilesDeletedChk));
	prefs.SetBool ("rv_timeTaken", IsChecked (rv_timeTakenChk));
}

/*============================================================================================================*/

void PrefsWindow::LoadViewReport ()
{
	PRINT (("PrefsWindow::LoadViewReport ()\n"));

	rv_spaceFreeChk->SetValue (CheckBoxValue (prefs.FindBoolDef ("rv_spaceFree", true)));
	rv_nFilesDeletedChk->SetValue (CheckBoxValue (prefs.FindBoolDef ("rv_nFilesDeleted", true)));
	rv_timeTakenChk->SetValue (CheckBoxValue (prefs.FindBoolDef ("rv_timeTaken", true)));
}

/*============================================================================================================*/

void PrefsWindow::MakeViewItems (BView *vw)
{
	PRINT (("PrefsWindow::MakeViewItems (BView*)\n"));

	it_autoCheckStartChk = new BCheckBox ("Preferences:it_autoCheckStartChk", it_autoCheckStartStr, NULL);

	it_autoCheckLiveChk = new BCheckBox ("Preferences:it_autoCheckLiveChk",
							it_autoCheckLiveStr, NULL);

	it_unCheckAfterDelChk = new BCheckBox ("Preferences:it_unCheckAfterDelChk",
							it_unCheckAfterDelStr, NULL);

	vw->AddChild (it_autoCheckStartChk);
	vw->AddChild (it_autoCheckLiveChk);
	vw->AddChild (it_unCheckAfterDelChk);
}

/*============================================================================================================*/

void PrefsWindow::SaveViewItems ()
{
	PRINT (("PrefsWindow::SaveViewItems ()\n"));
	
	prefs.SetBool ("it_autoCheckStart", IsChecked (it_autoCheckStartChk));
	prefs.SetBool ("it_autoCheckLive", IsChecked (it_autoCheckLiveChk));
	prefs.SetBool ("it_unCheckAfterDel", IsChecked (it_unCheckAfterDelChk));
}

/*============================================================================================================*/

void PrefsWindow::LoadViewItems ()
{
	PRINT (("PrefsWindow::LoadViewItems ()\n"));

	it_autoCheckStartChk->SetValue (CheckBoxValue (prefs.FindBoolDef ("it_autoCheckStart", false)));
	it_autoCheckLiveChk->SetValue (CheckBoxValue (prefs.FindBoolDef ("it_autoCheckLive", false)));
	it_unCheckAfterDelChk->SetValue (CheckBoxValue (prefs.FindBoolDef ("it_unCheckAfterDel", false)));
}

/*============================================================================================================*/

void PrefsWindow::MakeViewLoopers (BView *vw)
{
	PRINT (("PrefsWindow::MakeViewLoopers (BView*)\n"));
	
	lo_syncChk = new BCheckBox ("Preferences:lo_syncChk", lo_syncStr, NULL);

	lo_safeChk = new BCheckBox ("Preferences:lo_safeChk", lo_safeStr, NULL);
	lo_safeChk->SetValue (B_CONTROL_ON); lo_safeChk->SetEnabled (false); // no need this option
	
	lo_monitorChk = new BCheckBox ("Preferences:lo_monitorChk", lo_monitorStr, NULL);

	/* Construct eraser looper priority menu */
	lo_priorityPopup = new BPopUpMenu ("");
	lo_priorityField = new BMenuField (BRect (SmallMargin, lo_monitorChk->Frame().bottom + SmallMargin, 
						Bounds().right - SmallMargin, 0), "Preferences:lo_priorityField",
						lo_priorityFieldStr, (BMenu*)lo_priorityPopup);
	lo_priorityField->SetDivider (backView->StringWidth (lo_priorityField->Label()) + 
						backView->StringWidth ("W"));
	
	lo_priorityList.AddItem ((void*)B_LOW_PRIORITY);
	lo_priorityList.AddItem ((void*)B_NORMAL_PRIORITY);
	lo_priorityList.AddItem ((void*)B_DISPLAY_PRIORITY);
	lo_priorityList.AddItem ((void*)B_URGENT_DISPLAY_PRIORITY);
	lo_priorityList.AddItem ((void*)LOW_CUSTOM_PRIORITY);
	lo_priorityList.AddItem ((void*)NORMAL_CUSTOM_PRIORITY);
	lo_priorityList.AddItem ((void*)HIGH_CUSTOM_PRIORITY);
	
	lo_item1 = new BMenuItem (lo_item1Str, NULL);
	lo_item2 = new BMenuItem (lo_item2Str, NULL);
	lo_item3 = new BMenuItem (lo_item3Str, NULL);
	lo_item4 = new BMenuItem (lo_item4Str, NULL);
	lo_item5 = new BMenuItem (lo_item5Str, NULL);
	lo_item6 = new BMenuItem (lo_item6Str, NULL);
	lo_item7 = new BMenuItem (lo_item7Str, NULL);

	lo_priorityPopup->AddItem (lo_item1);
	lo_priorityPopup->AddItem (lo_item2); lo_item2->SetMarked(true);
	lo_priorityPopup->AddItem (lo_item3);
	lo_priorityPopup->AddItem (lo_item4);
	lo_priorityPopup->AddItem (lo_item5);
	lo_priorityPopup->AddItem (lo_item6);
	lo_priorityPopup->AddItem (lo_item7);

	vw->AddChild (lo_priorityField);
	
	/* Construct the looper capacity menus */
	lo_capacityPopup = new BPopUpMenu ("");
	lo_capacityField = new BMenuField ("Preferences:lo_capacityField",
						lo_capacityFieldStr, (BMenu*)lo_capacityPopup);
	lo_capacityField->SetDivider (backView->StringWidth (lo_capacityField->Label()) +
						backView->StringWidth ("W"));
	
	lo_capacityList.AddItem ((void*)CAPACITY_100);
	lo_capacityList.AddItem ((void*)CAPACITY_500);
	lo_capacityList.AddItem ((void*)CAPACITY_1000);
	lo_capacityList.AddItem ((void*)CAPACITY_1500);
	lo_capacityList.AddItem ((void*)CAPACITY_2000);
	
	lo_cap1 = new BMenuItem ("100", NULL);
	lo_cap2 = new BMenuItem ("500", NULL);
	lo_cap3 = new BMenuItem ("1000", NULL); lo_cap3->SetMarked (true);
	lo_cap4 = new BMenuItem ("1500", NULL);
	lo_cap5 = new BMenuItem ("2000", NULL);
	
	lo_capacityPopup->AddItem (lo_cap1);
	lo_capacityPopup->AddItem (lo_cap2);
	lo_capacityPopup->AddItem (lo_cap3);
	lo_capacityPopup->AddItem (lo_cap4);
	lo_capacityPopup->AddItem (lo_cap5);
		
	vw->AddChild (lo_syncChk);
	vw->AddChild (lo_safeChk);
	vw->AddChild (lo_monitorChk);
	vw->AddChild (lo_capacityField);
}

/*============================================================================================================*/

void PrefsWindow::SaveViewLoopers ()
{
	PRINT (("PrefsWindow::SaveViewLoopers ()\n"));

	prefs.SetBool ("lo_sync", IsChecked (lo_syncChk));
	prefs.SetBool ("lo_safe", IsChecked (lo_safeChk));
	prefs.SetBool ("lo_monitor", IsChecked (lo_monitorChk));
	
	prefs.SetInt8 ("lo_priority",
		(int8)(addr_t)lo_priorityList.ItemAt(lo_priorityPopup->IndexOf(lo_priorityPopup->FindMarked())));
		
	prefs.SetInt16 ("lo_capacity",
		(int16)(addr_t)lo_capacityList.ItemAt(lo_capacityPopup->IndexOf(lo_capacityPopup->FindMarked())));
}

/*============================================================================================================*/

void PrefsWindow::LoadViewLoopers ()
{
	PRINT (("PrefsWindow::LoadViewLoopers ()\n"));

	lo_syncChk->SetValue (CheckBoxValue (prefs.FindBoolDef ("lo_sync", false)));
	lo_safeChk->SetValue (CheckBoxValue (prefs.FindBoolDef ("lo_safe", true)));
	lo_monitorChk->SetValue (CheckBoxValue (prefs.FindBoolDef ("lo_monitor", true)));

	/* Load the priority menu from prefs */
	int8 p;
	if (prefs.FindInt8 ("lo_priority", &p) == B_OK)
	{
		for (int32 x = 0; x < lo_priorityList.CountItems(); x++)
			if ((int8)(addr_t)lo_priorityList.ItemAt(x) == p)
			{
				(lo_priorityPopup->ItemAt(x))->SetMarked (true);
				break;
			}
	}
	else
	{
		lo_priorityPopup->ItemAt(1L)->SetMarked (true);
	}

	/* Load the port capacity menu from prefs */
	int16 c;
	if (prefs.FindInt16 ("lo_capacity", &c) == B_OK)
	{
		for (int32 x = 0; x < lo_capacityList.CountItems(); x++)
			if ((int16)(addr_t)lo_capacityList.ItemAtFast(x) == c)
			{
				(lo_capacityPopup->ItemAt(x))->SetMarked (true);
				break;
			}
	}
	else
	{
		lo_capacityPopup->ItemAt(2L)->SetMarked (true);
	}
}

/*============================================================================================================*/

void PrefsWindow::MakeViewRemember (BView *vw)
{
	PRINT (("PrefsWindow::MakeViewRemember (BView*)\n"));
	
	rm_treeChk = new BCheckBox ("Preferences:rm_treeChk", rm_treeStr, NULL);

	rm_winPosChk = new BCheckBox ("Preferences:rm_winPosChk", rm_winPosStr, NULL);

	rm_itemsChk = new BCheckBox ("Preferences:rm_itemsChk", rm_itemsStr, NULL);

	vw->AddChild (rm_treeChk);
	vw->AddChild (rm_winPosChk);
	vw->AddChild (rm_itemsChk);
}

/*============================================================================================================*/

void PrefsWindow::SaveViewRemember ()
{
	PRINT (("PrefsWindow::SaveViewRemember ()\n"));
	
	prefs.SetBool ("rm_tree", IsChecked (rm_treeChk));
	prefs.SetBool ("rm_winPos", IsChecked (rm_winPosChk));
	prefs.SetBool ("rm_items", IsChecked (rm_itemsChk));
}

/*============================================================================================================*/

void PrefsWindow::LoadViewRemember ()
{
	PRINT (("PrefsWindow::LoadViewRemember ()\n"));

	rm_treeChk->SetValue (CheckBoxValue (prefs.FindBoolDef ("rm_tree", true)));
	rm_winPosChk->SetValue (CheckBoxValue (prefs.FindBoolDef ("rm_winPos", true)));
	rm_itemsChk->SetValue (CheckBoxValue (prefs.FindBoolDef ("rm_items", true)));
}

/*============================================================================================================*/

void PrefsWindow::MakeViewPlugins (BView *vw)
{
	PRINT (("PrefsWindow::MakeViewPlugins (BView*)\n"));

	pv_asyncLoadChk = new BCheckBox ("Preferences:pv_asyncLoadChk", pv_asyncLoadStr, NULL);

	pv_debugChk = new BCheckBox ("Preferences:pv_debugChk", pv_debugStr, NULL);

	pv_checkInstallChk = new BCheckBox ("Preferences:pv_checkInstallChk",
							pv_checkInstallStr, NULL);

	pv_recurseChk = new BCheckBox ("Preferences:pv_recuseChk",
							pv_recurseStr, NULL);

	vw->AddChild (pv_asyncLoadChk);
	vw->AddChild (pv_debugChk);
	vw->AddChild (pv_checkInstallChk);
	vw->AddChild (pv_recurseChk);
}

/*============================================================================================================*/

void PrefsWindow::SaveViewPlugins ()
{
	PRINT (("PrefsWindow::SaveViewPlugins ()\n"));

	prefs.SetBool ("pv_asyncLoad", IsChecked (pv_asyncLoadChk));
	prefs.SetBool ("pv_debug", IsChecked (pv_debugChk));
	prefs.SetBool ("pv_checkInstall", IsChecked (pv_checkInstallChk));
	prefs.SetBool ("pv_recurse", IsChecked (pv_recurseChk));
}

/*============================================================================================================*/

void PrefsWindow::LoadViewPlugins ()
{
	PRINT (("PrefsWindow::LoadViewPlugins ()\n"));

	pv_asyncLoadChk->SetValue (CheckBoxValue (prefs.FindBoolDef ("pv_asyncLoad", false)));
	pv_debugChk->SetValue (CheckBoxValue (prefs.FindBoolDef ("pv_debug", false)));
	pv_checkInstallChk->SetValue (CheckBoxValue (prefs.FindBoolDef ("pv_checkInstall", true)));
	pv_recurseChk->SetValue (CheckBoxValue (prefs.FindBoolDef ("pv_recurse", true)));
}

/*============================================================================================================*/

void PrefsWindow::MakeViewMiscellaneous (BView *vw)
{
	PRINT (("PrefsWindow::MakeViewMiscellaneous (BView*)\n"));
	
	ms_confirmChk = new BCheckBox ("Preferences:ms_confirm", ms_confirmStr, NULL);

	ms_registerChk = new BCheckBox ("Preferences:ms_register", ms_registerStr, NULL);

	ms_quitAppChk = new BCheckBox ("Preferences:ms_quitApp", ms_quitAppStr, NULL);

	vw->AddChild (ms_confirmChk);
	vw->AddChild (ms_registerChk);
	vw->AddChild (ms_quitAppChk);
}

/*============================================================================================================*/

void PrefsWindow::SaveViewMiscellaneous ()
{
	PRINT (("PrefsWindow::SaveViewMiscellaneous ()\n"));

	prefs.SetBool ("ms_confirm", IsChecked (ms_confirmChk));
	prefs.SetBool ("ms_register", IsChecked (ms_registerChk));
	prefs.SetBool ("ms_quitApp", IsChecked (ms_quitAppChk));
}

/*============================================================================================================*/

void PrefsWindow::LoadViewMiscellaneous ()
{
	PRINT (("PrefsWindow::LoadViewMiscellanous ()\n"));

	ms_confirmChk->SetValue (CheckBoxValue (prefs.FindBoolDef ("ms_confirm", true)));
	ms_registerChk->SetValue (CheckBoxValue (prefs.FindBoolDef ("ms_register", false)));
	ms_quitAppChk->SetValue (CheckBoxValue (prefs.FindBoolDef ("ms_quitApp", false)));
}


void PrefsWindow::AddOptionsToListView(BListView* listView, BStringItem* item)
{
	listView->AddItem(item);
	// constraint the listview width so that the longest item fits
	float width = 0;
	listView->GetPreferredSize(&width, NULL);
	width += B_V_SCROLL_BAR_WIDTH;
	listView->SetExplicitMinSize(BSize(width, 0));
	listView->SetExplicitMaxSize(BSize(width, B_SIZE_UNLIMITED));

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

float PrefsWindow::CheckBoxWidth (const char *str) const
{
	return backView->StringWidth (str) + 30.0f;
}

/*============================================================================================================*/
