#ifndef _ELEMENT_LIST_VIEW_H
#define _ELEMENT_LIST_VIEW_H

#include <InterfaceKit.h>
#include <ColumnListView.h>

class ElementListView : public BColumnListView {
public:

	ElementListView(const char *name);

	BPopUpMenu *operationMenu;
	BMenu* priorityMenu;

	BPopUpMenu *ActionMenu();
	virtual void MakeFocus(bool focused = true);
	virtual void SelectionChanged(void);
	virtual void KeyDown(const char *bytes, int32 numBytes);
	virtual	void ExpandOrCollapse(BRow* row, bool expand);
	void FullListDoForEach(bool (*func)(BRow*, void*), void* data);
	void SaveTreeState(BMessage *prefsMessage);
	void LoadTreeState(BMessage *prefsMessage);
	void SavePreset(BMessage *message);
	void LoadPreset(BMessage *message);
};
#endif
