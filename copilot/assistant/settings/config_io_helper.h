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

#ifndef CONFIG_IO_HELPER_H
#define CONFIG_IO_HELPER_H

#include <nlohmann/json.hpp>
#include <string>
#include <wx/filename.h>
#include <wx/log.h>
#include <wx/stdpaths.h>
#include <wx/file.h>
#include <fstream>
#include <fmt/format.h>
#include <iostream>
#include <macros.h>
#include "copilot_config_version.h"

inline auto save_cnf( std::string const& path, const auto& cnf )
{
    try
    {
        std::ofstream out( path );
        out << ( nlohmann::json( cnf ) ).dump();
        out.close();
    }
    catch( std::exception& e )
    {
        std::cerr << e.what() << std::endl;
    }
    catch( ... )
    {
        std::cerr << "unknown exception" << std::endl;
    }
}


template <typename T>
auto load_cnf( std::string const& path, T& aCnf )
{
    if( !wxFileExists( path ) )
    {
        wxLogTrace( "COPILOT_SETTINGS_MANAGER",
                    wxT( "COPILOT_SETTINGS_MANAGER(): Setting file %s does not exist, creating a "
                         "new one." ),
                    path.c_str() );

        save_cnf( path, aCnf );
    }
    else
    {
        try
        {
            std::ifstream in( path );
            auto          cnf = nlohmann::json::parse( in ).get<T>();

            if( cnf.version == kConfigVersion )
            {
                aCnf = cnf;
            }
        }
        catch( const std::exception& e )
        {
            wxLogTrace( "COPILOT_SETTINGS_MANAGER",
                        wxT( "COPILOT_SETTINGS_MANAGER(): Failed to parse setting file %s, using "
                             "default settings." ) );
            save_cnf( path, aCnf );
        }
    }
}

#endif
