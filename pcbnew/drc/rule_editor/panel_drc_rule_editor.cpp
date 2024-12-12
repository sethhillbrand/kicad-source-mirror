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
#include <drc/rule_editor/drc_re_basic_clearance_panel.h>
#include <grid_layer_box_helpers.h>
#include <bitmaps.h>
#include <confirm.h>
#include <kidialog.h>
#include <layer_ids.h>
#include <layer_range.h>
#include <board.h>
#include <idf_parser.h>
#include "drc_re_base_clearance_constraint_data.h"


PANEL_DRC_RULE_EDITOR::PANEL_DRC_RULE_EDITOR( wxWindow* aParent, BOARD* aBoard, DRC_RULE_EDITOR_CONSTRAINT_NAME aConstraintType, 
                                              wxString* aConstraintTitle, std::shared_ptr<DrcReBaseConstraintData> aConstraintData ) :
        PANEL_DRC_RULE_EDITOR_BASE( aParent ),
        m_board( aBoard ), 
        m_constraintTitle( aConstraintTitle ),
        m_constraintData( aConstraintData ), 
        m_constraintType( aConstraintType )
{
    m_childPanel = getConstraintPanel( this, aConstraintType );
    m_constraintSizer->Add( dynamic_cast<wxWindow*>( m_childPanel ), 1, wxEXPAND | wxTOP, 15 );      
    m_layerList = m_board->GetEnabledLayers().UIOrder();

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
    case BASIC_CLEARANCE:
    {
        return new DRC_RE_BASIC_CLEARANCE_PANEL( aParent, m_constraintTitle, dynamic_pointer_cast<DrcReBaseClearanceConstraintData>( m_constraintData) );
    }

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

    if( m_constraintType == BASIC_CLEARANCE )
    {
        auto derivedPtr = dynamic_pointer_cast<DrcReBaseClearanceConstraintData>(m_constraintData);

        if (derivedPtr) 
        {
            derivedPtr->SetClearanceValue( dynamic_cast<DRC_RE_BASIC_CLEARANCE_PANEL*>(m_childPanel)->GetClearance() );
            derivedPtr->SetRuleName( m_textNameCtrl->GetValue() );
            derivedPtr->SetComment( m_textCommentCtrl->GetValue() );
        }
    }
}