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

#ifndef DRC_RE_SMD_ENTRY_CONSTRAINT_DATA_H_
#define DRC_RE_SMD_ENTRY_CONSTRAINT_DATA_H_

#include "drc_re_base_constraint_data.h"


class DrcReSmdEntryConstraintData : public DrcReBaseConstraintData
{
public:
    DrcReSmdEntryConstraintData() = default;

    explicit DrcReSmdEntryConstraintData( const DrcReBaseConstraintData& baseData ) :
            DrcReBaseConstraintData( baseData )
    {
    }

    explicit DrcReSmdEntryConstraintData( unsigned int aId, unsigned int aParentId, wxString aRuleName,
                                          bool aSideAngle, 
                                          bool aCornerAngle, 
                                          bool aAnyAngle ) :
            DrcReBaseConstraintData( aId, aParentId, aRuleName ),
            m_sideAngle( aSideAngle ),
            m_cornerAngle( aCornerAngle ), 
            m_anyAngle( aAnyAngle )
    {
    }

    virtual ~DrcReSmdEntryConstraintData() = default;

    bool GetIsSideAngleEnabled() { return m_sideAngle; }
    void SetIsSideAngleEnabled( double aSideAngle ) { m_sideAngle = aSideAngle; }

    bool GetIsCornerAngleEnabled() { return m_cornerAngle; }
    void SetIsCornerAngleEnabled( double aCornerAngle ) { m_cornerAngle = aCornerAngle; }

    bool GetIsAnyAngleEnabled() { return m_anyAngle; }
    void SetIsAnyAngleEnabled( double aAnyAngle ) { m_anyAngle = aAnyAngle; }

    void CopyFrom( const ICopyable& source ) override
    {
        const auto& viaSource = dynamic_cast<const DrcReSmdEntryConstraintData&>( source );

        // Call base class method
        DrcReBaseConstraintData::CopyFrom( viaSource );

        // Copy via-specific data
        m_sideAngle = viaSource.m_sideAngle;
        m_cornerAngle = viaSource.m_cornerAngle;
        m_anyAngle = viaSource.m_anyAngle;
    }

private:
    bool m_sideAngle{ false };
    bool m_cornerAngle{ false };
    bool m_anyAngle{ false };
};


#endif // DRC_RE_SMD_ENTRY_CONSTRAINT_DATA_H_