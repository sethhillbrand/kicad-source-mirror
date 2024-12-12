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
#include "drc_re_base_clearance_constraint_data.h"


DIALOG_DRC_RULE_EDITOR::DIALOG_DRC_RULE_EDITOR( PCB_EDIT_FRAME* aFrame ) :
        RULE_EDITOR_DIALOG_BASE( aFrame, _( "Design Rule Builder" ),
                                 aFrame->FromDIP( wxSize( 980, 600 ) ) ),
        m_frame( aFrame ), 
        m_parentPanel( nullptr )
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


RuleTreeNode buildTreeNode( const std::string& name, const DRC_RULE_EDITOR_ITEM_TYPE& nodeType,    
                            const std::vector<RuleTreeNode>& children = {}, 
                            const std::optional<DRC_RULE_EDITOR_CONSTRAINT_NAME>& constraintType = std::nullopt )
{
    return { .node_name = name, .node_type = nodeType, .rule_type =  constraintType, .children = children };
}


std::vector<RuleTreeNode> DIALOG_DRC_RULE_EDITOR::GetDefaultTreeItems() const
{
    std::vector<RuleTreeNode> result = 
    {
        buildTreeNode( "Design Rules", DRC_RULE_EDITOR_ITEM_TYPE::ROOT,
        {
            buildTreeNode( "Electrical", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY, createElectricalItems() ),
            buildTreeNode( "Manufacturability", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY, createManufacturabilityItems() ),
            buildTreeNode( "Highspeed design", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY, createHighspeedDesignItems() ),
            buildTreeNode( "Footprints", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY, createFootprintsItems() ),
        } )
    };
    
    return result;
}

std::vector<RuleTreeNode> DIALOG_DRC_RULE_EDITOR::createElectricalItems() const
{
    std::vector<RuleTreeNode> result =  
    {
        buildTreeNode( "Clearance", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY, 
        {
            buildTreeNode( "Basic clearance", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, {}, BASIC_CLEARANCE ),
            buildTreeNode( "Board outline clearance", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, {}, BOARD_OUTLINE_CLEARANCE ),
            buildTreeNode( "Minimum clearance", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, {}, MINIMUM_CLEARANCE ),
            buildTreeNode( "Minimum item clearance", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, {}, MINIMUM_ITEM_CLEARANCE ),
            buildTreeNode( "Net antenna", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, {}, NET_ANTENNA ),
            buildTreeNode( "Short circuit", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, {}, SHORT_CIRCUIT ),
            buildTreeNode( "Creepage distance", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, {}, CREEPAGE_DISTANCE ),
        } ),
        buildTreeNode( "Connection Width",  DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY, 
        {
            buildTreeNode( "Minimum connection width", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, {}, MINIMUM_CONNECTION_WIDTH ),
            buildTreeNode( "Minimum track width", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, {}, MINIMUM_TRACK_WIDTH ),
            buildTreeNode( "Unrouted", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, {}, UNROUTED ),
        } ),
        buildTreeNode("Hole Clearance",  DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY, 
        {
            buildTreeNode( "Copper to hole clearance", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, {}, COPPER_TO_HOLE_CLEARANCE ),
            buildTreeNode( "Hole to hole clearance", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, {}, HOLE_TO_HOLE_CLEARANCE ),
        } ),
        buildTreeNode( "Spoke Count",  DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY,
        {
            buildTreeNode( "Minimum thermal relief spoke count", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, {}, MINIMUM_THERMAL_RELIEF_SPOKE_COUNT ),
        } ),
        buildTreeNode( "Zone Connection",  DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY, 
        {
            buildTreeNode( "Allow fillets outside zone outline", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, {}, ALLOW_FILLET_OUTSIDE_ZONE_OUTLINE ),
        } )
    };

    return result;
}


std::vector<RuleTreeNode> DIALOG_DRC_RULE_EDITOR::createManufacturabilityItems() const
{
     std::vector<RuleTreeNode> result = 
     {
        buildTreeNode( "Annular Width", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY,
        {
            buildTreeNode( "Minimum annular width", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, {}, MINIMUM_ANNULAR_WIDTH ),
        } ),
        buildTreeNode( "Courtyard Clearance", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY ),
        buildTreeNode( "Edge Clearance", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY,
        {
            buildTreeNode( "Copper to edge clearance", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, {}, COPPER_TO_EDGE_CLEARANCE ),
        } ),
        buildTreeNode( "Physical Clearance", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY ),
        buildTreeNode( "Hole Size", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY,
        {
            buildTreeNode( "Minimum through hole", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, {}, MINIMUM_THROUGH_HOLE ),
            buildTreeNode( "Hole size", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, {}, HOLE_SIZE ),
            buildTreeNode( "Hole to hole distance", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, {}, HOLE_TO_HOLE_DISTANCE ),
            buildTreeNode( "Minimum uvia hole", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, {}, MINIMUM_UVIA_HOLE ),
            buildTreeNode( "Minimum uvia diameter", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, {}, MINIMUM_UVIA_DIAMETER ),
            buildTreeNode( "Minimum via diameter", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, {}, MINIMUM_VIA_DIAMETER ),
            buildTreeNode( "Via style", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, {}, VIA_STYLE ),
        } ),
        buildTreeNode( "Text Geometry", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY,
        {
            buildTreeNode( "Minimum text height and thickness",DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, {}, MINIMUM_TEXT_HEIGHT_AND_THICKNESS ),
        } ),
        buildTreeNode( "Silkscreen Clearance", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY,
        {
            buildTreeNode( "Silk to silk clearance", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, {}, SILK_TO_SILK_CLEARANCE ),
            buildTreeNode( "Silk to soldermask clearance", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, {}, SILK_TO_SOLDERMASK_CLEARANCE ),
        } ),
        buildTreeNode( "Soldermask", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY,
        {
            buildTreeNode( "Minimum soldermask silver", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, {}, MINIMUM_SOLDERMASK_SILVER ),
            buildTreeNode( "Soldermask expansion", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, {}, SOLDERMASK_EXPANSION ),
        } ),
        buildTreeNode( "Solderpaste", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY,
        {
            buildTreeNode( "Solderpaste expansion", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, {}, SOLDERPASTE_EXPANSION ),
        } ),
        buildTreeNode( "Deviation", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY,
        {
            buildTreeNode( "Maximum allowed deviation", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, {}, MAXIMUM_ALLOWED_DEVIATION ),
        } ),
        buildTreeNode( "Angles", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY,
        {
            buildTreeNode( "Minimum acute angle", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, {}, MINIMUM_ACUTE_ANGLE ),
            buildTreeNode( "Minimum angular ring", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, {}, MINIMUM_ANGULAR_RING ),
        } ),
    };

     return result;
}


std::vector<RuleTreeNode> DIALOG_DRC_RULE_EDITOR::createHighspeedDesignItems() const
{
    std::vector<RuleTreeNode> result = 
    {
        buildTreeNode( "Diff Pair (width, gap, uncoupled length)", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY,
        {
            buildTreeNode( "Matched length diff pair", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, {}, MATCHED_LENGTH_DIFF_PAIR ),
            buildTreeNode( "Routing diff pair", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, {}, ROUTING_DIFF_PAIR ),
            buildTreeNode( "Routing width", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, {}, ROUTING_WIDTH ),
            buildTreeNode( "Maximum via count", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, {}, MAXIMUM_VIA_COUNT ),
        } ),
        buildTreeNode( "Skew", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY ),
        buildTreeNode( "Length Matching", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY,
        {
            buildTreeNode( "Matched length all traces in group",DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, {}, MATCHED_LENGTH_ALL_TRACES_IN_GROUP ),
            buildTreeNode( "Absolute length", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, {}, ABSOLUTE_LENGTH ),
            buildTreeNode( "Absolute length 2", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, {}, ABSOLUTE_LENGTH_2 ),
        } ),
        buildTreeNode( "Parallelism", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY,
        {
            buildTreeNode( "Parallel limit", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, {}, PARALLEL_LIMIT ),
        } ),
        buildTreeNode( "Daisy Chain", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY,
        {
            buildTreeNode( "Daisy chain stub", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, {}, DAISY_CHAIN_STUB ),
            buildTreeNode( "Daisy chain stub 2", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, {}, DAISY_CHAIN_STUB_2 ),
        } ),
    };

    return result;
}


std::vector<RuleTreeNode> DIALOG_DRC_RULE_EDITOR::createFootprintsItems() const
{
    std::vector<RuleTreeNode> result = 
    {
        buildTreeNode( "Allowed Layers", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY, 
        {
            buildTreeNode( "Permitted layers", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, {}, PERMITTED_LAYERS ),
        } ),
        buildTreeNode( "Orientation", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY, 
        {
            buildTreeNode( "Allowed orientation", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, {}, ALLOWED_ORIENTATION ),
        } ),
        buildTreeNode( "Corner Style", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY,            
        {
            buildTreeNode( "Corner style", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, {}, CORNER_STYLE ),
        } ),
        buildTreeNode( "SMD", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY, 
        {
            buildTreeNode( "SMD corner", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, {}, SMD_CORNER ),
            buildTreeNode( "SMD entry", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, {}, SMD_ENTRY ),
            buildTreeNode( "SMD to plane plus", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, {}, SMD_TO_PLANE_PLUS ),
            buildTreeNode( "Vias under SMD", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, {}, VIAS_UNDER_SMD ),
        } ),
    };

    return result;
}


void DIALOG_DRC_RULE_EDITOR::AddNewRule( RuleTreeItemData* aRuleTreeItemData )
{
    wxTreeItemId treeItemId;

    if( aRuleTreeItemData->GetData()->node_type == CONSTRAINT )
    {
        treeItemId = aRuleTreeItemData->GetTreeItemId();
    }
    else
    {
        treeItemId = aRuleTreeItemData->GetParentTreeItemId();
    }    

    AppendTreeItem( m_treeCtrl, buildNewTreeData( aRuleTreeItemData ), treeItemId );
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
    RuleTreeNode* itemData = aCurrentRuleTreeItemData->GetData(); 

    if( itemData->node_type == DRC_RULE_EDITOR_ITEM_TYPE::RULE )
    {
        RuleTreeItemData* parentItemData = dynamic_cast<RuleTreeItemData*>( m_treeCtrl->GetItemData( aCurrentRuleTreeItemData->GetParentTreeItemId() ) );
        wxString constraintName = parentItemData->GetData()->node_name + " Constraint";

        if( m_parentPanel )
        {
            m_parentPanel->ProcessConstraintData();

            if( GetPreviouslySelectedTreeItemId() )
            {
                RuleTreeItemData* previousItemData = dynamic_cast<RuleTreeItemData*>( m_treeCtrl->GetItemData( GetPreviouslySelectedTreeItemId() ) );

                previousItemData->GetData()->node_name =
                        previousItemData->GetData()->rule_data->GetRuleName();


                UpdateTreeItem( m_treeCtrl, *previousItemData );
            }
        }

        m_parentPanel = new PANEL_DRC_RULE_EDITOR( this, m_frame->GetBoard(), static_cast<DRC_RULE_EDITOR_CONSTRAINT_NAME>( itemData->rule_type.value() ),
                                                   &constraintName, dynamic_pointer_cast<DrcReBaseConstraintData>( itemData->rule_data) );

        SetContentPanel( m_parentPanel );
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
    for( const auto& child : aRuleTreeNode.children )
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
    RuleTreeItemData* itemData;
    RuleTreeNode* data;

    if( aRuleTreeItemData->GetData()->node_type == CONSTRAINT )
    {
        itemData = aRuleTreeItemData;
        data = aRuleTreeItemData->GetData();
    }
    else
    {
        itemData = dynamic_cast<RuleTreeItemData*>( m_treeCtrl->GetItemData( aRuleTreeItemData->GetParentTreeItemId() ) );
        data = itemData->GetData();
    }    

    RuleTreeNode newRuleNode;
    newRuleNode.node_name = data->node_name + " 1";
    newRuleNode.node_type = RULE;
    newRuleNode.rule_type = data->rule_type;
    newRuleNode.children = {};  

    int loop = 2;
    bool check = false;

    do
    {
        check = false;

        if( NodeExists( m_RuleTreeNodes, newRuleNode.node_name ) )
        {
            newRuleNode.node_name = data->node_name + wxString::Format( " %d", loop );
            loop++;
            check = true;
        }
    } while( check );

    DrcReBaseClearanceConstraintData clearanceData;
    clearanceData.SetClearanceValue( 0 );
    clearanceData.SetRuleName( newRuleNode.node_name );
    newRuleNode.rule_data = std::make_shared<DrcReBaseClearanceConstraintData>( clearanceData );

    addRuleNodeToAConstraint( m_RuleTreeNodes, newRuleNode );

    return newRuleNode;
}


void DIALOG_DRC_RULE_EDITOR::addRuleNodeToAConstraint( std::vector<RuleTreeNode>& nodes, const RuleTreeNode& newRuleNode )
{
    for( auto& node : nodes )
    {
        // Check if the node's category matches the target category
        if( node.node_type == DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT && node.rule_type == newRuleNode.rule_type )
        {
            node.children.push_back( newRuleNode ); // Add the new node as a child
            return;                             // Exit once the node is added
        }

        // Recurse into children
        if( !node.children.empty() )
        {
            addRuleNodeToAConstraint( node.children, newRuleNode );
        }
    }
}
