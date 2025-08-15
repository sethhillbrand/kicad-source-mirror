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

#include "drc_re_min_txt_ht_th_panel.h"


DRC_RE_MINIMUM_TEXT_HEIGHT_THICKNESS_PANEL::DRC_RE_MINIMUM_TEXT_HEIGHT_THICKNESS_PANEL(
        wxWindow* aParent, wxString* aConstraintTitle,
        std::shared_ptr<DRC_RE_MINIMUM_TEXT_HEIGHT_THICKNESS_CONSTRAINT_DATA> aConstraintData ) :
        DRC_RE_MINIMUM_TEXT_HEIGHT_THICKNESS_PANEL_BASE( aParent ),
        m_constraintData( aConstraintData )
{
    bConstraintImageSizer->Add(
            GetConstraintImage( this, BITMAPS::constraint_minimum_text_height_and_thickness ), 0,
                                wxALL | wxEXPAND, 10 );
}


DRC_RE_MINIMUM_TEXT_HEIGHT_THICKNESS_PANEL::~DRC_RE_MINIMUM_TEXT_HEIGHT_THICKNESS_PANEL()
{
}


bool DRC_RE_MINIMUM_TEXT_HEIGHT_THICKNESS_PANEL::TransferDataToWindow()
{
    if( m_constraintData )
    {
        m_minTextHeightTextCtrl->SetValue(
                wxString::Format( _( "%.2f" ), m_constraintData->GetMinTextHeight() ) );
        m_minTextThicknessTextCtrl->SetValue(
                wxString::Format( _( "%.2f" ), m_constraintData->GetMinTextThickness() ) );
    }

    return true;
}


bool DRC_RE_MINIMUM_TEXT_HEIGHT_THICKNESS_PANEL::TransferDataFromWindow()
{
    m_constraintData->SetMinTextHeight(
            std::stod( m_minTextHeightTextCtrl->GetValue().ToStdString() ) );
    m_constraintData->SetMinTextThickness(
            std::stod( m_minTextThicknessTextCtrl->GetValue().ToStdString() ) );

    return true;
}


bool DRC_RE_MINIMUM_TEXT_HEIGHT_THICKNESS_PANEL::ValidateInputs( int* aErrorCount,
                                                                 std::string* aValidationMessage )
{
    if( !DRC_RULE_EDITOR_UTILS::ValidateNumericCtrl( m_minTextHeightTextCtrl, "Minimum Text Height",
                                                     false, aErrorCount, aValidationMessage ) )
        return false;

    if( !DRC_RULE_EDITOR_UTILS::ValidateNumericCtrl( m_minTextThicknessTextCtrl,
                                                     "Minimum Text Thickness", false, aErrorCount,
                                                     aValidationMessage ) )
        return false;

    return true;
}