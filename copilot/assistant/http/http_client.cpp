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

#include "http_client.h"
#include "assistant/settings/copilot_settings_manager.h"
#include <httplib.h>
#include <wx/log.h>
#include "http_utils.h"
#include "proto/data_buried_point.h"
HTTP_CLIENT::HTTP_CLIENT() :
        _client( new httplib::Client(
                COPILOT_SETTINGS_MANAGER::get_instance().get_data_buried_point_host() ) )
{
}

HTTP_CLIENT::~HTTP_CLIENT()
{
}

void HTTP_CLIENT::send_data_buried_point( DATA_BURIED_POINT const& data_buried_point )
{
    auto res = _client->Post(
            COPILOT_SETTINGS_MANAGER::get_instance().get_data_buried_point_endpoint(),
            nlohmann::json( data_buried_point ).dump(), kJsonContextType );

    if( !res )
    {
        wxLogDebug( "send data buried point failed", "error: ", res.error() );
        return;
    }


    if( res->status != HTTP_CODE::SUCCESS )
    {
        wxLogDebug( "send data buried point result error ", "status :", res->status,
                    "error: ", res.error() );
    }
}
