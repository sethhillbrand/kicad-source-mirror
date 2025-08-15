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
                           wxString* aConstraintTitle,
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

    void SetShowMatchesCallBack(
            std::function<void( int aNodeId )> aCallBackShowMatches )
    {
        m_callBackShowMatches = aCallBackShowMatches;
    }

    bool GetIsValidationSucceeded() { return m_validationSucceeded; }

    std::string GetValidationMessage() { return m_validationMessage; }

    bool ValidateInputs( int* aErrorCount, std::string* aValidationMessage ) override;

    void Save( wxCommandEvent& aEvent );

    void Cancel( wxCommandEvent& aEvent );

private:
    DRC_RULE_EDITOR_CONTENT_PANEL_BASE* getConstraintPanel( wxWindow* aParent,
        const DRC_RULE_EDITOR_CONSTRAINT_NAME& aConstraintType );

    /**
     * Handles the save button click event, validating inputs and invoking the save callback if valid.
     *
     * @param aEvent The event triggered by the save button click.
     */
    void onSaveButtonClicked( wxCommandEvent& aEvent );

    /**
     * Handles the remove button click event, invoking the remove callback.
     *
     * @param aEvent The event triggered by the remove button click.
     */
    void onRemoveButtonClicked( wxCommandEvent& aEvent );

    /**
     * Handles the close button click event, invoking the close callback.
     *
     * @param aEvent The event triggered by the close button click.
     */
    void onCloseButtonClicked( wxCommandEvent& aEvent );

    /**
     * Handles character addition in the Scintilla text control, performing auto-complete and context-sensitive operations.
     *
     * @param aEvent The event triggered when a character is added.
     */
    void onScintillaCharAdded( wxStyledTextEvent& aEvent );

    /**
     * Displays a modeless help window with syntax and rule documentation.
     *
     * @param aEvent The event triggered by the hyperlink click.
     */
    void onSyntaxHelp( wxHyperlinkEvent& aEvent ) override;

    /**
     * Checks the syntax of the DRC rule condition and reports any errors.
     *
     * @param event The event triggered by the syntax check action.
     */
    void onCheckSyntax( wxCommandEvent& aEvent ) override;

    /**
     * Handles clicks on error links in the syntax error report and navigates to the error location.
     *
     * @param event The event triggered when an error link is clicked.
     */
    void onErrorLinkClicked( wxHtmlLinkEvent& aEvent ) override;

    /**
     * Handles right-click context menu actions for text editing (undo, redo, cut, copy, paste, delete, select all, zoom).
     *
     * @param event The event triggered by the right-click menu.
     */
    void onContextMenu( wxMouseEvent& aEvent ) override;

    void onShowMatchesButtonClicked( wxCommandEvent& aEvent );

private:
    wxButton*         m_btnSave;
    wxButton*         m_btnRemove;
    wxButton*         m_btnClose;
    wxButton*         m_btnShowMatches;
    wxComboCtrl*      m_comboCtrl;
    std::vector<int>  m_validLayers;
    LSEQ              m_layerList;
    BOARD*            m_board;
    wxString*         m_constraintTitle;
    bool              m_basicDetailValidated;
    bool              m_syntaxChecked;
    bool              m_isModified;
    bool              m_validationSucceeded;
    std::string       m_validationMessage;

    std::unique_ptr<SCINTILLA_TRICKS>            m_scintillaTricks;
    DRC_RE_LAYER_SELECTION_COMBO*                m_layerListCmbCtrl;
    DRC_RULE_EDITOR_CONSTRAINT_NAME              m_constraintType;
    DRC_RULE_EDITOR_CONTENT_PANEL_BASE*          m_constraintPanel;
    std::shared_ptr<DRC_RE_BASE_CONSTRAINT_DATA> m_constraintData;

    std::function<void( int aNodeId )>                     m_callBackSave;
    std::function<void( int aNodeId )>                     m_callBackRemove;
    std::function<void( int aNodeId )>                     m_callBackClose;
    std::function<bool( int aNodeId, wxString aRuleName )> m_callBackRuleNameValidation;
    std::function<void( int aNodeId )> m_callBackShowMatches;

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
