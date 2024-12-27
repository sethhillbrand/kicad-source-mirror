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

#ifndef DRC_RE_ROUTING_DIFF_PAIR_CONSTRAINT_DATA_H_
#define DRC_RE_ROUTING_DIFF_PAIR_CONSTRAINT_DATA_H_

#include "drc_re_base_constraint_data.h"


class DrcReRoutingDiffPairConstraintData : public DrcReBaseConstraintData
{
public:
    DrcReRoutingDiffPairConstraintData() = default;

    explicit DrcReRoutingDiffPairConstraintData( const DrcReBaseConstraintData& baseData ) :
            DrcReBaseConstraintData( baseData ), 
            m_maxUncoupledLength( 0 ),
            m_minWidth( 0 ),
            m_preferredWidth( 0 ),
            m_maxWidth( 0 ),
            m_minGap( 0 ),
            m_preferredGap( 0 ),
            m_maxGap( 0 )
    {
    }

    explicit DrcReRoutingDiffPairConstraintData( unsigned int aId, unsigned int aParentId, wxString aRuleName,
                                                 double aMaxUncoupledLength,
                                                 double aMinWidth,
                                                 double aPreferredWidth,
                                                 double aMaxWidth,
                                                 double aMinGap,
                                                 double aPreferredGap,
                                                 double aMaxGap ) :
            DrcReBaseConstraintData( aId, aParentId, aRuleName ),
            m_maxUncoupledLength( aMaxUncoupledLength ),
            m_minWidth( aMinWidth ),
            m_preferredWidth( aPreferredWidth ),
            m_maxWidth( aMaxWidth ),
            m_minGap( aMinGap ),
            m_preferredGap( aPreferredGap ),
            m_maxGap( aMaxGap )
    {
    }

    virtual ~DrcReRoutingDiffPairConstraintData() = default;

    double GetMaxUncoupledLength() { return m_maxUncoupledLength; }
    void   SetMaxUncoupledLength( double aMaxUncoupledLength ) { m_maxUncoupledLength = aMaxUncoupledLength; }

    double GetMinWidth() { return m_minWidth; }
    void   SetMinWidth( double aMinWidth ) { m_minWidth = aMinWidth; }

    double GetPreferredWidth() { return m_preferredWidth; }
    void   SetPreferredWidth( double aPreferredWidth ) { m_preferredWidth = aPreferredWidth; }

    double GetMaxWidth() { return m_maxWidth; }
    void   SetMaxWidth( double aMaxWidth ) { m_maxWidth = aMaxWidth; }

    double GetMinGap() { return m_minGap; }
    void   SetMinGap( double aMinGap ) { m_minGap = aMinGap; }

    double GetPreferredGap() { return m_preferredGap; }
    void   SetPreferredGap( double aPreferredGap ) { m_preferredGap = aPreferredGap; }

    double GetMaxGap() { return m_maxGap; }
    void   SetMaxGap( double aMaxGap ) { m_maxGap = aMaxGap; }

private:
    double m_maxUncoupledLength;
    double m_minWidth;
    double m_preferredWidth;
    double m_maxWidth;
    double m_minGap;
    double m_preferredGap;
    double m_maxGap;
};

#endif // DRC_RE_ROUTING_DIFF_PAIR_CONSTRAINT_DATA_H_