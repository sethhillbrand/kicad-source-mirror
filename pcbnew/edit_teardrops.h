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

    /**
     * @brief FilterSelection filters selected objects and removes all objects except tracks.
     * @param selection contains the list of currently selected objects
     */
    void FilterSelection(SELECTION &selection);

    /**
     * @brief IterateTracks creates teardrop for all tracks connected to \a aObject
     * @param aObject is a board object a which teardrops should be created. Currently such an object can
     * be via or circular pad.
     * @return \a true if at least one teardrop was successfully added and \a false otherwise
     */
    bool IterateTracks(const BOARD_CONNECTED_ITEM *aObject);
    bool AddToAll(const DIALOG_TEARDROPS::TEARDROPS_SETTINGS &settings);
    bool AddToSelected(SELECTION &selection, const DIALOG_TEARDROPS::TEARDROPS_SETTINGS &settings);

    /**
     * @brief RemoveAll removes all teardrops form board.
     */
    void RemoveAll();

    void DrawSegments(TEARDROP &teardrop, TRACK &track);
};

#endif // TEARDROPS_EDITOR_H
