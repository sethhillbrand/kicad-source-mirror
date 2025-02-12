/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2025 Ethan Chien <liangtie.qian@gmail.com>
 * Copyright (C) 2025 KiCad Developers, see AUTHORS.txt for contributors.
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

#ifndef NETLIST_JSON_SCHEMA_H
#define NETLIST_JSON_SCHEMA_H

namespace copilot {

    constexpr const char* NETLIST_JSON_SCHEMA = R"json(
    {
      "$schema": "http://json-schema.org/draft-07/schema#",
      "title": "KiCad SCHEMATIC SELECTION Schema",
      "description": "The selection of items on a KiCad SCHEMATIC.",
      "type": "object",
      "properties": {
        "components": {
          "type": "array",
          "description": "List of components used in the schematic selections.",
          "items": {
            "type": "object",
            "properties": {
              "ref": {
                "type": "string",
                "description": "Unique reference designator of the component (e.g., 'R1', 'C2')."
              },
              "fields": {
                "type": "array",
                "description": "List of fields associated with the component, such as value, footprint, or custom attributes.",
                "items": {
                  "type": "object",
                  "properties": {
                    "name": {
                      "type": "string",
                      "description": "Name of the field (e.g., 'Value', 'Footprint')."
                    },
                    "value": {
                      "type": "string",
                      "description": "Value assigned to the field."
                    }
                  },
                  "required": ["name", "value"],
                  "additionalProperties": false
                }
              }
            },
            "required": ["ref", "fields"],
            "additionalProperties": false
          }
        },
        "nets": {
          "type": "array",
          "description": "List of nets defining electrical connections between component pins.",
          "items": {
            "type": "object",
            "properties": {
              "net_name": {
                "type": "string",
                "description": "Name of the electrical net (e.g., 'GND', 'VCC', 'Net-(R1-Pad1)')."
              },
              "nodes": {
                "type": "array",
                "description": "List of nodes (connections) within the net.",
                "items": {
                  "type": "object",
                  "properties": {
                    "ref": {
                      "type": "string",
                      "description": "Reference designator of the component connected to this node."
                    },
                    "pin": {
                      "type": "string",
                      "description": "Pin number on the component (e.g., '1', '2')."
                    },
                    "pinfunction": {
                      "type": "string",
                      "description": "Name of the pin function; defaults to pin number if not explicitly named."
                    },
                    "pintype": {
                      "type": "string",
                      "description": "Type of the pin (e.g., 'input', 'output', 'power_in', etc.)."
                    }
                  },
                  "required": ["ref", "pin", "pinfunction", "pintype"],
                  "additionalProperties": false
                }
              }
            },
            "required": ["net_name", "nodes"],
            "additionalProperties": false
          }
        }
      },
      "required": ["components", "nets"],
      "additionalProperties": false
    }
    )json";
    
    } // namespace copilot
    

#endif
