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

#include "optional_context.h"
#include "sch/symbol_properties.h"
#include <nlohmann/json.hpp>
#include <string>
#include <chrono>


struct DESIGN_GLOBAL_CONTEXT
{
    std::string net_list;
    long long   timestamp{ (
            []
            {
                return std::chrono::duration_cast<std::chrono::milliseconds>(
                               std::chrono::system_clock::now().time_since_epoch() )
                        .count();
            } )() };
    NLOHMANN_DEFINE_TYPE_INTRUSIVE( DESIGN_GLOBAL_CONTEXT, timestamp, net_list )
};


struct SYMBOL_CMD_CONTEXT_BASE
{
    std::string       designator;
    SYMBOL_PROPERTIES symbol_properties;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE( SYMBOL_CMD_CONTEXT_BASE, designator, symbol_properties )
};


static const char kDesignGlobalCtx[] = "design_global_ctx";
using OPTIONAL_DESIGN_GLOBAL_CONTEXT = OPTIONAL_CONTEXT<kDesignGlobalCtx, DESIGN_GLOBAL_CONTEXT>;

struct SYMBOL_CMD_CONTEXT
{
    SYMBOL_CMD_CONTEXT_BASE        symbol_ctx;
    OPTIONAL_DESIGN_GLOBAL_CONTEXT global_ctx;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE( SYMBOL_CMD_CONTEXT, global_ctx, symbol_ctx )
};


#endif
