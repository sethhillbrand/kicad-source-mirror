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

    return retVal;
}

bool TEARDROPS_EDITOR::AddToSelected(SELECTION &selection, const DIALOG_TEARDROPS::TEARDROPS_SETTINGS &settings)
{
    bool retVal = false;

    for (size_t i = 0; i < selection.items.GetCount(); i++) {
        TRACK *track = static_cast<TRACK *>(selection.items.GetPickedItem(i));
        TEARDROP teardropEnd;
        retVal = teardropEnd.Create(*track, ENDPOINT_END, m_type);
        if (retVal == true) {
            DrawSegments(teardropEnd, *track);
        }
        TEARDROP teardropStart;
        retVal = teardropStart.Create(*track, ENDPOINT_START, m_type);
        if (retVal == true) {
            DrawSegments(teardropStart, *track);
        }
        if (settings.m_clearSelection == true) {
            track->ClearSelected();
        }
    }

    return retVal;
}

void TEARDROPS_EDITOR::DrawSegments(TEARDROP &teardrop, TRACK &aTrack)
{
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
    }
}
