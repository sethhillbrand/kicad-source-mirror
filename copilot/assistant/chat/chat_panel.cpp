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
#include "assistant/client/websocket_event.h"
#include <nlohmann/json.hpp>
#include <assistant/client/websocket_worker.h>
#include <interface/cmd/copilot_cmd.h>
#include <wx/msgqueue.h>
#include <wx/string.h>


CHAT_PANEL::CHAT_PANEL( wxWindow* parent ) :
        _previous_msg_type(), CHAT_PANEL_BASE( parent ),
        _client_worker( new WEBSOCKET_WORKER( this, _cmds ) )
{
    _client_worker->Run();

    Bind( EVT_WEBSOCKET_PAYLOAD, &CHAT_PANEL::on_websocket_event, this );
    m_usr_input->Bind( wxEVT_TEXT_ENTER, &CHAT_PANEL::on_send_button_clicked, this );
}

CHAT_PANEL::~CHAT_PANEL()
{
    _client_worker->quit();
    _client_worker->Wait();
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
    on_send_button_clicked( event );
}


void CHAT_PANEL::on_send_button_clicked( wxCommandEvent& event )
{
    WXUNUSED( event );

    if( _previous_msg_type == MEG_TYPE::CONTENT )
        return;

    const auto usr_input = m_usr_input->GetValue();

    if( !m_chat_ctrl->GetValue().empty() )
        m_chat_ctrl->AppendText( "\n" );

    m_chat_ctrl->AppendText( "Q:" + usr_input );
    GENERIC_CHAT chat{ {}, { {}, usr_input.ToUTF8().data() } };
    _cmds.Post( nlohmann::json( chat ).dump() );
    m_usr_input->Clear();
    m_btn_send->Enable( false );
    _previous_msg_type = MEG_TYPE::END_OF_CHAT;
}

void CHAT_PANEL::on_websocket_event( const WEBSOCKET_EVENT& event )
{
    auto payload = event.GetCommandResult();

    switch( payload.type )
    {
    case MEG_TYPE::CONTENT:
    {
        m_chat_ctrl->AppendText( payload.msg );
        break;
    }
    case MEG_TYPE::END_OF_CHAT:
    {
        if( _previous_msg_type == MEG_TYPE::CONTENT )
        {
            m_chat_ctrl->AppendText( "\n");
        }

        m_btn_send->Enable( true );
        break;
    }
    }

    _previous_msg_type = payload.type;
}
