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

#include <View.h>
#include <Region.h>

#include <string>
#include <malloc.h>

#include "PrefsListItem.h"
#include "Constants.h"

/*============================================================================================================*/

PrefsListItem::PrefsListItem (const char *text)
	: BListItem ()
{
	/* Use "new" where possible. Stupid PPC compiler does not allow char[strlen(text)+1] as it
		does not consider strlen() as a constant. So use C's strdup in case of PPC ie not Intel.  */
	#ifdef __INTEL__
		label = new char [strlen (text) + 1];
		strcpy (label, text);
	#else
		label = strdup (label);
	#endif
	
	selBackColor = PrefsItemSelBackColor;
	selTextColor = PrefsItemSelForeColor;
}

/*============================================================================================================*/

PrefsListItem::~PrefsListItem()
{
	if (label)
	{
		#ifdef __INTEL__
			delete[] label;
		#else
			free (label);
		#endif
	}
}

/*============================================================================================================*/

void PrefsListItem::DrawItem (BView *owner, BRect frame, bool complete)
{
	if (IsSelected() || complete)
	{
		rgb_color color;
		color = IsSelected() ? selBackColor : owner->ViewColor();
		owner->SetHighColor (color);
		owner->FillRect (frame);
		owner->SetLowColor (color);
	}
	else
		owner->SetLowColor (owner->ViewColor());
	
	owner->MovePenTo (frame.left + 5, frame.bottom - 4);

	if (IsEnabled())
	{
		if (IsSelected())
			owner->SetHighColor (selTextColor);
		else
			owner->SetHighColor (0,0,0,255);
	}
	else
		owner->SetHighColor (182,182,182,255);
	
	owner->DrawString (label);
}

/*============================================================================================================*/
