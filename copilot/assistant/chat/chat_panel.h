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

#ifndef CHAT_PANEL_H
#define CHAT_PANEL_H


#include "chat_panel_base.h"
#include <memory>
#include <wx/log.h>
#include "assistant/client/chat_cmd_queue.h"

class WEBSOCKET_WORKER;
class CHAT_PANEL : public CHAT_PANEL_BASE, wxLog
{
public:
    CHAT_PANEL( wxWindow* parent );
    ~CHAT_PANEL();

protected:
    void DoLogRecord( wxLogLevel level, const wxString& msg, const wxLogRecordInfo& info ) override;

    // logging helper
    void DoLogLine( wxRichTextCtrl* text, const wxString& msg );


private:
    void m_chat_ctrlOnTextMaxLen( wxCommandEvent& event ) override;
    void m_chat_ctrlOnTextURL( wxTextUrlEvent& event ) override;
    void m_usr_inputOnTextMaxLen( wxCommandEvent& event ) override;
    void m_btn_sendOnButtonClick( wxCommandEvent& event ) override;


private:
    CHAT_CMDS                         _cmds;
    std::unique_ptr<WEBSOCKET_WORKER> _client_worker;
};

#endif
