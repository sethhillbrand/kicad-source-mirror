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


#ifndef RULE_EDITOR_DIALOG_BASE_H
#define RULE_EDITOR_DIALOG_BASE_H

#include <wx/treectrl.h>
#include <dialog_shim.h>
#include <variant>
#include <optional>
#include <wx/srchctrl.h>
#include <unordered_map>

#include <dialogs/rule_editor_base_data.h>
#include <dialogs/rule_tree_ctrl.h>

class WX_INFOBAR;

enum RULE_EDITOR_TREE_CONTEXT_OPT
{
    ADD_RULE = 0,
    DUPLICATE_RULE,
    DELETE_RULE,
    MOVE_UP,
    MOVE_DOWN,
};


struct RuleTreeNode
{
    int                                    node_id;
    wxString                               node_name;
    int                                    node_type;
    int                                    node_level;
    std::optional<int>                     node_type_map;
    std::vector<RuleTreeNode>              child_nodes;
    std::shared_ptr<RuleEditorBaseData>    node_data;
};


class RuleTreeItemData : public wxTreeItemData
{
public:
    explicit RuleTreeItemData( int aNodeId, wxTreeItemId aParentTreeItemId, wxTreeItemId aTreeItemId ) :
            m_nodeId( aNodeId ),
            m_ParentTreeItemId( aParentTreeItemId ), 
            m_TreeItemId( aTreeItemId )
    {
    }

    int GetNodeId() const { return m_nodeId; }

    wxTreeItemId GetParentTreeItemId() const { return m_ParentTreeItemId; }

    void SetParentTreeItemId( wxTreeItemId aParentTreeItemId ) { m_ParentTreeItemId = aParentTreeItemId; }

    wxTreeItemId GetTreeItemId() const { return m_TreeItemId; }

    void SetTreeItemId( wxTreeItemId aTreeItemId ) { m_TreeItemId = aTreeItemId; }

private:
    int  m_nodeId;
    wxTreeItemId  m_ParentTreeItemId;
    wxTreeItemId  m_TreeItemId;
};


class RULE_EDITOR_DIALOG_BASE : public DIALOG_SHIM
{
public:

    RULE_EDITOR_DIALOG_BASE( wxWindow* aParent, const wxString& aTitle,
                         const wxSize& aInitialSize = wxDefaultSize );

    ~RULE_EDITOR_DIALOG_BASE() override;

    enum RULE_EDITOR_TREE_CONTEXT_ID
    {
        ID_NEWRULE = wxID_HIGHEST + 1,
        ID_COPYRULE,
        ID_DELETERULE,
        ID_MOVEUP,
        ID_MOVEDOWN
    };

    RuleTreeCtrl* GetRuleTreeCtrl() { return m_ruleTreeCtrl; }

    void SetModified() { m_modified = true; }

    static RULE_EDITOR_DIALOG_BASE* GetDialog( wxWindow* aWindow );

    virtual std::vector<RuleTreeNode> GetDefaultRuleTreeItems() = 0;

    virtual void AddNewRule( RuleTreeItemData* aRuleTreeItemData ) = 0;

    virtual void DuplicateRule( RuleTreeItemData* aRuleTreeItemData ) = 0;

    virtual void RemoveRule( int aNodeId ) = 0;

    virtual void RuleTreeItemSelectionChanged( RuleTreeItemData* aCurrentRuleTreeItemData ) = 0;

    virtual void UpdateRuleTypeTreeItemData( RuleTreeItemData* aRuleTreeItemData ) = 0;

    virtual bool VerifyRuleTreeContextMenuOptionToEnable( RuleTreeItemData* aRuleTreeItemData,
                                                         RULE_EDITOR_TREE_CONTEXT_OPT aOption ) = 0;

    void InitRuleTreeItems( const std::vector<RuleTreeNode>& aRuleTreeNodes );

    wxPanel* GetContentPanel() { return m_contentPanel; }

    void SetContentPanel( wxPanel* aContentPanel );  

    void AppendNewRuleTreeItem( const RuleTreeNode& aRuleTreeNode, wxTreeItemId aParentTreeItemId );   

    RuleTreeItemData* GetCurrentlySelectedRuleTreeItemData() { return m_selectedTreeItemData; }

    wxTreeItemId GetPreviouslySelectedRuleTreeItemId() { return m_previouslySelectedTreeItemId; }

    void UpdateRuleTreeItemText( wxTreeItemId aItemId, wxString aItemText );

    void SetControlsEnabled( bool aEnable );

    void DeleteRuleTreeItem( wxTreeItemId aItemId, const int& aNodeId );    

protected:

    void finishInitialization();

    bool TransferDataToWindow() override;

    bool TransferDataFromWindow() override;

private:
    void populateRuleTreeCtrl( const std::vector<RuleTreeNode>& aRuleTreeNodes,
                               const RuleTreeNode& aRuleTreeNode, wxTreeItemId aParentTreeItemId );

    void onRuleTreeItemRightClick( wxTreeEvent& aEvent );

    void onRuleTreeItemSelectionChanged( wxTreeEvent& aEvent );

    void onNewRuleOptionClick( wxCommandEvent& aEvent );

    void onDuplicateRuleOptionClick( wxCommandEvent& aEvent );

    void onDeleteRuleOptionClick( wxCommandEvent& aEvent );

    void onMoveUpRuleOptionClick( wxCommandEvent& aEvent );

    void onMoveDownRuleOptionClick( wxCommandEvent& aEvent );

    void onRuleTreeItemLeftDown( wxMouseEvent& aEvent );

    void onRuleTreeItemMouseMotion( wxMouseEvent& aEvent );

    void onRuleTreeItemLeftUp( wxMouseEvent& aEvent );

    /**
     * Handle a change in the hotkey filter text.
     *
     * @param aEvent is the search event, used to get the search query.
     */
    void onFilterSearch( wxCommandEvent& aEvent );

    bool filterRuleTree( const wxTreeItemId& aItem, const wxString& aFilter );

    void saveRuleTreeState( const wxTreeItemId& aItem, const int& aNodeId = 0 );

    void restoreRuleTree( const wxTreeItemId& aParent, const int& aNodeId );    

    wxTreeItemId appendRuleTreeItem( const RuleTreeNode& aRuleTreeNode, wxTreeItemId aParentTreeItemId ); 

    void getRuleTreeChildNodes( const std::vector<RuleTreeNode>& aNodes, int aParentId, std::vector<RuleTreeNode>& aResult );

    void moveRuleTreeItemChildrensTooOnDrag( wxTreeItemId aSrcTreeItemId, wxTreeItemId aDestTreeItemId );

    void updateRuleTreeItemMoveOptionState();

    void updateRuleTreeActionButtonsState( RuleTreeItemData* aRuleTreeItemData );

protected:
    RuleTreeCtrl*   m_ruleTreeCtrl;
    WX_INFOBAR*     m_infoBar;
    wxPanel*        m_contentPanel;
    wxSizer*        m_contentSizer;
    wxSearchCtrl*   m_filterSearch;
    wxTextCtrl*     m_filterText;
    wxBitmapButton* m_addRuleButton;
    wxBitmapButton* m_copyRuleButton;
    wxBitmapButton* m_moveTreeItemUpButton;
    wxBitmapButton* m_moveTreeItemDownButton;
    wxBitmapButton* m_deleteRuleButton;

private:
    bool m_isDragging { false };
    bool m_enableMoveUp { false };
    bool m_enableMoveDown { false };
    bool m_enableAddRule { false };
    bool m_enableDuplicateRule { false };
    bool m_enableDeleteRule { false };
    bool m_preventSelectionChange { false };

    wxString                  m_title;
    wxBoxSizer*               m_buttonsSizer;
    std::vector<RuleTreeNode> m_defaultTreeItems;
    RuleTreeItemData*         m_selectedTreeItemData;
    wxTreeItemId              m_previouslySelectedTreeItemId;
    wxTreeItemId              m_draggedItem;
    wxPoint                   m_startPos;
    wxDragImage*              m_dragImage{ nullptr };

    std::unordered_map<int, std::tuple<wxString, std::vector<int>, wxTreeItemId>> m_treeHistoryData;
};

#endif //RULE_EDITOR_DIALOG_BASE_H
