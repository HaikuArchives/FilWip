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
 * Humdinger
 */

#include <ControlLook.h>
#include <View.h>
#include <Region.h>

#include <string>
#include <malloc.h>
#include <string.h>

#include "PrefsListItem.h"
#include "Constants.h"

/*============================================================================================================*/

PrefsListItem::PrefsListItem (const char *text)
	: BListItem ()
{
	label = new char [strlen (text) + 1];
	strcpy (label, text);
	
	selBackColor = PrefsItemSelBackColor;
	selTextColor = PrefsItemSelForeColor;
}

/*============================================================================================================*/

PrefsListItem::~PrefsListItem()
{
	if (label)
		delete[] label;
}

/*============================================================================================================*/

void PrefsListItem::DrawItem (BView *owner, BRect frame, bool complete)
{
	bool isSelected = IsSelected();

	if (isSelected || complete) {
		rgb_color color;
		if (isSelected)
			color = ui_color(B_LIST_SELECTED_BACKGROUND_COLOR);
		else
			color = owner->ViewColor();

		owner->SetHighColor(color);
		owner->SetLowColor(color);
		owner->FillRect(frame);
	}

	if (isSelected)
		owner->SetHighColor(ui_color(B_LIST_SELECTED_ITEM_TEXT_COLOR));
	else
		owner->SetHighColor(ui_color(B_LIST_ITEM_TEXT_COLOR));

	font_height fheight;
	owner->GetFontHeight(&fheight);

	owner->DrawString(label,
		BPoint(frame.left + be_control_look->DefaultLabelSpacing(),
			frame.top + fheight.ascent + floorf(fheight.leading / 2)));

	owner->SetHighColor(ui_color(B_LIST_ITEM_TEXT_COLOR));
	owner->SetLowColor(owner->ViewColor());
}

/*============================================================================================================*/
