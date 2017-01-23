/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2017 KiCad Developers, see CHANGELOG.TXT for contributors.
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

/**
 * @file  pcbnew_action_plugins.cpp
 * @brief Class PCBNEW_PYTHON_ACTION_PLUGINS
 */

#include "pcbnew_action_plugins.h"
#include <python_scripting.h>
#include <stdio.h>
#include <macros.h>
#include <pcbnew_id.h>
#include <menus_helpers.h>
#include <class_drawpanel.h>    // m_canvas
#include <class_board.h>
#include <class_module.h>
#include <class_track.h>
#include <board_commit.h>
#include <kicad_device_context.h>

PYTHON_ACTION_PLUGIN::PYTHON_ACTION_PLUGIN( PyObject* aAction )
{
    PyLOCK lock;

    this->m_PyAction = aAction;
    Py_XINCREF( aAction );
}


PYTHON_ACTION_PLUGIN::~PYTHON_ACTION_PLUGIN()
{
    PyLOCK lock;

    Py_XDECREF( this->m_PyAction );
}


PyObject* PYTHON_ACTION_PLUGIN::CallMethod( const char* aMethod, PyObject* aArglist )
{
    PyLOCK lock;

    PyErr_Clear();
    // pFunc is a new reference to the desired method
    PyObject* pFunc = PyObject_GetAttrString( this->m_PyAction, aMethod );

    if( pFunc && PyCallable_Check( pFunc ) )
    {
        PyObject* result = PyObject_CallObject( pFunc, aArglist );

        if( PyErr_Occurred() )
        {
            wxMessageBox( PyErrStringWithTraceback(),
                    wxT( "Exception on python action plugin code" ),
                    wxICON_ERROR | wxOK );
        }

        if( result )
        {
            Py_XDECREF( pFunc );
            return result;
        }
    }
    else
    {
        printf( "method not found, or not callable: %s\n", aMethod );
    }

    if( pFunc )
    {
        Py_XDECREF( pFunc );
    }

    return NULL;
}


wxString PYTHON_ACTION_PLUGIN::CallRetStrMethod( const char* aMethod, PyObject* aArglist )
{
    wxString    ret;
    PyLOCK      lock;

    PyObject* result = CallMethod( aMethod, aArglist );

    if( result )
    {
        const char* str_res = PyString_AsString( result );
        ret = FROM_UTF8( str_res );
        Py_DECREF( result );
    }

    return ret;
}


wxString PYTHON_ACTION_PLUGIN::GetCategoryName()
{
    PyLOCK lock;

    return CallRetStrMethod( "GetCategoryName" );
}


wxString PYTHON_ACTION_PLUGIN::GetName()
{
    PyLOCK lock;

    return CallRetStrMethod( "GetName" );
}


wxString PYTHON_ACTION_PLUGIN::GetDescription()
{
    PyLOCK lock;

    return CallRetStrMethod( "GetDescription" );
}


void PYTHON_ACTION_PLUGIN::Run()
{
    PyLOCK lock;

    CallMethod( "Run" );
}


void* PYTHON_ACTION_PLUGIN::GetObject()
{
    return (void*) m_PyAction;
}


void PYTHON_ACTION_PLUGINS::register_action( PyObject* aPyAction )
{
    PYTHON_ACTION_PLUGIN* fw = new PYTHON_ACTION_PLUGIN( aPyAction );

    fw->register_action();
}


void PYTHON_ACTION_PLUGINS::deregister_action( PyObject* aPyAction )
{
    // deregister also destroyes the previously created "PYTHON_ACTION_PLUGIN object"
    ACTION_PLUGINS::deregister_object( (void*) aPyAction );
}


#if defined(KICAD_SCRIPTING) && defined(KICAD_SCRIPTING_ACTION_MENU)

void PCB_EDIT_FRAME::OnActionPlugin( wxCommandEvent& aEvent )
{
    int id = aEvent.GetId();

    ACTION_PLUGIN* actionPlugin = ACTION_PLUGINS::GetActionByMenu( id );

    if( actionPlugin )
    {
        // TODO: Adding recovery point for jobs
        // BOARD_COMMIT commit( this );
        // commit.Push( _( "External plugin" ) );

        actionPlugin->Run();

        OnModify();

        if( IsGalCanvasActive() )
        {
            UseGalCanvas( GetGalCanvas() );
        }
        else
        {
            GetScreen()->SetModify();
            Refresh();
        }
    }
}


void PCB_EDIT_FRAME::OnActionPluginRefresh( wxCommandEvent& aEvent )
{
    char cmd[1024];

    snprintf( cmd, sizeof(cmd),
            "pcbnew.LoadPlugins(\"%s\")", TO_UTF8( PyScriptingPath() ) );

    PyLOCK lock;
    // ReRun the Python method pcbnew.LoadPlugins (already called when starting Pcbnew)
    PyRun_SimpleString( cmd );

    RebuildActionPluginMenus();
}


void PCB_EDIT_FRAME::RebuildActionPluginMenus()
{
    wxMenu* actionMenu = GetMenuBar()->FindItem( ID_TOOLBARH_PCB_ACTION_PLUGIN )->GetSubMenu();

    if( !actionMenu )   // Should not occur.
        return;

    // First, remove existing submenus, if they are too many
    wxMenuItemList list = actionMenu->GetMenuItems();
    // The first menuitems are the refresh menu and separator. do not count them
    int act_menu_count = -2;

    std::vector<wxMenuItem*> available_menus;

    for( auto iter = list.begin(); iter != list.end(); ++iter, act_menu_count++ )
    {
        if( act_menu_count < 0 )
            continue;

        wxMenuItem* item = *iter;

        if( act_menu_count < ACTION_PLUGINS::GetActionsCount() )
        {
            available_menus.push_back( item );
            continue;
        }

        // Remove menus which are not usable for our current plugin list
        Disconnect( item->GetId(), wxEVT_COMMAND_MENU_SELECTED,
                (wxObjectEventFunction) (wxEventFunction) (wxCommandEventFunction) &
                PCB_EDIT_FRAME::OnActionPlugin );
        actionMenu->Delete( item );
    }

    for( int ii = 0; ii < ACTION_PLUGINS::GetActionsCount(); ii++ )
    {
        wxMenuItem* item;

        if( ii < (int)available_menus.size() )
        {
            item = available_menus[ii];
            item->SetItemLabel( ACTION_PLUGINS::GetAction( ii )->GetName() );
            item->SetHelp( ACTION_PLUGINS::GetAction( ii )->GetDescription() );
        }
        else
        {
            item = AddMenuItem( actionMenu, wxID_ANY,
                    ACTION_PLUGINS::GetAction( ii )->GetName(),
                    ACTION_PLUGINS::GetAction( ii )->GetDescription(),
                    KiBitmap( hammer_xpm ) );

                Connect( item->GetId(), wxEVT_COMMAND_MENU_SELECTED,
                         (wxObjectEventFunction) (wxEventFunction) (wxCommandEventFunction) &
                         PCB_EDIT_FRAME::OnActionPlugin );
        }

        ACTION_PLUGINS::SetActionMenu( ii, item->GetId() );
    }
}


#endif
