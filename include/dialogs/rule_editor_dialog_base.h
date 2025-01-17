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
#include <wx/srchctrl.h>
#include <wx/bmpbuttn.h>

#include <variant>
#include <optional>
#include <unordered_map>

#include <dialog_shim.h>
#include <dialogs/rule_editor_data_base.h>

class WX_INFOBAR;

/**
 * Enumeration representing the available context menu options for the rule editor tree.
 */
enum RULE_EDITOR_TREE_CONTEXT_OPT
{
    ADD_RULE = 0,
    DUPLICATE_RULE,
    DELETE_RULE,
    MOVE_UP,
    MOVE_DOWN,
};

/**
 * Structure representing a node in a rule tree, collection of this used for building the rule tree.
 */
struct RULE_TREE_NODE
{
    int                                    m_nodeId;
    wxString                               m_nodeName;
    int                                    m_nodeType;
    int                                    m_nodeLevel;
    std::optional<int>                     m_nodeTypeMap;
    std::vector<RULE_TREE_NODE>            m_childNodes;
    std::shared_ptr<RULE_EDITOR_DATA_BASE> m_nodeData;
};


/**
 * A class representing additional data associated with a wxTree item.
 * This class is used to store and manage metadata for individual items in a wxTree,
 * linking each tree item to its corresponding node ID and parent tree item.
 */
class RULE_TREE_ITEM_DATA : public wxTreeItemData
{
public:
    explicit RULE_TREE_ITEM_DATA( int aNodeId, wxTreeItemId aParentTreeItemId,
                                  wxTreeItemId aTreeItemId ) :
            m_nodeId( aNodeId ), m_parentTreeItemId( aParentTreeItemId ),
            m_treeItemId( aTreeItemId )
    {
    }

    int GetNodeId() const { return m_nodeId; }

    wxTreeItemId GetParentTreeItemId() const { return m_parentTreeItemId; }

    void SetParentTreeItemId( wxTreeItemId aParentTreeItemId )
    {
        m_parentTreeItemId = aParentTreeItemId;
    }

    wxTreeItemId GetTreeItemId() const { return m_treeItemId; }

    void SetTreeItemId( wxTreeItemId aTreeItemId ) { m_treeItemId = aTreeItemId; }

private:
    int          m_nodeId;
    wxTreeItemId m_treeItemId;
    wxTreeItemId m_parentTreeItemId;
};


class RULE_EDITOR_DIALOG_BASE : public DIALOG_SHIM
{
public:
    RULE_EDITOR_DIALOG_BASE( wxWindow* aParent, const wxString& aTitle,
                             const wxSize& aInitialSize = wxDefaultSize );

    ~RULE_EDITOR_DIALOG_BASE() override;

    /**
     * Enumeration representing the available context menu options for the rule editor tree.
     */
    enum RULE_EDITOR_TREE_CONTEXT_OPT_ID
    {
        ID_NEWRULE = wxID_HIGHEST + 1,
        ID_COPYRULE,
        ID_DELETERULE,
        ID_MOVEUP,
        ID_MOVEDOWN
    };

    /**
     * Gets the tree control used for displaying and managing rules.
     * 
     * @return A pointer to the wxTreeCtrl instance.
     */
    wxTreeCtrl* GetRuleTreeCtrl() { return m_ruleTreeCtrl; }

    /**
     * Marks the dialog as modified, typically used to indicate unsaved changes.
     */
    void SetModified() { m_modified = true; }

    /**
     * Static method to retrieve the rule editor dialog instance associated with a given window.
     * 
     * @param aWindow The window for which the dialog is being retrieved.
     * 
     * @return A pointer to the RULE_EDITOR_DIALOG_BASE instance, or nullptr if not found.
     */
    static RULE_EDITOR_DIALOG_BASE* GetDialog( wxWindow* aWindow );

    /**
     * Pure virtual method to get the default rule tree items.
     * Must be implemented by derived classes.
     * 
     * @return A vector of default RULE_TREE_NODE items.
     */
    virtual std::vector<RULE_TREE_NODE> GetDefaultRuleTreeItems() = 0;

    /**
     * Pure virtual method to add a new rule to the tree.
     * Must be implemented by derived classes.
     * 
     * @param aRuleTreeItemData The data associated with the new rule.
     */
    virtual void AddNewRule( RULE_TREE_ITEM_DATA* aRuleTreeItemData ) = 0;

    /**
     * Pure virtual method to duplicate an existing rule in the tree.
     * Must be implemented by derived classes.
     * 
     * @param aRuleTreeItemData The data of the rule to duplicate.
     */
    virtual void DuplicateRule( RULE_TREE_ITEM_DATA* aRuleTreeItemData ) = 0;

    /**
     * Pure virtual method to remove a rule from the tree.
     * Must be implemented by derived classes.
     * 
     * @param aNodeId The ID of the rule node to remove.
     */
    virtual void RemoveRule( int aNodeId ) = 0;

    /**
     * Pure virtual method to handle tree item selection changes.
     * Must be implemented by derived classes.
     * 
     * @param aCurrentRuleTreeItemData The data of the newly selected rule tree item.
     */
    virtual void RuleTreeItemSelectionChanged( RULE_TREE_ITEM_DATA* aCurrentRuleTreeItemData ) = 0;

    /**
     * Pure virtual method to update the rule tree item data.
     * Must be implemented by derived classes.
     * 
     * @param aRuleTreeItemData The data of the rule tree item to be updated.
     */
    virtual void UpdateRuleTypeTreeItemData( RULE_TREE_ITEM_DATA* aRuleTreeItemData ) = 0;

    /**
     * Pure virtual method to verify if a context menu option for a rule tree item should be enabled.
     * Must be implemented by derived classes.
     * 
     * @param aRuleTreeItemData The data of the rule tree item to check.
     * @param aOption The context menu option to verify.
     * 
     * @return true if the option should be enabled, false otherwise.
     */
    virtual bool VerifyRuleTreeContextMenuOptionToEnable( RULE_TREE_ITEM_DATA* aRuleTreeItemData,
                                                          RULE_EDITOR_TREE_CONTEXT_OPT aOption ) = 0;

    /**
     * Initializes the rule tree items by adding nodes and setting up the tree structure.
     * 
     * @param aRuleTreeNodes A vector of rule tree nodes to initialize.
     */
    void InitRuleTreeItems( const std::vector<RULE_TREE_NODE>& aRuleTreeNodes );

    /**
     * Retrieves the current content panel.
     * 
     * @return A pointer to the current content panel.
     */
    wxPanel* GetContentPanel() { return m_contentPanel; }

    /**
     * Sets a new content panel for the dialog.
     * 
     * @param aContentPanel A pointer to the new content panel to set.
     */
    void SetContentPanel( wxPanel* aContentPanel );

    /**
     * Appends a new rule tree item under a specified parent in the tree.
     * 
     * @param aRuleTreeNode The rule tree node to add.
     * @param aParentTreeItemId The parent tree item under which the new item will be added.
     */
    void AppendNewRuleTreeItem( const RULE_TREE_NODE& aRuleTreeNode,
                                wxTreeItemId          aParentTreeItemId );

    /**
     * Retrieves the currently selected rule tree item data.
     * 
     * @return A pointer to the currently selected rule tree item data.
     */
    RULE_TREE_ITEM_DATA* GetCurrentlySelectedRuleTreeItemData() { return m_selectedTreeItemData; }

    /**
     * Retrieves the previously selected rule tree item ID.
     * 
     * @return The ID of the previously selected rule tree item.
     */
    wxTreeItemId GetPreviouslySelectedRuleTreeItemId() { return m_previouslySelectedTreeItemId; }

    /**
     * Updates the text of a specified rule tree item.
     * 
     * @param aItemId The ID of the tree item to update.
     * @param aItemText The new text to set for the tree item.
     */
    void UpdateRuleTreeItemText( wxTreeItemId aItemId, wxString aItemText );

    /**
     * Enables or disables controls within the rule editor dialog.
     * 
     * @param aEnable true to enable the controls, false to disable them.
     */
    void SetControlsEnabled( bool aEnable );

    /**
     * Deletes a specified rule tree item and removes it from the history data.
     * 
     * @param aItemId The ID of the tree item to delete.
     * @param aNodeId The node ID associated with the tree item to delete.
     */
    void DeleteRuleTreeItem( wxTreeItemId aItemId, const int& aNodeId );

protected:
    void finishInitialization();

    bool TransferDataToWindow() override;

    bool TransferDataFromWindow() override;

private:
    /**
     * Populates the rule tree control with nodes.
     * 
     * @param aRuleTreeNodes A list of rule tree nodes to populate.
     * @param aRuleTreeNode The current rule tree node to be processed.
     * @param aParentTreeItemId The parent tree item ID to attach new nodes to.
     */
    void populateRuleTreeCtrl( const std::vector<RULE_TREE_NODE>& aRuleTreeNodes,
                               const RULE_TREE_NODE&              aRuleTreeNode,
                               wxTreeItemId                       aParentTreeItemId );

    /**
     * Handles right-click events on rule tree items.
     * 
     * @param aEvent The tree event triggered by the right-click action.
     */
    void onRuleTreeItemRightClick( wxTreeEvent& aEvent );

    /**
     * Handles the event triggered when a rule tree item selection changes.
     * 
     * @param aEvent The tree event triggered by the selection change.
     */
    void onRuleTreeItemSelectionChanged( wxTreeEvent& aEvent );

    /**
     * Handles the event triggered when the "New Rule" option is clicked.
     * 
     * @param aEvent The command event triggered by the click.
     */
    void onNewRuleOptionClick( wxCommandEvent& aEvent );

    /**
     * Handles the event triggered when the "Duplicate Rule" option is clicked.
     * 
     * @param aEvent The command event triggered by the click.
     */
    void onDuplicateRuleOptionClick( wxCommandEvent& aEvent );

    /**
     * Handles the event triggered when the "Delete Rule" option is clicked.
     * 
     * @param aEvent The command event triggered by the click.
     */
    void onDeleteRuleOptionClick( wxCommandEvent& aEvent );

    /**
     * Handles the event triggered when the "Move Up Rule" option is clicked.
     * 
     * @param aEvent The command event triggered by the click.
     */
    void onMoveUpRuleOptionClick( wxCommandEvent& aEvent );

    /**
     * Handles the event triggered when the "Move Down Rule" option is clicked.
     * 
     * @param aEvent The command event triggered by the click.
     */
    void onMoveDownRuleOptionClick( wxCommandEvent& aEvent );

    /**
     * Handles the event triggered when a tree item is left-clicked.
     * 
     * @param aEvent The mouse event triggered by the left-click.
     */
    void onRuleTreeItemLeftDown( wxMouseEvent& aEvent );

    /**
     * Handles the event triggered when the mouse moves over a rule tree item.
     * 
     * @param aEvent The mouse event triggered by the mouse motion.
     */
    void onRuleTreeItemMouseMotion( wxMouseEvent& aEvent );

    /**
     * Handles the event triggered when a left-click on a rule tree item is released.
     * 
     * @param aEvent The mouse event triggered by the left-click release.
     */
    void onRuleTreeItemLeftUp( wxMouseEvent& aEvent );

    /**
     * Handle a change in the hotkey filter text.
     *
     * @param aEvent is the search event, used to get the search query.
     */
    void onFilterSearch( wxCommandEvent& aEvent );

    /**
     * Filters the rule tree based on the provided filter string.
     * 
     * @param aItem The tree item to filter.
     * @param aFilter The filter string used to determine visibility.
     * @return true if the item matches the filter, false otherwise.
     */
    bool filterRuleTree( const wxTreeItemId& aItem, const wxString& aFilter );

    /**
     * Saves the current state of the rule tree to a persistent location.
     * 
     * @param aItem The tree item to save the state of.
     * @param aNodeId The ID of the node to associate with the saved state.
     */
    void saveRuleTreeState( const wxTreeItemId& aItem, const int& aNodeId = 0 );

    /**
     * Restores the rule tree structure from a saved state.
     * 
     * @param aParent The parent tree item to attach restored nodes to.
     * @param aNodeId The ID of the node to restore the tree from.
     */
    void restoreRuleTree( const wxTreeItemId& aParent, const int& aNodeId );

    /**
     * Appends a new rule tree item to the control.
     * 
     * @param aRuleTreeNode The rule tree node data for the item.
     * @param aParentTreeItemId The parent tree item ID to append the item to.
     * @return The ID of the newly appended tree item.
     */

    wxTreeItemId appendRuleTreeItem( const RULE_TREE_NODE& aRuleTreeNode,
                                     wxTreeItemId          aParentTreeItemId );

    /**
     * Retrieves child nodes of the rule tree for the specified parent node.
     * 
     * @param aNodes The list of rule tree nodes.
     * @param aParentId The parent node ID.
     * @param aResult The list to store the resulting child nodes.
     */
    void getRuleTreeChildNodes( const std::vector<RULE_TREE_NODE>& aNodes, int aParentId,
                                std::vector<RULE_TREE_NODE>& aResult );

    /**
     * Moves the child items of a tree item when it is dragged.
     * 
     * @param aSrcTreeItemId The source tree item ID.
     * @param aDestTreeItemId The destination tree item ID.
     */
    void moveRuleTreeItemChildrensTooOnDrag( wxTreeItemId aSrcTreeItemId,
                                             wxTreeItemId aDestTreeItemId );

    /**
     * Updates the state of the rule tree move options based on the current selection.
     */
    void updateRuleTreeItemMoveOptionState();

    /**
     * Updates the state of the rule tree action buttons based on the selected item data.
     * 
     * @param aRuleTreeItemData The rule tree item data to determine the button states.
     */
    void updateRuleTreeActionButtonsState( RULE_TREE_ITEM_DATA* aRuleTreeItemData );

protected:
    wxTreeCtrl*     m_ruleTreeCtrl;
    WX_INFOBAR*     m_infoBar;
    wxPanel*        m_contentPanel;
    wxSizer*        m_contentSizer;
    wxSearchCtrl*   m_filterSearch;
    wxTextCtrl*     m_filterText;
    wxBoxSizer*     m_buttonsSizer;
    wxBitmapButton* m_addRuleButton;
    wxBitmapButton* m_copyRuleButton;
    wxBitmapButton* m_moveTreeItemUpButton;
    wxBitmapButton* m_moveTreeItemDownButton;
    wxBitmapButton* m_deleteRuleButton;

private:
    bool m_isDragging;
    bool m_enableMoveUp;
    bool m_enableMoveDown;
    bool m_enableAddRule;
    bool m_enableDuplicateRule;
    bool m_enableDeleteRule;
    bool m_preventSelectionChange;

    wxString                    m_title;
    std::vector<RULE_TREE_NODE> m_defaultTreeItems;
    RULE_TREE_ITEM_DATA*        m_selectedTreeItemData;
    wxTreeItemId                m_previouslySelectedTreeItemId;
    wxTreeItemId                m_draggedItem;
    wxPoint                     m_draggedItemStartPos;
    wxDragImage*                m_dragImage;

    std::unordered_map<int, std::tuple<wxString, std::vector<int>, wxTreeItemId>> m_treeHistoryData;
};

#endif //RULE_EDITOR_DIALOG_BASE_H
