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
    std::string settings = "/settings";
    std::string new_chat = "/new-chat";
    std::string masks = "/masks";
    std::string plugins = "/plugins";
    std::string auth = "/auth";
    std::string sd = "/sd";
    std::string sd_new = "/sd-new";
    std::string artifacts = "/artifacts";
    std::string search_chat = "/search-chat";
    std::string mcp_market = "/mcp-market";
    NLOHMANN_DEFINE_TYPE_INTRUSIVE( WEBVIEW_PATH, home, chat, settings, new_chat, masks, plugins,
                                    auth, sd, sd_new, artifacts, search_chat, mcp_market )
};

struct WEBVIEW_SETTINGS
{
    std::string  url = "http://192.168.50.230:3000";
    WEBVIEW_PATH path;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE( WEBVIEW_SETTINGS, url, path )
};

struct FORCE_UPDATE_SETTING{
    std::string v_0_0_0 = "0.0.0";
    NLOHMANN_DEFINE_TYPE_INTRUSIVE( FORCE_UPDATE_SETTING, v_0_0_0 )
};


struct COPILOT_SETTINGS
{
    DATA_BURIED_POINT_SETTINGS data_buried_point_settings;
    WEBVIEW_SETTINGS           webview_settings;
    FORCE_UPDATE_SETTING          force_update_setting;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE( COPILOT_SETTINGS, data_buried_point_settings, webview_settings , force_update_setting)
};

#endif
