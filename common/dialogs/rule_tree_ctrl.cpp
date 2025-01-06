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

#include "rule_tree_ctrl.h"
#include <wx/dcclient.h>


// Event table for handling wxTreeCtrl events
wxBEGIN_EVENT_TABLE(RuleTreeCtrl, wxTreeCtrl)
    EVT_TREE_ITEM_ACTIVATED(wxID_ANY, RuleTreeCtrl::OnItemActivated)  // Handle item activation
    EVT_TREE_SEL_CHANGING(wxID_ANY, RuleTreeCtrl::OnSelChanging)  // Handle selection change
    EVT_PAINT(RuleTreeCtrl::OnPaint)  // Handle paint events
wxEND_EVENT_TABLE()


// Constructor for RuleTreeCtrl
RuleTreeCtrl::RuleTreeCtrl(wxWindow* parent) : wxTreeCtrl(parent, wxID_ANY) 
{
    // Initialization logic can be placed here if needed
}

// Disable a specific tree item
void RuleTreeCtrl::DisableItem(const wxTreeItemId& item)
{
    m_disabledItems.insert(item);  // Add item to disabled set
    RefreshItem( item );            // Refresh to visually update the enabled item
    
}

// Enable a specific tree item
void RuleTreeCtrl::EnableItem(const wxTreeItemId& item) 
{
    m_disabledItems.erase(item);  // Remove item from disabled set
    RefreshItem(item);  // Refresh to visually update the enabled item
}

// Check if a specific item is disabled
bool RuleTreeCtrl::IsItemDisabled(const wxTreeItemId& item) const 
{
    return m_disabledItems.find(item) != m_disabledItems.end();  // Check if item is in the disabled set
}

// Custom paint event to render disabled items differently
void RuleTreeCtrl::OnPaint(wxPaintEvent& event) 
{
    wxTreeCtrl::OnPaint(event);  // Call the base class paint method
    wxClientDC dc(this);  // Device context for drawing

    // Loop through the disabled items and apply custom drawing
    for (const auto& item : m_disabledItems)
    {
        wxRect rect;
        if (GetBoundingRect(item, rect, true)) 
        {
            dc.SetTextForeground(*wxLIGHT_GREY);  // Set color for disabled items
            dc.DrawText(GetItemText(item), rect.GetTopLeft());  // Draw text with the disabled color
        }
    }
}

// Handle item activation event
void RuleTreeCtrl::OnItemActivated(wxTreeEvent& event) 
{
    if (IsItemDisabled(event.GetItem())) 
    {
        event.Veto();  // Prevent activation for disabled items
    }
}

// Handle selection changing event
void RuleTreeCtrl::OnSelChanging(wxTreeEvent& event) 
{
    if (IsItemDisabled(event.GetItem())) 
    {
        event.Veto();  // Prevent selection change for disabled items
    }
}
