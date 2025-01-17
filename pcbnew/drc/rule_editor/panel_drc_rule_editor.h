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

#include <wx/wx.h>
#include <wx/combo.h>
#include <wx/popupwin.h>

#include <lset.h>
#include <lseq.h>
#include <variant>

#include "panel_drc_rule_editor_base.h"
#include "drc_rule_editor_enums.h"
#include "drc_rule_editor_utils.h"
#include "drc_re_content_panel_base.h"
#include "drc_re_base_constraint_data.h"
#include <dialogs/rule_editor_data_base.h>
#include "drc_re_layers_selection_combo.h"


class SCINTILLA_TRICKS;
class HTML_MESSAGE_BOX;

class PANEL_DRC_RULE_EDITOR : public PANEL_DRC_RULE_EDITOR_BASE,
                              public DRC_RULE_EDITOR_CONTENT_PANEL_BASE
{
public:
    PANEL_DRC_RULE_EDITOR( wxWindow* aParent, BOARD* aBoard,
                           DRC_RULE_EDITOR_CONSTRAINT_NAME aConstraintType,
                           wxString* aConstraintTitlee,
                           std::shared_ptr<DRC_RE_BASE_CONSTRAINT_DATA> aConstraintData );

    ~PANEL_DRC_RULE_EDITOR() override;

    bool TransferDataToWindow() override;

    bool TransferDataFromWindow() override;

    auto GetConstraintData() { return m_constraintData; }

    void SetSaveCallback( std::function<void( int aNodeId )> aCallBackSave )
    {
        m_callBackSave = aCallBackSave;
    }

    void SetRemoveCallback( std::function<void( int aNodeId )> aCallBackRemove )
    {
        m_callBackRemove = aCallBackRemove;
    }

    void SetCloseCallback( std::function<void( int aNodeId )> aCallBackClose )
    {
        m_callBackClose = aCallBackClose;
    }

    void SetRuleNameValidationCallback(
            std::function<bool( int aNodeId, wxString aRuleName )> aCallbackRuleNameValidation )
    {
        m_callBackRuleNameValidation = aCallbackRuleNameValidation;
    }

    bool GetIsValidationSucceeded() { return m_validationSucceeded; }

    std::string GetValidationMessage() { return m_validationMessage; }

    void RefreshScreen();

    bool ValidateInputs( int* aErrorCount, std::string* aValidationMessage ) override;

private:
    DRC_RULE_EDITOR_CONTENT_PANEL_BASE* getConstraintPanel( wxWindow* aParent, 
        const DRC_RULE_EDITOR_CONSTRAINT_NAME& aConstraintType );

    void onSaveButtonClicked( wxCommandEvent& aEvent );

    void onRemoveButtonClicked( wxCommandEvent& aEvent );

    void onCloseButtonClicked( wxCommandEvent& aEvent );

    void onScintillaCharAdded( wxStyledTextEvent& aEvent );

    void onSyntaxHelp( wxHyperlinkEvent& aEvent ) override;

    void onCheckSyntax( wxCommandEvent& aEvent ) override;

    void onErrorLinkClicked( wxHtmlLinkEvent& aEvent ) override;

    void onContextMenu( wxMouseEvent& aEvent ) override;

protected:
    wxButton*         btnSave;
    wxButton*         btnRemove;
    wxButton*         btnClose;
    wxComboCtrl*      m_comboCtrl;
    SCINTILLA_TRICKS* m_scintillaTricks;

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

    DRC_RE_LAYER_SELECTION_COMBO*                m_layerListCmbCtrl;
    DRC_RULE_EDITOR_CONSTRAINT_NAME              m_constraintType;
    DRC_RULE_EDITOR_CONTENT_PANEL_BASE*          m_constraintPanel;
    std::shared_ptr<DRC_RE_BASE_CONSTRAINT_DATA> m_constraintData;

    std::function<void( int aNodeId )>                     m_callBackSave;
    std::function<void( int aNodeId )>                     m_callBackRemove;
    std::function<void( int aNodeId )>                     m_callBackClose;
    std::function<bool( int aNodeId, wxString aRuleName )> m_callBackRuleNameValidation;

    wxRegEx m_netClassRegex;
    wxRegEx m_netNameRegex;
    wxRegEx m_typeRegex;
    wxRegEx m_viaTypeRegex;
    wxRegEx m_padTypeRegex;
    wxRegEx m_pinTypeRegex;
    wxRegEx m_fabPropRegex;
    wxRegEx m_shapeRegex;
    wxRegEx m_padShapeRegex;
    wxRegEx m_padConnectionsRegex;
    wxRegEx m_zoneConnStyleRegex;
    wxRegEx m_lineStyleRegex;
    wxRegEx m_hJustRegex;
    wxRegEx m_vJustRegex;

    HTML_MESSAGE_BOX* m_helpWindow;
};

#endif // PANEL_DRC_RULE_EDITOR_H
