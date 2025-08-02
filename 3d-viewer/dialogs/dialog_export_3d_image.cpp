/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright The KiCad Developers, see AUTHORS.txt for contributors.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you may find one here:
 * http://www.gnu.org/licenses/gpl-3.0.html
 * or you may search the http://www.gnu.org website for the version 3 license,
 * or you may write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#include "dialog_export_3d_image.h"
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/button.h>
#include <wx/valnum.h>

DIALOG_EXPORT_3D_IMAGE::DIALOG_EXPORT_3D_IMAGE( wxWindow* aParent,
                                                EDA_3D_VIEWER_EXPORT_FORMAT aFormat,
                                                const wxSize& aSize )
    : DIALOG_SHIM( aParent, wxID_ANY, _( "Export 3D View" ), wxDefaultPosition,
                   wxDefaultSize, wxDEFAULT_DIALOG_STYLE ),
      m_format( aFormat ),
      m_width( aSize.GetWidth() ),
      m_height( aSize.GetHeight() )
{
    wxBoxSizer* mainSizer = new wxBoxSizer( wxVERTICAL );

    wxFlexGridSizer* grid = new wxFlexGridSizer( 2, 5, 5 );

    grid->Add( new wxStaticText( this, wxID_ANY, _( "Width:" ) ), 0, wxALIGN_CENTER_VERTICAL );
    m_spinWidth = new wxSpinCtrl( this, wxID_ANY );
    m_spinWidth->SetRange( 1, 10000 );
    m_spinWidth->SetValue( m_width );
    grid->Add( m_spinWidth, 0, wxEXPAND );

    grid->Add( new wxStaticText( this, wxID_ANY, _( "Height:" ) ), 0, wxALIGN_CENTER_VERTICAL );
    m_spinHeight = new wxSpinCtrl( this, wxID_ANY );
    m_spinHeight->SetRange( 1, 10000 );
    m_spinHeight->SetValue( m_height );
    grid->Add( m_spinHeight, 0, wxEXPAND );

    grid->Add( new wxStaticText( this, wxID_ANY, _( "Format:" ) ), 0, wxALIGN_CENTER_VERTICAL );
    wxString choices[] = { wxT("PNG"), wxT("JPEG") };
    m_choiceFormat = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                  2, choices );
    m_choiceFormat->SetSelection( aFormat == EDA_3D_VIEWER_EXPORT_FORMAT::JPEG ? 1 : 0 );
    grid->Add( m_choiceFormat, 0, wxEXPAND );

    grid->AddGrowableCol( 1 );

    mainSizer->Add( grid, 0, wxEXPAND | wxALL, 10 );

    wxStdDialogButtonSizer* btnSizer = CreateStdDialogButtonSizer( wxOK | wxCANCEL );
    mainSizer->Add( btnSizer, 0, wxEXPAND | wxALL, 10 );

    SetSizerAndFit( mainSizer );
    Centre();
}

bool DIALOG_EXPORT_3D_IMAGE::TransferDataFromWindow()
{
    m_width = m_spinWidth->GetValue();
    m_height = m_spinHeight->GetValue();
    m_format = ( m_choiceFormat->GetSelection() == 1 ) ?
                    EDA_3D_VIEWER_EXPORT_FORMAT::JPEG :
                    EDA_3D_VIEWER_EXPORT_FORMAT::PNG;
    return true;
}