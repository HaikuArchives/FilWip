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

#ifndef _IMAGE_BUTTON_H
#define _IMAGE_BUTTON_H

#include <View.h>

class BBitmap;

class ImageButton : public BView
{
	public:
		ImageButton (const char *name, BBitmap *bitmap, BMessage *message, const rgb_color backColor);
		virtual ~ImageButton ();
		
		/* Inherited Hooks */
		virtual void		Draw (BRect updateRect);
		virtual void		MouseMoved (BPoint point, uint32 status, const BMessage *dragInfo);
		virtual void		MouseDown (BPoint point);
		virtual void		MouseUp (BPoint point);
		virtual void		MessageReceived (BMessage *message);
		
		/* Additional hooks */
		void				DrawOutsideEdge ();
		void				DrawShinyEdge (bool isPressing);
		void				PushButton ();
		
	protected:
		/* Protected members */
		BBitmap				*clickBitmap;
		BMessage			*clickMessage;
		rgb_color			backgroundColor;
		bool				firstClick,
							lastClick;
};

#endif /* _IMAGE_BUTTON_H */
