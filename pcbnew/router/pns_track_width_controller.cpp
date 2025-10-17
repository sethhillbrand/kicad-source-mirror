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
#include <string>

#include <geometry/shape_simple.h>
#include <trace_helpers.h>
#include <board.h>
#include <footprint.h>
#include <zone.h>
#include <kiid.h>
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

    std::shared_ptr<DRC_ENGINE> drcEngine = m_board->GetDesignSettings().m_DRCEngine;

    if( drcEngine && drcEngine->HasRulesForConstraintType( TRACK_WIDTH_CONSTRAINT ) )
    {
        struct PROCESSED_KEY
        {
            NET_HANDLE   net;
            int          layer;
            const ZONE*  zone;

            bool operator<( const PROCESSED_KEY& aOther ) const
            {
                if( net != aOther.net )
                    return reinterpret_cast<uintptr_t>( net )
                           < reinterpret_cast<uintptr_t>( aOther.net );

                if( layer != aOther.layer )
                    return layer < aOther.layer;

                return zone < aOther.zone;
            }
        };

        struct ZONE_GEOMETRY
        {
            std::shared_ptr<SHAPE_POLY_SET> interiorShape;
            std::optional<VECTOR2I>         interiorPoint;
            std::optional<VECTOR2I>         exteriorPoint;
        };

        const NETCODES_MAP& netsByCode = m_board->GetNetInfo().NetsByNetcode();
        std::vector<NETINFO_ITEM*>       netItems;
        netItems.reserve( netsByCode.size() );

        for( const auto& entry : netsByCode )
        {
            if( entry.second )
                netItems.push_back( entry.second );
        }

        std::vector<ZONE*> ruleAreas;
        ruleAreas.reserve( m_board->Zones().size() );

        for( ZONE* zone : m_board->Zones() )
        {
            if( zone && zone->GetIsRuleArea() )
                ruleAreas.push_back( zone );
        }

        for( FOOTPRINT* footprint : m_board->Footprints() )
        {
            for( ZONE* zone : footprint->Zones() )
            {
                if( zone && zone->GetIsRuleArea() )
                    ruleAreas.push_back( zone );
            }
        }

        std::map<const ZONE*, ZONE_GEOMETRY> zoneGeometryCache;
        auto ensureGeometry =
                [&]( ZONE* aZone ) -> ZONE_GEOMETRY*
                {
                    auto geomIt = zoneGeometryCache.find( aZone );

                    if( geomIt != zoneGeometryCache.end() )
                        return &geomIt->second;

                    ZONE_GEOMETRY geom;
                    geom.interiorShape = std::make_shared<SHAPE_POLY_SET>( *aZone->Outline() );

                    if( !geom.interiorShape || geom.interiorShape->OutlineCount() == 0 )
                        return nullptr;

                    geom.interiorPoint = findInteriorPoint( *geom.interiorShape );

                    if( !geom.interiorPoint )
                        return nullptr;

                    geom.exteriorPoint = findExteriorPoint( *geom.interiorShape );
                    auto [iter, inserted] = zoneGeometryCache.emplace( aZone, std::move( geom ) );
                    (void) inserted;
                    return &iter->second;
                };

        auto extractAreaIdentifiers =
                []( const wxString& aExpression ) -> std::vector<wxString>
                {
                    std::vector<wxString> identifiers;

                    wxString exprLower = aExpression.Lower();
                    std::string exprLowerStd = exprLower.ToStdString();
                    size_t searchPos = 0;

                    while( true )
                    {
                        size_t intersectsPos = exprLowerStd.find( "intersectsarea", searchPos );
                        size_t insidePos = exprLowerStd.find( "insidearea", searchPos );
                        size_t callPos = std::string::npos;

                        if( intersectsPos != std::string::npos
                                && ( insidePos == std::string::npos || intersectsPos < insidePos ) )
                        {
                            callPos = intersectsPos;
                        }
                        else if( insidePos != std::string::npos )
                        {
                            callPos = insidePos;
                        }

                        if( callPos == std::string::npos )
                            break;

                        size_t openParen = exprLowerStd.find( '(', callPos );

                        if( openParen == std::string::npos )
                        {
                            searchPos = callPos + 1;
                            continue;
                        }

                        size_t firstQuote = exprLowerStd.find_first_of( "\"'", openParen + 1 );

                        if( firstQuote == std::string::npos )
                        {
                            searchPos = openParen + 1;
                            continue;
                        }

                        char quote = exprLowerStd[firstQuote];
                        size_t closeQuote = exprLowerStd.find( quote, firstQuote + 1 );

                        if( closeQuote == std::string::npos )
                        {
                            searchPos = firstQuote + 1;
                            continue;
                        }

                        size_t start = firstQuote + 1;
                        size_t length = closeQuote - start;
                        identifiers.emplace_back( aExpression.Mid( start, length ) );
                        searchPos = closeQuote + 1;
                    }

                    return identifiers;
                };

        auto zonesForIdentifier =
                [&]( const wxString& aIdentifier ) -> std::vector<ZONE*>
                {
                    std::vector<ZONE*> matches;

                    if( aIdentifier.IsEmpty() )
                        return matches;

                    bool looksUuid = KIID::SniffTest( aIdentifier );
                    KIID uuid;

                    if( looksUuid )
                        uuid = KIID( aIdentifier );

                    for( ZONE* zone : ruleAreas )
                    {
                        if( looksUuid )
                        {
                            if( zone->m_Uuid == uuid )
                                matches.push_back( zone );
                        }
                        else if( zone->GetZoneName().CmpNoCase( aIdentifier ) == 0 )
                        {
                            matches.push_back( zone );
                        }
                    }

                    return matches;
                };

        std::set<PROCESSED_KEY> processedRegions;
        std::map<KEY, std::optional<int>> baselineCache;

        auto baselineWidthFor =
                [&]( NET_HANDLE aNet, PCB_LAYER_ID aLayer, int aPnsLayer,
                     const std::optional<VECTOR2I>& aPoint ) -> std::optional<int>
                {
                    KEY key{ aNet, aPnsLayer };
                    auto cacheIt = baselineCache.find( key );

                    if( cacheIt != baselineCache.end() )
                        return cacheIt->second;

                    std::optional<int> width;

                    if( aPoint )
                        width = evaluateWidthAtPoint( aNet, aLayer, *aPoint );

                    if( width && !m_defaultWidths.count( key ) )
                        m_defaultWidths[key] = *width;

                    baselineCache.emplace( key, width );
                    return width;
                };

        size_t regionsBefore = m_regions.size();

        for( const std::shared_ptr<DRC_RULE>& rule : drcEngine->GetRules() )
        {
            if( !rule || !rule->m_Condition )
                continue;

            std::vector<wxString> areaIds = extractAreaIdentifiers( rule->m_Condition->GetExpression() );

            if( areaIds.empty() )
                continue;

            for( const DRC_CONSTRAINT& constraint : rule->m_Constraints )
            {
                if( constraint.m_Type != TRACK_WIDTH_CONSTRAINT )
                    continue;

                std::optional<int> constraintWidth;

                if( constraint.m_Value.HasOpt() )
                    constraintWidth = constraint.m_Value.Opt();
                else if( constraint.m_Value.HasMin() )
                    constraintWidth = constraint.m_Value.Min();

                if( !constraintWidth || *constraintWidth <= 0 )
                    continue;

                std::set<ZONE*> matchedZones;

                for( const wxString& areaId : areaIds )
                {
                    for( ZONE* zone : zonesForIdentifier( areaId ) )
                    {
                        if( zone )
                            matchedZones.insert( zone );
                    }
                }

                if( matchedZones.empty() )
                {
                    wxLogTrace( tracePnsTrackWidth,
                                wxT( "Build: Rule '%s' referenced area(s) but matches no zones" ),
                                rule->m_Name );
                    continue;
                }

                for( ZONE* zone : matchedZones )
                {
                    ZONE_GEOMETRY* geom = ensureGeometry( zone );

                    if( !geom || !geom->interiorPoint )
                        continue;

                    std::shared_ptr<SHAPE_POLY_SET> shape = geom->interiorShape;
                    LSET candidateLayers = zone->GetLayerSet();

                    if( rule->m_LayerCondition.any() )
                        candidateLayers &= rule->m_LayerCondition;

                    for( PCB_LAYER_ID boardLayer : candidateLayers.Seq() )
                    {
                        int pnsLayer = m_iface->GetPNSLayerFromBoardLayer( boardLayer );

                        if( pnsLayer < 0
                                || !m_iface->IsPNSCopperLayer(
                                            static_cast<PCB_LAYER_ID>( pnsLayer ) ) )
                        {
                            continue;
                        }

                        for( NETINFO_ITEM* netItem : netItems )
                        {
                            if( !netItem )
                                continue;

                            NET_HANDLE netHandle = static_cast<NET_HANDLE>( netItem );

                            PROCESSED_KEY regionKey{ netHandle, pnsLayer, zone };

                            if( processedRegions.count( regionKey ) )
                                continue;

                            std::optional<int> insideWidth =
                                    evaluateWidthAtPoint( netHandle, boardLayer, *geom->interiorPoint );

                            if( !insideWidth || insideWidth.value() != constraintWidth.value() )
                                continue;

                            std::optional<int> baseline =
                                    baselineWidthFor( netHandle, boardLayer, pnsLayer, geom->exteriorPoint );

                            if( baseline && baseline.value() == insideWidth.value() )
                                continue;

                            addRegion( KEY{ netHandle, pnsLayer }, shape, insideWidth.value() );
                            processedRegions.insert( regionKey );

                            wxLogTrace( tracePnsTrackWidth,
                                        wxT( "Build: Added width region=%d for net='%s' layer=%d" ),
                                        insideWidth.value(), netItem->GetNetname().c_str(),
                                        boardLayer );
                        }
                    }
                }
            }
        }

        wxLogTrace( tracePnsTrackWidth,
                    wxT( "Build: Created %zu width regions from DRC rules" ),
                    m_regions.size() - regionsBefore );
    }
    else
    {
        wxLogTrace( tracePnsTrackWidth,
                    wxT( "Build: No DRC engine or width constraints available" ) );
    }

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
