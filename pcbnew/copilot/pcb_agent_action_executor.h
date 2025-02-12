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

#ifndef PCB_AGENT_ACTION_EXECUTOR_H
#define PCB_AGENT_ACTION_EXECUTOR_H


#include "chat/general_chat_cmd.h"
#include <context/pcb/details/board_json_schema.h>
#include "webview/webview_container.h"
#include <context/symbol_context.h>
#include <exception>
#include <passive_action/agent/agent_action.h>
#include <passive_action/agent/agent_action_type.h>
#include <passive_action/agent/context/launch_pcb_plugin_context.h>
#include <pcb_edit_frame.h>
#include <magic_enum.hpp>
#include <string>
#include <wx/log.h>
#include <fmt/format.h>

void PCB_EDIT_FRAME::ExecuteAgentAction( AGENT_ACTION const& aAction )
{
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
        case AGENT_ACTION_TYPE::basic_run:
        {
            const auto plugin_context = aAction.context.get<LAUNCH_PCB_PLUGIN_CONTEXT>();
            LaunchPlugin( plugin_context.type, plugin_context.params );
            break;
        }
        case AGENT_ACTION_TYPE::ask_selection:
        {
            const auto pcb = GetSelectionJsonString();
            m_copilotPanel->fire_host_active_cmd(
                    nlohmann::json(
                            GENERAL_CHAT_CMD{
                                    { { aAction.context.get<std::string>() } },
                                    { { fmt::format(
                                            "The user is working on a pcb board design, and has "
                                            "just selected some items on the board. All the "
                                            "selected items are serialized into the "
                                            "the following json :\n{}. It's "
                                            "defined "
                                            "following the json schema:\n {}\n"
                                            "NOTE: You always provide clear answers "
                                            "rather than asking for more information.\n "                                            
                                            "Please note that our users are hardware engineers who are neither aware nor interested in the existence of this JSON data. Never expose the existence of a JSON file.",
                                            pcb, copilot::BOARD_JSON_SCHEMA ) } } } )
                            .dump()
                            .c_str() );

            break;
        }


        default: wxLogWarning( "Unknown action received: %s", aAction.action.c_str() ); break;
        }
    }
    catch( std::exception const& e )
    {
        wxLogWarning( "Exception caught: %s", e.what() );
    }
}

#endif
