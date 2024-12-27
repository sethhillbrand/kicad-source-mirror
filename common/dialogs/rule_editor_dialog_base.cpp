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
#include <widgets/resettable_panel.h>
#include <widgets/wx_infobar.h>
#include <widgets/wx_panel.h>
#include <widgets/ui_common.h>

#include <wx/button.h>
#include <wx/grid.h>
#include <wx/sizer.h>
#include <wx/treectrl.h>
#include <wx/listctrl.h>
#include <wx/stc/stc.h>
#include <wx/menu.h>

#include <paths.h>

#include <launch_ext.h>

#include <algorithm>
#include <wx/log.h>


#include <dialogs/rule_editor_dialog_base.h>

static wxSearchCtrl* CreateTextFilterBox( wxWindow* aParent, const wxString& aDescriptiveText )
{
    wxSearchCtrl* search_widget = new wxSearchCtrl( aParent, wxID_ANY );

    search_widget->ShowSearchButton( false );
    search_widget->ShowCancelButton( true );

    search_widget->SetDescriptiveText( aDescriptiveText );

#ifdef __WXGTK__
    // wxSearchCtrl vertical height is not calculated correctly on some GTK setups
    // See https://gitlab.com/kicad/code/kicad/-/issues/9019
    search_widget->SetMinSize( wxSize( -1, aParent->GetTextExtent( wxT( "qb" ) ).y + 10 ) );
#endif

    return search_widget;
}

BEGIN_EVENT_TABLE( RULE_EDITOR_DIALOG_BASE, DIALOG_SHIM )
    EVT_MENU(1, RULE_EDITOR_DIALOG_BASE::onNewRule)
    EVT_MENU(2, RULE_EDITOR_DIALOG_BASE::onDuplicateRule)
    EVT_MENU(3, RULE_EDITOR_DIALOG_BASE::onDeleteRule)
    EVT_MENU(4, RULE_EDITOR_DIALOG_BASE::onMoveUpRule)
    EVT_MENU(5, RULE_EDITOR_DIALOG_BASE::onMoveDownRule)
END_EVENT_TABLE()


RULE_EDITOR_DIALOG_BASE::RULE_EDITOR_DIALOG_BASE( wxWindow* aParent, const wxString& aTitle, const wxSize& aInitialSize ) :
        DIALOG_SHIM( aParent, wxID_ANY, aTitle, wxDefaultPosition, aInitialSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER ),
        m_title( aTitle ), 
        m_treeCtrl( nullptr ), 
        m_selectedTreeItemData( nullptr ),
        m_contextMenuActiveTreeItemData( nullptr ),
        m_previouslySelectedTreeItemId( nullptr )
{
   
   // Create the main vertical sizer
    wxBoxSizer* mainSizer = new wxBoxSizer( wxVERTICAL );
    SetSizer( mainSizer );

    // Create a horizontal sizer for the info bar
    wxBoxSizer* infoBarSizer = new wxBoxSizer( wxHORIZONTAL );

    // Create the info bar and add it to the horizontal sizer
    m_infoBar = new WX_INFOBAR( this );
    infoBarSizer->Add( m_infoBar, 1, wxEXPAND, 0 );

    // Add the info bar sizer to the main sizer
    mainSizer->Add( infoBarSizer, 0, wxEXPAND, 0 );

    // Create the horizontal content sizer
    m_contentSizer = new wxBoxSizer( wxHORIZONTAL );

    // Create the tree control panel
    WX_PANEL* treeCtrlPanel = new WX_PANEL( this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                            wxBORDER_NONE | wxTAB_TRAVERSAL );
    treeCtrlPanel->SetBorders( true, true, true, true );
    wxBoxSizer* treeCtrlSizer = new wxBoxSizer( wxVERTICAL );
    treeCtrlPanel->SetSizer( treeCtrlSizer );

    // Add a search text box above the tree control
    m_filterSearch = CreateTextFilterBox( treeCtrlPanel, _( "Type filter text" ) );
    treeCtrlSizer->Add( m_filterSearch, 0, wxEXPAND | wxBOTTOM,
                        5 ); // Add search box with bottom margin

    // Create the tree control and set its font
    m_treeCtrl = new wxTreeCtrl( treeCtrlPanel );
    m_treeCtrl->SetFont( KIUI::GetControlFont( this ) );

    // Adjust the tree control's window style to remove the border
    long treeCtrlFlags = m_treeCtrl->GetWindowStyleFlag();
    treeCtrlFlags = ( treeCtrlFlags & ~wxBORDER_MASK ) | wxBORDER_NONE;
    m_treeCtrl->SetWindowStyleFlag( treeCtrlFlags );

    // Add the tree control to its sizer
    treeCtrlSizer->Add( m_treeCtrl, 1, wxEXPAND | wxBOTTOM, 5 );

    // Add the tree panel to the content sizer
    m_contentSizer->Add( treeCtrlPanel, 3, wxEXPAND | wxALL, 5 );

    // Create the dynamic content panel
    m_contentPanel = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE );
    m_contentPanel->SetBackgroundColour( *wxLIGHT_GREY );

    // Adjust minimum size for the dynamic panel (optional customization)
    // m_contentPanel->SetMinSize(aParent->FromDIP(wxSize(-1, 300)));

    // Add the dynamic panel to the content sizer
    m_contentSizer->Add( m_contentPanel, 7, wxEXPAND | wxLEFT, 5 );

    // Add the content sizer to the main sizer
    mainSizer->Add( m_contentSizer, 1, wxEXPAND, 0 );

    // We normally save the dialog size and position based on its class-name.  This class
    // substitutes the title so that each distinctly-titled dialog can have its own saved
    // size and position.
    m_hash_key = aTitle;

     // Bind the context menu event
    m_treeCtrl->Bind( wxEVT_TREE_ITEM_RIGHT_CLICK, &RULE_EDITOR_DIALOG_BASE::onTreeItemRightClick, this );
    m_treeCtrl->Bind( wxEVT_TREE_SEL_CHANGED, &RULE_EDITOR_DIALOG_BASE::onSelectionChanged, this );
    m_filterSearch->Bind( wxEVT_COMMAND_TEXT_UPDATED, &RULE_EDITOR_DIALOG_BASE::OnFilterSearch, this );
}


void RULE_EDITOR_DIALOG_BASE::finishInitialization()
{
    finishDialogSettings();
}


RULE_EDITOR_DIALOG_BASE::~RULE_EDITOR_DIALOG_BASE()
{
    m_treeCtrl->Unbind( wxEVT_TREE_ITEM_RIGHT_CLICK, &RULE_EDITOR_DIALOG_BASE::onTreeItemRightClick, this );
    m_treeCtrl->Unbind( wxEVT_TREE_SEL_CHANGED, &RULE_EDITOR_DIALOG_BASE::onSelectionChanged, this );
    m_filterSearch->Unbind( wxEVT_COMMAND_TEXT_UPDATED, &RULE_EDITOR_DIALOG_BASE::OnFilterSearch, this );
}


bool RULE_EDITOR_DIALOG_BASE::TransferDataToWindow()
{
    finishInitialization();

    // Call TransferDataToWindow() only once:
    // this is enough on wxWidgets 3.1
    if( !DIALOG_SHIM::TransferDataToWindow() )
        return false;

    return true;
}


bool RULE_EDITOR_DIALOG_BASE::TransferDataFromWindow()
{
    bool ret = true;

    // Call TransferDataFromWindow() only once:
    // this is enough on wxWidgets 3.1
    if( !DIALOG_SHIM::TransferDataFromWindow() )
        ret = false;

    return ret;
}


RULE_EDITOR_DIALOG_BASE* RULE_EDITOR_DIALOG_BASE::GetDialog( wxWindow* aParent )
{
    while( aParent )
    {
        if( RULE_EDITOR_DIALOG_BASE* parentDialog = dynamic_cast<RULE_EDITOR_DIALOG_BASE*>( aParent ) )
            return parentDialog;

        aParent = aParent->GetParent();
    }

    return nullptr;
}


void RULE_EDITOR_DIALOG_BASE::InitTreeItems( wxTreeCtrl* aTreeCtrl, const std::vector<RuleTreeNode>& aRuleTreeNodes )
{
    if( aRuleTreeNodes.empty() )
        return;

    m_defaultTreeItems = aRuleTreeNodes;

    // Add the root node (first element in the vector)
    wxTreeItemId rootId = aTreeCtrl->AddRoot( aRuleTreeNodes[0].node_name );

     // Attach TreeItemData to the root tree item
    RuleTreeItemData* itemData = new RuleTreeItemData( aRuleTreeNodes[0].node_id, nullptr, rootId );
    aTreeCtrl->SetItemData( rootId, itemData );

    std::vector<RuleTreeNode> childNodes;
    getChildNodes( aRuleTreeNodes, aRuleTreeNodes[0].node_id, childNodes );

    // Add all children recursively
    for( const auto& child : childNodes )
    {
        PopulateTreeCtrl( aTreeCtrl, aRuleTreeNodes, child, rootId );
    }

    // Optionally expand the root node
    aTreeCtrl->Expand( rootId );

    m_originalChildren.clear();
    SaveTreeState( m_treeCtrl->GetRootItem(), m_defaultTreeItems[0].node_id );

    aTreeCtrl->SelectItem( rootId );
}


void RULE_EDITOR_DIALOG_BASE::getChildNodes( const std::vector<RuleTreeNode>& nodes, int parentId, std::vector<RuleTreeNode>& result )
{
    // Filter nodes that match the parentId
    std::vector<RuleTreeNode> filteredNodes;
    std::copy_if( nodes.begin(), nodes.end(), std::back_inserter( filteredNodes ),
                  [&parentId]( const RuleTreeNode& node )
                  {
                      return node.node_data->GetParentId() == parentId;
                  } );

    if( filteredNodes.size() > 0 )
    {
        // Add the filtered nodes to the result
        result.insert( result.end(), filteredNodes.begin(), filteredNodes.end() );  
    }
}


// Example usage
std::vector<RuleTreeNode> RULE_EDITOR_DIALOG_BASE::getAllChildrenByParentId( const std::vector<RuleTreeNode>& nodes, unsigned int parentId)
{
    std::vector<RuleTreeNode> result;
    getChildNodes(nodes, parentId, result);
    return result;
}


void RULE_EDITOR_DIALOG_BASE::PopulateTreeCtrl( wxTreeCtrl* aTreeCtrl, const std::vector<RuleTreeNode>& aRuleTreeNodes, 
                                                const RuleTreeNode& aRuleTreeNode, wxTreeItemId aParentTreeItemId )
{
    wxTreeItemId currentId = appendTreeItem( aTreeCtrl, aRuleTreeNode, aParentTreeItemId );

    std::vector<RuleTreeNode> childNodes;
    getChildNodes( aRuleTreeNodes, aRuleTreeNode.node_id, childNodes );

    // Recursively add children 
    for( const auto& child : childNodes )
    {
        PopulateTreeCtrl( aTreeCtrl, aRuleTreeNodes, child, currentId );
    }

    // Optionally expand the current node
    aTreeCtrl->Expand( currentId );
}


void RULE_EDITOR_DIALOG_BASE::SetContentPanel( wxPanel* aContentPanel )
{
    if( m_contentPanel )
    {
        // Remove and destroy the existing panel
        m_contentSizer->Detach( m_contentPanel );
        m_contentPanel->Destroy();
    }

    // Set the new panel
    m_contentPanel = aContentPanel;
    m_contentSizer->Add( m_contentPanel, 7, wxEXPAND | wxLEFT, 5 );

    //// Adjust the main sizer and layout
    //m_contentSizer->Fit(this);          // Adjust the sizer to fit the containing dialog
    //m_contentSizer->SetSizeHints(this); // Ensure minimum size hints are updated

    // Update the layout and refresh
    Layout();
    Refresh();
}


wxTreeItemId RULE_EDITOR_DIALOG_BASE::appendTreeItem( wxTreeCtrl* aTreeCtrl, const RuleTreeNode& aRuleTreeNode, wxTreeItemId aParentTreeItemId)
{
    wxTreeItemId currentTreeItemId = aTreeCtrl->AppendItem( aParentTreeItemId, aRuleTreeNode.node_name );

    // Attach TreeItemData to the tree item
    RuleTreeItemData* itemData = new RuleTreeItemData( aRuleTreeNode.node_id, aParentTreeItemId, currentTreeItemId );
    aTreeCtrl->SetItemData( currentTreeItemId, itemData );

     // Optionally expand the root node
    aTreeCtrl->Expand( currentTreeItemId );

    return currentTreeItemId;
}


void RULE_EDITOR_DIALOG_BASE::AppendNewTreeItem( wxTreeCtrl* aTreeCtrl, const RuleTreeNode& aRuleTreeNode, wxTreeItemId aParentTreeItemId)
{
    wxTreeItemId currentTreeItemId = appendTreeItem( aTreeCtrl, aRuleTreeNode, aParentTreeItemId );

    auto it = m_originalChildren.find( aRuleTreeNode.node_data->GetParentId() );

    if( it != m_originalChildren.end() )
    {        
        std::vector<unsigned int>& existingChildren = std::get<1>(it->second);
        existingChildren.push_back( aRuleTreeNode.node_id );

        m_originalChildren[aRuleTreeNode.node_id] = { aRuleTreeNode.node_name, {}, currentTreeItemId  };
    }    

    //for( const auto& entry : m_originalChildren )
    //{
    //    const wxTreeItemId& treeItemId =  std::get<2>( entry.second ); // Get the wxTreeItemId from the tuple

    //    // Check if this is the excluded item, if not, add it to the list
    //    if( treeItemId != currentTreeItemId )
    //    {
    //        aTreeCtrl->DisableItem( treeItemId );
    //    }
    //}
    //aTreeCtrl->Disable();
    aTreeCtrl->SelectItem( currentTreeItemId );

    SetControlsEnabled( false );
}


void RULE_EDITOR_DIALOG_BASE::UpdateTreeItemText( wxTreeCtrl* aTreeCtrl, wxTreeItemId aItemId, wxString aItemText )
{
    aTreeCtrl->SetItemText( aItemId, aItemText );
}


void RULE_EDITOR_DIALOG_BASE::onTreeItemRightClick( wxTreeEvent& aEvent )
{
    wxTreeItemId item = aEvent.GetItem();

    if( !item.IsOk() )
        return; // No valid item 

    m_treeCtrl->SelectItem( item );

     // Get the associated TreeData
    RuleTreeItemData* ruleTreeItemData= dynamic_cast<RuleTreeItemData*>( m_treeCtrl->GetItemData( item ) );

    if( !ruleTreeItemData )
        return;

    if( !CanShowContextMenu( ruleTreeItemData ) )
        return;

    /*if( m_selectedTreeItemData )
        UpdateRuleTypeTreeItemData( m_selectedTreeItemData );*/

    m_contextMenuActiveTreeItemData = ruleTreeItemData;

    wxMenu menu;
    menu.Append( 1, "New Rule" );

    if( CheckAndAppendRuleOperations( ruleTreeItemData ) )
    {
        menu.Append( 2, "Duplicate Rule" );
        menu.Append( 3, "Delete Rule" );
        menu.AppendSeparator();
        menu.Append( 4, "Move Up" );
        menu.Append( 5, "Move Down" );
    }

    // Show the menu
    PopupMenu( &menu );
}


void RULE_EDITOR_DIALOG_BASE::onNewRule( wxCommandEvent& aEvent )
{
    AddNewRule( m_contextMenuActiveTreeItemData );
}


void RULE_EDITOR_DIALOG_BASE::onDuplicateRule( wxCommandEvent& aEvent )
{
    DuplicateRule( m_contextMenuActiveTreeItemData );
}


void RULE_EDITOR_DIALOG_BASE::onDeleteRule( wxCommandEvent& aEvent )
{
    wxLogMessage( "Delete Rule action triggered." );
}


void RULE_EDITOR_DIALOG_BASE::onMoveUpRule( wxCommandEvent& aEvent )
{
    wxLogMessage( "Move Up action triggered." );
}


void RULE_EDITOR_DIALOG_BASE::onMoveDownRule( wxCommandEvent& aEvent )
{
    wxLogMessage( "Move Down action triggered." );
}


void RULE_EDITOR_DIALOG_BASE::onSelectionChanged( wxTreeEvent& aEvent )
{
    wxTreeItemId selectedItem = aEvent.GetItem();

      // Get the associated TreeData
    RuleTreeItemData* ruleTreeItemData = dynamic_cast<RuleTreeItemData*>( m_treeCtrl->GetItemData( selectedItem ) );

    if( !ruleTreeItemData )
        return;

    if( m_selectedTreeItemData )
        m_previouslySelectedTreeItemId = m_selectedTreeItemData->GetTreeItemId();

    m_selectedTreeItemData = ruleTreeItemData;

    TreeItemSelectionChanged( m_selectedTreeItemData );
}


void RULE_EDITOR_DIALOG_BASE::OnFilterSearch( wxCommandEvent& aEvent )
{
    const auto searchStr = aEvent.GetString().Lower();

     // Capture the selected item
    wxTreeItemId selectedItem = m_treeCtrl->GetSelection();
    unsigned int selectedNodeId = wxNOT_FOUND;

    if( selectedItem.IsOk() )
    {
        RuleTreeItemData* itemData = dynamic_cast<RuleTreeItemData*>( m_treeCtrl->GetItemData( selectedItem ) );

        if( itemData )
        {
            selectedNodeId = itemData->GetNodeId();
        }
    }

    m_treeCtrl->DeleteAllItems();

    RestoreTree( nullptr, m_defaultTreeItems[0].node_id );

    wxTreeItemId root = m_treeCtrl->GetRootItem();

    // Apply the filter
    if ( root.IsOk() )
    {
        FilterTree( root, searchStr );
    }
}

void RULE_EDITOR_DIALOG_BASE::SaveTreeState( const wxTreeItemId& item, const unsigned int& aNodeId )
{
    wxString          itemText = m_treeCtrl->GetItemText( item );
    RuleTreeItemData* itemData = dynamic_cast<RuleTreeItemData*>( m_treeCtrl->GetItemData( item ) );
    std::vector<unsigned int> children;

    wxTreeItemIdValue cookie;
    wxTreeItemId      child = m_treeCtrl->GetFirstChild( item, cookie );
    int               index = 0;

    while( child.IsOk() )
    {
        RuleTreeItemData* childItemData = dynamic_cast<RuleTreeItemData*>( m_treeCtrl->GetItemData( child ) );
        unsigned int childId = childItemData->GetNodeId();

        //std::string childId = itemId + "/" + std::to_string( index++ );
        children.push_back( childId );

        // Recursively save the state of the child
        SaveTreeState( child, childId );

        child = m_treeCtrl->GetNextChild( item, cookie );
    }

    // Store the current item in the map
    m_originalChildren[aNodeId] = { itemText, children, item };
}


void RULE_EDITOR_DIALOG_BASE::RestoreTree( const wxTreeItemId& parent, const unsigned int& aNodeId )
{
    auto it = m_originalChildren.find( aNodeId );
    if (it == m_originalChildren.end())
        return;

    const auto& [itemText, children, treeItemId ] = it->second;

    wxTreeItemId newItem;

    if( parent )
    {
        // Add the restored item to the parent
        newItem = m_treeCtrl->AppendItem( parent, itemText );
        wxTreeItemId& treeItemId = std::get<2>( it->second );
        treeItemId = newItem;
    }
    else
    {
        newItem = m_treeCtrl->AddRoot( itemText );
        wxTreeItemId& treeItemId = std::get<2>( it->second );
        treeItemId = newItem;
    }

    RuleTreeItemData* itemData = new RuleTreeItemData( aNodeId, parent, newItem );
    m_treeCtrl->SetItemData( newItem, itemData );

    // Recursively restore each child
    for (const auto& childId : children)
    {
        RestoreTree(newItem, childId);
    }

    m_treeCtrl->Expand( newItem );

   
}


bool RULE_EDITOR_DIALOG_BASE::FilterTree( const wxTreeItemId& item, const wxString& filter )
{
    bool matches = m_treeCtrl->GetItemText( item ).Lower().Contains( filter );

    wxTreeItemIdValue cookie;
    wxTreeItemId      child = m_treeCtrl->GetFirstChild( item, cookie );
    bool              hasVisibleChildren = false;

    // List to store items for deletion
    std::vector<wxTreeItemId> itemsToDelete;

    while( child.IsOk() )
    {
        if( FilterTree( child, filter ) )
        {
            hasVisibleChildren = true;
        }
        else
        {
            // Collect child for deletion
            itemsToDelete.push_back( child );
        }

        child = m_treeCtrl->GetNextChild( item, cookie );
    }

    // Delete non-matching children after iteration
    for( const auto& id : itemsToDelete )
    {
        m_treeCtrl->Delete( id );
    }

    return matches || hasVisibleChildren;
}

void RULE_EDITOR_DIALOG_BASE::SetControlsEnabled( bool aEnable )
{
    m_treeCtrl->Enable( aEnable );
    m_filterSearch->Enable( aEnable );
}