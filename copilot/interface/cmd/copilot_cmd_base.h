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

#ifndef COPILOT_CMD_BASE_H
#define COPILOT_CMD_BASE_H

#include "copilot_cmd_type.h"
#include <nlohmann/json.hpp>
#include <kicad_version_info.h>
#include <optional>
#include <string>
#include <context/copilot_context.h>
#include <context/context_fields.h>
#include "cmd_base.h"


struct COPILOT_CMD_BASE : CMD_BASE
{
    std::optional<std::string>           global_context_uuid;
    std::optional<DESIGN_GLOBAL_CONTEXT> design_global_context;
    friend void to_json( nlohmann ::json& nlohmann_json_j, const COPILOT_CMD_BASE& nlohmann_json_t )
    {
        if( nlohmann_json_t.global_context_uuid )
            nlohmann_json_j[kGlobalContextUUID] = *nlohmann_json_t.global_context_uuid;

        if( nlohmann_json_t.design_global_context )
            nlohmann_json_j[kDesignGlobalContext] = *nlohmann_json_t.design_global_context;
    }
    friend void from_json( const nlohmann ::json& nlohmann_json_j, COPILOT_CMD_BASE& nlohmann_json_t )
    {
        if( nlohmann_json_j.contains( kGlobalContextUUID ) )
        {
            nlohmann_json_t.global_context_uuid =
                    nlohmann_json_j.at( kGlobalContextUUID ).get<std::string>();
        }

        if( nlohmann_json_j.contains( kDesignGlobalContext ) )
        {
            nlohmann_json_t.design_global_context =
                    nlohmann_json_j.at( kDesignGlobalContext ).get<DESIGN_GLOBAL_CONTEXT>();
        }
    }
};


inline auto fill_cmd( COPILOT_CMD_BASE& cmd, DESIGN_GLOBAL_CONTEXT const& design_global_context )
{
    cmd.global_context_uuid = design_global_context.uuid;
    cmd.design_global_context = design_global_context;
}

template <class T>
T create_cmd( DESIGN_GLOBAL_CONTEXT const& design_global_context )
{
    T it;
    fill_cmd( it, design_global_context );
    return it;
}

template <class T, class C>
T create_cmd( DESIGN_GLOBAL_CONTEXT const& design_global_context, C const& context )
{
    T it;
    fill_cmd( it, design_global_context );
    it.context = context;
    return it;
}


template <auto CMD_TYPE>
struct CONCRETE_TYPE_COPILOT_CMD : COPILOT_CMD_BASE
{
    std::string type = CMD_TYPE;
    friend void      to_json( nlohmann ::json&         nlohmann_json_j,
                              const CONCRETE_TYPE_COPILOT_CMD& nlohmann_json_t )
    {
        to_json( nlohmann_json_j, static_cast<COPILOT_CMD_BASE const&>( nlohmann_json_t ) );
        nlohmann_json_j[kType] = nlohmann_json_t.type;
    }
    friend void from_json( const nlohmann ::json& nlohmann_json_j,
                           CONCRETE_TYPE_COPILOT_CMD&     nlohmann_json_t )
    {
        from_json( nlohmann_json_j, static_cast<COPILOT_CMD_BASE&>( nlohmann_json_t ) );
        nlohmann_json_j.at( kType ).get_to( nlohmann_json_t.type );
    }
};


template <auto CMD_TYPE, typename CONTEXT>
struct COPILOT_CMD_WITH_CONTEXT : CONCRETE_TYPE_COPILOT_CMD<CMD_TYPE>
{
    CONTEXT     context;
    friend void to_json( nlohmann ::json& nlohmann_json_j, const COPILOT_CMD_WITH_CONTEXT& nlohmann_json_t )
    {
        to_json( nlohmann_json_j,
                 static_cast<CONCRETE_TYPE_COPILOT_CMD<CMD_TYPE> const&>( nlohmann_json_t ) );
        nlohmann_json_j[kContext] = nlohmann_json_t.context;
    }
    friend void from_json( const nlohmann ::json& nlohmann_json_j,
                           COPILOT_CMD_WITH_CONTEXT&      nlohmann_json_t )
    {
        from_json( nlohmann_json_j, static_cast<CONCRETE_TYPE_COPILOT_CMD<CMD_TYPE>&>( nlohmann_json_t ) );
        nlohmann_json_j.at( kContext ).get_to( nlohmann_json_t.context );
    }
};


#endif
