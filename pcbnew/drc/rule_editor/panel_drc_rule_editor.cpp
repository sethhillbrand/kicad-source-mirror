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
#include <drc/rule_editor/drc_re_rtg_diff_pair_panel.h>
#include <drc/rule_editor/drc_re_routing_width_panel.h>
#include <drc/rule_editor/drc_re_parallel_limit_panel.h>
#include <drc/rule_editor/drc_re_permitted_layers_panel.h>
#include <drc/rule_editor/drc_re_allowed_orientation_panel.h>
#include <drc/rule_editor/drc_re_corner_style_panel.h>
#include <drc/rule_editor/drc_re_smd_entry_panel.h>
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
#include "drc_re_rtg_diff_pair_constraint_data.h"
#include "drc_re_routing_width_constraint_data.h"
#include "drc_re_parallel_limit_constraint_data.h"
#include "drc_re_permitted_layers_constraint_data.h"
#include "drc_re_allowed_orientation_constraint_data.h"
#include "drc_re_corner_style_constraint_data.h"
#include "drc_re_smd_entry_constraint_data.h"



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
}


PANEL_DRC_RULE_EDITOR::~PANEL_DRC_RULE_EDITOR()
{
}


bool PANEL_DRC_RULE_EDITOR::TransferDataToWindow()
{
    if( m_constraintData )
    {
        m_nameCtrl->SetValue( m_constraintData->GetRuleName() );
        m_commentCtrl->SetValue( m_constraintData->GetComment() );
    }

    return dynamic_cast<wxPanel*>( m_childPanel )->TransferDataToWindow();
}


bool PANEL_DRC_RULE_EDITOR::TransferDataFromWindow()
{
    dynamic_cast<wxPanel*>( m_childPanel )->TransferDataFromWindow();

    m_constraintData->SetRuleName( m_nameCtrl->GetValue() );
    m_constraintData->SetComment( m_commentCtrl->GetValue() );

    if( DrcRuleEditorUtils::IsNumericInputType( m_constraintType ) )
    {
        auto derivedPtr = dynamic_pointer_cast<DrcReNumericInputConstraintData>( m_constraintData );

        if( derivedPtr )
        {
            derivedPtr->SetNumericInputValue( dynamic_cast<DRC_RE_NUMERIC_INPUT_PANEL*>(m_childPanel)->GetNumericInputValue() );
        }
    }
    else if( DrcRuleEditorUtils::IsBoolInputType( m_constraintType ) )
    {
        auto derivedPtr = dynamic_pointer_cast<DrcReBoolInputConstraintData>( m_constraintData );

        if( derivedPtr )
        {
            derivedPtr->SetBoolInputValue( dynamic_cast<DRC_RE_BOOL_INPUT_PANEL*>(m_childPanel)->GetBoolInputValue() );
        }
    }

    return true;
}


DrcRuleEditorContentPanelBase* PANEL_DRC_RULE_EDITOR::getConstraintPanel( wxWindow* aParent,
                                                                          const DRC_RULE_EDITOR_CONSTRAINT_NAME& aConstraintType )
{
    switch( aConstraintType )
    { 
    case VIA_STYLE:    
        return new DRC_RE_VIA_STYLE_PANEL( aParent, m_constraintTitle, dynamic_pointer_cast<DrcReViaStyleConstraintData>( m_constraintData) );
    case ABSOLUTE_LENGTH_2:    
        return new DRC_RE_ABSOLUTE_LENGTH_TWO_PANEL( aParent, m_constraintTitle, dynamic_pointer_cast<DrcReAbsoluteLengthTwoConstraintData>( m_constraintData) );
    case MINIMUM_TEXT_HEIGHT_AND_THICKNESS:    
        return new DRC_RE_MINIMUM_TEXT_HEIGHT_THICKNESS_PANEL( aParent, m_constraintTitle, dynamic_pointer_cast<DrcReMinimumTextHeightThicknessConstraintData>( m_constraintData) );
    case ROUTING_DIFF_PAIR:    
        return new DRC_RE_ROUTING_DIFF_PAIR_PANEL( aParent, m_constraintTitle, dynamic_pointer_cast<DrcReRoutingDiffPairConstraintData>( m_constraintData) );
    case ROUTING_WIDTH:    
        return new DRC_RE_ROUTING_WIDTH_PANEL( aParent, m_constraintTitle, dynamic_pointer_cast<DrcReRoutingWidthConstraintData>( m_constraintData) );
    case PARALLEL_LIMIT:    
        return new DRC_RE_PARALLEL_LIMIT_PANEL( aParent, m_constraintTitle, dynamic_pointer_cast<DrcReParallelLimitConstraintData>( m_constraintData) );
    case PERMITTED_LAYERS:    
        return new DRC_RE_PERMITTED_LAYERS_PANEL( aParent, m_constraintTitle, dynamic_pointer_cast<DrcRePermittedLayersConstraintData>( m_constraintData) );
    case ALLOWED_ORIENTATION:    
        return new DRC_RE_ALLOWED_ORIENTATION_PANEL( aParent, m_constraintTitle, dynamic_pointer_cast<DrcReAllowedOrientationConstraintData>( m_constraintData) );
    case CORNER_STYLE:    
        return new DRC_RE_CORNER_STYLE_PANEL( aParent, m_constraintTitle, dynamic_pointer_cast<DrcReCornerStyleConstraintData>( m_constraintData) );
    case SMD_ENTRY:    
        return new DRC_RE_SMD_ENTRY_PANEL( aParent, m_constraintTitle, dynamic_pointer_cast<DrcReSmdEntryConstraintData>( m_constraintData) );
    default:
        if( DrcRuleEditorUtils::IsNumericInputType( aConstraintType ) )
        {
            wxString customLabel;

            switch( aConstraintType )
            {
            case NET_ANTENNA: customLabel = "Net Antenna Tolerance"; break;
            case DAISY_CHAIN_STUB: customLabel = "Maximum Stub Length"; break;
            case ABSOLUTE_LENGTH: customLabel = "Net Length"; break;
            case SMD_CORNER: customLabel = "Minimum Permissible Distance"; break;
            case SMD_TO_PLANE_PLUS: customLabel = "Maximum Permissible Distance"; break;
            default: customLabel = *m_constraintTitle;
            }
            
            DrcReNumericInputConstraintPanelParams numericInputParams( *m_constraintTitle, dynamic_pointer_cast<DrcReNumericInputConstraintData>( m_constraintData ), aConstraintType, customLabel );
        
            if( aConstraintType == MINIMUM_THERMAL_RELIEF_SPOKE_COUNT || aConstraintType == MAXIMUM_VIA_COUNT )
                numericInputParams.SetInputIsCount( true );

            return new DRC_RE_NUMERIC_INPUT_PANEL( aParent, numericInputParams );
        }
        else if( DrcRuleEditorUtils::IsBoolInputType( aConstraintType ) )
        {
             wxString customLabel;

            switch( aConstraintType )
            {
            case SHORT_CIRCUIT: customLabel = "Allow Short Circuit"; break;
            case UNROUTED: customLabel = "Check for incomplete connections"; break;
            case VIAS_UNDER_SMD: customLabel = "Allow Vias under SMD Pads"; break;
            default: customLabel = *m_constraintTitle;
            }
            
            DrcReBoolInputConstraintPanelParams boolInputParams( *m_constraintTitle, dynamic_pointer_cast<DrcReBoolInputConstraintData>( m_constraintData ), aConstraintType, customLabel );
        
            return new DRC_RE_BOOL_INPUT_PANEL( aParent, boolInputParams );
        }
        else
        {
            return nullptr;
        };
    }
}