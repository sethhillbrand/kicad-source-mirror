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

#ifndef RULE_TREE_CTRL_H
#define RULE_TREE_CTRL_H

#include <memory>
#include <unordered_set>
#include <git/kicad_git_common.h>

#include <wx/treectrl.h>
#include <wx/dcclient.h>

#include <wx/treectrl.h>
#include <unordered_set>

// Custom hash function for wxTreeItemId
struct wxTreeItemIdHash
{
    std::size_t operator()( const wxTreeItemId& item ) const
    {
        return std::hash<void*>()( item.GetID() );
    }
};

// Custom equality function for wxTreeItemId
struct wxTreeItemIdEqual
{
    bool operator()( const wxTreeItemId& lhs, const wxTreeItemId& rhs ) const { return lhs == rhs; }
};

/**
 * @class RuleTreeCtrl
 * @brief A custom wxTreeCtrl implementation that supports disabling specific tree items.
 */
class RuleTreeCtrl : public wxTreeCtrl
{
public:
    /**
     * @brief Constructs a CustomTreeCtrl instance.
     * @param parent The parent window.
     */
    explicit RuleTreeCtrl( wxWindow* parent );

    /**
     * @brief Disables a specific tree item.
     * @param item The tree item to disable.
     */
    void DisableItem( const wxTreeItemId& item );   

    /**
     * @brief Enables a specific tree item.
     * @param item The tree item to enable.
     */
    void EnableItem( const wxTreeItemId& item );

    /**
     * @brief Checks if a tree item is disabled.
     * @param item The tree item to check.
     * @return True if the item is disabled, false otherwise.
     */
    bool IsItemDisabled( const wxTreeItemId& item ) const;

protected:
    /**
     * @brief Handles the tree item activation event.
     * @param event The tree event.
     */
    void OnItemActivated( wxTreeEvent& event );

    /**
     * @brief Handles the tree item selection changing event.
     * @param event The tree event.
     */
    void OnSelChanging( wxTreeEvent& event );

    /**
     * @brief Handles the paint event to visually indicate disabled items.
     * @param event The paint event.
     */
    void OnPaint( wxPaintEvent& event );

private:
    std::unordered_set<wxTreeItemId, wxTreeItemIdHash, wxTreeItemIdEqual> m_disabledItems;

    wxDECLARE_EVENT_TABLE();
};

#endif // RULE_TREE_CTRL_H
