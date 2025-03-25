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

#ifndef SCH_COPILOT_GLOBAL_CONTEXT_H
#define SCH_COPILOT_GLOBAL_CONTEXT_H

#include <context/copilot_global_context.h>
#include <context/variable_context.h>
#include <context/context_fields.h>
#include <string>


struct SCH_COPILOT_GLOBAL_CONTEXT : COPILOT_GLOBAL_CONTEXT, VARIABLE_CONTEXT
{
    std::string net_list;
    std::list<std::string> designators;

    friend void to_json( nlohmann ::json&                  nlohmann_json_j,
                         const SCH_COPILOT_GLOBAL_CONTEXT& nlohmann_json_t )
    {
        to_json( nlohmann_json_j, static_cast<COPILOT_GLOBAL_CONTEXT const&>( nlohmann_json_t ) );
        nlohmann_json_j[kNetList] = nlohmann_json_t.net_list;
        nlohmann_json_j[kDesignators] = nlohmann_json_t.designators;
    }
    friend void from_json( const nlohmann ::json&      nlohmann_json_j,
                           SCH_COPILOT_GLOBAL_CONTEXT& nlohmann_json_t )
    {
        from_json( nlohmann_json_j, static_cast<COPILOT_GLOBAL_CONTEXT&>( nlohmann_json_t ) );
        nlohmann_json_j.at( kNetList ).get_to( nlohmann_json_t.net_list );
        nlohmann_json_j.at( kDesignators ).get_to( nlohmann_json_t.designators );
    }
    std::string dump() const override { return nlohmann::json( *this ).dump(); }
};

#endif
