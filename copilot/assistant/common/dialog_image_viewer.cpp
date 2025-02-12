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

#include "dialog_image_viewer.h"
#include "settings/copilot_settings_manager.h"
#include "settings/copilot_settings.h"
#include <wx/event.h>
#include <wx/log.h>
#include <wx/sizer.h>
#include <fmt/format.h>

#if wxUSE_WEBVIEW_EDGE
#include <wx/msw/webview_edge.h>
#endif


DIALOG_IMAGE_VIEWER::DIALOG_IMAGE_VIEWER( wxWindow* aParent, wxWindowID id, const wxString& title,
                                          const wxPoint& pos, const wxSize& size, long style,
                                          const wxString& name ) :
        wxDialog( aParent, id, title, pos, size, style, name ), _browser( wxWebView::New() )
{
    auto       top_sizer = new wxBoxSizer( wxVERTICAL );
    const auto url = COPILOT_SETTINGS_MANAGER::get_instance().get_webview_image_viewer_path();
    _browser->Create( this, wxID_ANY, url, wxDefaultPosition, wxDefaultSize );
    top_sizer->Add( _browser, wxSizerFlags().Expand().Proportion( 1 ) );
    SetSizer( top_sizer );

    Bind( wxEVT_CLOSE_WINDOW,
          [this]( wxCloseEvent& event )
          {
              const auto pos = GetPosition();
              const auto size = GetSize();
              COPILOT_SETTINGS_MANAGER::get_instance().set_image_viewer_geometry(
                      DIALOG_GEOMETRY{ pos.x, pos.y, size.x, size.y

                      } );
              event.Skip();
          } );


#ifdef DEBUG

#if wxUSE_WEBVIEW_EDGE

    if( auto edge = dynamic_cast<wxWebViewEdge*>( _browser ) )
    {
        edge->EnableAccessToDevTools( true );
    }

#endif


#endif // DEBUG
}

void DIALOG_IMAGE_VIEWER::reload()
{
    _browser->RunScriptAsync( "window.dispatchEvent(new Event(\"updateLocalImage\"));" );
}

DIALOG_IMAGE_VIEWER::~DIALOG_IMAGE_VIEWER()
{
}
