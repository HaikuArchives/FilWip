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

#ifndef _ADVANCED_PREFS_WINDOW_H
#define _ADVANCED_PREFS_WINDOW_H

#include <Window.h>

#define M_PREF_OPTION_CHANGED	'pfcg'
#define M_SAVE_PREFS			'pfsv'

#define LOW_CUSTOM_PRIORITY		30
#define NORMAL_CUSTOM_PRIORITY	35
#define HIGH_CUSTOM_PRIORITY	40

#define CAPACITY_100			100
#define CAPACITY_500			500
#define CAPACITY_1000			1000
#define CAPACITY_1500			1500
#define CAPACITY_2000			2000

class BListView;
class BButton;
class BCheckBox;
class BList;
class BRect;
class BStringView;
class BPopUpMenu;
class BMenuField;
class BMenuItem;

class PrefsListItem;
class PrefsView;

class AdvancedPrefsWindow : public BWindow
{
	/* Function pointers:	RenderFunc -- Pointers to function that render panels
							SLFunc -- Save/Load pointers: to function that save/load panels */
	typedef void		(AdvancedPrefsWindow::*RenderFunc)(BView*);
	typedef void		(AdvancedPrefsWindow::*SLFunc)();

	public:
		AdvancedPrefsWindow ();
		
		virtual void		MessageReceived (BMessage *message);
		virtual void		Show ();
		virtual void		Quit ();
		
	protected:
		/* Protected hooks */
		PrefsView			*ConstructPrefsView (const char *desc, RenderFunc func, SLFunc slfunc);
		void				MakeViewReport (BView *vw);
		void				MakeViewItems (BView *vw);
		void				MakeViewLoopers (BView *vw);
		void				MakeViewRemember (BView *vw);
		void				MakeViewPlugins (BView *vw);
		void				MakeViewMiscellaneous (BView *vw);

		void				SaveViewReport();
		void				SaveViewItems ();
		void				SaveViewLoopers ();
		void				SaveViewRemember ();
		void				SaveViewPlugins ();
		void				SaveViewMiscellaneous ();

		void				LoadViewReport();
		void				LoadViewItems ();
		void				LoadViewLoopers ();
		void				LoadViewRemember ();
		void				LoadViewPlugins ();
		void				LoadViewMiscellaneous ();

		bool				IsChecked (BCheckBox *chkBox) const;
		int32				CheckBoxValue (bool value) const;
		void				AddOptionsToListView(BListView* listView, BStringItem* item);
		
		/* Protected members */
		BListView			*optionsListView;
		BScrollView			*scrollView;
		BButton				*saveBtn,
							*cancelBtn;
		
		BList				viewList,
							funcList,
							saveList,
							loadList;

		BCheckBox			*rv_spaceFreeChk,
							*rv_nFilesDeletedChk,
							*rv_timeTakenChk,
							
							*it_autoCheckStartChk,
							*it_autoCheckLiveChk,
							*it_unCheckAfterDelChk,
							
							*lo_syncChk,
							*lo_safeChk,
							*lo_monitorChk,
							
							*rm_treeChk,
							*rm_winPosChk,
							*rm_itemsChk,
							
							*pv_asyncLoadChk,
							*pv_debugChk,
							*pv_checkInstallChk,
							*pv_recurseChk,
							
							*ms_confirmChk,
							*ms_registerChk,
							*ms_quitAppChk;
		
		BPopUpMenu			*lo_priorityPopup,
							*lo_capacityPopup;
		BMenuField			*lo_priorityField,
							*lo_capacityField;
		BList				lo_priorityList,
							lo_capacityList;
		BMenuItem			*lo_item1,
							*lo_item2,
							*lo_item3,
							*lo_item4,
							*lo_item5,
							*lo_item6,
							*lo_item7,
							*lo_cap1,
							*lo_cap2,
							*lo_cap3,
							*lo_cap4,
							*lo_cap5;
		int32				fPreviousSelection;
		BView *				fSettingsContainerBox;
};

#endif /* _PREFS_WINDOW_H */
