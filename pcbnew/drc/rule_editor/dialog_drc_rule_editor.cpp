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


#include <widgets/wx_progress_reporters.h>

#include <confirm.h>
#include <pcb_edit_frame.h>
#include <kiface_base.h>

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


const RULE_TREE_NODE* FindNodeById( const std::vector<RULE_TREE_NODE>& aNodes,
                                    int aTargetId )
{
    auto it = std::find_if( aNodes.begin(), aNodes.end(),
                            [aTargetId]( const RULE_TREE_NODE& node )
                            {
                                return node.m_nodeId == aTargetId;
                            } );

    if( it != aNodes.end() )
    {
        return &( *it );
    }

    return nullptr;
}


DIALOG_DRC_RULE_EDITOR::DIALOG_DRC_RULE_EDITOR( PCB_EDIT_FRAME* aFrame ) :
        RULE_EDITOR_DIALOG_BASE( aFrame, _( "Design Rule Builder" ),
                                 aFrame->FromDIP( wxSize( 980, 680 ) ) ),
        m_frame( aFrame ), 
        m_nodeId( 0 )
{
    m_ruleTreeCtrl->DeleteAllItems();

    m_ruleTreeNodeDatas = GetDefaultRuleTreeItems();

    InitRuleTreeItems( m_ruleTreeNodeDatas );

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
    if( m_ruleEditorPanel )
    {
        delete m_ruleEditorPanel;
        m_ruleEditorPanel = nullptr;
    }

    if( m_groupHeaderPanel )
    {
        delete m_groupHeaderPanel;
        m_groupHeaderPanel = nullptr;
    }
}


bool DIALOG_DRC_RULE_EDITOR::TransferDataToWindow()
{
    return RULE_EDITOR_DIALOG_BASE::TransferDataToWindow();
}


bool DIALOG_DRC_RULE_EDITOR::TransferDataFromWindow()
{
    return RULE_EDITOR_DIALOG_BASE::TransferDataFromWindow();
}


std::vector<RULE_TREE_NODE> DIALOG_DRC_RULE_EDITOR::GetDefaultRuleTreeItems()
{
    std::vector<RULE_TREE_NODE> result;

    int lastParentId;
    int electricalItemId;
    int manufacturabilityItemId;
    int highSpeedDesignId;
    int footprintItemId;

    result.push_back( buildRuleTreeNodeData( "Design Rules", DRC_RULE_EDITOR_ITEM_TYPE::ROOT ) );
    lastParentId = m_nodeId;

    result.push_back( buildRuleTreeNodeData( "Electrical", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY,
                                             lastParentId ) );
    electricalItemId = m_nodeId;

    result.push_back( buildRuleTreeNodeData( "Manufacturability",
                                             DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY, lastParentId ) );
    manufacturabilityItemId = m_nodeId;

    result.push_back( buildRuleTreeNodeData( "Highspeed design",
                                             DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY, lastParentId ) );
    highSpeedDesignId = m_nodeId;

    result.push_back( buildRuleTreeNodeData( "Footprints", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY,
                                             lastParentId ) );
    footprintItemId = m_nodeId;

    std::vector<RULE_TREE_NODE> subItemNodes = buildElectricalRuleTreeNodes( electricalItemId );
    result.insert( result.end(), subItemNodes.begin(), subItemNodes.end() );

    subItemNodes = buildManufacturabilityRuleTreeNodes( manufacturabilityItemId );
    result.insert( result.end(), subItemNodes.begin(), subItemNodes.end() );

    subItemNodes = buildHighspeedDesignRuleTreeNodes( highSpeedDesignId );
    result.insert( result.end(), subItemNodes.begin(), subItemNodes.end() );

    subItemNodes = buildFootprintsRuleTreeNodes( footprintItemId );
    result.insert( result.end(), subItemNodes.begin(), subItemNodes.end() );

    return result;
}


void DIALOG_DRC_RULE_EDITOR::AddNewRule( RULE_TREE_ITEM_DATA* aRuleTreeItemData )
{
    wxTreeItemId    treeItemId;
    RULE_TREE_NODE* nodeDetail = getRuleTreeNodeInfo( aRuleTreeItemData->GetNodeId() );

    if( nodeDetail->m_nodeType == CONSTRAINT )
    {
        treeItemId = aRuleTreeItemData->GetTreeItemId();
    }
    else
    {
        treeItemId = aRuleTreeItemData->GetParentTreeItemId();
    }

    AppendNewRuleTreeItem( buildRuleTreeNode( aRuleTreeItemData ), treeItemId );
}


void DIALOG_DRC_RULE_EDITOR::DuplicateRule( RULE_TREE_ITEM_DATA* aRuleTreeItemData )
{
    RULE_TREE_NODE* sourceTreeNode = getRuleTreeNodeInfo( aRuleTreeItemData->GetNodeId() );
    RULE_TREE_NODE  targetTreeNode = buildRuleTreeNode( aRuleTreeItemData );
    RULE_TREE_NODE* nodeDetail = getRuleTreeNodeInfo( targetTreeNode.m_nodeId );

    auto sourceDataPtr = dynamic_pointer_cast<RULE_EDITOR_DATA_BASE>( sourceTreeNode->m_nodeData );

    if( sourceDataPtr )
    {
        targetTreeNode.m_nodeData->CopyFrom( *sourceDataPtr );
    }

    wxTreeItemId treeItemId = aRuleTreeItemData->GetParentTreeItemId();
    AppendNewRuleTreeItem( targetTreeNode, treeItemId );
}


void DIALOG_DRC_RULE_EDITOR::RuleTreeItemSelectionChanged(
        RULE_TREE_ITEM_DATA* aCurrentRuleTreeItemData )
{
    RULE_TREE_NODE* nodeDetail = getRuleTreeNodeInfo( aCurrentRuleTreeItemData->GetNodeId() );

    if( nodeDetail->m_nodeType == ROOT || nodeDetail->m_nodeType == CATEGORY
        || nodeDetail->m_nodeType == CONSTRAINT )
    {
        m_groupHeaderPanel = new PANEL_DRC_GROUP_HEADER(
                this, m_frame->GetBoard(),
                static_cast<DRC_RULE_EDITOR_ITEM_TYPE>( nodeDetail->m_nodeType ) );
        SetContentPanel( m_groupHeaderPanel );
        m_ruleEditorPanel = nullptr;
    }
    else if( nodeDetail->m_nodeType == RULE )
    {
        RULE_TREE_ITEM_DATA* parentItemData = dynamic_cast<RULE_TREE_ITEM_DATA*>(
                m_ruleTreeCtrl->GetItemData( aCurrentRuleTreeItemData->GetParentTreeItemId() ) );
        RULE_TREE_NODE* paretNodeDetail = getRuleTreeNodeInfo( parentItemData->GetNodeId() );
        wxString constraintName = paretNodeDetail->m_nodeName;

        m_ruleEditorPanel = new PANEL_DRC_RULE_EDITOR( this, m_frame->GetBoard(),
                static_cast<DRC_RULE_EDITOR_CONSTRAINT_NAME>(
                        nodeDetail->m_nodeTypeMap.value_or( -1 ) ), &constraintName,
                dynamic_pointer_cast<DRC_RE_BASE_CONSTRAINT_DATA>( nodeDetail->m_nodeData ) );

        SetContentPanel( m_ruleEditorPanel );

        m_ruleEditorPanel->TransferDataToWindow();

        m_ruleEditorPanel->SetSaveCallback(
                [this]( int aNodeId )
                {
                    this->saveRule( aNodeId );
                } );

        m_ruleEditorPanel->SetRemoveCallback(
                [this]( int aNodeId )
                {
                    this->RemoveRule( aNodeId );
                } );

        m_ruleEditorPanel->SetCloseCallback(
                [this]( int aNodeId )
                {
                    this->closeRuleEntryView( aNodeId );
                } );

        m_ruleEditorPanel->SetRuleNameValidationCallback(
                [this]( int aNodeId, wxString aRuleName )
                {
                    return this->validateRuleName( aNodeId, aRuleName );
                } );

        m_groupHeaderPanel = nullptr;
    }
}


void DIALOG_DRC_RULE_EDITOR::UpdateRuleTypeTreeItemData( RULE_TREE_ITEM_DATA* aRuleTreeItemData )
{
    RULE_TREE_NODE* nodeDetail = getRuleTreeNodeInfo( aRuleTreeItemData->GetNodeId() );

    if( nodeDetail->m_nodeType == DRC_RULE_EDITOR_ITEM_TYPE::RULE && m_ruleEditorPanel )
    {
        m_ruleEditorPanel->TransferDataFromWindow();

        nodeDetail->m_nodeName = nodeDetail->m_nodeData->GetRuleName();
        nodeDetail->m_nodeData->SetIsNew( false );
        UpdateRuleTreeItemText( aRuleTreeItemData->GetTreeItemId(), nodeDetail->m_nodeName );

        m_ruleEditorPanel->RefreshScreen();
    }
}


bool DIALOG_DRC_RULE_EDITOR::VerifyRuleTreeContextMenuOptionToEnable(
        RULE_TREE_ITEM_DATA* aRuleTreeItemData, RULE_EDITOR_TREE_CONTEXT_OPT aOption )
{
    RULE_TREE_NODE* nodeDetail = getRuleTreeNodeInfo( aRuleTreeItemData->GetNodeId() );

    switch( aOption )
    {
    case RULE_EDITOR_TREE_CONTEXT_OPT::ADD_RULE:
        return nodeDetail->m_nodeType == DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT
               || nodeDetail->m_nodeType == DRC_RULE_EDITOR_ITEM_TYPE::RULE;
    case RULE_EDITOR_TREE_CONTEXT_OPT::DUPLICATE_RULE:
    case RULE_EDITOR_TREE_CONTEXT_OPT::DELETE_RULE:
        return nodeDetail->m_nodeType == DRC_RULE_EDITOR_ITEM_TYPE::RULE;
    default: return true;
    }
}


void DIALOG_DRC_RULE_EDITOR::RemoveRule( int aNodeId )
{
    RULE_TREE_ITEM_DATA* itemData = dynamic_cast<RULE_TREE_ITEM_DATA*>( m_ruleTreeCtrl->GetItemData(
            GetCurrentlySelectedRuleTreeItemData()->GetTreeItemId() ) );
    RULE_TREE_NODE* nodeDetail = getRuleTreeNodeInfo( itemData->GetNodeId() );

    if( !nodeDetail->m_nodeData->IsNew() )
    {
        if( OKOrCancelDialog( this, _( "Confirmation" ), "",
                              _( "Are you sure you want to delete?" ), _( "Delete" ) )
            != wxID_OK )
        {
            return;
        }
    }

    if( itemData )
    {
        int nodeId = itemData->GetNodeId();

        DeleteRuleTreeItem( GetCurrentlySelectedRuleTreeItemData()->GetTreeItemId(), nodeId );
        deleteTreeNodeData( nodeId );
    }

    SetControlsEnabled( true );
}


std::vector<RULE_TREE_NODE> DIALOG_DRC_RULE_EDITOR::buildElectricalRuleTreeNodes( int& aParentId )
{
    std::vector<RULE_TREE_NODE> result;
    int lastParentId;

    result.push_back(
            buildRuleTreeNodeData( "Clearance", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY, aParentId ) );
    lastParentId = m_nodeId;

    result.push_back( buildRuleTreeNodeData( "Basic clearance",
                                             DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, lastParentId,
                                             BASIC_CLEARANCE ) );
    result.push_back( buildRuleTreeNodeData( "Board outline clearance",
                                             DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, lastParentId,
                                             BOARD_OUTLINE_CLEARANCE ) );
    result.push_back( buildRuleTreeNodeData( "Minimum clearance",
                                             DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, lastParentId,
                                             MINIMUM_CLEARANCE ) );
    result.push_back( buildRuleTreeNodeData( "Minimum item clearance",
                                             DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, lastParentId,
                                             MINIMUM_ITEM_CLEARANCE ) );
    result.push_back( buildRuleTreeNodeData( "Net antenna", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT,
                                             lastParentId, NET_ANTENNA ) );
    result.push_back( buildRuleTreeNodeData( "Short circuit", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT,
                                             lastParentId, SHORT_CIRCUIT ) );
    result.push_back( buildRuleTreeNodeData( "Creepage distance",
                                             DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, lastParentId,
                                             CREEPAGE_DISTANCE ) );

    result.push_back( buildRuleTreeNodeData( "Connection Width",
                                             DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY, aParentId ) );
    lastParentId = m_nodeId;

    result.push_back( buildRuleTreeNodeData( "Minimum connection width",
                                             DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, lastParentId,
                                             MINIMUM_CONNECTION_WIDTH ) );
    result.push_back( buildRuleTreeNodeData( "Minimum track width",
                                             DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, lastParentId,
                                             MINIMUM_TRACK_WIDTH ) );
    result.push_back( buildRuleTreeNodeData( "Unrouted", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT,
                                             lastParentId, UNROUTED ) );

    result.push_back( buildRuleTreeNodeData( "Hole Clearance", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY,
                                             aParentId ) );
    lastParentId = m_nodeId;

    result.push_back( buildRuleTreeNodeData( "Copper to hole clearance",
                                             DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, lastParentId,
                                             COPPER_TO_HOLE_CLEARANCE ) );
    result.push_back( buildRuleTreeNodeData( "Hole to hole clearance",
                                             DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, lastParentId,
                                             HOLE_TO_HOLE_CLEARANCE ) );

    result.push_back( buildRuleTreeNodeData( "Spoke Count", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY,
                                             aParentId ) );
    lastParentId = m_nodeId;

    result.push_back( buildRuleTreeNodeData( "Minimum thermal relief spoke count",
                                             DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, lastParentId,
                                             MINIMUM_THERMAL_RELIEF_SPOKE_COUNT ) );

    result.push_back( buildRuleTreeNodeData( "Zone Connection", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY,
                                             aParentId ) );
    lastParentId = m_nodeId;

    result.push_back( buildRuleTreeNodeData( "Allow fillets outside zone outline",
                                             DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, lastParentId,
                                             ALLOW_FILLET_OUTSIDE_ZONE_OUTLINE ) );

    return result;
}


std::vector<RULE_TREE_NODE>
DIALOG_DRC_RULE_EDITOR::buildManufacturabilityRuleTreeNodes( int& aParentId )
{
    std::vector<RULE_TREE_NODE> result;
    int lastParentId;

    result.push_back( buildRuleTreeNodeData( "Annular Width", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY,
                                             aParentId ) );
    lastParentId = m_nodeId;

    result.push_back( buildRuleTreeNodeData( "Minimum annular width",
                                             DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, lastParentId,
                                             MINIMUM_ANNULAR_WIDTH ) );

    result.push_back( buildRuleTreeNodeData( "Courtyard Clearance",
                                             DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY, aParentId ) );
    lastParentId = m_nodeId;

    result.push_back( buildRuleTreeNodeData( "Edge Clearance", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY,
                                             aParentId ) );
    result.push_back( buildRuleTreeNodeData( "Copper to edge clearance",
                                             DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, lastParentId,
                                             COPPER_TO_EDGE_CLEARANCE ) );

    result.push_back( buildRuleTreeNodeData( "Physical Clearance",
                                             DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY, aParentId ) );
    lastParentId = m_nodeId;

    result.push_back(
            buildRuleTreeNodeData( "Hole Size", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY, aParentId ) );
    result.push_back( buildRuleTreeNodeData( "Minimum through hole",
                                             DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, lastParentId,
                                             MINIMUM_THROUGH_HOLE ) );
    result.push_back( buildRuleTreeNodeData( "Hole to hole distance",
                                             DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, lastParentId,
                                             HOLE_TO_HOLE_DISTANCE ) );
    result.push_back( buildRuleTreeNodeData( "Minimum uvia hole",
                                             DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, lastParentId,
                                             MINIMUM_UVIA_HOLE ) );
    result.push_back( buildRuleTreeNodeData( "Minimum uvia diameter",
                                             DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, lastParentId,
                                             MINIMUM_UVIA_DIAMETER ) );
    result.push_back( buildRuleTreeNodeData( "Via style", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT,
                                             lastParentId, VIA_STYLE ) );

    result.push_back( buildRuleTreeNodeData( "Text Geometry", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY,
                                             aParentId ) );
    lastParentId = m_nodeId;

    result.push_back( buildRuleTreeNodeData( "Minimum text height and thickness",
                                             DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, lastParentId,
                                             MINIMUM_TEXT_HEIGHT_AND_THICKNESS ) );

    result.push_back( buildRuleTreeNodeData( "Silkscreen Clearance",
                                             DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY, aParentId ) );
    lastParentId = m_nodeId;

    result.push_back( buildRuleTreeNodeData( "Silk to silk clearance",
                                             DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, lastParentId,
                                             SILK_TO_SILK_CLEARANCE ) );
    result.push_back( buildRuleTreeNodeData( "Silk to soldermask clearance",
                                             DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, lastParentId,
                                             SILK_TO_SOLDERMASK_CLEARANCE ) );

    result.push_back(
            buildRuleTreeNodeData( "Soldermask", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY, aParentId ) );
    lastParentId = m_nodeId;

    result.push_back( buildRuleTreeNodeData( "Minimum soldermask silver",
                                             DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, lastParentId,
                                             MINIMUM_SOLDERMASK_SILVER ) );
    result.push_back( buildRuleTreeNodeData( "Soldermask expansion",
                                             DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, lastParentId,
                                             SOLDERMASK_EXPANSION ) );

    result.push_back( buildRuleTreeNodeData( "Solderpaste", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY,
                                             aParentId ) );
    lastParentId = m_nodeId;

    result.push_back( buildRuleTreeNodeData( "Solderpaste expansion",
                                             DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, lastParentId,
                                             SOLDERPASTE_EXPANSION ) );

    result.push_back(
            buildRuleTreeNodeData( "Deviation", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY, aParentId ) );
    lastParentId = m_nodeId;

    result.push_back( buildRuleTreeNodeData( "Maximum allowed deviation",
                                             DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, lastParentId,
                                             MAXIMUM_ALLOWED_DEVIATION ) );

    result.push_back(
            buildRuleTreeNodeData( "Angles", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY, aParentId ) );
    lastParentId = m_nodeId;

    result.push_back( buildRuleTreeNodeData( "Minimum acute angle",
                                             DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, lastParentId,
                                             MINIMUM_ACUTE_ANGLE ) );
    result.push_back( buildRuleTreeNodeData( "Minimum annular ring",
                                             DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, lastParentId,
                                             MINIMUM_ANGULAR_RING ) );

    return result;
}


std::vector<RULE_TREE_NODE>
DIALOG_DRC_RULE_EDITOR::buildHighspeedDesignRuleTreeNodes( int& aParentId )
{
    std::vector<RULE_TREE_NODE> result;
    int lastParentId;

    result.push_back( buildRuleTreeNodeData( "Diff Pair (width, gap, uncoupled length)",
                                             DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY, aParentId ) );
    lastParentId = m_nodeId;

    result.push_back( buildRuleTreeNodeData( "Routing diff pair",
                                             DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, lastParentId,
                                             ROUTING_DIFF_PAIR ) );
    result.push_back( buildRuleTreeNodeData( "Routing width", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT,
                                             lastParentId, ROUTING_WIDTH ) );
    result.push_back( buildRuleTreeNodeData( "Maximum via count",
                                             DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, lastParentId,
                                             MAXIMUM_VIA_COUNT ) );

    result.push_back(
            buildRuleTreeNodeData( "Skew", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY, aParentId ) );
    lastParentId = m_nodeId;

    result.push_back( buildRuleTreeNodeData( "Length Matching", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY,
                                             aParentId ) );
    lastParentId = m_nodeId;

    result.push_back( buildRuleTreeNodeData( "Matched length diff pair",
                                             DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, lastParentId,
                                             MATCHED_LENGTH_DIFF_PAIR ) );
    result.push_back( buildRuleTreeNodeData( "Matched length all traces in group",
                                             DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, lastParentId,
                                             MATCHED_LENGTH_ALL_TRACES_IN_GROUP ) );
    result.push_back( buildRuleTreeNodeData( "Absolute length",
                                             DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, lastParentId,
                                             ABSOLUTE_LENGTH ) );
    result.push_back( buildRuleTreeNodeData( "Segment Length",
                                             DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, lastParentId,
                                             ABSOLUTE_LENGTH_2 ) );

    result.push_back( buildRuleTreeNodeData( "Parallelism", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY,
                                             aParentId ) );
    lastParentId = m_nodeId;

    result.push_back( buildRuleTreeNodeData( "Parallel limit",
                                             DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, lastParentId,
                                             PARALLEL_LIMIT ) );

    result.push_back( buildRuleTreeNodeData( "Daisy Chain", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY,
                                             aParentId ) );
    lastParentId = m_nodeId;

    result.push_back( buildRuleTreeNodeData( "Daisy chain stub",
                                             DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, lastParentId,
                                             DAISY_CHAIN_STUB ) );
    result.push_back( buildRuleTreeNodeData( "Daisy chain stub 2",
                                             DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, lastParentId,
                                             DAISY_CHAIN_STUB_2 ) );

    return result;
}


std::vector<RULE_TREE_NODE> DIALOG_DRC_RULE_EDITOR::buildFootprintsRuleTreeNodes( int& aParentId )
{
    std::vector<RULE_TREE_NODE> result;
    int lastParentId;

    result.push_back( buildRuleTreeNodeData( "Allowed Layers", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY,
                                             aParentId ) );
    lastParentId = m_nodeId;

    result.push_back( buildRuleTreeNodeData( "Permitted layers",
                                             DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, lastParentId,
                                             PERMITTED_LAYERS ) );

    result.push_back( buildRuleTreeNodeData( "Orientation", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY,
                                             aParentId ) );
    lastParentId = m_nodeId;

    result.push_back( buildRuleTreeNodeData( "Allowed orientation",
                                             DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, lastParentId,
                                             ALLOWED_ORIENTATION ) );

    result.push_back( buildRuleTreeNodeData( "Corner Style", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY,
                                             aParentId ) );
    lastParentId = m_nodeId;

    result.push_back( buildRuleTreeNodeData( "Corner style", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT,
                                             lastParentId, CORNER_STYLE ) );

    result.push_back(
            buildRuleTreeNodeData( "SMD", DRC_RULE_EDITOR_ITEM_TYPE::CATEGORY, aParentId ) );
    lastParentId = m_nodeId;

    result.push_back( buildRuleTreeNodeData( "SMD corner", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT,
                                             lastParentId, SMD_CORNER ) );
    result.push_back( buildRuleTreeNodeData( "SMD entry", DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT,
                                             lastParentId, SMD_ENTRY ) );
    result.push_back( buildRuleTreeNodeData( "SMD to plane plus",
                                             DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, lastParentId,
                                             SMD_TO_PLANE_PLUS ) );
    result.push_back( buildRuleTreeNodeData( "Vias under SMD",
                                             DRC_RULE_EDITOR_ITEM_TYPE::CONSTRAINT, lastParentId,
                                             VIAS_UNDER_SMD ) );

    return result;
}


/**
 * Checks if a node with the given name exists in the rule tree or its child nodes.
 *
 * @param aRuleTreeNode The node to check.
 * @param aTargetName The name of the target node to search for.
 * @return true if the node exists, false otherwise.
 */
bool nodeExists( const RULE_TREE_NODE& aRuleTreeNode, const wxString& aTargetName )
{
    if( aRuleTreeNode.m_nodeName == aTargetName )
    {
        return true;
    }

    for( const auto& child : aRuleTreeNode.m_childNodes )
    {
        if( nodeExists( child, aTargetName ) )
        {
            return true;
        }
    }

    return false;
}


/**
 * Checks if a node with the given name exists in a list of rule tree nodes.
 *
 * @param aRuleTreeNodes The list of rule tree nodes to check.
 * @param aTargetName The name of the target node to search for.
 * @return true if the node exists, false otherwise.
 */
bool nodeExists( const std::vector<RULE_TREE_NODE>& aRuleTreeNodes, const wxString& aTargetName )
{
    for( const auto& node : aRuleTreeNodes )
    {
        if( nodeExists( node, aTargetName ) )
        {
            return true;
        }
    }

    return false;
}


RULE_TREE_NODE DIALOG_DRC_RULE_EDITOR::buildRuleTreeNode( RULE_TREE_ITEM_DATA* aRuleTreeItemData )
{
    RULE_TREE_ITEM_DATA* treeItemData;
    RULE_TREE_NODE*      nodeDetail = getRuleTreeNodeInfo( aRuleTreeItemData->GetNodeId() );

    if( nodeDetail->m_nodeType == CONSTRAINT )
    {
        treeItemData = aRuleTreeItemData;
    }
    else
    {
        treeItemData = dynamic_cast<RULE_TREE_ITEM_DATA*>(
                m_ruleTreeCtrl->GetItemData( aRuleTreeItemData->GetParentTreeItemId() ) );
        nodeDetail = getRuleTreeNodeInfo( treeItemData->GetNodeId() );
    }

    m_nodeId++;

    wxString nodeName = nodeDetail->m_nodeName + " 1";

    int  loop = 2;
    bool check = false;

    do
    {
        check = false;

        if( nodeExists( m_ruleTreeNodeDatas, nodeName ) )
        {
            nodeName = nodeDetail->m_nodeName + wxString::Format( " %d", loop );
            loop++;
            check = true;
        }
    } while( check );

    RULE_TREE_NODE newRuleNode = buildRuleTreeNodeData(
            nodeName.ToStdString(), RULE, nodeDetail->m_nodeId,
            static_cast<DRC_RULE_EDITOR_CONSTRAINT_NAME>( nodeDetail->m_nodeTypeMap.value_or( 0 ) ),
            {}, m_nodeId );

    auto nodeType = static_cast<DRC_RULE_EDITOR_CONSTRAINT_NAME>(
            newRuleNode.m_nodeTypeMap.value_or( -1 ) );
    DRC_RE_BASE_CONSTRAINT_DATA clearanceData( m_nodeId, nodeDetail->m_nodeData->GetId(),
                                               newRuleNode.m_nodeName );

    switch( nodeType )
    {
    case DRC_RULE_EDITOR_CONSTRAINT_NAME::VIA_STYLE:
        newRuleNode.m_nodeData =
                std::make_shared<DRC_RE_VIA_STYLE_CONSTRAINT_DATA>( clearanceData );
        break;

    case DRC_RULE_EDITOR_CONSTRAINT_NAME::MINIMUM_TEXT_HEIGHT_AND_THICKNESS:
        newRuleNode.m_nodeData =
                std::make_shared<DRC_RE_MINIMUM_TEXT_HEIGHT_THICKNESS_CONSTRAINT_DATA>(
                        clearanceData );
        break;

    case DRC_RULE_EDITOR_CONSTRAINT_NAME::ROUTING_DIFF_PAIR:
        newRuleNode.m_nodeData =
                std::make_shared<DRC_RE_ROUTING_DIFF_PAIR_CONSTRAINT_DATA>( clearanceData );
        break;

    case DRC_RULE_EDITOR_CONSTRAINT_NAME::ROUTING_WIDTH:
        newRuleNode.m_nodeData =
                std::make_shared<DRC_RE_ROUTING_WIDTH_CONSTRAINT_DATA>( clearanceData );
        break;

    case DRC_RULE_EDITOR_CONSTRAINT_NAME::PARALLEL_LIMIT:
        newRuleNode.m_nodeData =
                std::make_shared<DRC_RE_PARALLEL_LIMIT_CONSTRAINT_DATA>( clearanceData );
        break;

    case DRC_RULE_EDITOR_CONSTRAINT_NAME::PERMITTED_LAYERS:
        newRuleNode.m_nodeData =
                std::make_shared<DRC_RE_PERMITTED_LAYERS_CONSTRAINT_DATA>( clearanceData );
        break;

    case DRC_RULE_EDITOR_CONSTRAINT_NAME::ALLOWED_ORIENTATION:
        newRuleNode.m_nodeData =
                std::make_shared<DRC_RE_ALLOWED_ORIENTATION_CONSTRAINT_DATA>( clearanceData );
        break;

    case DRC_RULE_EDITOR_CONSTRAINT_NAME::CORNER_STYLE:
        newRuleNode.m_nodeData =
                std::make_shared<DRC_RE_CORNER_STYLE_CONSTRAINT_DATA>( clearanceData );
        break;

    case DRC_RULE_EDITOR_CONSTRAINT_NAME::SMD_ENTRY:
        newRuleNode.m_nodeData =
                std::make_shared<DRC_RE_SMD_ENTRY_CONSTRAINT_DATA>( clearanceData );
        break;

    default:
    {
        if( DRC_RULE_EDITOR_UTILS::IsNumericInputType( nodeType ) )
            newRuleNode.m_nodeData =
                    std::make_shared<DRC_RE_NUMERIC_INPUT_CONSTRAINT_DATA>( clearanceData );
        else if( DRC_RULE_EDITOR_UTILS::IsBoolInputType( nodeType ) )
            newRuleNode.m_nodeData =
                    std::make_shared<DRC_RE_BOOL_INPUT_CONSTRAINT_DATA>( clearanceData );
    }
    break;
    }

    newRuleNode.m_nodeData->SetIsNew( true );

    m_ruleTreeNodeDatas.push_back( newRuleNode );

    return newRuleNode;
}


RULE_TREE_NODE* DIALOG_DRC_RULE_EDITOR::getRuleTreeNodeInfo( const int& aNodeId )
{
    auto it = std::find_if( m_ruleTreeNodeDatas.begin(), m_ruleTreeNodeDatas.end(),
                            [aNodeId]( const RULE_TREE_NODE& node )
                            {
                                return node.m_nodeId == aNodeId;
                            } );

    if( it != m_ruleTreeNodeDatas.end() )
    {
        return &( *it ); // Return pointer to the found node
    }
    else
        return nullptr;
}


void DIALOG_DRC_RULE_EDITOR::saveRule( int aNodeId )
{
    if( !m_ruleEditorPanel->GetIsValidationSucceeded() )
    {
        std::string validationMessage = m_ruleEditorPanel->GetValidationMessage();

        DisplayErrorMessage( this, validationMessage );
    }
    else
    {
        RULE_TREE_ITEM_DATA* itemData =
                dynamic_cast<RULE_TREE_ITEM_DATA*>( m_ruleTreeCtrl->GetItemData(
                        GetCurrentlySelectedRuleTreeItemData()->GetTreeItemId() ) );

        if( itemData )
        {
            RULE_TREE_NODE* nodeDetail = getRuleTreeNodeInfo( itemData->GetNodeId() );
            UpdateRuleTypeTreeItemData( itemData );
        }

        SetControlsEnabled( true );
    }
}


void DIALOG_DRC_RULE_EDITOR::closeRuleEntryView( int aNodeId )
{
    SetControlsEnabled( true );
}


bool DIALOG_DRC_RULE_EDITOR::validateRuleName( int aNodeId, wxString aRuleName )
{
    auto it = std::find_if( m_ruleTreeNodeDatas.begin(), m_ruleTreeNodeDatas.end(),
                            [aNodeId, aRuleName]( const RULE_TREE_NODE& node )
                            {
                                return node.m_nodeName == aRuleName && node.m_nodeId != aNodeId
                                       && node.m_nodeType == RULE;
                            } );

    if( it != m_ruleTreeNodeDatas.end() )
    {
        return false;
    }

    return true;
}


bool DIALOG_DRC_RULE_EDITOR::deleteTreeNodeData( const int& aNodeId )
{
    size_t initial_size = m_ruleTreeNodeDatas.size();

    m_ruleTreeNodeDatas.erase( std::remove_if( m_ruleTreeNodeDatas.begin(),
                                               m_ruleTreeNodeDatas.end(),
                                               [aNodeId]( const RULE_TREE_NODE& node )
                                               {
                                                   return node.m_nodeId == aNodeId;
                                               } ),
                               m_ruleTreeNodeDatas.end() );

    if( m_ruleTreeNodeDatas.size() < initial_size )
        return true;
    else
        return false;
}


RULE_TREE_NODE DIALOG_DRC_RULE_EDITOR::buildRuleTreeNodeData(
        const std::string& aName, const DRC_RULE_EDITOR_ITEM_TYPE& aNodeType,
        const std::optional<int>& aParentId,
        const std::optional<DRC_RULE_EDITOR_CONSTRAINT_NAME>& aConstraintType,
        const std::vector<RULE_TREE_NODE>& aChildNodes, const std::optional<int>& id )
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

    RULE_EDITOR_DATA_BASE baseData;
    baseData.SetId( newId );

    if( aParentId )
    {
        baseData.SetParentId( *aParentId );
    }

    return { .m_nodeId = m_nodeId,
             .m_nodeName = aName,
             .m_nodeType = aNodeType,
             .m_nodeTypeMap = aConstraintType,
             .m_childNodes = aChildNodes,
             .m_nodeData = std::make_shared<RULE_EDITOR_DATA_BASE>( baseData ) };
}