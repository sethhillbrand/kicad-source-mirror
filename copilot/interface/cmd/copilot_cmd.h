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

#ifndef COPILOT_CMD_H
#define COPILOT_CMD_H

#include "cmd_base.h"
#include <context/copilot_context.h>

struct DESIGN_INTENTION : CONCRETE_TYPE_CMD<COPILOT_CMD_TYPE::DESIGN_INTENTION>
{
};

struct CORE_COMPONENTS : CONCRETE_TYPE_CMD<COPILOT_CMD_TYPE::CORE_COMPONENTS>
{
};

struct CURRENT_COMPONENT : CMD_WITH_CONTEXT<COPILOT_CMD_TYPE::CURRENT_COMPONENT, SYMBOL_CMD_CONTEXT>
{
};

struct SIMILAR_COMPONENTS
        : CMD_WITH_CONTEXT<COPILOT_CMD_TYPE::SIMILAR_COMPONENTS, SYMBOL_CMD_CONTEXT>
{
};

struct CHECK_SYMBOL_CONNECTIONS
        : CMD_WITH_CONTEXT<COPILOT_CMD_TYPE::CHECK_SYMBOL_CONNECTIONS, SYMBOL_CMD_CONTEXT>
{
};

struct COMPONENT_PINS_DETAILS
        : CMD_WITH_CONTEXT<COPILOT_CMD_TYPE::COMPONENT_PINS_DETAILS, SYMBOL_CMD_CONTEXT>
{
};

struct SYMBOL_UNCONNECTED_PINS
        : CMD_WITH_CONTEXT<COPILOT_CMD_TYPE::SYMBOL_UNCONNECTED_PINS, SYMBOL_CMD_CONTEXT>
{
};


#endif // COPILOT_CMD_H
