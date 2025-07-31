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

#include "drc_re_abs_length_two_panel.h"


DRC_RE_ABSOLUTE_LENGTH_TWO_PANEL::DRC_RE_ABSOLUTE_LENGTH_TWO_PANEL( wxWindow* aParent,
        wxString* aConstraintTitle,
        std::shared_ptr<DRC_RE_ABSOLUTE_LENGTH_TWO_CONSTRAINT_DATA> aConstraintData ) :
        DRC_RE_ABSOLUTE_LENGTH_TWO_PANEL_BASE( aParent ), m_constraintData( aConstraintData )
{
    bConstraintImageSizer->Add( GetConstraintImage( this, BITMAPS::constraint_absolute_length_2 ),
                                0, wxALL | wxEXPAND, 10 );
}


DRC_RE_ABSOLUTE_LENGTH_TWO_PANEL::~DRC_RE_ABSOLUTE_LENGTH_TWO_PANEL()
{
}


bool DRC_RE_ABSOLUTE_LENGTH_TWO_PANEL::TransferDataToWindow()
{
    return true;
}


bool DRC_RE_ABSOLUTE_LENGTH_TWO_PANEL::TransferDataFromWindow()
{
    return true;
}


bool DRC_RE_ABSOLUTE_LENGTH_TWO_PANEL::ValidateInputs( int* aErrorCount,
                                                       std::string* aValidationMessage )
{
    return true;
}