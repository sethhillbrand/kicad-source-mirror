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
#include "drc_re_bool_input_panel.h"
#include <grid_layer_box_helpers.h>
#include <bitmaps.h>
#include <confirm.h>
#include <kidialog.h>
#include <wx/bitmap.h>
#include <wx/statbmp.h>


const std::map<DRC_RULE_EDITOR_CONSTRAINT_NAME, BITMAPS> BoolConstraintBitMapPairs = 
{
    { SHORT_CIRCUIT, BITMAPS::constraint_short_circuit },
    { UNROUTED, BITMAPS::constraint_unrouted },
    { ALLOW_FILLET_OUTSIDE_ZONE_OUTLINE, BITMAPS::constraint_allow_fillet_outside_zone_outline },
    { VIAS_UNDER_SMD, BITMAPS::constraint_vias_under_smd },
};


DRC_RE_BOOL_INPUT_PANEL::DRC_RE_BOOL_INPUT_PANEL( wxWindow* aParent, const DrcReBoolInputConstraintPanelParams& aConstraintPanelParams ) :
        DRC_RE_BOOL_INPUT_PANEL_BASE( aParent ),
        m_constraintData( aConstraintPanelParams.m_constraintData )
{
    wxStaticBitmap* constraintBitmap = new wxStaticBitmap( this,  wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, 0 );

    auto it = BoolConstraintBitMapPairs.find( aConstraintPanelParams.m_constraintType );

    constraintBitmap->SetBitmap( KiBitmapBundle( it->second ) );

    bConstraintImageSizer->Add( constraintBitmap, 0, wxALL | wxEXPAND, 10 );   

    if( !aConstraintPanelParams.m_customLabelText.IsEmpty() )
        m_boolConstraintChkCtrl->SetLabelText( aConstraintPanelParams.m_customLabelText );
    else
        m_boolConstraintChkCtrl->SetLabelText( aConstraintPanelParams.m_constraintTitle );
}


DRC_RE_BOOL_INPUT_PANEL::~DRC_RE_BOOL_INPUT_PANEL()
{
}


bool DRC_RE_BOOL_INPUT_PANEL::TransferDataToWindow()
{
    if( m_constraintData )
    {
        m_boolConstraintChkCtrl->SetValue( m_constraintData->GetBoolInputValue() );
    }   

    return true;
}


bool DRC_RE_BOOL_INPUT_PANEL::TransferDataFromWindow()
{
    m_constraintData->SetBoolInputValue( m_boolConstraintChkCtrl->GetValue() );
    return false;
}


bool DRC_RE_BOOL_INPUT_PANEL::ValidateInputs( int* aErrorCount, std::string* aValidationMessage )
{
    return true;
}