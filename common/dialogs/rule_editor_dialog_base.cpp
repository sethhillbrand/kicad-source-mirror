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
        m_selectedTreeItemData( nullptr )
{
    // Create the main vertical sizer
    wxBoxSizer* mainSizer = new wxBoxSizer( wxVERTICAL );
    SetSizer( mainSizer );

    m_infoBar = new WX_INFOBAR( this );
    mainSizer->Add( m_infoBar, 0, wxEXPAND, 0 );

    // Create a horizontal sizer for the tree and dynamic panel
    m_contentSizer = new wxBoxSizer( wxHORIZONTAL );

    WX_PANEL* treeCtrlPanel = new WX_PANEL( this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                            wxBORDER_NONE | wxTAB_TRAVERSAL );
    treeCtrlPanel->SetBorders( true, true, true, true );
    wxBoxSizer* treeCtrlSizer = new wxBoxSizer( wxVERTICAL );
    treeCtrlPanel->SetSizer( treeCtrlSizer );
    treeCtrlPanel->SetMinSize( wxSize( 350, -1 ) );

    // Add a search text box above the tree control
    wxTextCtrl* searchBox = new wxTextCtrl( treeCtrlPanel, wxID_ANY, wxEmptyString,
                                            wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER );
    treeCtrlSizer->Add( searchBox, 0, wxEXPAND | wxBOTTOM, 5 ); // Add search box to the sizer

    m_treeCtrl = new wxTreeCtrl( treeCtrlPanel, wxID_ANY );
    m_treeCtrl->SetFont( KIUI::GetControlFont( this ) );
    //m_treeCtrl->curr( true );

    long treeCtrlFlags = m_treeCtrl->GetWindowStyleFlag();
    treeCtrlFlags = ( treeCtrlFlags & ~wxBORDER_MASK ) | wxBORDER_NONE;
    m_treeCtrl->SetWindowStyleFlag( treeCtrlFlags );

    treeCtrlSizer->Add( m_treeCtrl, 1, wxEXPAND | wxBOTTOM, 2 );

    // Add the tree panel to the contentSizer
    m_contentSizer->Add( treeCtrlPanel, 0, wxEXPAND | wxALL, 5 );

    // Placeholder for the dynamic panel
    m_contentPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
    m_contentPanel->SetBackgroundColour( *wxLIGHT_GREY );     
    // Adjust minimum size for the dynamic panel
    m_contentSizer->Add( m_contentPanel, 1, wxEXPAND | wxLEFT, 5 ); // Dynamic panel on the right

    // Add contentSizer to mainSizer
    mainSizer->Add( m_contentSizer, 1, wxEXPAND, 0 );

    m_buttonsSizer = new wxBoxSizer( wxHORIZONTAL );

    m_buttonsSizer->AddStretchSpacer();

    wxStdDialogButtonSizer* sdbSizer = new wxStdDialogButtonSizer();
    wxButton*               sdbSizerOK = new wxButton( this, wxID_OK );
    sdbSizer->AddButton( sdbSizerOK );
    wxButton* sdbSizerCancel = new wxButton( this, wxID_CANCEL );
    sdbSizer->AddButton( sdbSizerCancel );
    sdbSizer->Realize();

    m_buttonsSizer->Add( sdbSizer, 1, 0, 5 );
    mainSizer->Add( m_buttonsSizer, 0, wxALL | wxEXPAND, 5 );

    SetupStandardButtons();

    // We normally save the dialog size and position based on its class-name.  This class
    // substitutes the title so that each distinctly-titled dialog can have its own saved
    // size and position.
    m_hash_key = aTitle;

     // Bind the context menu event
    m_treeCtrl->Bind( wxEVT_CONTEXT_MENU, &RULE_EDITOR_DIALOG_BASE::onTreeContextMenu, this );
    m_treeCtrl->Bind( wxEVT_TREE_SEL_CHANGED, &RULE_EDITOR_DIALOG_BASE::onSelectionChanged, this );
}


void RULE_EDITOR_DIALOG_BASE::finishInitialization()
{
    finishDialogSettings();
}


RULE_EDITOR_DIALOG_BASE::~RULE_EDITOR_DIALOG_BASE()
{
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

    // Add the root node (first element in the vector)
    wxTreeItemId rootId = aTreeCtrl->AddRoot( aRuleTreeNodes[0].node_name );

    // Add all children recursively
    for( const auto& child : aRuleTreeNodes[0].children )
    {
        PopulateTreeCtrl( aTreeCtrl, child, rootId );
    }

    // Optionally expand the root node
    aTreeCtrl->Expand( rootId );
}


void RULE_EDITOR_DIALOG_BASE::PopulateTreeCtrl( wxTreeCtrl* aTreeCtrl, const RuleTreeNode& aRuleTreeNode,
                                                wxTreeItemId aParentTreeItemId )
{
    // Add the current node as a child to the parent
    wxTreeItemId currentId = aTreeCtrl->AppendItem( aParentTreeItemId, aRuleTreeNode.node_name );

    // Allocate TreeData dynamically
    RuleTreeNode* nodeData = new RuleTreeNode( aRuleTreeNode );

    // Attach TreeItemData to the tree item
    RuleTreeItemData* itemData = new RuleTreeItemData( nodeData, aParentTreeItemId, currentId );
    aTreeCtrl->SetItemData( currentId, itemData );

    // Recursively add children 
    for( const auto& child : aRuleTreeNode.children )
    {
        PopulateTreeCtrl( aTreeCtrl, child, currentId );
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
    m_contentSizer->Add( m_contentPanel, 1, wxEXPAND | wxALL, 5 );

    // Adjust the sizer and the layout to ensure proper fit
    m_contentSizer->Fit( this );          // Adjust the sizer to fit the containing dialog
    m_contentSizer->SetSizeHints( this ); // Ensure minimum size hints are updated

    // Update the layout
    Layout();
    Refresh(); // Refresh to ensure visual updates
}


void RULE_EDITOR_DIALOG_BASE::AppendTreeItem( wxTreeCtrl* aTreeCtrl, const RuleTreeNode& aRuleTreeNode, wxTreeItemId aParentTreeItemId)
{
    wxTreeItemId currentTreeItemId = aTreeCtrl->AppendItem( aParentTreeItemId, aRuleTreeNode.node_name );

    // Allocate TreeData dynamically
    RuleTreeNode* nodeData = new RuleTreeNode( aRuleTreeNode );

    // Attach TreeItemData to the tree item
    RuleTreeItemData* itemData = new RuleTreeItemData( nodeData, aParentTreeItemId, currentTreeItemId );
    aTreeCtrl->SetItemData( currentTreeItemId, itemData );

     // Optionally expand the root node
    aTreeCtrl->Expand( currentTreeItemId );
    aTreeCtrl->SelectItem( currentTreeItemId );
}


void RULE_EDITOR_DIALOG_BASE::UpdateTreeItem( wxTreeCtrl* aTreeCtrl, RuleTreeItemData aRuleTreeItemData )
{
    aTreeCtrl->SetItemText( aRuleTreeItemData.GetTreeItemId(), aRuleTreeItemData.GetData()->node_name );
}


void RULE_EDITOR_DIALOG_BASE::onTreeContextMenu( wxContextMenuEvent& aEvent )
{
    // Get the mouse position
    wxPoint pos = m_treeCtrl->ScreenToClient( wxGetMousePosition() );

    int          flags;
    wxTreeItemId item = m_treeCtrl->HitTest( pos, flags );

    if( !item.IsOk() )
        return; // No valid item 

     // Get the associated TreeData
    RuleTreeItemData* ruleTreeItemData= dynamic_cast<RuleTreeItemData*>( m_treeCtrl->GetItemData( item ) );

    if( !ruleTreeItemData )
        return;

    if( !CanShowContextMenu( ruleTreeItemData->GetData() ) )
        return;

    m_selectedTreeItemData = ruleTreeItemData;

    wxMenu menu;
    menu.Append( 1, "New Rule" );

    if( CheckAndAppendRuleOperations( ruleTreeItemData->GetData() ) )
    {
        menu.Append( 2, "Duplicate Rule" );
        menu.Append( 3, "Delete Rule" );
        menu.AppendSeparator();
        menu.Append( 4, "Move Up" );
        menu.Append( 5, "Move Down" );
    }

    // Show the menu
    PopupMenu( &menu, pos );
}


void RULE_EDITOR_DIALOG_BASE::onNewRule( wxCommandEvent& aEvent )
{
    AddNewRule( m_selectedTreeItemData );
}


void RULE_EDITOR_DIALOG_BASE::onDuplicateRule( wxCommandEvent& aEvent )
{
    wxLogMessage( "Duplicate Rule action triggered." );
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