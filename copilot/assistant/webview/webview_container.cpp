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

#include <wx/log.h>
#include <wx/sizer.h>
#include <wx/filename.h>
#include <wx/stdpaths.h>
#include <wx/webview.h>
#include <wx/webviewarchivehandler.h>
#include <wx/webviewfshandler.h>
#include <context/copilot_global_ctx_hdl.h>
#include <context/copilot_context.h>
#include <copilot_global.h>
#include <format>
#include <magic_enum.hpp>

#if wxUSE_WEBVIEW_EDGE
#include <wx/msw/webview_edge.h>
#endif

extern "C"

{
    COPILOT_API COPILOT_GLOBAL_CONTEXT_HDL get_global_context_hdl = nullptr;
}


enum START_UP_SIZE
{
    WIDTH = 400,
    HEIGHT = 1040
};

WEBVIEW_CONTAINER::WEBVIEW_CONTAINER( wxWindow* parent ) :
        wxPanel( parent ), m_browser( wxWebView::New() )
{
#ifdef DEBUG
    new wxLogWindow( this, _( "Logging" ), true, false );
#endif // DEBUG

    auto top_sizer = new wxBoxSizer( wxVERTICAL );
#ifdef __WXMAC__
    // With WKWebView handlers need to be registered before creation
    m_browser->RegisterHandler(
            wxSharedPtr<wxWebViewHandler>( new wxWebViewArchiveHandler( "wxfs" ) ) );
    m_browser->RegisterHandler(
            wxSharedPtr<wxWebViewHandler>( new wxWebViewFSHandler( "memory" ) ) );
#endif
    m_browser->Create( this, wxID_ANY,
                       COPILOT_SETTINGS_MANAGER::get_instance().get_webview_chat_path(),
                       wxDefaultPosition, wxDefaultSize );
    top_sizer->Add( m_browser, wxSizerFlags().Expand().Proportion( 1 ) );

    // Log backend information
    wxLogMessage( "Backend: %s Version: %s", m_browser->GetClassInfo()->GetClassName(),
                  wxWebView::GetBackendVersionInfo().ToString() );
    wxLogMessage( "User Agent: %s", m_browser->GetUserAgent() );

#ifndef __WXMAC__
    //We register the wxfs:// protocol for testing purposes
    m_browser->RegisterHandler(
            wxSharedPtr<wxWebViewHandler>( new wxWebViewArchiveHandler( "wxfs" ) ) );
    //And the memory: file system
    m_browser->RegisterHandler(
            wxSharedPtr<wxWebViewHandler>( new wxWebViewFSHandler( "memory" ) ) );
#endif
    if( !m_browser->AddScriptMessageHandler(
                magic_enum::enum_name( WEBVIEW_MSG_HANDLES::kicad_desktop ).data() ) )
        wxLogError( "Could not add script message handler" );


    SetSizer( top_sizer );
    //Set a more sensible size for web browsing
    SetSize( FromDIP( wxSize( START_UP_SIZE::WIDTH, START_UP_SIZE::HEIGHT ) ) );

    // Connect the webview events
    Bind( wxEVT_WEBVIEW_NAVIGATING, &WEBVIEW_CONTAINER::OnNavigationRequest, this,
          m_browser->GetId() );
    Bind( wxEVT_WEBVIEW_NAVIGATED, &WEBVIEW_CONTAINER::OnNavigationComplete, this,
          m_browser->GetId() );
    Bind( wxEVT_WEBVIEW_LOADED, &WEBVIEW_CONTAINER::OnDocumentLoaded, this, m_browser->GetId() );
    Bind( wxEVT_WEBVIEW_ERROR, &WEBVIEW_CONTAINER::OnError, this, m_browser->GetId() );
    Bind( wxEVT_WEBVIEW_NEWWINDOW, &WEBVIEW_CONTAINER::OnNewWindow, this, m_browser->GetId() );
    Bind( wxEVT_WEBVIEW_TITLE_CHANGED, &WEBVIEW_CONTAINER::OnTitleChanged, this,
          m_browser->GetId() );
    Bind( wxEVT_WEBVIEW_FULLSCREEN_CHANGED, &WEBVIEW_CONTAINER::OnFullScreenChanged, this,
          m_browser->GetId() );
    Bind( wxEVT_WEBVIEW_SCRIPT_MESSAGE_RECEIVED, &WEBVIEW_CONTAINER::OnScriptMessage, this,
          m_browser->GetId() );
    Bind( wxEVT_WEBVIEW_SCRIPT_RESULT, &WEBVIEW_CONTAINER::OnScriptResult, this,
          m_browser->GetId() );

#ifdef DEBUG

#if wxUSE_WEBVIEW_EDGE

    if( auto edge = dynamic_cast<wxWebViewEdge*>( m_browser ) )
    {
        edge->EnableAccessToDevTools( true );
    }

#endif


#endif // DEBUG
}

WEBVIEW_CONTAINER::~WEBVIEW_CONTAINER()
{
}

void WEBVIEW_CONTAINER::fire_cmd( const char* cmd )
{
    wxString out;
    m_browser->RunScriptAsync(
            std::format( " {}({});", magic_enum::enum_name( WEBVIEW_FUNCTIONS::fire_cmd ), cmd ),
            &out );
    wxLogDebug( "WEBVIEW_CONTAINER::fire_cmd result : ", out );
}

void WEBVIEW_CONTAINER::OnNavigationRequest( wxWebViewEvent& evt )
{
    wxLogMessage( "%s", "Navigation request to '" + evt.GetURL() + "' (target='" + evt.GetTarget()
                                + "')" );
}


void WEBVIEW_CONTAINER::OnNavigationComplete( wxWebViewEvent& evt )
{
    wxLogMessage( "%s", "Navigation complete; url='" + evt.GetURL() + "'" );
}

void WEBVIEW_CONTAINER::OnDocumentLoaded( wxWebViewEvent& evt )
{ //Only notify if the document is the main frame, not a subframe
    if( evt.GetURL() == m_browser->GetCurrentURL() )
    {
        wxLogMessage( "%s", "Document loaded; url='" + evt.GetURL() + "'" );
    }
}

void WEBVIEW_CONTAINER::OnNewWindow( wxWebViewEvent& evt )
{
    wxString flag = " (other)";

    if( evt.GetNavigationAction() == wxWEBVIEW_NAV_ACTION_USER )
    {
        flag = " (user)";
    }

    wxLogMessage( "%s", "New window; url='" + evt.GetURL() + "'" + flag );

    //If we handle new window events then just load them in this window as we
    //are a single window browser
    m_browser->LoadURL( evt.GetURL() );
}

void WEBVIEW_CONTAINER::OnTitleChanged( wxWebViewEvent& evt )
{
    wxLogMessage( "%s", "Title changed; title='" + evt.GetString() + "'" );
}

void WEBVIEW_CONTAINER::OnFullScreenChanged( wxWebViewEvent& evt )
{
    wxLogMessage( "Full screen changed; status = %d", evt.GetInt() );
    // TODO
    // ShowFullScreen( evt.GetInt() != 0 );
}

void WEBVIEW_CONTAINER::OnScriptMessage( wxWebViewEvent& evt )
{
    wxLogMessage( "Script message received; value = %s, handler = %s", evt.GetString(),
                  evt.GetMessageHandler() );
}

void WEBVIEW_CONTAINER::OnScriptResult( wxWebViewEvent& evt )
{
    if( evt.IsError() )
        wxLogError( "Async script execution failed: %s", evt.GetString() );
    else
        wxLogMessage( "Async script result received; value = %s", evt.GetString() );
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
