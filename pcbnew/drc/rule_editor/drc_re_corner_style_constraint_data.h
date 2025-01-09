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

#ifndef DRC_RE_CORNER_STYLE_CONSTRAINT_DATA_H_
#define DRC_RE_CORNER_STYLE_CONSTRAINT_DATA_H_

#include "drc_re_base_constraint_data.h"


class DrcReCornerStyleConstraintData : public DrcReBaseConstraintData
{
public:
    DrcReCornerStyleConstraintData() = default;

    explicit DrcReCornerStyleConstraintData( const DrcReBaseConstraintData& baseData ) :
            DrcReBaseConstraintData( baseData )
    {
    }

    explicit DrcReCornerStyleConstraintData( unsigned int aId, unsigned int aParentId, wxString& aRuleName,
                                             wxString& aCornerStyle, 
                                             double& aMinSetbackLength, 
                                             double& aMaxSetbackLength ) :
            DrcReBaseConstraintData( aId, aParentId, aRuleName ),
            m_cornerStyle( aCornerStyle ),
            m_minSetbackLength( aMinSetbackLength ),
            m_maxSetbackLength( aMaxSetbackLength )
    {
    }

    virtual ~DrcReCornerStyleConstraintData() = default;

    wxString GetCornerStyle() { return m_cornerStyle; }
    void     SetCornerStyle( wxString aCornerStyle ) { m_cornerStyle = aCornerStyle; }

    double GetMinSetbackLength() { return m_minSetbackLength; }
    void   SetMinSetbackLength( double aMinSetbackLength ) { m_minSetbackLength = aMinSetbackLength; }

    double GetMaxSetbackLength() { return m_maxSetbackLength; }
    void   SetMaxSetbackLength( double aMaxSetbackLength ) { m_maxSetbackLength = aMaxSetbackLength; }

    void CopyFrom( const ICopyable& source ) override
    {
        const auto& viaSource = dynamic_cast<const DrcReCornerStyleConstraintData&>( source );

        // Call base class method
        DrcReBaseConstraintData::CopyFrom( viaSource );

        // Copy via-specific data
        m_minSetbackLength = viaSource.m_minSetbackLength;
        m_maxSetbackLength = viaSource.m_maxSetbackLength;
    }

private:
    wxString m_cornerStyle{ wxEmptyString };
    double   m_minSetbackLength{ 0 };
    double   m_maxSetbackLength{ 0 };
};

#endif // DRC_RE_CORNER_STYLE_CONSTRAINT_DATA_H_