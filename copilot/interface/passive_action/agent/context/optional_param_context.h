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

#ifndef OPTIONAL_PARAM_CONTEXT_H
#define OPTIONAL_PARAM_CONTEXT_H

#include <optional>
#include <nlohmann/json.hpp>


struct OPTIONAL_PARAM_CONTEXT
{
    std::optional<nlohmann::json> params;

    friend void to_json( nlohmann::json&               nlohmann_json_j,
                         const OPTIONAL_PARAM_CONTEXT& nlohmann_json_t )
    {
        if( nlohmann_json_t.params.has_value() )
            nlohmann_json_j["params"] = nlohmann_json_t.params.value();

    }
    friend void from_json( const nlohmann::json&   nlohmann_json_j,
                           OPTIONAL_PARAM_CONTEXT& nlohmann_json_t )
    {
        if( nlohmann_json_j.contains( "params" ) )
            nlohmann_json_t.params = nlohmann_json_j.at( "params" );

    }
};

#endif
