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

#include <drc/rule_editor/drc_re_base_constraint_data.h>
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
    unsigned int                           node_id;
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

    unsigned int GetNodeId() const { return m_nodeId; }

    wxTreeItemId GetParentTreeItemId() const { return m_ParentTreeItemId; }

    wxTreeItemId GetTreeItemId() const { return m_TreeItemId; }

private:
    unsigned int  m_nodeId;
    wxTreeItemId  m_ParentTreeItemId;
    wxTreeItemId  m_TreeItemId;
};


class RULE_EDITOR_DIALOG_BASE : public DIALOG_SHIM
{
public:

    RULE_EDITOR_DIALOG_BASE( wxWindow* aParent, const wxString& aTitle,
                         const wxSize& aInitialSize = wxDefaultSize );

    ~RULE_EDITOR_DIALOG_BASE() override;

    RuleTreeCtrl* GetTreeCtrl() { return m_treeCtrl; }

    void SetModified() { m_modified = true; }

    static RULE_EDITOR_DIALOG_BASE* GetDialog( wxWindow* aWindow );

    virtual std::vector<RuleTreeNode> GetDefaultTreeItems() = 0;

    void InitTreeItems( RuleTreeCtrl* aTreeCtrl, const std::vector<RuleTreeNode>& aRuleTreeNodes );

    void PopulateTreeCtrl( RuleTreeCtrl* aTreeCtrl, const std::vector<RuleTreeNode>& aRuleTreeNodes,
                           const RuleTreeNode& aRuleTreeNode, wxTreeItemId aParentTreeItemId );

    wxPanel* GetContentPanel() { return m_contentPanel; }

    void SetContentPanel( wxPanel* aContentPanel );  

    void AppendNewTreeItem( RuleTreeCtrl* aTreeCtrl, const RuleTreeNode& aRuleTreeNode, wxTreeItemId aParentTreeItemId );   

    virtual void AddNewRule( RuleTreeItemData* aRuleTreeItemData ) = 0;

    virtual void DuplicateRule( RuleTreeItemData* aRuleTreeItemData ) = 0;

    virtual void TreeItemSelectionChanged( RuleTreeItemData* aCurrentRuleTreeItemData ) = 0;

    virtual void UpdateRuleTypeTreeItemData( RuleTreeItemData* aRuleTreeItemData ) = 0;

    RuleTreeItemData* GetContextMenuActiveTreeItemData() { return m_contextMenuActiveTreeItemData; }

    RuleTreeItemData* GetCurrentlySelectedTreeItemData() { return m_selectedTreeItemData; }

    wxTreeItemId GetPreviouslySelectedTreeItemId() { return m_previouslySelectedTreeItemId; }

    void UpdateTreeItemText( RuleTreeCtrl* aTreeCtrl, wxTreeItemId aItemId, wxString aItemText );

    virtual bool VerifyTreeContextMenuOptionToEnable( RuleTreeItemData* aRuleTreeItemData,
                                                      RULE_EDITOR_TREE_CONTEXT_OPT aOption  ) = 0;

    void SetSelectedItem( wxTreeItemId aTreeItemId );

    void SetControlsEnabled( bool aEnable );

    void DeleteTreeItem( wxTreeItemId aItemId, const unsigned int& aNodeId );

protected:

    void finishInitialization();

    bool TransferDataToWindow() override;

    bool TransferDataFromWindow() override;

private:
    void onTreeItemRightClick( wxTreeEvent& aEvent );

    void onSelectionChanged( wxTreeEvent& aEvent );

    void onNewRule( wxCommandEvent& aEvent );

    void onDuplicateRule( wxCommandEvent& aEvent );

    void onDeleteRule( wxCommandEvent& aEvent );

    void onMoveUpRule( wxCommandEvent& aEvent );

    void onMoveDownRule( wxCommandEvent& aEvent );

    /**
     * Handle a change in the hotkey filter text.
     *
     * @param aEvent is the search event, used to get the search query.
     */
    void onFilterSearch( wxCommandEvent& aEvent );

    void saveTreeState( const wxTreeItemId& item, const unsigned int& aNodeId=0 );

    bool filterTree( const wxTreeItemId& item, const wxString& filter );

    void restoreTree( const wxTreeItemId& parent, const unsigned int& aNodeId );    

    wxTreeItemId appendTreeItem( RuleTreeCtrl* aTreeCtrl, const RuleTreeNode& aRuleTreeNode, wxTreeItemId aParentTreeItemId );  

    std::vector<RuleTreeNode> getAllChildrenByParentId( const std::vector<RuleTreeNode>& nodes, unsigned int parentId);

    void getChildNodes( const std::vector<RuleTreeNode>& nodes, int parentId, std::vector<RuleTreeNode>& result );

    void onTreeItemLeftDown( wxMouseEvent& event );

    void onTreeItemMouseMotion( wxMouseEvent& event );

    void onTreeItemLeftUp( wxMouseEvent& event );

    void moveChildrenOnTreeItemDrag( wxTreeCtrl* tree, wxTreeItemId srcItem,
                                     wxTreeItemId destItem );

    void updateTreeItemMoveOptionState();

    void updateTreeActionButtonsState( RuleTreeItemData* ruleTreeItemData );
    

    DECLARE_EVENT_TABLE();

protected:
    RuleTreeCtrl* m_treeCtrl;
    WX_INFOBAR* m_infoBar;
    wxPanel*    m_contentPanel;
    wxSizer*      m_contentSizer;
    wxSearchCtrl* m_filterSearch;
    wxTextCtrl*   m_filterText;
    wxBitmapButton* m_addRuleButton;
    wxBitmapButton* m_copyRuleButton;
    wxBitmapButton* m_moveTreeItemUpButton;
    wxBitmapButton* m_moveTreeItemDownButton;
    wxBitmapButton* m_deleteRuleButton;


private:
    wxString                  m_title;
    wxBoxSizer*               m_buttonsSizer;
    std::vector<RuleTreeNode> m_defaultTreeItems;
    RuleTreeItemData*         m_contextMenuActiveTreeItemData;
    RuleTreeItemData*         m_selectedTreeItemData;
    wxTreeItemId              m_previouslySelectedTreeItemId;
    std::unordered_map<unsigned int, std::tuple<wxString, std::vector<unsigned int>, wxTreeItemId>> m_originalChildren;
    wxTreeItemId m_draggedItem;
    wxPoint      m_startPos;
    wxDragImage* m_dragImage = nullptr;
    bool         m_isDragging = false; // Track drag state
    bool         m_enableMoveUp = false;
    bool         m_enableMoveDown = false;
    bool         m_enableAddRule = false;
    bool         m_enableDuplicateRule = false;
    bool         m_enableDeleteRule = false;
};

#endif //RULE_EDITOR_DIALOG_BASE_H
