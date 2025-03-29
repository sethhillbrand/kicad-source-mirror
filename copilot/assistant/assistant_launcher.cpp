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

#include "assistant_launcher.h"
#include "webview/webview_container.h"
#include <thread>
#include "http/http_client.h"
#include "proto/data_buried_point.h"


inline auto send_data_buried_point()
{
    std::thread t(
            [&]()
            {
                try
                {
                    HTTP_CLIENT http_client;
                    http_client.send_data_buried_point( DATA_BURIED_POINT{} );
                }
                catch( std::exception const& e )
                {
                    wxLogTrace( wxString::Format( "send_data_buried_point error: %s", e.what() ),
                                "send_data_buried_point" );
                }
                catch( ... )
                {
                    wxLogTrace( wxString::Format( "send_data_buried_point error: unknown" ),
                                "send_data_buried_point" );
                }
            } );
    t.detach();
}


wxPanel* create_assistant_panel( wxWindow*                  parent,
                                 COPILOT_GLOBAL_CONTEXT_HDL get_design_global_context )
{
    send_data_buried_point();
    return new WEBVIEW_CONTAINER( parent, get_design_global_context );
}

void fire_cmd( wxPanel* target, const char* cmd )
{
    static_cast<WEBVIEW_CONTAINER*>( target )->fire_host_active_cmd( cmd );
}
