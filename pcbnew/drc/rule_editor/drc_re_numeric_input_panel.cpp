/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2024 KiCad Developers, see AUTHORS.txt for contributors.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you may find one here:
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 * or you may search the http://www.gnu.org website for the version 2 license,
 * or you may write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#include "drc_re_numeric_input_panel.h"


const std::map<DRC_RULE_EDITOR_CONSTRAINT_NAME, BITMAPS> NumericConstraintBitMapPairs = {
        { BASIC_CLEARANCE, BITMAPS::constraint_basic_clearance },
        { BOARD_OUTLINE_CLEARANCE, BITMAPS::constraint_outline_clearance },
        { MINIMUM_CLEARANCE, BITMAPS::constraint_minimum_clearance },
        { MINIMUM_ITEM_CLEARANCE, BITMAPS::constraint_minimum_item_clearance },
        { NET_ANTENNA, BITMAPS::constraint_net_antenna },
        { CREEPAGE_DISTANCE, BITMAPS::constraint_creepage_distance},
        { MINIMUM_CONNECTION_WIDTH, BITMAPS::constraint_minimum_connection_width },
        { MINIMUM_TRACK_WIDTH, BITMAPS::constraint_minimum_track_width},
        { COPPER_TO_HOLE_CLEARANCE, BITMAPS::constraint_copper_to_hole_clearance},
        { HOLE_TO_HOLE_CLEARANCE, BITMAPS::constraint_hole_to_hole_clearance},
        { MINIMUM_ANNULAR_WIDTH, BITMAPS::constraint_minimum_annular_width},
        { COPPER_TO_EDGE_CLEARANCE, BITMAPS::constraint_copper_to_edge_clearance},
        { MINIMUM_THROUGH_HOLE, BITMAPS::constraint_minimum_through_hole },
        { HOLE_SIZE, BITMAPS::constraint_hole_size },
        { HOLE_TO_HOLE_DISTANCE, BITMAPS::constraint_hole_to_hole_distance },
        { MINIMUM_UVIA_HOLE, BITMAPS::constraint_minimum_uvia_hole },
        { MINIMUM_UVIA_DIAMETER, BITMAPS::constraint_minimum_uvia_diameter },
        { MINIMUM_VIA_DIAMETER, BITMAPS::constraint_minimum_via_diameter },
        { SILK_TO_SILK_CLEARANCE, BITMAPS::constraint_silk_to_silk_clearance },
        { SILK_TO_SOLDERMASK_CLEARANCE, BITMAPS::constraint_silk_to_soldermask_clearance },
        { MINIMUM_SOLDERMASK_SILVER, BITMAPS::constraint_minimum_soldermask_silver },
        { SOLDERMASK_EXPANSION, BITMAPS::constraint_soldermask_expansion },
        { SOLDERPASTE_EXPANSION, BITMAPS::constraint_solderpaste_expansion },
        { MAXIMUM_ALLOWED_DEVIATION, BITMAPS::constraint_maximum_allowed_deviation },
        { MINIMUM_ACUTE_ANGLE, BITMAPS::constraint_minimum_acute_angle },
        { MINIMUM_ANGULAR_RING, BITMAPS::constraint_minimum_angular_ring },
        { MINIMUM_THERMAL_RELIEF_SPOKE_COUNT, BITMAPS::constraint_minimum_thermal_relief_spoke_count },
        { MAXIMUM_VIA_COUNT, BITMAPS::constraint_maximum_via_count },
        { ABSOLUTE_LENGTH, BITMAPS::constraint_absolute_length },
        { MATCHED_LENGTH_DIFF_PAIR, BITMAPS::constraint_matched_length_diff_pair },
        { MATCHED_LENGTH_ALL_TRACES_IN_GROUP, BITMAPS::constraint_matched_length_all_traces_in_group },
        { DAISY_CHAIN_STUB, BITMAPS::constraint_daisy_chain_stub },
        { SMD_CORNER, BITMAPS::constraint_smd_corner },
        { SMD_TO_PLANE_PLUS, BITMAPS::constraint_smd_to_plane_plus }
    };


DRC_RE_NUMERIC_INPUT_PANEL::DRC_RE_NUMERIC_INPUT_PANEL( wxWindow* aParent, 
        const DRC_RE_NUMERIC_INPUT_CONSTRAINT_PANEL_PARAMS& aConstraintPanelParams ) :
        DRC_RE_NUMERIC_INPUT_PANEL_BASE( aParent ),
        m_constraintData( aConstraintPanelParams.m_constraintData ),
        m_isCountInput( aConstraintPanelParams.m_isCountInput )
{
    wxStaticBitmap* constraintBitmap =
            new wxStaticBitmap( this, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, 0 );

    auto it = NumericConstraintBitMapPairs.find( aConstraintPanelParams.m_constraintType );

    constraintBitmap->SetBitmap( KiBitmapBundle( it->second ) );

    bConstraintImageSizer->Add( GetConstraintImage( this, it->second ), 0,
            wxALL | wxEXPAND, 10 );

    if( !aConstraintPanelParams.m_customLabelText.IsEmpty() )
        m_numericConstraintLabel->SetLabelText( aConstraintPanelParams.m_customLabelText );
    else
        m_numericConstraintLabel->SetLabelText( aConstraintPanelParams.m_constraintTitle );

    if( aConstraintPanelParams.m_isCountInput )
        m_numericConstraintUnit->Hide();
}


DRC_RE_NUMERIC_INPUT_PANEL::~DRC_RE_NUMERIC_INPUT_PANEL()
{
}


bool DRC_RE_NUMERIC_INPUT_PANEL::TransferDataToWindow()
{
    if( m_constraintData )
    {
        m_numericConstraintCtrl->SetValue(
                wxString::Format( _( "%.2f" ), m_constraintData->GetNumericInputValue() ) );
    }

    return true;
}


bool DRC_RE_NUMERIC_INPUT_PANEL::TransferDataFromWindow()
{
    m_constraintData->SetNumericInputValue(
            std::stod( m_numericConstraintCtrl->GetValue().ToStdString() ) );
    return true;
}


bool DRC_RE_NUMERIC_INPUT_PANEL::ValidateInputs( int* aErrorCount, std::string* aValidationMessage )
{
    if( m_isCountInput )
    {
        return DRC_RULE_EDITOR_UTILS::ValidateIntegerCtrl(
                m_numericConstraintCtrl, m_numericConstraintLabel->GetLabelText().ToStdString(),
                false, aErrorCount, aValidationMessage );
    }
    else
    {
        return DRC_RULE_EDITOR_UTILS::ValidateNumericCtrl(
                m_numericConstraintCtrl, m_numericConstraintLabel->GetLabelText().ToStdString(),
                false, aErrorCount, aValidationMessage );
    }
}