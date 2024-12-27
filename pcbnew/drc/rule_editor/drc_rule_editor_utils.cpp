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

#include "drc_rule_editor_utils.h"
#include "drc_re_validator_numeric_ctrl.h"
#include "drc_re_validator_min_max_ctrl.h"
#include "drc_re_validator_min_preferred_max_ctrl.h"
#include "drc_re_validator_checkbox_list.h"
#include "drc_re_validator_combo_ctrl.h"


bool DRC_RULE_EDITOR_UTILS::IsBoolInputType(const DRC_RULE_EDITOR_CONSTRAINT_NAME& aConstraintType)
{
    switch (aConstraintType)
    {
        case SHORT_CIRCUIT:
        case UNROUTED:
        case ALLOW_FILLET_OUTSIDE_ZONE_OUTLINE:
        case VIAS_UNDER_SMD:
            return true;
        default:
            return false;
    }
}

bool DRC_RULE_EDITOR_UTILS::IsNumericInputType(const DRC_RULE_EDITOR_CONSTRAINT_NAME& aConstraintType)
{
    switch (aConstraintType)
    {
        case NET_ANTENNA:
        case BASIC_CLEARANCE:
        case BOARD_OUTLINE_CLEARANCE:
        case MINIMUM_CLEARANCE:
        case MINIMUM_ITEM_CLEARANCE:
        case CREEPAGE_DISTANCE:
        case MINIMUM_CONNECTION_WIDTH:
        case MINIMUM_TRACK_WIDTH:
        case COPPER_TO_HOLE_CLEARANCE:
        case HOLE_TO_HOLE_CLEARANCE:
        case MINIMUM_ANNULAR_WIDTH:
        case COPPER_TO_EDGE_CLEARANCE:
        case MINIMUM_THROUGH_HOLE:
        case HOLE_SIZE:
        case HOLE_TO_HOLE_DISTANCE:
        case MINIMUM_UVIA_HOLE:
        case MINIMUM_UVIA_DIAMETER:
        case MINIMUM_VIA_DIAMETER:
        case SILK_TO_SILK_CLEARANCE:
        case SILK_TO_SOLDERMASK_CLEARANCE:
        case MINIMUM_SOLDERMASK_SILVER:
        case SOLDERMASK_EXPANSION:
        case SOLDERPASTE_EXPANSION:
        case MAXIMUM_ALLOWED_DEVIATION:
        case MINIMUM_ACUTE_ANGLE:
        case MINIMUM_ANGULAR_RING:
        case MINIMUM_THERMAL_RELIEF_SPOKE_COUNT:
        case MAXIMUM_VIA_COUNT:
        case MATCHED_LENGTH_DIFF_PAIR:
        case MATCHED_LENGTH_ALL_TRACES_IN_GROUP:
        case ABSOLUTE_LENGTH:
        case DAISY_CHAIN_STUB:
        case SMD_CORNER:
        case SMD_TO_PLANE_PLUS:
            return true;
        default:
            return false;
    }
}


bool DRC_RULE_EDITOR_UTILS::ValidateIntegerCtrl( wxTextCtrl* textCtrl, std::string label, bool canBeZero,
                                                 int* aErrorCount, std::string* aValidationMessage )
{
    // Set the custom validator
    auto* validator = new VALIDATOR_NUMERIC_CTRL( canBeZero, true );
    textCtrl->SetValidator( *validator );

    if( !textCtrl->Validate() )
    {
        VALIDATOR_NUMERIC_CTRL* v =
                static_cast<VALIDATOR_NUMERIC_CTRL*>( textCtrl->GetValidator() );

        switch( v->GetValidationState() )
        {
        case VALIDATOR_NUMERIC_CTRL::ValidationState::Empty:
        {
            ( *aErrorCount )++;
            *aValidationMessage += DRC_RULE_EDITOR_UTILS::FormatErrorMessage(
                    *aErrorCount, label + " should not be empty !!" );
            return false;
        }
        case VALIDATOR_NUMERIC_CTRL::ValidationState::NotInteger:
        {
            ( *aErrorCount )++;
            *aValidationMessage += DRC_RULE_EDITOR_UTILS::FormatErrorMessage(
                    *aErrorCount, "The value of " + label + " should be valid integer value !!" );
            return false;
        }
        case VALIDATOR_NUMERIC_CTRL::ValidationState::NotGreaterThanZero:
        {
            ( *aErrorCount )++;
            *aValidationMessage += DRC_RULE_EDITOR_UTILS::FormatErrorMessage(
                    *aErrorCount, "The value of " + label + " must be greater than 0 !!" );
            return false;
        }
        default: break;
        }
    }

    return true;
}


bool DRC_RULE_EDITOR_UTILS::ValidateNumericCtrl( wxTextCtrl* textCtrl, std::string label, bool canBeZero,
                                                 int* aErrorCount, std::string* aValidationMessage )
{
    // Set the custom validator
    auto* validator = new VALIDATOR_NUMERIC_CTRL( canBeZero );
    textCtrl->SetValidator( *validator );

    if( !textCtrl->Validate() )
    {
        VALIDATOR_NUMERIC_CTRL* v =
                static_cast<VALIDATOR_NUMERIC_CTRL*>( textCtrl->GetValidator() );

        switch( v->GetValidationState() )
        {
        case VALIDATOR_NUMERIC_CTRL::ValidationState::Empty:
        {
            ( *aErrorCount )++;
            *aValidationMessage += DRC_RULE_EDITOR_UTILS::FormatErrorMessage(
                    *aErrorCount, label + " should not be empty !!" );
            return false;
        }
        case VALIDATOR_NUMERIC_CTRL::ValidationState::NotNumeric:
        {
            ( *aErrorCount )++;
            *aValidationMessage += DRC_RULE_EDITOR_UTILS::FormatErrorMessage(
                    *aErrorCount, "The value of " + label + " should be valid numeric value !!" );
            return false;
        }
        case VALIDATOR_NUMERIC_CTRL::ValidationState::NotGreaterThanZero:
        {
            ( *aErrorCount )++;
            *aValidationMessage += DRC_RULE_EDITOR_UTILS::FormatErrorMessage(
                    *aErrorCount, "The value of " + label + " must be greater than 0 !!" );
            return false;
        }
        default: break;
        }
    }

    return true;
}


bool DRC_RULE_EDITOR_UTILS::ValidateComboCtrl( wxComboBox* aComboBox, std::string label,
                                               int* aErrorCount, std::string* aValidationMessage)
{
    // Set the custom validator
    auto* cmbCtrlValidator = new VALIDATOR_COMBO_CTRL();
    aComboBox->SetValidator( *cmbCtrlValidator );

    if( !aComboBox->Validate() )
    {
        VALIDATOR_COMBO_CTRL* v = static_cast<VALIDATOR_COMBO_CTRL*>( aComboBox->GetValidator() );

        switch( v->GetValidationState() )
        {
        case VALIDATOR_COMBO_CTRL::ValidationState::NothingSelected:
        {
            ( *aErrorCount )++;
            *aValidationMessage += DRC_RULE_EDITOR_UTILS::FormatErrorMessage(
                    *aErrorCount, "Please choose " + label );
            return false;
        }
        default: break;
        }
    }

    return true;
}

bool DRC_RULE_EDITOR_UTILS::ValidateMinMaxCtrl( wxTextCtrl* minTextCtrl, wxTextCtrl* maxTextCtrl,
                                                std::string minLabel, std::string maxLabel,
                                                int* aErrorCount, std::string* aValidationMessage )
{
    minTextCtrl->SetName( "min" );
    maxTextCtrl->SetName( "max" );

    minTextCtrl->SetValidator( VALIDATE_MIN_MAX_CTRL( minTextCtrl, maxTextCtrl ) );

    if( !minTextCtrl->Validate() )
    {
        VALIDATE_MIN_MAX_CTRL* v = static_cast<VALIDATE_MIN_MAX_CTRL*>( minTextCtrl->GetValidator() );

        switch( v->GetValidationState() )
        {
        case VALIDATE_MIN_MAX_CTRL::ValidationState::MinGreaterThanMax:
        {
            ( *aErrorCount )++;
            *aValidationMessage += DRC_RULE_EDITOR_UTILS::FormatErrorMessage(
                    *aErrorCount, minLabel + " value cannot be greater than "+ maxLabel +" value" );
            return false;
        }
        default: break;
        }
    }

    minTextCtrl->SetName( "text" );
    maxTextCtrl->SetName( "text" );

    return true;
}

bool DRC_RULE_EDITOR_UTILS::ValidateMinPreferredMaxCtrl( wxTextCtrl* minTextCtrl, wxTextCtrl* preferredTextCtrl,
                                  wxTextCtrl* maxTextCtrl, std::string minLabel, 
                                  std::string preferredLabel, std::string maxLabel,
                                  int* aErrorCount, std::string* aValidationMessage )
{
    minTextCtrl->SetName( "min" );
    preferredTextCtrl->SetName( "preferred" );
    maxTextCtrl->SetName( "max" );

    minTextCtrl->SetValidator( VALIDATE_MIN_PREFERRED_MAX_CTRL( minTextCtrl, preferredTextCtrl, maxTextCtrl ) );

    if( !minTextCtrl->Validate() )
    {
        VALIDATE_MIN_PREFERRED_MAX_CTRL* v =
                static_cast<VALIDATE_MIN_PREFERRED_MAX_CTRL*>( minTextCtrl->GetValidator() );

        switch( v->GetValidationState() )
        {
        case VALIDATE_MIN_PREFERRED_MAX_CTRL::ValidationState::MinGreaterThanPreferred:
        {
            ( *aErrorCount )++;
            *aValidationMessage += DRC_RULE_EDITOR_UTILS::FormatErrorMessage(
                    *aErrorCount,
                    minLabel + " value cannot be greater than " + preferredLabel + " value" );
            return false;
        }
        case VALIDATE_MIN_PREFERRED_MAX_CTRL::ValidationState::PreferredGreaterThanMax:
        {
            ( *aErrorCount )++;
            *aValidationMessage += DRC_RULE_EDITOR_UTILS::FormatErrorMessage(
                    *aErrorCount,
                    preferredLabel + " value cannot be greater than " + maxLabel + " value" );
            return false;
        }
        case VALIDATE_MIN_PREFERRED_MAX_CTRL::ValidationState::MinGreaterThanMax:
        {
            ( *aErrorCount )++;
            *aValidationMessage += DRC_RULE_EDITOR_UTILS::FormatErrorMessage(
                    *aErrorCount,
                    minLabel + " value cannot be greater than " + maxLabel + " value" );
            return false;
        }
        default: break;
        }
    }

    minTextCtrl->SetName( "text" );
    preferredTextCtrl->SetName( "text" );
    maxTextCtrl->SetName( "text" );

    return true;
}

bool DRC_RULE_EDITOR_UTILS::ValidateCheckBoxCtrls( const std::vector<wxCheckBox*>& checkboxes, std::string label,
                                                   int* aErrorCount, std::string* aValidationMessage )
{
    // Create the validator
    auto* validator = new VALIDATE_CHECKBOX_LIST( checkboxes );

    // Set the validator to a control (e.g., a wxTextCtrl)
    checkboxes[0]->SetValidator( *validator );

    if( !checkboxes[0]->Validate() )
    {
        VALIDATE_CHECKBOX_LIST* v =
                static_cast<VALIDATE_CHECKBOX_LIST*>( checkboxes[0]->GetValidator() );

        switch( v->GetValidationState() )
        {
        case VALIDATE_CHECKBOX_LIST::ValidationState::NotSelected:
        {
            ( *aErrorCount )++;
            *aValidationMessage += DRC_RULE_EDITOR_UTILS::FormatErrorMessage( *aErrorCount, 
                "Please select at least one option from " + label + " list" );
            return false;
        }
        default: break;
        }
    }

    return true;
}


std::string DRC_RULE_EDITOR_UTILS::FormatErrorMessage( const int& aErrorCount, const std::string aErrorMessage )
{
    return std::to_string( aErrorCount ) + ". " + aErrorMessage + "\n";
}