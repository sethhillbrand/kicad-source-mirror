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

#include "drc_re_smd_entry_panel.h"


DRC_RE_SMD_ENTRY_PANEL::DRC_RE_SMD_ENTRY_PANEL( wxWindow* aParent, wxString* aConstraintTitle,
        std::shared_ptr<DRC_RE_SMD_ENTRY_CONSTRAINT_DATA> aConstraintData ) :
        DRC_RE_SMD_ENTRY_PANEL_BASE( aParent ), m_constraintData( aConstraintData )
{
    bConstraintImageSizer->Add( GetConstraintImage( this, BITMAPS::constraint_smd_entry ),
                                0, wxALL | wxEXPAND, 10 );
}


DRC_RE_SMD_ENTRY_PANEL::~DRC_RE_SMD_ENTRY_PANEL()
{
}


bool DRC_RE_SMD_ENTRY_PANEL::TransferDataToWindow()
{
    if( m_constraintData )
    {
        m_sideAngleChkCtrl->SetValue( m_constraintData->GetIsSideAngleEnabled() );
        m_cornerAngleChkCtrl->SetValue( m_constraintData->GetIsCornerAngleEnabled() );
        m_anyAngleChkCtrl->SetValue( m_constraintData->GetIsAnyAngleEnabled() );
    }

    return true;
}


bool DRC_RE_SMD_ENTRY_PANEL::TransferDataFromWindow()
{
    m_constraintData->SetIsSideAngleEnabled( m_sideAngleChkCtrl->GetValue() );
    m_constraintData->SetIsCornerAngleEnabled( m_cornerAngleChkCtrl->GetValue() );
    m_constraintData->SetIsAnyAngleEnabled( m_anyAngleChkCtrl->GetValue() );

    return true;
}


bool DRC_RE_SMD_ENTRY_PANEL::ValidateInputs( int* aErrorCount, std::string* aValidationMessage )
{
    std::vector<wxCheckBox*> checkboxes = { m_sideAngleChkCtrl, m_cornerAngleChkCtrl,
                                            m_anyAngleChkCtrl };

    return DRC_RULE_EDITOR_UTILS::ValidateCheckBoxCtrls( checkboxes, "SMD Entry Angles",
                                                         aErrorCount, aValidationMessage );
}