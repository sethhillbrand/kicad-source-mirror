#include "edit_teardrops.h"
#include "class_board.h"
#include "class_module.h"
#include "ratsnest_data.h"
#include "view/view.h"
#include "common_actions.h"
#include "router/pns_utils.h"

TEARDROPS_EDITOR::TEARDROPS_EDITOR() :
    TOOL_BASE(BATCH, TOOL_MANAGER::MakeToolId("pcbnew.TeardropsEditor"), "pcbnew.TeardropsEditor")
{
}

TEARDROPS_EDITOR::~TEARDROPS_EDITOR()
{
}

void TEARDROPS_EDITOR::Reset(RESET_REASON aReason)
{
    m_frame = getEditFrame<PCB_EDIT_FRAME>();
    m_view = getView();
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

bool TEARDROPS_EDITOR::EditTeardrops(const DIALOG_TEARDROPS::TEARDROPS_SETTINGS &settings)
{
    bool retVal = false;
    SELECTION selection = GetManager()->GetTool<SELECTION_TOOL>()->GetSelection();

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
        RemoveAll();
        retVal = true;
    }
    return retVal;
}

bool TEARDROPS_EDITOR::AddToAll(const DIALOG_TEARDROPS::TEARDROPS_SETTINGS &settings)
{
    bool added = false;

    // Iterate through all vias and add teardrops to connected tracks
    if ((settings.m_scope & DIALOG_TEARDROPS::TEARDROPS_SCOPE_VIAS) == DIALOG_TEARDROPS::TEARDROPS_SCOPE_VIAS) {
        for (VIA *via = GetFirstVia(m_frame->GetBoard()->m_Track); via != NULL;
             via = GetFirstVia(via->Next())) {
            if (IterateTracks(via) == true) {
                added = true;
            }
        }
    }

    // Iterate through all modules and add teardrops to tracks connected to their pads
    if ((settings.m_scope & DIALOG_TEARDROPS::TEARDROPS_SCOPE_PADS) == DIALOG_TEARDROPS::TEARDROPS_SCOPE_PADS) {
        for (MODULE *module = m_frame->GetBoard()->m_Modules.GetFirst(); module != NULL; module = module->Next()) {
           D_PAD *pad = module->Pads();
           while (pad != NULL) {
               if ((pad->GetShape() == PAD_CIRCLE) && IterateTracks(pad) == true) {
                   added = true;
               }
               pad = pad->Next();
           }
        }
    }

    if (added == true) {
        m_frame->SaveCopyInUndoList(m_undoListPicker, UR_NEW);
        m_undoListPicker.ClearItemsList();
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
    }
    if (settings.m_clearSelection == true) {
        GetManager()->RunAction(COMMON_ACTIONS::selectionClear, true);
    }

    if (added == true) {
        m_frame->SaveCopyInUndoList(m_undoListPicker, UR_NEW);
        m_undoListPicker.ClearItemsList();
    }
    return added;
}

bool TEARDROPS_EDITOR::IterateTracks(const BOARD_CONNECTED_ITEM *aObject)
{
    assert(aObject);

    bool retVal = false;
    bool added = false;

    for (size_t i = 0; i < aObject->m_TracksConnected.size(); i++) {
        TRACK *track = aObject->m_TracksConnected[i];
        STATUS_FLAGS objPosition = track->IsPointOnEnds(aObject->GetPosition());
        if (objPosition == STARTPOINT || objPosition == ENDPOINT) {
            ENDPOINT_T endpoint = (objPosition == STARTPOINT ? ENDPOINT_START : ENDPOINT_END);
            TEARDROP teardrop;
            retVal = teardrop.Create(*track, endpoint, m_type);
            if (retVal == true) {
                DrawSegments(teardrop, *track);
                added = true;
            }
        }
    }
    return added;
}

void TEARDROPS_EDITOR::RemoveAll()
{
    ITEM_PICKER picker(NULL, UR_DELETED);
    TRACK *nextTrack = NULL;
    bool removed = false;

    for (TRACK *track = m_frame->GetBoard()->m_Track.begin(); track != NULL; ) {
        nextTrack = track->Next();
        if (track->GetState(FLAG1) == FLAG1) {
            picker.SetItem(track);
            m_undoListPicker.PushItem(picker);
            removed = true;
            m_view->Remove(track);
            m_frame->GetBoard()->Remove(track);
        }
        track = nextTrack;
    }
    m_frame->GetBoard()->GetRatsnest()->Recalculate();
    if (removed == true) {
        m_frame->SaveCopyInUndoList(m_undoListPicker, UR_DELETED);
        m_undoListPicker.ClearItemsList();
    }
}

void TEARDROPS_EDITOR::DrawSegments(TEARDROP &teardrop, TRACK &aTrack)
{
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
        track->SetState(FLAG1, true);
        board->Add(track);
        m_view->Add(track);
        prevPoint = currentPoint;
        picker.SetItem(track);
        m_undoListPicker.PushItem(picker);
    }
}
