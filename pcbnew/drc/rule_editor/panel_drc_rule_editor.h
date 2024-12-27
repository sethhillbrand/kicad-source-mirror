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

#ifndef PANEL_DRC_RULE_EDITOR_H
#define PANEL_DRC_RULE_EDITOR_H

#include "panel_drc_rule_editor_base.h"
#include <lset.h>
#include <lseq.h>
#include <wx/wx.h>
#include <wx/combo.h>
#include <wx/popupwin.h>
#include "drc_rule_editor_enums.h"
#include "drc_rule_editor_utils.h"
#include "drc_re_content_panel_base.h"
#include "drc_re_base_constraint_data.h"
#include <variant>
#include <dialogs/rule_editor_base_data.h>


// Custom Popup for MultiChoiceComboBox
class MultiChoicePopup : public wxComboPopup
{
public:
    MultiChoicePopup() = default;

    // Initialize the popup control
    void Init() override { m_checkListBox = nullptr; }

    // Create the popup control
    bool Create( wxWindow* aParent ) override
    {
        m_checkListBox = new wxCheckListBox( aParent, wxID_ANY );
        return true;
    }

    // Return the control
    wxWindow* GetControl() override { return m_checkListBox; }

    // Update the combo box's value when the popup is dismissed
    void SetStringValue( const wxString& value ) override { m_selectedItemsString = value; }

    // Return the combo box's current value
    wxString GetStringValue() const override { return m_selectedItemsString; }

    // Populate the check list box
    void Populate( const wxArrayString& items )
    {
        m_checkListBox->Clear();
        m_checkListBox->Append( items );
    }

    // Get selected items as a comma-separated string
    wxString GetSelectedItems()
    {
        wxArrayInt checkedItems;
        wxString   selectedItems;

        m_checkListBox->GetCheckedItems( checkedItems );

        for( size_t i = 0; i < checkedItems.GetCount(); ++i )
        {
            if( !selectedItems.IsEmpty() )
                selectedItems += ", ";
            selectedItems += m_checkListBox->GetString( checkedItems[i] );
        }

        m_selectedItemsString = selectedItems;
        return selectedItems;
    }

private:
    wxCheckListBox* m_checkListBox;
    wxString        m_selectedItemsString;
};


// MultiChoiceComboBox Control
class MultiChoiceComboBox : public wxComboCtrl
{
public:
    MultiChoiceComboBox( wxWindow* parent, const wxArrayString& items ) :
            wxComboCtrl( parent, wxID_ANY )
    {
        // Create and assign the custom popup
        m_popup = new MultiChoicePopup();
        SetPopupControl( m_popup );

        // Populate the popup with items
        m_popup->Populate( items );

         // Bind to update the value when the popup is dismissed
        Bind( wxEVT_COMBOBOX_CLOSEUP, &MultiChoiceComboBox::OnPopupClose, this );

        // Disable manual typing
        Bind( wxEVT_KEY_DOWN, &MultiChoiceComboBox::OnKeyDown, this );
        Bind( wxEVT_LEFT_DOWN, &MultiChoiceComboBox::OnMouseClick, this );
    }

    wxString GetSelectedItems() { return m_popup->GetSelectedItems(); }

private:
    void OnPopupClose( wxCommandEvent& )
    {
        // Update the combo box value with selected items
        SetValue( m_popup->GetSelectedItems() );
    }

    void OnKeyDown( wxKeyEvent& event )
    {
        // Ignore key events to prevent typing
        event.Skip( false );
    }

    void OnMouseClick( wxMouseEvent& event )
    {
        // Open the dropdown on mouse click
        if( !IsPopupShown() )
        {
            ShowPopup();
        }
        event.Skip( false );
    }

    MultiChoicePopup* m_popup;
};

class PANEL_DRC_RULE_EDITOR : public PANEL_DRC_RULE_EDITOR_BASE, public DrcRuleEditorContentPanelBase
{
public:
    PANEL_DRC_RULE_EDITOR( wxWindow* aParent, BOARD* aBoard, DRC_RULE_EDITOR_CONSTRAINT_NAME aConstraintType, 
                           wxString* aConstraintTitlee, std::shared_ptr<DrcReBaseConstraintData> aConstraintData );

    ~PANEL_DRC_RULE_EDITOR() override;

    bool TransferDataToWindow() override;

    bool TransferDataFromWindow() override;

    auto GetConstraintData() { return m_constraintData; }

    void SetSaveOrUpdateCallback( std::function<void( int aNodeId )> aCallbackSaveOrUpdate ) { m_callbackSaveOrUpdate = aCallbackSaveOrUpdate; }

    void SetCancelOrDeleteCallback( std::function<void( int aNodeId )> aCallbackCancelOrDelete ) { m_callbackCancelOrDelete = aCallbackCancelOrDelete; }

    void SetCloseCallback( std::function<void( int aNodeId )> aCallbackClose ) { m_callbackClose = aCallbackClose; }

    void SetRuleNameValidationCallback( std::function<bool( int aNodeId, wxString aRuleName )> aCallbackRuleNameValidation ) { m_callbackRuleNameValidation = aCallbackRuleNameValidation; }

    bool GetIsValidationSucceeded() { return m_validationSucceeded; }

    std::string GetValidationMessage() { return m_validationMessage; }

    void RefreshScreen();

    bool ValidateInputs( int* aErrorCount, std::string* aValidationMessage ) override;

private:
    DrcRuleEditorContentPanelBase* getConstraintPanel( wxWindow* aParent, const DRC_RULE_EDITOR_CONSTRAINT_NAME& aConstraintType );

    void onSaveOrUpdateButtonClicked( wxCommandEvent& event );

    void onCancelOrDeleteButtonClicked( wxCommandEvent& event );

    void onCloseButtonClicked( wxCommandEvent& event );

private:
    std::vector<int> m_validLayers;
    LSEQ             m_layerList; 
    MultiChoiceComboBox* m_layerListCmbCtrl;
    BOARD*           m_board;
    wxComboCtrl*     m_comboCtrl;
    wxString*        m_constraintTitle;
    DRC_RULE_EDITOR_CONSTRAINT_NAME m_constraintType;
    DrcRuleEditorContentPanelBase*  m_constraintPanel;
    std::shared_ptr<DrcReBaseConstraintData> m_constraintData;
    wxString* m_name;
    wxString* m_comment;
    bool m_basicDetailValidated;
    bool m_syntaxChecked;

    bool m_isModified;
    bool m_validationSucceeded;
    std::string m_validationMessage;

    wxButton* btnSave;
    wxButton* btnCancel;

    std::function<void( int aNodeId )> m_callbackSaveOrUpdate;
    std::function<void( int aNodeId )> m_callbackCancelOrDelete;
    std::function<void( int aNodeId )> m_callbackClose;
    std::function<bool( int aNodeId, wxString aRuleName )> m_callbackRuleNameValidation;
};

#endif // PANEL_DRC_RULE_EDITOR_H
