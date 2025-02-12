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

#ifndef PCB_COPILOT_CONTEXT_INTERFACE_H
#define PCB_COPILOT_CONTEXT_INTERFACE_H

#include "tool/tool_manager.h"
#include "tools/pcb_selection_tool.h"
#include "pcb_items_jsonify_helper.h"
#include <webview/webview_container.h>
#include <board_design_settings.h>
#include <string_utils.h>
#include <algorithm>
#include <pcb_edit_frame.h>
#include <context/pcb/pcb_copilot_global_context.h>
#include <context/pcb/details/board_ir.h>
#include <string>
#include <board.h>
#include <footprint.h>

void PCB_EDIT_FRAME::UpdateCopilotContextCache()
{
    if( m_copilotContextCache->is_newest )
        return;

    m_copilotContextCache->uuid = KIID().AsStdString();
    m_copilotContextCache->is_newest = true;
    m_copilotContextCache->designators = ([this](){

        std::vector<std::string> designators;

        for(const auto& fp : GetBoard()->Footprints())        
            designators.push_back(UnescapeString(fp->Reference().GetText()).ToStdString());        
        std::sort(designators.begin(), designators.end(),[&](const std::string& a, const std::string& b){
            return StrNumCmp(a, b) < 0;
        });

        designators.erase( std::unique( designators.begin(), designators.end() ),
                           designators.end() );

        return designators;

    })();

    m_copilotContextCache->fab_settings.distance_settings = ([this](){

        return  PCB_FAB_DISTANCE_SETTINGS{
            GetBoard()->GetDesignSettings().m_TrackMinWidth,
            GetBoard()->GetDesignSettings().m_MinClearance,
             std::min(GetBoard()->GetDesignSettings().m_MinThroughDrill, GetBoard()->GetDesignSettings().m_ViasMinSize),
        };
    })();
}


copilot::BOARD_SELECTIONS PCB_EDIT_FRAME::GetSelection() const
{
    return jsonify_pcb_items( GetToolManager()->GetTool<PCB_SELECTION_TOOL>()->GetSelection() );
}

static void RemoveEmptyObjectsOrArrays( nlohmann::json& j )
{
    if( j.is_object() )
    {
        for( auto it = j.begin(); it != j.end(); )
        {
            RemoveEmptyObjectsOrArrays( it.value() ); // Recurse first

            if( ( it.value().is_object() || it.value().is_array() ) && it.value().empty() )
                it = j.erase( it );
            else
                ++it;
        }
    }
    else if( j.is_array() )
    {
        for( auto it = j.begin(); it != j.end(); )
        {
            RemoveEmptyObjectsOrArrays( *it ); // Recurse into array elements

            if( ( it->is_object() || it->is_array() ) && it->empty() )
                it = j.erase( it );
            else
                ++it;
        }
    }
}

std::string PCB_EDIT_FRAME::GetSelectionJsonString() const
{
    nlohmann::json selection = GetSelection();
    RemoveEmptyObjectsOrArrays( selection );
    return selection.dump();
}


void PCB_EDIT_FRAME::SetHasSelection( bool aHasSelection )
{
    if( m_copilotPanel )
        m_copilotPanel->on_host_selection_changed( aHasSelection );
}

#endif
