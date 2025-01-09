/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2024 KiCad Developers, see AUTHORS.txt for contributors.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DRC_RE_LAYER_SELECTION_CHOICE_POPUP_H
#define DRC_RE_LAYER_SELECTION_CHOICE_POPUP_H

#include <wx/wx.h>
#include <wx/combo.h>
#include <wx/checklst.h>
#include <lset.h>
#include <lseq.h>


class DRC_RE_LAYER_SELECTION_CHOICE_POPUP : public wxComboPopup
{
public:
    DRC_RE_LAYER_SELECTION_CHOICE_POPUP();

    // Initialize the popup control
    void Init() override;

    // Create the popup control
    bool Create( wxWindow* aParent ) override;

    // Return the control
    wxWindow* GetControl() override;

    // Update the combo box's value when the popup is dismissed
    void SetStringValue( const wxString& value ) override;

    // Return the combo box's current value
    wxString GetStringValue() const override;

    // Get selected items as a comma-separated string
    wxString GetSelectedItemsString();

    std::vector<PCB_LAYER_ID> GetSelectedLayers( const std::vector<PCB_LAYER_ID>& aAllLayerIds,
        const std::function<wxString(PCB_LAYER_ID)>& nameGetter );

    // Set selections by item strings
    void SetSelections( const std::vector<PCB_LAYER_ID>& layerIDs,
                        const std::function<wxString(PCB_LAYER_ID)>& nameGetter );

    void PopulateWithLayerIDs( const std::vector<PCB_LAYER_ID>& layerIDs, 
                               const std::function<wxString(PCB_LAYER_ID)>& nameGetter );

private:
    // Populate the check list box
    void populate( const wxArrayString& items );

private:
    wxCheckListBox* m_checkListBox;
    wxString        m_selectedItemsString;
};

#endif // DRC_RE_LAYER_SELECTION_CHOICE_POPUP_H