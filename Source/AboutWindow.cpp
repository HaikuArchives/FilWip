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
#include <Alert.h>
#include <Screen.h>
#include <View.h>
#include <TranslationUtils.h>
#include <String.h>
#include <MessageRunner.h>
#include <Bitmap.h>

#include "Constants.h"
#include "DataBits.h"

#include "AboutWindow.h"
#include "FilWip.h"

/*============================================================================================================*/

MarqueeView::MarqueeView (BRect frame, const char *name, BRect textRect,
		uint32 resizeMask, uint32 flags = B_WILL_DRAW)
	: BTextView (frame, name, textRect, resizeMask, flags)
{
	curPos = Bounds().top;
	rightEdge = Bounds().right;
}

/*============================================================================================================*/

MarqueeView::MarqueeView (BRect frame, const char *name, BRect textRect, const BFont *initialFont,
		const rgb_color *initialColor, uint32 resizeMask, uint32 flags)
	: BTextView (frame, name, textRect, initialFont, initialColor, resizeMask, flags)
{
	curPos = Bounds().top;
	rightEdge = Bounds().right;
}

/*============================================================================================================*/

void MarqueeView::ScrollTo (float x, float y)
{
	/* Reset curPos */
	curPos = y;
	BTextView::ScrollTo (x, y);
}

/*============================================================================================================*/

void MarqueeView::ScrollBy (float dh, float dv)
{
	/* Perform the fading effect, curPos records the TOP co-ord of the shading zone  */
	curPos += dv;


	/* Render the zone ;-) */
	SetDrawingMode (B_OP_BLEND);
	SetHighColor (255, 255, 255, 255);
	FillRect (BRect (0, curPos, rightEdge, curPos + 4));
	

	/* Restore the original drawing mode for Draw() */	
	SetDrawingMode (B_OP_COPY);
	BTextView::ScrollBy (dh, dv);
}

/*============================================================================================================*/
/*============================================================================================================*/
/*============================================================================================================*/

AboutWindow::AboutWindow ()
	: BWindow (BRect (0, 0, 300, 200), "About", B_MODAL_WINDOW,
		B_NOT_ZOOMABLE | B_NOT_MINIMIZABLE | B_NOT_RESIZABLE)
{
	PRINT (("AboutWindow::AboutWindow ()\n"));
	SetFeel (B_MODAL_APP_WINDOW_FEEL);
	SetLook (B_MODAL_WINDOW_LOOK);

	/* Create the BBitmap objects and set its data with error checking */
	BBitmap *appIcon = new BBitmap (BRect (0, 0, kLargeIconWidth - 1, kLargeIconHeight - 1), B_COLOR_8_BIT);
	appIcon->SetBits (iconBits, 32 * 32 * 8, 0, B_COLOR_8_BIT);
	BBitmap *bmp = BTranslationUtils::GetBitmap ('PNG ', "Image:AboutTitle");
	if (bmp == NULL)
	{
		BAlert *err = new BAlert ("Error", "An error was encountered while "
							"trying to load the resource element \"Image:AboutTitle\"\n", "Hmm..",
							NULL, NULL, B_WIDTH_AS_USUAL, B_OFFSET_SPACING, B_STOP_ALERT);
		err->Go();
		Hide();
		Quit();
		QuitRequested();
		return;
	}
	
	/* Yet another annoying control rendering section :( */
	BRect bounds (Bounds());
	backView = new BView (bounds.InsetBySelf (1, 1), "AboutWindow:BackView", B_FOLLOW_ALL_SIDES, B_WILL_DRAW);
	AddChild (backView);

	iconView = new BView (BRect (1.5 * DialogMargin + 3, 1.5 * DialogMargin,
					1.5 * DialogMargin + kLargeIconWidth - 1 + 3, 1.5 * DialogMargin + kLargeIconWidth - 1),
					"AboutWindow:IconView", B_FOLLOW_LEFT, B_WILL_DRAW);
	backView->AddChild (iconView);
	iconView->SetViewBitmap (appIcon);
	
	float left = DialogMargin + kLargeIconWidth + 1.5 * ButtonSpacing - 3;
	float top = DialogMargin / 2.0;
	titleView = new BView (BRect (left, top, 214 + left, 58 + top), "AboutWindow:TitleView",
						B_FOLLOW_LEFT, B_WILL_DRAW);
	backView->AddChild (titleView);
	titleView->SetViewBitmap (bmp);
	
	lineView = new BView (BRect (0, titleView->Frame().bottom + 3, bounds.right, titleView->Frame().bottom + 3),
						"AboutWindow:LineView", B_FOLLOW_LEFT, B_WILL_DRAW);
	lineView->SetViewColor (128, 128, 128);
	backView->AddChild (lineView);
	
	textView = new MarqueeView (BRect (2, lineView->Frame().bottom + ButtonSpacing / 2 + 2,
									bounds.right - DialogMargin - 1, bounds.bottom - 2 - ButtonSpacing / 2),
									"AboutWindow:CreditsView", BRect (0, 0, bounds.right - DialogMargin, 0),
									B_FOLLOW_LEFT, B_WILL_DRAW);
	backView->AddChild (textView);
	textView->SetStylable (true);
	textView->MakeSelectable (false);
	textView->MakeEditable (false);
	textView->SetAlignment (B_ALIGN_CENTER);
	textView->SetViewColor (BePureWhite);
	backView->SetViewColor (BePureWhite);
	textView->SetFontAndColor (be_plain_font, B_FONT_ALL, &BeJetBlack);
	
	/* Calculate no of '\n's to leave to make the text go to the bottom, calculate the no. of lines */
	font_height fntHt;
	textView->GetFontHeight (&fntHt);
	int32 noOfLines = (int32)(textView->Frame().Height() / fntHt.ascent) - 1;
	for (int32 i = 1; i < (int32)noOfLines; i++)
		lineFeeds << "\n";

	creditsText =
		"Freeware, Version " AppVersion "\n"
		"Copyright " B_UTF8_COPYRIGHT " 2002 Ramshankar\n\n\n"
		CODING "\nRamshankar\n(ramshankar@themail.com)\n\n* * *\n\n\n\n\n\n\n\n\n"

		THANKS_TO "\n\n"
		BUBBLEHELP "\nMarco Nelissen\n\n"
		BESHARE "\nJeremy Friesner\n\n"
		"Thank you all for your\n"
		"contributions with the code...\n\n* * *\n\n\n\n\n\n\n\n\n"
		"Also thanks to\n\n"
		"John Trujillo\nSebastian Benner\nM Floro\n\n"
		"for your support and contributions...\n\n* * *\n\n\n\n\n\n\n\n\n"
		"A special round of applause\n"
		"to BeShare members (in no\n"
		"particular order) :\n\n"
		"lillo\nshatty\nProcton\n"
		"Bryan\nPahtz\nmmu_man\nBeMikko\nNeil\nskiBUM\nand "
		"others\n\n"
		"for being so good... :)\n\n* * *\n\n\n\n\n\n\n\n\n"
		
		LEGAL_MUMBO_JUMBO "\n\n"
		"This program is distributed under\n"
		"its own license, and the gist of\n"
		"the license is attached to each\n"
		"source file of this program\n\n"
		
		"For third party code, the license's\n"
		"terms and conditions are explicitly\n"
		"stated and the author disclaimed of\n"
		"any and all liabilities\n\n"
		
		"For the full license read the\n"
		"file \"License.txt\" and for\n"
		"information on how to use this\n"
		"program read \"Readme.txt\"\n\n* * *\n\n\n\n\n\n\n\n\n"
		
		DISCLAIMER "\n\n"
		"Because the software is licensed\n"
		"free of charge, there is no warranty\n"
		"for the software. The copyright\n"
		"holders and/or other parties provide\n"
		"the software \"AS IS\" without warranty\n"
		"of any kind, either expressed or\n"
		"implied, including, but not limited to,\n"
		"the implied warranties of merchantability\n"
		"and fitness for a particular purpose.\n"
		"The entire risk as to the quality and\n"
		"performance of the software is with you.\n"
		"Should the software prove defective, you\n"
		"assume the cost of all necessary\n"
		"servicing, repair or correction.\n\n"
		
		"In no event will the copyright holder,\n"
		"or any other party who may modify and/or\n"
		"redistribute the software as permitted\n"
		"above, be liable to you for damages,\n"
		"including any general, special, incidental\n"
		"or consequential damages arising out of\n"
		"the use or inability to use the software\n"
		"(including but not limited to the loss of\n"
		"data or data being rendered inaccurate or\n"
		"losses sustained by you or third parties\n"
		"or a failure of the software to operate\n"
		"with any other programs), even if such\n"
		"holder or other party has been advised\n"
		"of the possibility of such damages.\n\n\n\n\n\n\n\n\n"
		
		FINAL_THANKS "\n\n"
		"Be, Inc., for making this OS\n"
		"in the first place\n\n"
		"OpenBeOS for their efforts to\n"
		"keep BeOS alive\n\n"
		"BeBits.com, BeGroovy.com, BeZip.de and\n"
		"other BeOS related sites for their\n"
		"continued enthusiasm and effort!\n\n"
		"BeOS programmers, designers, artists for\n"
		"their contributions to the OS' growth\n\n"
		"and a big applause goes to the Be\n"
		"community (it includes me too :)\n\n* * *\n\n\n\n\n\n\n\n\n"
		"OK... you can close this window now :)\n\n\n\n\n\n";
		
	textView->SetText (lineFeeds.String());
	textView->Insert (lineFeeds.Length(), creditsText, strlen(creditsText));
	
	int32 nSubHeadings = 4;
	BString subHeadings[] =
	{
		CODING,				// 0
		BUBBLEHELP,			// 2
		BESHARE,			// 3
		DISCLAIMER			// 4
	};

	int32 nMainHeadings = 3;
	BString mainHeadings[] =
	{
		THANKS_TO,			// 0
		LEGAL_MUMBO_JUMBO,	// 1
		FINAL_THANKS		// 2
	};
	
	/* Search and color sub headings */
	BString temp = textView->Text();
	int32 strt;
	for (int32 i = 0; i < nSubHeadings; i++)
	{
		if ((strt = temp.FindFirst (subHeadings[i].String())) != B_ERROR)
		{
			textView->SetFontAndColor (strt, strt + strlen(subHeadings[i].String()),
							be_plain_font, B_FONT_ALL, &TreeLabelColor);
		}
	}
	
	/* Search and color main headings */
	for (int32 i = 0; i < nMainHeadings; i++)
	{
		if ((strt = temp.FindFirst (mainHeadings[i].String())) != B_ERROR)
		{
			textView->SetFontAndColor (strt, strt + strlen(mainHeadings[i].String()),
							be_plain_font, B_FONT_ALL, &(rgb_color){0, 0, 200});
		}
	}
	
	/* Center window on-screen */
	BRect screen_rect (BScreen().Frame());
	MoveTo (screen_rect.Width() / 2 - Frame().Width() / 2, screen_rect.Height() / 2 - Frame().Height() / 2);

	/* Delete unwanted stuff */
	delete appIcon;
	delete bmp;

	/* Spawn & resume the scroller thread now */
	PRINT ((" >> spawning_thread: Magic_Scroll\n"));
	thread_id tid = spawn_thread (ScrollIt, "Magic_Scroll", B_NORMAL_PRIORITY, (void*)this);
	resume_thread (tid);
}

/*============================================================================================================*/

void AboutWindow::Quit ()
{
	PRINT (("AboutWindow::Quit ()\n"));
	be_app_messenger.SendMessage (M_CLOSE_ABOUT);
	BWindow::Quit();
}

/*============================================================================================================*/

void AboutWindow::DispatchMessage (BMessage *message, BHandler *handler)
{
	switch (message->what)
	{
		case B_KEY_DOWN: case B_MOUSE_DOWN:
		{
			Quit ();
			break;
		}
		
		default:
			BWindow::DispatchMessage (message, handler);
	}
}

/*============================================================================================================*/

int32 AboutWindow::ScrollIt (void *data)
{
	/* This thread function controls the scrolling of the marqueeview */
	AboutWindow *wnd ((AboutWindow*)data);
	float textLen, height, ptY;
	BPoint pt;

	/* Calculate a few things here so that our 'while' loop is as fast as possible */	
	wnd->textView->LockLooper();
	textLen = wnd->textView->TextLength() - 1;
	height = wnd->Bounds().Height();
	pt = wnd->textView->PointAt (wnd->textView->TextLength() - 1);
	ptY = pt.y + height;
	wnd->textView->UnlockLooper();
	MarqueeView *vw = wnd->textView;

	/* Control the scrolling view */
	for (;;)
	{
		if (vw->LockLooper() == true)
			vw->ScrollBy (0, 1);
		else
			break;
		
		if (vw->Bounds().bottom > ptY)
			vw->ScrollTo (0, 0);
		
		vw->UnlockLooper();
		snooze (SCROLL_DELAY);
	}

	return 0;
}

/*============================================================================================================*/
