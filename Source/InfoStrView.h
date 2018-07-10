/*
 * FilWip
 * Copyright (c) 2002 Ramshankar
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

#ifndef _INFO_STR_VIEW_H
#define _INFO_STR_VIEW_H

#include <StringView.h>

class InfoStrView : public BStringView
{
	public:
		InfoStrView (const char *name, const char *text, uint32 flags = B_WILL_DRAW);
		virtual ~InfoStrView ();
		
		/* Inherited Hooks */
		virtual void		MouseDown (BPoint where);
		
		/* Additional Hooks */
		void				OpenFolder (const char *path) const;
		void				SetPath (const char *path);
		void				AllocPath (const char *str);
		void				DeAllocPath ();
		const char			*Path ();
		BRect				CalcAnimationRect (BPoint pt) const;
		void				DrawOpenAnimation (BRect rect);
		
	protected:
		char				*itemPath;
		int32				lastButton,
							clickCount;
		const char			*trackerSignature;
};

#endif /* _INFO_STR_VIEW_H */
