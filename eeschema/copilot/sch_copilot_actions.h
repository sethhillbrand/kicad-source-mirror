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

#ifndef SCH_COPILOT_ACTIONS_H
#define SCH_COPILOT_ACTIONS_H

#include <sch_actions.h>

#include <bitmaps.h>
#include <sch_bitmap.h>
#include <tool/tool_action.h>

#undef _
#define _( s ) s


TOOL_ACTION SCH_ACTIONS::copilotDesignIntention(
        TOOL_ACTION_ARGS()
                .Name( "eeschema.copilot.copilotDesignIntention" )
                .Scope( AS_GLOBAL )
                .FriendlyName( _( "Explain design intention" ) ) );


TOOL_ACTION SCH_ACTIONS::copilotCoreComponents(
        TOOL_ACTION_ARGS()
                .Name( "eeschema.copilot.copilotCoreComponents" )
                .Scope( AS_GLOBAL )
                .FriendlyName( _( "Explain core components" ) ) );


TOOL_ACTION SCH_ACTIONS::copilotCurrentSymbol(
        TOOL_ACTION_ARGS()
                .Name( "eeschema.copilot.copilotCurrentSymbol" )
                .Scope( AS_GLOBAL )
                .FriendlyName( _( "Explain current symbol" ) ) );


TOOL_ACTION SCH_ACTIONS::copilotSimilarComponents(
        TOOL_ACTION_ARGS()
                .Name( "eeschema.copilot.copilotSimilarComponents" )
                .Scope( AS_GLOBAL )
                .FriendlyName( _( "Find similar components" ) ) );


TOOL_ACTION SCH_ACTIONS::copilotCheckSymbolConnections(
        TOOL_ACTION_ARGS()
                .Name( "eeschema.copilot.copilotCheckSymbolConnections" )
                .Scope( AS_GLOBAL )
                .FriendlyName( _( "Check symbol connections" ) ) );

TOOL_ACTION SCH_ACTIONS::copilotComponentPinsDetails(
        TOOL_ACTION_ARGS()
                .Name( "eeschema.copilot.copilotComponentPinsDetails" )
                .Scope( AS_GLOBAL )
                .FriendlyName( _( "Explain pins details" ) ) );

TOOL_ACTION SCH_ACTIONS::copilotSymbolUnconnectedPins(
        TOOL_ACTION_ARGS()
                .Name( "eeschema.copilot.copilotSymbolUnconnectedPins" )
                .Scope( AS_GLOBAL )
                .FriendlyName( _( "Check unconnected pins" ) ) );

#endif
