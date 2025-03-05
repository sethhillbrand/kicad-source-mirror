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

#ifndef SCH_COPILOT_SYMBOL_CTX_MENU_H
#define SCH_COPILOT_SYMBOL_CTX_MENU_H


#include "bitmaps/bitmaps_list.h"
#include <ee_actions.h>
#include <functional>
#include <tool/action_menu.h>
#include <vector>


class SCH_COPILOT_SYMBOL_CTX_MENU : public ACTION_MENU
{
public:
    SCH_COPILOT_SYMBOL_CTX_MENU( TOOL_INTERACTIVE* aTool ) : ACTION_MENU( true, aTool )
    {
        SetIcon( BITMAPS::copilot );
        SetTitle( _( "Copilot" ) );
        for( const auto act : std::vector<std::reference_wrapper<TOOL_ACTION>>{
                     EE_ACTIONS::copilotCurrentSymbol, EE_ACTIONS::copilotSimilarComponents,
                     EE_ACTIONS::copilotCheckSymbolConnections,
                     EE_ACTIONS::copilotComponentPinsDetails,
                     EE_ACTIONS::copilotSymbolUnconnectedPins } )

            Add( act );
    }

protected:
    ACTION_MENU* create() const override { return new SCH_COPILOT_SYMBOL_CTX_MENU( m_tool ); }
};


#endif
