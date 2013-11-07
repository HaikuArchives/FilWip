/*
 * BeOS Clean-Up Manager
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

#ifndef _ABOUT_WINDOW_H
#define _ABOUT_WINDOW_H

#include <Window.h>
#include <TextView.h>

/* Message constants */
#define M_TIMER_MSG		'tm00'

/* Other misc. literals and constants */
#define SCROLL_DELAY		28000

#define THANKS_TO			"CREDITS"
#define LEGAL_MUMBO_JUMBO	"LEGAL MUMBO JUMBO"
#define FINAL_THANKS		"SPECIAL THANKS TO"

#define CODING				"[ Programming ]"
#define BUBBLEHELP			"[ BubbleHelp ]"
#define BESHARE				"[ BeShare ]"
#define DISCLAIMER			"[ Disclaimer ]"

class MarqueeView : public BTextView
{
	public:
		MarqueeView (BRect frame, const char *name, BRect textRect, uint32 resizeMask,
				uint32 flags = B_WILL_DRAW);
		MarqueeView (BRect frame, const char *name, BRect textRect, const BFont *initialFont,
				const rgb_color *initialColor, uint32 resizeMask, uint32 flags);
	
		void				ScrollBy (float dh, float dv);
		void				ScrollTo (float x, float y);
		
		float				curPos,
							rightEdge;
};

class AboutWindow : public BWindow
{
	public:
		AboutWindow ();
		
		/* Inherited hooks */
		virtual void		Quit ();
		virtual void		DispatchMessage (BMessage *message, BHandler *handler);
		
		/* Thread functions */
		static int32		ScrollIt (void *data);
	
	protected:
		/* Control pointers */
		BView				*backView,
							*backBlackView,
							*iconView,
							*titleView,
							*lineView;
		
		MarqueeView			*textView;
		
		/* Other variables */
		BString				lineFeeds;
		const char			*creditsText;
};

#endif /* _ABOUT_WINDOW_H */
