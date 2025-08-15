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

class WX_INFOBAR;

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

//class RuleTreeItemData : public wxTreeItemData
//{
//public:
//    explicit RuleTreeItemData( RuleTreeNode* aNodeData, wxTreeItemId aParentTreeItemId,
//                               wxTreeItemId aTreeItemId ) :
//            m_TreeItem( aNodeData ),
//            m_ParentTreeItemId( aParentTreeItemId ), 
//            m_TreeItemId( aTreeItemId )
//    {
//    }
//
//    explicit RuleTreeItemData( RuleTreeNode* aNodeData ) :
//            m_TreeItem( aNodeData ),
//            m_ParentTreeItemId( nullptr ), 
//            m_TreeItemId( nullptr )
//    {
//    }
//
//    RuleTreeNode* GetTreeItem() const { return m_TreeItem; }
//
//    wxTreeItemId  GetParentTreeItemId() const { return m_ParentTreeItemId; }
//
//    wxTreeItemId  GetTreeItemId() const { return m_TreeItemId; }
//
//    wxString GetNodeName() { return this->GetTreeItem()->node_name; }
//
//    void SyncNodeName() 
//    {  
//        GetTreeItem()->node_name = this->GetTreeItem()->node_data->GetRuleName();
//    }
//
//private:
//    RuleTreeNode* m_TreeItem;             
//    wxTreeItemId  m_ParentTreeItemId; 
//    wxTreeItemId  m_TreeItemId; 
//};


class RULE_EDITOR_DIALOG_BASE : public DIALOG_SHIM
{
public:

    RULE_EDITOR_DIALOG_BASE( wxWindow* aParent, const wxString& aTitle,
                         const wxSize& aInitialSize = wxDefaultSize );

    ~RULE_EDITOR_DIALOG_BASE() override;

    wxTreeCtrl* GetTreeCtrl() { return m_treeCtrl; }

    void SetModified() { m_modified = true; }

    static RULE_EDITOR_DIALOG_BASE* GetDialog( wxWindow* aWindow );

    virtual std::vector<RuleTreeNode> GetDefaultTreeItems() = 0;

    void InitTreeItems( wxTreeCtrl* aTreeCtrl, const std::vector<RuleTreeNode>& aRuleTreeNodes );

    void PopulateTreeCtrl( wxTreeCtrl* aTreeCtrl, const std::vector<RuleTreeNode>& aRuleTreeNodes,
                           const RuleTreeNode& aRuleTreeNode, wxTreeItemId aParentTreeItemId );

    wxPanel* GetContentPanel() { return m_contentPanel; }

    void SetContentPanel( wxPanel* aContentPanel );  

    void AppendNewTreeItem( wxTreeCtrl* aTreeCtrl, const RuleTreeNode& aRuleTreeNode, wxTreeItemId aParentTreeItemId );   

    virtual void AddNewRule( RuleTreeItemData* aRuleTreeItemData ) = 0;

    virtual void DuplicateRule( RuleTreeItemData* aRuleTreeItemData ) = 0;

    virtual void TreeItemSelectionChanged( RuleTreeItemData* aCurrentRuleTreeItemData ) = 0;

    virtual void UpdateRuleTypeTreeItemData( RuleTreeItemData* aRuleTreeItemData ) = 0;

    RuleTreeItemData* GetContextMenuActiveTreeItemData() { return m_contextMenuActiveTreeItemData; }

    RuleTreeItemData* GetCurrentlySelectedTreeItemData() { return m_selectedTreeItemData; }

    wxTreeItemId GetPreviouslySelectedTreeItemId() { return m_previouslySelectedTreeItemId; }

    void UpdateTreeItemText( wxTreeCtrl* aTreeCtrl, wxTreeItemId aItemId, wxString aItemText );

    virtual bool CanShowContextMenu( RuleTreeItemData* aRuleTreeItemData ) = 0;

    virtual bool CheckAndAppendRuleOperations( RuleTreeItemData* aRuleTreeItemData ) = 0;

    void SetSelectedItem( wxTreeItemId aTreeItemId );

    void SetControlsEnabled( bool aEnable );

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
    void OnFilterSearch( wxCommandEvent& aEvent );

    void SaveTreeState( const wxTreeItemId& item, const unsigned int& aNodeId=0 );

    bool FilterTree( const wxTreeItemId& item, const wxString& filter );

    void RestoreTree( const wxTreeItemId& parent, const unsigned int& aNodeId );    

    wxTreeItemId appendTreeItem( wxTreeCtrl* aTreeCtrl, const RuleTreeNode& aRuleTreeNode, wxTreeItemId aParentTreeItemId );  

    std::vector<RuleTreeNode> getAllChildrenByParentId( const std::vector<RuleTreeNode>& nodes, unsigned int parentId);

    void getChildNodes( const std::vector<RuleTreeNode>& nodes, int parentId, std::vector<RuleTreeNode>& result );

    DECLARE_EVENT_TABLE();

protected:
    wxTreeCtrl* m_treeCtrl;
    WX_INFOBAR* m_infoBar;
    wxPanel*    m_contentPanel;
    wxSizer*      m_contentSizer;
    wxSearchCtrl* m_filterSearch;
    wxTextCtrl*   m_filterText;

private:
    wxString                  m_title;
    wxBoxSizer*               m_buttonsSizer;
    std::vector<RuleTreeNode> m_defaultTreeItems;
    RuleTreeItemData*         m_contextMenuActiveTreeItemData;
    RuleTreeItemData*         m_selectedTreeItemData;
    wxTreeItemId              m_previouslySelectedTreeItemId;
    std::unordered_map<unsigned int, std::tuple<wxString, std::vector<unsigned int>, wxTreeItemId>> m_originalChildren;
};

#endif //RULE_EDITOR_DIALOG_BASE_H
