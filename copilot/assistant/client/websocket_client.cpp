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
#include <exception>
#include <nlohmann/json.hpp>
#include <websocketpp/close.hpp>
#include <wx/log.h>
#include <iostream>


using websocketpp::lib::bind;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;


struct RESPONSE
{
    MEG_TYPE    type{};
    std::string msg;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE( RESPONSE, type, msg );
};


// constexpr auto kUri = "ws://localhost:9002";

constexpr auto kUri = "ws://www.fdatasheets.com/kicad/chat/36335";

void on_message( client* c, MEG_TYPE& pre_type, websocketpp::connection_hdl hdl, message_ptr msg )
{
    const auto msg_str = msg->get_payload();
    wxString   wx_msg_str( msg_str.c_str(), wxConvUTF8 );

    if( wx_msg_str.empty() )
        return;

    try
    {
        RESPONSE r;
        nlohmann::json::parse( msg_str ).get_to( r );
        wxString wx_msg( r.msg.c_str(), wxConvUTF8 );

        if( wx_msg.empty() )
            return;

        switch( r.type )
        {
        case CONTENT:
        {
            if( pre_type == END_OF_CHAT )
                wxLogMessage( "\nA:" );

            wxLogMessage( wx_msg );
            break;
        }
        case END_OF_CHAT:
        {
            if( pre_type == CONTENT )
                wxLogMessage( "\n" );
            break;
        }
        }

        pre_type = r.type;
    }
    catch( std::exception const& e )
    {
        wxLogMessage( "error while parsing message: %s\n", wx_msg_str );
    }
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
        _client->set_message_handler(
                bind( &on_message, _client.get(), _previous_type, ::_1, ::_2 ) );

        websocketpp::lib::error_code ec;
        _con = _client->get_connection( kUri, ec );
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
        std::cerr << e.what() << std::endl;
    }
}

WEBSOCKET_CLIENT::~WEBSOCKET_CLIENT()
{
    std::cout << "WEBSOCKET_CLIENT::~WEBSOCKET_CLIENT()" << std::endl;

    _client->stop_perpetual();

    try
    {
        _client->close( _con, websocketpp::close::status::going_away, "user close" );
    }
    catch( std::exception const& e )
    {
        std::cerr << e.what() << std::endl;
    }

    try
    {
        _thread->join();
    }
    catch( const std::system_error& e )
    {
        std::cerr << "Caught a system_error exception: " << e.what() << std::endl;
    }
    catch( const std::exception& e )
    {
        std::cerr << "Caught an exception: " << e.what() << std::endl;
    }
    catch( ... )
    {
        std::cerr << "Caught an unknown exception" << std::endl;
    }
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
