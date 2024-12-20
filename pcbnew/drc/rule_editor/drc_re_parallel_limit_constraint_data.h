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


class DrcReParallelLimitConstraintData : public DrcReBaseConstraintData
{
public:
    DrcReParallelLimitConstraintData() = default;

    explicit DrcReParallelLimitConstraintData( unsigned int aId, unsigned int aParentId,
                                               double aClearanceValue, wxString aRuleName) :
            DrcReBaseConstraintData( aId, aParentId, aRuleName ),
            m_parallelLimit( aClearanceValue )
    {
    }

    virtual ~DrcReParallelLimitConstraintData() = default;

    double GetParallelLimit() { return m_parallelLimit; }
    void   SetParallelLimit( double aParallelLimit ) { m_parallelLimit = aParallelLimit; }

private:
    double m_parallelLimit;
};


#endif // DRC_RE_PARALLEL_LIMIT_CONSTRAINT_DATA_H_