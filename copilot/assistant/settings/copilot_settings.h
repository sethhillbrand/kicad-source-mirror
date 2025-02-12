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

#include "copilot_config_version.h"

struct DATA_BURIED_POINT_SETTINGS
{
    std::string host = "blog.eda.cn";
    std::string endpoint = "/data_buried_point";
    NLOHMANN_DEFINE_TYPE_INTRUSIVE( DATA_BURIED_POINT_SETTINGS, host, endpoint )
};


struct WEBVIEW_PATH
{
    std::string home = "/";
    std::string chat = "/chat";
    std::string image_viewer = "/image-viewer";
    NLOHMANN_DEFINE_TYPE_INTRUSIVE( WEBVIEW_PATH, home, chat, image_viewer )
};

struct WEBVIEW_SETTINGS
{
    std::string url = "https://chat.eda.cn/";
    WEBVIEW_PATH path;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE( WEBVIEW_SETTINGS, url, path )
};


struct DIALOG_GEOMETRY
{
    int x = -1;
    int y = -1;
    int width = 900;
    int height = 600;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE( DIALOG_GEOMETRY, x, y, width, height )
};


struct IMMUTABLE_SETTINGS
{
    DATA_BURIED_POINT_SETTINGS data_buried_point_settings;
    WEBVIEW_SETTINGS           webview_settings;
    std::string                version = kConfigVersion;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE( IMMUTABLE_SETTINGS, data_buried_point_settings,
                                    webview_settings, version )
};


struct MUTABLE_SETTINGS
{
    DIALOG_GEOMETRY image_viewer_geometry;
    std::string     version = kConfigVersion;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE( MUTABLE_SETTINGS, image_viewer_geometry, version )
};

#endif
