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

#include "dialog_lib_symbol_fields_table.h"

#include <symbol_edit_frame.h>
#include <symbol_library_manager.h>
#include <lib_symbol_library_manager.h>
#include <sch_field.h>
#include <lib_symbol.h>
#include <set>

#include <wx/sizer.h>
#include <wx/button.h>

DIALOG_LIB_SYMBOL_FIELDS_TABLE::DIALOG_LIB_SYMBOL_FIELDS_TABLE( SYMBOL_EDIT_FRAME* aParent ) :
        DIALOG_SHIM( aParent, wxID_ANY, _( "Library Symbol Fields Table" ),
                     wxDefaultPosition, wxDefaultSize,
                     wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxMAXIMIZE_BOX ),
        m_parent( aParent )
{
    wxBoxSizer* topSizer = new wxBoxSizer( wxVERTICAL );
    m_grid = new WX_GRID( this, wxID_ANY );
    topSizer->Add( m_grid, 1, wxEXPAND );

    wxStdDialogButtonSizer* btnSizer = new wxStdDialogButtonSizer();
    wxButton* closeBtn = new wxButton( this, wxID_OK, _( "Close" ) );
    btnSizer->AddButton( closeBtn );
    btnSizer->Realize();
    topSizer->Add( btnSizer, 0, wxEXPAND | wxALL, 5 );

    SetSizerAndFit( topSizer );

    populateGrid();

    Bind( wxEVT_BUTTON, &DIALOG_LIB_SYMBOL_FIELDS_TABLE::OnButtonClose, this, wxID_OK );
    Bind( wxEVT_CLOSE_WINDOW, &DIALOG_LIB_SYMBOL_FIELDS_TABLE::OnClose, this );
}

void DIALOG_LIB_SYMBOL_FIELDS_TABLE::populateGrid()
{
    wxString curLib = m_parent->GetCurLib();

    if( curLib.IsEmpty() )
        return;

    SYMBOL_LIBRARY_MANAGER* libMgr = static_cast<SYMBOL_LIBRARY_MANAGER*>( &m_parent->GetLibManager() );
    LIB_BUFFER& libBuf = libMgr->GetLibraryBuffer( curLib );

    const auto& buffers = libBuf.GetBuffers();

    for( const auto& buf : buffers )
        m_symbols.push_back( &buf->GetSymbol() );

    std::set<wxString> fieldSet;

    for( LIB_SYMBOL* sym : m_symbols )
    {
        std::vector<SCH_FIELD*> fields;
        sym->GetFields( fields );

        for( SCH_FIELD* field : fields )
            fieldSet.insert( field->GetName() );
    }

    m_fieldNames.assign( fieldSet.begin(), fieldSet.end() );

    int rows = (int) m_symbols.size();
    int cols = 1 + (int) m_fieldNames.size();

    m_grid->CreateGrid( rows, cols );
    m_grid->SetColLabelValue( 0, _( "Symbol" ) );

    for( size_t i = 0; i < m_fieldNames.size(); ++i )
        m_grid->SetColLabelValue( (int) i + 1, m_fieldNames[i] );

    for( int r = 0; r < rows; ++r )
    {
        LIB_SYMBOL* sym = m_symbols[r];
        m_grid->SetCellValue( r, 0, sym->GetName() );

        std::vector<SCH_FIELD*> fields;
        sym->GetFields( fields );

        for( SCH_FIELD* field : fields )
        {
            for( size_t i = 0; i < m_fieldNames.size(); ++i )
            {
                if( m_fieldNames[i] == field->GetName() )
                    m_grid->SetCellValue( r, (int) i + 1, field->GetText() );
            }
        }
    }

    m_grid->EnableEditing( false );
    m_grid->AutoSizeColumns();
}

void DIALOG_LIB_SYMBOL_FIELDS_TABLE::OnButtonClose( wxCommandEvent& aEvent )
{
    wxCommandEvent evt( EDA_EVT_CLOSE_DIALOG_SYMBOL_FIELDS_TABLE );
    wxPostEvent( m_parent, evt );
    Destroy();
}

void DIALOG_LIB_SYMBOL_FIELDS_TABLE::OnClose( wxCloseEvent& aEvent )
{
    wxCommandEvent evt( EDA_EVT_CLOSE_DIALOG_SYMBOL_FIELDS_TABLE );
    wxPostEvent( m_parent, evt );
    Destroy();
}

