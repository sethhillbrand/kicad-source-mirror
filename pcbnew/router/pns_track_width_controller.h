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

#ifndef PNS_TRACK_WIDTH_CONTROLLER_H
#define PNS_TRACK_WIDTH_CONTROLLER_H

#include <map>
#include <memory>
#include <optional>
#include <vector>

#include <geometry/shape_poly_set.h>
#include <geometry/shape_segment.h>
#include <geometry/shape_index.h>

#include <layer_ids.h>

#include "pns_item.h"

class BOARD;

namespace PNS
{

class ROUTER_IFACE;
class RULE_RESOLVER;
class NODE;

/**
 * TRACK_WIDTH_CONTROLLER builds spatial indices describing the parts of the board where
 * width constraints change for a given net/layer pair.  Placer implementations consult the
 * controller to automatically adapt the routed track width while the user moves the cursor.
 */
class TRACK_WIDTH_CONTROLLER
{
public:
    TRACK_WIDTH_CONTROLLER();

    void Initialize( ROUTER_IFACE* aIface, RULE_RESOLVER* aResolver );

    void Build( BOARD* aBoard );

    int ResolveWidth( NET_HANDLE aNet, int aPnsLayer, const VECTOR2I& aPoint, int aFallback ) const;

    std::optional<int> WidthFor( NET_HANDLE aNet, int aPnsLayer, const VECTOR2I& aPoint ) const;

    std::optional<int> DefaultWidth( NET_HANDLE aNet, int aPnsLayer, const VECTOR2I& aPoint ) const;

    bool HasGeometry( NET_HANDLE aNet, int aPnsLayer ) const;

private:
    struct KEY
    {
        NET_HANDLE net = nullptr;
        int        layer = -1;

        bool operator<( const KEY& aOther ) const
        {
            if( net != aOther.net )
                return reinterpret_cast<uintptr_t>( net )
                        < reinterpret_cast<uintptr_t>( aOther.net );

            return layer < aOther.layer;
        }
    };

    class REGION
    {
    public:
        REGION( const std::shared_ptr<SHAPE_POLY_SET>& aShape, int aWidth );

        const SHAPE* Shape( int ) const { return m_shape.get(); }

        const SHAPE_POLY_SET& Poly() const { return *m_shape; }

        int Width() const { return m_width; }

    private:
        std::shared_ptr<SHAPE_POLY_SET> m_shape;
        int                             m_width;
    };

    using REGION_INDEX = SHAPE_INDEX<REGION*>;

private:
    void clear();

    std::optional<VECTOR2I> findInteriorPoint( SHAPE_POLY_SET& aPoly ) const;
    std::optional<VECTOR2I> findExteriorPoint( const SHAPE_POLY_SET& aPoly ) const;

    std::optional<int> evaluateWidthAtPoint( NET_HANDLE aNet, PCB_LAYER_ID aBoardLayer,
                                             const VECTOR2I& aPoint ) const;

    std::optional<int> lookupWidth( const KEY& aKey, const VECTOR2I& aPoint ) const;

    void addRegion( const KEY& aKey, const std::shared_ptr<SHAPE_POLY_SET>& aShape, int aWidth );

private:
    ROUTER_IFACE*  m_iface;
    RULE_RESOLVER* m_resolver;
    BOARD*         m_board;

    struct CACHE_ENTRY
    {
        VECTOR2I              point;
        std::optional<int>    width;
        bool                  valid = false;
    };

    std::map<KEY, std::unique_ptr<REGION_INDEX>> m_indices;
    std::vector<std::unique_ptr<REGION>>          m_regions;
    mutable std::map<KEY, int>                    m_defaultWidths;
    std::vector<std::pair<PCB_LAYER_ID, int>>     m_layers;
    mutable std::map<KEY, CACHE_ENTRY>            m_lookupCache;
};

} // namespace PNS

#endif
