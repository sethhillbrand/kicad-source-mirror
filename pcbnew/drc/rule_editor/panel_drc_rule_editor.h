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
#include "drc_re_layers_selection_combo.h"


class PANEL_DRC_RULE_EDITOR : public PANEL_DRC_RULE_EDITOR_BASE, public DrcRuleEditorContentPanelBase
{
public:
    PANEL_DRC_RULE_EDITOR( wxWindow* aParent, BOARD* aBoard, DRC_RULE_EDITOR_CONSTRAINT_NAME aConstraintType, 
                           wxString* aConstraintTitlee, std::shared_ptr<DrcReBaseConstraintData> aConstraintData );

    ~PANEL_DRC_RULE_EDITOR() override;

    bool TransferDataToWindow() override;

    bool TransferDataFromWindow() override;

    auto GetConstraintData() { return m_constraintData; }

    void SetSaveCallback( std::function<void( int aNodeId )> aCallbackSave ) { m_callbackSave = aCallbackSave; }

    void SetRemoveCallback( std::function<void( int aNodeId )> aCallbackRemove ) { m_callbackRemove = aCallbackRemove; }

    void SetCloseCallback( std::function<void( int aNodeId )> aCallbackClose ) { m_callbackClose = aCallbackClose; }

    void SetRuleNameValidationCallback( std::function<bool( int aNodeId, wxString aRuleName )> aCallbackRuleNameValidation ) 
    { 
        m_callbackRuleNameValidation = aCallbackRuleNameValidation; 
    }

    bool GetIsValidationSucceeded() { return m_validationSucceeded; }

    std::string GetValidationMessage() { return m_validationMessage; }

    void RefreshScreen();

    bool ValidateInputs( int* aErrorCount, std::string* aValidationMessage ) override;

private:
    DrcRuleEditorContentPanelBase* getConstraintPanel( wxWindow* aParent, const DRC_RULE_EDITOR_CONSTRAINT_NAME& aConstraintType );

    void onSaveButtonClicked( wxCommandEvent& aEvent );

    void onRemoveButtonClicked( wxCommandEvent& aEvent );

    void onCloseButtonClicked( wxCommandEvent& aEvent );

protected:
    wxButton* btnSave;
    wxButton* btnRemove;
    wxButton* btnClose;
    wxComboCtrl* m_comboCtrl;

private:
    std::vector<int> m_validLayers;
    LSEQ             m_layerList; 
    BOARD*           m_board;    
    wxString*        m_constraintTitle;
    wxString*        m_name;
    wxString*        m_comment;
    bool             m_basicDetailValidated;
    bool             m_syntaxChecked;
    bool             m_isModified;
    bool             m_validationSucceeded;
    std::string      m_validationMessage;

    DRC_RE_LAYER_SELECTION_COMBO*   m_layerListCmbCtrl;
    DRC_RULE_EDITOR_CONSTRAINT_NAME m_constraintType;
    DrcRuleEditorContentPanelBase*  m_constraintPanel;
    std::shared_ptr<DrcReBaseConstraintData> m_constraintData;   

    std::function<void( int aNodeId )> m_callbackSave;
    std::function<void( int aNodeId )> m_callbackRemove;
    std::function<void( int aNodeId )> m_callbackClose;
    std::function<bool( int aNodeId, wxString aRuleName )> m_callbackRuleNameValidation;
};

#endif // PANEL_DRC_RULE_EDITOR_H
