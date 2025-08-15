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
#include <optional>

#include "drc_rule_editor_enums.h"
#include "drc_re_base_constraint_data.h"


class DRC_RULE_EDITOR_UTILS
{
public:
    static bool IsBoolInputType( const DRC_RULE_EDITOR_CONSTRAINT_NAME& aConstraintType );

    static bool IsNumericInputType( const DRC_RULE_EDITOR_CONSTRAINT_NAME& aConstraintType );

    static std::string FormatErrorMessage( const int& aErrorCount, const std::string aErrorMessage );

    /**
     * Translate a rule tree node type into the keyword used by the rules
     * file for that constraint.
     *
     * @param aConstraintType The constraint type selected in the rule tree.
     * @return The textual keyword for the constraint, or an empty string if
     *         no mapping exists.
     */
    static wxString GetConstraintCode( DRC_RULE_EDITOR_CONSTRAINT_NAME aConstraintType );

    /**
     * Resolve a constraint keyword from a rules file into the corresponding
     * rule tree enumeration value.
     *
     * @param aCode The textual keyword for the constraint.
     * @return The associated constraint enumeration if found; otherwise
     *         an empty optional.
     */
    static std::optional<DRC_RULE_EDITOR_CONSTRAINT_NAME> GetConstraintTypeFromCode( const wxString& aCode );

    /**
     * Convert a constraint type into the keyword used in a .kicad_drc file.
     *
     * @param aType The constraint type to convert.
     * @return The textual representation suitable for a rules file.
     */
    static wxString ConstraintToKicadDrc( DRC_RULE_EDITOR_CONSTRAINT_NAME aType );

    /**
     * Populate a constraint data object using a keyword from a .kicad_drc file.
     *
     * @param aCode The keyword read from the rules file.
     * @param aData The constraint data instance to update.
     * @return True if a matching constraint type was found.
     */
    static bool ConstraintFromKicadDrc( const wxString& aCode, DRC_RE_BASE_CONSTRAINT_DATA* aData );

    /**
     * Validates a numeric input control, checking if the value is valid, non-empty, and greater than zero.
     *
     * @param aTextCtrl The text control to validate.
     * @param aLabel The label of the control for error messages.
     * @param aCanBeZero If true, zero is allowed; otherwise, the value must be greater than zero.
     * @param aErrorCount The count of errors encountered during validation.
     * @param aValidationMessage The validation error messages accumulated.
     *
     * @return True if the control value is valid; otherwise, false.
     */
    static bool ValidateNumericCtrl( wxTextCtrl* aTextCtrl, std::string aLabel, bool aCanBeZero, int* aErrorCount,
                                     std::string* aValidationMessage );

    /**
     * Validates an integer input control, ensuring the value is a valid integer, non-empty, and greater than zero.
     *
     * @param aTextCtrl The text control to validate.
     * @param aLabel The label of the control for error messages.
     * @param aCanBeZero If true, zero is allowed; otherwise, the value must be greater than zero.
     * @param aErrorCount The count of errors encountered during validation.
     * @param aValidationMessage The validation error messages accumulated.
     *
     * @return True if the control value is valid; otherwise, false.
     */
    static bool ValidateIntegerCtrl( wxTextCtrl* aTextCtrl, std::string aLabel, bool aCanBeZero, int* aErrorCount,
                                     std::string* aValidationMessage );

    /**
     * Validates a combo box control, ensuring that a selection has been made.
     *
     * @param aComboBox The combo box control to validate.
     * @param aLabel The label of the combo box for error messages.
     * @param aErrorCount The count of errors encountered during validation.
     * @param aValidationMessage The validation error messages accumulated.
     *
     * @return True if the control value is valid; otherwise, false.
     */
    static bool ValidateComboCtrl( wxComboBox* aComboBox, std::string aLabel, int* aErrorCount,
                                   std::string* aValidationMessage );

    /**
     * Validates the minimum and maximum value controls, ensuring that the minimum value is not greater than the maximum value.
     *
     * @param aMinTextCtrl The minimum value text control to validate.
     * @param aMaxTextCtrl The maximum value text control to validate.
     * @param aMinLabel The label for the minimum value text control.
     * @param aMaxLabel The label for the maximum value text control.
     * @param aErrorCount The count of errors encountered during validation.
     * @param aValidationMessage The validation error messages accumulated.
     *
     * @return True if the validation passes; otherwise, false.
     */
    static bool ValidateMinMaxCtrl( wxTextCtrl* aMinTextCtrl, wxTextCtrl* aMaxTextCtrl, std::string aMinLabel,
                                    std::string aMaxLabel, int* aErrorCount, std::string* aValidationMessage );

    /**
     * Validates the minimum, preferred, and maximum value controls, ensuring that:
     * - The minimum value is not greater than the preferred value.
     * - The preferred value is not greater than the maximum value.
     * - The minimum value is not greater than the maximum value.
     *
     * @param aMinTextCtrl The minimum value text control to validate.
     * @param aPreferredTextCtrl The preferred value text control to validate.
     * @param aMaxTextCtrl The maximum value text control to validate.
     * @param aMinLabel The label for the minimum value text control.
     * @param aPreferredLabel The label for the preferred value text control.
     * @param aMaxLabel The label for the maximum value text control.
     * @param aErrorCount The count of errors encountered during validation.
     * @param aValidationMessage The validation error messages accumulated.
     *
     * @return True if the validation passes; otherwise, false.
     */
    static bool ValidateMinPreferredMaxCtrl( wxTextCtrl* aMinTextCtrl, wxTextCtrl* aPreferredTextCtrl,
                                             wxTextCtrl* aMaxTextCtrl, std::string aMinLabel,
                                             std::string aPreferredLabel, std::string aMaxLabel, int* aErrorCount,
                                             std::string* aValidationMessage );

    /**
     * Validates a list of checkboxes, ensuring that at least one option is selected.
     *
     * @param aCheckboxes A vector of wxCheckBox controls to validate.
     * @param aLabel The label for the checkbox list.
     * @param aErrorCount The count of errors encountered during validation.
     * @param aValidationMessage The validation error messages accumulated.
     *
     * @return True if the validation passes; otherwise, false.
     */
    static bool ValidateCheckBoxCtrls( const std::vector<wxCheckBox*>& aCheckboxes, std::string aLabel,
                                       int* aErrorCount, std::string* aValidationMessage );
};

#endif // DRC_RULE_EDITOR_UTILS_H_
