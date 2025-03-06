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

#include "copilot_settings_manager.h"
#include "copilot_settings.h"
#include <nlohmann/json.hpp>
#include <wx/filename.h>
#include <wx/log.h>
#include <wx/stdpaths.h>
#include <wx/file.h>
#include <fstream>

COPILOT_SETTINGS_MANAGER::COPILOT_SETTINGS_MANAGER() : _settings( new COPILOT_SETTINGS )
{
    // Check if the setting file exists
    const auto setting_path = get_copilot_setting_path();

    if( !wxFileExists( setting_path ) )
    {
        wxLogTrace( "COPILOT_SETTINGS_MANAGER",
                    wxT( "COPILOT_SETTINGS_MANAGER(): Setting file %s does not exist, creating a "
                         "new one." ),
                    setting_path.c_str() );

        std::ofstream out( setting_path );
        out << ( nlohmann::json( *_settings ) ).dump();
        out.close();
    }
    else
    {
        try
        {
            std::ifstream in( setting_path );
            nlohmann::json::parse( in ).get_to( *_settings );
            in.close();
        }
        catch( const std::exception& e )
        {
            wxLogTrace( "COPILOT_SETTINGS_MANAGER",
                        wxT( "COPILOT_SETTINGS_MANAGER(): Failed to parse setting file %s, using "
                             "default settings." ) );
        }
    }


    _runtime_websocket_uri =
            _settings->websocket_uri + "/" + std::to_string( std::rand() % 90000 + 10000 );
}

COPILOT_SETTINGS_MANAGER::~COPILOT_SETTINGS_MANAGER()
{
}

COPILOT_SETTINGS_MANAGER& COPILOT_SETTINGS_MANAGER::get_instance()
{
    static COPILOT_SETTINGS_MANAGER instance;
    return instance;
}

std::string COPILOT_SETTINGS_MANAGER::get_copilot_setting_dir()
{
    static const auto kCopilotSettingDir = ([]{
        wxFileName path;
        const auto mk_dir =[&]{        
            if( !path.DirExists() )
            {
                if( !wxMkdir( path.GetPath() ) )
                {
                    wxLogTrace( "COPILOT_SETTINGS_MANAGER",
                                wxT( "get_copilot_setting_dir(): Path %s missing and could not be created!" ),
                                path.GetPath() );
                }
            }
        };

        path.AssignDir(  wxStandardPaths::Get().GetUserConfigDir() );
        
        for(const auto& dir : { wxS( "kicad" ), wxS( "copilot" ) } ){
            path.AppendDir( dir );
            mk_dir();
        }
        
        return path.GetPath().ToStdString( wxConvUTF8 );
    })();


    return kCopilotSettingDir;
}

std::string COPILOT_SETTINGS_MANAGER::get_copilot_history_db_path()
{
    return get_copilot_setting_dir() + "/history.db";
}

std::string COPILOT_SETTINGS_MANAGER::get_copilot_setting_path()
{
    return get_copilot_setting_dir() + "/copilot_settings.json";
}


std::string const& COPILOT_SETTINGS_MANAGER::get_websocket_uri() const
{
    return _runtime_websocket_uri;
}

std::string const& COPILOT_SETTINGS_MANAGER::get_data_buried_point_url() const
{
    return _settings->data_buried_point_url;
}
