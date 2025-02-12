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

#include "copilot_setting_path.h"
#include <macros.h>
#include <settings/json_settings.h>
#include <wx/filename.h>
#include <wx/log.h>
#include <wx/stdpaths.h>
#include <paths.h>
#include <copilot_config.h>

const char* GetCopilotSettingsPath()
{
    static const auto kCopilotSettingsPath =([]{
    wxFileName path;

    path.AssignDir( PATHS::GetUserSettingsPath() );
    path.AppendDir( TO_STR( COPILOT_CONFIG_DIR ) );

    if( !path.DirExists() )
    {
        if( !wxMkdir( path.GetPath() ) )
        {
            wxLogTrace( traceSettings,
                        wxT( "GetCopilotSettingsPath(): Path %s missing and could not be created!" ),
                        path.GetPath() );
        }
    }

    return path.GetPath();})();

    return kCopilotSettingsPath.c_str();
}
