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


#include "drc_re_layers_selection_combo.h"


DRC_RE_LAYER_SELECTION_COMBO::DRC_RE_LAYER_SELECTION_COMBO( wxWindow* parent, const std::vector<PCB_LAYER_ID>& layerIDs,
        const std::function<wxString( PCB_LAYER_ID )>& nameGetter ) :
        wxComboCtrl( parent, wxID_ANY )
{
    // Create and assign the custom popup
    m_popup = new DRC_RE_LAYER_SELECTION_CHOICE_POPUP();
    SetPopupControl( m_popup );

    m_layerIDs = layerIDs;
    m_nameGetter = nameGetter;

    // Populate the popup with the layer IDs and resolve their names using the nameGetter function
    m_popup->PopulateWithLayerIDs( m_layerIDs, m_nameGetter );   

    // Bind to update the value when the popup is dismissed
    Bind( wxEVT_COMBOBOX_CLOSEUP, &DRC_RE_LAYER_SELECTION_COMBO::OnPopupClose, this );

    // Disable manual typing
    Bind( wxEVT_KEY_DOWN, &DRC_RE_LAYER_SELECTION_COMBO::OnKeyDown, this );
    Bind( wxEVT_LEFT_DOWN, &DRC_RE_LAYER_SELECTION_COMBO::OnMouseClick, this );
}

    // Get selected items as a comma-separated string
wxString DRC_RE_LAYER_SELECTION_COMBO::GetSelectedItemsString()
{
    return m_popup->GetSelectedItemsString();
}


std::vector<PCB_LAYER_ID> DRC_RE_LAYER_SELECTION_COMBO::GetSelectedLayers()
{
    return m_popup->GetSelectedLayers( m_layerIDs, m_nameGetter );
}


void DRC_RE_LAYER_SELECTION_COMBO::SetItemsSelected( const std::vector<PCB_LAYER_ID>& aSelectedLayers )
{
    // Set initial selections by indices
    m_popup->SetSelections( aSelectedLayers, m_nameGetter );

    SetValue( m_popup->GetSelectedItemsString() );
}


void DRC_RE_LAYER_SELECTION_COMBO::OnPopupClose( wxCommandEvent& )
{
    // Update the combo box value with selected items
    SetValue( m_popup->GetSelectedItemsString() );
}


void DRC_RE_LAYER_SELECTION_COMBO::OnKeyDown( wxKeyEvent& event )
{
    // Ignore key events to prevent typing
    event.Skip( false );
}


void DRC_RE_LAYER_SELECTION_COMBO::OnMouseClick( wxMouseEvent& event )
{
    // Open the dropdown on mouse click
    if( !IsPopupShown() )
    {
        ShowPopup();
    }

    event.Skip( false );
}