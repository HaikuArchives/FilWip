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

#ifndef _MAIN_WINDOW_H
#define _MAIN_WINDOW_H

#include <Window.h>
#include <Bitmap.h>
#include <Directory.h>

class BBox;
class BMenuField;
class BPopUpMenu;
class BStatusBar;
class BTextView;

class BevelView;
class BubbleHelper;
class EraserLooper;
class ImageButton;
class ItemsView;
class PluginContainerItem;

extern class MainWindow *pWnd;

class MainWindow : public BWindow
{
	public:
		MainWindow ();
		~MainWindow ();
		
		/* Inherited Hooks */
		virtual void		Quit ();
		virtual void		MessageReceived (BMessage *message);
		virtual void		WindowActivated (bool state);
		virtual void		FrameResized (float newWidth, float newHeight);
		virtual void		Show ();
		
		/* Additional Hooks */
		void				CreateSavePanel ();
		void				DeleteSavePanel ();
		void				RecalcItems (int8 index, uint32 action);
		void				RecalcScrollBar ();
		void				RestoreWindowPosition ();
		void				SaveWindowPosition () const;
		void				TellUserNoOptions ();
		void				TellUserNoPlugins () const;
		bool				ConfirmCleanUp () const;
		bool				CheckIfPluginsExist (const char* errStr) const;
		void				ResetLoopers (bool monitorNodes);
		int					CountOptions () const;
		void				ListPresets ();
		void				FillTreeState (BMessage *message) const;
		void				LoadTreeState (BMessage *message);
		void				LoadPreset (BMessage *message);
		void				FillPreset (BMessage *message) const;
		float				AddSubItems (PluginContainerItem *item, BView *parent, char *fileName);
		bool				GetGUIModeFromMessage (BMessage *message) const;
		BBitmap				*ResourceBitmap (const char *resourceName) const;
		BString				GetByteSizeString (int64 size) const;
		status_t			SavePreset (entry_ref *saveRef);
		status_t			OpenPreset (entry_ref *openRef, bool guiMode, bool addToList);
		void				ParsePlugins (BDirectory pluginFolder);
		void				ParseAndSetupUI ();
		void				AddLinearItem (PluginContainerItem *item, float yPos, BView *vw, char *fileName);
		float				AddHierarchialItem (PluginContainerItem *item, float yPos, BView *vw, int index,
								char *fileName);
		
		EraserLooper		*eraserLooper;
		
		/* Critical to thread variable */
		BMessage			wndMessage;

		/* Clean-up related variables */
		int					nRemainingItems;
		volatile bool		isModeGUI,
							asyncLoadAllowed;
		double				startTime,
							endTime,
							totalTime;

		/* Imagebutton and image pointers */
		BList				toolButtons;
		BScrollBar			*vertScrollBar;
		BScrollView			*scrollView;
		
		ImageButton			*helpButton,
							*optionsButton,
							*saveButton,
							*aboutButton,
							*previewButton,
							*selectAllButton,
							*deselectAllButton,
							*smartSelectButton;

		BBitmap				*helpButtonBitmap,
							*optionsButtonBitmap,
							*saveButtonBitmap,
							*aboutButtonBitmap,
							*previewButtonBitmap,
							*selectAllButtonBitmap,
							*deselectAllButtonBitmap,
							*smartSelectButtonBitmap;
							
		/* Other control pointers */
		BStatusBar			*statusBar;
		BButton				*cleanUp;
		BBox				*boxView;
		BMenuField			*presetField;
		BPopUpMenu			*presetPopup;
		BubbleHelper		*toolTip;

		BList				checkBoxes,
							infoViews,
							superItems,
							subViews,
							treeViews,
							containerItems,
							hierarchialItems,
							fileLoopers;
		
		ItemsView			*itemsView;
		BevelView			*backView;
		BTextView			*descView;

		/* Interface headaches */
		float				strHeight,
							mLeft,
							mTop,
							mMargin,
							mVGap,
							checkRight,
							infoRight;
				
		/* Controller variable */
		volatile bool		isProcessingPlugins;
				
		/* Save and load related variables */
		BFilePanel			*savePanel;
		BDirectory			presetsFolder,
							pluginsFolder;
		BMessenger			*messenger;
		BDirectory			docsFolder;
		
		/* Some pref variables */
		bool				debugMode,
							liveMonitoring,
							allowRecurse;
		int16				looperPortCapacity;
};

#endif /* _MAIN_WINDOW_H */
