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


#ifndef DIALOG_DRC_RULE_EDITOR_H
#define DIALOG_DRC_RULE_EDITOR_H

#include <dialogs/rule_editor_dialog_base.h>
#include "panel_drc_rule_editor.h"
#include "drc_rule_editor_utils.h"
#include "panel_drc_group_header.h"


class PCB_EDIT_FRAME;

class DIALOG_DRC_RULE_EDITOR : public RULE_EDITOR_DIALOG_BASE
{
public:
    DIALOG_DRC_RULE_EDITOR( PCB_EDIT_FRAME* aFrame );

    ~DIALOG_DRC_RULE_EDITOR();    

    bool TransferDataToWindow() override;

    bool TransferDataFromWindow() override;
  
    std::vector<RuleTreeNode> GetDefaultRuleTreeItems() override;

    void AddNewRule( RuleTreeItemData* aRuleTreeItemData ) override;

    void DuplicateRule( RuleTreeItemData* aRuleTreeItemData ) override;

    void RuleTreeItemSelectionChanged( RuleTreeItemData* aCurrentRuleTreeItemData ) override;

    void UpdateRuleTypeTreeItemData( RuleTreeItemData* aCurrentRuleTreeItemData ) override;   

    bool VerifyRuleTreeContextMenuOptionToEnable( RuleTreeItemData* aRuleTreeItemData,
                                                  RULE_EDITOR_TREE_CONTEXT_OPT aOption ) override;

    void RemoveRule( int aNodeId ) override;

private:
    std::vector<RuleTreeNode> buildElectricalRuleTreeNodes( int& aParentId );

    std::vector<RuleTreeNode> buildManufacturabilityRuleTreeNodes( int& aParentId );

    std::vector<RuleTreeNode> buildHighspeedDesignRuleTreeNodes( int& aParentId );

    std::vector<RuleTreeNode> buildFootprintsRuleTreeNodes( int& aParentId );

    RuleTreeNode              buildRuleTreeNode( RuleTreeItemData* aRuleTreeItemData );

    RuleTreeNode              buildRuleTreeNodeData(const std::string& aName, 
                                            const DRC_RULE_EDITOR_ITEM_TYPE& aNodeType,    
                                            const std::optional<int>& aParentId = std::nullopt,        
                                            const std::optional<DRC_RULE_EDITOR_CONSTRAINT_NAME>& aConstraintType = std::nullopt,
                                            const std::vector<RuleTreeNode>& aChildNodes = {}, 
                                            const std::optional<int>& aId = std::nullopt );

    RuleTreeNode*             getRuleTreeNodeInfo( const int& aNodeId );

    void saveRule( int aNodeId );

    void closeRuleEntryView( int aNodeId );

    bool validateRuleName( int aNodeId, wxString aRuleName );

    bool deleteTreeNodeData( const int& aNodeId );

protected:
    PCB_EDIT_FRAME* m_frame;

private:
    int m_nodeId;
    std::vector<RuleTreeNode> m_RuleTreeNodeDatas;
    PANEL_DRC_RULE_EDITOR*    m_ruleEditorPanel;
    PANEL_DRC_GROUP_HEADER*   m_groupHeaderPanel;
};

#endif //DIALOG_DRC_RULE_EDITOR_H
