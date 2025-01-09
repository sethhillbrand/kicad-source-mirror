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

#ifndef DRC_RE_ALLOWED_ORIENTATION_CONSTRAINT_DATA_H_
#define DRC_RE_ALLOWED_ORIENTATION_CONSTRAINT_DATA_H_

#include "drc_re_base_constraint_data.h"


class DrcReAllowedOrientationConstraintData : public DrcReBaseConstraintData
{
public:
    DrcReAllowedOrientationConstraintData() = default;

    explicit DrcReAllowedOrientationConstraintData( const DrcReBaseConstraintData& baseData ) :
            DrcReBaseConstraintData( baseData )
    {
    }

    explicit DrcReAllowedOrientationConstraintData( unsigned int aId, unsigned int aParentId,
                                                    bool aAllowZeroDegreess,
                                                    bool aAllowNintyDegreess,
                                                    bool aAllowOneEightyDegreess,
                                                    bool aAllowTwoSeventyDegreess,
                                                    bool aAllowAllDegreess,
                                                    wxString aRuleName ) :
            DrcReBaseConstraintData( aId, aParentId, aRuleName ),
            m_allowZeroDegreess( aAllowZeroDegreess ),
            m_allowNintyDegreess( aAllowNintyDegreess ),
            m_allowOneEightyDegreess( aAllowOneEightyDegreess ),
            m_allowTwoSeventyDegreess( aAllowTwoSeventyDegreess ),
            m_allowAllDegreess( aAllowAllDegreess )
    {
    }

    virtual ~DrcReAllowedOrientationConstraintData() = default;

    bool GetIsZeroDegressAllowed() { return m_allowZeroDegreess; }
    void SetIsZeroDegressAllowed( double aAllowZeroDegreess ) { m_allowZeroDegreess = aAllowZeroDegreess; }

    bool GetIsNintyDegressAllowed() { return m_allowNintyDegreess; }
    void SetIsNintyDegressAllowed( double aAllowNintyDegreess ) { m_allowNintyDegreess = aAllowNintyDegreess; }

    bool GetIsOneEightyDegressAllowed() { return m_allowOneEightyDegreess; }
    void SetIsOneEightyDegressAllowed( double aAllowOneEightyDegreess ) { m_allowOneEightyDegreess = aAllowOneEightyDegreess; }

    bool GetIsTwoSeventyDegressAllowed() { return m_allowTwoSeventyDegreess; }
    void SetIsTwoSeventyDegressAllowed( double aAllowTwoSeventyDegreess ) { m_allowTwoSeventyDegreess = aAllowTwoSeventyDegreess; }

    bool GetIsAllDegressAllowed() { return m_allowAllDegreess; }
    void SetIsAllDegressAllowed( double aAllowAllDegreess ) { m_allowAllDegreess = aAllowAllDegreess; }

    void CopyFrom( const ICopyable& source ) override
    {
        const auto& viaSource = dynamic_cast<const DrcReAllowedOrientationConstraintData&>( source );

        // Call base class method
        DrcReBaseConstraintData::CopyFrom( viaSource );

        // Copy via-specific data
        m_allowZeroDegreess = viaSource.m_allowZeroDegreess;
        m_allowNintyDegreess = viaSource.m_allowNintyDegreess;
        m_allowOneEightyDegreess = viaSource.m_allowOneEightyDegreess;
        m_allowTwoSeventyDegreess = viaSource.m_allowTwoSeventyDegreess;
        m_allowAllDegreess = viaSource.m_allowAllDegreess;
    }

private:
    bool m_allowZeroDegreess{ false };
    bool m_allowNintyDegreess{ false };
    bool m_allowOneEightyDegreess{ false };
    bool m_allowTwoSeventyDegreess{ false };
    bool m_allowAllDegreess{ false };
};


#endif // DRC_RE_ALLOWED_ORIENTATION_CONSTRAINT_DATA_H_