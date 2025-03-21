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

#include <bitmaps/bitmaps_list.h>
#include <common.h>
#include <eda_units.h>
#include <frame_type.h>
#include <tool/actions.h>
#include <tool/tool_action.h>
#include <tool/tool_event.h>

#ifndef COPILOT_COMMON_ACTION_H
#define COPILOT_COMMON_ACTION_H

#undef _
#define _( s ) s

TOOL_ACTION ACTIONS::toggleCopilotPanel(
        TOOL_ACTION_ARGS()
                .Name( "eeschema.SchDesignBlockControl.toggleCopilotPanel" )
                .Scope( AS_GLOBAL )
                .FriendlyName( _( "Copilot" ) )
                .Tooltip( _( "Show/hide copilot panel" ) )
                .Icon( BITMAPS::copilot ) );

TOOL_ACTION ACTIONS::showCopilotPanel(
        TOOL_ACTION_ARGS()
                .Name( "eeschema.SchDesignBlockControl.showCopilotPanel" )
                .Scope( AS_GLOBAL )
                .FriendlyName( _( "Show copilot" ) )
                .Tooltip( _( "Show copilot panel" ) )
                .Icon( BITMAPS::copilot ) );

#endif
