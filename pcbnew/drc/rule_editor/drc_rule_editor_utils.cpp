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
#include <unordered_map>

using CODE_MAP = std::unordered_map<DRC_RULE_EDITOR_CONSTRAINT_NAME, const char*>;
using REVERSE_CODE_MAP = std::unordered_map<wxString, DRC_RULE_EDITOR_CONSTRAINT_NAME, wxStringHash, wxStringEqual>;

static const CODE_MAP sCodeMap = { { BASIC_CLEARANCE, "clearance" },
                                   { BOARD_OUTLINE_CLEARANCE, "edge_clearance" },
                                   { MINIMUM_CLEARANCE, "clearance" },
                                   { MINIMUM_ITEM_CLEARANCE, "clearance" },
                                   { NET_ANTENNA, "net_antenna" },
                                   { SHORT_CIRCUIT, "short_circuit" },
                                   { CREEPAGE_DISTANCE, "creepage" },
                                   { MINIMUM_CONNECTION_WIDTH, "connection_width" },
                                   { MINIMUM_TRACK_WIDTH, "track_width" },
                                   { UNROUTED, "unrouted" },
                                   { COPPER_TO_HOLE_CLEARANCE, "hole_clearance" },
                                   { HOLE_TO_HOLE_CLEARANCE, "hole_to_hole" },
                                   { MINIMUM_THERMAL_RELIEF_SPOKE_COUNT, "thermal_spoke_width" },
                                   { ALLOW_FILLET_OUTSIDE_ZONE_OUTLINE, "allow_fillet_outside_zone_outline" },
                                   { MINIMUM_ANNULAR_WIDTH, "annular_width" },
                                   { COPPER_TO_EDGE_CLEARANCE, "edge_clearance" },
                                   { COURTYARD_CLEARANCE, "courtyard_clearance" },
                                   { PHYSICAL_CLEARANCE, "physical_clearance" },
                                   { MINIMUM_THROUGH_HOLE, "hole" },
                                   { HOLE_SIZE, "hole" },
                                   { HOLE_TO_HOLE_DISTANCE, "hole_to_hole" },
                                   { MINIMUM_UVIA_HOLE, "hole" },
                                   { MINIMUM_UVIA_DIAMETER, "via_diameter" },
                                   { MINIMUM_VIA_DIAMETER, "via_diameter" },
                                   { VIA_STYLE, "via_style" },
                                   { MINIMUM_TEXT_HEIGHT_AND_THICKNESS, "text_height" },
                                   { SILK_TO_SILK_CLEARANCE, "silk_clearance" },
                                   { SILK_TO_SOLDERMASK_CLEARANCE, "silk_clearance" },
                                   { MINIMUM_SOLDERMASK_SILVER, "solder_mask_sliver" },
                                   { SOLDERMASK_EXPANSION, "solder_mask_expansion" },
                                   { SOLDERPASTE_EXPANSION, "solder_paste_abs_margin" },
                                   { MAXIMUM_ALLOWED_DEVIATION, "maximum_allowed_deviation" },
                                   { MINIMUM_ACUTE_ANGLE, "track_angle" },
                                   { MINIMUM_ANGULAR_RING, "annular_width" },
                                   { MATCHED_LENGTH_DIFF_PAIR, "length" },
                                   { ROUTING_DIFF_PAIR, "diff_pair_gap" },
                                   { ROUTING_WIDTH, "track_width" },
                                   { MAXIMUM_VIA_COUNT, "via_count" },
                                   { MATCHED_LENGTH_ALL_TRACES_IN_GROUP, "length" },
                                   { ABSOLUTE_LENGTH, "length" },
                                   { ABSOLUTE_LENGTH_2, "length" },
                                   { PARALLEL_LIMIT, "parallel_limit" },
                                   { DAISY_CHAIN_STUB, "daisy_chain_stub" },
                                   { DAISY_CHAIN_STUB_2, "daisy_chain_stub_2" },
                                   { PERMITTED_LAYERS, "permitted_layers" },
                                   { ALLOWED_ORIENTATION, "allowed_orientation" },
                                   { CORNER_STYLE, "corner_style" },
                                   { SMD_CORNER, "smd_corner" },
                                   { SMD_ENTRY, "smd_entry" },
                                   { SMD_TO_PLANE_PLUS, "smd_to_plane_plus" },
                                   { VIAS_UNDER_SMD, "vias_under_smd" } };

static const REVERSE_CODE_MAP sCodeReverse = []
{
    REVERSE_CODE_MAP map;
    for( const auto& [type, code] : sCodeMap )
        map.emplace( wxString::FromUTF8( code ), type );
    return map;
}();


wxString DRC_RULE_EDITOR_UTILS::GetConstraintCode( DRC_RULE_EDITOR_CONSTRAINT_NAME aConstraintType )
{
    auto it = sCodeMap.find( aConstraintType );
    if( it != sCodeMap.end() )
        return wxString::FromUTF8( it->second );

    return wxString();
}


std::optional<DRC_RULE_EDITOR_CONSTRAINT_NAME> DRC_RULE_EDITOR_UTILS::GetConstraintTypeFromCode( const wxString& aCode )
{
    auto it = sCodeReverse.find( aCode );
    if( it != sCodeReverse.end() )
        return it->second;

    return std::nullopt;
}


wxString DRC_RULE_EDITOR_UTILS::ConstraintToKicadDrc( DRC_RULE_EDITOR_CONSTRAINT_NAME aType )
{
    return GetConstraintCode( aType );
}


bool DRC_RULE_EDITOR_UTILS::ConstraintFromKicadDrc( const wxString& aCode, DRC_RE_BASE_CONSTRAINT_DATA* aData )
{
    if( !aData )
        return false;

    auto type = GetConstraintTypeFromCode( aCode );
    if( type )
    {
        aData->SetConstraintCode( GetConstraintCode( *type ) );
        return true;
    }

    aData->SetConstraintCode( aCode );
    return false;
}


bool DRC_RULE_EDITOR_UTILS::IsBoolInputType( const DRC_RULE_EDITOR_CONSTRAINT_NAME& aConstraintType )
{
    switch( aConstraintType )
    {
    case SHORT_CIRCUIT:
    case UNROUTED:
    case ALLOW_FILLET_OUTSIDE_ZONE_OUTLINE:
    case VIAS_UNDER_SMD: return true;
    default: return false;
    }
}


bool DRC_RULE_EDITOR_UTILS::IsNumericInputType( const DRC_RULE_EDITOR_CONSTRAINT_NAME& aConstraintType )
{
    switch( aConstraintType )
    {
    case ABSOLUTE_LENGTH:
    case BASIC_CLEARANCE:
    case BOARD_OUTLINE_CLEARANCE:
    case COPPER_TO_EDGE_CLEARANCE:
    case COPPER_TO_HOLE_CLEARANCE:
    case COURTYARD_CLEARANCE:
    case CREEPAGE_DISTANCE:
    case DAISY_CHAIN_STUB:
    case HOLE_SIZE:
    case HOLE_TO_HOLE_CLEARANCE:
    case HOLE_TO_HOLE_DISTANCE:
    case MATCHED_LENGTH_ALL_TRACES_IN_GROUP:
    case MATCHED_LENGTH_DIFF_PAIR:
    case MAXIMUM_ALLOWED_DEVIATION:
    case MAXIMUM_VIA_COUNT:
    case MINIMUM_ACUTE_ANGLE:
    case MINIMUM_ANGULAR_RING:
    case MINIMUM_ANNULAR_WIDTH:
    case MINIMUM_CLEARANCE:
    case MINIMUM_CONNECTION_WIDTH:
    case MINIMUM_ITEM_CLEARANCE:
    case MINIMUM_SOLDERMASK_SILVER:
    case MINIMUM_THERMAL_RELIEF_SPOKE_COUNT:
    case MINIMUM_THROUGH_HOLE:
    case MINIMUM_TRACK_WIDTH:
    case MINIMUM_UVIA_DIAMETER:
    case MINIMUM_UVIA_HOLE:
    case MINIMUM_VIA_DIAMETER:
    case NET_ANTENNA:
    case SILK_TO_SILK_CLEARANCE:
    case SILK_TO_SOLDERMASK_CLEARANCE:
    case SMD_CORNER:
    case SMD_TO_PLANE_PLUS:
    case SOLDERMASK_EXPANSION:
    case SOLDERPASTE_EXPANSION:
        return true;
    default:
        return false;
    }
}


bool DRC_RULE_EDITOR_UTILS::ValidateNumericCtrl( wxTextCtrl* aTextCtrl, std::string aLabel, bool aCanBeZero,
                                                 int* aErrorCount, std::string* aValidationMessage )
{
    VALIDATOR_NUMERIC_CTRL validator( aCanBeZero );
    aTextCtrl->SetValidator( validator );

    if( !aTextCtrl->Validate() )
    {
        VALIDATOR_NUMERIC_CTRL* v = static_cast<VALIDATOR_NUMERIC_CTRL*>( aTextCtrl->GetValidator() );

        switch( v->GetValidationState() )
        {
        case VALIDATOR_NUMERIC_CTRL::VALIDATION_STATE::Empty:
        {
            ( *aErrorCount )++;
            *aValidationMessage +=
                    DRC_RULE_EDITOR_UTILS::FormatErrorMessage( *aErrorCount, aLabel + " should not be empty !!" );
            return false;
        }
        case VALIDATOR_NUMERIC_CTRL::VALIDATION_STATE::NotNumeric:
        {
            ( *aErrorCount )++;
            *aValidationMessage += DRC_RULE_EDITOR_UTILS::FormatErrorMessage(
                    *aErrorCount, "The value of " + aLabel + " should be valid numeric value !!" );
            return false;
        }
        case VALIDATOR_NUMERIC_CTRL::VALIDATION_STATE::NotGreaterThanZero:
        {
            ( *aErrorCount )++;
            *aValidationMessage += DRC_RULE_EDITOR_UTILS::FormatErrorMessage(
                    *aErrorCount, "The value of " + aLabel + " must be greater than 0 !!" );
            return false;
        }
        default: break;
        }
    }

    return true;
}


bool DRC_RULE_EDITOR_UTILS::ValidateIntegerCtrl( wxTextCtrl* aTextCtrl, std::string aLabel, bool aCanBeZero,
                                                 int* aErrorCount, std::string* aValidationMessage )
{
    VALIDATOR_NUMERIC_CTRL validator( aCanBeZero, true );
    aTextCtrl->SetValidator( validator );

    if( !aTextCtrl->Validate() )
    {
        VALIDATOR_NUMERIC_CTRL* v = static_cast<VALIDATOR_NUMERIC_CTRL*>( aTextCtrl->GetValidator() );

        switch( v->GetValidationState() )
        {
        case VALIDATOR_NUMERIC_CTRL::VALIDATION_STATE::Empty:
        {
            ( *aErrorCount )++;
            *aValidationMessage +=
                    DRC_RULE_EDITOR_UTILS::FormatErrorMessage( *aErrorCount, aLabel + " should not be empty !!" );
            return false;
        }
        case VALIDATOR_NUMERIC_CTRL::VALIDATION_STATE::NotInteger:
        {
            ( *aErrorCount )++;
            *aValidationMessage += DRC_RULE_EDITOR_UTILS::FormatErrorMessage(
                    *aErrorCount, "The value of " + aLabel + " should be valid integer value !!" );
            return false;
        }
        case VALIDATOR_NUMERIC_CTRL::VALIDATION_STATE::NotGreaterThanZero:
        {
            ( *aErrorCount )++;
            *aValidationMessage += DRC_RULE_EDITOR_UTILS::FormatErrorMessage(
                    *aErrorCount, "The value of " + aLabel + " must be greater than 0 !!" );
            return false;
        }
        default: break;
        }
    }

    return true;
}


bool DRC_RULE_EDITOR_UTILS::ValidateComboCtrl( wxComboBox* aComboBox, std::string aLabel, int* aErrorCount,
                                               std::string* aValidationMessage )
{
    VALIDATOR_COMBO_CTRL cmbCtrlValidator;
    aComboBox->SetValidator( cmbCtrlValidator );

    if( !aComboBox->Validate() )
    {
        VALIDATOR_COMBO_CTRL* v = static_cast<VALIDATOR_COMBO_CTRL*>( aComboBox->GetValidator() );

        switch( v->GetValidationState() )
        {
        case VALIDATOR_COMBO_CTRL::VALIDATION_STATE::NothingSelected:
        {
            ( *aErrorCount )++;
            *aValidationMessage += DRC_RULE_EDITOR_UTILS::FormatErrorMessage( *aErrorCount, "Please choose " + aLabel );
            return false;
        }
        default: break;
        }
    }

    return true;
}


bool DRC_RULE_EDITOR_UTILS::ValidateMinMaxCtrl( wxTextCtrl* aMinTextCtrl, wxTextCtrl* aMaxTextCtrl,
                                                std::string aMinLabel, std::string aMaxLabel, int* aErrorCount,
                                                std::string* aValidationMessage )
{
    aMinTextCtrl->SetName( "min" );
    aMaxTextCtrl->SetName( "max" );

    aMinTextCtrl->SetValidator( VALIDATE_MIN_MAX_CTRL( aMinTextCtrl, aMaxTextCtrl ) );

    if( !aMinTextCtrl->Validate() )
    {
        VALIDATE_MIN_MAX_CTRL* v = static_cast<VALIDATE_MIN_MAX_CTRL*>( aMinTextCtrl->GetValidator() );

        switch( v->GetValidationState() )
        {
        case VALIDATE_MIN_MAX_CTRL::VALIDATION_STATE::MinGreaterThanMax:
        {
            ( *aErrorCount )++;
            *aValidationMessage += DRC_RULE_EDITOR_UTILS::FormatErrorMessage(
                    *aErrorCount, aMinLabel + " value cannot be greater than " + aMaxLabel + " value" );
            return false;
        }
        default: break;
        }
    }

    aMinTextCtrl->SetName( "text" );
    aMaxTextCtrl->SetName( "text" );

    return true;
}


bool DRC_RULE_EDITOR_UTILS::ValidateMinPreferredMaxCtrl( wxTextCtrl* aMinTextCtrl, wxTextCtrl* aPreferredTextCtrl,
                                                         wxTextCtrl* aMaxTextCtrl, std::string aMinLabel,
                                                         std::string aPreferredLabel, std::string aMaxLabel,
                                                         int* aErrorCount, std::string* aValidationMessage )
{
    aMinTextCtrl->SetName( "min" );
    aPreferredTextCtrl->SetName( "preferred" );
    aMaxTextCtrl->SetName( "max" );

    aMinTextCtrl->SetValidator( VALIDATE_MIN_PREFERRED_MAX_CTRL( aMinTextCtrl, aPreferredTextCtrl, aMaxTextCtrl ) );

    if( !aMinTextCtrl->Validate() )
    {
        VALIDATE_MIN_PREFERRED_MAX_CTRL* v =
                static_cast<VALIDATE_MIN_PREFERRED_MAX_CTRL*>( aMinTextCtrl->GetValidator() );

        switch( v->GetValidationState() )
        {
        case VALIDATE_MIN_PREFERRED_MAX_CTRL::VALIDATION_STATE::MinGreaterThanPreferred:
        {
            ( *aErrorCount )++;
            *aValidationMessage += DRC_RULE_EDITOR_UTILS::FormatErrorMessage(
                    *aErrorCount, aMinLabel + " value cannot be greater than " + aPreferredLabel + " value" );
            return false;
        }
        case VALIDATE_MIN_PREFERRED_MAX_CTRL::VALIDATION_STATE::PreferredGreaterThanMax:
        {
            ( *aErrorCount )++;
            *aValidationMessage += DRC_RULE_EDITOR_UTILS::FormatErrorMessage(
                    *aErrorCount, aPreferredLabel + " value cannot be greater than " + aMaxLabel + " value" );
            return false;
        }
        case VALIDATE_MIN_PREFERRED_MAX_CTRL::VALIDATION_STATE::MinGreaterThanMax:
        {
            ( *aErrorCount )++;
            *aValidationMessage += DRC_RULE_EDITOR_UTILS::FormatErrorMessage(
                    *aErrorCount, aMinLabel + " value cannot be greater than " + aMaxLabel + " value" );
            return false;
        }
        default: break;
        }
    }

    aMinTextCtrl->SetName( "text" );
    aPreferredTextCtrl->SetName( "text" );
    aMaxTextCtrl->SetName( "text" );

    return true;
}


bool DRC_RULE_EDITOR_UTILS::ValidateCheckBoxCtrls( const std::vector<wxCheckBox*>& aCheckboxes, std::string aLabel,
                                                   int* aErrorCount, std::string* aValidationMessage )
{
    VALIDATE_CHECKBOX_LIST validator( aCheckboxes );

    aCheckboxes[0]->SetValidator( validator );

    if( !aCheckboxes[0]->Validate() )
    {
        VALIDATE_CHECKBOX_LIST* v = static_cast<VALIDATE_CHECKBOX_LIST*>( aCheckboxes[0]->GetValidator() );

        switch( v->GetValidationState() )
        {
        case VALIDATE_CHECKBOX_LIST::VALIDATION_STATE::NotSelected:
        {
            ( *aErrorCount )++;
            *aValidationMessage += DRC_RULE_EDITOR_UTILS::FormatErrorMessage(
                    *aErrorCount, "Please select at least one option from " + aLabel + " list" );
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