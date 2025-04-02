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

#ifndef WEB_UTILS_H
#define WEB_UTILS_H

#include <map>
#include <optional>
#include <string>
#include <host_type.h>


inline auto add_parameter_to_url( std::string const&                        url,
                                  std::map<std::string, std::string> const& parameters )
{
    std::string result = url;

    // Check if the URL already contains a query string
    bool has_query = result.find( '?' ) != std::string::npos;

    // Append each parameter to the URL
    for( auto const& [key, value] : parameters )
    {
        // Add either '&' or '?' depending on whether the URL already has a query string
        result += ( has_query ? '&' : '?' ) + key + '=' + value;
        has_query = true; // After the first parameter, always use '&'
    }

    return result;
}

inline auto add_parameter_to_url( std::string const& url, std::optional<HOST_TYPE> const& host )
{
    if( !host )
        return url;

    return add_parameter_to_url(
            url, { { "host_name", host->host_name }, { "editor_name", host->editor_name } } );
}


#endif
