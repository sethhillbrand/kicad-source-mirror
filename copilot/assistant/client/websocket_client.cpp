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

#include "websocket_client.h"
#include <wx/log.h>
#include <iostream>


using websocketpp::lib::bind;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;


constexpr auto kUri = "ws://localhost:9002";

// constexpr auto kUri = "ws://www.fdatasheets.com/kicad/chat/36335";

void on_message( client* c, websocketpp::connection_hdl hdl, message_ptr msg )
{
    const auto msg_str = msg->get_payload();
    wxLogMessage( msg_str.c_str() );
}

WEBSOCKET_CLIENT::WEBSOCKET_CLIENT() : _client( new client )
{
    try
    {
        // Set logging to be pretty verbose (everything except message payloads)
        _client->set_access_channels( websocketpp::log::alevel::all );
        _client->clear_access_channels( websocketpp::log::alevel::frame_payload );

        // Initialize ASIO
        _client->init_asio();

        // Register our message handler
        _client->set_message_handler( bind( &on_message, _client.get(), ::_1, ::_2 ) );

        websocketpp::lib::error_code ec;
        _con = _client->get_connection( kUri , ec );
        if( ec )
        {
            const auto er_msg = ec.message();
            std::cerr << "Connect failed because: " << er_msg << std::endl;
            return;
        }

        // Note that connect here only requests a connection. No network messages are
        // exchanged until the event loop starts running in the next line.

        _client->start_perpetual();


        // Start the ASIO io_service run loop
        // this will cause a single connection to be made to the server. c.run()
        // will exit when this connection is closed.
        _thread.reset( new websocketpp::lib::thread( &client::run, _client.get() ) );


        _client->connect( _con );
    }
    catch( websocketpp::exception const& e )
    {
        std::cout << e.what() << std::endl;
    }
}

WEBSOCKET_CLIENT::~WEBSOCKET_CLIENT()
{
}

void WEBSOCKET_CLIENT::send( std::string const& msg )
{
    websocketpp::lib::error_code ec;
    _client->send( _con, msg, websocketpp::frame::opcode::text, ec );

    if( ec )
    {
        const auto er_msg = ec.message();
        std::cerr << "Echo failed because: " << er_msg << std::endl;
    }
}


void WEBSOCKET_CLIENT::quit()
{
    try
    {
        _client->stop_perpetual();
        _client->close( _con, websocketpp::close::status::normal, "user close" );
        _thread->join();
    }
    catch( ... )
    {
    }
}