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

#include <confirm.h>
#include <pcb_edit_frame.h>
#include <kiface_base.h>
#include <widgets/wx_progress_reporters.h>

#include "dialog_drc_rule_editor.h"
#include "panel_drc_rule_editor.h"
#include "drc_rule_editor_enums.h"
#include "drc_re_numeric_input_constraint_data.h"


DIALOG_DRC_RULE_EDITOR::DIALOG_DRC_RULE_EDITOR( PCB_EDIT_FRAME* aFrame ) :
        RULE_EDITOR_DIALOG_BASE( aFrame, _( "Design Rule Builder" ),
                                 aFrame->FromDIP( wxSize( 980, 600 ) ) ),
        m_frame( aFrame ), 
        m_parentPanel( nullptr ), 
        m_MaxTreeItemId( 0 )
{
    m_treeCtrl->DeleteAllItems();

    m_RuleTreeNodes = GetDefaultTreeItems();

    InitTreeItems( m_treeCtrl, m_RuleTreeNodes );

    finishDialogSettings();

    if( Prj().IsReadOnly() )
    {
        m_infoBar->ShowMessage( _( "Project is missing or read-only. Settings will not be "
                                   "editable." ),
                                wxICON_WARNING );
    }    
}


DIALOG_DRC_RULE_EDITOR::~DIALOG_DRC_RULE_EDITOR()
{
}


RuleTreeNode DIALOG_DRC_RULE_EDITOR::buildTreeNode( const std::string& name, 
                                                    const DRC_RULE_EDITOR_ITEM_TYPE& nodeType,    
                                                    const std::vector<RuleTreeNode>& childNodes, 
                                                    const std::optional<unsigned int>& id,
                                                    const std::optional<unsigned int>& parentId,
                                                    const std::optional<DRC_RULE_EDITOR_CONSTRAINT_NAME>& constraintType )
{
    unsigned int newId;

    if( id )
    {
        newId = *id; // Use provided ID
    }
    else
    {
        newId = 1;

        if( m_MaxTreeItemId )
            newId = m_MaxTreeItemId + 1;
    }

    m_MaxTreeItemId = newId;

    RuleEditorBaseData baseData;
    baseData.SetId( newId );

    if( parentId )
    {
        baseData.SetParentId( *parentId );
    }

    return { 
             .node_name = name, .node_type = nodeType, .node_type_map = constraintType, .child_nodes = childNodes, 
             .node_data = std::make_shared<RuleEditorBaseData>( baseData ) 
           };
}


std::vector<RuleTreeNode> DIALOG_DRC_RULE_EDITOR::GetDefaultTreeItems()
{ 
    std::vector<RuleTreeNode> result =
    {
        buildTreeNode( "Design Rules", DRC_RULE_EDITOR_ITEM_TYPE::ROOT,
        {
            buildTreeNode( "Electrical", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY, createElectricalItems( m_MaxTreeItemId ) ),
            buildTreeNode( "Manufacturability", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY, createManufacturabilityItems( m_MaxTreeItemId ) ),
            buildTreeNode( "Highspeed design", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY, createHighspeedDesignItems( m_MaxTreeItemId ) ),
            buildTreeNode( "Footprints", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY, createFootprintsItems( m_MaxTreeItemId ) )
        } )
    };
    
    return result;
}

std::vector<RuleTreeNode> DIALOG_DRC_RULE_EDITOR::createElectricalItems( unsigned int& parentId )
{
    std::vector<RuleTreeNode> result =  
    {
        buildTreeNode( "Clearance", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY, 
        {
            buildTreeNode( "Basic clearance", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, {}, std::nullopt, m_MaxTreeItemId, BASIC_CLEARANCE ),
            buildTreeNode( "Board outline clearance", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, {}, std::nullopt, m_MaxTreeItemId,  BOARD_OUTLINE_CLEARANCE ),
            buildTreeNode( "Minimum clearance", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, {}, std::nullopt, m_MaxTreeItemId,  MINIMUM_CLEARANCE ),
            buildTreeNode( "Minimum item clearance", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, {}, std::nullopt, m_MaxTreeItemId,  MINIMUM_ITEM_CLEARANCE ),
            buildTreeNode( "Net antenna", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, {}, std::nullopt, m_MaxTreeItemId,  NET_ANTENNA ),
            buildTreeNode( "Short circuit", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, {}, std::nullopt, m_MaxTreeItemId,  SHORT_CIRCUIT ),
            buildTreeNode( "Creepage distance", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, {}, std::nullopt, m_MaxTreeItemId,  CREEPAGE_DISTANCE ),
        }, std::nullopt, parentId ),
        buildTreeNode( "Connection Width",  DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY, 
        {
            buildTreeNode( "Minimum connection width", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, {}, std::nullopt, m_MaxTreeItemId,  MINIMUM_CONNECTION_WIDTH ),
            buildTreeNode( "Minimum track width", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, {}, std::nullopt, m_MaxTreeItemId,  MINIMUM_TRACK_WIDTH ),
            buildTreeNode( "Unrouted", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, {}, std::nullopt, m_MaxTreeItemId,  UNROUTED ),
        }, std::nullopt, parentId ),
        buildTreeNode("Hole Clearance",  DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY, 
        {
            buildTreeNode( "Copper to hole clearance", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, {}, std::nullopt, m_MaxTreeItemId,  COPPER_TO_HOLE_CLEARANCE ),
            buildTreeNode( "Hole to hole clearance", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, {}, std::nullopt, m_MaxTreeItemId,  HOLE_TO_HOLE_CLEARANCE ),
        }, std::nullopt, parentId ),
        buildTreeNode( "Spoke Count",  DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY,
        {
            buildTreeNode( "Minimum thermal relief spoke count", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, {}, std::nullopt, m_MaxTreeItemId,  MINIMUM_THERMAL_RELIEF_SPOKE_COUNT ),
        }, std::nullopt, parentId ),
        buildTreeNode( "Zone Connection",  DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY, 
        {
            buildTreeNode( "Allow fillets outside zone outline", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, {}, std::nullopt, m_MaxTreeItemId,  ALLOW_FILLET_OUTSIDE_ZONE_OUTLINE ),
        }, std::nullopt, parentId ),
    };

    return result;
}


std::vector<RuleTreeNode> DIALOG_DRC_RULE_EDITOR::createManufacturabilityItems( unsigned int& parentId )
{
     std::vector<RuleTreeNode> result = 
     {
        buildTreeNode( "Annular Width", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY,
        {
            buildTreeNode( "Minimum annular width", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, {}, std::nullopt, m_MaxTreeItemId,  MINIMUM_ANNULAR_WIDTH ),
        }, std::nullopt, parentId ),
        buildTreeNode( "Courtyard Clearance", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY, {}, std::nullopt, parentId ),
        buildTreeNode( "Edge Clearance", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY,
        {
            buildTreeNode( "Copper to edge clearance", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, {}, std::nullopt, m_MaxTreeItemId,  COPPER_TO_EDGE_CLEARANCE ),
        }, std::nullopt, parentId ),
        buildTreeNode( "Physical Clearance", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY, {}, std::nullopt, parentId ),
        buildTreeNode( "Hole Size", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY,
        {
            buildTreeNode( "Minimum through hole", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, {}, std::nullopt, m_MaxTreeItemId,  MINIMUM_THROUGH_HOLE ),
            buildTreeNode( "Hole size", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, {}, std::nullopt, m_MaxTreeItemId,  HOLE_SIZE ),
            buildTreeNode( "Hole to hole distance", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, {}, std::nullopt, m_MaxTreeItemId,  HOLE_TO_HOLE_DISTANCE ),
            buildTreeNode( "Minimum uvia hole", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, {}, std::nullopt, m_MaxTreeItemId,  MINIMUM_UVIA_HOLE ),
            buildTreeNode( "Minimum uvia diameter", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, {}, std::nullopt, m_MaxTreeItemId,  MINIMUM_UVIA_DIAMETER ),
            buildTreeNode( "Minimum via diameter", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, {}, std::nullopt, m_MaxTreeItemId,  MINIMUM_VIA_DIAMETER ),
            buildTreeNode( "Via style", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, {}, std::nullopt, m_MaxTreeItemId,  VIA_STYLE ),
        }, std::nullopt, parentId ),
        buildTreeNode( "Text Geometry", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY,
        {
            buildTreeNode( "Minimum text height and thickness",DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, {}, std::nullopt, m_MaxTreeItemId,  MINIMUM_TEXT_HEIGHT_AND_THICKNESS ),
        }, std::nullopt, parentId ),
        buildTreeNode( "Silkscreen Clearance", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY,
        {
            buildTreeNode( "Silk to silk clearance", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, {}, std::nullopt, m_MaxTreeItemId,  SILK_TO_SILK_CLEARANCE ),
            buildTreeNode( "Silk to soldermask clearance", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, {}, std::nullopt, m_MaxTreeItemId,  SILK_TO_SOLDERMASK_CLEARANCE ),
        }, std::nullopt, parentId ),
        buildTreeNode( "Soldermask", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY,
        {
            buildTreeNode( "Minimum soldermask silver", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, {}, std::nullopt, m_MaxTreeItemId,  MINIMUM_SOLDERMASK_SILVER ),
            buildTreeNode( "Soldermask expansion", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, {}, std::nullopt, m_MaxTreeItemId,  SOLDERMASK_EXPANSION ),
        }, std::nullopt, parentId ),
        buildTreeNode( "Solderpaste", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY,
        {
            buildTreeNode( "Solderpaste expansion", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, {}, std::nullopt, m_MaxTreeItemId,  SOLDERPASTE_EXPANSION ),
        }, std::nullopt, parentId ),
        buildTreeNode( "Deviation", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY,
        {
            buildTreeNode( "Maximum allowed deviation", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, {}, std::nullopt, m_MaxTreeItemId,  MAXIMUM_ALLOWED_DEVIATION ),
        }, std::nullopt, parentId ),
        buildTreeNode( "Angles", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY,
        {
            buildTreeNode( "Minimum acute angle", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, {}, std::nullopt, m_MaxTreeItemId,  MINIMUM_ACUTE_ANGLE ),
            buildTreeNode( "Minimum angular ring", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, {}, std::nullopt, m_MaxTreeItemId,  MINIMUM_ANGULAR_RING ),
        }, std::nullopt, parentId ),
    };

     return result;
}


std::vector<RuleTreeNode> DIALOG_DRC_RULE_EDITOR::createHighspeedDesignItems( unsigned int& parentId )
{
    std::vector<RuleTreeNode> result = 
    {
        buildTreeNode( "Diff Pair (width, gap, uncoupled length)", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY,
        {
            buildTreeNode( "Routing diff pair", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, {}, std::nullopt, m_MaxTreeItemId,  ROUTING_DIFF_PAIR ),
            buildTreeNode( "Routing width", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, {}, std::nullopt, m_MaxTreeItemId,  ROUTING_WIDTH ),
            buildTreeNode( "Maximum via count", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, {}, std::nullopt, m_MaxTreeItemId,  MAXIMUM_VIA_COUNT ),
        }, std::nullopt, parentId ),
        buildTreeNode( "Skew", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY ),
        buildTreeNode( "Length Matching", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY,
        {
            buildTreeNode( "Matched length diff pair", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, {}, std::nullopt, m_MaxTreeItemId,  MATCHED_LENGTH_DIFF_PAIR ),
            buildTreeNode( "Matched length all traces in group",DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, {}, std::nullopt, m_MaxTreeItemId,  MATCHED_LENGTH_ALL_TRACES_IN_GROUP ),
            buildTreeNode( "Absolute length", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, {}, std::nullopt, m_MaxTreeItemId,  ABSOLUTE_LENGTH ),
            buildTreeNode( "Absolute length 2", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, {}, std::nullopt, m_MaxTreeItemId,  ABSOLUTE_LENGTH_2 ),
        }, std::nullopt, parentId ),
        buildTreeNode( "Parallelism", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY,
        {
            buildTreeNode( "Parallel limit", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, {}, std::nullopt, m_MaxTreeItemId,  PARALLEL_LIMIT ),
        }, std::nullopt, parentId ),
        buildTreeNode( "Daisy Chain", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY,
        {
            buildTreeNode( "Daisy chain stub", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, {}, std::nullopt, m_MaxTreeItemId,  DAISY_CHAIN_STUB ),
            buildTreeNode( "Daisy chain stub 2", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, {}, std::nullopt, m_MaxTreeItemId,  DAISY_CHAIN_STUB_2 ),
        }, std::nullopt, parentId ),
    };

    return result;
}


std::vector<RuleTreeNode> DIALOG_DRC_RULE_EDITOR::createFootprintsItems( unsigned int& parentId )
{
    std::vector<RuleTreeNode> result = 
    {
        buildTreeNode( "Allowed Layers", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY, 
        {
            buildTreeNode( "Permitted layers", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, {}, std::nullopt, m_MaxTreeItemId,  PERMITTED_LAYERS ),
        }, std::nullopt, parentId ),
        buildTreeNode( "Orientation", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY, 
        {
            buildTreeNode( "Allowed orientation", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, {}, std::nullopt, m_MaxTreeItemId,  ALLOWED_ORIENTATION ),
        }, std::nullopt, parentId ),
        buildTreeNode( "Corner Style", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY,            
        {
            buildTreeNode( "Corner style", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, {}, std::nullopt, m_MaxTreeItemId,  CORNER_STYLE ),
        }, std::nullopt, parentId ),
        buildTreeNode( "SMD", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY, 
        {
            buildTreeNode( "SMD corner", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, {}, std::nullopt, m_MaxTreeItemId,  SMD_CORNER ),
            buildTreeNode( "SMD entry", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, {}, std::nullopt, m_MaxTreeItemId,  SMD_ENTRY ),
            buildTreeNode( "SMD to plane plus", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, {}, std::nullopt, m_MaxTreeItemId,  SMD_TO_PLANE_PLUS ),
            buildTreeNode( "Vias under SMD", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, {}, std::nullopt, m_MaxTreeItemId,  VIAS_UNDER_SMD ),
        }, std::nullopt, parentId ),
    };

    return result;
}


void DIALOG_DRC_RULE_EDITOR::AddNewRule( RuleTreeItemData* aRuleTreeItemData )
{
    wxTreeItemId treeItemId;

    if( aRuleTreeItemData->GetTreeItem()->node_type == CONSTRAINT )
    {
        treeItemId = aRuleTreeItemData->GetTreeItemId();
    }
    else
    {
        treeItemId = aRuleTreeItemData->GetParentTreeItemId();
    }    

    AppendTreeItem( m_treeCtrl, buildNewTreeData( aRuleTreeItemData ), treeItemId );
}


void DIALOG_DRC_RULE_EDITOR::DuplicateRule( RuleTreeItemData* aRuleTreeItemData )
{
    wxTreeItemId  treeItemId   = aRuleTreeItemData->GetParentTreeItemId();
    RuleTreeNode* ruleTreeNode = aRuleTreeItemData->GetTreeItem();
    RuleTreeNode  newTreeData  = buildNewTreeData( aRuleTreeItemData );
    wxString      ruleName = newTreeData.node_name;
    newTreeData.node_data = ruleTreeNode->node_data;
    newTreeData.node_name = ruleName;
    newTreeData.node_data->SetRuleName( ruleName );

    AppendTreeItem( m_treeCtrl, newTreeData, treeItemId );
}

 
bool DIALOG_DRC_RULE_EDITOR::CanShowContextMenu( RuleTreeNode* aRuleTreeNode )
{
    if( aRuleTreeNode->node_type != DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT && aRuleTreeNode->node_type != DRC_RULE_EDITOR_ITEM_TYPE::RULE )
        return false;
    
    return true;
}

bool DIALOG_DRC_RULE_EDITOR::CheckAndAppendRuleOperations( RuleTreeNode* aRuleTreeNode )
{
    if( aRuleTreeNode->node_type == DRC_RULE_EDITOR_ITEM_TYPE::RULE )
        return true;
    
    return false;
}


void DIALOG_DRC_RULE_EDITOR::TreeItemSelectionChanged( RuleTreeItemData* aCurrentRuleTreeItemData )
{
    RuleTreeNode* itemData = aCurrentRuleTreeItemData->GetTreeItem(); 

    if( itemData->node_type == DRC_RULE_EDITOR_ITEM_TYPE::RULE )
    {
        RuleTreeItemData* parentItemData = dynamic_cast<RuleTreeItemData*>( m_treeCtrl->GetItemData( aCurrentRuleTreeItemData->GetParentTreeItemId() ) );
        wxString constraintName = parentItemData->GetNodeName();

        if( GetPreviouslySelectedTreeItemId() )
        {
            RuleTreeItemData* previousItemData = dynamic_cast<RuleTreeItemData*>( m_treeCtrl->GetItemData( GetPreviouslySelectedTreeItemId() ) );
            UpdateRuleTypeTreeItemData( previousItemData );
        }

        m_parentPanel = new PANEL_DRC_RULE_EDITOR( this, m_frame->GetBoard(), static_cast<DRC_RULE_EDITOR_CONSTRAINT_NAME>( itemData->node_type_map.value_or(-1) ),
                                                   &constraintName, dynamic_pointer_cast<DrcReBaseConstraintData>( itemData->node_data) );

        SetContentPanel( m_parentPanel );
    }
}


void DIALOG_DRC_RULE_EDITOR::UpdateRuleTypeTreeItemData( RuleTreeItemData* aRuleTreeItemData )
{
    RuleTreeNode* itemData = aRuleTreeItemData->GetTreeItem(); 

    if( itemData->node_type == DRC_RULE_EDITOR_ITEM_TYPE::RULE && m_parentPanel )
    {            
        m_parentPanel->ProcessConstraintData();

        aRuleTreeItemData->SyncNodeName();

        UpdateTreeItem( m_treeCtrl, *aRuleTreeItemData );            
    }
}


// Recursive function to check if a node_name exists
bool NodeExists( const RuleTreeNode& aRuleTreeNode, const wxString& aTargetName )
{
    // Check the current node
    if( aRuleTreeNode.node_name == aTargetName )
    {
        return true;
    }

    // Check children nodes recursively
    for( const auto& child : aRuleTreeNode.child_nodes )
    {
        if( NodeExists( child, aTargetName ) )
        {
            return true;
        }
    }

    // Node name not found
    return false;
}


bool NodeExists( const std::vector<RuleTreeNode>& aRuleTreeNodes, const wxString& aTargetName )
{
    for( const auto& node : aRuleTreeNodes )
    {
        if( NodeExists( node, aTargetName ) )
        {
            return true;
        }
    }

    // Node name not found in the vector
    return false;
}


RuleTreeNode DIALOG_DRC_RULE_EDITOR::buildNewTreeData( RuleTreeItemData* aRuleTreeItemData )
{
    RuleTreeItemData* treeItemData;
    RuleTreeNode* treeNode;

    if( aRuleTreeItemData->GetTreeItem()->node_type == CONSTRAINT )
    {
        treeItemData = aRuleTreeItemData;
        treeNode = aRuleTreeItemData->GetTreeItem();
    }
    else
    {
        treeItemData = dynamic_cast<RuleTreeItemData*>( m_treeCtrl->GetItemData( aRuleTreeItemData->GetParentTreeItemId() ) );
        treeNode = treeItemData->GetTreeItem();
    }    

    RuleTreeNode newRuleNode;
    newRuleNode.node_name = treeNode->node_name + " 1";
    newRuleNode.node_type = RULE;
    newRuleNode.node_type_map = treeNode->node_type_map;
    newRuleNode.child_nodes = {};  

    int loop = 2;
    bool check = false;

    do
    {
        check = false;

        if( NodeExists( m_RuleTreeNodes, newRuleNode.node_name ) )
        {
            newRuleNode.node_name = treeNode->node_name + wxString::Format( " %d", loop );
            loop++;
            check = true;
        }
    } while( check );

    m_MaxTreeItemId++;
    unsigned int newId = m_MaxTreeItemId;

    DrcReNumericInputConstraintData clearanceData( newId, treeNode->node_data->GetId(), 0, newRuleNode.node_name );
    newRuleNode.node_data = std::make_shared<DrcReNumericInputConstraintData>( clearanceData );

    addRuleNodeToAConstraint( m_RuleTreeNodes, newRuleNode, treeNode->node_data->GetId() );

    return newRuleNode;
}


void DIALOG_DRC_RULE_EDITOR::addRuleNodeToAConstraint( std::vector<RuleTreeNode>& nodes, const RuleTreeNode& newRuleNode, const unsigned int& parentId )
{
    for( auto& node : nodes )
    {
        // Check if the node's category matches the target category
        if( node.node_data->GetId() == parentId )
        {
            node.child_nodes.push_back( newRuleNode ); // Add the new node as a child
            return;                             // Exit once the node is added
        }

        // Recurse into children
        if( !node.child_nodes.empty() )
        {
            addRuleNodeToAConstraint( node.child_nodes, newRuleNode, parentId );
        }
    }
}
