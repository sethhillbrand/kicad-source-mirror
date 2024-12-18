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

#include <pgm_base.h>
#include <settings/settings_manager.h>
#include <footprint_editor_settings.h>
#include <template_fieldnames.h>
#include <widgets/std_bitmap_button.h>
#include <grid_tricks.h>
#include <eda_text.h>
#include <drc/rule_editor/panel_drc_rule_editor.h>
#include <drc/rule_editor/drc_re_numeric_input_panel.h>
#include <drc/rule_editor/drc_re_bool_input_panel.h>
#include <drc/rule_editor/drc_re_via_style_panel.h>
#include <drc/rule_editor/drc_re_abs_length_two_panel.h>
#include <drc/rule_editor/drc_re_min_txt_ht_th_panel.h>
#include <grid_layer_box_helpers.h>
#include <bitmaps.h>
#include <confirm.h>
#include <kidialog.h>
#include <layer_ids.h>
#include <layer_range.h>
#include <board.h>
#include <idf_parser.h>
#include "drc_re_numeric_input_constraint_data.h"
#include "drc_re_bool_input_constraint_data.h"
#include "drc_re_via_style_constraint_data.h"
#include "drc_re_abs_length_two_constraint_data.h"
#include "drc_re_min_txt_ht_th_constraint_data.h"


PANEL_DRC_RULE_EDITOR::PANEL_DRC_RULE_EDITOR( wxWindow* aParent, BOARD* aBoard, DRC_RULE_EDITOR_CONSTRAINT_NAME aConstraintType, 
                                              wxString* aConstraintTitle, std::shared_ptr<DrcReBaseConstraintData> aConstraintData ) :
        PANEL_DRC_RULE_EDITOR_BASE( aParent ),
        m_board( aBoard ), 
        m_constraintTitle( aConstraintTitle ),
        m_constraintData( aConstraintData ), 
        m_constraintType( aConstraintType )
{
    m_childPanel = getConstraintPanel( this, aConstraintType );
    m_constraintContentSizer->Add( dynamic_cast<wxPanel*>( m_childPanel ), 0, wxEXPAND | wxTOP, 5 );      
    m_layerList = m_board->GetEnabledLayers().UIOrder();
    m_constraintHeaderTitle->SetLabelText( *aConstraintTitle + " Constraint" );

    wxArrayString items;

    for( PCB_LAYER_ID layer : m_layerList )
    {
        items.Add( m_board->GetLayerName( layer ) );
    }

    MultiChoiceComboBox* multiChoice = new MultiChoiceComboBox( this, items );
    multiChoice->SetMinSize( wxSize( -1, 70 ) );
    m_LayersComboBoxSizer->Add( multiChoice, 0, wxALL | wxEXPAND, 15 ); 

    BindStoredValues();
}


PANEL_DRC_RULE_EDITOR::~PANEL_DRC_RULE_EDITOR()
{
}


bool PANEL_DRC_RULE_EDITOR::TransferDataToWindow()
{
    return true;
}


bool PANEL_DRC_RULE_EDITOR::TransferDataFromWindow()
{
    return false;
}


DrcRuleEditorContentPanelBase* PANEL_DRC_RULE_EDITOR::getConstraintPanel( wxWindow* aParent,
                                                                          const DRC_RULE_EDITOR_CONSTRAINT_NAME& aConstraintType)
{
    switch( aConstraintType )
    { 
    case SHORT_CIRCUIT:
    case UNROUTED:
    case ALLOW_FILLET_OUTSIDE_ZONE_OUTLINE:        
    {
        wxString customLabel;

        switch( aConstraintType )
        {
        case SHORT_CIRCUIT: customLabel = "Allow Short Circuit"; break;
        case UNROUTED: customLabel = "Check for incomplete connections"; break;
        default: customLabel = *m_constraintTitle;
        }
            
        DrcReBoolInputConstraintPanelParams boolInputParams( *m_constraintTitle, dynamic_pointer_cast<DrcReBoolInputConstraintData>( m_constraintData ), aConstraintType, customLabel );
        
        return new DRC_RE_BOOL_INPUT_PANEL( aParent, boolInputParams );   
    }
    case NET_ANTENNA:
    case BASIC_CLEARANCE:
    case BOARD_OUTLINE_CLEARANCE:
    case MINIMUM_CLEARANCE:
    case MINIMUM_ITEM_CLEARANCE:
    case CREEPAGE_DISTANCE:
    case MINIMUM_CONNECTION_WIDTH:
    case MINIMUM_TRACK_WIDTH:
    case COPPER_TO_HOLE_CLEARANCE:
    case HOLE_TO_HOLE_CLEARANCE:
    case MINIMUM_ANNULAR_WIDTH:
    case COPPER_TO_EDGE_CLEARANCE:
    case MINIMUM_THROUGH_HOLE:
    case HOLE_SIZE:
    case HOLE_TO_HOLE_DISTANCE:
    case MINIMUM_UVIA_HOLE:
    case MINIMUM_UVIA_DIAMETER:
    case MINIMUM_VIA_DIAMETER:
    case SILK_TO_SILK_CLEARANCE:
    case SILK_TO_SOLDERMASK_CLEARANCE:
    case MINIMUM_SOLDERMASK_SILVER:
    case SOLDERMASK_EXPANSION:
    case SOLDERPASTE_EXPANSION:
    case MAXIMUM_ALLOWED_DEVIATION:
    case MINIMUM_ACUTE_ANGLE:
    case MINIMUM_ANGULAR_RING:
    case MINIMUM_THERMAL_RELIEF_SPOKE_COUNT:
    case MAXIMUM_VIA_COUNT:
    case ABSOLUTE_LENGTH:
    {
        wxString customLabel;

        switch( aConstraintType )
        {
        case NET_ANTENNA: customLabel = "Net Antenna Tolerance"; break;
        case ABSOLUTE_LENGTH: customLabel = "Net Length"; break;
        default: customLabel = *m_constraintTitle;
        }
            
        DrcReNumericInputConstraintPanelParams numericInputParams( *m_constraintTitle, dynamic_pointer_cast<DrcReNumericInputConstraintData>( m_constraintData ), aConstraintType, customLabel );
        
        if( aConstraintType == MINIMUM_THERMAL_RELIEF_SPOKE_COUNT || aConstraintType == MAXIMUM_VIA_COUNT )
            numericInputParams.SetInputIsCount( true );

        return new DRC_RE_NUMERIC_INPUT_PANEL( aParent, numericInputParams );   
    }
    case VIA_STYLE:    
        return new DRC_RE_VIA_STYLE_PANEL( aParent, m_constraintTitle, dynamic_pointer_cast<DrcReViaStyleConstraintData>( m_constraintData) );
    case ABSOLUTE_LENGTH_2:    
        return new DRC_RE_ABSOLUTE_LENGTH_TWO_PANEL( aParent, m_constraintTitle, dynamic_pointer_cast<DrcReAbsoluteLengthTwoConstraintData>( m_constraintData) );
    case MINIMUM_TEXT_HEIGHT_AND_THICKNESS:    
        return new DRC_RE_MINIMUM_TEXT_HEIGHT_THICKNESS_PANEL( aParent, m_constraintTitle, dynamic_pointer_cast<DrcReMinimumTextHeightThicknessConstraintData>( m_constraintData) );
    default: return nullptr;
    }
}


void PANEL_DRC_RULE_EDITOR::StoreCatpuredValues()
{
    m_childPanel->StoreCatpuredValues();
}


void PANEL_DRC_RULE_EDITOR::BindStoredValues()
{
    if( m_constraintData )
    {
        m_textNameCtrl->SetValue( m_constraintData->GetRuleName() );
        m_textCommentCtrl->SetValue( m_constraintData->GetComment() );
    }
}


void PANEL_DRC_RULE_EDITOR::ProcessConstraintData()
{
    StoreCatpuredValues();

    /*if( m_constraintType == BASIC_CLEARANCE )
    {*/
        auto derivedPtr = dynamic_pointer_cast<DrcReNumericInputConstraintData>(m_constraintData);

        if (derivedPtr) 
        {
            derivedPtr->SetNumericInputValue( dynamic_cast<DRC_RE_NUMERIC_INPUT_PANEL*>(m_childPanel)->GetNumericInputValue() );
            derivedPtr->SetRuleName( m_textNameCtrl->GetValue() );
            derivedPtr->SetComment( m_textCommentCtrl->GetValue() );
        }
    //}
}