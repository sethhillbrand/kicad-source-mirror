#include "edit_teardrops.h"
#include "class_board.h"

TEARDROPS_EDITOR::TEARDROPS_EDITOR(PCB_EDIT_FRAME *frame, KIGFX::VIEW *view)
{
    m_frame = frame;
    m_view = view;
}

void TEARDROPS_EDITOR::FilterSelection(SELECTION &selection)
{
    EDA_ITEM *item = NULL;

    for (size_t i = 0; i < selection.items.GetCount(); i++) {
        item = selection.items.GetPickedItem(i);
        if ((item != NULL) && (item->Type() != PCB_TRACE_T)) {
            selection.items.RemovePicker(i);
        }
    }
}

bool TEARDROPS_EDITOR::EditTeardrops(SELECTION &selection, const DIALOG_TEARDROPS::TEARDROPS_SETTINGS &settings)
{
    bool retVal = false;

    switch (settings.m_type) {
    case DIALOG_TEARDROPS::TEARDROPS_TYPE_CURVED:
        m_type = TEARDROP::TEARDROP_CURVED;
        break;
    default:
        m_type = TEARDROP::TEARDROP_STRAIGHT;
    }
    FilterSelection(selection);
    if (settings.m_mode == DIALOG_TEARDROPS::TEARDROPS_MODE_ADD) {
        if (settings.m_track == DIALOG_TEARDROPS::TEARDROPS_TRACKS_ALL) {
            retVal = AddToAll(settings);
        }
        else if (settings.m_track == DIALOG_TEARDROPS::TEARDROPS_TRACKS_SELECTED) {
            retVal = AddToSelected(selection, settings);
        }
    }
    else if (settings.m_mode == DIALOG_TEARDROPS::TEARDROPS_MODE_REMOVE) {
        // TODO: consider using TRACKS_CLEARNER to remove teardrops
    }
    return retVal;
}

bool TEARDROPS_EDITOR::AddToAll(const DIALOG_TEARDROPS::TEARDROPS_SETTINGS &settings)
{
    bool retVal = false;
    bool added = false;

    if ((settings.m_scope & DIALOG_TEARDROPS::TEARDROPS_SCOPE_VIAS) == DIALOG_TEARDROPS::TEARDROPS_SCOPE_VIAS) {
        for (VIA *via = GetFirstVia(m_frame->GetBoard()->m_Track); via != NULL;
             via = GetFirstVia(via->Next())) {
            for (size_t i = 0; i < via->m_TracksConnected.size(); i++) {
                TRACK *track = via->m_TracksConnected[i];
                STATUS_FLAGS viaPosition = track->IsPointOnEnds(via->GetPosition());
                if (viaPosition == STARTPOINT || viaPosition == ENDPOINT) {
                    ENDPOINT_T endpoint = viaPosition == STARTPOINT ? ENDPOINT_START : ENDPOINT_END;
                    TEARDROP teardrop;
                    retVal = teardrop.Create(*track, endpoint, m_type);
                    if (retVal == true) {
                        DrawSegments(teardrop, *track);
                        added = true;
                    }
                }
            }
        }
    }

    if (added == true) {
        m_frame->SaveCopyInUndoList(m_undoListPicker, UR_NEW);
    }
    return added;
}

bool TEARDROPS_EDITOR::AddToSelected(SELECTION &selection, const DIALOG_TEARDROPS::TEARDROPS_SETTINGS &settings)
{
    bool retVal = false;
    bool added = false;

    for (size_t i = 0; i < selection.items.GetCount(); i++) {
        TRACK *track = static_cast<TRACK *>(selection.items.GetPickedItem(i));
        TEARDROP teardropEnd;
        retVal = teardropEnd.Create(*track, ENDPOINT_END, m_type);
        if (retVal == true) {
            DrawSegments(teardropEnd, *track);
            added = true;
        }
        TEARDROP teardropStart;
        retVal = teardropStart.Create(*track, ENDPOINT_START, m_type);
        if (retVal == true) {
            DrawSegments(teardropStart, *track);
            added = true;
        }
        if (settings.m_clearSelection == true) {
            track->ClearSelected();
        }
    }

    if (added == true) {
        m_frame->SaveCopyInUndoList(m_undoListPicker, UR_NEW);
    }
    return added;
}

void TEARDROPS_EDITOR::DrawSegments(TEARDROP &teardrop, TRACK &aTrack)
{
    PICKED_ITEMS_LIST undoListPicker;
    ITEM_PICKER picker(NULL, UR_NEW);

    std::vector<VECTOR2I> coordinates;
    teardrop.GetCoordinates(coordinates);
    BOARD *board = aTrack.GetBoard();
    wxPoint currentPoint(0, 0);
    wxPoint prevPoint(coordinates[0].x, coordinates[0].y);
    for (size_t i = 1; i < coordinates.size(); i++) {
        TRACK *track = new TRACK(aTrack);
        track->SetWidth(aTrack.GetWidth());
        track->SetLayer(aTrack.GetLayer());
        track->SetNetCode(aTrack.GetNetCode());
        currentPoint.x = coordinates[i].x;
        currentPoint.y = coordinates[i].y;
        track->SetStart(prevPoint);
        track->SetEnd(currentPoint);
        track->ClearFlags();
        board->Add(track);
        m_view->Add(track);
        prevPoint = currentPoint;
        picker.SetItem(track);
        m_undoListPicker.PushItem(picker);
    }
}
