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

#ifndef COPILOT_SETTINGS_H
#define COPILOT_SETTINGS_H

#include <string>
#include <nlohmann/json.hpp>


struct COPILOT_WEBSOCKET_SETTINGS{
    std::string websocket_uri = "ws://www.fdatasheets.com/kicad/chat";
    NLOHMANN_DEFINE_TYPE_INTRUSIVE( COPILOT_WEBSOCKET_SETTINGS, websocket_uri );
};

struct DATA_BURIED_POINT_SETTINGS
{
    std::string host = "blog.eda.cn";
    std::string endpoint = "/data_buried_point";
    NLOHMANN_DEFINE_TYPE_INTRUSIVE( DATA_BURIED_POINT_SETTINGS, host, endpoint)
};



struct WEBVIEW_SETTINGS {
    std::string url = "http://localhost:3001";

    NLOHMANN_DEFINE_TYPE_INTRUSIVE( WEBVIEW_SETTINGS, url )
};




struct COPILOT_SETTINGS
{
   COPILOT_WEBSOCKET_SETTINGS websocket_settings;
   DATA_BURIED_POINT_SETTINGS data_buried_point_settings;
   WEBVIEW_SETTINGS webview_settings;
   NLOHMANN_DEFINE_TYPE_INTRUSIVE( COPILOT_SETTINGS, websocket_settings, data_buried_point_settings ,webview_settings)
};

#endif
