/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright The KiCad Developers, see AUTHORS.txt for contributors.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you may find one here:
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 * or you may search the http://www.gnu.org website for the version 2 license,
 * or you may write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#include <tool/tool_manager.h>
#include <tools/pcb_selection_tool.h>
#include <tools/pcb_actions.h>
#include <collectors.h>
#include <dialogs/dialog_table_properties.h>
#include <board_stackup_manager/pcb_stackup_table.h>
#include <wx/numdlg.h>
#include <wx/translation.h>
#include <tools/pcb_edit_table_tool.h>


PCB_EDIT_TABLE_TOOL::PCB_EDIT_TABLE_TOOL() :
        PCB_TOOL_BASE( "pcbnew.TableEditor" )
{
}


bool PCB_EDIT_TABLE_TOOL::Init()
{
    PCB_TOOL_BASE::Init();

    CONDITIONAL_MENU& menu = m_toolMgr->GetTool<PCB_SELECTION_TOOL>()->GetToolMenu().GetMenu();
    addMenus( menu );
    addStackupMenus( menu );

    return true;
}


PCB_TABLECELL* PCB_EDIT_TABLE_TOOL::copyCell( PCB_TABLECELL* aSource )
{
    PCB_TABLECELL* cell = new PCB_TABLECELL( aSource->GetParent() );

    cell->SetStart( aSource->GetStart() );
    cell->SetEnd( aSource->GetEnd() );

    return cell;
}


const SELECTION& PCB_EDIT_TABLE_TOOL::getTableCellSelection()
{
    PCB_SELECTION_TOOL*  selTool = m_toolMgr->GetTool<PCB_SELECTION_TOOL>();

    return selTool->RequestSelection(
            []( const VECTOR2I& aPt, GENERAL_COLLECTOR& aCollector, PCB_SELECTION_TOOL* sTool )
            {
                // Iterate from the back so we don't have to worry about removals.
                for( int i = aCollector.GetCount() - 1; i >= 0; --i )
                {
                    if( !dynamic_cast<PCB_TABLECELL*>( aCollector[ i ] ) )
                        aCollector.Remove( aCollector[ i ] );
                }
            },
            false /* prompt user regarding locked items */ );
}


void PCB_EDIT_TABLE_TOOL::clearSelection()
{
    m_toolMgr->RunAction( ACTIONS::selectionClear );
};


int PCB_EDIT_TABLE_TOOL::EditTable( const TOOL_EVENT& aEvent )
{
    const SELECTION& selection = getTableCellSelection();
    bool             clearSelection = selection.IsHover();
    PCB_TABLE*       parentTable = nullptr;

    for( EDA_ITEM* item : selection.Items() )
    {
        if( item->Type() != PCB_TABLECELL_T )
            return 0;

        PCB_TABLE* table = static_cast<PCB_TABLE*>( item->GetParent() );

        if( !parentTable )
        {
            parentTable = table;
        }
        else if( parentTable != table )
        {
            parentTable = nullptr;
            break;
        }
    }

    if( parentTable )
    {
        DIALOG_TABLE_PROPERTIES dlg( frame(), parentTable );

        dlg.ShowQuasiModal();   // Scintilla's auto-complete requires quasiModal
    }

    if( clearSelection )
        m_toolMgr->RunAction( ACTIONS::selectionClear );

    return 0;
}

int PCB_EDIT_TABLE_TOOL::HideStackupColumn( const TOOL_EVENT& aEvent )
{
    const SELECTION& selection = getTableCellSelection();

    if( selection.Empty() )
        return 0;

    PCB_TABLECELL* cell = static_cast<PCB_TABLECELL*>( selection.Front() );
    PCB_STACKUP_TABLE* table = dynamic_cast<PCB_STACKUP_TABLE*>( cell->GetParent() );

    if( !table )
        return 0;

    BOARD_COMMIT commit( m_toolMgr );
    commit.Modify( table, getScreen() );

    STACKUP_TABLE_COLUMN_ID colId = table->GetVisibleColumnId( cell->GetColumn() );
    table->SetColumnVisible( colId, false );
    table->Update( frame<PCB_BASE_FRAME>()->GetBoard(), &commit );

    commit.Push( _( "Hide Column" ) );

    return 0;
}

int PCB_EDIT_TABLE_TOOL::ShowAllStackupColumns( const TOOL_EVENT& aEvent )
{
    const SELECTION& selection = getTableCellSelection();

    if( selection.Empty() )
        return 0;

    PCB_TABLECELL* cell = static_cast<PCB_TABLECELL*>( selection.Front() );
    PCB_STACKUP_TABLE* table = dynamic_cast<PCB_STACKUP_TABLE*>( cell->GetParent() );

    if( !table )
        return 0;

    BOARD_COMMIT commit( m_toolMgr );
    commit.Modify( table, getScreen() );

    table->ShowAllColumns();
    table->Update( frame<PCB_BASE_FRAME>()->GetBoard(), &commit );

    commit.Push( _( "Show All Columns" ) );

    return 0;
}

int PCB_EDIT_TABLE_TOOL::SetStackupPrecision( const TOOL_EVENT& aEvent )
{
    const SELECTION& selection = getTableCellSelection();

    if( selection.Empty() )
        return 0;

    PCB_TABLECELL* cell = static_cast<PCB_TABLECELL*>( selection.Front() );
    PCB_STACKUP_TABLE* table = dynamic_cast<PCB_STACKUP_TABLE*>( cell->GetParent() );

    if( !table )
        return 0;

    long prec = wxGetNumberFromUser( wxEmptyString, _( "Number of decimal places:" ),
                                     _( "Set Numeric Precision" ), table->GetNumericPrecision(),
                                     0, 10, frame<PCB_BASE_FRAME>() );

    if( prec < 0 )
        return 0;

    BOARD_COMMIT commit( m_toolMgr );
    commit.Modify( table, getScreen() );

    table->SetNumericPrecision( (int) prec );
    table->Update( frame<PCB_BASE_FRAME>()->GetBoard(), &commit );

    commit.Push( _( "Set Numeric Precision" ) );

    return 0;
}

int PCB_EDIT_TABLE_TOOL::setStackupUnits( STACKUP_UNIT_MODE aMode )
{
    const SELECTION& selection = getTableCellSelection();

    if( selection.Empty() )
        return 0;

    PCB_TABLECELL* cell = static_cast<PCB_TABLECELL*>( selection.Front() );
    PCB_STACKUP_TABLE* table = dynamic_cast<PCB_STACKUP_TABLE*>( cell->GetParent() );

    if( !table )
        return 0;

    BOARD_COMMIT commit( m_toolMgr );
    commit.Modify( table, getScreen() );

    table->SetUnitsMode( aMode );
    table->Update( frame<PCB_BASE_FRAME>()->GetBoard(), &commit );

    commit.Push( _( "Set Units" ) );

    return 0;
}

int PCB_EDIT_TABLE_TOOL::SetStackupUnitsAuto( const TOOL_EVENT& aEvent )
{
    return setStackupUnits( STACKUP_UNIT_MODE::AUTO );
}

int PCB_EDIT_TABLE_TOOL::SetStackupUnitsMM( const TOOL_EVENT& aEvent )
{
    return setStackupUnits( STACKUP_UNIT_MODE::MM );
}

int PCB_EDIT_TABLE_TOOL::SetStackupUnitsInch( const TOOL_EVENT& aEvent )
{
    return setStackupUnits( STACKUP_UNIT_MODE::INCH );
}

int PCB_EDIT_TABLE_TOOL::ToggleStackupTitle( const TOOL_EVENT& aEvent )
{
    const SELECTION& selection = getTableCellSelection();

    if( selection.Empty() )
        return 0;

    PCB_TABLECELL* cell = static_cast<PCB_TABLECELL*>( selection.Front() );
    PCB_STACKUP_TABLE* table = dynamic_cast<PCB_STACKUP_TABLE*>( cell->GetParent() );

    if( !table )
        return 0;

    bool hadTitle = table->HasTitle();

    BOARD_COMMIT commit( m_toolMgr );
    commit.Modify( table, getScreen() );

    if( hadTitle )
        table->SetTitle( wxEmptyString );
    else
        table->SetTitle( _( "Board Stackup" ) );

    table->Update( frame<PCB_BASE_FRAME>()->GetBoard(), &commit );

    commit.Push( hadTitle ? _( "Remove Title" ) : _( "Add Title" ) );

    return 0;
}

void PCB_EDIT_TABLE_TOOL::addStackupMenus( CONDITIONAL_MENU& aMenu )
{
    auto stackupTableSelected = [&]( const SELECTION& sel )
    {
        for( EDA_ITEM* item : sel )
        {
            if( PCB_TABLECELL* cell = dynamic_cast<PCB_TABLECELL*>( item ) )
            {
                if( cell->GetParent() && cell->GetParent()->GetClass() == wxT( "PCB_STACKUP_TABLE" ) )
                    return true;
            }
            else if( PCB_TABLE* table = dynamic_cast<PCB_TABLE*>( item ) )
            {
                if( table->GetClass() == wxT( "PCB_STACKUP_TABLE" ) )
                    return true;
            }
        }

        return false;
    };

    auto stackupCellSelected = [&]( const SELECTION& sel )
    {
        if( sel.Size() != 1 )
            return false;

        if( PCB_TABLECELL* cell = dynamic_cast<PCB_TABLECELL*>( sel.Front() ) )
            return cell->GetParent() && cell->GetParent()->GetClass() == wxT( "PCB_STACKUP_TABLE" );

        return false;
    };

    auto stackupNumericColumn = [&]( const SELECTION& sel )
    {
        if( sel.Size() != 1 )
            return false;

        PCB_TABLECELL* cell = dynamic_cast<PCB_TABLECELL*>( sel.Front() );

        if( !cell || !cell->GetParent() || cell->GetParent()->GetClass() != wxT( "PCB_STACKUP_TABLE" ) )
            return false;

        PCB_STACKUP_TABLE* table = static_cast<PCB_STACKUP_TABLE*>( cell->GetParent() );
        STACKUP_TABLE_COLUMN_ID colId = table->GetVisibleColumnId( cell->GetColumn() );

        return colId == STACKUP_TABLE_COLUMN_ID::THICKNESS
               || colId == STACKUP_TABLE_COLUMN_ID::EPSILON_R
               || colId == STACKUP_TABLE_COLUMN_ID::LOSS_TANGENT;
    };

    aMenu.AddSeparator( 100 );
    aMenu.AddItem( PCB_ACTIONS::stackupHideColumn,
                   stackupCellSelected && SELECTION_CONDITIONS::Idle, 100 );
    aMenu.AddItem( PCB_ACTIONS::stackupShowAllColumns,
                   stackupTableSelected && SELECTION_CONDITIONS::Idle, 100 );

    aMenu.AddSeparator( 100 );
    aMenu.AddItem( PCB_ACTIONS::stackupSetPrecision,
                   stackupNumericColumn && SELECTION_CONDITIONS::Idle, 100 );

    aMenu.AddSeparator( 100 );
    aMenu.AddItem( PCB_ACTIONS::stackupUnitsAuto,
                   stackupTableSelected && SELECTION_CONDITIONS::Idle, 100 );
    aMenu.AddItem( PCB_ACTIONS::stackupUnitsMM,
                   stackupTableSelected && SELECTION_CONDITIONS::Idle, 100 );
    aMenu.AddItem( PCB_ACTIONS::stackupUnitsInch,
                   stackupTableSelected && SELECTION_CONDITIONS::Idle, 100 );

    aMenu.AddSeparator( 100 );
    aMenu.AddItem( PCB_ACTIONS::stackupToggleTitle,
                   stackupTableSelected && SELECTION_CONDITIONS::Idle, 100 );
}


void PCB_EDIT_TABLE_TOOL::setTransitions()
{
    Go( &PCB_EDIT_TABLE_TOOL::AddRowAbove,        ACTIONS::addRowAbove.MakeEvent() );
    Go( &PCB_EDIT_TABLE_TOOL::AddRowBelow,        ACTIONS::addRowBelow.MakeEvent() );

    Go( &PCB_EDIT_TABLE_TOOL::AddColumnBefore,    ACTIONS::addColBefore.MakeEvent() );
    Go( &PCB_EDIT_TABLE_TOOL::AddColumnAfter,     ACTIONS::addColAfter.MakeEvent() );

    Go( &PCB_EDIT_TABLE_TOOL::DeleteRows,         ACTIONS::deleteRows.MakeEvent() );
    Go( &PCB_EDIT_TABLE_TOOL::DeleteColumns,      ACTIONS::deleteColumns.MakeEvent() );

    Go( &PCB_EDIT_TABLE_TOOL::MergeCells,         ACTIONS::mergeCells.MakeEvent() );
    Go( &PCB_EDIT_TABLE_TOOL::UnmergeCells,       ACTIONS::unmergeCells.MakeEvent() );

    Go( &PCB_EDIT_TABLE_TOOL::EditTable,          ACTIONS::editTable.MakeEvent() );

    Go( &PCB_EDIT_TABLE_TOOL::HideStackupColumn,  PCB_ACTIONS::stackupHideColumn.MakeEvent() );
    Go( &PCB_EDIT_TABLE_TOOL::ShowAllStackupColumns,
        PCB_ACTIONS::stackupShowAllColumns.MakeEvent() );
    Go( &PCB_EDIT_TABLE_TOOL::SetStackupPrecision,
        PCB_ACTIONS::stackupSetPrecision.MakeEvent() );
    Go( &PCB_EDIT_TABLE_TOOL::SetStackupUnitsAuto,
        PCB_ACTIONS::stackupUnitsAuto.MakeEvent() );
    Go( &PCB_EDIT_TABLE_TOOL::SetStackupUnitsMM,
        PCB_ACTIONS::stackupUnitsMM.MakeEvent() );
    Go( &PCB_EDIT_TABLE_TOOL::SetStackupUnitsInch,
        PCB_ACTIONS::stackupUnitsInch.MakeEvent() );
    Go( &PCB_EDIT_TABLE_TOOL::ToggleStackupTitle,
        PCB_ACTIONS::stackupToggleTitle.MakeEvent() );
}
