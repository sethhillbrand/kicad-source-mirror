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

#include "websocket_endpoint.h"
#include "websocket_event.h"
#include <cmd/cmd_base.h>
#include <exception>
#include <nlohmann/json.hpp>
#include <string>
#include <utility>
#include <websocketpp/close.hpp>
#include <wx/log.h>
#include <iostream>


using websocketpp::lib::bind;

WEBSOCKET_ENDPOINT::WEBSOCKET_ENDPOINT( wxEvtHandler* eventSink, std::atomic_bool& should_quit,
                                        std::string websocket_uri ) :
        _eventSink( eventSink ), _should_quit( should_quit ), m_next_id( 0 ),
        _websocket_uri( std::move( websocket_uri ) )
{
    m_endpoint.clear_access_channels( websocketpp::log::alevel::all );
    m_endpoint.clear_error_channels( websocketpp::log::elevel::all );

    m_endpoint.init_asio();
    m_endpoint.start_perpetual();

    m_thread.reset( new websocketpp::lib::thread( &client::run, &m_endpoint ) );

    // Register our message handler
    m_endpoint.set_message_handler(
            [this]( websocketpp::connection_hdl hdl, message_ptr msg )
            {
                const auto msg_str = msg->get_payload();
                wxString   wx_msg_str( msg_str.c_str(), wxConvUTF8 );

                if( wx_msg_str.empty() )
                    return;

                try
                {
                    WEBSOCKET_PAYLOAD_INTERFACE r;
                    nlohmann::json::parse( msg_str ).get_to( r );
                    wxString          wx_msg( r.msg.c_str(), wxConvUTF8 );
                    WEBSOCKET_PAYLOAD payload{ r.type, wx_msg };
                    const auto        evt = new WEBSOCKET_EVENT( EVT_WEBSOCKET_PAYLOAD );
                    evt->SetPayload( payload );
                    _eventSink->QueueEvent( evt );
                }
                catch( std::exception const& e )
                {
                    wxLogError( "error while parsing message: %s\n", wx_msg_str );
                    wxLogError( e.what() );
                }
            } );

    connect();
}

WEBSOCKET_ENDPOINT::~WEBSOCKET_ENDPOINT()
{
    m_endpoint.stop_perpetual();

    m_endpoint.stop_perpetual();

    for( con_list::const_iterator it = m_connection_list.begin(); it != m_connection_list.end();
         ++it )
    {
        if( it->second->get_status() != CONNECTION_STATE::OPEN )
        {
            // Only close open connections
            continue;
        }

        std::cout << "> Closing connection " << it->second->get_id() << std::endl;

        websocketpp::lib::error_code ec;
        m_endpoint.close( it->second->get_hdl(), websocketpp::close::status::going_away, "", ec );
        if( ec )
        {
            wxLogError( "> Error closing connection ", it->second->get_id(), ": ", ec.message() );
        }
    }

    try
    {
        m_thread->join();
    }
    catch( const std::system_error& e )
    {
        wxLogError( "Caught a system_error exception: ", e.what() );
    }
    catch( const std::exception& e )
    {
        wxLogError( "Caught an exception: ", e.what() );
    }
    catch( ... )
    {
        wxLogError( "Caught an unknown exception" );
    }
}

void WEBSOCKET_ENDPOINT::send( std::string const& msg )
{
    try
    {
        auto       j = nlohmann::json::parse( msg );
        const auto cmd_base = j.get<CMD_BASE>();


        if( cmd_base.global_context_uuid
            && _consumed_context_ids.contains( *cmd_base.global_context_uuid ) )
        {
            j.erase( kDesignGlobalContext );
        }

        for( const auto [k, v] : m_connection_list )
        {
            if( v->get_status() == CONNECTION_STATE::OPEN )
            {
                websocketpp::lib::error_code ec;
                m_endpoint.send( v->get_hdl(), j.dump(), websocketpp::frame::opcode::text, ec );

                if( ec )
                {
                    const auto er_msg = ec.message();
                    wxLogError( "Echo failed because: ", er_msg );
                    return;
                }

                if( cmd_base.global_context_uuid )
                    _consumed_context_ids.insert( *cmd_base.global_context_uuid );

                return;
            }
        }
    }
    catch( std::exception const& e )
    {
        wxLogError( "error while parsing message: %s\n", msg );
        wxLogError( e.what() );
    }
}

int WEBSOCKET_ENDPOINT::connect()
{
    if( _should_quit )
        return INVALID_ID;

    websocketpp::lib::error_code ec;

    client::connection_ptr con = m_endpoint.get_connection( _websocket_uri, ec );

    if( ec )
    {
        wxLogError( "> Connect initialization error: ", ec.message() );
        return INVALID_ID;
    }

    int                      new_id = m_next_id++;
    connection_metadata::ptr metadata_ptr( new connection_metadata( new_id, con->get_handle(),
                                                                    _websocket_uri,
                                                                    [this]()
                                                                    {
                                                                        // FIXME shall be event driven with queued msg
                                                                        connect();
                                                                    } ) );
    m_connection_list[new_id] = metadata_ptr;

    con->set_open_handler( websocketpp::lib::bind( &connection_metadata::on_open, metadata_ptr,
                                                   &m_endpoint,
                                                   websocketpp::lib::placeholders::_1 ) );
    con->set_fail_handler( websocketpp::lib::bind( &connection_metadata::on_fail, metadata_ptr,
                                                   &m_endpoint,
                                                   websocketpp::lib::placeholders::_1 ) );
    con->set_close_handler( websocketpp::lib::bind( &connection_metadata::on_close, metadata_ptr,
                                                    &m_endpoint,
                                                    websocketpp::lib::placeholders::_1 ) );

    m_endpoint.connect( con );

    return new_id;
}
