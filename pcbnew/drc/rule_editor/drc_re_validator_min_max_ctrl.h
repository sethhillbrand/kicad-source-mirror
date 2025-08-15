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

#ifndef DRC_RULE_EDITOR_VALIDATOR_MIN_MAX_CTRL_H_
#define DRC_RULE_EDITOR_VALIDATOR_MIN_MAX_CTRL_H_

#include <string>
#include "drc_rule_editor_enums.h"
#include <wx/wx.h>
#include <wx/object.h>


class VALIDATE_MIN_MAX_CTRL : public wxValidator
{
public:
    enum class ValidationState
    {
        Valid,            // Both min and max are valid
        MinGreaterThanMax // Minimum value is greater than maximum value
    };

    // Constructor
    VALIDATE_MIN_MAX_CTRL( wxTextCtrl* minCtrl, wxTextCtrl* maxCtrl );

    // Clone method for validator
    virtual wxObject* Clone() const override;

    // Validation logic
    virtual bool Validate( wxWindow* parent ) override;

    // Accessor for validation state
    ValidationState GetValidationState() const;

private:
    wxTextCtrl*     m_minCtrl;         // Name of the min control
    wxTextCtrl*     m_maxCtrl;         // Name of the max control
    std::string     m_minCtrlName;     // Name of the min control
    std::string     m_maxCtrlName;     // Name of the max control
    ValidationState m_validationState; // Store the result of validation
};

#endif // DRC_RULE_EDITOR_VALIDATOR_MIN_MAX_CTRL_H_