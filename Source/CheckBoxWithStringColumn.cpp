/*
 * Copyright 2017. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 * Authors:
 *		Janus
 */

#include <ControlLook.h>
#include <Window.h>
#include "CheckBoxWithStringColumn.h"
#include <stdio.h>

#include "Constants.h"
// #pragma mark - CheckBoxWithStringField


CheckBoxWithStringField::CheckBoxWithStringField(const char* string, bool hasCheckBox)
	:
	BStringField(string),
	fHasCheckBox(hasCheckBox),
	fMarked(false)
{
}


bool
CheckBoxWithStringField::HasCheckBox() const
{
	return fHasCheckBox;
}


// #pragma mark - CheckBoxWithStringColumn


CheckBoxWithStringColumn::CheckBoxWithStringColumn(const char* title, float width, float minWidth,
	float maxWidth, uint32 truncate, alignment align)
	:
	BStringColumn(title, width, minWidth, maxWidth, align)
{
	SetWantsEvents(true);
}


bool
CheckBoxWithStringColumn::AcceptsField(const BField* field) const
{
	return static_cast<bool>(dynamic_cast<const CheckBoxWithStringField*>(field));
}

void CheckBoxWithStringColumn::MouseDown(BColumnListView* parent, BRow* row,
		BField* field, BRect field_rect, BPoint point, uint32 buttons)
{
	CheckBoxWithStringField* checkBoxStringField = static_cast<CheckBoxWithStringField*>(field);
	BRect checkFrame = CheckBoxFrame(parent, field_rect);
	BRow* parentRow = NULL;
	bool isVisible;
	parent->FindParent(row, &parentRow, &isVisible);
	checkFrame.OffsetBy(parentRow != NULL ? parent->LatchWidth() : 0, 0);

	if(checkFrame.Contains(point) ) {
		checkBoxStringField->Toggle();
		parent->Window()->PostMessage(M_CHECKBOX_CHANGED);
	}
}

BRect 
CheckBoxWithStringColumn::CheckBoxFrame(BView* parent, BRect rect)
{
	BRect checkBoxFrame(rect);
	float height = rect.Height();
	checkBoxFrame.right = rect.left + height;
	checkBoxFrame.InsetBy(2, 2);
	checkBoxFrame.OffsetBy(2, 0);
	return checkBoxFrame;
}

void
CheckBoxWithStringColumn::DrawField(BField* _field, BRect rect, BView* parent)
{
	CheckBoxWithStringField* field = static_cast<CheckBoxWithStringField*>(_field);
	rgb_color color = parent->HighColor();
	
	BRect checkBoxFrame(CheckBoxFrame(parent, rect));
	if (field->HasCheckBox()) {
		uint32 flags = BControlLook::B_BLEND_FRAME;
		if( field->IsMarked() )
			flags |= BControlLook::B_ACTIVATED;
		be_control_look->DrawCheckBox(parent, checkBoxFrame, rect,  ui_color(B_PANEL_BACKGROUND_COLOR), flags);
	}
	if (field->HasCheckBox())
		rect.left += rect.Height();
	parent->SetHighColor(color);	
	BStringColumn::DrawField(_field, rect, parent);
	parent->SetHighColor(color);
}
