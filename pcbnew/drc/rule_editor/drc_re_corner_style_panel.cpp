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

#include "drc_re_corner_style_panel.h"


DRC_RE_CORNER_STYLE_PANEL::DRC_RE_CORNER_STYLE_PANEL( wxWindow* aParent, wxString* aConstraintTitle,
        std::shared_ptr<DRC_RE_CORNER_STYLE_CONSTRAINT_DATA> aConstraintData ) :
        DRC_RE_CORNER_STYLE_PANEL_BASE( aParent ), m_constraintData( aConstraintData )
{
    bConstraintImageSizer->Add( GetConstraintImage( this, BITMAPS::constraint_corner_style ), 0,
                                wxALL | wxEXPAND, 10 );

    std::vector<std::string> items = { "90 Degrees", "45 Degrees", "Rounded" };

    for( const auto& item : items )
    {
        m_cornerStyleCmbCtrl->Append( item );
    }

    enableSetbackControls( false );

    m_cornerStyleCmbCtrl->Bind( wxEVT_COMBOBOX, &DRC_RE_CORNER_STYLE_PANEL::onCornerStyleSelection,
                                this );
}


DRC_RE_CORNER_STYLE_PANEL::~DRC_RE_CORNER_STYLE_PANEL()
{
    m_cornerStyleCmbCtrl->Unbind( wxEVT_COMBOBOX,
                                  &DRC_RE_CORNER_STYLE_PANEL::onCornerStyleSelection, this );
}


bool DRC_RE_CORNER_STYLE_PANEL::TransferDataToWindow()
{
    if( m_constraintData )
    {
        m_cornerStyleCmbCtrl->SetValue( m_constraintData->GetCornerStyle() );
        m_minSetbackTextCtrl->SetValue(
                wxString::Format( _( "%.2f" ), m_constraintData->GetMinSetbackLength() ) );
        m_maxSetbackTextCtrl->SetValue(
                wxString::Format( _( "%.2f" ), m_constraintData->GetMaxSetbackLength() ) );
    }

    return true;
}


bool DRC_RE_CORNER_STYLE_PANEL::TransferDataFromWindow()
{
    m_constraintData->SetCornerStyle( m_cornerStyleCmbCtrl->GetValue() );
    m_constraintData->SetMinSetbackLength(
            std::stod( m_minSetbackTextCtrl->GetValue().ToStdString() ) );
    m_constraintData->SetMaxSetbackLength(
            std::stod( m_maxSetbackTextCtrl->GetValue().ToStdString() ) );

    return true;
}


void DRC_RE_CORNER_STYLE_PANEL::onCornerStyleSelection( wxCommandEvent& aEvent )
{
    wxString selectedText = aEvent.GetString();

    enableSetbackControls( true );

    if( selectedText == wxEmptyString || selectedText == "90 Degrees" )
    {
        enableSetbackControls( false );
    }
}


void DRC_RE_CORNER_STYLE_PANEL::enableSetbackControls( bool aEnable )
{
    m_minSetbackTextCtrl->Enable( aEnable );
    m_maxSetbackTextCtrl->Enable( aEnable );
}


bool DRC_RE_CORNER_STYLE_PANEL::ValidateInputs( int* aErrorCount, std::string* aValidationMessage )
{
    if( DRC_RULE_EDITOR_UTILS::ValidateComboCtrl( m_cornerStyleCmbCtrl, "corner style", aErrorCount,
                                                  aValidationMessage ) )
    {
        if( m_cornerStyleCmbCtrl->GetStringSelection() != "90 Degrees" )
        {
            if( !DRC_RULE_EDITOR_UTILS::ValidateNumericCtrl( m_minSetbackTextCtrl,
                                                             "Minimum Setback", false, aErrorCount,
                                                             aValidationMessage ) )
                return false;

            if( !DRC_RULE_EDITOR_UTILS::ValidateNumericCtrl( m_maxSetbackTextCtrl,
                                                             "Maximum Setback", false, aErrorCount,
                                                             aValidationMessage ) )
                return false;

            if( !DRC_RULE_EDITOR_UTILS::ValidateMinMaxCtrl(
                        m_minSetbackTextCtrl, m_maxSetbackTextCtrl, "Minimum Setback",
                        "Maximum Setback", aErrorCount, aValidationMessage ) )
                return false;
        }
    }
    else
        return false;

    return true;
}