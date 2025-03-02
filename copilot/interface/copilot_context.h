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

#ifndef COPILOT_CONTEXT_H
#define COPILOT_CONTEXT_H

#include "sch/symbol_properties.h"
#include <nlohmann/json.hpp>
#include <string>

using NET_LIST = std::string;
using NET = std::string;
using BOM = std::string;


struct DESIGN_GLOBAL_CONTEXT
{
    BOM bom;
    NET_LIST net_list;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE( DESIGN_GLOBAL_CONTEXT, bom, net_list )
};


struct SYMBOL_CMD_CONTEXT
{
    std::string       designator;
    SYMBOL_PROPERTIES symbol_properties;
    NET               net;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE( SYMBOL_CMD_CONTEXT, designator, symbol_properties, net )
};

struct GENERAL_CHAT_CONTEXT : DESIGN_GLOBAL_CONTEXT
{
    std::string user_input;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE( GENERAL_CHAT_CONTEXT, user_input , bom, net_list )
};


#endif
