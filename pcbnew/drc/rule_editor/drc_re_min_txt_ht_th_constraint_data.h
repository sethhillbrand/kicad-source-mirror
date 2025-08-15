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

#ifndef DRC_RE_MINIMUM_TEXT_HEIGHT_THICKNESS_CONSTRAINT_DATA_H_
#define DRC_RE_MINIMUM_TEXT_HEIGHT_THICKNESS_CONSTRAINT_DATA_H_

#include "drc_re_base_constraint_data.h"


class DrcReMinimumTextHeightThicknessConstraintData : public DrcReBaseConstraintData
{
public:
    DrcReMinimumTextHeightThicknessConstraintData() = default;

    explicit DrcReMinimumTextHeightThicknessConstraintData( const DrcReBaseConstraintData& baseData ) :
            DrcReBaseConstraintData( baseData ), 
            m_minTextHeight( 0 ),
            m_minTextThickness( 0 )
    {
    }

    explicit DrcReMinimumTextHeightThicknessConstraintData( unsigned int aId, unsigned int aParentId, wxString aRuleName,
                                                            double aMinTextHeight,
                                                            double aMinTextThickness ) :
            DrcReBaseConstraintData( aId, aParentId, aRuleName ),
            m_minTextHeight( aMinTextHeight ),
            m_minTextThickness( aMinTextThickness )
    {
    }

    virtual ~DrcReMinimumTextHeightThicknessConstraintData() = default;      

    double GetMinTextHeight() { return m_minTextHeight; }
    void   SetMinTextHeight( double aMinTextHeight ) { m_minTextHeight = aMinTextHeight; }

    double GetMinTextThickness() { return m_minTextThickness; }
    void   SetMinTextThickness( double aMinTextThickness ) { m_minTextThickness = aMinTextThickness; }

private:
    double m_minTextHeight;
    double m_minTextThickness;
};

#endif // DRC_RE_MINIMUM_TEXT_HEIGHT_THICKNESS_CONSTRAINT_DATA_H_