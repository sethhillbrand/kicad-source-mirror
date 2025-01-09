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
#include <grid_layer_box_helpers.h>
#include <bitmaps.h>
#include <confirm.h>
#include <kidialog.h>
#include <layer_ids.h>
#include <layer_range.h>
#include <board.h>
#include <idf_parser.h>

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
        m_constraintType( aConstraintType ),
        m_validationSucceeded( false ),
        m_isModified( false )
{
    m_constraintPanel = getConstraintPanel( this, aConstraintType );
    m_constraintContentSizer->Add( dynamic_cast<wxPanel*>( m_constraintPanel ), 0, wxEXPAND | wxTOP, 5 );      
    m_layerList = m_board->GetEnabledLayers().UIOrder();
    m_constraintHeaderTitle->SetLabelText( *aConstraintTitle + " Constraint" );

    std::vector<PCB_LAYER_ID> layerIDs = m_layerList;

    auto layerNameGetter = [this]( PCB_LAYER_ID layer )
    {
        return m_board->GetLayerName( layer );
    };

    wxArrayInt initiallySelectedIndices = { 0, 2 }; // Indices of pre-selected layers
    m_layerListCmbCtrl = new DRC_RE_LAYER_SELECTION_COMBO( this, layerIDs, layerNameGetter );

    m_LayersComboBoxSizer->Add( m_layerListCmbCtrl, 0, wxALL | wxEXPAND, 5 ); 

     // Create another sizer for the buttons on the right
    wxBoxSizer* buttonSizer = new wxBoxSizer( wxHORIZONTAL );
    btnSave = new wxButton( this, wxID_ANY, m_constraintData->IsNew() ? "Save" : "Update" );
    btnRemove = new wxButton( this, wxID_ANY, m_constraintData->IsNew() ? "Cancel" : "Delete" );
    btnClose = new wxButton( this, wxID_ANY, "Close" );
    buttonSizer->Add( btnSave, 0, wxALL, 5 );
    buttonSizer->Add( btnRemove, 0, wxALL, 5 );
    buttonSizer->Add( btnClose, 0, wxALL, 5 );

    // Add the button sizer to the main sizer (to the right)
    bContentSizer->Add( buttonSizer, 0, wxALIGN_RIGHT | wxALL, 2 );  

    // Bind the button's event to an event handler
    btnSave->Bind( wxEVT_BUTTON, &PANEL_DRC_RULE_EDITOR::onSaveButtonClicked, this );
    btnRemove->Bind( wxEVT_BUTTON, &PANEL_DRC_RULE_EDITOR::onRemoveButtonClicked, this );
    btnClose->Bind( wxEVT_BUTTON, &PANEL_DRC_RULE_EDITOR::onCloseButtonClicked, this );
}


PANEL_DRC_RULE_EDITOR::~PANEL_DRC_RULE_EDITOR()
{
    btnSave->Unbind( wxEVT_BUTTON, &PANEL_DRC_RULE_EDITOR::onSaveButtonClicked, this );
    btnRemove->Unbind( wxEVT_BUTTON, &PANEL_DRC_RULE_EDITOR::onRemoveButtonClicked, this );
    btnClose->Unbind( wxEVT_BUTTON, &PANEL_DRC_RULE_EDITOR::onCloseButtonClicked, this );
}


bool PANEL_DRC_RULE_EDITOR::TransferDataToWindow()
{
    if( m_constraintData )
    {
        m_nameCtrl->SetValue( m_constraintData->GetRuleName() );
        m_commentCtrl->SetValue( m_constraintData->GetComment() );
        m_layerListCmbCtrl->SetItemsSelected( m_constraintData->GetLayers() );
    }

    return dynamic_cast<wxPanel*>( m_constraintPanel )->TransferDataToWindow();
}


bool PANEL_DRC_RULE_EDITOR::TransferDataFromWindow()
{
    dynamic_cast<wxPanel*>( m_constraintPanel )->TransferDataFromWindow();

    m_constraintData->SetRuleName( m_nameCtrl->GetValue() );
    m_constraintData->SetComment( m_commentCtrl->GetValue() );
    m_constraintData->SetLayers( m_layerListCmbCtrl->GetSelectedLayers() );

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
        if( DRC_RULE_EDITOR_UTILS::IsNumericInputType( aConstraintType ) )
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
        else if( DRC_RULE_EDITOR_UTILS::IsBoolInputType( aConstraintType ) )
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

bool PANEL_DRC_RULE_EDITOR::ValidateInputs( int* aErrorCount, std::string* aValidationMessage )
{
    if( !m_callbackRuleNameValidation( m_constraintData->GetId(), m_nameCtrl->GetValue() ) )
    {
        m_validationSucceeded = false;
        (*aErrorCount)++;
        m_validationMessage += DRC_RULE_EDITOR_UTILS::FormatErrorMessage( *aErrorCount, "Rule Name should be unique !!");
    }

    if( m_layerListCmbCtrl->GetSelectedItemsString() == wxEmptyString )
    {
        m_validationSucceeded = false;
        (*aErrorCount)++;
        m_validationMessage += DRC_RULE_EDITOR_UTILS::FormatErrorMessage( *aErrorCount, "Layers selection should not be empty !!");
    }    

    return m_validationSucceeded;
}

void PANEL_DRC_RULE_EDITOR::onSaveButtonClicked( wxCommandEvent& aEvent )
{
    m_validationSucceeded = true;
    int errorCount = 0;
    m_validationMessage = "";

    ValidateInputs( &errorCount, &m_validationMessage );

    if( !m_constraintPanel->ValidateInputs( &errorCount, &m_validationMessage ) )
    {
        m_validationSucceeded = false;
    }

    if( m_callbackSave )
    {
        m_callbackSave( m_constraintData->GetId() ); // Invoke the callback
    }
}

void PANEL_DRC_RULE_EDITOR::onRemoveButtonClicked( wxCommandEvent& aEvent )
{
    if( m_callbackRemove )
    {
        m_callbackRemove( m_constraintData->GetId() ); // Invoke the callback
    }
}

void PANEL_DRC_RULE_EDITOR::onCloseButtonClicked( wxCommandEvent& aEvent )
{
    if( m_callbackClose )
    {
        m_callbackClose( m_constraintData->GetId() ); // Invoke the callback
    }
}

void PANEL_DRC_RULE_EDITOR::RefreshScreen()
{
    btnSave->SetLabelText( "Update" );
    btnRemove->SetLabelText( "Delete" );
}