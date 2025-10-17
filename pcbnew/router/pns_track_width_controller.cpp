/*
 * KiRouter - a push-and-(sometimes-)shove PCB router
 *
 * Copyright The KiCad Developers, see AUTHORS.txt for contributors.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "pns_track_width_controller.h"

#include <array>
#include <set>

#include <geometry/shape_simple.h>
#include <trace_helpers.h>
#include <board.h>
#include <drc/drc_engine.h>

#include "pns_node.h"
#include "pns_index.h"
#include "pns_segment.h"
#include "pns_solid.h"
#include "pns_router.h"

namespace PNS
{

TRACK_WIDTH_CONTROLLER::REGION::REGION( const std::shared_ptr<SHAPE_POLY_SET>& aShape, int aWidth ) :
        m_shape( aShape ),
        m_width( aWidth )
{
}


TRACK_WIDTH_CONTROLLER::TRACK_WIDTH_CONTROLLER() :
        m_iface( nullptr ),
        m_resolver( nullptr ),
        m_board( nullptr )
{
}


void TRACK_WIDTH_CONTROLLER::Initialize( ROUTER_IFACE* aIface, RULE_RESOLVER* aResolver )
{
    m_iface = aIface;
    m_resolver = aResolver;
    m_board = nullptr;
}


void TRACK_WIDTH_CONTROLLER::clear()
{
    m_indices.clear();
    m_regions.clear();
    m_defaultWidths.clear();
    m_layers.clear();
    m_lookupCache.clear();
}


void TRACK_WIDTH_CONTROLLER::Build( BOARD* aBoard )
{
    wxLogTrace( tracePnsTrackWidth, wxT( "TRACK_WIDTH_CONTROLLER::Build() - starting" ) );

    m_board = aBoard;
    clear();

    if( !m_board || !m_iface )
    {
        wxLogTrace( tracePnsTrackWidth, wxT( "Build: Missing board or iface" ) );
        return;
    }

    // TODO: Iterate over DRC rules to extract geometry-based width constraints
    // For now, this is a placeholder that logs the availability of the board
    wxLogTrace( tracePnsTrackWidth,
                wxT( "Build: Board has %d nets" ),
                static_cast<int>( m_board->GetNetInfo().GetNetCount() ) );

    // Collect all copper layers
    std::set<int> copperLayers;
    for( PCB_LAYER_ID boardLayer : LSET::AllCuMask().Seq() )
    {
        if( m_iface && m_iface->IsPNSCopperLayer( m_iface->GetPNSLayerFromBoardLayer( boardLayer ) ) )
        {
            copperLayers.insert( m_iface->GetPNSLayerFromBoardLayer( boardLayer ) );
        }
    }

    for( int pnsLayer : copperLayers )
    {
        PCB_LAYER_ID boardLayer = m_iface->GetBoardLayerFromPNSLayer( static_cast<PCB_LAYER_ID>( pnsLayer ) );

        if( boardLayer >= 0 )
            m_layers.emplace_back( boardLayer, pnsLayer );
    }

    wxLogTrace( tracePnsTrackWidth,
                wxT( "Build: Initialized %zu layers" ),
                m_layers.size() );
}


void TRACK_WIDTH_CONTROLLER::addRegion( const KEY& aKey, const std::shared_ptr<SHAPE_POLY_SET>& aShape, int aWidth )
{
    wxLogTrace( tracePnsTrackWidth,
                wxT( "addRegion: Adding width=%d region for layer=%d" ),
                aWidth, aKey.layer );

    auto& tree = m_indices[aKey];

    if( !tree )
        tree = std::make_unique<REGION_INDEX>( aKey.layer );

    auto region = std::make_unique<REGION>( aShape, aWidth );
    REGION* raw = region.get();

    tree->Add( raw );
    m_regions.push_back( std::move( region ) );
}


std::optional<VECTOR2I> TRACK_WIDTH_CONTROLLER::findInteriorPoint( SHAPE_POLY_SET& aPoly ) const
{
    BOX2I bbox = aPoly.BBox();

    if( !bbox.IsValid() )
        return std::nullopt;

    VECTOR2I origin = bbox.GetOrigin();
    int      width = bbox.GetWidth();
    int      height = bbox.GetHeight();

    const int steps = 8;

    for( int ix = 0; ix <= steps; ++ix )
    {
        for( int iy = 0; iy <= steps; ++iy )
        {
            VECTOR2I candidate( origin.x + ( width * ix ) / steps,
                                origin.y + ( height * iy ) / steps );

            if( aPoly.Contains( candidate ) )
                return candidate;
        }
    }

    const SHAPE_LINE_CHAIN& outline = aPoly.COutline( 0 );

    if( outline.PointCount() >= 3 )
    {
        VECTOR2I centroid( 0, 0 );

        for( int i = 0; i < outline.PointCount(); ++i )
            centroid += outline.CPoint( i );

        centroid.x /= outline.PointCount();
        centroid.y /= outline.PointCount();

        if( aPoly.Contains( centroid ) )
            return centroid;
    }

    return std::nullopt;
}


std::optional<VECTOR2I> TRACK_WIDTH_CONTROLLER::findExteriorPoint( const SHAPE_POLY_SET& aPoly ) const
{
    BOX2I bbox = aPoly.BBox();

    if( !bbox.IsValid() )
        return std::nullopt;

    VECTOR2I origin = bbox.GetOrigin();
    int      width = bbox.GetWidth();
    int      height = bbox.GetHeight();

    int stepX = width > 0 ? width : 1;
    int stepY = height > 0 ? height : 1;

    VECTOR2I centre = bbox.Centre();

    std::array<VECTOR2I, 8> candidates = {
        VECTOR2I( origin.x - stepX, centre.y ),
        VECTOR2I( origin.x + width + stepX, centre.y ),
        VECTOR2I( centre.x, origin.y - stepY ),
        VECTOR2I( centre.x, origin.y + height + stepY ),
        VECTOR2I( origin.x - stepX, origin.y - stepY ),
        VECTOR2I( origin.x + width + stepX, origin.y - stepY ),
        VECTOR2I( origin.x - stepX, origin.y + height + stepY ),
        VECTOR2I( origin.x + width + stepX, origin.y + height + stepY )
    };

    for( const VECTOR2I& candidate : candidates )
    {
        if( !aPoly.Contains( candidate ) )
            return candidate;
    }

    return std::nullopt;
}


std::optional<int> TRACK_WIDTH_CONTROLLER::evaluateWidthAtPoint( NET_HANDLE aNet, PCB_LAYER_ID aBoardLayer,
                                                                 const VECTOR2I& aPoint ) const
{
    if( !m_resolver || !m_iface )
    {
        wxLogTrace( tracePnsTrackWidth,
                    wxT( "evaluateWidthAtPoint: Missing resolver or iface" ) );
        return std::nullopt;
    }

    int pnsLayer = m_iface->GetPNSLayerFromBoardLayer( aBoardLayer );

    if( pnsLayer < 0 )
    {
        wxLogTrace( tracePnsTrackWidth,
                    wxT( "evaluateWidthAtPoint: Invalid PNS layer for board layer=%d" ),
                    aBoardLayer );
        return std::nullopt;
    }

    wxLogTrace( tracePnsTrackWidth,
                wxT( "evaluateWidthAtPoint: Evaluating at (%d, %d) on layer=%d" ),
                aPoint.x, aPoint.y, aBoardLayer );

    SEG segmentGeom( aPoint, aPoint + VECTOR2I( 1, 0 ) );
    SEGMENT testSeg( segmentGeom, aNet );

    testSeg.SetLayer( pnsLayer );
    testSeg.SetWidth( 1 );

    CONSTRAINT constraint;

    if( m_resolver->QueryConstraint( CONSTRAINT_TYPE::CT_WIDTH, &testSeg, nullptr, pnsLayer, &constraint ) )
    {
        if( constraint.m_Value.HasOpt() )
        {
            int width = constraint.m_Value.Opt();
            wxLogTrace( tracePnsTrackWidth,
                        wxT( "evaluateWidthAtPoint: Found optimal width=%d" ),
                        width );
            return width;
        }

        if( constraint.m_Value.HasMin() )
        {
            int width = constraint.m_Value.Min();
            wxLogTrace( tracePnsTrackWidth,
                        wxT( "evaluateWidthAtPoint: Found minimum width=%d" ),
                        width );
            return width;
        }

        wxLogTrace( tracePnsTrackWidth,
                    wxT( "evaluateWidthAtPoint: Constraint found but no value" ) );
    }
    else
    {
        wxLogTrace( tracePnsTrackWidth,
                    wxT( "evaluateWidthAtPoint: No constraint found at (%d, %d)" ),
                    aPoint.x, aPoint.y );
    }

    return std::nullopt;
}


std::optional<int> TRACK_WIDTH_CONTROLLER::lookupWidth( const KEY& aKey, const VECTOR2I& aPoint ) const
{
    auto cacheIt = m_lookupCache.find( aKey );

    if( cacheIt != m_lookupCache.end() && cacheIt->second.valid && cacheIt->second.point == aPoint )
    {
        wxLogTrace( tracePnsTrackWidth,
                    wxT( "lookupWidth: Cache hit at (%d, %d), width=%s" ),
                    aPoint.x, aPoint.y,
                    cacheIt->second.width ? wxString::Format( wxT( "%d" ), cacheIt->second.width.value() )
                                           : wxT( "no width" ) );
        return cacheIt->second.width;
    }

    auto it = m_indices.find( aKey );

    std::optional<int> result;

    if( it != m_indices.end() )
    {
        wxLogTrace( tracePnsTrackWidth,
                    wxT( "lookupWidth: Found index for layer=%d at (%d, %d)" ),
                    aKey.layer, aPoint.x, aPoint.y );

        const REGION_INDEX& index = *it->second;

        struct VISITOR
        {
            VECTOR2I              point;
            std::optional<int>    width;

            bool operator()( REGION* aRegion )
            {
                if( aRegion && aRegion->Poly().Contains( point ) )
                {
                    width = aRegion->Width();
                    wxLogTrace( tracePnsTrackWidth,
                                wxT( "lookupWidth: Found region with width=%d" ),
                                width.value() );
                    return false;
                }

                return true;
            }
        } visitor{ aPoint, std::nullopt };

        SHAPE_SEGMENT probe( SEG( aPoint, aPoint ) );
        index.Query( &probe, 0, visitor );

        result = visitor.width;

        if( !result )
        {
            wxLogTrace( tracePnsTrackWidth,
                        wxT( "lookupWidth: No region found at (%d, %d)" ),
                        aPoint.x, aPoint.y );
        }
    }
    else
    {
        wxLogTrace( tracePnsTrackWidth,
                    wxT( "lookupWidth: No index found for layer=%d" ),
                    aKey.layer );
    }

    CACHE_ENTRY& cache = m_lookupCache[aKey];
    cache.point = aPoint;
    cache.width = result;
    cache.valid = true;

    return result;
}


int TRACK_WIDTH_CONTROLLER::ResolveWidth( NET_HANDLE aNet, int aPnsLayer, const VECTOR2I& aPoint,
                                          int aFallback ) const
{
    if( !aNet )
    {
        wxLogTrace( tracePnsTrackWidth,
                    wxT( "ResolveWidth: No net handle provided, using fallback=%d" ),
                    aFallback );
        return aFallback;
    }

    KEY key{ aNet, aPnsLayer };

    wxLogTrace( tracePnsTrackWidth,
                wxT( "ResolveWidth: Starting at (%d, %d) on layer=%d, fallback=%d" ),
                aPoint.x, aPoint.y, aPnsLayer, aFallback );

    if( auto width = lookupWidth( key, aPoint ) )
    {
        wxLogTrace( tracePnsTrackWidth,
                    wxT( "ResolveWidth: Found width from lookup=%d" ),
                    width.value() );
        return width.value();
    }

    if( auto def = DefaultWidth( aNet, aPnsLayer, aPoint ) )
    {
        wxLogTrace( tracePnsTrackWidth,
                    wxT( "ResolveWidth: Found default width=%d" ),
                    def.value() );
        return def.value();
    }

    wxLogTrace( tracePnsTrackWidth,
                wxT( "ResolveWidth: Using fallback=%d" ),
                aFallback );

    return aFallback;
}


std::optional<int> TRACK_WIDTH_CONTROLLER::WidthFor( NET_HANDLE aNet, int aPnsLayer,
                                                     const VECTOR2I& aPoint ) const
{
    if( !aNet )
        return std::nullopt;

    KEY key{ aNet, aPnsLayer };
    return lookupWidth( key, aPoint );
}


std::optional<int> TRACK_WIDTH_CONTROLLER::DefaultWidth( NET_HANDLE aNet, int aPnsLayer,
                                                        const VECTOR2I& aPoint ) const
{
    if( !aNet || !m_iface )
    {
        wxLogTrace( tracePnsTrackWidth,
                    wxT( "DefaultWidth: Missing net or iface" ) );
        return std::nullopt;
    }

    KEY key{ aNet, aPnsLayer };

    auto it = m_defaultWidths.find( key );

    if( it != m_defaultWidths.end() )
    {
        wxLogTrace( tracePnsTrackWidth,
                    wxT( "DefaultWidth: Found cached default width=%d" ),
                    it->second );
        return it->second;
    }

    wxLogTrace( tracePnsTrackWidth,
                wxT( "DefaultWidth: No cached width, evaluating at (%d, %d) layer=%d" ),
                aPoint.x, aPoint.y, aPnsLayer );

    PCB_LAYER_ID boardLayer = m_iface->GetBoardLayerFromPNSLayer( static_cast<PCB_LAYER_ID>( aPnsLayer ) );

    if( boardLayer < 0 )
    {
        wxLogTrace( tracePnsTrackWidth,
                    wxT( "DefaultWidth: Invalid board layer for PNS layer=%d" ),
                    aPnsLayer );
        return std::nullopt;
    }

    std::optional<int> width = evaluateWidthAtPoint( aNet, boardLayer, aPoint );

    if( width )
    {
        wxLogTrace( tracePnsTrackWidth,
                    wxT( "DefaultWidth: Caching evaluated width=%d for future use" ),
                    width.value() );
        m_defaultWidths[key] = width.value();
    }
    else
    {
        wxLogTrace( tracePnsTrackWidth,
                    wxT( "DefaultWidth: No width found at (%d, %d)" ),
                    aPoint.x, aPoint.y );
    }

    return width;
}


bool TRACK_WIDTH_CONTROLLER::HasGeometry( NET_HANDLE aNet, int aPnsLayer ) const
{
    KEY key{ aNet, aPnsLayer };

    return m_indices.find( key ) != m_indices.end();
}

} // namespace PNS
