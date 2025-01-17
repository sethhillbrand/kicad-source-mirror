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
#include <wx/log.h>
#include <wx/dragimag.h> // For wxDragImage
#include <wx/wx.h>
#include <wx/dnd.h>
#include <wx/dcmemory.h> // Include for wxMemoryDC
#include <wx/dcclient.h> // Include for wxClientDC if needed
#include <wx/dcbuffer.h> // Include for double-buffered drawing
#include <wx/bitmap.h>

#include <confirm.h>
#include <paths.h>
#include <bitmaps.h>
#include <launch_ext.h>
#include <algorithm>
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


RULE_EDITOR_DIALOG_BASE::RULE_EDITOR_DIALOG_BASE( wxWindow* aParent, const wxString& aTitle,
                                                  const wxSize& aInitialSize ) :
        DIALOG_SHIM( aParent, wxID_ANY, aTitle, wxDefaultPosition, aInitialSize,
                     wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER ),
        m_title( aTitle ), m_ruleTreeCtrl( nullptr ), m_selectedTreeItemData( nullptr ),
        m_previouslySelectedTreeItemId( nullptr ), m_draggedItemStartPos( 0, 0 ),
        m_dragImage( nullptr ), m_isDragging( false ), m_enableMoveUp( false ),
        m_enableMoveDown( false ), m_enableAddRule( false ), m_enableDuplicateRule( false ),
        m_enableDeleteRule( false ), m_preventSelectionChange( false )
{
    wxBoxSizer* mainSizer = new wxBoxSizer( wxVERTICAL );
    SetSizer( mainSizer );

    wxBoxSizer* infoBarSizer = new wxBoxSizer( wxHORIZONTAL );

    m_infoBar = new WX_INFOBAR( this );
    infoBarSizer->Add( m_infoBar, 1, wxEXPAND, 0 );

    // Add the info bar sizer to the main sizer
    mainSizer->Add( infoBarSizer, 0, wxEXPAND, 0 );

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
    m_ruleTreeCtrl = new wxTreeCtrl( treeCtrlPanel );
    m_ruleTreeCtrl->SetFont( KIUI::GetControlFont( this ) );

    // Adjust the tree control's window style to remove the border
    long treeCtrlFlags = m_ruleTreeCtrl->GetWindowStyleFlag();
    treeCtrlFlags = ( treeCtrlFlags & ~wxBORDER_MASK ) | wxBORDER_NONE;
    m_ruleTreeCtrl->SetWindowStyleFlag( treeCtrlFlags );

    // Add the tree control to its sizer
    treeCtrlSizer->Add( m_ruleTreeCtrl, 1, wxEXPAND | wxBOTTOM, 5 );

    // Create a sizer for the bitmap buttons
    wxBoxSizer* actionButtonsSizer = new wxBoxSizer( wxHORIZONTAL );

    // Create the bitmap buttons
    m_addRuleButton =
            new wxBitmapButton( treeCtrlPanel, wxID_ANY, KiBitmapBundle( BITMAPS::small_plus ),
                                wxDefaultPosition, wxDefaultSize, 0 );
    m_copyRuleButton = new wxBitmapButton( treeCtrlPanel, wxID_ANY, KiBitmapBundle( BITMAPS::copy ),
                                           wxDefaultPosition, wxDefaultSize, 0 );
    m_moveTreeItemUpButton =
            new wxBitmapButton( treeCtrlPanel, wxID_ANY, KiBitmapBundle( BITMAPS::small_up ),
                                wxDefaultPosition, wxDefaultSize, 0 );
    m_moveTreeItemDownButton =
            new wxBitmapButton( treeCtrlPanel, wxID_ANY, KiBitmapBundle( BITMAPS::small_down ),
                                wxDefaultPosition, wxDefaultSize, 0 );
    m_deleteRuleButton =
            new wxBitmapButton( treeCtrlPanel, wxID_ANY, KiBitmapBundle( BITMAPS::small_trash ),
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

    // Add the dynamic panel to the content sizer
    m_contentSizer->Add( m_contentPanel, 7, wxEXPAND | wxLEFT, 5 );

    // Add the content sizer to the main sizer
    mainSizer->Add( m_contentSizer, 1, wxEXPAND, 0 );

    // We normally save the dialog size and position based on its class-name.  This class
    // substitutes the title so that each distinctly-titled dialog can have its own saved
    // size and position.
    m_hash_key = aTitle;

    // Bind the context menu event
    m_filterSearch->Bind( wxEVT_COMMAND_TEXT_UPDATED, &RULE_EDITOR_DIALOG_BASE::onFilterSearch,
                          this );
    m_ruleTreeCtrl->Bind( wxEVT_TREE_ITEM_RIGHT_CLICK,
                          &RULE_EDITOR_DIALOG_BASE::onRuleTreeItemRightClick, this );
    m_ruleTreeCtrl->Bind( wxEVT_TREE_SEL_CHANGED,
                          &RULE_EDITOR_DIALOG_BASE::onRuleTreeItemSelectionChanged, this );
    m_ruleTreeCtrl->Bind( wxEVT_LEFT_DOWN, &RULE_EDITOR_DIALOG_BASE::onRuleTreeItemLeftDown, this );
    m_ruleTreeCtrl->Bind( wxEVT_MOTION, &RULE_EDITOR_DIALOG_BASE::onRuleTreeItemMouseMotion, this );
    m_ruleTreeCtrl->Bind( wxEVT_LEFT_UP, &RULE_EDITOR_DIALOG_BASE::onRuleTreeItemLeftUp, this );

    m_addRuleButton->Bind( wxEVT_BUTTON, &RULE_EDITOR_DIALOG_BASE::onNewRuleOptionClick, this );
    m_copyRuleButton->Bind( wxEVT_BUTTON, &RULE_EDITOR_DIALOG_BASE::onDuplicateRuleOptionClick,
                            this );
    m_moveTreeItemUpButton->Bind( wxEVT_BUTTON, &RULE_EDITOR_DIALOG_BASE::onMoveUpRuleOptionClick,
                                  this );
    m_moveTreeItemDownButton->Bind( wxEVT_BUTTON,
                                    &RULE_EDITOR_DIALOG_BASE::onMoveDownRuleOptionClick, this );
    m_deleteRuleButton->Bind( wxEVT_BUTTON, &RULE_EDITOR_DIALOG_BASE::onDeleteRuleOptionClick,
                              this );
}


void RULE_EDITOR_DIALOG_BASE::finishInitialization()
{
    finishDialogSettings();
}


RULE_EDITOR_DIALOG_BASE::~RULE_EDITOR_DIALOG_BASE()
{
    if( m_selectedTreeItemData )
    {
        delete m_selectedTreeItemData;
        m_selectedTreeItemData = nullptr;
    }

    if( m_dragImage )
    {
        delete m_dragImage;
        m_dragImage = nullptr;
    }

    m_filterSearch->Unbind( wxEVT_COMMAND_TEXT_UPDATED, &RULE_EDITOR_DIALOG_BASE::onFilterSearch,
                            this );
    m_ruleTreeCtrl->Unbind( wxEVT_TREE_ITEM_RIGHT_CLICK,
                            &RULE_EDITOR_DIALOG_BASE::onRuleTreeItemRightClick, this );
    m_ruleTreeCtrl->Unbind( wxEVT_TREE_SEL_CHANGED,
                            &RULE_EDITOR_DIALOG_BASE::onRuleTreeItemSelectionChanged, this );
    m_ruleTreeCtrl->Unbind( wxEVT_LEFT_DOWN, &RULE_EDITOR_DIALOG_BASE::onRuleTreeItemLeftDown,
                            this );
    m_ruleTreeCtrl->Unbind( wxEVT_MOTION, &RULE_EDITOR_DIALOG_BASE::onRuleTreeItemMouseMotion,
                            this );
    m_ruleTreeCtrl->Unbind( wxEVT_LEFT_UP, &RULE_EDITOR_DIALOG_BASE::onRuleTreeItemLeftUp, this );

    m_addRuleButton->Unbind( wxEVT_BUTTON, &RULE_EDITOR_DIALOG_BASE::onNewRuleOptionClick, this );
    m_copyRuleButton->Unbind( wxEVT_BUTTON, &RULE_EDITOR_DIALOG_BASE::onDuplicateRuleOptionClick,
                              this );
    m_moveTreeItemUpButton->Unbind( wxEVT_BUTTON, &RULE_EDITOR_DIALOG_BASE::onMoveUpRuleOptionClick,
                                    this );
    m_moveTreeItemDownButton->Unbind( wxEVT_BUTTON,
                                      &RULE_EDITOR_DIALOG_BASE::onMoveDownRuleOptionClick, this );
    m_deleteRuleButton->Unbind( wxEVT_BUTTON, &RULE_EDITOR_DIALOG_BASE::onDeleteRuleOptionClick,
                                this );
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
        if( RULE_EDITOR_DIALOG_BASE* parentDialog =
                    dynamic_cast<RULE_EDITOR_DIALOG_BASE*>( aParent ) )
            return parentDialog;

        aParent = aParent->GetParent();
    }

    return nullptr;
}


void RULE_EDITOR_DIALOG_BASE::InitRuleTreeItems( const std::vector<RULE_TREE_NODE>& aRuleTreeNodes )
{
    if( aRuleTreeNodes.empty() )
        return;

    m_defaultTreeItems = aRuleTreeNodes;

    wxTreeItemId rootId = m_ruleTreeCtrl->AddRoot( aRuleTreeNodes[0].m_nodeName );

    RULE_TREE_ITEM_DATA* itemData =
            new RULE_TREE_ITEM_DATA( aRuleTreeNodes[0].m_nodeId, nullptr, rootId );
    m_ruleTreeCtrl->SetItemData( rootId, itemData );

    std::vector<RULE_TREE_NODE> childNodes;
    getRuleTreeChildNodes( aRuleTreeNodes, aRuleTreeNodes[0].m_nodeId, childNodes );

    for( const auto& child : childNodes )
    {
        populateRuleTreeCtrl( aRuleTreeNodes, child, rootId );
    }

    m_ruleTreeCtrl->Expand( rootId );

    m_treeHistoryData.clear();
    saveRuleTreeState( m_ruleTreeCtrl->GetRootItem(), m_defaultTreeItems[0].m_nodeId );

    m_ruleTreeCtrl->SelectItem( rootId );
}


void RULE_EDITOR_DIALOG_BASE::SetContentPanel( wxPanel* aContentPanel )
{
    if( m_contentPanel )
    {
        m_contentSizer->Detach( m_contentPanel );
        m_contentPanel->Destroy();
    }

    m_contentPanel = aContentPanel;
    m_contentSizer->Add( m_contentPanel, 7, wxEXPAND | wxLEFT, 5 );

    Layout();
    Refresh();
}


void RULE_EDITOR_DIALOG_BASE::AppendNewRuleTreeItem( const RULE_TREE_NODE& aRuleTreeNode,
                                                     wxTreeItemId          aParentTreeItemId )
{
    wxTreeItemId currentTreeItemId = appendRuleTreeItem( aRuleTreeNode, aParentTreeItemId );

    auto it = m_treeHistoryData.find( aRuleTreeNode.m_nodeData->GetParentId() );

    if( it != m_treeHistoryData.end() )
    {
        std::vector<int>& existingChildren = std::get<1>( it->second );
        existingChildren.push_back( aRuleTreeNode.m_nodeId );

        m_treeHistoryData[aRuleTreeNode.m_nodeId] = { aRuleTreeNode.m_nodeName,
                                                      {},
                                                      currentTreeItemId };
    }

    m_ruleTreeCtrl->SelectItem( currentTreeItemId );
    SetControlsEnabled( false );
}


void RULE_EDITOR_DIALOG_BASE::UpdateRuleTreeItemText( wxTreeItemId aItemId, wxString aItemText )
{
    m_ruleTreeCtrl->SetItemText( aItemId, aItemText );
}


void RULE_EDITOR_DIALOG_BASE::SetControlsEnabled( bool aEnable )
{
    m_ruleTreeCtrl->Enable( aEnable );
    m_filterSearch->Enable( aEnable );

    updateRuleTreeActionButtonsState( m_selectedTreeItemData );
}


void RULE_EDITOR_DIALOG_BASE::DeleteRuleTreeItem( wxTreeItemId aItemId, const int& aNodeId )
{
    m_ruleTreeCtrl->Delete( aItemId );
    m_treeHistoryData.erase( aNodeId );
}


void RULE_EDITOR_DIALOG_BASE::populateRuleTreeCtrl(
        const std::vector<RULE_TREE_NODE>& aRuleTreeNodes, const RULE_TREE_NODE& aRuleTreeNode,
        wxTreeItemId aParentTreeItemId )
{
    wxTreeItemId currentId = appendRuleTreeItem( aRuleTreeNode, aParentTreeItemId );

    std::vector<RULE_TREE_NODE> childNodes;
    getRuleTreeChildNodes( aRuleTreeNodes, aRuleTreeNode.m_nodeId, childNodes );

    for( const auto& child : childNodes )
    {
        populateRuleTreeCtrl( aRuleTreeNodes, child, currentId );
    }

    m_ruleTreeCtrl->Expand( currentId );
}


void RULE_EDITOR_DIALOG_BASE::onRuleTreeItemRightClick( wxTreeEvent& aEvent )
{
    wxTreeItemId item = aEvent.GetItem();

    if( !item.IsOk() )
        return;

    m_ruleTreeCtrl->SelectItem( item );

    RULE_TREE_ITEM_DATA* ruleTreeItemData =
            dynamic_cast<RULE_TREE_ITEM_DATA*>( m_ruleTreeCtrl->GetItemData( item ) );

    if( !ruleTreeItemData )
        return;

    updateRuleTreeActionButtonsState( ruleTreeItemData );

    wxMenu menu;

    if( m_enableAddRule )
        KIUI::AddMenuItem( &menu, ID_NEWRULE, _( "New Rule" ), KiBitmap( BITMAPS::small_plus ) );

    if( m_enableAddRule )
        KIUI::AddMenuItem( &menu, ID_COPYRULE, _( "Copy Rule" ), KiBitmap( BITMAPS::copy ) );

    if( m_enableDeleteRule )
        KIUI::AddMenuItem( &menu, ID_DELETERULE, _( "Delete Rule" ),
                           KiBitmap( BITMAPS::small_trash ) );

    if( VerifyRuleTreeContextMenuOptionToEnable( ruleTreeItemData,
                                                 RULE_EDITOR_TREE_CONTEXT_OPT::MOVE_UP )
        && m_enableMoveUp )
        KIUI::AddMenuItem( &menu, ID_MOVEUP, _( "Move Up" ), KiBitmap( BITMAPS::small_up ) );

    if( VerifyRuleTreeContextMenuOptionToEnable( ruleTreeItemData,
                                                 RULE_EDITOR_TREE_CONTEXT_OPT::MOVE_DOWN )
        && m_enableMoveDown )
        KIUI::AddMenuItem( &menu, ID_MOVEDOWN, _( "Move Down" ), KiBitmap( BITMAPS::small_down ) );

    menu.Bind( wxEVT_COMMAND_MENU_SELECTED,
               [&]( wxCommandEvent& aCmd )
               {
                   switch( aCmd.GetId() )
                   {
                   case ID_NEWRULE: onNewRuleOptionClick( aCmd ); break;

                   case ID_COPYRULE: onDuplicateRuleOptionClick( aCmd ); break;

                   case ID_DELETERULE: onDeleteRuleOptionClick( aCmd ); break;

                   case ID_MOVEUP: onMoveUpRuleOptionClick( aCmd ); break;

                   case ID_MOVEDOWN: onMoveDownRuleOptionClick( aCmd ); break;
                   default: aCmd.Skip();
                   }
               } );

    PopupMenu( &menu );
}


void RULE_EDITOR_DIALOG_BASE::onRuleTreeItemSelectionChanged( wxTreeEvent& aEvent )
{
    wxTreeItemId selectedItem = aEvent.GetItem();

    RULE_TREE_ITEM_DATA* selectedItemData =
            dynamic_cast<RULE_TREE_ITEM_DATA*>( m_ruleTreeCtrl->GetItemData( selectedItem ) );

    if( !selectedItemData )
        return;

    if( m_selectedTreeItemData )
    {
        if( m_selectedTreeItemData->GetNodeId() == selectedItemData->GetNodeId() )
        {
            m_selectedTreeItemData = selectedItemData;
            updateRuleTreeActionButtonsState( selectedItemData );
            return;
        }

        m_previouslySelectedTreeItemId = m_selectedTreeItemData->GetTreeItemId();
    }

    m_selectedTreeItemData = selectedItemData;

    RuleTreeItemSelectionChanged( m_selectedTreeItemData );

    updateRuleTreeActionButtonsState( selectedItemData );
}


void RULE_EDITOR_DIALOG_BASE::onNewRuleOptionClick( wxCommandEvent& aEvent )
{
    AddNewRule( m_selectedTreeItemData );
}


void RULE_EDITOR_DIALOG_BASE::onDuplicateRuleOptionClick( wxCommandEvent& aEvent )
{
    DuplicateRule( m_selectedTreeItemData );
}


void RULE_EDITOR_DIALOG_BASE::onDeleteRuleOptionClick( wxCommandEvent& aEvent )
{
    RemoveRule( m_selectedTreeItemData->GetNodeId() );
}


void RULE_EDITOR_DIALOG_BASE::onMoveUpRuleOptionClick( wxCommandEvent& aEvent )
{
    if( !m_enableMoveUp )
        return; 

    wxTreeItemId selectedItem = m_ruleTreeCtrl->GetSelection();
    if( !selectedItem.IsOk() )
        return;

    wxTreeItemId parent = m_ruleTreeCtrl->GetItemParent( selectedItem );

    if( !parent.IsOk() )
        return; // Exit if the item has no parent (root-level item)

    wxTreeItemIdValue cookie;
    wxTreeItemId      current = m_ruleTreeCtrl->GetFirstChild( parent, cookie );
    wxTreeItemId      previous;
    wxTreeItemId      beforePrevious;

    while( current.IsOk() && current != selectedItem )
    {
        beforePrevious = previous;
        previous = current;
        current = m_ruleTreeCtrl->GetNextChild( parent, cookie );
    }

    if( !previous.IsOk() )
    {
        return;
    }

    wxTreeItemId insertPosition = beforePrevious.IsOk() ? beforePrevious : wxTreeItemId();

    // Insert the selected item at the calculated position
    wxTreeItemId newItem = m_ruleTreeCtrl->InsertItem(
            parent, insertPosition, m_ruleTreeCtrl->GetItemText( selectedItem ) );

    // Move children from the old item to the new one
    moveRuleTreeItemChildrensTooOnDrag( selectedItem, newItem );

    // Transfer item data
    wxTreeItemData* itemData = m_ruleTreeCtrl->GetItemData( selectedItem );

    if( itemData )
    {
        RULE_TREE_ITEM_DATA* ruleTreeItemData = dynamic_cast<RULE_TREE_ITEM_DATA*>( itemData );
        ruleTreeItemData->SetTreeItemId( newItem );

        m_ruleTreeCtrl->SetItemData( newItem, itemData );
        m_ruleTreeCtrl->SetItemData( selectedItem, nullptr ); // Detach data from the old item

        saveRuleTreeState( newItem, ruleTreeItemData->GetNodeId() );

        m_ruleTreeCtrl->SelectItem( newItem );

        m_ruleTreeCtrl->Expand( newItem );
        m_ruleTreeCtrl->ExpandAllChildren( newItem );
    }

    // Delete the old item
    m_ruleTreeCtrl->DeleteChildren( selectedItem );
    m_ruleTreeCtrl->Delete( selectedItem );
}


void RULE_EDITOR_DIALOG_BASE::onMoveDownRuleOptionClick( wxCommandEvent& aEvent )
{
    if( !m_enableMoveDown )
        return; // Exit if moving down is not allowed

    wxTreeItemId selectedItem = m_ruleTreeCtrl->GetSelection();

    if( !selectedItem.IsOk() )
        return;

    wxTreeItemId      parent = m_ruleTreeCtrl->GetItemParent( selectedItem );
    wxTreeItemIdValue cookie;
    wxTreeItemId      current = m_ruleTreeCtrl->GetFirstChild( parent, cookie );

    while( current.IsOk() && current != selectedItem )
    {
        current = m_ruleTreeCtrl->GetNextChild( parent, cookie );
    }

    wxTreeItemId next = m_ruleTreeCtrl->GetNextChild( parent, cookie );

    if( !next.IsOk() )
    {
        return; // No previous sibling means item is already at the top
    }

    wxString        itemText = m_ruleTreeCtrl->GetItemText( selectedItem );
    wxTreeItemData* itemData = m_ruleTreeCtrl->GetItemData( selectedItem );

    wxTreeItemId newItem = m_ruleTreeCtrl->InsertItem( parent, next, itemText );

    moveRuleTreeItemChildrensTooOnDrag( selectedItem, newItem );

    if( itemData )
    {
        RULE_TREE_ITEM_DATA* ruleTreeItemData = dynamic_cast<RULE_TREE_ITEM_DATA*>( itemData );
        ruleTreeItemData->SetTreeItemId( newItem );

        m_ruleTreeCtrl->SetItemData( newItem, itemData );
        m_ruleTreeCtrl->SetItemData( selectedItem, nullptr ); // Detach data

        saveRuleTreeState( newItem, ruleTreeItemData->GetNodeId() );

        m_ruleTreeCtrl->SelectItem( newItem );

        m_ruleTreeCtrl->Expand( newItem );
    }

    m_ruleTreeCtrl->DeleteChildren( selectedItem );
    m_ruleTreeCtrl->Delete( selectedItem );
}


void RULE_EDITOR_DIALOG_BASE::onRuleTreeItemLeftDown( wxMouseEvent& aEvent )
{
    m_draggedItemStartPos = aEvent.GetPosition();
    int          flags;
    wxTreeItemId item = m_ruleTreeCtrl->HitTest( m_draggedItemStartPos, flags );

    if( item.IsOk() )
    {
        m_draggedItem = item;

        // Create a custom drag image
        wxString   text = m_ruleTreeCtrl->GetItemText( item );
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

    aEvent.Skip();
}


void RULE_EDITOR_DIALOG_BASE::onRuleTreeItemMouseMotion( wxMouseEvent& aEvent )
{
    if( aEvent.Dragging() && m_draggedItem.IsOk() && m_dragImage )
    {
        wxPoint currentPos = aEvent.GetPosition();

        if( !m_isDragging )
        {
            m_dragImage->BeginDrag( wxPoint( 0, 0 ), m_ruleTreeCtrl, true );
            m_dragImage->Show();
            m_isDragging = true; // Set flag that dragging has started
        }
        else
        {
            m_dragImage->Move( currentPos );
        }
    }

    aEvent.Skip();
}


void RULE_EDITOR_DIALOG_BASE::onRuleTreeItemLeftUp( wxMouseEvent& aEvent )
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
        wxPoint      pos = aEvent.GetPosition();
        int          flags;
        wxTreeItemId targetItem = m_ruleTreeCtrl->HitTest( pos, flags );

        // Debug: Check if target item is valid
        if( !targetItem.IsOk() )
        {
            DisplayErrorMessage( this, "Invalid target item." );
            return; // Exit if target is not valid
        }

        wxTreeItemId draggedParent = m_ruleTreeCtrl->GetItemParent( m_draggedItem );
        wxTreeItemId targetParent = m_ruleTreeCtrl->GetItemParent( targetItem );

        // Debug: Check if dragged and target items are in the same parent
        if( draggedParent != targetParent )
        {
            DisplayErrorMessage( this,
                                 "Invalid move: Items can only be moved within the same parent." );
            return; // Exit if parents do not match
        }

        if( draggedParent == targetParent )
        {
            wxTreeItemId      newItem;
            wxTreeItemIdValue cookie;

            // Case 1: Drop directly on the first child (or before the first child)
            if( flags & wxTREE_HITTEST_ONITEMLABEL
                && targetItem == m_ruleTreeCtrl->GetFirstChild( targetParent, cookie ) )
            {
                // If the drop is on the first child, make it the first child
                newItem = m_ruleTreeCtrl->PrependItem(
                        targetParent, m_ruleTreeCtrl->GetItemText( m_draggedItem ) );
            }
            else
            {
                // Case 2: Insert before or after the target item based on previous or next sibling
                wxTreeItemId current = m_ruleTreeCtrl->GetFirstChild( targetParent, cookie );
                wxTreeItemId previous;

                // Traverse through siblings to find the right place to insert
                while( current.IsOk() && current != targetItem )
                {
                    previous = current;
                    current = m_ruleTreeCtrl->GetNextChild( targetParent, cookie );
                }

                if( !previous.IsOk() )
                {
                    // If the target is the first child, insert as first child
                    newItem = m_ruleTreeCtrl->PrependItem(
                            targetParent, m_ruleTreeCtrl->GetItemText( m_draggedItem ) );
                }
                else
                {
                    // Check if there is a next sibling (to insert after the target)
                    wxTreeItemId nextSibling = m_ruleTreeCtrl->GetNextChild( targetParent, cookie );

                    if( nextSibling.IsOk() )
                    {
                        // Insert after the target item
                        newItem = m_ruleTreeCtrl->InsertItem(
                                targetParent, previous,
                                m_ruleTreeCtrl->GetItemText( m_draggedItem ) );
                    }
                    else
                    {
                        // If no next sibling, insert as the last child
                        newItem = m_ruleTreeCtrl->AppendItem(
                                targetParent, m_ruleTreeCtrl->GetItemText( m_draggedItem ) );
                    }
                }
            }

            // Recursively move children to the new item
            moveRuleTreeItemChildrensTooOnDrag( m_draggedItem, newItem );

            // Handle item data transfer
            wxTreeItemData* itemData = m_ruleTreeCtrl->GetItemData( m_draggedItem );

            if( itemData )
            {
                RULE_TREE_ITEM_DATA* ruleTreeItemData =
                        dynamic_cast<RULE_TREE_ITEM_DATA*>( itemData );
                ruleTreeItemData->SetTreeItemId( newItem );

                m_ruleTreeCtrl->SetItemData( newItem, ruleTreeItemData );
                m_ruleTreeCtrl->SetItemData( m_draggedItem, nullptr ); // Detach data

                saveRuleTreeState( newItem, ruleTreeItemData->GetNodeId() );

                m_ruleTreeCtrl->SelectItem( newItem );

                m_ruleTreeCtrl->Expand( newItem );
            }

            // Delete the original dragged item
            m_ruleTreeCtrl->DeleteChildren( m_draggedItem );
            m_ruleTreeCtrl->Delete( m_draggedItem );
        }
        else
        {
            DisplayErrorMessage( this,
                                 "Invalid move: Items can only be moved within the same parent." );
        }

        m_draggedItem = wxTreeItemId(); // Reset dragged item
    }

    aEvent.Skip();
}


void RULE_EDITOR_DIALOG_BASE::onFilterSearch( wxCommandEvent& aEvent )
{
    const auto searchStr = aEvent.GetString().Lower();

    // Capture the selected item
    wxTreeItemId selectedItem = m_ruleTreeCtrl->GetSelection();
    int          selectedNodeId = wxNOT_FOUND;

    if( selectedItem.IsOk() )
    {
        RULE_TREE_ITEM_DATA* itemData =
                dynamic_cast<RULE_TREE_ITEM_DATA*>( m_ruleTreeCtrl->GetItemData( selectedItem ) );

        if( itemData )
        {
            selectedNodeId = itemData->GetNodeId();
        }
    }

    m_ruleTreeCtrl->DeleteAllItems();

    restoreRuleTree( nullptr, m_defaultTreeItems[0].m_nodeId );

    wxTreeItemId root = m_ruleTreeCtrl->GetRootItem();

    // Apply the filter
    if( root.IsOk() )
    {
        filterRuleTree( root, searchStr );
    }
}


bool RULE_EDITOR_DIALOG_BASE::filterRuleTree( const wxTreeItemId& aItem, const wxString& aFilter )
{
    bool matches = m_ruleTreeCtrl->GetItemText( aItem ).Lower().Contains( aFilter );

    wxTreeItemIdValue cookie;
    wxTreeItemId      child = m_ruleTreeCtrl->GetFirstChild( aItem, cookie );
    bool              hasVisibleChildren = false;

    // List to store items for deletion
    std::vector<wxTreeItemId> itemsToDelete;

    while( child.IsOk() )
    {
        if( filterRuleTree( child, aFilter ) )
        {
            hasVisibleChildren = true;
        }
        else
        {
            // Collect child for deletion
            itemsToDelete.push_back( child );
        }

        child = m_ruleTreeCtrl->GetNextChild( aItem, cookie );
    }

    // Delete non-matching children after iteration
    for( const auto& id : itemsToDelete )
    {
        m_ruleTreeCtrl->Delete( id );
    }

    return matches || hasVisibleChildren;
}


void RULE_EDITOR_DIALOG_BASE::saveRuleTreeState( const wxTreeItemId& aItem, const int& aNodeId )
{
    wxString         itemText = m_ruleTreeCtrl->GetItemText( aItem );
    std::vector<int> children;

    wxTreeItemIdValue cookie;
    wxTreeItemId      child = m_ruleTreeCtrl->GetFirstChild( aItem, cookie );
    int               index = 0;

    while( child.IsOk() )
    {
        RULE_TREE_ITEM_DATA* childItemData =
                dynamic_cast<RULE_TREE_ITEM_DATA*>( m_ruleTreeCtrl->GetItemData( child ) );
        int childId = childItemData->GetNodeId();

        //std::string childId = itemId + "/" + std::to_string( index++ );
        children.push_back( childId );

        // Recursively save the state of the child
        saveRuleTreeState( child, childId );

        child = m_ruleTreeCtrl->GetNextChild( aItem, cookie );
    }

    // Store the current item in the map
    m_treeHistoryData[aNodeId] = { itemText, children, aItem };
}


void RULE_EDITOR_DIALOG_BASE::restoreRuleTree( const wxTreeItemId& aParent, const int& aNodeId )
{
    auto it = m_treeHistoryData.find( aNodeId );
    if( it == m_treeHistoryData.end() )
        return;

    const auto& [itemText, children, treeItemId] = it->second;

    wxTreeItemId newItem;

    if( aParent )
    {
        // Add the restored item to the parent
        newItem = m_ruleTreeCtrl->AppendItem( aParent, itemText );
        wxTreeItemId& treeItemId = std::get<2>( it->second );
        treeItemId = newItem;
    }
    else
    {
        newItem = m_ruleTreeCtrl->AddRoot( itemText );
        wxTreeItemId& treeItemId = std::get<2>( it->second );
        treeItemId = newItem;
    }

    RULE_TREE_ITEM_DATA* itemData = new RULE_TREE_ITEM_DATA( aNodeId, aParent, newItem );
    m_ruleTreeCtrl->SetItemData( newItem, itemData );

    // Recursively restore each child
    for( const auto& childId : children )
    {
        restoreRuleTree( newItem, childId );
    }

    m_ruleTreeCtrl->Expand( newItem );
}


wxTreeItemId RULE_EDITOR_DIALOG_BASE::appendRuleTreeItem( const RULE_TREE_NODE& aRuleTreeNode,
                                                          wxTreeItemId          aParentTreeItemId )
{
    wxTreeItemId currentTreeItemId =
            m_ruleTreeCtrl->AppendItem( aParentTreeItemId, aRuleTreeNode.m_nodeName );

    // Attach TreeItemData to the tree item
    RULE_TREE_ITEM_DATA* itemData =
            new RULE_TREE_ITEM_DATA( aRuleTreeNode.m_nodeId, aParentTreeItemId, currentTreeItemId );
    m_ruleTreeCtrl->SetItemData( currentTreeItemId, itemData );

    // Optionally expand the root node
    m_ruleTreeCtrl->Expand( currentTreeItemId );

    return currentTreeItemId;
}


void RULE_EDITOR_DIALOG_BASE::getRuleTreeChildNodes( const std::vector<RULE_TREE_NODE>& aNodes,
                                                     int                                aParentId,
                                                     std::vector<RULE_TREE_NODE>&       aResult )
{
    // Filter nodes that match the parentId
    std::vector<RULE_TREE_NODE> filteredNodes;
    std::copy_if( aNodes.begin(), aNodes.end(), std::back_inserter( filteredNodes ),
                  [&aParentId]( const RULE_TREE_NODE& node )
                  {
                      return node.m_nodeData->GetParentId() == aParentId;
                  } );

    if( filteredNodes.size() > 0 )
    {
        // Add the filtered nodes to the result
        aResult.insert( aResult.end(), filteredNodes.begin(), filteredNodes.end() );
    }
}

void RULE_EDITOR_DIALOG_BASE::moveRuleTreeItemChildrensTooOnDrag( wxTreeItemId aSrcTreeItemId,
                                                                  wxTreeItemId aDestTreeItemId )
{
    wxTreeItemIdValue cookie;
    wxTreeItemId      child = m_ruleTreeCtrl->GetFirstChild( aSrcTreeItemId, cookie );

    while( child.IsOk() )
    {
        // Create a new child under the destination item
        wxTreeItemId newChild =
                m_ruleTreeCtrl->AppendItem( aDestTreeItemId, m_ruleTreeCtrl->GetItemText( child ) );

        RULE_TREE_ITEM_DATA* ruleTreeItemData =
                dynamic_cast<RULE_TREE_ITEM_DATA*>( m_ruleTreeCtrl->GetItemData( child ) );
        ruleTreeItemData->SetParentTreeItemId( aDestTreeItemId );
        ruleTreeItemData->SetTreeItemId( newChild );

        // Copy any associated data
        m_ruleTreeCtrl->SetItemData( newChild, ruleTreeItemData );
        m_ruleTreeCtrl->SetItemData( child, nullptr );

        // Recurse for the current child
        moveRuleTreeItemChildrensTooOnDrag( child, newChild );

        // Move to the next sibling
        child = m_ruleTreeCtrl->GetNextChild( aSrcTreeItemId, cookie );
    }
}


void RULE_EDITOR_DIALOG_BASE::updateRuleTreeItemMoveOptionState()
{
    wxTreeItemId selectedItem = m_ruleTreeCtrl->GetSelection();

    // Reset flags if no selection
    m_enableMoveUp = false;
    m_enableMoveDown = false;

    if( !selectedItem.IsOk() )
    {
        return;
    }

    // If the root item is selected, no need to check move options
    if( selectedItem == m_ruleTreeCtrl->GetRootItem() )
    {
        return;
    }

    wxTreeItemId      parent = m_ruleTreeCtrl->GetItemParent( selectedItem );
    wxTreeItemIdValue cookie;
    wxTreeItemId      firstChild = m_ruleTreeCtrl->GetFirstChild( parent, cookie );
    wxTreeItemId      lastChild = firstChild;

    // Traverse to find the last child
    while( lastChild.IsOk() )
    {
        wxTreeItemId next = m_ruleTreeCtrl->GetNextChild( parent, cookie );
        if( !next.IsOk() )
            break;
        lastChild = next;
    }

    // Update flags based on the position
    m_enableMoveUp = ( selectedItem != firstChild );
    m_enableMoveDown = ( selectedItem != lastChild );
}


void RULE_EDITOR_DIALOG_BASE::updateRuleTreeActionButtonsState(
        RULE_TREE_ITEM_DATA* aRuleTreeItemData )
{
    wxTreeItemId selectedItem = m_ruleTreeCtrl->GetSelection();

    if( !selectedItem.IsOk() || !m_ruleTreeCtrl->IsEnabled() )
    {
        m_addRuleButton->Enable( false );
        m_copyRuleButton->Enable( false );
        m_moveTreeItemUpButton->Enable( false );
        m_moveTreeItemDownButton->Enable( false );
        m_deleteRuleButton->Enable( false );

        return;
    }

    m_enableAddRule = false;
    m_enableDuplicateRule = false;
    m_enableDeleteRule = false;

    if( VerifyRuleTreeContextMenuOptionToEnable( aRuleTreeItemData,
                                                 RULE_EDITOR_TREE_CONTEXT_OPT::ADD_RULE ) )
        m_enableAddRule = true;

    if( VerifyRuleTreeContextMenuOptionToEnable( aRuleTreeItemData,
                                                 RULE_EDITOR_TREE_CONTEXT_OPT::DUPLICATE_RULE ) )
        m_enableDuplicateRule = true;

    if( VerifyRuleTreeContextMenuOptionToEnable( aRuleTreeItemData,
                                                 RULE_EDITOR_TREE_CONTEXT_OPT::DELETE_RULE ) )
        m_enableDeleteRule = true;

    updateRuleTreeItemMoveOptionState();

    m_addRuleButton->Enable( m_enableAddRule );
    m_copyRuleButton->Enable( m_enableDuplicateRule );
    m_moveTreeItemUpButton->Enable( m_enableMoveUp );
    m_moveTreeItemDownButton->Enable( m_enableMoveDown );
    m_deleteRuleButton->Enable( m_enableDeleteRule );
}