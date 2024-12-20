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

bool DrcRuleEditorUtils::IsBoolInputType(const DRC_RULE_EDITOR_CONSTRAINT_NAME& aConstraintType)
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

bool DrcRuleEditorUtils::IsNumericInputType(const DRC_RULE_EDITOR_CONSTRAINT_NAME& aConstraintType)
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