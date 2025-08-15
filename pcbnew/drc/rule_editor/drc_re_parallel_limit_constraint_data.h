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

#ifndef DRC_RE_PARALLEL_LIMIT_CONSTRAINT_DATA_H_
#define DRC_RE_PARALLEL_LIMIT_CONSTRAINT_DATA_H_

#include "drc_re_base_constraint_data.h"


class DRC_RE_PARALLEL_LIMIT_CONSTRAINT_DATA : public DRC_RE_BASE_CONSTRAINT_DATA
{
public:
    DRC_RE_PARALLEL_LIMIT_CONSTRAINT_DATA() = default;

    explicit DRC_RE_PARALLEL_LIMIT_CONSTRAINT_DATA( const DRC_RE_BASE_CONSTRAINT_DATA& aBaseData ) :
            DRC_RE_BASE_CONSTRAINT_DATA( aBaseData )
    {
    }

    explicit DRC_RE_PARALLEL_LIMIT_CONSTRAINT_DATA( int aId, int aParentId, wxString aRuleName,
                                                    double aParallelLimit, double aParallelGap ) :
            DRC_RE_BASE_CONSTRAINT_DATA( aId, aParentId, aRuleName ),
            m_parallelGap( aParallelGap ), m_parallelLimit( aParallelLimit )
    {
    }

    virtual ~DRC_RE_PARALLEL_LIMIT_CONSTRAINT_DATA() = default;

    double GetParallelLimit() { return m_parallelLimit; }

    void SetParallelLimit( double aParallelLimit ) { m_parallelLimit = aParallelLimit; }

    double GetParallelGap() { return m_parallelGap; }

    void SetParallelGap( double aParallelGap ) { m_parallelGap = aParallelGap; }

    void CopyFrom( const ICopyable& aSource ) override
    {
        const auto& source = dynamic_cast<const DRC_RE_PARALLEL_LIMIT_CONSTRAINT_DATA&>( aSource );

        DRC_RE_BASE_CONSTRAINT_DATA::CopyFrom( source );

        m_parallelGap = source.m_parallelGap;
        m_parallelLimit = source.m_parallelLimit;
    }

private:
    double m_parallelGap{ 0 };
    double m_parallelLimit{ 0 };
};

#endif // DRC_RE_PARALLEL_LIMIT_CONSTRAINT_DATA_H_