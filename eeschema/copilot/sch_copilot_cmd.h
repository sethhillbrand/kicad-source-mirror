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
#include <cmd/copilot_cmd.h>
#include <assistant_interface.h>
#include <build_version.h>
#include "context/context_fields.h"
#include "context/copilot_context.h"
#include "sch_copilot_context_cache.h"


void SCH_EDIT_FRAME::FireCopilotCommand( std::string const&  aCmdType )
{
    if( !m_copilotPanel )
        return;

    UpdateCopilotContextCache();

    nlohmann::json cmd;

    if(aCmdType.starts_with("chat.design"))
    {
        cmd = create_cmd<DESIGN_INTENTION>( *m_copilotContextCache );

    }
    else if (aCmdType.starts_with("chat.symbol"))
    {
        cmd = create_cmd<CURRENT_COMPONENT, SYMBOL_CMD_CONTEXT>( *m_copilotContextCache,
            GetSelectedSymbolContext() );
    }
    else {
        std::cerr << "Unknown command type: " << aCmdType << std::endl;
    }

    if( !cmd.empty() )
    {
        cmd[kType] = aCmdType;
        ShowCopilot();
        ASSISTANT_INTERFACE::get_instance().fire_cmd( m_copilotPanel, cmd.dump() );
    }
}


void SCH_EDIT_FRAME::DesignIntention()
{
    FireCopilotCommand( COPILOT_CMD_TYPE::DESIGN_INTENTION );
}
void SCH_EDIT_FRAME::CoreComponents()
{
    FireCopilotCommand( COPILOT_CMD_TYPE::CORE_COMPONENTS );
}
void SCH_EDIT_FRAME::CurrentComponent()
{
    FireCopilotCommand( COPILOT_CMD_TYPE::CURRENT_COMPONENT );
}
void SCH_EDIT_FRAME::SimilarComponents()
{
    FireCopilotCommand( COPILOT_CMD_TYPE::SIMILAR_COMPONENTS );
}
void SCH_EDIT_FRAME::CheckSymbolConnections()
{
    FireCopilotCommand( COPILOT_CMD_TYPE::CHECK_SYMBOL_CONNECTIONS );
}
void SCH_EDIT_FRAME::ComponentPinsDetails()
{
    FireCopilotCommand( COPILOT_CMD_TYPE::COMPONENT_PINS_DETAILS );
}
void SCH_EDIT_FRAME::SymbolUnconnectedPins()
{
    FireCopilotCommand( COPILOT_CMD_TYPE::SYMBOL_UNCONNECTED_PINS );
}


#endif
