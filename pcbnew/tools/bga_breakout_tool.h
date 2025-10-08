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

#include <map>
#include <vector>

#include <math/vector2d.h>
#include <tools/pcb_tool_base.h>
#include <tools/generator_tool_pns_proxy.h>
#include <wx/string.h>

#include <dialogs/dialog_bga_breakout.h>

class PAD;
class FOOTPRINT;
class BOARD_COMMIT;
class PCB_VIA;

class BGA_BREAKOUT_TOOL : public GENERATOR_TOOL_PNS_PROXY
{
public:
    BGA_BREAKOUT_TOOL();
    ~BGA_BREAKOUT_TOOL() override;

    void Reset( RESET_REASON aReason ) override;
    bool Init() override;

    enum class QUADRANT
    {
        TOP_LEFT,
        TOP_RIGHT,
        BOTTOM_LEFT,
        BOTTOM_RIGHT
    };

private:
    struct PAD_ESCAPE_INFO
    {
        PAD* pad;
        QUADRANT quadrant;
        int availableSpace; ///< Calculated remaining channel width in internal units
        int remainingSpace; ///< Remaining width after planning routes
        VECTOR2I direction;
        int ringOrder;      ///< Approximate ring index used for outside-in traversal
        double angle;       ///< Angular position used for circular iteration
        int gridRing;       ///< Integer ring index derived from pad grid coordinates
    };

    struct GRID_MAPPING
    {
        std::map<int, int> xIndex;
        std::map<int, int> yIndex;
        int                centerXIndex = 0;
        int                centerYIndex = 0;
        int                pitchX = 0;
        int                pitchY = 0;
    };

    int startBreakout( const TOOL_EVENT& aEvent );

    void performBreakout( FOOTPRINT* aFootprint, const BGA_BREAKOUT_SETTINGS& aSettings );
    QUADRANT determineQuadrant( const VECTOR2I& aPadPos, const VECTOR2I& aCenter ) const;
    std::vector<PAD*> collectPadsForQuadrant( FOOTPRINT* aFootprint, QUADRANT aQuadrant ) const;
    std::vector<PAD*> findDirectionalNeighbors( PAD* aPad, QUADRANT aQuadrant,
                                                const std::vector<PAD*>& aAllPads ) const;
    int calculateAvailableSpace( PAD* aPad, const std::vector<PAD*>& aNeighbors,
                                 QUADRANT aQuadrant ) const;
    bool isLikelyDiffPairNet( const wxString& aNetName ) const;
    bool shouldSkipPad( PAD* aPad, const BGA_BREAKOUT_SETTINGS& aSettings ) const;
    int getPadClearance( PAD* aPad ) const;
    GRID_MAPPING buildGridMapping( FOOTPRINT* aFootprint ) const;
    int computeRingOrder( PAD* aPad, const VECTOR2I& aCenter ) const;
    int computeGridRing( PAD* aPad, const GRID_MAPPING& aGrid ) const;
    double computeAngularPosition( PAD* aPad, const VECTOR2I& aCenter ) const;
    VECTOR2I computeBreakoutPoint( const VECTOR2I& aOrigin, const VECTOR2D& aDirection,
                                   const BOX2I& aBoundingBox, int aMargin ) const;
    PCB_VIA* createViaForPad( PAD* aPad, const BGA_BREAKOUT_SETTINGS& aSettings,
                              const VECTOR2I& aPosition ) const;
    VECTOR2I computeDogboneViaPosition( PAD* aPad, QUADRANT aQuadrant,
                                        const GRID_MAPPING& aGrid,
                                        const BGA_BREAKOUT_SETTINGS& aSettings ) const;
    bool ensureLayerCapacity( FOOTPRINT* aFootprint, const BGA_BREAKOUT_SETTINGS& aSettings,
                              const std::map<QUADRANT, std::vector<PAD_ESCAPE_INFO>>& aPadInfo,
                              int& aAvailableLayers, int& aRequiredLayers ) const;
    bool routeWithRouter( PAD_ESCAPE_INFO& aInfo, const BGA_BREAKOUT_SETTINGS& aSettings,
                          const BOX2I& aBoundingBox, BOARD_COMMIT& aCommit,
                          double aLateralOffset, int aTrackWidth, int aRequiredWidth,
                          int& aTrackCount, int& aViaCount, const GRID_MAPPING& aGrid );
    bool routePadEscape( PAD_ESCAPE_INFO& aInfo, const BGA_BREAKOUT_SETTINGS& aSettings,
                         const BOX2I& aBoundingBox, BOARD_COMMIT& aCommit,
                         double aLateralOffset, int aTrackWidth, int aRequiredWidth,
                         int& aTrackCount, int& aViaCount, const GRID_MAPPING& aGrid );

    void setTransitions() override;

    DIALOG_BGA_BREAKOUT* m_dialog;
};

