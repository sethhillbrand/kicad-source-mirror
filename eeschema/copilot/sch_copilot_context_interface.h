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

#ifndef SCH_COPILOT_CONTEXT_INTERFACE_H
#define SCH_COPILOT_CONTEXT_INTERFACE_H

#include <connection_graph.h>
#include "sch_symbol.h"
#include "webview/webview_container.h"
#include <cstddef>
#include <memory>
#include <nlohmann/json.hpp>
#include <netlist_exporter_xml.h>
#include <kiid.h>
#include <sch_selection_tool.h>
#include <string>
#include <tool/tool_manager.h>
#include <sch_edit_frame.h>
#include <fields_data_model.h>
#include <context/symbol_context.h>
#include <unordered_map>
#include <utils/base64.hpp>
#include <context/sch/sch_copilot_global_context.h>
#include <context/sch/details/sch_netlist.h>
#include <string_utils.h>
#include <algorithm>
#include <vector>
#include <wx/string.h>

void SCH_EDIT_FRAME::UpdateCopilotContextCache()
{
    if( m_copilotContextCache->is_newest )
        return;

    m_copilotContextCache->uuid = KIID().AsStdString();
    m_copilotContextCache->net_list = base64::to_base64(
            ( (
                      [&]() -> wxString
                      {
                          Schematic().Hierarchy().AnnotatePowerSymbols();
                          SCHEMATIC*           sch = &Schematic();
                          WX_STRING_REPORTER   reporter;
                          NETLIST_EXPORTER_XML helper( sch );

                          wxString tmp_file_path =
                                  wxFileName::CreateTempFileName( wxFileName::GetTempDir() + "/" );

                          std::unique_ptr<void, std::function<void( void* )>> on_leave( nullptr,
                                                [tmp_file_path]( void* )
                                                {
                                                    wxRemoveFile(
                                                            tmp_file_path );
                                                } );

                          if( !helper.WriteNetlist( tmp_file_path, 0, reporter ) )
                              return {};
                          {
                              wxFile tmp_file( tmp_file_path, wxFile::read );

                              if( tmp_file.IsOpened() )
                              {
                                  wxString content;
                                  tmp_file.ReadAll( &content );
                                  tmp_file.Close();
                                  return content;
                              }
                          }
                          return {};
                      } )() )
                    .ToStdString() );

    m_copilotContextCache->designators = ([this]{
        decltype( m_copilotContextCache->designators)   designators;
        SCH_REFERENCE_LIST referenceList;
        Schematic().Hierarchy().GetSymbols( referenceList, false, false );
        const auto instances = referenceList.GetSymbolInstances();

        for( const auto& instance : instances )
            designators.push_back( instance.m_Reference.ToStdString() );

        std::sort(designators.begin(), designators.end(),[&](const std::string& a, const std::string& b){
                return StrNumCmp(a, b) < 0;
            });

        designators.erase( std::unique( designators.begin(), designators.end() ),
                           designators.end() );

        return designators;

    })();

    m_copilotContextCache->is_newest = true;
}
wxString SCH_EDIT_FRAME::GetBomList()
{
    return {};
}

wxString SCH_EDIT_FRAME::GetNetList()
{
    UpdateCopilotContextCache();
    return m_copilotContextCache->net_list;
}

SYMBOL_CMD_CONTEXT const& SCH_EDIT_FRAME::GetSelectedSymbolContext()
{
    auto        selTool = GetToolManager()->GetTool<SCH_SELECTION_TOOL>();
    auto&       selection = selTool->GetSelection();
    SCH_SYMBOL* symbol = dynamic_cast<SCH_SYMBOL*>( selection.Front() );

    if( !symbol )
    {
        // NOTE shall not come here
        return *m_symbolCmdContext;
    }

    const wxString ref = symbol->GetRefProp();
    m_symbolCmdContext->designator = ref;
    m_symbolCmdContext->symbol_properties = {
        symbol->GetValueProp().ToStdString(),
        symbol->GetDescription().ToStdString(),
        symbol->GetFootprintFieldText( true, &Schematic().CurrentSheet(), false ).ToStdString(),
    };


    for( const auto& pin : symbol->GetPins() )
    {
        m_symbolCmdContext->symbol_properties.pins.push_back(
                { pin->GetNumber().ToStdString(), pin->GetName().ToStdString(),
                  PinShapeGetText( pin->GetShape() ).ToStdString() } );
    }


    return *m_symbolCmdContext;
}

wxString SCH_EDIT_FRAME::GetSymbolNetList( wxString const& aDesignator )
{
    WXUNUSED( aDesignator );
    return GetNetList();
}

copilot::NETLIST SCH_EDIT_FRAME::GetSelection() const
{
    static const auto get_fields =
            []( SCH_SHEET_PATH* aSheetPath, const std::vector<SCH_FIELD>& fields )
    {
        std::vector<copilot::FIELD> result;

        for( const auto& field : fields )
            result.emplace_back ( copilot::FIELD{ field.GetName().ToStdString(),
                                field.GetShownText( aSheetPath, false ).ToStdString() } );

        return result;
    };


    auto             selTool = GetToolManager()->GetTool<SCH_SELECTION_TOOL>();
    auto&            selections = selTool->GetSelection();
    copilot::NETLIST netlist;
    const auto       sch_path = &GetCurrentSheet();

    std::set<SCH_PIN*> selection_pins;

    for( const auto& selection : selections )
    {
        if( selection->IsType( { SCH_SYMBOL_T } ) )
        {
            auto       symbol = static_cast<SCH_SYMBOL*>( selection );
            const auto ref = symbol->GetRef( sch_path ).ToStdString();
            netlist.components.emplace_back(  copilot::COMPONENT { ref, get_fields( sch_path, symbol->GetFields() ) } );

            for( const auto pin : symbol->GetPins( sch_path ) )
                selection_pins.insert( pin );
        }
    }

    std::unordered_map<std::string, std::shared_ptr<copilot::NET>> uniq_nets;

    for( const auto& [key, subgraphs] : m_schematic->ConnectionGraph()->GetNetMap() )
    {
        if( subgraphs.empty() )
            continue;

        for( CONNECTION_SUBGRAPH* subgraph : subgraphs )
        {
            if( !std::any_of( subgraph->GetItems().begin(), subgraph->GetItems().end(),
                              [&selection_pins]( SCH_ITEM* item )
                              {
                                  return item->Type() == SCH_PIN_T
                                         && selection_pins.contains(
                                                 static_cast<SCH_PIN*>( item ) );
                              } ) )
                continue;

            const SCH_SHEET_PATH& sheet = subgraph->GetSheet();
            const std::string     net_name_code = subgraph->GetNetName().ToStdString();


            if( !uniq_nets.contains( net_name_code ) )
            {
                uniq_nets.try_emplace( net_name_code, std::make_shared<copilot::NET>(
                                                              copilot::NET{ net_name_code } ) );
            }

            auto& nodes = uniq_nets[net_name_code]->nodes;

            for( SCH_ITEM* item : subgraph->GetItems() )
            {
                if( item->Type() != SCH_PIN_T )
                    continue;

                SCH_PIN*    pin = static_cast<SCH_PIN*>( item );
                SCH_SYMBOL* symbol = dynamic_cast<SCH_SYMBOL*>( pin->GetParentSymbol() );

                if( !symbol )
                    continue;

                nodes.emplace_back( copilot::NODE{
                        symbol->GetRef( &sheet ).ToStdString(),
                        pin->GetShownNumber().ToStdString(),
                        pin->GetShownName().ToStdString(),
                        PinShapeGetText( pin->GetShape() ).ToStdString(),
                } );
            }
        }
    }


    for( auto& [k, v] : uniq_nets )
    {
        if( !v->nodes.empty() )
        {
            netlist.nets.push_back( std::move( *v ) );
        }
    }

    return netlist;
}

void SCH_EDIT_FRAME::SetHasSelection( bool aHasSelection )
{
    if( m_copilotPanel )
        m_copilotPanel->on_host_selection_changed( aHasSelection );
}

#endif
