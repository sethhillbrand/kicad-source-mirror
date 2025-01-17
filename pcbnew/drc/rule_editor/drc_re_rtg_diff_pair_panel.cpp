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

#include "drc_re_rtg_diff_pair_panel.h"


DRC_RE_ROUTING_DIFF_PAIR_PANEL::DRC_RE_ROUTING_DIFF_PAIR_PANEL( wxWindow* aParent, wxString* aConstraintTitle,
        std::shared_ptr<DRC_RE_ROUTING_DIFF_PAIR_CONSTRAINT_DATA> aConstraintData ) :
        DRC_RE_ROUTING_DIFF_PAIR_PANEL_BASE( aParent ), m_constraintData( aConstraintData )
{
    bConstraintImageSizer->Add( GetConstraintImage( this, BITMAPS::constraint_routing_diff_pair ),
                                0,
                                wxALL | wxEXPAND, 10 );
}


DRC_RE_ROUTING_DIFF_PAIR_PANEL::~DRC_RE_ROUTING_DIFF_PAIR_PANEL()
{
}


bool DRC_RE_ROUTING_DIFF_PAIR_PANEL::TransferDataToWindow()
{
    if( m_constraintData )
    {
        m_maxUncoupledLengthTextCtrl->SetValue(
                wxString::Format( _( "%.2f" ), m_constraintData->GetMaxUncoupledLength() ) );

        m_minWidthTextCtrl->SetValue(
                wxString::Format( _( "%.2f" ), m_constraintData->GetMinWidth() ) );
        m_maxWidthTextCtrl->SetValue(
                wxString::Format( _( "%.2f" ), m_constraintData->GetMaxWidth() ) );
        m_preferredWidthTextCtrl->SetValue(
                wxString::Format( _( "%.2f" ), m_constraintData->GetPreferredWidth() ) );

        m_minGapTextCtrl->SetValue(
                wxString::Format( _( "%.2f" ), m_constraintData->GetMinGap() ) );
        m_maxGapTextCtrl->SetValue(
                wxString::Format( _( "%.2f" ), m_constraintData->GetMaxGap() ) );
        m_preferredGapTextCtrl->SetValue(
                wxString::Format( _( "%.2f" ), m_constraintData->GetPreferredGap() ) );
    }

    return true;
}


bool DRC_RE_ROUTING_DIFF_PAIR_PANEL::TransferDataFromWindow()
{
    m_constraintData->SetMaxUncoupledLength(
            std::stod( m_maxUncoupledLengthTextCtrl->GetValue().ToStdString() ) );

    m_constraintData->SetMinWidth( std::stod( m_minWidthTextCtrl->GetValue().ToStdString() ) );
    m_constraintData->SetMaxWidth( std::stod( m_maxWidthTextCtrl->GetValue().ToStdString() ) );
    m_constraintData->SetPreferredWidth(
            std::stod( m_preferredWidthTextCtrl->GetValue().ToStdString() ) );

    m_constraintData->SetMinGap( std::stod( m_minGapTextCtrl->GetValue().ToStdString() ) );
    m_constraintData->SetMaxGap( std::stod( m_maxGapTextCtrl->GetValue().ToStdString() ) );
    m_constraintData->SetPreferredGap(
            std::stod( m_preferredGapTextCtrl->GetValue().ToStdString() ) );

    return true;
}


bool DRC_RE_ROUTING_DIFF_PAIR_PANEL::ValidateInputs( int* aErrorCount,
                                                     std::string* aValidationMessage )
{
    if( !DRC_RULE_EDITOR_UTILS::ValidateNumericCtrl( m_maxUncoupledLengthTextCtrl,
                                                     "Maximum Uncoupled Length", false, aErrorCount,
                                                     aValidationMessage ) )
        return false;

    if( !DRC_RULE_EDITOR_UTILS::ValidateNumericCtrl( m_minWidthTextCtrl, "Minimum Width", false,
                                                     aErrorCount, aValidationMessage ) )
        return false;

    if( !DRC_RULE_EDITOR_UTILS::ValidateNumericCtrl( m_maxWidthTextCtrl, "Preferred Width", false,
                                                     aErrorCount, aValidationMessage ) )
        return false;

    if( !DRC_RULE_EDITOR_UTILS::ValidateNumericCtrl( m_preferredWidthTextCtrl, "Maximum Width",
                                                     false, aErrorCount, aValidationMessage ) )
        return false;

    if( !DRC_RULE_EDITOR_UTILS::ValidateNumericCtrl( m_minGapTextCtrl, "Minimum Gap", false,
                                                     aErrorCount, aValidationMessage ) )
        return false;

    if( !DRC_RULE_EDITOR_UTILS::ValidateNumericCtrl( m_maxGapTextCtrl, "Preferred Gap", false,
                                                     aErrorCount, aValidationMessage ) )
        return false;

    if( !DRC_RULE_EDITOR_UTILS::ValidateNumericCtrl( m_preferredGapTextCtrl, "Maximum Gap", false,
                                                     aErrorCount, aValidationMessage ) )
        return false;

    if( !DRC_RULE_EDITOR_UTILS::ValidateMinPreferredMaxCtrl( m_minWidthTextCtrl, 
                m_preferredWidthTextCtrl, m_maxWidthTextCtrl, "Minimum Width", "Preferred Width", 
                "Maximum Width", aErrorCount, aValidationMessage ) )
        return false;

    if( !DRC_RULE_EDITOR_UTILS::ValidateMinPreferredMaxCtrl( m_minGapTextCtrl, m_preferredGapTextCtrl, 
                m_maxGapTextCtrl, "Minimum Gap", "Preferred Gap", "Maximum Gap", aErrorCount, 
                aValidationMessage ) )
        return false;

    return true;
}