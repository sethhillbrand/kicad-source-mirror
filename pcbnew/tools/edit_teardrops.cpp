/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2015 Elphel, Inc.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "edit_teardrops.h"
#include <class_board.h>
#include <class_module.h>
#include <ratsnest_data.h>
#include <view/view.h>
#include <bitmaps.h>
#include <hotkeys.h>

#include <router/pns_router.h>
#include <tool/tool_manager.h>
#include "tools/pcb_actions.h"

#include <boost/foreach.hpp>

TOOL_ACTION PCB_ACTIONS::teardropsEditor( "pcbnew.TeardropsEditor.EditTeardrops",
        AS_GLOBAL, 0,
        _( "Run teardrops editor" ), _( "Run teardrops editor" ), NULL, AF_ACTIVATE);

TEARDROPS_EDITOR::TEARDROPS_EDITOR() :
    TOOL_BASE( BATCH, TOOL_MANAGER::MakeToolId( "pcbnew.TeardropsEditor" ),
            "pcbnew.TeardropsEditor" )
{
    m_frame = NULL;
    m_view  = NULL;
    m_type  = TEARDROP::TEARDROP_STRAIGHT;
    m_strategy = DRC_COMPLY;
}


TEARDROPS_EDITOR::~TEARDROPS_EDITOR()
{
}


void TEARDROPS_EDITOR::Reset( RESET_REASON aReason )
{
    m_frame = getEditFrame<PCB_EDIT_FRAME>();
    m_view  = getView();
    m_type  = TEARDROP::TEARDROP_STRAIGHT;
    m_strategy = DRC_COMPLY;
}


void TEARDROPS_EDITOR::filterSelection(SELECTION& aSelection )
{
    for( auto& item : aSelection )
    {
        if( (item != NULL) && (item->Type() != PCB_TRACE_T) )
        {
            aSelection.Remove( item );
        }
    }
}


bool TEARDROPS_EDITOR::EditTeardrops(const DIALOG_TEARDROPS::TEARDROPS_SETTINGS& aSettings )
{
    bool retVal = false;
    SELECTION& selection = GetManager()->GetTool<SELECTION_TOOL>()->GetSelection();

    switch( aSettings.m_type )
    {
    case DIALOG_TEARDROPS::TEARDROPS_TYPE_CURVED:
        m_type = TEARDROP::TEARDROP_CURVED;
        break;

    default:
        m_type = TEARDROP::TEARDROP_STRAIGHT;
    }

    if( aSettings.m_ignoreDrc == true )
    {
        m_strategy = DRC_IGNORE;
    }
    else
    {
        m_strategy = DRC_COMPLY;
    }

    filterSelection( selection );

    if( aSettings.m_mode == DIALOG_TEARDROPS::TEARDROPS_MODE_ADD )
    {
        if( aSettings.m_track == DIALOG_TEARDROPS::TEARDROPS_TRACKS_ALL )
        {
            retVal = addToAll( aSettings );
        }
        else if( aSettings.m_track == DIALOG_TEARDROPS::TEARDROPS_TRACKS_SELECTED )
        {
            retVal = addToSelected( selection, aSettings );
        }
    }
    else if( aSettings.m_mode == DIALOG_TEARDROPS::TEARDROPS_MODE_REMOVE )
    {
        removeAll();
        retVal = true;
    }

    return retVal;
}


bool TEARDROPS_EDITOR::addToAll(const DIALOG_TEARDROPS::TEARDROPS_SETTINGS& aSettings )
{
    bool added = false;


    // Iterate through all vias and add teardrops to connected tracks
    if( (aSettings.m_scope & DIALOG_TEARDROPS::TEARDROPS_SCOPE_VIAS) ==
        DIALOG_TEARDROPS::TEARDROPS_SCOPE_VIAS )
    {
        for( VIA* via = GetFirstVia( m_frame->GetBoard()->m_Track ); via != NULL;
             via = GetFirstVia( via->Next() ) )
        {
            if( iterateTracks( via ) == true )
            {
                added = true;
            }
        }
    }

    // Iterate through all modules and add teardrops to tracks connected to their pads
    if( (aSettings.m_scope & DIALOG_TEARDROPS::TEARDROPS_SCOPE_PADS) ==
        DIALOG_TEARDROPS::TEARDROPS_SCOPE_PADS )
    {
        for( MODULE* module = m_frame->GetBoard()->m_Modules.GetFirst();
             module != NULL;
             module = module->Next() )
        {
            D_PAD* pad = module->Pads();

            while( pad != NULL )
            {
                if( (pad->GetShape() == PAD_SHAPE_CIRCLE) && iterateTracks( pad ) == true )
                {
                    added = true;
                }

                pad = pad->Next();
            }
        }
    }

    if( added == true )
    {
        m_frame->SaveCopyInUndoList( m_undoListPicker, UR_NEW );
        m_undoListPicker.ClearItemsList();
    }

    return added;
}


bool TEARDROPS_EDITOR::addToSelected(SELECTION& aSelection,
        const DIALOG_TEARDROPS::TEARDROPS_SETTINGS& aSettings )
{
    bool retVal     = false;
    bool added      = false;
    int addedNum    = 0;

    for( size_t i = 0; i < aSelection.GetSize(); i++ )
    {
        TRACK* track = static_cast<TRACK*>( aSelection[i] );
        TEARDROP teardropEnd;
        retVal = teardropEnd.Create( *track, ENDPOINT_END, m_type );

        if( retVal == true )
        {
            added = drawSegments( teardropEnd, *track );

            if( added == true )
            {
                addedNum++;
            }
        }

        TEARDROP teardropStart;
        retVal = teardropStart.Create( *track, ENDPOINT_START, m_type );

        if( retVal == true )
        {
            added = drawSegments( teardropStart, *track );

            if( added == true )
            {
                addedNum++;
            }
        }
    }

    if( aSettings.m_clearSelection == true )
    {
        GetManager()->RunAction( PCB_ACTIONS::selectionClear, true );
    }

    if( addedNum > 0 )
    {
        m_frame->SaveCopyInUndoList( m_undoListPicker, UR_NEW );
        m_undoListPicker.ClearItemsList();
    }

    return added;
}


bool TEARDROPS_EDITOR::iterateTracks( const BOARD_CONNECTED_ITEM* aObject )
{
    assert( aObject );

    bool retVal = false;
    bool flagAdded = false;

    for( size_t i = 0; i < aObject->m_TracksConnected.size(); i++ )
    {
        TRACK* track = aObject->m_TracksConnected[i];
        STATUS_FLAGS objPosition = track->IsPointOnEnds( aObject->GetPosition() );

        if( objPosition == STARTPOINT || objPosition == ENDPOINT )
        {
            ENDPOINT_T endpoint = (objPosition == STARTPOINT ? ENDPOINT_START : ENDPOINT_END);
            TEARDROP teardrop;
            retVal = teardrop.Create( *track, endpoint, m_type );

            if( retVal == true )
            {
                if( drawSegments( teardrop, *track ) == true && flagAdded == false )
                {
                    flagAdded = true;
                }
            }
        }
    }

    return flagAdded;
}


void TEARDROPS_EDITOR::removeAll()
{
    ITEM_PICKER picker( NULL, UR_DELETED );

    TRACK* nextTrack = NULL;
    bool removed = false;

    for( TRACK* track = m_frame->GetBoard()->m_Track.begin(); track != NULL; )
    {
        nextTrack = track->Next();

        if( track->GetState( FLAG1 ) == FLAG1 )
        {
            picker.SetItem( track );
            m_undoListPicker.PushItem( picker );
            removed = true;
            m_view->Remove( track );
            m_frame->GetBoard()->Remove( track );
        }

        track = nextTrack;
    }

    m_frame->GetBoard()->GetRatsnest()->Recalculate();

    if( removed == true )
    {
        m_frame->SaveCopyInUndoList( m_undoListPicker, UR_DELETED );
        m_undoListPicker.ClearItemsList();
    }
}


bool TEARDROPS_EDITOR::drawSegments(TEARDROP& aTeardrop, TRACK& aTrack )
{
    bool tracksAdded = true;
    bool proceedBuild = true;
    ITEM_PICKER picker( NULL, UR_NEW );
    PNS::NODE* world = PNS::ROUTER::GetInstance()->GetWorld();
    BOARD* board = aTrack.GetBoard();
    std::vector<TRACK*> tracks;
    std::vector<VECTOR2I> coordinates;
    aTeardrop.GetCoordinates( coordinates );

    assert( coordinates.size() != 0 );

    wxPoint currentPoint( 0, 0 );
    wxPoint prevPoint( coordinates[0].x, coordinates[0].y );

    for( size_t i = 1; i < coordinates.size(); i++ )
    {
        if( m_strategy != DRC_IGNORE )
        {
            PNS::SEGMENT segment( SEG( coordinates[i - 1], coordinates[i] ), aTrack.GetNetCode() );
            segment.SetWidth( aTrack.GetWidth() );
            segment.SetLayers( LAYER_RANGE( aTrack.GetLayer() ) );
            segment.SetParent( &aTrack );
            PNS::NODE::OBSTACLES obstacles;

            if( world->QueryColliding( &segment, obstacles, PNS::ITEM::ANY_T, 1 ) > 0 )
            {
                // DRC violation found, the segment of a teadrop can not be placed
                tracksAdded = false;
                proceedBuild = false;
                break;
            }
        }

        if( proceedBuild == true )
        {
            TRACK* track = new TRACK( aTrack );

            track->SetWidth( aTrack.GetWidth() );
            track->SetLayer( aTrack.GetLayer() );
            track->SetNetCode( aTrack.GetNetCode() );
            currentPoint = wxPoint( coordinates[i].x, coordinates[i].y );
            track->SetStart( prevPoint );
            track->SetEnd( currentPoint );
            track->ClearFlags();
            track->SetState( FLAG1, true );
            tracks.push_back( track );
            prevPoint = currentPoint;
            picker.SetItem( track );
            m_undoListPicker.PushItem( picker );
        }

        prevPoint = currentPoint;
    }

    if( tracksAdded == true )
    {
        // The actual addition is done here
        BOOST_FOREACH( TRACK * item, tracks ) {
            board->Add( item );
            m_view->Add( item );
        }
    }
    else
    {
        // The teardrop can not be created thus delete all allocated tracks and
        // remove them from undo list
        BOOST_FOREACH( TRACK * item, tracks ) {
            m_undoListPicker.PopItem();
            delete item;
        }
    }

    return tracksAdded;
}
