#ifndef _ELEMENT_LIST_VIEW_H
#define _ELEMENT_LIST_VIEW_H

#include <InterfaceKit.h>
#include <ColumnListView.h>



#define TEAM_INV	'tein'
#define SELECTION_CHANGED 'sech'
#define SET_PRIORITY 'pset'

class ElementListView : public BColumnListView {
public:
	static const int
		expander_ndx = 0,
		icon_ndx = 1,
		name_ndx = 2,
		id_ndx = 3,
		priority_ndx = 4,
		state_ndx = 5,
		areas_ndx = 6,
		CPU_ndx = 7;

	ElementListView(const char *name);

	BPopUpMenu *operationMenu;
	BMenu* priorityMenu;

	BPopUpMenu *ActionMenu();
	virtual void MakeFocus(bool focused = true);
	virtual void SelectionChanged(void);
	virtual void KeyDown(const char *bytes, int32 numBytes);
	void FullListDoForEach(bool (*func)(BRow*, void*), void* data);
	void SaveTreeState(BMessage *prefsMessage);
	void LoadTreeState(BMessage *prefsMessage);
	void SavePreset(BMessage *message);
	void LoadPreset(BMessage *message);
};
#endif
