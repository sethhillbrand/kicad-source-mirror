/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2025 Ethan Chien <liangtie.qian@gmail.com>
 * Copyright (C) 2025 KiCad Developers, see AUTHORS.txt for contributors.
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

#ifndef SCH_AGENT_ACTION_EXECUTOR_H
#define SCH_AGENT_ACTION_EXECUTOR_H

#include "sch_editor_control.h"
#include "tool/tool_manager.h"
#include <context/symbol_context.h>
#include <exception>
#include <passive_action/agent/agent_action.h>
#include <passive_action/agent/agent_action_type.h>
#include <passive_action/agent/context/component_agent_action_context.h>
#include <passive_action/agent/context/highlight_symbol_context.h>
#include <sch_edit_frame.h>
#include <magic_enum.hpp>
#include <string>
#include <wx/log.h>
#include <active_action/cmd/sch/sch_copilot_cmd_type.h>
#include <utils/copilot_utils.h>
#include <webview/webview_container.h>
#include <chat/general_chat_cmd.h>
#include <context/sch/details/sch_netlist.h>
#include <context/sch/details/netlist_json_schema.h>
#include <fmt/format.h>

void SCH_EDIT_FRAME::ExecuteAgentAction( AGENT_ACTION const& aAction )
{
    const auto hight_symbol = [&]( std::string const& designator )
    {
        wxString            part_ref{ designator };
        SCH_EDITOR_CONTROL* editor = m_toolManager->GetTool<SCH_EDITOR_CONTROL>();
        editor->FindSymbolAndItem( nullptr, &part_ref, true, HIGHLIGHT_SYMBOL, {} );
    };

    try
    {
        auto t = magic_enum::enum_cast<AGENT_ACTION_TYPE>( aAction.action );

        if( !t.has_value() )
        {
            wxLogWarning( "Unknown action received: %s", aAction.action.c_str() );
            return;
        }

        switch( *t )
        {
        case AGENT_ACTION_TYPE::highlight_symbol:
        {
            hight_symbol( aAction.context.get<HIGHLIGHT_SYMBOL_CONTEXT>().designator );
        }
        break;
        case AGENT_ACTION_TYPE::ask_selection:
        {
            const auto netlist = nlohmann::json( GetSelection() ).dump();
            m_copilotPanel->fire_host_active_cmd(
                    nlohmann::json(
                            GENERAL_CHAT_CMD{
                                    { { aAction.context.get<std::string>() } },
                                    { { fmt::format( "The user is working on a schematic the "
                                                     "current selection of which is represented by "
                                                     "the following netlist:\n{}. The "
                                                     "netlist is defined "
                                                     "following the json schema:\n {}\n"
                                                     "NOTE: All the connections for any component "
                                                     "presented inside the components have been "
                                                     "included in the nets.\n"
                                                     "NOTE: You always provide clear answers "
                                                     "rather than asking for more information.\n "
                                                     "Please note that our users are hardware engineers who are neither aware nor interested in the existence of this JSON data. Never expose the existence of a JSON file",
                                                     netlist, copilot::NETLIST_JSON_SCHEMA ) } } } )
                            .dump()
                            .c_str() );
        }
        break;
        default: wxLogWarning( "Unknown action received: %s", aAction.action.c_str() ); break;
        }
    }
    catch( std::exception const& e )
    {
        wxLogWarning( "Exception caught: %s", e.what() );
    }
}


#endif
