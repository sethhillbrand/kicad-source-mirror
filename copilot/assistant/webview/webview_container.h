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

#ifndef WEBVIEW_CONTAINER_H
#define WEBVIEW_CONTAINER_H


#include "assistant/assistant.h"
#include <wx/panel.h>
#include <wx/log.h>
#include <wx/webview.h>


class WEBVIEW_CONTAINER : public wxPanel, public ASSISTANT
{
public:
    WEBVIEW_CONTAINER( wxWindow* parent );
    ~WEBVIEW_CONTAINER();

    void fire_cmd( const char* cmd ) override;

    void OnNavigationRequest(wxWebViewEvent& evt);
    void OnNavigationComplete(wxWebViewEvent& evt);
    void OnDocumentLoaded(wxWebViewEvent& evt);
    void OnNewWindow(wxWebViewEvent& evt);
    void OnTitleChanged(wxWebViewEvent& evt);
    void OnFullScreenChanged(wxWebViewEvent& evt);
    void OnScriptMessage(wxWebViewEvent& evt);
    void OnScriptResult(wxWebViewEvent& evt);
    void OnError(wxWebViewEvent& evt);




private:
    wxWebView* m_browser;
};

#endif
