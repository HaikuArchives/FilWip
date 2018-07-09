/*
 * Copyright 2017. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 * Authors:
 *		Janus
 */
#ifndef CHECKBOX_WITH_STRING_COLUMN_H
#define CHECKBOX_WITH_STRING_COLUMN_H

#include <ColumnTypes.h>

class CheckBoxWithStringField : public BStringField {
public:
			CheckBoxWithStringField(const char* string, bool hasCheckBox = true);

	bool	HasCheckBox() const;
	void	SetMarked(bool marked) {fMarked = marked;};
	bool	IsMarked() {return fMarked;};
	void 	Toggle() {fMarked = !fMarked;};
	
private:

	bool 	fHasCheckBox;
	bool 	fMarked;
};


class CheckBoxWithStringColumn : public BStringColumn {
public:
					CheckBoxWithStringColumn(const char* title, float width,
						float minWidth, float maxWidth, uint32 truncate,
						alignment align = B_ALIGN_LEFT);

	virtual	bool	AcceptsField(const BField* field) const;
	virtual	void	DrawField(BField* field, BRect rect, BView* parent);
	virtual	void MouseDown(BColumnListView* /*parent*/, BRow* /*row*/,
					BField* /*field*/, BRect /*field_rect*/, BPoint /*point*/,
					uint32 /*buttons*/);
private:
			BRect 	CheckBoxFrame(BView* parent, BRect rect);
			float 	fOffsetH;
};

#endif	// CHECKBOX_WITH_STRING_COLUMN_H
