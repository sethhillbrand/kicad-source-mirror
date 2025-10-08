/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright The KiCad Developers, see AUTHORS.txt for contributors.
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

#include "dialogs/dialog_bga_breakout.h"

#include <algorithm>

#include <board_design_settings.h>
#include <netinfo.h>

#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/stattext.h>

DIALOG_BGA_BREAKOUT::DIALOG_BGA_BREAKOUT( PCB_BASE_FRAME* aParent, BOARD* aBoard,
                                          FOOTPRINT* aFootprint ) :
        wxDialog( aParent, wxID_ANY, _( "BGA Breakout" ), wxDefaultPosition, wxDefaultSize,
                  wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER ),
        m_frame( aParent ),
        m_board( aBoard ),
        m_footprint( aFootprint ),
        m_netListCtrl( nullptr ),
        m_netclassListCtrl( nullptr ),
        m_trackWidthCtrl( nullptr ),
        m_useBlindViasCtrl( nullptr ),
        m_useViaInPadCtrl( nullptr ),
        m_escapeUnconnectedCtrl( nullptr ),
        m_breakoutDiffPairsCtrl( nullptr )
{
    wxBoxSizer* topSizer = new wxBoxSizer( wxVERTICAL );

    wxStaticBoxSizer* powerNetSizer = new wxStaticBoxSizer( wxVERTICAL, this, _( "Power Nets" ) );
    m_netListCtrl = new wxCheckListBox( this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                        wxArrayString() );
    powerNetSizer->Add( m_netListCtrl, 1, wxEXPAND | wxALL, 5 );
    topSizer->Add( powerNetSizer, 1, wxEXPAND | wxALL, 5 );

    wxStaticBoxSizer* netclassSizer =
            new wxStaticBoxSizer( wxVERTICAL, this, _( "Power Netclasses" ) );
    m_netclassListCtrl = new wxCheckListBox( this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                             wxArrayString() );
    netclassSizer->Add( m_netclassListCtrl, 1, wxEXPAND | wxALL, 5 );
    topSizer->Add( netclassSizer, 1, wxEXPAND | wxLEFT | wxRIGHT, 5 );

    wxFlexGridSizer* gridSizer = new wxFlexGridSizer( 0, 2, 5, 5 );
    gridSizer->AddGrowableCol( 1, 1 );

    wxStaticText* trackLabel = new wxStaticText( this, wxID_ANY, _( "Breakout track width" ) );
    gridSizer->Add( trackLabel, 0, wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT, 5 );

    m_trackWidthCtrl = new wxSpinCtrlDouble( this, wxID_ANY );
    m_trackWidthCtrl->SetDigits( 3 );
    m_trackWidthCtrl->SetIncrement( 0.01 );
    m_trackWidthCtrl->SetRange( 0.01, 5.0 );
    gridSizer->Add( m_trackWidthCtrl, 0, wxEXPAND | wxRIGHT, 5 );

    m_useBlindViasCtrl = new wxCheckBox( this, wxID_ANY, _( "Use blind vias" ) );
    gridSizer->Add( m_useBlindViasCtrl, 0, wxLEFT | wxRIGHT, 5 );
    m_useViaInPadCtrl = new wxCheckBox( this, wxID_ANY, _( "Use via in pad" ) );
    gridSizer->Add( m_useViaInPadCtrl, 0, wxLEFT | wxRIGHT, 5 );

    m_escapeUnconnectedCtrl = new wxCheckBox( this, wxID_ANY, _( "Escape unconnected pads" ) );
    gridSizer->Add( m_escapeUnconnectedCtrl, 0, wxLEFT | wxRIGHT, 5 );
    m_breakoutDiffPairsCtrl =
            new wxCheckBox( this, wxID_ANY, _( "Break out differential pairs first" ) );
    gridSizer->Add( m_breakoutDiffPairsCtrl, 0, wxLEFT | wxRIGHT, 5 );

    topSizer->Add( gridSizer, 0, wxEXPAND | wxALL, 5 );

    wxStdDialogButtonSizer* buttons = new wxStdDialogButtonSizer();
    buttons->AddButton( new wxButton( this, wxID_OK ) );
    buttons->AddButton( new wxButton( this, wxID_CANCEL ) );
    buttons->Realize();

    topSizer->Add( buttons, 0, wxALIGN_RIGHT | wxALL, 5 );

    SetSizerAndFit( topSizer );

    populateNetLists();

    double defaultWidth = 0.15;

    if( m_board )
    {
        int boardWidth = m_board->GetDesignSettings().GetCurrentTrackWidth();

        if( boardWidth > 0 )
            defaultWidth = pcbIUScale.IUTomm( boardWidth );
    }

    m_trackWidthCtrl->SetValue( defaultWidth );
}

BGA_BREAKOUT_SETTINGS DIALOG_BGA_BREAKOUT::GetSettings() const
{
    BGA_BREAKOUT_SETTINGS settings;

    for( size_t ii = 0; ii < m_netNames.size(); ++ii )
    {
        if( m_netListCtrl->IsChecked( ii ) )
            settings.powerNets.insert( m_netNames[ii] );
    }

    for( size_t ii = 0; ii < m_netclassNames.size(); ++ii )
    {
        if( m_netclassListCtrl->IsChecked( ii ) )
            settings.powerNetclasses.insert( m_netclassNames[ii] );
    }

    settings.trackWidth = pcbIUScale.mmToIU( m_trackWidthCtrl->GetValue() );
    settings.useBlindVias = m_useBlindViasCtrl->GetValue();
    settings.useViaInPad = m_useViaInPadCtrl->GetValue();
    settings.escapeUnconnectedPads = m_escapeUnconnectedCtrl->GetValue();
    settings.breakoutDiffPairsFirst = m_breakoutDiffPairsCtrl->GetValue();

    return settings;
}

void DIALOG_BGA_BREAKOUT::populateNetLists()
{
    if( !m_board )
        return;

    const NETNAMES_MAP& nets = m_board->GetNetInfo().NetsByName();

    m_netNames.reserve( nets.size() );

    for( const auto& [ name, net ] : nets )
    {
        if( !net )
            continue;

        m_netNames.push_back( name );
        m_netListCtrl->Append( name );
    }

    BOARD_DESIGN_SETTINGS& designSettings = m_board->GetDesignSettings();

    if( designSettings.m_NetSettings )
    {
        const auto& netclasses = designSettings.m_NetSettings->GetNetclasses();

        for( const auto& [ name, netclass ] : netclasses )
        {
            if( !netclass )
                continue;

            m_netclassNames.push_back( name );
            m_netclassListCtrl->Append( name );
        }
    }
}

