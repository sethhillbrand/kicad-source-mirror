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
#include "config_io_helper.h"


constexpr auto kMutableSettingsFileName = "mutable_settings.json";
constexpr auto kImmutableSettingsFileName = "immutable_settings.json";

COPILOT_SETTINGS_MANAGER::COPILOT_SETTINGS_MANAGER()
{
    load_cnf( get_copilot_setting_path( kImmutableSettingsFileName ), _immutable_settings );
    load_cnf( get_copilot_setting_path( kMutableSettingsFileName ), _mutable_settings );

    auto url = _immutable_settings.webview_settings.url;

    if( url.back() == '/' )
        url = url.substr( 0, url.length() - 1 );


    _webview_chat_path = fmt::format( "{}/#{}", _immutable_settings.webview_settings.url,
                                      _immutable_settings.webview_settings.path.chat );

    _webview_image_viewer_path =
            fmt::format( "{}/#{}", _immutable_settings.webview_settings.url,
                         _immutable_settings.webview_settings.path.image_viewer );
}

COPILOT_SETTINGS_MANAGER::~COPILOT_SETTINGS_MANAGER()
{
    save_cnf( get_copilot_setting_path( kMutableSettingsFileName ), _mutable_settings );
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
        
        for(const auto& dir : { TO_STR( KICAD_CONFIG_DIR ) , TO_STR( COPILOT_CONFIG_DIR ) } ){
            path.AppendDir( dir );
            mk_dir();
        }
        
        return path.GetPath().ToStdString( wxConvUTF8 );
    })();


    return kCopilotSettingDir;
}

std::string COPILOT_SETTINGS_MANAGER::get_copilot_setting_path( std::string const& file_name )
{
    return get_copilot_setting_dir() + "/" + file_name;
}


std::string const& COPILOT_SETTINGS_MANAGER::get_data_buried_point_host() const
{
    return _immutable_settings.data_buried_point_settings.host;
}

std::string const& COPILOT_SETTINGS_MANAGER::get_data_buried_point_endpoint() const
{
    return _immutable_settings.data_buried_point_settings.endpoint;
}

std::string const& COPILOT_SETTINGS_MANAGER::get_webview_url() const
{
    return _immutable_settings.webview_settings.url;
}

WEBVIEW_PATH const& COPILOT_SETTINGS_MANAGER::get_webview_path() const
{
    return _immutable_settings.webview_settings.path;
}

std::string const& COPILOT_SETTINGS_MANAGER::get_webview_chat_path() const
{
    return _webview_chat_path;
}

std::string const& COPILOT_SETTINGS_MANAGER::get_webview_image_viewer_path() const
{
    return _webview_image_viewer_path;
}

DIALOG_GEOMETRY const& COPILOT_SETTINGS_MANAGER::get_image_viewer_geometry() const
{
    return _mutable_settings.image_viewer_geometry;
}

void COPILOT_SETTINGS_MANAGER::set_image_viewer_geometry( DIALOG_GEOMETRY const& geometry )
{
    _mutable_settings.image_viewer_geometry = geometry;
}
