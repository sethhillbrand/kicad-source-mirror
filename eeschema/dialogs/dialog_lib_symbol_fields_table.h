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

#pragma once

#include <dialog_shim.h>
#include <widgets/wx_grid.h>

#include <vector>

class SYMBOL_EDIT_FRAME;
class LIB_SYMBOL;
class LIB_BUFFER;
class SYMBOL_LIBRARY_MANAGER;
class SCH_FIELD;

wxDECLARE_EVENT( EDA_EVT_CLOSE_DIALOG_SYMBOL_FIELDS_TABLE, wxCommandEvent );

/**
 * Dialog displaying all symbols in the current library and their fields.
 */
class DIALOG_LIB_SYMBOL_FIELDS_TABLE : public DIALOG_SHIM
{
public:
    explicit DIALOG_LIB_SYMBOL_FIELDS_TABLE( SYMBOL_EDIT_FRAME* aParent );

private:
    void populateGrid();
    void OnButtonClose( wxCommandEvent& aEvent );
    void OnClose( wxCloseEvent& aEvent );

private:
    SYMBOL_EDIT_FRAME* m_parent;
    WX_GRID*           m_grid;
    std::vector<LIB_SYMBOL*> m_symbols;
    std::vector<wxString>    m_fieldNames;
};

