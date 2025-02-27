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

#ifndef ASSISTANT_INTERFACE_H
#define ASSISTANT_INTERFACE_H

#include <dylib.hpp>
#include <optional>
#include <string>
#include <wx/app.h>
#include <memory>
#include <wx/stdpaths.h>
#include <wx/panel.h>
#include <nlohmann/json.hpp>


using CREATE_CHAT_PANEL_HANDEL = wxPanel* (*) ( wxWindow* );
using FIRE_CMD_HANDEL = void ( * )( wxPanel*, const char* );
class ASSISTANT_INTERFACE
{
    bool _is_assistant_available = false;
public:
    ~ASSISTANT_INTERFACE() {}

    static ASSISTANT_INTERFACE& get_instance()
    {
        static ASSISTANT_INTERFACE assistant;
        return assistant;
    }

    auto is_assistant_available() const
    {
        return _is_assistant_available;
    }

    auto load()
    {
        try
        {
            std::string exe_path = wxStandardPaths::Get().GetExecutablePath().ToStdString();
            exe_path = std::filesystem::path( exe_path ).parent_path().string();
            std::string assistant_full_path = exe_path + "/assistant";
            _assistant = std::make_unique<dylib>( assistant_full_path );
            _create_chat_panel_handel =
                    _assistant->get_function<wxPanel*( wxWindow* )>( "create_chat_panel" );
            if( !_create_chat_panel_handel )
            {
                wxLogError( "Failed to load function: create_chat_panel" );
                _assistant.reset();
                return false;
            }

            _fire_cmd_handel =
                    _assistant->get_function<void( wxPanel*, const char* )>( "fire_cmd" );
            if( !_fire_cmd_handel )
            {
                wxLogError( "Failed to load function: fire_cmd" );
                _assistant.reset();
                return false;
            }
            _assistant_version = _assistant->get_variable<const char*>( "COPILOT_VERSION" );
        }
        catch( const std::exception& e )
        {
            wxLogError( "Failed to load assistant: %s", e.what() );
            _assistant.reset();
            return false;
        }

        return true;
    }

    void close() { _assistant.reset(); }

    wxPanel* create_chat_panel( wxWindow* parent )
    {
        if( !_assistant )
        {
            if( !load() )
            {
                return nullptr;
            }
        }

        if( !_create_chat_panel_handel )
        {
            wxLogError( "create_chat_panel_handel is not initialized" );
            return nullptr;
        }

        return _create_chat_panel_handel( parent );
    }

    void fire_cmd( wxPanel* target, const std::string& cmd )
    {
        if( !_assistant )
        {
            if( !load() )
            {
                return;
            }
        }

        if( !_fire_cmd_handel )
        {
            wxLogError( "fire_cmd_handel is not initialized" );
            return;
        }

        _fire_cmd_handel( target, cmd.c_str() );
    }

    template <typename T>
    void chat( wxPanel* target, const T& context )
    {
        fire_cmd( target, nlohmann::json( context ).dump() );
    }


    std::optional<std::string> get_assistant_version()
    {
        if( !_assistant )
        {
            if( !load() )
            {
                return {};
            }
        }

        return _assistant_version;
    }


private:
    std::unique_ptr<dylib> _assistant;

    CREATE_CHAT_PANEL_HANDEL _create_chat_panel_handel{};
    FIRE_CMD_HANDEL          _fire_cmd_handel{};
    std::string              _assistant_version;

    ASSISTANT_INTERFACE() : _is_assistant_available(load()) {  }
};


#endif
