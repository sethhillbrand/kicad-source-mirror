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

#include <bitmaps.h>
#include <launch_ext.h>

#include <algorithm>
#include <wx/log.h>

#include <wx/dragimag.h>     // For wxDragImage
#include <wx/wx.h>
#include <wx/dnd.h>
#include <wx/dcmemory.h> // Include for wxMemoryDC
#include <wx/dcclient.h> // Include for wxClientDC if needed
#include <wx/dcbuffer.h> // Include for double-buffered drawing
#include <wx/bitmap.h>

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
    m_treeCtrl = new RuleTreeCtrl( treeCtrlPanel );
    m_treeCtrl->SetFont( KIUI::GetControlFont( this ) );

    // Adjust the tree control's window style to remove the border
    long treeCtrlFlags = m_treeCtrl->GetWindowStyleFlag();
    treeCtrlFlags = ( treeCtrlFlags & ~wxBORDER_MASK ) | wxBORDER_NONE;
    m_treeCtrl->SetWindowStyleFlag( treeCtrlFlags );

    // Add the tree control to its sizer
    treeCtrlSizer->Add( m_treeCtrl, 1, wxEXPAND | wxBOTTOM, 5 );

    // Create a sizer for the bitmap buttons
    wxBoxSizer* actionButtonsSizer = new wxBoxSizer( wxHORIZONTAL );

    // Create the bitmap buttons
    m_addRuleButton = new wxBitmapButton( treeCtrlPanel, wxID_ANY, KiBitmapBundle( BITMAPS::small_plus ),
                                wxDefaultPosition, wxDefaultSize, 0 );
    m_copyRuleButton = new wxBitmapButton( treeCtrlPanel, wxID_ANY, KiBitmapBundle( BITMAPS::copy ),
                                wxDefaultPosition, wxDefaultSize, 0 );
    m_moveTreeItemUpButton = new wxBitmapButton( treeCtrlPanel, wxID_ANY, KiBitmapBundle( BITMAPS::small_up ),
                                wxDefaultPosition, wxDefaultSize, 0 );
    m_moveTreeItemDownButton = new wxBitmapButton( treeCtrlPanel, wxID_ANY, KiBitmapBundle( BITMAPS::small_down ),
                                wxDefaultPosition, wxDefaultSize, 0 );
    m_deleteRuleButton = new wxBitmapButton( treeCtrlPanel, wxID_ANY, KiBitmapBundle( BITMAPS::small_trash ),
                                wxDefaultPosition, wxDefaultSize, 0 );

    // Add the "Add" button to the left side
    actionButtonsSizer->Add( m_addRuleButton, 0, wxLEFT | wxRIGHT, 5 );

    // Add the "New" button next to the "Add" button
    actionButtonsSizer->Add( m_copyRuleButton, 0, wxLEFT | wxRIGHT, 5 );

    // Add the "Move" buttons (Move Up, Move Down) centered
    actionButtonsSizer->Add( m_moveTreeItemUpButton, 0, wxLEFT | wxRIGHT, 5 );
    actionButtonsSizer->Add( m_moveTreeItemDownButton, 0, wxLEFT | wxRIGHT, 5 );

    // Add a spacer between the "Move" buttons and the "Delete" button
    actionButtonsSizer->AddStretchSpacer( 1 ); // Spacer between the Move and Delete buttons

    // Add the "Delete" button at the end
    actionButtonsSizer->Add( m_deleteRuleButton, 0, wxRIGHT, 5 );

    // Add the action buttons sizer to the tree control sizer
    treeCtrlSizer->Add( actionButtonsSizer, 0, wxBOTTOM | wxEXPAND, 5 );

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
    m_filterSearch->Bind( wxEVT_COMMAND_TEXT_UPDATED, &RULE_EDITOR_DIALOG_BASE::onFilterSearch, this );
    m_treeCtrl->Bind( wxEVT_TREE_ITEM_RIGHT_CLICK, &RULE_EDITOR_DIALOG_BASE::onTreeItemRightClick, this );
    m_treeCtrl->Bind( wxEVT_TREE_SEL_CHANGED, &RULE_EDITOR_DIALOG_BASE::onSelectionChanged, this );
    m_treeCtrl->Bind( wxEVT_LEFT_DOWN, &RULE_EDITOR_DIALOG_BASE::onTreeItemLeftDown, this );
    m_treeCtrl->Bind( wxEVT_MOTION, &RULE_EDITOR_DIALOG_BASE::onTreeItemMouseMotion, this );
    m_treeCtrl->Bind( wxEVT_LEFT_UP, &RULE_EDITOR_DIALOG_BASE::onTreeItemLeftUp, this );

    m_addRuleButton->Bind( wxEVT_BUTTON, &RULE_EDITOR_DIALOG_BASE::onNewRule, this );
    m_copyRuleButton->Bind( wxEVT_BUTTON, &RULE_EDITOR_DIALOG_BASE::onDuplicateRule, this );
    m_moveTreeItemUpButton->Bind( wxEVT_BUTTON, &RULE_EDITOR_DIALOG_BASE::onMoveUpRule, this );
    m_moveTreeItemDownButton->Bind( wxEVT_BUTTON, &RULE_EDITOR_DIALOG_BASE::onMoveDownRule, this );
    m_deleteRuleButton->Bind( wxEVT_BUTTON, &RULE_EDITOR_DIALOG_BASE::onDeleteRule, this );
}


void RULE_EDITOR_DIALOG_BASE::finishInitialization()
{
    finishDialogSettings();
}


RULE_EDITOR_DIALOG_BASE::~RULE_EDITOR_DIALOG_BASE()
{
    m_treeCtrl->Unbind( wxEVT_TREE_ITEM_RIGHT_CLICK, &RULE_EDITOR_DIALOG_BASE::onTreeItemRightClick, this );
    m_treeCtrl->Unbind( wxEVT_TREE_SEL_CHANGED, &RULE_EDITOR_DIALOG_BASE::onSelectionChanged, this );
    m_filterSearch->Unbind( wxEVT_COMMAND_TEXT_UPDATED, &RULE_EDITOR_DIALOG_BASE::onFilterSearch, this );
    m_treeCtrl->Unbind( wxEVT_LEFT_DOWN, &RULE_EDITOR_DIALOG_BASE::onTreeItemLeftDown, this );
    m_treeCtrl->Unbind( wxEVT_MOTION, &RULE_EDITOR_DIALOG_BASE::onTreeItemMouseMotion, this );
    m_treeCtrl->Unbind( wxEVT_LEFT_UP, &RULE_EDITOR_DIALOG_BASE::onTreeItemLeftUp, this );

    m_addRuleButton->Unbind( wxEVT_BUTTON, &RULE_EDITOR_DIALOG_BASE::onNewRule, this );
    m_copyRuleButton->Unbind( wxEVT_BUTTON, &RULE_EDITOR_DIALOG_BASE::onDuplicateRule, this );
    m_moveTreeItemUpButton->Unbind( wxEVT_BUTTON, &RULE_EDITOR_DIALOG_BASE::onMoveUpRule, this );
    m_moveTreeItemDownButton->Unbind( wxEVT_BUTTON, &RULE_EDITOR_DIALOG_BASE::onMoveDownRule, this );
    m_deleteRuleButton->Unbind( wxEVT_BUTTON, &RULE_EDITOR_DIALOG_BASE::onDeleteRule, this );
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


void RULE_EDITOR_DIALOG_BASE::InitTreeItems( RuleTreeCtrl* aTreeCtrl, const std::vector<RuleTreeNode>& aRuleTreeNodes )
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
    saveTreeState( m_treeCtrl->GetRootItem(), m_defaultTreeItems[0].node_id );

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


void RULE_EDITOR_DIALOG_BASE::PopulateTreeCtrl( RuleTreeCtrl* aTreeCtrl, const std::vector<RuleTreeNode>& aRuleTreeNodes, 
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

    // Update the layout and refresh
    Layout();
    Refresh();
}


wxTreeItemId RULE_EDITOR_DIALOG_BASE::appendTreeItem( RuleTreeCtrl* aTreeCtrl, const RuleTreeNode& aRuleTreeNode, wxTreeItemId aParentTreeItemId)
{
    wxTreeItemId currentTreeItemId = aTreeCtrl->AppendItem( aParentTreeItemId, aRuleTreeNode.node_name );

    // Attach TreeItemData to the tree item
    RuleTreeItemData* itemData = new RuleTreeItemData( aRuleTreeNode.node_id, aParentTreeItemId, currentTreeItemId );
    aTreeCtrl->SetItemData( currentTreeItemId, itemData );

     // Optionally expand the root node
    aTreeCtrl->Expand( currentTreeItemId );

    return currentTreeItemId;
}


void RULE_EDITOR_DIALOG_BASE::AppendNewTreeItem( RuleTreeCtrl* aTreeCtrl, const RuleTreeNode& aRuleTreeNode, wxTreeItemId aParentTreeItemId)
{
    wxTreeItemId currentTreeItemId = appendTreeItem( aTreeCtrl, aRuleTreeNode, aParentTreeItemId );

    auto it = m_originalChildren.find( aRuleTreeNode.node_data->GetParentId() );

    if( it != m_originalChildren.end() )
    {        
        std::vector<unsigned int>& existingChildren = std::get<1>(it->second);
        existingChildren.push_back( aRuleTreeNode.node_id );

        m_originalChildren[aRuleTreeNode.node_id] = { aRuleTreeNode.node_name, {}, currentTreeItemId  };
    }  

    aTreeCtrl->SelectItem( currentTreeItemId );

    SetControlsEnabled( false );
}


void RULE_EDITOR_DIALOG_BASE::UpdateTreeItemText( RuleTreeCtrl* aTreeCtrl, wxTreeItemId aItemId, wxString aItemText )
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

    m_contextMenuActiveTreeItemData = ruleTreeItemData;

    updateTreeActionButtonsState( ruleTreeItemData );

    wxMenu menu;

    if( m_enableAddRule )
        menu.Append( 1, "New Rule" );   

    if( m_enableDuplicateRule )
        menu.Append( 2, "Duplicate Rule" );

    if( m_enableDeleteRule )
        menu.Append( 3, "Delete Rule" );

    if( VerifyTreeContextMenuOptionToEnable( ruleTreeItemData, RULE_EDITOR_TREE_CONTEXT_OPT::MOVE_UP ) && m_enableMoveUp )
        menu.Append( 4, "Move Up" );

    if( VerifyTreeContextMenuOptionToEnable( ruleTreeItemData, RULE_EDITOR_TREE_CONTEXT_OPT::MOVE_DOWN ) && m_enableMoveDown )
        menu.Append( 5, "Move Down" );

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
     if( !m_enableMoveUp )
        return; // Exit if moving up is not allowed

    wxTreeItemId selectedItem = m_treeCtrl->GetSelection();
    if( !selectedItem.IsOk() )
        return;

    wxTreeItemId parent = m_treeCtrl->GetItemParent( selectedItem );

    if( !parent.IsOk() )
        return; // Exit if the item has no parent (root-level item)

    wxTreeItemIdValue cookie;
    wxTreeItemId      current = m_treeCtrl->GetFirstChild( parent, cookie );
    wxTreeItemId      previous;
    wxTreeItemId      beforePrevious;

    // Traverse siblings to find the previous and the one before it
    while( current.IsOk() && current != selectedItem )
    {
        beforePrevious = previous;
        previous = current;
        current = m_treeCtrl->GetNextChild( parent, cookie );
    }

    if( !previous.IsOk() )
    {
        wxLogMessage( "No previous sibling; item is already at the top." );
        return; // No previous sibling means item is already at the top
    }

    wxTreeItemId insertPosition = beforePrevious.IsOk() ? beforePrevious : wxTreeItemId();

    // Insert the selected item at the calculated position
    wxTreeItemId newItem = m_treeCtrl->InsertItem( parent, insertPosition,
                                                   m_treeCtrl->GetItemText( selectedItem ) );

    // Transfer item data
    wxTreeItemData* itemData = m_treeCtrl->GetItemData( selectedItem );
    if( itemData )
    {
        m_treeCtrl->SetItemData( newItem, itemData );
        m_treeCtrl->SetItemData( selectedItem, nullptr ); // Detach data from the old item
    }

    // Move children from the old item to the new one
    moveChildrenOnTreeItemDrag( m_treeCtrl, selectedItem, newItem );

    // Delete the old item
    m_treeCtrl->Delete( selectedItem );

    // Select the newly moved item
    m_treeCtrl->SelectItem( newItem );
}


void RULE_EDITOR_DIALOG_BASE::onMoveDownRule( wxCommandEvent& aEvent )
{
    if( !m_enableMoveDown )
        return; // Exit if moving down is not allowed

    wxTreeItemId selectedItem = m_treeCtrl->GetSelection();
    if( !selectedItem.IsOk() )
        return;

    wxTreeItemId      parent = m_treeCtrl->GetItemParent( selectedItem );
    wxTreeItemIdValue cookie;
    wxTreeItemId      current = m_treeCtrl->GetFirstChild( parent, cookie );

    while( current.IsOk() && current != selectedItem )
    {
        current = m_treeCtrl->GetNextChild( parent, cookie );
    }

    wxTreeItemId next = m_treeCtrl->GetNextChild( parent, cookie );
    if( next.IsOk() )
    {
        wxString        itemText = m_treeCtrl->GetItemText( selectedItem );
        wxTreeItemData* itemData = m_treeCtrl->GetItemData( selectedItem );

        wxTreeItemId newItem = m_treeCtrl->InsertItem( parent, next, itemText );

        if( itemData )
        {
            m_treeCtrl->SetItemData( newItem, itemData );
            m_treeCtrl->SetItemData( selectedItem, nullptr ); // Detach data
        }

        moveChildrenOnTreeItemDrag( m_treeCtrl, selectedItem, newItem );
        m_treeCtrl->Delete( selectedItem );
        m_treeCtrl->SelectItem( newItem );
    }
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

    updateTreeActionButtonsState( ruleTreeItemData );
}


void RULE_EDITOR_DIALOG_BASE::onFilterSearch( wxCommandEvent& aEvent )
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

    restoreTree( nullptr, m_defaultTreeItems[0].node_id );

    wxTreeItemId root = m_treeCtrl->GetRootItem();

    // Apply the filter
    if ( root.IsOk() )
    {
        filterTree( root, searchStr );
    }
}

void RULE_EDITOR_DIALOG_BASE::saveTreeState( const wxTreeItemId& item, const unsigned int& aNodeId )
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
        saveTreeState( child, childId );

        child = m_treeCtrl->GetNextChild( item, cookie );
    }

    // Store the current item in the map
    m_originalChildren[aNodeId] = { itemText, children, item };
}


void RULE_EDITOR_DIALOG_BASE::restoreTree( const wxTreeItemId& parent, const unsigned int& aNodeId )
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
        restoreTree(newItem, childId);
    }

    m_treeCtrl->Expand( newItem );

   
}


bool RULE_EDITOR_DIALOG_BASE::filterTree( const wxTreeItemId& item, const wxString& filter )
{
    bool matches = m_treeCtrl->GetItemText( item ).Lower().Contains( filter );

    wxTreeItemIdValue cookie;
    wxTreeItemId      child = m_treeCtrl->GetFirstChild( item, cookie );
    bool              hasVisibleChildren = false;

    // List to store items for deletion
    std::vector<wxTreeItemId> itemsToDelete;

    while( child.IsOk() )
    {
        if( filterTree( child, filter ) )
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


void RULE_EDITOR_DIALOG_BASE::DeleteTreeItem( wxTreeItemId aItemId, const unsigned int& aNodeId )
{
    m_treeCtrl->Delete( aItemId );
    m_originalChildren.erase( aNodeId );
}


void RULE_EDITOR_DIALOG_BASE::onTreeItemLeftDown( wxMouseEvent& event )
{
    m_startPos = event.GetPosition();
    int          flags;
    wxTreeItemId item = m_treeCtrl->HitTest( m_startPos, flags );

    if( item.IsOk() )
    {
        m_draggedItem = item;

        // Create a custom drag image
        wxString   text = m_treeCtrl->GetItemText( item );
        wxMemoryDC dc;
        wxBitmap   bitmap( 200, 30 );
        dc.SelectObject( bitmap );
        dc.SetBackground( *wxWHITE_BRUSH );
        dc.Clear();
        dc.SetFont( *wxNORMAL_FONT );
        dc.DrawText( text, 5, 5 );
        dc.SelectObject( wxNullBitmap );

        m_dragImage = new wxDragImage( bitmap );
        m_isDragging = false; // Reset drag state
    }

    event.Skip();
}


void RULE_EDITOR_DIALOG_BASE::onTreeItemMouseMotion( wxMouseEvent& event )
{
    if( event.Dragging() && m_draggedItem.IsOk() && m_dragImage )
    {
        wxPoint currentPos = event.GetPosition();

        if( !m_isDragging )
        {
            m_dragImage->BeginDrag( wxPoint( 0, 0 ), m_treeCtrl, true );
            m_dragImage->Show();
            m_isDragging = true; // Set flag that dragging has started
        }
        else
        {
            m_dragImage->Move( currentPos );
        }
    }

    event.Skip();
}


void RULE_EDITOR_DIALOG_BASE::onTreeItemLeftUp( wxMouseEvent& event )
{
    if( m_draggedItem.IsOk() )
    {
        if( m_dragImage )
        {
            m_dragImage->Hide();
            m_dragImage->EndDrag();
            delete m_dragImage;
            m_dragImage = nullptr;
            m_isDragging = false; // Reset drag state
        }

        // Handle item drop logic here
        wxPoint      pos = event.GetPosition();
        int          flags;
        wxTreeItemId targetItem = m_treeCtrl->HitTest( pos, flags );

        // Debug: Check if target item is valid
        if( !targetItem.IsOk() )
        {
            wxLogMessage( "Invalid target item." );
            return; // Exit if target is not valid
        }

        wxTreeItemId draggedParent = m_treeCtrl->GetItemParent( m_draggedItem );
        wxTreeItemId targetParent = m_treeCtrl->GetItemParent( targetItem );

        // Debug: Check if dragged and target items are in the same parent
        if( draggedParent != targetParent )
        {
            wxLogMessage( "Invalid move: Items are not in the same parent. Parent ID of dragged "
                          "item: %p, Target parent ID: %p",
                          draggedParent.GetID(), targetParent.GetID() );
            return; // Exit if parents do not match
        }

        if( draggedParent == targetParent )
        {
            wxTreeItemId      newItem;
            wxTreeItemIdValue cookie;

            // Case 1: Drop directly on the first child (or before the first child)
            if( flags & wxTREE_HITTEST_ONITEMLABEL
                && targetItem == m_treeCtrl->GetFirstChild( targetParent, cookie ) )
            {
                // If the drop is on the first child, make it the first child
                newItem = m_treeCtrl->PrependItem( targetParent,
                                                   m_treeCtrl->GetItemText( m_draggedItem ) );
            }
            else
            {
                // Case 2: Insert before or after the target item based on previous or next sibling
                wxTreeItemId current = m_treeCtrl->GetFirstChild( targetParent, cookie );
                wxTreeItemId previous;

                // Traverse through siblings to find the right place to insert
                while( current.IsOk() && current != targetItem )
                {
                    previous = current;
                    current = m_treeCtrl->GetNextChild( targetParent, cookie );
                }

                if( !previous.IsOk() )
                {
                    // If the target is the first child, insert as first child
                    newItem = m_treeCtrl->PrependItem( targetParent,
                                                       m_treeCtrl->GetItemText( m_draggedItem ) );
                }
                else
                {
                    // Check if there is a next sibling (to insert after the target)
                    wxTreeItemId nextSibling = m_treeCtrl->GetNextChild( targetParent, cookie );

                    if( nextSibling.IsOk() )
                    {
                        // Insert after the target item
                        newItem = m_treeCtrl->InsertItem(
                                targetParent, previous, m_treeCtrl->GetItemText( m_draggedItem ) );
                    }
                    else
                    {
                        // If no next sibling, insert as the last child
                        newItem = m_treeCtrl->AppendItem(
                                targetParent, m_treeCtrl->GetItemText( m_draggedItem ) );
                    }
                }
            }

            // Handle item data transfer
            wxTreeItemData* itemData = m_treeCtrl->GetItemData( m_draggedItem );

            if( itemData )
            {
                m_treeCtrl->SetItemData( newItem, itemData );
                m_treeCtrl->SetItemData( m_draggedItem, nullptr ); // Detach data
            }

            // Recursively move children to the new item
            moveChildrenOnTreeItemDrag( m_treeCtrl, m_draggedItem, newItem );

            // Delete the original dragged item
            m_treeCtrl->Delete( m_draggedItem );
        }
        else
        {
            wxLogMessage( "Invalid move: Items can only be moved within the same parent." );
        }

        m_draggedItem = wxTreeItemId(); // Reset dragged item
    }

    event.Skip();
}



void RULE_EDITOR_DIALOG_BASE::moveChildrenOnTreeItemDrag( wxTreeCtrl* tree, wxTreeItemId srcItem,
                                                          wxTreeItemId destItem )
{
    wxTreeItemIdValue cookie;
    wxTreeItemId      child = tree->GetFirstChild( srcItem, cookie );

    while( child.IsOk() )
    {
        // Create a new child under the destination item
        wxTreeItemId newChild = tree->AppendItem( destItem, tree->GetItemText( child ) );

        // Copy any associated data
        tree->SetItemData( newChild, tree->GetItemData( child ) );

        // Recurse for the current child
        moveChildrenOnTreeItemDrag( tree, child, newChild );

        // Move to the next sibling
        child = tree->GetNextChild( srcItem, cookie );
    }
}

void RULE_EDITOR_DIALOG_BASE::updateTreeItemMoveOptionState()
{
    wxTreeItemId selectedItem = m_treeCtrl->GetSelection();

    // Reset flags if no selection
    m_enableMoveUp = false;
    m_enableMoveDown = false;

    if( !selectedItem.IsOk() )
    {
        return;
    }

    // If the root item is selected, no need to check move options
    if( selectedItem == m_treeCtrl->GetRootItem() )
    {
        return;
    }

    wxTreeItemId      parent = m_treeCtrl->GetItemParent( selectedItem );
    wxTreeItemIdValue cookie;
    wxTreeItemId      firstChild = m_treeCtrl->GetFirstChild( parent, cookie );
    wxTreeItemId      lastChild = firstChild;

    // Traverse to find the last child
    while( lastChild.IsOk() )
    {
        wxTreeItemId next = m_treeCtrl->GetNextChild( parent, cookie );
        if( !next.IsOk() )
            break;
        lastChild = next;
    }

    // Update flags based on the position
    m_enableMoveUp = ( selectedItem != firstChild );
    m_enableMoveDown = ( selectedItem != lastChild );
}


void RULE_EDITOR_DIALOG_BASE::updateTreeActionButtonsState( RuleTreeItemData* ruleTreeItemData )
{
    if( VerifyTreeContextMenuOptionToEnable( ruleTreeItemData,
                                             RULE_EDITOR_TREE_CONTEXT_OPT::ADD_RULE ) )
        m_enableAddRule = true;

    if( VerifyTreeContextMenuOptionToEnable( ruleTreeItemData,
                                             RULE_EDITOR_TREE_CONTEXT_OPT::DUPLICATE_RULE ) )
        m_enableDuplicateRule = true;

    if( VerifyTreeContextMenuOptionToEnable( ruleTreeItemData,
                                             RULE_EDITOR_TREE_CONTEXT_OPT::DELETE_RULE ) )
        m_enableDeleteRule = true;

    updateTreeItemMoveOptionState();

    m_addRuleButton->Enable( m_enableAddRule );
    m_copyRuleButton->Enable( m_enableDuplicateRule );
    m_moveTreeItemUpButton->Enable( m_enableMoveUp );
    m_moveTreeItemDownButton->Enable( m_enableMoveDown );
    m_deleteRuleButton->Enable( m_enableDeleteRule );
}