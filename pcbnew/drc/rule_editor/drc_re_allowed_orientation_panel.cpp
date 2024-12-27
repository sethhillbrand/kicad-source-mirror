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
#include "drc_re_allowed_orientation_panel.h"
#include <grid_layer_box_helpers.h>
#include <bitmaps.h>
#include <confirm.h>
#include <kidialog.h>
#include <wx/bitmap.h>
#include <wx/statbmp.h>

DRC_RE_ALLOWED_ORIENTATION_PANEL::DRC_RE_ALLOWED_ORIENTATION_PANEL( wxWindow* aParent, wxString* aConstraintTitle, 
                                                        std::shared_ptr<DrcReAllowedOrientationConstraintData> aConstraintData ) :
        DRC_RE_ALLOWED_ORIENTATION_PANEL_BASE( aParent ),
        m_constraintData( aConstraintData )
{
    wxStaticBitmap* constraintBitmap = new wxStaticBitmap( this,  wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, 0 );
    constraintBitmap->SetBitmap( KiBitmapBundle( BITMAPS::constraint_allowed_orientation ) );

    bConstraintImageSizer->Add( constraintBitmap, 0, wxALL | wxEXPAND, 10 );

     // Bind the checkbox event
    m_zeroDegreesChkCtrl->Bind( wxEVT_CHECKBOX, &DRC_RE_ALLOWED_ORIENTATION_PANEL::OnCheckboxClicked, this );
    m_nintyDegreesChkCtrl->Bind( wxEVT_CHECKBOX, &DRC_RE_ALLOWED_ORIENTATION_PANEL::OnCheckboxClicked, this );
    m_oneEightyDegreesChkCtrl->Bind( wxEVT_CHECKBOX, &DRC_RE_ALLOWED_ORIENTATION_PANEL::OnCheckboxClicked, this );
    m_twoSeventyDegreesChkCtrl->Bind( wxEVT_CHECKBOX, &DRC_RE_ALLOWED_ORIENTATION_PANEL::OnCheckboxClicked, this );
    m_allOrientationsChkCtrl->Bind( wxEVT_CHECKBOX, &DRC_RE_ALLOWED_ORIENTATION_PANEL::OnAllOrientationCheckboxClicked, this );
}


DRC_RE_ALLOWED_ORIENTATION_PANEL::~DRC_RE_ALLOWED_ORIENTATION_PANEL()
{
    m_zeroDegreesChkCtrl->Unbind( wxEVT_CHECKBOX, &DRC_RE_ALLOWED_ORIENTATION_PANEL::OnCheckboxClicked, this );
    m_nintyDegreesChkCtrl->Unbind( wxEVT_CHECKBOX, &DRC_RE_ALLOWED_ORIENTATION_PANEL::OnCheckboxClicked, this );
    m_oneEightyDegreesChkCtrl->Unbind( wxEVT_CHECKBOX, &DRC_RE_ALLOWED_ORIENTATION_PANEL::OnCheckboxClicked, this );
    m_twoSeventyDegreesChkCtrl->Unbind( wxEVT_CHECKBOX, &DRC_RE_ALLOWED_ORIENTATION_PANEL::OnCheckboxClicked, this );
    m_allOrientationsChkCtrl->Unbind( wxEVT_CHECKBOX, &DRC_RE_ALLOWED_ORIENTATION_PANEL::OnAllOrientationCheckboxClicked, this );
}


bool DRC_RE_ALLOWED_ORIENTATION_PANEL::TransferDataToWindow()
{
    if( m_constraintData )
    {
        m_zeroDegreesChkCtrl->SetValue( m_constraintData->GetIsZeroDegressAllowed() );
        m_nintyDegreesChkCtrl->SetValue( m_constraintData->GetIsNintyDegressAllowed() );
        m_oneEightyDegreesChkCtrl->SetValue( m_constraintData->GetIsOneEightyDegressAllowed() );
        m_twoSeventyDegreesChkCtrl->SetValue( m_constraintData->GetIsTwoSeventyDegressAllowed() );
        m_allOrientationsChkCtrl->SetValue( m_constraintData->GetIsAllDegressAllowed() );
    }

    return true;
}


bool DRC_RE_ALLOWED_ORIENTATION_PANEL::TransferDataFromWindow()
{   
    m_constraintData->SetIsZeroDegressAllowed( m_zeroDegreesChkCtrl->GetValue() );
    m_constraintData->SetIsNintyDegressAllowed( m_nintyDegreesChkCtrl->GetValue() );
    m_constraintData->SetIsOneEightyDegressAllowed( m_oneEightyDegreesChkCtrl->GetValue() );
    m_constraintData->SetIsTwoSeventyDegressAllowed( m_twoSeventyDegreesChkCtrl->GetValue() );
    m_constraintData->SetIsAllDegressAllowed( m_allOrientationsChkCtrl->GetValue() );

    return false;
}

void DRC_RE_ALLOWED_ORIENTATION_PANEL::OnCheckboxClicked( wxCommandEvent& event )
{
    if ( !m_zeroDegreesChkCtrl->IsChecked() || !m_nintyDegreesChkCtrl->IsChecked()
        || !m_oneEightyDegreesChkCtrl->IsChecked() || !m_twoSeventyDegreesChkCtrl->IsChecked() )
    {
        m_allOrientationsChkCtrl->SetValue( false );
    }
    else
    {
        m_allOrientationsChkCtrl->SetValue( true );
    }
}

void DRC_RE_ALLOWED_ORIENTATION_PANEL::OnAllOrientationCheckboxClicked( wxCommandEvent& event )
{
    bool enable = false;

    if( m_allOrientationsChkCtrl->IsChecked() )
    {
        enable = true;
    }

    m_zeroDegreesChkCtrl->SetValue( enable );
    m_nintyDegreesChkCtrl->SetValue( enable );
    m_oneEightyDegreesChkCtrl->SetValue( enable );
    m_twoSeventyDegreesChkCtrl->SetValue( enable );
}


bool DRC_RE_ALLOWED_ORIENTATION_PANEL::ValidateInputs( int* aErrorCount, std::string* aValidationMessage )
{
    // Assuming you have a group of checkboxes like:
    std::vector<wxCheckBox*> checkboxes = { m_zeroDegreesChkCtrl, m_nintyDegreesChkCtrl,
                                            m_oneEightyDegreesChkCtrl, m_twoSeventyDegreesChkCtrl,
                                            m_allOrientationsChkCtrl };

    return DRC_RULE_EDITOR_UTILS::ValidateCheckBoxCtrls( checkboxes, "Allowed Orientations",
                                                         aErrorCount, aValidationMessage ); 
}