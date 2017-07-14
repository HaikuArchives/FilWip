/*
	Bubblehelp class Copyright  (c) 1998 Marco Nelissen <marcone@xs4all.nl>
	Freely usable in non-commercial applications, as long as proper credit
	is given.

	FilWip (c) 2002 Ramshankar <ramshankar@bemail.org>
	Many thanks to Marco Nelissen for providing the Be community with this
	easy to use tooltip class. Thanks Marco!
	
	* NOTICE -----------------------------------------------------------------------
	
		This file was generously distributed free of cost by a third party developer
		and does not fall under this  program's license. The  author of this program
		wishes to thank the thirdy party developer (Marco Nelissen) for his efforts.

	* ------------------------------------------------------------------------------
	
	Original code by Marco Nelissen
	Code modified by Ramshankar*
	
	* Nothing much, just made the code look like the rest of the code of this
		project and changed the background color to a lighter yellow
*/

#include <Application.h>
#include <Window.h>
#include <TextView.h>
#include <Region.h>
#include <Screen.h>

#include <string.h>
#include <malloc.h>

#include "BubbleHelper.h"

int32 BubbleHelper::runcount = 0;

/*============================================================================================================*/

struct helppair
{
	BView	*view;
	char	*text;
};

/*============================================================================================================*/

BubbleHelper::BubbleHelper()
{
	delayTime = 0;
	
	/* You only need one instance per application */
	if (atomic_add (&runcount, 1) == 0)
	{
		helplist = new BList (30);
		helperthread = spawn_thread (_helper, "Bubble_Helper", B_NORMAL_PRIORITY, this);
		if (helperthread >= 0)
			resume_thread (helperthread);
		
		enabled = true;
	}
	else
	{
		/* Since you shouldn't be creating more than one instance you may want to jump
			straight into the debugger here */
		debugger ("only one BubbleHelper instance allowed/necessary");
	}
}

/*============================================================================================================*/

BubbleHelper::~BubbleHelper()
{
	if (helperthread>=0)
	{
		/* force helper thread into a known state */
		bool locked = textwin->Lock();

		/* Be rude... */
		kill_thread (helperthread);

		/* Dispose of window */
		if (locked)
		{
			textwin->PostMessage (B_QUIT_REQUESTED);
			textwin->Unlock();
		}
	}

	if (helplist)
	{
		helppair *pair;
		int i = helplist->CountItems()-1;
		
		while (i >= 0)
		{
			pair= (helppair*)helplist->RemoveItem (i);
			if (pair && pair->text)
				free (pair->text);
			
			delete pair;
			i--;
		}
		delete helplist;
	}
	
	atomic_add (&runcount, -1);
}

/*============================================================================================================*/

void BubbleHelper::SetHelp (BView *view, char *text)
{
	if (this && view)
	{
		/* Delete previous text for this view, if any */
		for (int i = 0;;i++)
		{
			helppair *pair;
			pair = (helppair*)helplist->ItemAt (i);
			if (!pair)
				break;
			
			if (pair->view == view)
			{
				helplist->RemoveItem (pair);
				free (pair->text);
				delete pair;
				break;
			}
		}

		/* add new text, if any */
		if (text)
		{
			helppair *pair = new helppair;
			pair->view = view;
			pair->text = strdup (text);
			helplist->AddItem (pair);
		}
	}
}

/*============================================================================================================*/

char *BubbleHelper::GetHelp (BView *view)
{
	int i = 0;
	helppair *pair;
	
	/* This could be sped up by sorting the list and doing a binary search.
		Right now this is left as an exercise for the reader, or should I say "third party opportunity"? */
	while ((pair = (helppair*)helplist->ItemAt (i++)) != NULL)
		if (pair->view == view)
			return pair->text;
	
	return NULL;
}

/*============================================================================================================*/

int32 BubbleHelper::_helper (void *arg)
{
	((BubbleHelper*)arg)->Helper();
	return 0;
}

/*============================================================================================================*/

void BubbleHelper::Helper()
{
	/* Wait until the BApplication becomes valid, in case someone creates this as a global variable. */
	while (!be_app_messenger.IsValid())
		snooze (200000);

	/* Wait a little longer, until the BApplication is really up to speed */
	while (be_app->IsLaunching())
		snooze (200000);

	textwin = new BWindow (BRect (-100, -100, -50, -50), "", B_BORDERED_WINDOW_LOOK,
				B_FLOATING_ALL_WINDOW_FEEL,	B_NOT_MOVABLE | B_AVOID_FOCUS);
	
	textview = new BTextView (BRect (0, 0, 50, 50), "", BRect (2, 2, 48, 48),
				B_FOLLOW_ALL_SIDES, B_WILL_DRAW);
	textview->MakeEditable (false);
	textview->MakeSelectable (false);
	textview->SetWordWrap (false);
	textwin->AddChild (textview);

	
	/* Changed the default background colour to a lighter one :) */
	textview->SetLowColor (255, 255, 210);
	textview->SetViewColor (255, 255, 210);
	textview->SetHighColor (0, 0, 0);
	
	textwin->Run();
	textwin->Lock();
	textwin->Activate (false);
	rename_thread (textwin->Thread(), "Bubble");
	textwin->Unlock();
	
	ulong delaycounter = 0;
	BPoint lastwhere;

	while  (be_app_messenger.IsValid())
	{
		BPoint where;
		uint32 buttons;
		if (enabled)
		{
			if (textwin->Lock())
			{
				textview->GetMouse (&where, &buttons);
				textview->ConvertToScreen (&where);
				if (lastwhere != where || buttons)
				{
					delaycounter = 0;
				}
				else
				{
					/* Mouse didn't move */
					if (delaycounter++ > 5)
					{
						delaycounter = 0;

						/* Mouse didn't move for a while */
						BView *view = FindView (where);
						char *text = NULL;
						while (view &&  (text = GetHelp (view)) == NULL)
							view = view->Parent();
						if (text)
						{
							if (delayTime)
								snooze (delayTime);
							
							DisplayHelp (text, where);
							/* Wait until mouse moves out of view, or for timeout */
							long displaycounter = 0;
							BPoint where2;
							long displaytime = max_c (20, strlen (text));
							
							do
							{
								textwin->Unlock();
								snooze (100000);
							
								if (!textwin->Lock())
									goto end; /* Window is apparently gone */
							
								textview->GetMouse (&where2, &buttons);
								textview->ConvertToScreen (&where2);
							
							} while (!buttons && where2 == where
									&&  (displaycounter++ < displaytime));
							
							HideBubble();
							
							do
							{
								textwin->Unlock();
								snooze (100000);
								if (!textwin->Lock())
									goto end; /* Window is apparently gone */
								
								textview->GetMouse (&where2,&buttons);
								textview->ConvertToScreen (&where2);
								
							} while (where2 == where);
						}
					}
				}

				lastwhere = where;
				textwin->Unlock();
			}
		}
end:
		snooze (100000);
	}

	/* This thread normally gets killed by the destructor before arriving here */
}

/*============================================================================================================*/

void BubbleHelper::HideBubble()
{
	/* hide it */
	textwin->MoveTo (-1000, -1000);
	if (!textwin->IsHidden())
		textwin->Hide();
}

/*============================================================================================================*/

void BubbleHelper::ShowBubble (BPoint dest)
{
	textwin->MoveTo (dest);
	textwin->SetWorkspaces (B_CURRENT_WORKSPACE);
	if (textwin->IsHidden())
		textwin->Show();
}

/*============================================================================================================*/

BView *BubbleHelper::FindView (BPoint where)
{
	BView *winview = NULL;
	BWindow *win;
	long windex = 0;
	
	while ((winview == NULL) && ((win = be_app->WindowAt (windex++)) != NULL))
	{
		if (win != textwin)
		{
			/* Lock with timeout, in case somebody has a non-running window in their app */
			if (win->LockWithTimeout (1E6) == B_OK)
			{
				BRect frame = win->Frame();
				if (frame.Contains (where))
				{
					BPoint winpoint;
					winpoint = where - frame.LeftTop();
					winview = win->FindView (winpoint);
					if (winview)
					{
						BRegion region;
						BPoint newpoint = where;
						winview->ConvertFromScreen (&newpoint);
						winview->GetClippingRegion (&region);
						if (!region.Contains (newpoint))
							winview = 0;
					}
				}
				win->Unlock();
			}
		}
	}
	return winview;
}

/*============================================================================================================*/

void BubbleHelper::DisplayHelp (char *text, BPoint where)
{
	textview->SetText (text);
	
	float height = textview->TextHeight (0,2E6) + 4;
	float width = 0;
	int numlines = textview->CountLines();
	int linewidth;

	for (int i = 0; i < numlines; i++)
		if ((linewidth = int (textview->LineWidth (i))) > width)
			width = linewidth;

	textwin->ResizeTo (width + 4, height);
	textview->SetTextRect (BRect (2 , 2, width + 2, height + 2));
	
	BScreen screen;
	BPoint dest= where + BPoint (0, 20);
	BRect screenframe = screen.Frame();

	if ((dest.y + height) > (screenframe.bottom - 3))
		dest.y = dest.y - (16 + height + 8);

	if ((dest.x + width + 4) > (screenframe.right))
		dest.x = dest.x - ((dest.x + width + 4) - screenframe.right);

	ShowBubble (dest);
}

/*============================================================================================================*/

void BubbleHelper::EnableHelp (bool enable)
{
	enabled = enable;
}

/*============================================================================================================*/

void BubbleHelper::SetDelayTime (bigtime_t useconds)
{
	delayTime = useconds;
}

/*============================================================================================================*/

bigtime_t BubbleHelper::DelayTime ()
{
	return delayTime;
}

/*============================================================================================================*/
