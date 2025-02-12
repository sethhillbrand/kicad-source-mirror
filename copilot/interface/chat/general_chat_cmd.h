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

#ifndef GENERAL_CHAT_CMD_H
#define GENERAL_CHAT_CMD_H

#include <nlohmann/json.hpp>
#include <string>
#include <vector>


struct ChatCompletionSystemMessageParam
{
    std::string content;
    std::string role = "system";
    
    NLOHMANN_DEFINE_TYPE_INTRUSIVE( ChatCompletionSystemMessageParam, content, role )
};


struct USR_INPUT
{
    std::string              input_text;
    std::vector<std::string> base64_images;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE( USR_INPUT, input_text, base64_images )
};


struct GENERIC_CHAT_CONTEXT
{
    USR_INPUT chat;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE( GENERIC_CHAT_CONTEXT, chat )
};


struct GENERAL_CHAT_CMD
{
    GENERIC_CHAT_CONTEXT                          context;
    std::vector<ChatCompletionSystemMessageParam> messages;
    std::string                                   type = "chat.user.generic_chat";

    NLOHMANN_DEFINE_TYPE_INTRUSIVE( GENERAL_CHAT_CMD, context, messages, type )
};

#endif
