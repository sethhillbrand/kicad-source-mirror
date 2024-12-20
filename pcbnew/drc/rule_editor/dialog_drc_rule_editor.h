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


class PCB_EDIT_FRAME;
class PANEL_DRC_RULE_EDITOR_CLEARENCE;


class DIALOG_DRC_RULE_EDITOR : public RULE_EDITOR_DIALOG_BASE
{
public:
    DIALOG_DRC_RULE_EDITOR( PCB_EDIT_FRAME* aFrame );

    ~DIALOG_DRC_RULE_EDITOR();    

    bool TransferDataToWindow() override;

    bool TransferDataFromWindow() override;
  
    std::vector<RuleTreeNode> GetDefaultTreeItems() override;

    void AddNewRule( RuleTreeItemData* aRuleTreeItemData ) override;

    void DuplicateRule( RuleTreeItemData* aRuleTreeItemData ) override;

    void TreeItemSelectionChanged( RuleTreeItemData* aCurrentRuleTreeItemData ) override;

    void UpdateRuleTypeTreeItemData( RuleTreeItemData* aCurrentRuleTreeItemData ) override;   

    bool CanShowContextMenu( RuleTreeNode* aRuleTreeNode ) override;

    bool CheckAndAppendRuleOperations( RuleTreeNode* aRuleTreeNode ) override;    

private:
    std::vector<RuleTreeNode> createElectricalItems( unsigned int& parentId );

    std::vector<RuleTreeNode> createManufacturabilityItems( unsigned int& parentId );

    std::vector<RuleTreeNode> createHighspeedDesignItems( unsigned int& parentId );

    std::vector<RuleTreeNode> createFootprintsItems( unsigned int& parentId );

    RuleTreeNode              buildNewTreeData( RuleTreeItemData* aRuleTreeItemData );

    void                      addRuleNodeToAConstraint( std::vector<RuleTreeNode>& aRuleTreeItemDatas,
                                                        const RuleTreeNode& aRuleTreeNode, 
                                                        const unsigned int& parentId );

    RuleTreeNode              buildTreeNode(const std::string& name, 
                                            const DRC_RULE_EDITOR_ITEM_TYPE& nodeType,    
                                            const std::vector<RuleTreeNode>& childNodes = {}, 
                                            const std::optional<unsigned int>& id = std::nullopt,
                                            const std::optional<unsigned int>& parentId = std::nullopt,
                                            const std::optional<DRC_RULE_EDITOR_CONSTRAINT_NAME>& constraintType = std::nullopt );

protected:
    PCB_EDIT_FRAME* m_frame;

private:
    std::vector<RuleTreeNode> m_RuleTreeNodes;
    PANEL_DRC_RULE_EDITOR*    m_parentPanel;
    unsigned int              m_MaxTreeItemId;
};

#endif //DIALOG_DRC_RULE_EDITOR_H
