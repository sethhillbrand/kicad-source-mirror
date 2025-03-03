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
#include <copilot_global_ctx_hdl.h>
#include "assistant/client/websocket_event.h"
#include "copilot_context.h"
#include "copilot_global.h"
#include <exception>
#include <nlohmann/json.hpp>
#include <assistant/client/websocket_worker.h>
#include <interface/cmd/copilot_cmd.h>
#include <string>
#include <wx/log.h>
#include <wx/msgqueue.h>
#include <wx/string.h>

extern "C"

{
    COPILOT_API COPILOT_GLOBAL_CONTEXT_HDL get_global_context_hdl = nullptr;
}


struct CMD_TYPE_TRAITS
{
    COPILOT_CMD_TYPE type{ COPILOT_CMD_TYPE::GENERIC_CHAT };
    NLOHMANN_DEFINE_TYPE_INTRUSIVE( CMD_TYPE_TRAITS, type )
};

inline void CHAT_PANEL::append_msg(wxString const& msg)
{
    m_chat_ctrl->AppendText( msg);
}

CHAT_PANEL::CHAT_PANEL( wxWindow* parent ) :
        _previous_msg_type(), CHAT_PANEL_BASE( parent ),
        _client_worker( new WEBSOCKET_WORKER( this, _cmds ) )
{
    _client_worker->Run();

    Bind( EVT_WEBSOCKET_PAYLOAD, &CHAT_PANEL::on_websocket_event, this );
    m_usr_input->Bind( wxEVT_TEXT_ENTER, &CHAT_PANEL::on_send_button_clicked, this );
    m_usr_input->SetFocus();
}

CHAT_PANEL::~CHAT_PANEL()
{
    _client_worker->quit();
    _client_worker->Wait();
}

void CHAT_PANEL::fire_cmd( const char* cmd )
{
    if( _previous_msg_type == MEG_TYPE::CONTENT )
    {
        append_msg( "小助手正忙,请稍后再试" );
        return;
    }

    wxString cmd_desc;

    try
    {
        CMD_TYPE_TRAITS t;
        auto            cmd_type = nlohmann::json::parse( cmd ).get_to( t );

        switch( t.type )
        {
        case COPILOT_CMD_TYPE::GENERIC_CHAT: break;
        case COPILOT_CMD_TYPE::DESIGN_INTENTION: cmd_desc = "解释设计意图"; break;
        case COPILOT_CMD_TYPE::CORE_COMPONENTS: cmd_desc = "核心器件"; break;
        case COPILOT_CMD_TYPE::CURRENT_COMPONENT: cmd_desc = "解释当前器件"; break;
        case COPILOT_CMD_TYPE::SIMILAR_COMPONENTS: cmd_desc = "相似器件推荐"; break;
        case COPILOT_CMD_TYPE::CHECK_SYMBOL_CONNECTIONS: cmd_desc = "检查符号连接关系"; break;
        case COPILOT_CMD_TYPE::COMPONENT_PINS_DETAILS: cmd_desc = "介绍器件引脚详情"; break;
        case COPILOT_CMD_TYPE::SYMBOL_UNCONNECTED_PINS: cmd_desc = "检查符号未连接引脚"; break;
        }
    }
    catch( ... )
    {
    }

    if( !cmd_desc.empty() )
        append_msg( "Q:" + cmd_desc );

    _cmds.Post( std::string( cmd ) );

    _previous_msg_type = MEG_TYPE::END_OF_CHAT;
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
        append_msg( "\n" );

    append_msg( "Q:" + usr_input );
    GENERIC_CHAT chat{ {}, { {}, usr_input.ToUTF8().data() } };

    if( get_global_context_hdl )
    {
        try
        {
            DESIGN_GLOBAL_CONTEXT ctx;
            auto ctx_str = get_global_context_hdl();
            nlohmann::json::parse( ctx_str ).get_to( ctx );
            chat.context.bom = ctx.bom;
            chat.context.net_list = ctx.net_list;
        }
        catch( std::exception const& e )
        {
            wxLogError(e.what());
        }
    }

   
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
        append_msg( payload.msg );
        break;
    }
    case MEG_TYPE::END_OF_CHAT:
    {
        if( _previous_msg_type == MEG_TYPE::CONTENT )
        {
            append_msg( "\n" );
        }

        m_btn_send->Enable( true );
        break;
    }
    }

    _previous_msg_type = payload.type;
}
