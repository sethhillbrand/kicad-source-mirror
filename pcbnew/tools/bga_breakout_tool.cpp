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

#include "tools/bga_breakout_tool.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <map>
#include <memory>
#include <set>
#include <vector>

#include <confirm.h>
#include <footprint.h>
#include <board_design_settings.h>
#include <board_commit.h>
#include <pad.h>
#include <base_units.h>
#include <tools/pcb_actions.h>
#include <pcb_base_frame.h>
#include <pcb_edit_frame.h>
#include <pcb_track.h>
#include <tool/tool_manager.h>
#include <tools/pcb_selection.h>
#include <tools/pcb_selection_tool.h>
#include <zone.h>

#include <lset.h>
#include <router/pns_kicad_iface.h>
#include <router/pns_router.h>
#include <router/pns_sizes_settings.h>

#include <wx/log.h>
#include <wx/intl.h>

#include <netclass.h>
#include <netinfo.h>

namespace
{
wxString describeQuadrant( BGA_BREAKOUT_TOOL::QUADRANT aQuadrant )
{
    switch( aQuadrant )
    {
    case BGA_BREAKOUT_TOOL::QUADRANT::TOP_LEFT:      return _( "Top-left" );
    case BGA_BREAKOUT_TOOL::QUADRANT::TOP_RIGHT:     return _( "Top-right" );
    case BGA_BREAKOUT_TOOL::QUADRANT::BOTTOM_LEFT:   return _( "Bottom-left" );
    case BGA_BREAKOUT_TOOL::QUADRANT::BOTTOM_RIGHT:  return _( "Bottom-right" );
    }

    return wxString();
}

VECTOR2I quadrantDirection( BGA_BREAKOUT_TOOL::QUADRANT aQuadrant )
{
    switch( aQuadrant )
    {
    case BGA_BREAKOUT_TOOL::QUADRANT::TOP_LEFT:      return VECTOR2I( -1, -1 );
    case BGA_BREAKOUT_TOOL::QUADRANT::TOP_RIGHT:     return VECTOR2I( 1, -1 );
    case BGA_BREAKOUT_TOOL::QUADRANT::BOTTOM_LEFT:   return VECTOR2I( -1, 1 );
    case BGA_BREAKOUT_TOOL::QUADRANT::BOTTOM_RIGHT:  return VECTOR2I( 1, 1 );
    }

    return VECTOR2I( 0, 0 );
}
}

BGA_BREAKOUT_TOOL::BGA_BREAKOUT_TOOL() : GENERATOR_TOOL_PNS_PROXY( "pcbnew.BGABreakout" ),
    m_dialog( nullptr )
{
}

BGA_BREAKOUT_TOOL::~BGA_BREAKOUT_TOOL()
{
}

void BGA_BREAKOUT_TOOL::Reset( RESET_REASON aReason )
{
    GENERATOR_TOOL_PNS_PROXY::Reset( aReason );
    m_dialog = nullptr;
}

bool BGA_BREAKOUT_TOOL::Init()
{
    return true;
}

void BGA_BREAKOUT_TOOL::setTransitions()
{
    Go( &BGA_BREAKOUT_TOOL::startBreakout, PCB_ACTIONS::bgaBreakout.MakeEvent() );
}

int BGA_BREAKOUT_TOOL::startBreakout( const TOOL_EVENT& aEvent )
{
    PCB_BASE_FRAME* frame = getEditFrame<PCB_BASE_FRAME>();

    if( !frame )
        return 0;

    FOOTPRINT* footprint = nullptr;

    if( PCB_SELECTION_TOOL* selectionTool = m_toolMgr->GetTool<PCB_SELECTION_TOOL>() )
    {
        const PCB_SELECTION& selection = selectionTool->GetSelection();

        for( EDA_ITEM* item : selection )
        {
            if( FOOTPRINT::ClassOf( item ) )
            {
                footprint = static_cast<FOOTPRINT*>( item );
                break;
            }

            if( BOARD_ITEM* boardItem = dynamic_cast<BOARD_ITEM*>( item ) )
            {
                if( boardItem->GetParentFootprint() )
                {
                    footprint = boardItem->GetParentFootprint();
                    break;
                }
            }
        }
    }

    if( !footprint )
    {
        DisplayInfoMessage( frame, _( "Select a BGA footprint before running the breakout tool." ) );
        return 0;
    }

    DIALOG_BGA_BREAKOUT dlg( frame, board(), footprint );
    m_dialog = &dlg;

    if( dlg.ShowModal() == wxID_OK )
    {
        BGA_BREAKOUT_SETTINGS settings = dlg.GetSettings();
        performBreakout( footprint, settings );
    }

    m_dialog = nullptr;

    return 0;
}

BGA_BREAKOUT_TOOL::QUADRANT BGA_BREAKOUT_TOOL::determineQuadrant( const VECTOR2I& aPadPos,
                                                                  const VECTOR2I& aCenter ) const
{
    const bool right = aPadPos.x >= aCenter.x;
    const bool bottom = aPadPos.y >= aCenter.y;

    if( right && bottom )
        return QUADRANT::BOTTOM_RIGHT;
    else if( right && !bottom )
        return QUADRANT::TOP_RIGHT;
    else if( !right && bottom )
        return QUADRANT::BOTTOM_LEFT;

    return QUADRANT::TOP_LEFT;
}

std::vector<PAD*> BGA_BREAKOUT_TOOL::collectPadsForQuadrant( FOOTPRINT* aFootprint,
                                                             QUADRANT aQuadrant ) const
{
    std::vector<PAD*> pads;

    if( !aFootprint )
        return pads;

    const VECTOR2I center = aFootprint->GetBoundingBox().GetCenter();

    for( PAD* pad : aFootprint->Pads() )
    {
        if( !pad )
            continue;

        if( determineQuadrant( pad->GetPosition(), center ) == aQuadrant )
            pads.push_back( pad );
    }

    return pads;
}

std::vector<PAD*> BGA_BREAKOUT_TOOL::findDirectionalNeighbors( PAD* aPad, QUADRANT aQuadrant,
                                                               const std::vector<PAD*>& aAllPads ) const
{
    std::vector<PAD*> neighbors;

    if( !aPad )
        return neighbors;

    const VECTOR2I direction = quadrantDirection( aQuadrant );
    const VECTOR2I origin = aPad->GetPosition();

    neighbors.reserve( 3 );

    std::vector<std::pair<PAD*, int64_t>> candidates;

    for( PAD* pad : aAllPads )
    {
        if( pad == aPad || !pad )
            continue;

        VECTOR2I delta = pad->GetPosition() - origin;

        if( direction.x != 0 && delta.x * direction.x <= 0 )
            continue;

        if( direction.y != 0 && delta.y * direction.y <= 0 )
            continue;

        int64_t projection = static_cast<int64_t>( delta.x ) * direction.x
                              + static_cast<int64_t>( delta.y ) * direction.y;

        if( projection <= 0 )
            continue;

        int64_t distance = static_cast<int64_t>( delta.x ) * delta.x
                            + static_cast<int64_t>( delta.y ) * delta.y;

        candidates.emplace_back( pad, distance );
    }

    std::sort( candidates.begin(), candidates.end(),
               []( const std::pair<PAD*, int64_t>& a, const std::pair<PAD*, int64_t>& b )
               {
                   return a.second < b.second;
               } );

    for( size_t ii = 0; ii < candidates.size() && ii < 3; ++ii )
        neighbors.push_back( candidates[ii].first );

    return neighbors;
}

int BGA_BREAKOUT_TOOL::calculateAvailableSpace( PAD* aPad, const std::vector<PAD*>& aNeighbors,
                                                QUADRANT aQuadrant ) const
{
    if( !aPad )
        return 0;

    const BOX2I padBox = aPad->GetBoundingBox();
    int minSpace = std::numeric_limits<int>::max();

    for( PAD* neighbor : aNeighbors )
    {
        if( !neighbor )
            continue;

        const BOX2I neighborBox = neighbor->GetBoundingBox();
        const VECTOR2I delta = neighbor->GetPosition() - aPad->GetPosition();

        const int padHalfWidth = padBox.GetWidth() / 2;
        const int padHalfHeight = padBox.GetHeight() / 2;
        const int neighborHalfWidth = neighborBox.GetWidth() / 2;
        const int neighborHalfHeight = neighborBox.GetHeight() / 2;

        const int clearanceX = std::abs( delta.x ) - ( padHalfWidth + neighborHalfWidth );
        const int clearanceY = std::abs( delta.y ) - ( padHalfHeight + neighborHalfHeight );

        const int candidate = std::min( clearanceX, clearanceY );

        if( candidate < minSpace )
            minSpace = candidate;
    }

    if( minSpace == std::numeric_limits<int>::max() )
        minSpace = std::max( padBox.GetWidth(), padBox.GetHeight() );

    return std::max( 0, minSpace );
}

bool BGA_BREAKOUT_TOOL::isLikelyDiffPairNet( const wxString& aNetName ) const
{
    if( aNetName.IsEmpty() )
        return false;

    wxString upper = aNetName.Upper();

    if( upper.EndsWith( "_P" ) || upper.EndsWith( "_N" ) )
        return true;

    if( upper.EndsWith( "+" ) || upper.EndsWith( "-" ) )
        return true;

    return false;
}

bool BGA_BREAKOUT_TOOL::shouldSkipPad( PAD* aPad, const BGA_BREAKOUT_SETTINGS& aSettings ) const
{
    if( !aPad )
        return true;

    if( !aSettings.escapeUnconnectedPads && ( aPad->GetNet() == nullptr ) )
        return true;

    if( NETINFO_ITEM* net = aPad->GetNet() )
    {
        if( aSettings.powerNets.count( net->GetNetname() ) )
            return true;

        if( NETCLASS* netclass = net->GetNetClass() )
        {
            if( aSettings.powerNetclasses.count( netclass->GetName() ) )
                return true;
        }
    }

    return false;
}

int BGA_BREAKOUT_TOOL::getPadClearance( PAD* aPad ) const
{
    if( !aPad )
        return 0;

    NETCLASS* netclass = aPad->GetEffectiveNetClass();

    if( netclass && netclass->GetClearance() > 0 )
        return netclass->GetClearance();

    return 0;
}

BGA_BREAKOUT_TOOL::GRID_MAPPING BGA_BREAKOUT_TOOL::buildGridMapping( FOOTPRINT* aFootprint ) const
{
    GRID_MAPPING grid;

    if( !aFootprint )
        return grid;

    std::set<int> xPositions;
    std::set<int> yPositions;

    for( PAD* pad : aFootprint->Pads() )
    {
        if( !pad )
            continue;

        xPositions.insert( pad->GetPosition().x );
        yPositions.insert( pad->GetPosition().y );
    }

    std::vector<int> xSorted( xPositions.begin(), xPositions.end() );
    std::vector<int> ySorted( yPositions.begin(), yPositions.end() );

    for( size_t ii = 0; ii < xSorted.size(); ++ii )
        grid.xIndex[xSorted[ii]] = static_cast<int>( ii );

    for( size_t ii = 0; ii < ySorted.size(); ++ii )
        grid.yIndex[ySorted[ii]] = static_cast<int>( ii );

    VECTOR2I center = aFootprint->GetBoundingBox().GetCenter();

    auto findClosestIndex =[]( const std::vector<int>& values, int target )
    {
        if( values.empty() )
            return 0;

        int bestIndex = 0;
        int bestDistance = std::numeric_limits<int>::max();

        for( size_t ii = 0; ii < values.size(); ++ii )
        {
            int distance = std::abs( values[ii] - target );

            if( distance < bestDistance )
            {
                bestDistance = distance;
                bestIndex = static_cast<int>( ii );
            }
        }

        return bestIndex;
    };

    grid.centerXIndex = findClosestIndex( xSorted, center.x );
    grid.centerYIndex = findClosestIndex( ySorted, center.y );

    grid.pitchX = 0;
    grid.pitchY = 0;

    for( size_t ii = 1; ii < xSorted.size(); ++ii )
    {
        int diff = xSorted[ii] - xSorted[ii - 1];

        if( diff > 0 && ( grid.pitchX == 0 || diff < grid.pitchX ) )
            grid.pitchX = diff;
    }

    for( size_t ii = 1; ii < ySorted.size(); ++ii )
    {
        int diff = ySorted[ii] - ySorted[ii - 1];

        if( diff > 0 && ( grid.pitchY == 0 || diff < grid.pitchY ) )
            grid.pitchY = diff;
    }

    return grid;
}

int BGA_BREAKOUT_TOOL::computeRingOrder( PAD* aPad, const VECTOR2I& aCenter ) const
{
    if( !aPad )
        return 0;

    VECTOR2I delta = aPad->GetPosition() - aCenter;

    return std::max( std::abs( delta.x ), std::abs( delta.y ) );
}

int BGA_BREAKOUT_TOOL::computeGridRing( PAD* aPad, const GRID_MAPPING& aGrid ) const
{
    if( !aPad )
        return 0;

    VECTOR2I pos = aPad->GetPosition();

    auto xIt = aGrid.xIndex.find( pos.x );
    auto yIt = aGrid.yIndex.find( pos.y );

    if( xIt == aGrid.xIndex.end() || yIt == aGrid.yIndex.end() )
        return 0;

    int xDelta = std::abs( xIt->second - aGrid.centerXIndex );
    int yDelta = std::abs( yIt->second - aGrid.centerYIndex );

    return std::max( xDelta, yDelta );
}

double BGA_BREAKOUT_TOOL::computeAngularPosition( PAD* aPad, const VECTOR2I& aCenter ) const
{
    if( !aPad )
        return 0.0;

    VECTOR2I delta = aPad->GetPosition() - aCenter;

    return std::atan2( static_cast<double>( delta.y ), static_cast<double>( delta.x ) );
}

bool BGA_BREAKOUT_TOOL::ensureLayerCapacity( FOOTPRINT* aFootprint,
                                             const BGA_BREAKOUT_SETTINGS& aSettings,
                                             const std::map<QUADRANT, std::vector<PAD_ESCAPE_INFO>>& aPadInfo,
                                             int& aAvailableLayers, int& aRequiredLayers ) const
{
    aAvailableLayers = 0;
    aRequiredLayers = 0;

    if( !board() || !aFootprint )
        return false;

    std::set<int> gridRings;

    for( const auto& entry : aPadInfo )
    {
        for( const PAD_ESCAPE_INFO& info : entry.second )
            gridRings.insert( info.gridRing );
    }

    int layersNeeded = gridRings.empty() ? 0 : ( *gridRings.rbegin() + 1 );
    aRequiredLayers = layersNeeded;

    int copperLayers = board()->GetCopperLayerCount();

    if( copperLayers <= 0 )
        return false;

    LSET allCu = LSET::AllCuMask( copperLayers );

    auto boxesIntersect =[]( const BOX2I& a, const BOX2I& b )
    {
        if( a.GetWidth() <= 0 || a.GetHeight() <= 0 || b.GetWidth() <= 0 || b.GetHeight() <= 0 )
            return false;

        return !( a.GetRight() < b.GetLeft() || a.GetLeft() > b.GetRight()
                  || a.GetBottom() < b.GetTop() || a.GetTop() > b.GetBottom() );
    };

    std::set<PCB_LAYER_ID> blockedLayers;
    BOX2I footprintBox = aFootprint->GetBoundingBox();

    auto considerZone = [&]( ZONE* zone )
    {
        if( !zone )
            return;

        NETINFO_ITEM* zoneNet = zone->GetNet();

        if( !zoneNet )
            return;

        bool matches = aSettings.powerNets.count( zoneNet->GetNetname() ) > 0;

        if( !matches )
        {
            if( NETCLASS* zoneClass = zoneNet->GetNetClass() )
                matches = aSettings.powerNetclasses.count( zoneClass->GetName() ) > 0;
        }

        if( !matches )
            return;

        if( !boxesIntersect( zone->GetBoundingBox(), footprintBox ) )
            return;

        LSET layerSet = zone->GetLayerSet();

        for( PCB_LAYER_ID layer : layerSet.CuStack() )
        {
            if( allCu.Contains( layer ) )
                blockedLayers.insert( layer );
        }
    };

    for( ZONE* zone : board()->Zones() )
        considerZone( zone );

    if( aFootprint )
    {
        for( ZONE* zone : aFootprint->Zones() )
            considerZone( zone );
    }

    int available = 0;

    for( PCB_LAYER_ID layer : allCu.CuStack() )
    {
        if( blockedLayers.count( layer ) == 0 )
            available++;
    }

    aAvailableLayers = available;

    return layersNeeded <= available;
}

VECTOR2I BGA_BREAKOUT_TOOL::computeBreakoutPoint( const VECTOR2I& aOrigin, const VECTOR2D& aDirection,
                                                  const BOX2I& aBoundingBox, int aMargin ) const
{
    VECTOR2D originD( static_cast<double>( aOrigin.x ), static_cast<double>( aOrigin.y ) );
    VECTOR2D dir = aDirection;

    if( dir.SquaredEuclideanNorm() == 0.0 )
        return aOrigin;

    VECTOR2D dirNorm = dir.Resize( 1.0 );

    int targetX = dirNorm.x >= 0 ? aBoundingBox.GetRight() + aMargin : aBoundingBox.GetLeft() - aMargin;
    int targetY = dirNorm.y >= 0 ? aBoundingBox.GetBottom() + aMargin : aBoundingBox.GetTop() - aMargin;

    double distX = ( static_cast<double>( targetX ) - originD.x ) / dirNorm.x;
    double distY = ( static_cast<double>( targetY ) - originD.y ) / dirNorm.y;

    double distance = std::max( distX, distY );

    VECTOR2D result = originD + dirNorm * distance;

    return VECTOR2I( static_cast<int>( std::lround( result.x ) ),
                     static_cast<int>( std::lround( result.y ) ) );
}

PCB_VIA* BGA_BREAKOUT_TOOL::createViaForPad( PAD* aPad, const BGA_BREAKOUT_SETTINGS& aSettings,
                                             const VECTOR2I& aPosition ) const
{
    if( !aPad || !board() )
        return nullptr;

    PCB_VIA* via = new PCB_VIA( board() );

    via->SetPosition( aPosition );

    if( NETINFO_ITEM* net = aPad->GetNet() )
    {
        via->SetNet( net );
        via->SetNetCode( net->GetNetCode() );
        via->SetIsFree( false );
    }
    else
    {
        via->SetNetCode( 0 );
        via->SetIsFree( true );
    }

    BOARD_DESIGN_SETTINGS& designSettings = board()->GetDesignSettings();
    NETCLASS* netclass = aPad->GetEffectiveNetClass();

    // Determine the pad layer (for SMD pads on top or bottom)
    PCB_LAYER_ID padLayer = F_Cu;
    LSET padLayers = aPad->GetLayerSet();
    if( padLayers.Contains( B_Cu ) && !padLayers.Contains( F_Cu ) )
        padLayer = B_Cu;

    int viaDiameter = netclass && netclass->GetViaDiameter() > 0 ? netclass->GetViaDiameter()
                                                                 : designSettings.GetCurrentViaSize();
    int viaDrill = netclass && netclass->GetViaDrill() > 0 ? netclass->GetViaDrill()
                                                           : designSettings.GetCurrentViaDrill();

    via->SetWidth( PADSTACK::ALL_LAYERS, viaDiameter );
    via->SetDrill( viaDrill );
    via->SetRemoveUnconnected( true );

    if( aSettings.useBlindVias )
    {
        via->SetViaType( VIATYPE::BLIND );

        int copperLayers = designSettings.GetCopperLayerCount();
        int innerLayers = std::max( 0, copperLayers - 2 );
        PCB_LAYER_ID adjacent = padLayer;

        if( padLayer == F_Cu && innerLayers > 0 )
        {
            adjacent = static_cast<PCB_LAYER_ID>( In1_Cu );
        }
        else if( padLayer == B_Cu && innerLayers > 0 )
        {
            adjacent = static_cast<PCB_LAYER_ID>( static_cast<int>( In1_Cu ) + 2 * ( innerLayers - 1 ) );
        }

        via->SetLayerPair( padLayer, adjacent );
    }
    else
    {
        via->SetViaType( VIATYPE::THROUGH );

        PCB_LAYER_ID otherLayer = padLayer == F_Cu ? B_Cu : F_Cu;
        via->SetLayerPair( padLayer, otherLayer );
    }

    return via;
}

VECTOR2I BGA_BREAKOUT_TOOL::computeDogboneViaPosition( PAD* aPad, QUADRANT aQuadrant,
                                                       const GRID_MAPPING& aGrid,
                                                       const BGA_BREAKOUT_SETTINGS& aSettings ) const
{
    if( !aPad )
        return VECTOR2I();

    VECTOR2I viaPos = aPad->GetPosition();

    BOARD_DESIGN_SETTINGS& designSettings = board()->GetDesignSettings();
    NETCLASS* netclass = aPad->GetEffectiveNetClass();

    int viaDiameter = netclass && netclass->GetViaDiameter() > 0 ? netclass->GetViaDiameter()
                                                                 : designSettings.GetCurrentViaSize();

    if( aSettings.useViaInPad )
        return viaPos;

    const BOX2I padBox = aPad->GetBoundingBox();
    int padWidth = padBox.GetWidth();
    int padHeight = padBox.GetHeight();

    int clearance = getPadClearance( aPad );

    if( clearance <= 0 )
        clearance = designSettings.m_MinClearance;

    if( clearance <= 0 )
        clearance = pcbIUScale.mmToIU( 0.1 );

    int viaRadius = viaDiameter / 2;

    VECTOR2I direction = quadrantDirection( aQuadrant );

    auto computeOffset = [&]( int pitch, int padSize, int dirComponent )
    {
        if( dirComponent == 0 )
            return 0;

        int channel = pitch > 0 ? pitch - padSize : 0;
        int safety = clearance + viaRadius;

        if( channel <= 0 )
        {
            int offset = padSize / 2 + safety;
            return dirComponent > 0 ? offset : -offset;
        }

        int usable = std::max( 0, channel - 2 * safety );
        int offset = padSize / 2 + safety + usable / 2;

        return dirComponent > 0 ? offset : -offset;
    };

    int offsetX = computeOffset( aGrid.pitchX, padWidth, direction.x );
    int offsetY = computeOffset( aGrid.pitchY, padHeight, direction.y );

    viaPos.x += offsetX;
    viaPos.y += offsetY;

    BOX2I footprintBox = aPad->GetParentFootprint() ? aPad->GetParentFootprint()->GetBoundingBox() : BOX2I();

    if( footprintBox.GetWidth() > 0 && footprintBox.GetHeight() > 0 )
    {
        viaPos.x = std::clamp( viaPos.x, footprintBox.GetLeft(), footprintBox.GetRight() );
        viaPos.y = std::clamp( viaPos.y, footprintBox.GetTop(), footprintBox.GetBottom() );
    }

    return viaPos;
}

bool BGA_BREAKOUT_TOOL::routeWithRouter( PAD_ESCAPE_INFO& aInfo,
                                         const BGA_BREAKOUT_SETTINGS& aSettings,
                                         const BOX2I& aBoundingBox, BOARD_COMMIT& aCommit,
                                         double aLateralOffset, int aTrackWidth, int aRequiredWidth,
                                         int& aTrackCount, int& aViaCount, const GRID_MAPPING& aGrid )
{
    if( !board() || !aInfo.pad )
        return false;

    PNS::ROUTER* router = Router();
    PNS_KICAD_IFACE* iface = GetInterface();

    if( !router || !iface )
        return false;

    ClearRouterChanges();
    router->SyncWorld();

    PCB_LAYER_ID padLayer = aInfo.pad->GetLayer();
    VECTOR2I     padPos = aInfo.pad->GetPosition();

    PNS::ITEM* startItem = router->GetWorld()->FindItemByParent( aInfo.pad );

    if( !startItem )
        return false;

    router->SetMode( PNS::PNS_MODE_ROUTE_SINGLE );

    PNS::SIZES_SETTINGS sizes = router->Sizes();
    sizes.SetTrackWidth( aTrackWidth );
    sizes.SetTrackWidthIsExplicit( true );
    router->UpdateSizes( sizes );

    int startLayer = iface->GetPNSLayerFromBoardLayer( padLayer );

    if( !router->StartRouting( padPos, startItem, startLayer ) )
        return false;

    VECTOR2D direction( static_cast<double>( aInfo.direction.x ), static_cast<double>( aInfo.direction.y ) );

    if( direction.SquaredEuclideanNorm() == 0.0 )
    {
        router->StopRouting();
        ClearRouterChanges();
        return false;
    }

    direction = direction.Resize( 1.0 );
    VECTOR2D perpendicular( -direction.y, direction.x );

    VECTOR2D offsetPointD = VECTOR2D( static_cast<double>( padPos.x ), static_cast<double>( padPos.y ) )
                            + perpendicular * aLateralOffset;
    VECTOR2I offsetPoint( static_cast<int>( std::lround( offsetPointD.x ) ),
                          static_cast<int>( std::lround( offsetPointD.y ) ) );

    if( offsetPoint != padPos )
        router->Move( offsetPoint, nullptr );

    std::unique_ptr<PCB_VIA> viaPrototype;
    VECTOR2I viaPoint = padPos;

    viaPrototype.reset( createViaForPad( aInfo.pad, aSettings,
                                        computeDogboneViaPosition( aInfo.pad, aInfo.quadrant, aGrid,
                                                                   aSettings ) ) );

    if( viaPrototype )
    {
        viaPoint = viaPrototype->GetPosition();
        router->Move( viaPoint, nullptr );

        sizes = router->Sizes();
        sizes.SetViaDiameter( viaPrototype->GetWidth() );
        sizes.SetViaDrill( viaPrototype->GetDrillValue() );
        sizes.SetViaType( viaPrototype->GetViaType() );
        sizes.ClearLayerPairs();
        sizes.AddLayerPair( iface->GetPNSLayerFromBoardLayer( viaPrototype->TopLayer() ),
                            iface->GetPNSLayerFromBoardLayer( viaPrototype->BottomLayer() ) );
        router->UpdateSizes( sizes );

        if( !router->IsPlacingVia() )
            router->ToggleViaPlacement();

        router->Move( viaPoint, nullptr );

        if( router->IsPlacingVia() )
            router->ToggleViaPlacement();
    }

    int breakoutMargin = std::max( aRequiredWidth * 2, pcbIUScale.mmToIU( 0.3 ) );
    VECTOR2I breakoutPoint = computeBreakoutPoint( viaPoint, direction, aBoundingBox, breakoutMargin );

    router->Move( breakoutPoint, nullptr );

    if( !router->FixRoute( breakoutPoint, nullptr, true, true ) )
    {
        router->StopRouting();
        ClearRouterChanges();
        return false;
    }

    router->CommitRouting();

    const std::vector<GENERATOR_PNS_CHANGES>& routerChanges = GetRouterChanges();

    for( const GENERATOR_PNS_CHANGES& changes : routerChanges )
    {
        for( BOARD_ITEM* removed : changes.removedItems )
            aCommit.Remove( removed );

        for( BOARD_ITEM* added : changes.addedItems )
        {
            if( PCB_VIA::ClassOf( added ) )
            {
                PCB_VIA* via = static_cast<PCB_VIA*>( added );

                if( viaPrototype )
                {
                    via->SetWidth( viaPrototype->GetWidth() );
                    via->SetDrill( viaPrototype->GetDrillValue() );
                    via->SetViaType( viaPrototype->GetViaType() );
                    via->SetLayerPair( viaPrototype->TopLayer(), viaPrototype->BottomLayer() );
                    via->SetPosition( viaPrototype->GetPosition() );
                    via->SetRemoveUnconnected( true );
                }

                aViaCount++;
            }
            else if( PCB_TRACK::ClassOf( added ) )
            {
                PCB_TRACK* track = static_cast<PCB_TRACK*>( added );
                track->SetWidth( aTrackWidth );
                aTrackCount++;
            }

            aCommit.Add( added );
        }
    }

    ClearRouterChanges();

    return true;
}

bool BGA_BREAKOUT_TOOL::routePadEscape( PAD_ESCAPE_INFO& aInfo, const BGA_BREAKOUT_SETTINGS& aSettings,
                                        const BOX2I& aBoundingBox, BOARD_COMMIT& aCommit,
                                        double aLateralOffset, int aTrackWidth, int aRequiredWidth,
                                        int& aTrackCount, int& aViaCount, const GRID_MAPPING& aGrid )
{
    return routeWithRouter( aInfo, aSettings, aBoundingBox, aCommit, aLateralOffset, aTrackWidth,
                            aRequiredWidth, aTrackCount, aViaCount, aGrid );
}

void BGA_BREAKOUT_TOOL::performBreakout( FOOTPRINT* aFootprint,
                                         const BGA_BREAKOUT_SETTINGS& aSettings )
{
    if( !aFootprint )
        return;

    PCB_BASE_FRAME* frame = getEditFrame<PCB_BASE_FRAME>();

    if( !frame || !board() )
        return;

    const VECTOR2I center = aFootprint->GetBoundingBox().GetCenter();
    const BOX2I    footprintBBox = aFootprint->GetBoundingBox();
    GRID_MAPPING   grid = buildGridMapping( aFootprint );

    std::map<QUADRANT, std::vector<PAD_ESCAPE_INFO>> quadrantInfo;

    const std::array<QUADRANT, 4> quadrants = { QUADRANT::TOP_LEFT, QUADRANT::TOP_RIGHT,
                                                QUADRANT::BOTTOM_LEFT, QUADRANT::BOTTOM_RIGHT };

    for( QUADRANT quadrant : quadrants )
    {
        std::vector<PAD*> quadrantPads = collectPadsForQuadrant( aFootprint, quadrant );

        for( PAD* pad : quadrantPads )
        {
            if( !pad )
                continue;

            if( shouldSkipPad( pad, aSettings ) )
                continue;

            PAD_ESCAPE_INFO info;
            info.pad = pad;
            info.quadrant = quadrant;
            info.direction = quadrantDirection( quadrant );

            std::vector<PAD*> neighbors = findDirectionalNeighbors( pad, quadrant, quadrantPads );
            info.availableSpace = calculateAvailableSpace( pad, neighbors, quadrant );
            info.remainingSpace = info.availableSpace;
            info.ringOrder = computeRingOrder( pad, center );
            info.angle = computeAngularPosition( pad, center );
            info.gridRing = computeGridRing( pad, grid );

            quadrantInfo[quadrant].push_back( info );
        }
    }

    int availableLayers = 0;
    int requiredLayers = 0;

    if( !ensureLayerCapacity( aFootprint, aSettings, quadrantInfo, availableLayers, requiredLayers ) )
    {
        wxString message = _( "Insufficient copper layers available for the requested breakout." );
        message << ' ' << wxString::Format( _( "Required: %d, Available: %d" ), requiredLayers,
                                            availableLayers );
        DisplayInfoMessage( frame, message );
        return;
    }

    BOARD_COMMIT commit( frame );

    int routedPads = 0;
    int totalTracks = 0;
    int totalVias = 0;
    int blockedPads = 0;
    int insufficientPads = 0;

    auto orderPads = []( std::vector<PAD_ESCAPE_INFO*>& pads )
    {
        std::sort( pads.begin(), pads.end(),
                   []( const PAD_ESCAPE_INFO* lhs, const PAD_ESCAPE_INFO* rhs )
                   {
                       if( lhs->gridRing != rhs->gridRing )
                           return lhs->gridRing > rhs->gridRing;

                       if( lhs->ringOrder != rhs->ringOrder )
                           return lhs->ringOrder > rhs->ringOrder;

                       if( lhs->angle != rhs->angle )
                           return lhs->angle < rhs->angle;

                       return lhs->availableSpace > rhs->availableSpace;
                   } );
    };

    auto processPadList = [&]( std::vector<PAD_ESCAPE_INFO*>& pads,
                               std::map<int, bool>& ringSideState )
    {
        for( PAD_ESCAPE_INFO* infoPtr : pads )
        {
            if( !infoPtr || !infoPtr->pad )
                continue;

            PAD_ESCAPE_INFO& info = *infoPtr;

            NETCLASS* netclass = info.pad->GetEffectiveNetClass();
            int       clearance = getPadClearance( info.pad );
            int       trackWidth = aSettings.trackWidth;

            if( trackWidth <= 0 && netclass )
                trackWidth = netclass->GetTrackWidth();

            if( trackWidth <= 0 )
                trackWidth = pcbIUScale.mmToIU( 0.15 );

            const int requiredWidth = trackWidth + clearance;

            if( info.availableSpace <= 0 )
                blockedPads++;

            bool useSideChannel = info.availableSpace > 0 && requiredWidth <= info.availableSpace / 2;
            bool canCenterRoute = requiredWidth <= info.availableSpace && info.availableSpace > 0;
            double lateralOffset = 0.0;

            auto ringIt = ringSideState.find( info.ringOrder );

            if( ringIt == ringSideState.end() )
                ringIt = ringSideState.emplace( info.ringOrder, true ).first;

            if( useSideChannel )
            {
                double halfChannel = info.availableSpace / 2.0;
                double halfRequired = requiredWidth / 2.0;
                double offset = halfChannel - halfRequired;

                if( offset < 0 )
                    offset = 0.0;

                double sign = ringIt->second ? 1.0 : -1.0;
                lateralOffset = sign * offset;
                ringIt->second = !ringIt->second;
                info.remainingSpace = std::max( 0, info.availableSpace - requiredWidth );
            }
            else
            {
                lateralOffset = 0.0;
                info.remainingSpace = 0;

                if( !canCenterRoute )
                    insufficientPads++;
            }

            if( routePadEscape( info, aSettings, footprintBBox, commit, lateralOffset, trackWidth,
                                requiredWidth, totalTracks, totalVias, grid ) )
            {
                routedPads++;
            }
        }
    };

    for( QUADRANT quadrant : quadrants )
    {
        auto& pads = quadrantInfo[quadrant];

        if( pads.empty() )
            continue;

        std::vector<PAD_ESCAPE_INFO*> diffPads;
        std::vector<PAD_ESCAPE_INFO*> otherPads;

        diffPads.reserve( pads.size() );
        otherPads.reserve( pads.size() );

        for( PAD_ESCAPE_INFO& info : pads )
        {
            bool isDiff = info.pad && info.pad->GetNet()
                          && isLikelyDiffPairNet( info.pad->GetNet()->GetNetname() );

            if( aSettings.breakoutDiffPairsFirst && isDiff )
                diffPads.push_back( &info );
            else
                otherPads.push_back( &info );
        }

        orderPads( diffPads );
        orderPads( otherPads );

        std::map<int, bool> ringSideState;

        processPadList( diffPads, ringSideState );
        processPadList( otherPads, ringSideState );
    }

    if( routedPads > 0 )
        commit.Push( _( "BGA breakout routing" ) );

    wxString message;
    message << wxString::Format( _( "BGA breakout routed %d pads with %d tracks and %d vias." ),
                                 routedPads, totalTracks, totalVias ) << '\n';

    if( blockedPads > 0 )
        message << wxString::Format( _( "%d pads started with no available escape channel." ), blockedPads )
                << '\n';

    if( insufficientPads > 0 )
        message << wxString::Format( _( "%d pads exceeded available channel width and were forced through the center." ),
                                     insufficientPads )
                << '\n';

    DisplayInfoMessage( frame, message );
}

