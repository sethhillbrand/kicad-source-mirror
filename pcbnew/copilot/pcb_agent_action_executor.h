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

#include "active_action/cmd/copilot_cmd_base.h"
#include "context/sch/sch_copilot_global_context.h"
#include "tool/tool_manager.h"
#include <context/symbol_context.h>
#include <exception>
#include <passive_action/agent/agent_action.h>
#include <passive_action/agent/agent_action_type.h>
#include <passive_action/agent/context/component_agent_action_context.h>
#include <passive_action/agent/context/highlight_symbol_context.h>
#include <pcb_edit_frame.h>
#include <magic_enum.hpp>
#include <string>
#include <wx/log.h>
#include <assistant_interface.h>
#include <active_action/cmd/sch/sch_copilot_cmd_type.h>
#include <utils/copilot_utils.h>

void PCB_EDIT_FRAME::ExecuteAgentAction( AGENT_ACTION const& aAction )
{
    static const auto kComponentActMapping = std::unordered_map<AGENT_ACTION_TYPE, std::string>{
        { AGENT_ACTION_TYPE::part_detail, SCH_COPILOT_CMD_TYPE::CURRENT_COMPONENT },
        { AGENT_ACTION_TYPE::part_replace, SCH_COPILOT_CMD_TYPE::SIMILAR_COMPONENTS },
        { AGENT_ACTION_TYPE::link_check, SCH_COPILOT_CMD_TYPE::CHECK_SYMBOL_CONNECTIONS },
        { AGENT_ACTION_TYPE::foot_detail, SCH_COPILOT_CMD_TYPE::COMPONENT_PINS_DETAILS },
        { AGENT_ACTION_TYPE::foot_unconnected, SCH_COPILOT_CMD_TYPE::SYMBOL_UNCONNECTED_PINS },
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
        case AGENT_ACTION_TYPE::foot_detail:
        case AGENT_ACTION_TYPE::part_detail:
        case AGENT_ACTION_TYPE::part_replace:
        case AGENT_ACTION_TYPE::link_check:
        case AGENT_ACTION_TYPE::foot_unconnected:
        break;
        case AGENT_ACTION_TYPE::highlight_symbol:
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
