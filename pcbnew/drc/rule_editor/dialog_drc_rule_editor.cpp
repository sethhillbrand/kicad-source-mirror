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
#include "drc_re_bool_input_constraint_data.h"
#include "drc_re_via_style_constraint_data.h"
#include "drc_re_abs_length_two_constraint_data.h"
#include "drc_re_min_txt_ht_th_constraint_data.h"
#include "drc_re_rtg_diff_pair_constraint_data.h"
#include "drc_re_routing_width_constraint_data.h"
#include "drc_re_parallel_limit_constraint_data.h"
#include "drc_re_permitted_layers_constraint_data.h"
#include "drc_re_allowed_orientation_constraint_data.h"
#include "drc_re_corner_style_constraint_data.h"
#include "drc_re_smd_entry_constraint_data.h"


const RuleTreeNode* FindNodeById(const std::vector<RuleTreeNode>& nodes, unsigned int targetId)
{
    // Base case: Use std::find_if to search in the current level
    auto it = std::find_if(nodes.begin(), nodes.end(), [targetId](const RuleTreeNode& node) {
        return node.node_id == targetId;
    });

    if (it != nodes.end())
    {
        return &(*it); // Return pointer to the found node
    }

    // Node not found
    return nullptr;
}


DIALOG_DRC_RULE_EDITOR::DIALOG_DRC_RULE_EDITOR( PCB_EDIT_FRAME* aFrame ) :
        RULE_EDITOR_DIALOG_BASE( aFrame, _( "Design Rule Builder" ),
                                 aFrame->FromDIP( wxSize( 980, 640 ) ) ),
        m_frame( aFrame ), 
        m_ruleEditorPanel( nullptr ), 
        m_groupHeaderPanel( nullptr ), 
        m_nodeId( 0 )
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


bool DIALOG_DRC_RULE_EDITOR::TransferDataToWindow()
{
    return RULE_EDITOR_DIALOG_BASE::TransferDataToWindow();
}


bool DIALOG_DRC_RULE_EDITOR::TransferDataFromWindow()
{
    return RULE_EDITOR_DIALOG_BASE::TransferDataFromWindow();
}


RuleTreeNode DIALOG_DRC_RULE_EDITOR::buildTreeNode( const std::string& name, 
                                                    const DRC_RULE_EDITOR_ITEM_TYPE& nodeType,   
                                                    const std::optional<int>& parentId, 
                                                    const std::optional<DRC_RULE_EDITOR_CONSTRAINT_NAME>& constraintType,
                                                    const std::vector<RuleTreeNode>& childNodes, 
                                                    const std::optional<int>& id )
{
    unsigned int newId;

    if( id )
    {
        newId = *id; // Use provided ID
    }
    else
    {
        newId = 1;

        if( m_nodeId )
            newId = m_nodeId + 1;
    }

    m_nodeId = newId;

    RuleEditorBaseData baseData;
    baseData.SetId( newId );

    if( parentId )
    {
        baseData.SetParentId( *parentId );
    }

    return { 
             .node_id = m_nodeId,.node_name = name, .node_type = nodeType, .node_type_map = constraintType, 
             .child_nodes = childNodes, .node_data = std::make_shared<RuleEditorBaseData>( baseData ) 
           };
}


std::vector<RuleTreeNode> DIALOG_DRC_RULE_EDITOR::GetDefaultTreeItems()
{
    std::vector<RuleTreeNode> result;

    int lastParentId;
    int electricalItemId;
    int manufacturabilityItemId;
    int highSpeedDesignId;
    int footprintItemId;

    result.push_back( buildTreeNode( "Design Rules", DRC_RULE_EDITOR_ITEM_TYPE::ROOT ) );
    lastParentId = m_nodeId;

    result.push_back( buildTreeNode( "Electrical", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY, lastParentId ) );
    electricalItemId = m_nodeId;

    result.push_back( buildTreeNode( "Manufacturability", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY, lastParentId ) );
    manufacturabilityItemId = m_nodeId;

    result.push_back( buildTreeNode( "Highspeed design", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY, lastParentId ) );
    highSpeedDesignId = m_nodeId;

    result.push_back( buildTreeNode( "Footprints", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY, lastParentId ) );
    footprintItemId = m_nodeId;

    std::vector<RuleTreeNode> subItemNodes = createElectricalItems( electricalItemId );
    result.insert( result.end(), subItemNodes.begin(), subItemNodes.end() );

    subItemNodes = createManufacturabilityItems( manufacturabilityItemId );
    result.insert( result.end(), subItemNodes.begin(), subItemNodes.end() );

    subItemNodes = createHighspeedDesignItems( highSpeedDesignId );
    result.insert( result.end(), subItemNodes.begin(), subItemNodes.end() );

    subItemNodes = createFootprintsItems( footprintItemId );
    result.insert( result.end(), subItemNodes.begin(), subItemNodes.end() );
    
    return result;
}

std::vector<RuleTreeNode> DIALOG_DRC_RULE_EDITOR::createElectricalItems( int& parentId )
{
    std::vector<RuleTreeNode> result;
    int lastParentId;

    result.push_back( buildTreeNode( "Clearance", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY, parentId ) );
    lastParentId = m_nodeId;

    result.push_back( buildTreeNode( "Basic clearance", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, lastParentId, BASIC_CLEARANCE ) );
    result.push_back( buildTreeNode( "Board outline clearance", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, lastParentId, BOARD_OUTLINE_CLEARANCE ) );
    result.push_back( buildTreeNode( "Minimum clearance", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, lastParentId, MINIMUM_CLEARANCE ) );
    result.push_back( buildTreeNode( "Minimum item clearance", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, lastParentId, MINIMUM_ITEM_CLEARANCE ) );
    result.push_back( buildTreeNode( "Net antenna", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, lastParentId, NET_ANTENNA ) );
    result.push_back( buildTreeNode( "Short circuit", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, lastParentId, SHORT_CIRCUIT ) );
    result.push_back( buildTreeNode( "Creepage distance", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, lastParentId, CREEPAGE_DISTANCE ) );

    result.push_back( buildTreeNode( "Connection Width", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY, parentId ) );
    lastParentId = m_nodeId;

    result.push_back( buildTreeNode( "Minimum connection width", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, lastParentId, MINIMUM_CONNECTION_WIDTH ) );
    result.push_back( buildTreeNode( "Minimum track width", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, lastParentId, MINIMUM_TRACK_WIDTH ) );
    result.push_back( buildTreeNode( "Unrouted", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, lastParentId, UNROUTED ) );

    result.push_back( buildTreeNode( "Hole Clearance", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY, parentId ) );
    lastParentId = m_nodeId;

    result.push_back( buildTreeNode( "Copper to hole clearance", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, lastParentId, COPPER_TO_HOLE_CLEARANCE ) );
    result.push_back( buildTreeNode( "Hole to hole clearance", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, lastParentId, HOLE_TO_HOLE_CLEARANCE ) );

    result.push_back( buildTreeNode( "Spoke Count", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY, parentId ) );
    lastParentId = m_nodeId;

    result.push_back( buildTreeNode( "Minimum thermal relief spoke count", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, lastParentId, MINIMUM_THERMAL_RELIEF_SPOKE_COUNT ) );

    result.push_back( buildTreeNode( "Zone Connection", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY, parentId ) );
    lastParentId = m_nodeId;

    result.push_back( buildTreeNode( "Allow fillets outside zone outline", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, lastParentId, ALLOW_FILLET_OUTSIDE_ZONE_OUTLINE ) );

    return result;
}


std::vector<RuleTreeNode> DIALOG_DRC_RULE_EDITOR::createManufacturabilityItems( int& parentId )
{
    std::vector<RuleTreeNode> result;
    int lastParentId;

    result.push_back( buildTreeNode( "Annular Width", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY, parentId ) );
    lastParentId = m_nodeId;

    result.push_back( buildTreeNode( "Minimum annular width", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, lastParentId, MINIMUM_ANNULAR_WIDTH ) );

    result.push_back( buildTreeNode( "Courtyard Clearance", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY, parentId ) );
    lastParentId = m_nodeId;

    result.push_back( buildTreeNode( "Edge Clearance", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY, parentId ) );
    result.push_back( buildTreeNode( "Copper to edge clearance", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, lastParentId, COPPER_TO_EDGE_CLEARANCE ) );

    result.push_back( buildTreeNode( "Physical Clearance", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY, parentId ) );
    lastParentId = m_nodeId;

    result.push_back( buildTreeNode( "Hole Size", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY, parentId ) );
    result.push_back( buildTreeNode( "Minimum through hole", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, lastParentId, MINIMUM_THROUGH_HOLE ) );
    result.push_back( buildTreeNode( "Hole to hole distance", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, lastParentId, HOLE_TO_HOLE_DISTANCE ) );
    result.push_back( buildTreeNode( "Minimum uvia hole", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, lastParentId, MINIMUM_UVIA_HOLE ) );
    result.push_back( buildTreeNode( "Minimum uvia diameter", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, lastParentId, MINIMUM_UVIA_DIAMETER ) );
    result.push_back( buildTreeNode( "Via style", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, lastParentId, VIA_STYLE ) );

    result.push_back( buildTreeNode( "Text Geometry", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY, parentId ) );
    lastParentId = m_nodeId;

    result.push_back( buildTreeNode( "Minimum text height and thickness", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, lastParentId, MINIMUM_TEXT_HEIGHT_AND_THICKNESS ) );

    result.push_back( buildTreeNode( "Silkscreen Clearance", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY, parentId ) );
    lastParentId = m_nodeId;

    result.push_back( buildTreeNode( "Silk to silk clearance", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, lastParentId, SILK_TO_SILK_CLEARANCE ) );
    result.push_back( buildTreeNode( "Silk to soldermask clearance", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, lastParentId, SILK_TO_SOLDERMASK_CLEARANCE ) );

    result.push_back( buildTreeNode( "Soldermask", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY, parentId ) );
    lastParentId = m_nodeId;

    result.push_back( buildTreeNode( "Minimum soldermask silver", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, lastParentId, MINIMUM_SOLDERMASK_SILVER ) );
    result.push_back( buildTreeNode( "Soldermask expansion", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, lastParentId, SOLDERMASK_EXPANSION ) );

    result.push_back( buildTreeNode( "Solderpaste", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY, parentId ) );
    lastParentId = m_nodeId;

    result.push_back( buildTreeNode( "Solderpaste expansion", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, lastParentId, SOLDERPASTE_EXPANSION ) );

    result.push_back( buildTreeNode( "Deviation", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY, parentId ) );
    lastParentId = m_nodeId;

    result.push_back( buildTreeNode( "Maximum allowed deviation", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, lastParentId, MAXIMUM_ALLOWED_DEVIATION ) );

    result.push_back( buildTreeNode( "Angles", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY, parentId ) );
    lastParentId = m_nodeId;

    result.push_back( buildTreeNode( "Minimum acute angle", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, lastParentId, MINIMUM_ACUTE_ANGLE ) );
    result.push_back( buildTreeNode( "Minimum annular ring", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, lastParentId, MINIMUM_ANGULAR_RING ) );

     return result;
}


std::vector<RuleTreeNode> DIALOG_DRC_RULE_EDITOR::createHighspeedDesignItems( int& parentId )
{
    std::vector<RuleTreeNode> result;
    int lastParentId;

    result.push_back( buildTreeNode( "Diff Pair (width, gap, uncoupled length)", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY, parentId ) );
    lastParentId = m_nodeId;

    result.push_back( buildTreeNode( "Routing diff pair", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, lastParentId, ROUTING_DIFF_PAIR ) );
    result.push_back( buildTreeNode( "Routing width", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, lastParentId, ROUTING_WIDTH ) );
    result.push_back( buildTreeNode( "Maximum via count", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, lastParentId, MAXIMUM_VIA_COUNT ) );

    result.push_back( buildTreeNode( "Skew", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY, parentId ) );
    lastParentId = m_nodeId;

    result.push_back( buildTreeNode( "Length Matching", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY, parentId ) );
    lastParentId = m_nodeId;

    result.push_back( buildTreeNode( "Matched length diff pair", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, lastParentId, MATCHED_LENGTH_DIFF_PAIR ) );
    result.push_back( buildTreeNode( "Matched length all traces in group", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, lastParentId, MATCHED_LENGTH_ALL_TRACES_IN_GROUP ) );
    result.push_back( buildTreeNode( "Absolute length", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, lastParentId, ABSOLUTE_LENGTH ) );
    result.push_back( buildTreeNode( "Segment Length", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, lastParentId, ABSOLUTE_LENGTH_2 ) );

    result.push_back( buildTreeNode( "Parallelism", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY, parentId ) );
    lastParentId = m_nodeId;

    result.push_back( buildTreeNode( "Parallel limit", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, lastParentId, PARALLEL_LIMIT ) );

    result.push_back( buildTreeNode( "Daisy Chain", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY, parentId ) );
    lastParentId = m_nodeId;

    result.push_back( buildTreeNode( "Daisy chain stub", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, lastParentId, DAISY_CHAIN_STUB ) );
    result.push_back( buildTreeNode( "Daisy chain stub 2", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, lastParentId, DAISY_CHAIN_STUB_2 ) );

    return result;
}


std::vector<RuleTreeNode> DIALOG_DRC_RULE_EDITOR::createFootprintsItems( int& parentId )
{
    std::vector<RuleTreeNode> result;
    int lastParentId;

    result.push_back( buildTreeNode( "Allowed Layers", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY, parentId ) );
    lastParentId = m_nodeId;

    result.push_back( buildTreeNode( "Permitted layers", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, lastParentId, PERMITTED_LAYERS ) );

    result.push_back( buildTreeNode( "Orientation", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY, parentId ) );
    lastParentId = m_nodeId;

    result.push_back( buildTreeNode( "Allowed orientation", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, lastParentId, ALLOWED_ORIENTATION ) );

    result.push_back( buildTreeNode( "Corner Style", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY, parentId ) );
    lastParentId = m_nodeId;

    result.push_back( buildTreeNode( "Corner style", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, lastParentId, CORNER_STYLE ) );

    result.push_back( buildTreeNode( "SMD", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY, parentId ) );
    lastParentId = m_nodeId;

    result.push_back( buildTreeNode( "SMD corner", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, lastParentId, SMD_CORNER ) );
    result.push_back( buildTreeNode( "SMD entry", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, lastParentId, SMD_ENTRY ) );
    result.push_back( buildTreeNode( "SMD to plane plus", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, lastParentId, SMD_TO_PLANE_PLUS ) );
    result.push_back( buildTreeNode( "Vias under SMD", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, lastParentId, VIAS_UNDER_SMD ) );

    return result;
}


void DIALOG_DRC_RULE_EDITOR::AddNewRule( RuleTreeItemData* aRuleTreeItemData )
{
    wxTreeItemId treeItemId;
    RuleTreeNode* nodeDetail = getNodeInfo( aRuleTreeItemData );

    if( nodeDetail->node_type == CONSTRAINT )
    {
        treeItemId = aRuleTreeItemData->GetTreeItemId();
    }
    else
    {
        treeItemId = aRuleTreeItemData->GetParentTreeItemId();
    }    

    AppendNewTreeItem( m_treeCtrl, buildNewTreeData( aRuleTreeItemData ), treeItemId );
}


void DIALOG_DRC_RULE_EDITOR::DuplicateRule( RuleTreeItemData* aRuleTreeItemData )
{
    wxTreeItemId  treeItemId   = aRuleTreeItemData->GetParentTreeItemId();
    RuleTreeNode* ruleTreeNode = getNodeInfo( aRuleTreeItemData );
    RuleTreeNode  newTreeData  = buildNewTreeData( aRuleTreeItemData );
    wxString      ruleName = newTreeData.node_name;
    newTreeData.node_data = ruleTreeNode->node_data;
    newTreeData.node_name = ruleName;
    newTreeData.node_data->SetRuleName( ruleName );

    AppendNewTreeItem( m_treeCtrl, newTreeData, treeItemId );
}

 
bool DIALOG_DRC_RULE_EDITOR::CanShowContextMenu( RuleTreeItemData* aRuleTreeItemData )
{
    RuleTreeNode* nodeDetail = getNodeInfo( aRuleTreeItemData );

    if( nodeDetail->node_type != DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT && nodeDetail->node_type != DRC_RULE_EDITOR_ITEM_TYPE::RULE )
        return false;
    
    return true;
}

bool DIALOG_DRC_RULE_EDITOR::CheckAndAppendRuleOperations( RuleTreeItemData* aRuleTreeItemData )
{
    RuleTreeNode* nodeDetail = getNodeInfo( aRuleTreeItemData );

    if( nodeDetail->node_type == DRC_RULE_EDITOR_ITEM_TYPE::RULE )
        return true;
    
    return false;
}


void DIALOG_DRC_RULE_EDITOR::TreeItemSelectionChanged( RuleTreeItemData* aCurrentRuleTreeItemData )
{
    RuleTreeNode* nodeDetail = getNodeInfo( aCurrentRuleTreeItemData );

    if( nodeDetail->node_type == ROOT || nodeDetail->node_type == CATEGORY || nodeDetail->node_type == CONSTRAINT )
    {
        m_groupHeaderPanel = new PANEL_DRC_GROUP_HEADER( this, m_frame->GetBoard(), static_cast<DRC_RULE_EDITOR_ITEM_TYPE>( nodeDetail->node_type ) );
        SetContentPanel( m_groupHeaderPanel );
        m_ruleEditorPanel = nullptr;
    }
    else if( nodeDetail->node_type == RULE )
    {
        RuleTreeItemData* parentItemData = dynamic_cast<RuleTreeItemData*>( m_treeCtrl->GetItemData( aCurrentRuleTreeItemData->GetParentTreeItemId() ) );
        RuleTreeNode* paretNodeDetail = getNodeInfo( parentItemData );
        wxString      constraintName = paretNodeDetail->node_name;

       /* if( GetPreviouslySelectedTreeItemId() )
        {
            RuleTreeItemData* previousItemData = dynamic_cast<RuleTreeItemData*>( m_treeCtrl->GetItemData( GetPreviouslySelectedTreeItemId() ) );

            if( previousItemData )
            {
                RuleTreeNode* previousNodeDetail = getNodeInfo( previousItemData );

                if( previousNodeDetail->node_type == RULE )
                    UpdateRuleTypeTreeItemData( previousItemData );
            }
        }*/

        m_ruleEditorPanel = new PANEL_DRC_RULE_EDITOR( this, m_frame->GetBoard(), static_cast<DRC_RULE_EDITOR_CONSTRAINT_NAME>( nodeDetail->node_type_map.value_or(-1) ),
                                                   &constraintName, dynamic_pointer_cast<DrcReBaseConstraintData>( nodeDetail->node_data) );

        SetContentPanel( m_ruleEditorPanel );

        m_ruleEditorPanel->TransferDataToWindow();

        m_ruleEditorPanel->SetSaveOrUpdateCallback([this]( int aNodeId ) {
            this->onSaveOrUpdateButtonClicked( aNodeId );
        });

        m_ruleEditorPanel->SetCancelOrDeleteCallback([this]( int aNodeId ) {
            this->onCancelOrDeleteButtonClicked( aNodeId );
        });

        m_ruleEditorPanel->SetCloseCallback([this]( int aNodeId ) {
            this->onCloseButtonClicked( aNodeId );
        });

        m_ruleEditorPanel->SetRuleNameValidationCallback([this]( int aNodeId, wxString aRuleName ) {
           return this->validateRuleName( aNodeId, aRuleName );
        });

        m_groupHeaderPanel = nullptr;
    }
}


void DIALOG_DRC_RULE_EDITOR::UpdateRuleTypeTreeItemData( RuleTreeItemData* aRuleTreeItemData )
{
    RuleTreeNode* nodeDetail = getNodeInfo( aRuleTreeItemData );

    if( nodeDetail->node_type == DRC_RULE_EDITOR_ITEM_TYPE::RULE && m_ruleEditorPanel )
    {            
        m_ruleEditorPanel->TransferDataFromWindow();

        //aRuleTreeItemData->SyncNodeName();
        nodeDetail->node_name = nodeDetail->node_data->GetRuleName();
        nodeDetail->node_data->SetIsNew( false );
        UpdateTreeItemText( m_treeCtrl, aRuleTreeItemData->GetTreeItemId(), nodeDetail->node_name );   

        m_ruleEditorPanel->RefreshScreen();
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
    RuleTreeNode* nodeDetail = getNodeInfo( aRuleTreeItemData );

    if( nodeDetail->node_type == CONSTRAINT )
    {
        treeItemData = aRuleTreeItemData;
    }
    else
    {
        treeItemData = dynamic_cast<RuleTreeItemData*>( m_treeCtrl->GetItemData( aRuleTreeItemData->GetParentTreeItemId() ) );
        nodeDetail = getNodeInfo( treeItemData );
    }    

    m_nodeId++;

    RuleTreeNode newRuleNode;
    newRuleNode.node_id = m_nodeId;
    newRuleNode.node_name = nodeDetail->node_name + " 1";
    newRuleNode.node_type = RULE;
    newRuleNode.node_type_map = nodeDetail->node_type_map;
    newRuleNode.child_nodes = {};  

    int loop = 2;
    bool check = false;

    do
    {
        check = false;

        if( NodeExists( m_RuleTreeNodes, newRuleNode.node_name ) )
        {
            newRuleNode.node_name = nodeDetail->node_name + wxString::Format( " %d", loop );
            loop++;
            check = true;
        }
    } while( check );

   auto nodeType = static_cast<DRC_RULE_EDITOR_CONSTRAINT_NAME>(newRuleNode.node_type_map.value_or(-1));
   DrcReBaseConstraintData clearanceData( m_nodeId, nodeDetail->node_data->GetId(), newRuleNode.node_name );

    switch (nodeType) 
    {
        case DRC_RULE_EDITOR_CONSTRAINT_NAME::VIA_STYLE:
            newRuleNode.node_data = std::make_shared<DrcReViaStyleConstraintData>( clearanceData );
            break;

        case DRC_RULE_EDITOR_CONSTRAINT_NAME::MINIMUM_TEXT_HEIGHT_AND_THICKNESS:
            newRuleNode.node_data = std::make_shared<DrcReMinimumTextHeightThicknessConstraintData>( clearanceData );
            break;

        case DRC_RULE_EDITOR_CONSTRAINT_NAME::ROUTING_DIFF_PAIR:
            newRuleNode.node_data = std::make_shared<DrcReRoutingDiffPairConstraintData>( clearanceData );
            break;

        case DRC_RULE_EDITOR_CONSTRAINT_NAME::ROUTING_WIDTH:
            newRuleNode.node_data = std::make_shared<DrcReRoutingWidthConstraintData>( clearanceData );
            break;

        case DRC_RULE_EDITOR_CONSTRAINT_NAME::PARALLEL_LIMIT:
            newRuleNode.node_data = std::make_shared<DrcReParallelLimitConstraintData>( clearanceData );
            break;

        case DRC_RULE_EDITOR_CONSTRAINT_NAME::PERMITTED_LAYERS:
            newRuleNode.node_data = std::make_shared<DrcRePermittedLayersConstraintData>( clearanceData );
            break;

        case DRC_RULE_EDITOR_CONSTRAINT_NAME::ALLOWED_ORIENTATION:
            newRuleNode.node_data = std::make_shared<DrcReAllowedOrientationConstraintData>( clearanceData );
            break;

        case DRC_RULE_EDITOR_CONSTRAINT_NAME::CORNER_STYLE:
            newRuleNode.node_data = std::make_shared<DrcReCornerStyleConstraintData>( clearanceData );
            break;

        case DRC_RULE_EDITOR_CONSTRAINT_NAME::SMD_ENTRY:
            newRuleNode.node_data = std::make_shared<DrcReSmdEntryConstraintData>( clearanceData );
            break;

        default:
        {
            if( DRC_RULE_EDITOR_UTILS::IsNumericInputType( nodeType ) )
                newRuleNode.node_data = std::make_shared<DrcReNumericInputConstraintData>( clearanceData );
            else if( DRC_RULE_EDITOR_UTILS::IsBoolInputType( nodeType ) )
                newRuleNode.node_data = std::make_shared<DrcReBoolInputConstraintData>( clearanceData );
        }
            break;
    }

    newRuleNode.node_data->SetIsNew( true );

    RuleTreeNode ruleTreeNode = buildTreeNode( newRuleNode.node_name.ToStdString(),
                                              static_cast<DRC_RULE_EDITOR_ITEM_TYPE>( newRuleNode.node_type ), 
                                              nodeDetail->node_id, static_cast<DRC_RULE_EDITOR_CONSTRAINT_NAME>( newRuleNode.node_type_map.value_or(0) ), 
                                              {}, m_nodeId );
    ruleTreeNode.node_data = newRuleNode.node_data;

    m_RuleTreeNodes.push_back( ruleTreeNode );

    return newRuleNode;
}


RuleTreeNode* DIALOG_DRC_RULE_EDITOR::getNodeInfo( RuleTreeItemData* aRuleTreeItemData )
{
    unsigned int  targetId = aRuleTreeItemData->GetNodeId();

    auto it = std::find_if( m_RuleTreeNodes.begin(), m_RuleTreeNodes.end(),
                            [targetId]( const RuleTreeNode& node )
                            {
                                return node.node_id == targetId;
                            } );

    if( it != m_RuleTreeNodes.end() )
    {
        return &( *it ); // Return pointer to the found node
    }
    else
        return nullptr;
}


void DIALOG_DRC_RULE_EDITOR::onSaveOrUpdateButtonClicked( int aNodeId )
{
    if( !m_ruleEditorPanel->GetIsValidationSucceeded() )
    {
        std::string validationMessage = m_ruleEditorPanel->GetValidationMessage();

        DisplayErrorMessage( this, validationMessage );
    }
    else
    {
        RuleTreeItemData* itemData = dynamic_cast<RuleTreeItemData*>( m_treeCtrl->GetItemData( GetCurrentlySelectedTreeItemData()->GetTreeItemId() ) );

        if( itemData )
        {
            RuleTreeNode* nodeDetail = getNodeInfo( itemData );
            UpdateRuleTypeTreeItemData( itemData );
        }

        SetControlsEnabled( true );
    }
}


void DIALOG_DRC_RULE_EDITOR::onCancelOrDeleteButtonClicked( int aNodeId )
{
    SetControlsEnabled( true );
}


void DIALOG_DRC_RULE_EDITOR::onCloseButtonClicked( int aNodeId )
{
    SetControlsEnabled( true );
}


bool DIALOG_DRC_RULE_EDITOR::validateRuleName( int aNodeId, wxString aRuleName )
{
    auto it = std::find_if( m_RuleTreeNodes.begin(), m_RuleTreeNodes.end(),
                        [aNodeId, aRuleName]( const RuleTreeNode& node )
                        {
                            return node.node_name == aRuleName && node.node_id != aNodeId
                                && node.node_type == RULE;
                        } );

    if( it != m_RuleTreeNodes.end() )
    {
        return false;
    }

    return true;
}