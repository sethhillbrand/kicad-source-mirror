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

#include <cstddef>
#include <memory>
#include <nlohmann/json.hpp>
#include <netlist_exporter_xml.h>
#include <kiid.h>
#include <ee_selection_tool.h>
#include <string>
#include <tool/tool_manager.h>
#include <sch_edit_frame.h>
#include <fields_data_model.h>
#include <context/symbol_context.h>
#include <utils/base64.hpp>
#include <context/sch/sch_copilot_global_context.h>
#include <string_utils.h>
#include <algorithm>

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

                          std::shared_ptr<nullptr_t> on_leave( nullptr,
                                                               [=]( auto it )
                                                               {
                                                                   wxRemoveFile( tmp_file_path );
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

        return designators;

    })();

    m_copilotContextCache->is_newest = true;
}
wxString SCH_EDIT_FRAME::GetBomList()
{
    SCH_REFERENCE_LIST referenceList;
    Schematic().Hierarchy().GetSymbols( referenceList, false, false );
    FIELDS_EDITOR_GRID_DATA_MODEL dataModel( referenceList );

    for( int fieldId : MANDATORY_FIELDS )
    {
        dataModel.AddColumn( GetCanonicalFieldName( fieldId ),
                             GetDefaultFieldName( fieldId, DO_TRANSLATE ), false );
    }

    dataModel.ApplyBomPreset( BOM_PRESET::GroupedByValueFootprint() );
    return dataModel.Export( BOM_FMT_PRESET::CSV() ).ToStdString();
}

wxString SCH_EDIT_FRAME::GetNetList()
{
    UpdateCopilotContextCache();
    return m_copilotContextCache->net_list;
}

SYMBOL_CMD_CONTEXT const& SCH_EDIT_FRAME::GetSelectedSymbolContext()
{
    EE_SELECTION_TOOL* selTool = GetToolManager()->GetTool<EE_SELECTION_TOOL>();
    EE_SELECTION&      selection = selTool->GetSelection();
    SCH_SYMBOL*        symbol = dynamic_cast<SCH_SYMBOL*>( selection.Front() );

    if( !symbol )
        return *m_symbolCmdContext;

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

#endif
