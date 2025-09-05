/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright The KiCad Developers, see AUTHORS.TXT for contributors.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you may find one here:
 * https://www.gnu.org/licenses/gpl-3.0.en.html
 */

#include <boost/test/unit_test.hpp>

#include <qa_utils/wx_utils/unit_test_utils.h>
#include <schematic_utils/schematic_file_util.h>

#include <connection_graph.h>
#include <schematic.h>
#include <sch_sheet.h>
#include <settings/settings_manager.h>
#include <locale_io.h>

struct SIGNALS_TEST_FIXTURE
{
    SIGNALS_TEST_FIXTURE() : m_settingsManager( true /* headless */ ) {}

    SETTINGS_MANAGER           m_settingsManager;
    std::unique_ptr<SCHEMATIC> m_schematic;
};

BOOST_FIXTURE_TEST_CASE( RebuildSignals_GroupsFourNetsIntoOneSignal, SIGNALS_TEST_FIXTURE )
{
    LOCALE_IO dummy;

    // Load the test schematic from qa/data/eeschema/signals_four_nets.kicad_sch
    KI_TEST::LoadSchematic( m_settingsManager, wxString( "signals_four_nets" ), m_schematic );

    SCH_SHEET_LIST sheets = m_schematic->BuildSheetListSortedByPageNumbers();
    CONNECTION_GRAPH* graph = m_schematic->ConnectionGraph();

    // Force a full recalc to ensure signals are built
    graph->Recalculate( sheets, /*aUnconditional=*/true );

    const auto& signals = graph->GetSignals();

    bool foundFourNetSignal = false;
    for( const auto& sig : signals )
    {
        if( sig && sig->GetNets().size() == 4 )
        {
            foundFourNetSignal = true;
            break;
        }
    }

    BOOST_CHECK_MESSAGE( foundFourNetSignal,
                         "Expected at least one signal composed of exactly 4 nets to be built" );
}

BOOST_FIXTURE_TEST_CASE( RebuildSignals_RespectsSignalLabelAndKeepsGrouping, SIGNALS_TEST_FIXTURE )
{
    LOCALE_IO dummy;

    // Labeled variant of the four-nets series chain; label placed on the center wire as "SIG".
    KI_TEST::LoadSchematic( m_settingsManager, wxString( "signals_four_nets_labeled" ), m_schematic );

    SCH_SHEET_LIST sheets = m_schematic->BuildSheetListSortedByPageNumbers();
    CONNECTION_GRAPH* graph = m_schematic->ConnectionGraph();
    graph->Recalculate( sheets, /*aUnconditional=*/true );

    const auto& signals = graph->GetSignals();

    bool foundLabeled = false;
    for( const auto& sig : signals )
    {
        if( !sig )
            continue;

        wxString name = sig->GetName();
        // In some contexts names can be sheet-prefixed like "/SIG"; normalize by stripping leading '/'
        if( name.StartsWith( wxString( "/" ) ) )
            name = name.Mid( 1 );

        if( sig->GetNets().size() == 4 && name == wxString( "SIG" ) )
        {
            foundLabeled = true;
            break;
        }
    }

    BOOST_CHECK_MESSAGE( foundLabeled,
                         "Expected a 4-net signal named 'SIG' to be built from label" );
}

BOOST_FIXTURE_TEST_CASE( RebuildSignals_WithPullupBranch_IncludesPowerNetAndStillGroups, SIGNALS_TEST_FIXTURE )
{
    LOCALE_IO dummy;

    // Series chain with a pull-up branch to VCC via RPU from the middle node.
    KI_TEST::LoadSchematic( m_settingsManager, wxString( "signals_with_pullup" ), m_schematic );

    SCH_SHEET_LIST sheets = m_schematic->BuildSheetListSortedByPageNumbers();
    CONNECTION_GRAPH* graph = m_schematic->ConnectionGraph();
    graph->Recalculate( sheets, /*aUnconditional=*/true );

    const auto& signals = graph->GetSignals();

    bool hasGroupWithVCC = false;
    for( const auto& sig : signals )
    {
        if( !sig )
            continue;

        const auto& nets = sig->GetNets();

        // We expect at least the original 4 nets plus the power net (name may be "VCC").
        if( nets.size() >= 5 )
        {
            for( const wxString& n : nets )
            {
                // Normalize for hierarchical prefix like '/VCC'
                wxString nn = n;
                if( nn.StartsWith( wxString( "/" ) ) )
                    nn = nn.Mid( 1 );

                if( nn.CmpNoCase( wxString( "VCC" ) ) == 0 )
                {
                    hasGroupWithVCC = true;
                    break;
                }
            }
        }

        if( hasGroupWithVCC )
            break;
    }

    BOOST_CHECK_MESSAGE( hasGroupWithVCC,
                         "Expected grouped signal to include VCC net via pull-up branch" );
}

BOOST_FIXTURE_TEST_CASE( RebuildSignals_WithBypassCap_IncludesGNDNetAndStillGroups, SIGNALS_TEST_FIXTURE )
{
    LOCALE_IO dummy;

    // Series chain with a bypass capacitor from middle node to GND.
    KI_TEST::LoadSchematic( m_settingsManager, wxString( "signals_with_bypass" ), m_schematic );

    SCH_SHEET_LIST sheets = m_schematic->BuildSheetListSortedByPageNumbers();
    CONNECTION_GRAPH* graph = m_schematic->ConnectionGraph();
    graph->Recalculate( sheets, /*aUnconditional=*/true );

    const auto& signals = graph->GetSignals();

    bool hasGroupWithGND = false;
    for( const auto& sig : signals )
    {
        if( !sig )
            continue;

        const auto& nets = sig->GetNets();

        // We expect the original 4 nets plus GND in the group that includes the middle node.
        if( nets.size() >= 5 )
        {
            for( const wxString& n : nets )
            {
                wxString nn = n;
                if( nn.StartsWith( wxString( "/" ) ) )
                    nn = nn.Mid( 1 );

                if( nn.CmpNoCase( wxString( "GND" ) ) == 0 )
                {
                    hasGroupWithGND = true;
                    break;
                }
            }
        }

        if( hasGroupWithGND )
            break;
    }

    BOOST_CHECK_MESSAGE( hasGroupWithGND,
                         "Expected grouped signal to include GND net via bypass capacitor" );
}
