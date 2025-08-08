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

#ifndef DRC_RULE_EDITOR_ENUMS_H_
#define DRC_RULE_EDITOR_ENUMS_H_


enum DRC_RULE_EDITOR_ITEM_TYPE
{
    ROOT = 0,
    CATEGORY,
    RULE_TYPE,
    CONSTRAINT,
    RULE,
};

enum DRC_RULE_EDITOR_CONSTRAINT_NAME
{
    BASIC_CLEARANCE = 0,
    BOARD_OUTLINE_CLEARANCE,
    MINIMUM_CLEARANCE,
    MINIMUM_ITEM_CLEARANCE,
    NET_ANTENNA,
    SHORT_CIRCUIT,
    CREEPAGE_DISTANCE,
    MINIMUM_CONNECTION_WIDTH,
    MINIMUM_TRACK_WIDTH,
    UNROUTED,
    COPPER_TO_HOLE_CLEARANCE,
    HOLE_TO_HOLE_CLEARANCE,
    MINIMUM_THERMAL_RELIEF_SPOKE_COUNT,
    ALLOW_FILLET_OUTSIDE_ZONE_OUTLINE,
    MINIMUM_ANNULAR_WIDTH,
    COPPER_TO_EDGE_CLEARANCE,
    COURTYARD_CLEARANCE,
    PHYSICAL_CLEARANCE,
    MINIMUM_THROUGH_HOLE,
    HOLE_SIZE,
    HOLE_TO_HOLE_DISTANCE,
    MINIMUM_UVIA_HOLE,
    MINIMUM_UVIA_DIAMETER,
    MINIMUM_VIA_DIAMETER,
    VIA_STYLE,
    MINIMUM_TEXT_HEIGHT_AND_THICKNESS,
    SILK_TO_SILK_CLEARANCE,
    SILK_TO_SOLDERMASK_CLEARANCE,
    MINIMUM_SOLDERMASK_SILVER,
    SOLDERMASK_EXPANSION,
    SOLDERPASTE_EXPANSION,
    MAXIMUM_ALLOWED_DEVIATION,
    MINIMUM_ACUTE_ANGLE,
    MINIMUM_ANGULAR_RING,
    MATCHED_LENGTH_DIFF_PAIR,
    ROUTING_DIFF_PAIR,
    ROUTING_WIDTH,
    MAXIMUM_VIA_COUNT,
    MATCHED_LENGTH_ALL_TRACES_IN_GROUP,
    ABSOLUTE_LENGTH,
    ABSOLUTE_LENGTH_2,
    PARALLEL_LIMIT,
    DAISY_CHAIN_STUB,
    DAISY_CHAIN_STUB_2,
    PERMITTED_LAYERS,
    ALLOWED_ORIENTATION,
    CORNER_STYLE,
    SMD_CORNER,
    SMD_ENTRY,
    SMD_TO_PLANE_PLUS,
    VIAS_UNDER_SMD,
    CUSTOM_RULE
};

#endif // DRC_RULE_EDITOR_ENUMS_H_
