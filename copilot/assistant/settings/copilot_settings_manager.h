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

#ifndef COPILOT_SETTINGS_MANAGER_H
#define COPILOT_SETTINGS_MANAGER_H

#include <string>
#include <memory>

struct COPILOT_SETTINGS;
struct WEBVIEW_PATH;
class COPILOT_SETTINGS_MANAGER
{
public:
    ~COPILOT_SETTINGS_MANAGER();

    static COPILOT_SETTINGS_MANAGER& get_instance();

    static std::string get_copilot_setting_dir();

    static std::string get_copilot_setting_path();

    std::string const& get_data_buried_point_host() const;

    std::string const& get_data_buried_point_endpoint() const;

    std::string const& get_webview_url() const;

    WEBVIEW_PATH const& get_webview_path() const;

    std::string const& get_webview_chat_path() const;


private:
    COPILOT_SETTINGS_MANAGER();
    std::unique_ptr<COPILOT_SETTINGS> _settings;
    bool                              _settings_is_valid;
    std::string                       _webview_chat_path;
};

#endif
