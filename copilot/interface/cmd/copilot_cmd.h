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

#ifndef COPILOT_CMD_H
#define COPILOT_CMD_H

#include "copilot_cmd_type.h"
#include <nlohmann/json.hpp>
#include <copilot_context.h>
#include <string>

struct CMD_BASE
{
    std::string client_type = "kicad";
    NLOHMANN_DEFINE_TYPE_INTRUSIVE( CMD_BASE, client_type )
};

struct DESIGN_INTENTION : CMD_BASE
{
    DESIGN_GLOBAL_CONTEXT context;
    CMD_TYPE              type = CMD_TYPE::DESIGN_INTENTION;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE( DESIGN_INTENTION, context, type )
};

struct CORE_COMPONENTS : CMD_BASE
{
    DESIGN_GLOBAL_CONTEXT context;
    CMD_TYPE              type = CMD_TYPE::CORE_COMPONENTS;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE( CORE_COMPONENTS, context, type )
};

struct CURRENT_COMPONENT : CMD_BASE
{
    SYMBOL_CMD_CONTEXT context;
    CMD_TYPE           type = CMD_TYPE::CURRENT_COMPONENT;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE( CURRENT_COMPONENT, context, type )
};

struct SIMILAR_COMPONENTS : CMD_BASE
{
    SYMBOL_CMD_CONTEXT context;
    CMD_TYPE           type = CMD_TYPE::SIMILAR_COMPONENTS;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE( SIMILAR_COMPONENTS, context, type )
};

struct CHECK_COMPONENT_CONNECTIONS : CMD_BASE
{
    SYMBOL_CMD_CONTEXT context;
    CMD_TYPE           type = CMD_TYPE::CHECK_COMPONENT_CONNECTIONS;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE( CHECK_COMPONENT_CONNECTIONS, context, type )
};

struct COMPONENT_PINS_DETAILS : CMD_BASE
{
    SYMBOL_CMD_CONTEXT context;
    CMD_TYPE           type = CMD_TYPE::COMPONENT_PINS_DETAILS;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE( COMPONENT_PINS_DETAILS, context, type )
};

struct UNCONNECTED_PINS : CMD_BASE
{
    SYMBOL_CMD_CONTEXT context;
    CMD_TYPE           type = CMD_TYPE::UNCONNECTED_PINS;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE( UNCONNECTED_PINS, context, type )
};

struct GENERIC_CHAT : CMD_BASE
{
    GENERAL_CHAT_CONTEXT context;
    CMD_TYPE             type = CMD_TYPE::GENERIC_CHAT;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE( GENERIC_CHAT, context, type )
};


#endif // COPILOT_CMD_H
