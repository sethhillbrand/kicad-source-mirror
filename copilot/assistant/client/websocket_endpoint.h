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

#ifndef WEBSOCKET_ENDPOINT_H
#define WEBSOCKET_ENDPOINT_H

#include <functional>
#include <string>
#include <utility>
#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>
#include <memory>
#include <wx/event.h>


typedef websocketpp::client<websocketpp::config::asio_client> client;
typedef websocketpp::config::asio_client::message_type::ptr   message_ptr;


enum class CONNECTION_STATE
{
    CONNECTING,
    OPEN,
    FAILED,
    CLOSED
};

using ON_CLOSE = std::function<void()>;

class connection_metadata
{
    ON_CLOSE _on_close;
public:
    typedef websocketpp::lib::shared_ptr<connection_metadata> ptr;

    connection_metadata( int id, websocketpp::connection_hdl hdl, std::string uri  ,ON_CLOSE on_close) :
            m_id( id ), m_hdl( hdl ), m_uri( uri ), m_server( "N/A" ),_on_close(std::move(on_close))
    {
    }

    void on_open( client* c, websocketpp::connection_hdl hdl )
    {
        m_status = CONNECTION_STATE::OPEN;

        client::connection_ptr con = c->get_con_from_hdl( hdl );
        m_server = con->get_response_header( "Server" );
    }

    void on_fail( client* c, websocketpp::connection_hdl hdl )
    {
        m_status = CONNECTION_STATE::FAILED;

        client::connection_ptr con = c->get_con_from_hdl( hdl );
        m_server = con->get_response_header( "Server" );
        m_error_reason = con->get_ec().message();
    }

    void on_close( client* c, websocketpp::connection_hdl hdl )
    {
        _on_close();
        m_status = CONNECTION_STATE::CLOSED;
        client::connection_ptr con = c->get_con_from_hdl( hdl );
        std::stringstream      s;
        s << "close code: " << con->get_remote_close_code() << " ("
          << websocketpp::close::status::get_string( con->get_remote_close_code() )
          << "), close reason: " << con->get_remote_close_reason();
        m_error_reason = s.str();
    }

    websocketpp::connection_hdl get_hdl() const { return m_hdl; }

    int get_id() const { return m_id; }

    auto get_status() const { return m_status; }

    friend std::ostream& operator<<( std::ostream& out, connection_metadata const& data );

private:
    int                         m_id;
    websocketpp::connection_hdl m_hdl;
    CONNECTION_STATE            m_status{};
    std::string                 m_uri;
    std::string                 m_server;
    std::string                 m_error_reason;
};


class WEBSOCKET_ENDPOINT
{
    enum
    {
        INVALID_ID = -1
    };

public:
    WEBSOCKET_ENDPOINT( wxEvtHandler* eventSink ,std::atomic_bool& should_quit);
    ~WEBSOCKET_ENDPOINT();

    void send( std::string const& msg );

    void close( int id, websocketpp::close::status::value code, std::string reason )
    {
        websocketpp::lib::error_code ec;

        con_list::iterator metadata_it = m_connection_list.find( id );
        if( metadata_it == m_connection_list.end() )
        {
            std::cout << "> No connection found with id " << id << std::endl;
            return;
        }

        m_endpoint.close( metadata_it->second->get_hdl(), code, reason, ec );
        if( ec )
        {
            std::cout << "> Error initiating close: " << ec.message() << std::endl;
        }
    }

private:
    int connect();


private:
    wxEvtHandler*                                   _eventSink;
    typedef std::map<int, connection_metadata::ptr> con_list;

    client                                                 m_endpoint;
    websocketpp::lib::shared_ptr<websocketpp::lib::thread> m_thread;

    con_list m_connection_list;
    int      m_next_id;
    std::atomic_bool& _should_quit;
};

#endif
