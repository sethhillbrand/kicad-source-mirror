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

#ifndef SCH_COPILOT_CMD_H
#define SCH_COPILOT_CMD_H

#include <iostream>
#include <nlohmann/json.hpp>
#include <sch_edit_frame.h>
#include <build_version.h>
#include <context/context_fields.h>
#include <active_action/cmd/copilot_cmd_base.h>
#include <active_action/cmd/sch/sch_copilot_cmd_type.h>
#include <context/sch/sch_copilot_global_context.h>
#include <webview/webview_container.h>
#include "context/symbol_context.h"



void SCH_EDIT_FRAME::FireCopilotCommand( std::string const& aCmdType )
{
    if( !m_copilotPanel )
        return;

    UpdateCopilotContextCache();

    nlohmann::json cmd;

    if( aCmdType.starts_with( "chat.design" ) )
    {
        cmd = create_cmd<CONCRETE_TYPE_COPILOT_CMD<SCH_COPILOT_GLOBAL_CONTEXT>>(
                *m_copilotContextCache );
    }
    else if( aCmdType.starts_with( "chat.components" ) )
    {
        cmd = create_cmd<COPILOT_CMD_WITH_CONTEXT<SCH_COPILOT_GLOBAL_CONTEXT, SYMBOL_CMD_CONTEXT>,
                         SYMBOL_CMD_CONTEXT>( *m_copilotContextCache, GetSelectedSymbolContext() );
    }
    else
    {
        std::cerr << "Unknown command type: " << aCmdType << std::endl;
    }

    if( !cmd.empty() )
    {
        cmd[kType] = aCmdType;
        ShowCopilot();
        m_copilotPanel->fire_host_active_cmd( cmd.dump().c_str() );
    }
}


void SCH_EDIT_FRAME::DesignIntention()
{
    FireCopilotCommand( SCH_COPILOT_CMD_TYPE::DESIGN_INTENTION );
}
void SCH_EDIT_FRAME::CoreComponents()
{
    FireCopilotCommand( SCH_COPILOT_CMD_TYPE::CORE_COMPONENTS );
}
void SCH_EDIT_FRAME::CurrentComponent()
{
    FireCopilotCommand( SCH_COPILOT_CMD_TYPE::CURRENT_COMPONENT );
}
void SCH_EDIT_FRAME::SimilarComponents()
{
    FireCopilotCommand( SCH_COPILOT_CMD_TYPE::SIMILAR_COMPONENTS );
}
void SCH_EDIT_FRAME::CheckSymbolConnections()
{
    FireCopilotCommand( SCH_COPILOT_CMD_TYPE::CHECK_SYMBOL_CONNECTIONS );
}
void SCH_EDIT_FRAME::ComponentPinsDetails()
{
    FireCopilotCommand( SCH_COPILOT_CMD_TYPE::COMPONENT_PINS_DETAILS );
}
void SCH_EDIT_FRAME::SymbolUnconnectedPins()
{
    FireCopilotCommand( SCH_COPILOT_CMD_TYPE::SYMBOL_UNCONNECTED_PINS );
}


#endif
