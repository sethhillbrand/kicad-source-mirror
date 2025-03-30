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

#include "active_action/cmd/copilot_cmd_base.h"
#include "context/sch/sch_copilot_global_context.h"
#include "sch_editor_control.h"
#include "tool/tool_manager.h"
#include <context/symbol_context.h>
#include <exception>
#include <passive_action/agent/agent_action.h>
#include <passive_action/agent/agent_action_type.h>
#include <passive_action/agent/agent_action_context.h>
#include <sch_edit_frame.h>
#include <magic_enum.hpp>
#include <string>
#include <wx/log.h>
#include <assistant_interface.h>
#include <active_action/cmd/copilot_cmd_type.h>


void SCH_EDIT_FRAME::ExecuteAgentAction( AGENT_ACTION const& aAction )
{
    static const auto kComponentActMapping = std::unordered_map<AGENT_ACTION_TYPE, std::string>{
        { AGENT_ACTION_TYPE::part_detail, COPILOT_CMD_TYPE::CURRENT_COMPONENT },
        { AGENT_ACTION_TYPE::part_replace, COPILOT_CMD_TYPE::SIMILAR_COMPONENTS },
        { AGENT_ACTION_TYPE::link_check, COPILOT_CMD_TYPE::CHECK_SYMBOL_CONNECTIONS },
        { AGENT_ACTION_TYPE::foot_detail, COPILOT_CMD_TYPE::COMPONENT_PINS_DETAILS },
        { AGENT_ACTION_TYPE::foot_unconnected, COPILOT_CMD_TYPE::SYMBOL_UNCONNECTED_PINS },
    };

    const auto process_component_action =
            [&]( std::string const& designator, AGENT_ACTION_TYPE act )
    {
        if( !m_copilotPanel || kComponentActMapping.find( act ) == kComponentActMapping.end() )
            return;
        const auto build_cxt = [&]() -> SYMBOL_CMD_CONTEXT const&
        {
            SCH_SYMBOL* symbol{};

            SCH_REFERENCE_LIST referenceList;
            Schematic().Hierarchy().GetSymbols( referenceList, false, false );

            for( const auto& ref : referenceList )
            {
                if( ref.GetRef() == designator )
                {
                    symbol = ref.GetSymbol();
                    break;
                }
            }

            if( !symbol )
            {
                // NOTE shall not come here
                return *m_symbolCmdContext;
            }

            const wxString ref = symbol->GetRefProp();
            m_symbolCmdContext->designator = ref;
            m_symbolCmdContext->symbol_properties = {
                symbol->GetValueProp().ToStdString(),
                symbol->GetDescription().ToStdString(),
                symbol->GetFootprintFieldText( true, &Schematic().CurrentSheet(), false )
                        .ToStdString(),
            };


            for( const auto& pin : symbol->GetPins() )
            {
                m_symbolCmdContext->symbol_properties.pins.push_back(
                        { pin->GetNumber().ToStdString(), pin->GetName().ToStdString(),
                          PinShapeGetText( pin->GetShape() ).ToStdString() } );
            }


            return *m_symbolCmdContext;
        };

        wxString            part_ref{ designator };
        SCH_EDITOR_CONTROL* editor = m_toolManager->GetTool<SCH_EDITOR_CONTROL>();
        editor->FindSymbolAndItem( nullptr, &part_ref, true, HIGHLIGHT_SYMBOL, {} );


        UpdateCopilotContextCache();


        auto cc =
                create_cmd<COPILOT_CMD_WITH_CONTEXT<SCH_COPILOT_GLOBAL_CONTEXT, SYMBOL_CMD_CONTEXT>,
                           SYMBOL_CMD_CONTEXT>( *m_copilotContextCache, build_cxt() );
        cc.triggered_by_passive_action = true;

        nlohmann::json cmd = cc;
        cmd[kType] = kComponentActMapping.at( act );
        ShowCopilot();
        ASSISTANT_INTERFACE::get_instance().fire_cmd( m_copilotPanel, cmd.dump() );
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
            process_component_action( aAction.context.get<DESIGNATOR_CONTEXT>().d1, *t );
            break;
        }
    }
    catch( std::exception const& e )
    {
        wxLogWarning( "Exception caught: %s", e.what() );
    }
}


#endif
