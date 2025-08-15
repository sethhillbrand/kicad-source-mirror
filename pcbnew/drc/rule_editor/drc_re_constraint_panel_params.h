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


#ifndef DRC_RE_CONSTRAINT_PANEL_PARAMS_H_
#define DRC_RE_CONSTRAINT_PANEL_PARAMS_H_

#include <wx/string.h>
#include "drc_rule_editor_enums.h"
#include "drc_re_numeric_input_constraint_data.h"
#include "drc_re_bool_input_constraint_data.h"

class DrcReConstraintPanelParams
{
public:

    DrcReConstraintPanelParams(){}

    // Constructor for easy initialization
    DrcReConstraintPanelParams( const wxString& aConstraintTitle,
                                const DRC_RULE_EDITOR_CONSTRAINT_NAME& aConstraintType ) :
            m_constraintTitle( aConstraintTitle ), 
            m_constraintType( aConstraintType )
    {
    }

    wxString m_constraintTitle;
    DRC_RULE_EDITOR_CONSTRAINT_NAME m_constraintType;
};


class DrcReNumericInputConstraintPanelParams : public DrcReConstraintPanelParams
{
public:

    DrcReNumericInputConstraintPanelParams() : DrcReConstraintPanelParams()
    {
    }

    // Constructor for easy initialization
    DrcReNumericInputConstraintPanelParams( const wxString& aConstraintTitle,
                                            const std::shared_ptr<DrcReNumericInputConstraintData>& aConstraintData,
                                            const DRC_RULE_EDITOR_CONSTRAINT_NAME& aConstraintType ) :
            DrcReConstraintPanelParams( aConstraintTitle, aConstraintType ),
            m_constraintData( aConstraintData )
    {
    }

    DrcReNumericInputConstraintPanelParams( const wxString& aConstraintTitle,
                                            const std::shared_ptr<DrcReNumericInputConstraintData>& aConstraintData,
                                            const DRC_RULE_EDITOR_CONSTRAINT_NAME& aConstraintType,
                                            const wxString& aCustomLabelText ) :
            DrcReNumericInputConstraintPanelParams( aConstraintTitle, aConstraintData, aConstraintType )
    {
        m_customLabelText = aCustomLabelText;
    }


    void SetInputIsCount( bool aInputIsCount ) { m_isCountInput = aInputIsCount; }

    wxString m_customLabelText;
    bool m_isCountInput;
    std::shared_ptr<DrcReNumericInputConstraintData> m_constraintData;
};


class DrcReBoolInputConstraintPanelParams : public DrcReConstraintPanelParams
{
public:

    DrcReBoolInputConstraintPanelParams() : DrcReConstraintPanelParams()
    {
    }

    // Constructor for easy initialization
    DrcReBoolInputConstraintPanelParams( const wxString& aConstraintTitle,
                                         const std::shared_ptr<DrcReBoolInputConstraintData>& aConstraintData,
                                         const DRC_RULE_EDITOR_CONSTRAINT_NAME& aConstraintType ) :
            DrcReConstraintPanelParams( aConstraintTitle, aConstraintType ),
            m_constraintData( aConstraintData )
    {
    }

    DrcReBoolInputConstraintPanelParams( const wxString& aConstraintTitle,
                                         const std::shared_ptr<DrcReBoolInputConstraintData>& aConstraintData,
                                         const DRC_RULE_EDITOR_CONSTRAINT_NAME& aConstraintType,
                                         const wxString& aCustomLabelText ) :
            DrcReBoolInputConstraintPanelParams( aConstraintTitle, aConstraintData,
                                                 aConstraintType )
    {
        m_customLabelText = aCustomLabelText;
    }

    wxString m_customLabelText;
    std::shared_ptr<DrcReBoolInputConstraintData> m_constraintData;
};

#endif // DRC_RE_CONSTRAINT_PANEL_PARAMS_H_