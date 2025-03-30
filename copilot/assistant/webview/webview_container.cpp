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

#include "webview_container.h"
#include "settings/copilot_settings_manager.h"
#include "webview_constant.h"

#include <passive_action/web/web_host.h>
#include <passive_action/passive_action_container.h>
#include <passive_action/agent/agent_action_context.h>
#include <passive_action/agent/agent_action.h>
#include <passive_action/agent/agent_action_type.h>
#include <wx/log.h>
#include <wx/sizer.h>
#include <wx/filename.h>
#include <wx/stdpaths.h>
#include <wx/string.h>
#include <wx/webview.h>
#include <wx/webviewarchivehandler.h>
#include <wx/webviewfshandler.h>
#include <context/copilot_global_context_handle.h>
#include <context/copilot_global_context.h>
#include <copilot_global.h>
#include <format>
#include <magic_enum.hpp>

#if wxUSE_WEBVIEW_EDGE
#include <wx/msw/webview_edge.h>
#endif


enum START_UP_SIZE
{
    WIDTH = 400,
    HEIGHT = 1040
};

WEBVIEW_CONTAINER::WEBVIEW_CONTAINER( wxWindow*            parent,
                                      HOST_COPILOT_HANDLES host_copilot_handles ) :
        wxPanel( parent ), _browser( wxWebView::New() ),
        _host_copilot_handles( std::move( host_copilot_handles ) )
{
#ifdef DEBUG
    // new wxLogWindow( this, _( "Logging" ), true, false );
#endif // DEBUG
    auto top_sizer = new wxBoxSizer( wxVERTICAL );
#ifdef __WXMAC__
    // With WKWebView handlers need to be registered before creation
    _browser->RegisterHandler(
            wxSharedPtr<wxWebViewHandler>( new wxWebViewArchiveHandler( "wxfs" ) ) );
    _browser->RegisterHandler(
            wxSharedPtr<wxWebViewHandler>( new wxWebViewFSHandler( "memory" ) ) );
#endif
    _browser->Create( this, wxID_ANY,
                      COPILOT_SETTINGS_MANAGER::get_instance().get_webview_chat_path(),
                      wxDefaultPosition, wxDefaultSize );
    top_sizer->Add( _browser, wxSizerFlags().Expand().Proportion( 1 ) );

#ifndef __WXMAC__
    //We register the wxfs:// protocol for testing purposes
    _browser->RegisterHandler(
            wxSharedPtr<wxWebViewHandler>( new wxWebViewArchiveHandler( "wxfs" ) ) );
    //And the memory: file system
    _browser->RegisterHandler(
            wxSharedPtr<wxWebViewHandler>( new wxWebViewFSHandler( "memory" ) ) );
#endif


    if( !_browser->AddScriptMessageHandler(
                magic_enum::enum_name( WEBVIEW_MSG_HANDLES::eda_host ).data() ) )
    {
        wxLogError( "Could not add script message handler " );
    }

    SetSizer( top_sizer );
    //Set a more sensible size for web browsing
    SetSize( FromDIP( wxSize( START_UP_SIZE::WIDTH, START_UP_SIZE::HEIGHT ) ) );

    // Connect the webview events
    Bind( wxEVT_WEBVIEW_NAVIGATING, &WEBVIEW_CONTAINER::OnNavigationRequest, this,
          _browser->GetId() );
    Bind( wxEVT_WEBVIEW_NAVIGATED, &WEBVIEW_CONTAINER::OnNavigationComplete, this,
          _browser->GetId() );
    Bind( wxEVT_WEBVIEW_LOADED, &WEBVIEW_CONTAINER::OnDocumentLoaded, this, _browser->GetId() );
    Bind( wxEVT_WEBVIEW_ERROR, &WEBVIEW_CONTAINER::OnError, this, _browser->GetId() );
    Bind( wxEVT_WEBVIEW_NEWWINDOW, &WEBVIEW_CONTAINER::OnNewWindow, this, _browser->GetId() );
    Bind( wxEVT_WEBVIEW_TITLE_CHANGED, &WEBVIEW_CONTAINER::OnTitleChanged, this,
          _browser->GetId() );
    Bind( wxEVT_WEBVIEW_FULLSCREEN_CHANGED, &WEBVIEW_CONTAINER::OnFullScreenChanged, this,
          _browser->GetId() );
    Bind( wxEVT_WEBVIEW_SCRIPT_MESSAGE_RECEIVED, &WEBVIEW_CONTAINER::OnScriptMessage, this,
          _browser->GetId() );
    Bind( wxEVT_WEBVIEW_SCRIPT_RESULT, &WEBVIEW_CONTAINER::OnScriptResult, this,
          _browser->GetId() );

#ifdef DEBUG

#if wxUSE_WEBVIEW_EDGE

    if( auto edge = dynamic_cast<wxWebViewEdge*>( _browser ) )
    {
        edge->EnableAccessToDevTools( true );
    }

#endif


#endif // DEBUG
}

WEBVIEW_CONTAINER::~WEBVIEW_CONTAINER()
{
}

void WEBVIEW_CONTAINER::fire_host_active_cmd( const char* cmd )
{
    wxString out;
    _browser->RunScriptAsync(
            std::format( " {}({});",
                         magic_enum::enum_name( WEBVIEW_FUNCTIONS::fire_host_active_cmd ), cmd ),
            &out );
}

void WEBVIEW_CONTAINER::OnNavigationRequest( wxWebViewEvent& evt )
{
}


void WEBVIEW_CONTAINER::OnNavigationComplete( wxWebViewEvent& evt )
{
}

void WEBVIEW_CONTAINER::OnDocumentLoaded( wxWebViewEvent& evt )
{
    if( evt.GetURL() == _browser->GetCurrentURL() )
    {
    }
}

void WEBVIEW_CONTAINER::OnNewWindow( wxWebViewEvent& evt )
{
    wxString flag = " (other)";

    if( evt.GetNavigationAction() == wxWEBVIEW_NAV_ACTION_USER )
    {
        flag = " (user)";
    }


    //If we handle new window events then just load them in this window as we
    //are a single window browser
    _browser->LoadURL( evt.GetURL() );
}

void WEBVIEW_CONTAINER::OnTitleChanged( wxWebViewEvent& evt )
{
}

void WEBVIEW_CONTAINER::OnFullScreenChanged( wxWebViewEvent& evt )
{
    // TODO
    // ShowFullScreen( evt.GetInt() != 0 );
}

void WEBVIEW_CONTAINER::OnScriptMessage( wxWebViewEvent& evt )
{
    try
    {
        const auto act_container = nlohmann::json::parse( evt.GetString().ToStdString() )
                                           .get<PASSIVE_ACTION_CONTAINER>();
        switch( act_container.category )
        {
        case INVALID: throw std::runtime_error( "Invalid message received" );
        case PA_WEB_HOST:
        {
            const auto cmd = act_container.action.get<WEB_HOST_INTERNAL_CMD>();
            auto       t = magic_enum::enum_cast<WEB_HOST_INTERNAL_CMD_TYPE>( cmd.type );

            if( !t.has_value() )
            {
                wxLogError( "Invalid message received: %s", evt.GetString() );
                return;
            }

            switch( *t )
            {
            case WEB_HOST_INTERNAL_CMD_TYPE::fetch_global_context_from_host:
            {
                if( _host_copilot_handles.global_context_handle.expired() )
                {
                    wxLogError( "Get design global context handle expired" );
                    break;
                }

                auto context_function = _host_copilot_handles.global_context_handle.lock();

                auto& global_ctx = ( *context_function )();

                if( _consumed_global_ctx_keys.contains( global_ctx.uuid ) )
                    break;

                wxString out;
                _browser->RunScriptAsync(
                        std::format( " {}({});",
                                     magic_enum::enum_name(
                                             WEBVIEW_FUNCTIONS::update_copilot_global_context ),
                                     global_ctx.dump() ),
                        &out );

                _consumed_global_ctx_keys.insert( global_ctx.uuid );
                break;
            }
            }
        }
        break;
        case PA_AGENT:
        {
            if( !_host_copilot_handles.agent_action_handle.expired() )
            {
                auto context_function = _host_copilot_handles.agent_action_handle.lock();
                ( *context_function )( act_container.action );
            }
        }
        break;
        }
    }
    catch( const std::exception& e )
    {
        wxLogError( "Invalid message received: %s , %s", evt.GetString(), e.what() );
    }
}

void WEBVIEW_CONTAINER::OnScriptResult( wxWebViewEvent& evt )
{
    if( evt.IsError() )
        wxLogError( "Async script execution failed: %s", evt.GetString() );
}

void WEBVIEW_CONTAINER::OnError( wxWebViewEvent& evt )
{
#define WX_ERROR_CASE( type )                                                                      \
    case type: category = #type; break;

    wxString category;
    switch( evt.GetInt() )
    {
        WX_ERROR_CASE( wxWEBVIEW_NAV_ERR_CONNECTION );
        WX_ERROR_CASE( wxWEBVIEW_NAV_ERR_CERTIFICATE );
        WX_ERROR_CASE( wxWEBVIEW_NAV_ERR_AUTH );
        WX_ERROR_CASE( wxWEBVIEW_NAV_ERR_SECURITY );
        WX_ERROR_CASE( wxWEBVIEW_NAV_ERR_NOT_FOUND );
        WX_ERROR_CASE( wxWEBVIEW_NAV_ERR_REQUEST );
        WX_ERROR_CASE( wxWEBVIEW_NAV_ERR_USER_CANCELLED );
        WX_ERROR_CASE( wxWEBVIEW_NAV_ERR_OTHER );
    }

    wxLogMessage( "%s", "Error; url='" + evt.GetURL() + "', error='" + category + " ("
                                + evt.GetString() + ")'" );
}
