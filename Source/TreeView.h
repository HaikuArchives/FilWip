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

#ifndef _TREE_VIEW_H
#define _TREE_VIEW_H

#include <Bitmap.h>
#include <View.h>
#include <Message.h>
#include <Resources.h>
#include <Application.h>
#include <Window.h>

#include "Constants.h"

class TreeView : public BView
{
	public:
		TreeView (float left, float top, const char *name, rgb_color backColor, BMessage *expanded,
					BMessage *collapsed, BMessage *midway, int8 index);
		~TreeView ();

		/* Inherited hooks */
		virtual void		Draw (BRect updateRect);
		virtual void		MouseDown (BPoint point);
		virtual void		MouseUp (BPoint point);
		virtual void		MouseMoved (BPoint point, uint32 status, const BMessage *dragMessage);
		
		/* Extra hooks */
		void				NotifyStatus ();
		bool				IsExpanded ();
		void				SetStatus (bool expanded);
		
	protected:
		/* Protected pointers, variables */
		BBitmap				*collapsedBitmap,
							*midwayBitmap,
							*expandedBitmap;

		BMessage			*collapsedMsg,
							*expandedMsg,
							*midwayMsg;

		bool				isExpanded,
							isMidWay;

		int8				indexID;
};

#endif /* _TREE_VIEW_H */
