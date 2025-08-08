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

#ifndef DRC_RE_BASE_CONSTRAINT_DATA_H_
#define DRC_RE_BASE_CONSTRAINT_DATA_H_

#include <dialogs/rule_editor_data_base.h>


class DRC_RE_BASE_CONSTRAINT_DATA : public RULE_EDITOR_DATA_BASE
{
public:
    DRC_RE_BASE_CONSTRAINT_DATA() = default;

    explicit DRC_RE_BASE_CONSTRAINT_DATA( int aId, int aParentId, wxString aRuleName ) :
            RULE_EDITOR_DATA_BASE( aId, aParentId, aRuleName )
    {
    }

    virtual ~DRC_RE_BASE_CONSTRAINT_DATA() = default;

    std::vector<PCB_LAYER_ID> GetLayers() { return m_layers; }

    void SetLayers( std::vector<PCB_LAYER_ID> aLayers ) { m_layers = aLayers; }

    wxString GetRuleCondition() { return m_ruleCondition; }

    void SetRuleCondition( wxString aRuleCondition ) { m_ruleCondition = aRuleCondition; }

    wxString GetConstraintCode() { return m_constraintCode; }

    void SetConstraintCode( wxString aCode ) { m_constraintCode = aCode; }

    void CopyFrom( const ICopyable& aSource ) override
    {
        const auto& source = dynamic_cast<const DRC_RE_BASE_CONSTRAINT_DATA&>( aSource );

        RULE_EDITOR_DATA_BASE::CopyFrom( source );

        m_layers = source.m_layers;
        m_constraintCode = source.m_constraintCode;
    }

private:
    std::vector<PCB_LAYER_ID> m_layers;
    wxString m_ruleCondition;
    wxString m_constraintCode;
};

#endif // DRC_RE_BASE_CONSTRAINT_DATA_H_