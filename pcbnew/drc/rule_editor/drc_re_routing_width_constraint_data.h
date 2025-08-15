/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2024 KiCad Developers, see AUTHORS.txt for contributors.
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

#ifndef DRC_RE_ROUTING_WIDTH_CONSTRAINT_DATA_H_
#define DRC_RE_ROUTING_WIDTH_CONSTRAINT_DATA_H_

#include "drc_re_base_constraint_data.h"


class DRC_RE_ROUTING_WIDTH_CONSTRAINT_DATA : public DRC_RE_BASE_CONSTRAINT_DATA
{
public:
    DRC_RE_ROUTING_WIDTH_CONSTRAINT_DATA() = default;

    explicit DRC_RE_ROUTING_WIDTH_CONSTRAINT_DATA( const DRC_RE_BASE_CONSTRAINT_DATA& aBaseData ) :
            DRC_RE_BASE_CONSTRAINT_DATA( aBaseData )
    {
    }

    explicit DRC_RE_ROUTING_WIDTH_CONSTRAINT_DATA( int aId, int aParentId, wxString aRuleName,
                                                   double aMinRoutingWidth,
                                                   double aPreferredRoutingWidth,
                                                   double aMaxRoutingWidth ) :
            DRC_RE_BASE_CONSTRAINT_DATA( aId, aParentId, aRuleName ),
            m_minRoutingWidth( aMinRoutingWidth ),
            m_preferredRoutingWidth( aPreferredRoutingWidth ), m_maxRoutingWidth( aMaxRoutingWidth )
    {
    }

    virtual ~DRC_RE_ROUTING_WIDTH_CONSTRAINT_DATA() = default;

    double GetMinRoutingWidth() { return m_minRoutingWidth; }

    void SetMinRoutingWidth( double aMinRoutingWidth ) { m_minRoutingWidth = aMinRoutingWidth; }

    double GetPreferredRoutingWidth() { return m_preferredRoutingWidth; }

    void SetPreferredRoutingWidth( double aPreferredRoutingWidth )
    {
        m_preferredRoutingWidth = aPreferredRoutingWidth;
    }

    double GetMaxRoutingWidth() { return m_maxRoutingWidth; }

    void SetMaxRoutingWidth( double aMaxRoutingWidth ) { m_maxRoutingWidth = aMaxRoutingWidth; }

    void CopyFrom( const ICopyable& aSource ) override
    {
        const auto& source = dynamic_cast<const DRC_RE_ROUTING_WIDTH_CONSTRAINT_DATA&>( aSource );

        DRC_RE_BASE_CONSTRAINT_DATA::CopyFrom( source );

        m_minRoutingWidth = source.m_minRoutingWidth;
        m_preferredRoutingWidth = source.m_preferredRoutingWidth;
        m_maxRoutingWidth = source.m_maxRoutingWidth;
    }

private:
    double m_minRoutingWidth{ 0 };
    double m_preferredRoutingWidth{ 0 };
    double m_maxRoutingWidth{ 0 };
};

#endif // DRC_RE_ROUTING_WIDTH_CONSTRAINT_DATA_H_