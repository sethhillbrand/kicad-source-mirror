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

#pragma once

#include <set>
#include <vector>

#include <wx/dialog.h>
#include <wx/checklst.h>
#include <wx/checkbox.h>
#include <wx/spinctrl.h>

#include <base_units.h>
#include <board.h>
#include <footprint.h>
#include <pcb_base_frame.h>

struct BGA_BREAKOUT_SETTINGS
{
    std::set<wxString> powerNets;
    std::set<wxString> powerNetclasses;
    int                trackWidth;            ///< Track width to use for breakout routes (internal units)
    bool               useBlindVias;          ///< Use blind vias instead of through hole
    bool               useViaInPad;           ///< Place vias at pad centers instead of dogbones
    bool               escapeUnconnectedPads; ///< Break out pads even without assigned nets
    bool               breakoutDiffPairsFirst;///< Preferentially break out differential pairs
};

class DIALOG_BGA_BREAKOUT : public wxDialog
{
public:
    DIALOG_BGA_BREAKOUT( PCB_BASE_FRAME* aParent, BOARD* aBoard, FOOTPRINT* aFootprint );

    BGA_BREAKOUT_SETTINGS GetSettings() const;

private:
    void populateNetLists();

    PCB_BASE_FRAME* m_frame;
    BOARD*          m_board;
    FOOTPRINT*      m_footprint;

    wxCheckListBox*    m_netListCtrl;
    wxCheckListBox*    m_netclassListCtrl;
    wxSpinCtrlDouble*  m_trackWidthCtrl;
    wxCheckBox*        m_useBlindViasCtrl;
    wxCheckBox*        m_useViaInPadCtrl;
    wxCheckBox*        m_escapeUnconnectedCtrl;
    wxCheckBox*        m_breakoutDiffPairsCtrl;

    std::vector<wxString> m_netNames;
    std::vector<wxString> m_netclassNames;
};

