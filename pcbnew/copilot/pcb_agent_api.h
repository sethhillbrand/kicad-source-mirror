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

#ifndef PCB_AGENT_API_H
#define PCB_AGENT_API_H

#include <pcb_edit_frame.h>
#include <action_plugin.h>
#include <wx/log.h>
#include <wx/msgdlg.h>

void PCB_EDIT_FRAME::LaunchPlugin( std::string const&            aPluginName,
                                   std::optional<nlohmann::json> aParams )
{
    static const auto kSupportedPlugins =
            std::unordered_map<std::string, std::string>{ { "dfm", "HQ DFM" } };

    if( !kSupportedPlugins.contains( aPluginName ) )
    {
        wxLogError( wxT( "Plugin %s is not supported" ), aPluginName );
        return;
    }


    const auto plugin_name = kSupportedPlugins.at( aPluginName );

    if( ACTION_PLUGINS::IsActionRunning() )
    {
        wxLogError( wxT( "Another plugin is running" ) );
        return;
    }


    for( auto i = 0; i < ACTION_PLUGINS::GetActionsCount(); i++ )
    {
        const auto plugin = ACTION_PLUGINS::GetAction( i );
        const auto act_name = plugin->GetName().ToStdString();

        if( act_name == plugin_name )
        {
            plugin->Run();
            return;
        }
    }

    wxLogError( wxT( "Action plugin %s not installed" ), aPluginName );
}


#endif
