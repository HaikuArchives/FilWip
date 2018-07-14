#include "ElementListView.h"
//#include <Catalog.h>

#include "Constants.h"
#include <ColumnListView.h>
#include <ColumnTypes.h>
#include "CheckBoxWithStringColumn.h"

#define B_TRANSLATE(x) x

/*
#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "ElementListView"
*/

ElementListView::ElementListView(const char *name)
	: BColumnListView("elementListView", B_FRAME_EVENTS|B_NAVIGABLE, B_FANCY_BORDER, false)
{	

	int32 i = 0;
	AddColumn(new CheckBoxWithStringColumn(B_TRANSLATE("Remove"), 180, 10, 600, 0), i++);
	AddColumn(new BIntegerColumn(B_TRANSLATE("Folders"), 70, 10, 100, B_ALIGN_RIGHT), i++);
	AddColumn(new BIntegerColumn(B_TRANSLATE("Files"), 70, 10, 100, B_ALIGN_RIGHT), i++);
	AddColumn(new BSizeColumn(B_TRANSLATE("Size"), 80, 10, 600), i++);

	SetSortingEnabled(false);
	SetColumnFlags(B_ALLOW_COLUMN_RESIZE);

	SetSelectionMode(B_SINGLE_SELECTION_LIST);
	BMessage* selected = new BMessage(M_SELECTION_CHANGED);
	selected->AddInt32("buttons",0);
	SetSelectionMessage(selected);

	MakeFocus(true);
}

void ElementListView::MakeFocus(bool focused)
{
	BColumnListView::MakeFocus(focused);
}

void ElementListView::SelectionChanged()
{
	uint32 buttons;

	BMessage *msg = Window()->CurrentMessage();

	if(msg)
		msg->FindInt32("buttons", (int32 *)&buttons);

	SelectionMessage()->ReplaceInt32("buttons",buttons);

	BMessage* invocationMessage = NULL;
	BRow* row =	CurrentSelection();
	if (row != NULL) {
		CheckBoxWithStringField *checkBoxWithStringField =
				(CheckBoxWithStringField*)row->GetField(ROW_FIELD_ENTRIES_STRING_WITH_CHECKBOX);
		if (checkBoxWithStringField->HasCheckBox()) {
				BPoint point;
				uint32 state;
				GetMouse(&point, &state);
				CheckBoxWithStringColumn* column = dynamic_cast<CheckBoxWithStringColumn*>(ColumnAt(point));
				if (column == NULL) {
					invocationMessage = new BMessage(M_OPEN_FOLDER);
					invocationMessage->AddPointer("row_pointer",row);
					invocationMessage->AddInt32("buttons",buttons);
				}
		}
	}
	SetInvocationMessage(invocationMessage);

	BColumnListView::SelectionChanged();
}

void ElementListView::KeyDown(const char *bytes, int32 numBytes)
{
	switch (bytes[0]) {
		case B_SPACE: {
			BRow* row = CurrentSelection();
			if (row != NULL) {
				CheckBoxWithStringField *checkBoxWithStringField =
					(CheckBoxWithStringField*)row->GetField(ROW_FIELD_ENTRIES_STRING_WITH_CHECKBOX);
				if (checkBoxWithStringField->HasCheckBox()) {
					checkBoxWithStringField->Toggle();
					InvalidateRow(row);
					Window()->PostMessage(M_CHECKBOX_CHANGED);
				}
			}
			break;
		}
		default: {
			BColumnListView::KeyDown(bytes, numBytes);
		}
	}
}


void ElementListView::ExpandOrCollapse(BRow* row, bool expand)
{
	BColumnListView::ExpandOrCollapse(row, expand);
	Window()->PostMessage(M_UPDATE_ZOOM_LIMITS);
}


BPopUpMenu *
ElementListView::ActionMenu()
{
	return operationMenu;
}


void
ElementListView::SaveTreeState(BMessage *prefsMessage)
{
	BMessage message(M_TREE_STATES);
	for(int32 i = 0; i < CountRows(); i++) {
		if (CountRows(RowAt(i)) > 0)
			message.AddBool (((BStringField*)(RowAt(i)->GetField(ROW_FIELD_ENTRIES_STRING_WITH_CHECKBOX)))->String(),
					RowAt(i)->IsExpanded());	
	}
	prefsMessage->RemoveName ("tree_states");
	prefsMessage->AddMessage ("tree_states", &message);
}

void
ElementListView::LoadTreeState(BMessage *prefsMessage)
{
	BMessage treeMessage (M_TREE_STATES);
	prefsMessage->FindMessage ("tree_states", &treeMessage);
	for(int32 i = 0; i < CountRows(); i++) {
		if (CountRows(RowAt(i)) > 0) {
			bool isExpanded = treeMessage.FindBool (((BStringField*)(RowAt(i)->GetField(ROW_FIELD_ENTRIES_STRING_WITH_CHECKBOX)))->String());
			ExpandOrCollapse(RowAt(i), isExpanded);
		}
	}
}


void ElementListView::FullListDoForEach(bool (*func)(BRow*, void*), void* data)
{
	for(int i = 0; i < CountRows(); i++) {
		func(RowAt(i), data);
		//printf("%s\n", ((BStringField*)(RowAt(i)->GetField(1)))->String());
//		if (RowAt(i)->IsExpanded())
		for (int j = 0; j < CountRows(RowAt(i)); j++) {
			//printf("%s\n", ((BStringField*)(RowAt(j, RowAt(i))->GetField(1)))->String());
			func(RowAt(j, RowAt(i)), data);
		}
	}
}

void ElementListView::SavePreset(BMessage *message)
{
	for(int i = 0; i < CountRows(); i++) {
		BRow *row = RowAt(i);
		CheckBoxWithStringField* checkBoxStringField = 
			static_cast<CheckBoxWithStringField*>(row->GetField(ROW_FIELD_ENTRIES_STRING_WITH_CHECKBOX));
		if (checkBoxStringField->HasCheckBox())
			message->AddBool(checkBoxStringField->String(), checkBoxStringField->IsMarked());
		else {
			BMessage subTree;
			for (int j = 0; j < CountRows(row); j++) {
				BRow *subRow = RowAt(j, row);
				CheckBoxWithStringField* checkBoxStringFieldSubRow = 
					static_cast<CheckBoxWithStringField*>(subRow->GetField(ROW_FIELD_ENTRIES_STRING_WITH_CHECKBOX));				
					subTree.AddBool(checkBoxStringFieldSubRow->String(), checkBoxStringFieldSubRow->IsMarked());		

			}
			message->AddMessage(checkBoxStringField->String(),  &subTree);
		}
	}
}


// TODO improve check error for FindBool

void ElementListView::LoadPreset(BMessage *message)
{
	for(int i = 0; i < CountRows(); i++) {
		BRow *row = RowAt(i);
		CheckBoxWithStringField* checkBoxStringField = 
			static_cast<CheckBoxWithStringField*>(row->GetField(ROW_FIELD_ENTRIES_STRING_WITH_CHECKBOX));
		if (checkBoxStringField->HasCheckBox()) {
			checkBoxStringField->SetMarked(message->FindBool(checkBoxStringField->String()));
			InvalidateRow(row);
		}
		else {
			BMessage subTree;
			if (message->FindMessage (checkBoxStringField->String(), &subTree) == B_OK)
			{
				for (int j = 0; j < CountRows(row); j++) {
					BRow *subRow = RowAt(j, row);
					CheckBoxWithStringField* checkBoxStringFieldSubRow = 
						static_cast<CheckBoxWithStringField*>(subRow->GetField(ROW_FIELD_ENTRIES_STRING_WITH_CHECKBOX));				
						checkBoxStringFieldSubRow->SetMarked(subTree.FindBool(checkBoxStringFieldSubRow->String()));
					InvalidateRow(subRow);		
				}
			}
		}
	}
}
