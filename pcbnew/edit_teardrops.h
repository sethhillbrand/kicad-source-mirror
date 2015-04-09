#ifndef TEARDROPS_EDITOR_H
#define TEARDROPS_EDITOR_H

#include "wxPcbStruct.h"
#include "tools/selection_tool.h"
#include "dialogs/dialog_teardrops.h"
#include "class_teardrop.h"

class TEARDROPS_EDITOR
{
public:
    TEARDROPS_EDITOR(PCB_EDIT_FRAME *frame, KIGFX::VIEW *view);
    bool EditTeardrops(SELECTION &selection, const DIALOG_TEARDROPS::TEARDROPS_SETTINGS &settings);

private:
    PCB_EDIT_FRAME *m_frame;
    KIGFX::VIEW *m_view;
    TEARDROP::TEARDROP_TYPE m_type;
    PICKED_ITEMS_LIST m_undoListPicker;

    void FilterSelection(SELECTION &selection);
    bool AddToAll(const DIALOG_TEARDROPS::TEARDROPS_SETTINGS &settings);
    bool AddToSelected(SELECTION &selection, const DIALOG_TEARDROPS::TEARDROPS_SETTINGS &settings);
    void DrawSegments(TEARDROP &teardrop, TRACK &track);
};

#endif // TEARDROPS_EDITOR_H
