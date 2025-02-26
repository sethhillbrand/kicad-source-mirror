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

#include "chat_panel.h"
#include "nlohmann/json.hpp"
#include <assistant/client/websocket_worker.h>
#include <interface/cmd/copilot_cmd.h>
#include <wx/msgqueue.h>
#include <wx/string.h>


CHAT_PANEL::CHAT_PANEL( wxWindow* parent ) :
        CHAT_PANEL_BASE( parent ), _client_worker( new WEBSOCKET_WORKER( _cmds ) ),
        _old( wxLog::GetActiveTarget() )
{
    _client_worker->Run();

    wxLog::SetActiveTarget( this );
    m_usr_input->Bind( wxEVT_TEXT_ENTER,
                       [this]( wxCommandEvent& event )
                       {
                           on_send_button_clicked();
                       } );
}

CHAT_PANEL::~CHAT_PANEL()
{
    wxLog::SetActiveTarget( _old );
    _client_worker->quit();
}

void CHAT_PANEL::DoLogRecord( wxLogLevel level, const wxString& msg, const wxLogRecordInfo& info )
{
    DoLogLine( m_chat_ctrl, msg );
}

void CHAT_PANEL::DoLogLine( wxRichTextCtrl* text, const wxString& msg )
{
    text->AppendText( msg );
}

void CHAT_PANEL::m_chat_ctrlOnTextMaxLen( wxCommandEvent& event )
{
    event.Skip();
}

void CHAT_PANEL::m_chat_ctrlOnTextURL( wxTextUrlEvent& event )
{
    event.Skip();
}

void CHAT_PANEL::m_btn_sendOnButtonClick( wxCommandEvent& event )
{
    WXUNUSED( event );
}


void CHAT_PANEL::on_send_button_clicked()
{
    const auto usr_input = m_usr_input->GetValue();
    m_chat_ctrl->AppendText( "\nQ:" + usr_input );
    GENERIC_CHAT chat{ {}, { {}, usr_input.ToUTF8().data() }

    };
    _cmds.Post( nlohmann::json( chat ).dump() );
    m_usr_input->Clear();
}
