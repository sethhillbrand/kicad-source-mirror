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


#ifndef ASSISTANT_INTERFACE_PATH_H
#define ASSISTANT_INTERFACE_PATH_H

#include <macros.h>
#include <string>
#include <wx/app.h>
#include <wx/stdpaths.h>
#include <nlohmann/json.hpp>
#include <copilot_config.h>
#include <filesystem>


extern "C"
{
    const char* GetCopilotSettingsPath();
}


class ASSISTANT_INTERFACE_PATH
{
private:
    static auto get_exe_path()
    {
        std::string exe_path = wxStandardPaths::Get().GetExecutablePath().ToStdString();
        exe_path = std::filesystem::path( exe_path ).parent_path().string();
        return exe_path;
    }

    static auto get_copilot_dir( std::string const& prefix )
    {
        std::string copilot_dir = prefix + "/" TO_STR( COPILOT_CONFIG_DIR );
        return copilot_dir;
    }

    static auto get_copilot_dll_path( std::string const& copilot_dir )
    {
        return copilot_dir + "/" + TO_STR( ASSISTANT_DLL_NAME );
    }


    static auto get_copilot_dir_under_exe() { return get_copilot_dir( get_exe_path() ); }

    static auto get_copilot_dir_under_cnf() { return GetCopilotSettingsPath(); }


    static auto get_copilot_dll_under_exe()
    {
        return get_copilot_dll_path( get_copilot_dir_under_exe() );
    }

    static auto get_copilot_dll_under_cnf()
    {
        return get_copilot_dll_path( get_copilot_dir_under_cnf() );
    }

public:
    static auto generic_get_assistant_dll_path()
    {
        auto assistant_dll_path = get_copilot_dll_under_cnf();
        if( !std::filesystem::exists( assistant_dll_path ) )
        {
            assistant_dll_path = get_copilot_dll_under_exe();
        }

        return assistant_dll_path;
    }
};


#endif
