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

#ifndef DRC_RULE_EDITOR_UTILS_H_
#define DRC_RULE_EDITOR_UTILS_H_

#include <wx/wx.h>
#include <wx/object.h>

#include <string>

#include "drc_rule_editor_enums.h"


class DRC_RULE_EDITOR_UTILS
{
public:   
    static bool IsBoolInputType( const DRC_RULE_EDITOR_CONSTRAINT_NAME& aConstraintType );

    static bool IsNumericInputType( const DRC_RULE_EDITOR_CONSTRAINT_NAME& aConstraintType );

    static std::string FormatErrorMessage( const int& aErrorCount,
                                           const std::string aErrorMessage );

    static bool ValidateNumericCtrl( wxTextCtrl* aTextCtrl, std::string aLabel, bool aCanBeZero,
                                     int* aErrorCount, std::string* aValidationMessage );

    static bool ValidateIntegerCtrl( wxTextCtrl* aTextCtrl, std::string aLabel, bool aCanBeZero,
                                     int* aErrorCount, std::string* aValidationMessage );

    static bool ValidateComboCtrl( wxComboBox* aComboBox, std::string aLabel, int* aErrorCount,
                                   std::string* aValidationMessage );

    static bool ValidateMinMaxCtrl( wxTextCtrl* aMinTextCtrl, wxTextCtrl* aMaxTextCtrl,
                                    std::string aMinLabel, std::string aMaxLabel, int* aErrorCount,
                                    std::string* aValidationMessage );

    static bool ValidateMinPreferredMaxCtrl( wxTextCtrl* aMinTextCtrl,
                                             wxTextCtrl* aPreferredTextCtrl,
                                             wxTextCtrl* aMaxTextCtrl, std::string aMinLabel,
                                             std::string aPreferredLabel, std::string aMaxLabel,
                                             int* aErrorCount, std::string* aValidationMessage );

    static bool ValidateCheckBoxCtrls( const std::vector<wxCheckBox*>& aCheckboxes,
                                       std::string aLabel, int* aErrorCount,
                                       std::string* aValidationMessage );
};

#endif // DRC_RULE_EDITOR_UTILS_H_
