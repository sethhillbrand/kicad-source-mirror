/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright The KiCad Developers, see AUTHORS.txt for contributors.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you may find one here:
 * http://www.gnu.org/licenses/gpl-3.0.html
 * or you may write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#ifndef ALLEGRO_STRUCTS_H_
#define ALLEGRO_STRUCTS_H_

#include <cstdint>
#include <string>
#include <vector>
#include <map>
#include <array>

namespace ALLEGRO
{

// File format version magic numbers
enum class FILE_VERSION : uint32_t
{
    V16_0 = 0x00130000,
    V16_0_2 = 0x00130200,
    V16_2 = 0x00130402,
    V16_4 = 0x00130C03,
    V16_5 = 0x00131003,
    V16_6 = 0x00131503,
    V16_6_1 = 0x00131504,
    V17_2 = 0x00140400,
    V17_2_1 = 0x00140500,
    V17_2_2 = 0x00140501,
    V17_2_3 = 0x00140502,
    V17_2_4 = 0x00140600,
    V17_2_5 = 0x00140700,
    V17_4 = 0x00140900,
    V17_4_1 = 0x00140901,
    V17_4_2 = 0x00140902,
    V17_4_3 = 0x00140E00,
    V17_5 = 0x00141500,
    V17_5_1 = 0x00141501,
    V17_5_2 = 0x00141502,
};

// Object type identifiers
enum class OBJECT_TYPE : uint8_t
{
    ARC = 0x01,
    UNKNOWN_03 = 0x03,
    NET_ASSIGNMENT = 0x04,
    TRACK = 0x05,
    COMPONENT_DEF = 0x06,
    INSTANCE = 0x07,
    COMPONENT_REF = 0x08,
    UNKNOWN_09 = 0x09,
    DRC_MARKER = 0x0A,
    UNKNOWN_0C = 0x0C,
    PAD = 0x0D,
    UNKNOWN_0E = 0x0E,
    FOOTPRINT_DEF = 0x0F,
    COMPONENT_PIN = 0x10,
    UNKNOWN_11 = 0x11,
    UNKNOWN_12 = 0x12,
    ANNOTATION = 0x14,
    SEGMENT_15 = 0x15,
    SEGMENT_16 = 0x16,
    SEGMENT_17 = 0x17,
    NET = 0x1B,
    PAD_STACK = 0x1C,
    UNKNOWN_1D = 0x1D,
    MODEL_INFO = 0x1E,
    UNKNOWN_1F = 0x1F,
    UNKNOWN_20 = 0x20,
    METADATA = 0x21,
    UNKNOWN_22 = 0x22,
    RAT_LINE = 0x23,
    RECTANGLE = 0x24,
    GROUP_MEMBER = 0x26,
    UNKNOWN_27 = 0x27,
    SHAPE = 0x28,
    LAYER_INFO = 0x2A,
    FOOTPRINT = 0x2B,
    TABLE = 0x2C,
    PLACED_FOOTPRINT = 0x2D,
    UNKNOWN_2E = 0x2E,
    UNKNOWN_2F = 0x2F,
    STRING_GRAPHIC_WRAPPER = 0x30,
    STRING_GRAPHIC = 0x31,
    PLACED_PAD = 0x32,
    VIA = 0x33,
    RULE_REGION = 0x34,
    UNKNOWN_35 = 0x35,
    FONT_DATA = 0x36,
    UNKNOWN_37 = 0x37,
    FILM = 0x38,
    FILM_LAYER_LIST = 0x39,
    FILM_LAYER_NODE = 0x3A,
    SI_MODEL = 0x3B,
    PAIR = 0x3C,
};

// Layer families for categorizing layers
enum class LAYER_FAMILY : uint8_t
{
    BOARD_GEOMETRY = 0x01,
    COPPER = 0x06,
    SILK_SCREEN = 0x09,
};

// Board units
enum class BOARD_UNITS : uint8_t
{
    IMPERIAL = 0x01,
    METRIC = 0x03,
};

// Pad types
enum class PAD_TYPE : uint8_t
{
    THROUGH_HOLE = 0x00,
    VIA = 0x01,
    SMD_PIN = 0x02,
    SLOT = 0x04,
    NON_PLATED_HOLE = 0x08,
    SMD_PIN_ALT = 0x0A,
};

// Text alignment values
enum class TEXT_ALIGNMENT : uint8_t
{
    TOP_LEFT = 1,
    CENTER_LEFT = 2,
    BOTTOM_LEFT = 3,
    TOP_CENTER = 4,
    CENTER = 5,
    BOTTOM_CENTER = 6,
    TOP_RIGHT = 7,
    CENTER_RIGHT = 8,
    BOTTOM_RIGHT = 9,
};

// Text reversal flags
enum class TEXT_REVERSAL : uint8_t
{
    NORMAL = 0,
    REVERSED = 1,
};

// String layer types
enum class STRING_LAYER : uint16_t
{
    BOT_TEXT = 0xF001,
    TOP_TEXT = 0xF101,
    BOT_PIN = 0xF609,
    TOP_PIN = 0xF709,
    TOP_PIN_LABEL = 0xF909,
    BOT_REFDES = 0xFA0D,
    TOP_REFDES = 0xFB0D,
};

// Constants for file parsing
constexpr uint32_t HEADER_STRINGS_OFFSET = 0x1200;
constexpr uint32_t MAX_LAYER_COUNT = 32;
constexpr uint32_t WORD_ALIGNMENT = 4;

// Helper structure for 64-bit floating point values stored in Allegro format
struct CADENCE_FLOAT
{
    uint32_t low;
    uint32_t high;

    double ToDouble() const;
};

// Layer information
struct LAYER_INFO
{
    LAYER_FAMILY family;
    uint8_t      ordinal;

    bool operator<( const LAYER_INFO& aOther ) const
    {
        if( family == aOther.family )
            return ordinal < aOther.ordinal;
        return family < aOther.family;
    }
};

// Linked list pointers used throughout the file format
struct LINKED_LIST_PTRS
{
    uint32_t tail;
    uint32_t head;
};

// Layer mapping entry in the header
struct LAYER_MAP_ENTRY
{
    uint32_t unknown;
    uint32_t layerSetPtr;
};

// Main file header structure
struct FILE_HEADER
{
    uint32_t magic;
    uint32_t reserved1[4];
    uint32_t objectCount;
    uint32_t reserved2[9];

    // Linked lists for different object types
    LINKED_LIST_PTRS netAssignments;    // 0x04
    LINKED_LIST_PTRS components;        // 0x06
    LINKED_LIST_PTRS unknown0C_2;       // 0x0C
    LINKED_LIST_PTRS shapes;            // 0x0E/0x28
    LINKED_LIST_PTRS annotations;       // 0x14
    LINKED_LIST_PTRS nets;              // 0x1B
    LINKED_LIST_PTRS padStacks;         // 0x1C
    LINKED_LIST_PTRS rectangleShapes;   // 0x24/0x28
    LINKED_LIST_PTRS unused1;
    LINKED_LIST_PTRS footprints;        // 0x2B
    LINKED_LIST_PTRS stringsAndUnknown; // 0x03/0x30
    LINKED_LIST_PTRS drcMarkers;        // 0x0A
    LINKED_LIST_PTRS models;            // 0x1D/0x1E/0x1F
    LINKED_LIST_PTRS unused2;
    LINKED_LIST_PTRS films;             // 0x38
    LINKED_LIST_PTRS tables;            // 0x2C
    LINKED_LIST_PTRS unknown0C;         // 0x0C
    LINKED_LIST_PTRS unused3;

    uint32_t unknown35Start;
    uint32_t unknown35End;

    LINKED_LIST_PTRS fontData;          // 0x36
    LINKED_LIST_PTRS metadata;          // 0x21
    LINKED_LIST_PTRS unused4;
    LINKED_LIST_PTRS drcMarkers2;       // 0x0A

    uint32_t reserved3;
    char     allegroVersion[60];
    uint32_t reserved4;
    uint32_t maxKey;
    uint32_t reserved5[17];

    BOARD_UNITS units;
    uint8_t     reserved6;
    uint16_t    reserved7;

    uint32_t reserved8[2];
    uint32_t section27EndOffset;
    uint32_t reserved9;
    uint32_t stringCount;
    uint32_t reserved10[53];
    uint32_t unitDivisor;
    uint32_t reserved11[110];

    std::array<LAYER_MAP_ENTRY, 26> layerSets;
};

// Base structure for all objects with common fields
struct OBJECT_BASE
{
    uint32_t type;
    uint32_t key;
    uint32_t next;
};

// Arc segment data
struct ARC_DATA
{
    uint16_t type;
    uint8_t  unknown1;
    uint8_t  subtype;
    uint32_t key;
    uint32_t next;
    uint32_t parent;
    uint32_t flags;
    uint32_t unknown2;  // Added in v17.2+
    uint32_t width;
    int32_t  coords[4]; // start x, y, end x, y
    CADENCE_FLOAT centerX;
    CADENCE_FLOAT centerY;
    CADENCE_FLOAT radius;
    int32_t  boundingBox[4];
};

// Line segment data
struct SEGMENT_DATA
{
    uint32_t type;
    uint32_t key;
    uint32_t next;
    uint32_t parent;
    uint32_t flags;
    uint32_t unknown;   // Added in v17.2+
    uint32_t width;
    int32_t  coords[4]; // start x, y, end x, y
};

// Net assignment data
struct NET_ASSIGNMENT_DATA
{
    uint32_t type;
    uint32_t key;
    uint32_t next;
    uint32_t netPtr;    // Points to NET_DATA
    uint32_t shapePtr;  // Points to shape or track
    uint32_t unknown;   // Added in v17.4+
};

// Track (collection of segments) data
struct TRACK_DATA
{
    uint16_t   type;
    LAYER_INFO layer;
    uint32_t   key;
    uint32_t   ptr0;
    uint32_t   netAssignmentPtr;
    uint32_t   unknown1[2];
    uint32_t   ptr2[2];
    uint32_t   unknown2;
    uint32_t   ptr3[2];
    uint32_t   unknown3[3]; // Added in v17.2+
    uint32_t   firstSegmentPtr;
    uint32_t   ptr5;
    uint32_t   unknown4;
};

// Pad definition data
struct PAD_DATA
{
    uint32_t type;
    uint32_t key;
    uint32_t stringPtr;     // Pad number/name
    uint32_t ptr2;
    uint32_t unknown1;      // Added in v17.4+
    int32_t  coords[2];     // Relative to footprint origin
    uint32_t padStackPtr;   // Points to PAD_STACK_DATA
    uint32_t unknown2;
    uint32_t unknown3;      // Added in v17.2+
    uint32_t bitmask;
    uint32_t rotation;      // In millidegrees
};

// Component instance data
struct INSTANCE_DATA
{
    uint32_t type;
    uint32_t key;
    uint32_t unknown1;
    uint32_t ptr0;      // Added in v17.2+
    uint32_t unknown2;  // Added in v17.2+
    uint32_t unknown3;  // Added in v17.2+
    uint32_t placedFootprintPtr;
    uint32_t unknown4;  // Removed in v17.2+
    uint32_t refdesStringPtr;
    uint32_t ptr2;
    uint32_t ptr3;
    uint32_t unknown5;
    uint32_t ptr4;
};

// Net data
struct NET_DATA
{
    uint32_t type;
    uint32_t key;
    uint32_t next;
    uint32_t netNameStringPtr;
    uint32_t unknown1;
    uint32_t unknown2;      // Added in v17.2+
    uint32_t flags;
    uint32_t netAssignmentPtr;
    uint32_t ptr2;
    uint32_t pathStringPtr; // Schematic path
    uint32_t ptr4;
    uint32_t modelPtr;
    uint32_t unknown3[2];
    uint32_t ptr6;
};

// Pad stack component (layer-specific pad shape)
struct PAD_STACK_COMPONENT
{
    uint8_t  shapeType;
    uint8_t  unknown1;
    uint8_t  unknown2;
    uint8_t  unknown3;
    uint32_t unknown4;      // Added in v17.2+
    int32_t  width;
    int32_t  height;
    uint32_t unknown5;      // Added in v17.2+
    int32_t  offsetX;
    int32_t  offsetY;
    uint32_t unknown6;      // Added in v17.2+
    uint32_t shapePtr;      // For custom shapes
    uint32_t unknown7;      // Removed in v17.2+
};

// Pad type information
struct PAD_INFO
{
    PAD_TYPE type : 4;
    uint8_t  flagsA : 4;
    uint8_t  flagsB;
    uint8_t  flagsC;
    uint8_t  flagsD;
};

// Pad stack (padstack) data
struct PAD_STACK_DATA
{
    uint16_t type;
    uint8_t  n;
    uint8_t  unknown1;
    uint32_t key;
    uint32_t next;
    uint32_t padStringPtr;
    uint32_t unknown2;
    uint32_t unknown3;
    uint32_t padPathPtr;
    uint32_t unknown4[4];   // Removed in v17.2+
    PAD_INFO padInfo;
    uint32_t unknown5[3];   // Added in v17.2+
    uint16_t unknown6;      // Removed in v17.2+
    uint16_t layerCount;
    uint16_t unknown7;      // Added in v17.2+
    uint32_t unknown8[8];
    uint32_t unknown9[28];  // Added in v17.2+
    uint32_t unknown10[8];  // Added in v16.5/16.6

    // Followed by array of PAD_STACK_COMPONENT structures
};

// Model information header
struct MODEL_INFO_HEADER
{
    uint32_t type;
    uint32_t key;
    uint32_t unknown1;
    uint16_t unknown2;
    uint16_t unknown3;
    uint32_t stringPtr;
    uint32_t size;
};

// Shape (polygon/zone) data
struct SHAPE_DATA
{
    uint16_t   type;
    LAYER_INFO layer;
    uint32_t   key;
    uint32_t   next;
    uint32_t   parentPtr;
    uint32_t   unknown1;
    uint32_t   unknown2[2];  // Added in v17.2+
    uint32_t   ptr2;
    uint32_t   ptr3;
    uint32_t   cutoutsPtr;
    uint32_t   firstSegmentPtr;
    uint32_t   unknown3;
    uint32_t   flags;
    uint32_t   ptr7;        // Added in v17.2+
    uint32_t   ptr6;
    uint32_t   ptr7Alt;     // Removed in v17.2+
    int32_t    boundingBox[4];
};

// Layer set header
struct LAYER_SET_HEADER
{
    uint16_t type;
    uint16_t count;
};

// Layer properties
struct LAYER_PROPERTIES
{
    uint8_t hasBottomReference : 1;
    uint8_t hasTopReference : 1;
    uint8_t : 0;
    uint8_t isPower : 1;
    uint8_t isInner : 1;
    uint8_t isInner2 : 1;
    uint8_t isPower2 : 1;
    uint8_t : 3;
    uint8_t isSignal : 1;
    uint8_t : 3;
    uint8_t isTop : 1;
    uint8_t isBottom : 1;
    uint8_t : 0;
    uint8_t empty;
};

// Local layer properties (v16.4 and earlier)
struct LOCAL_LAYER_PROPERTIES
{
    char             layerName[32];
    LAYER_PROPERTIES properties;
};

// Reference layer properties (v16.5+)
struct REFERENCE_LAYER_PROPERTIES
{
    uint32_t         layerNameStringPtr;
    LAYER_PROPERTIES properties;
    uint32_t         unknown;
};

// Footprint definition data
struct FOOTPRINT_DATA
{
    uint32_t type;
    uint32_t key;
    uint32_t footprintStringPtr;
    uint32_t unknown1;
    int32_t  coords[4];
    uint32_t next;
    uint32_t placedSymbolPtr;
    uint32_t padPtr;
    uint32_t symbolPadPtr;
    uint32_t ptr5;
    uint32_t stringPtr;
    uint32_t ptr6;
    uint32_t ptr7;
    uint32_t ptr8;
    uint32_t unknown2;      // Added in v16.4+
    uint32_t unknown3;      // Added in v17.2+
};

// Placed footprint data
struct PLACED_FOOTPRINT_DATA
{
    uint16_t type;
    uint8_t  layer;
    uint8_t  unknown1;
    uint32_t key;
    uint32_t next;
    uint32_t unknown2;      // Added in v17.2+
    uint32_t instanceRef;   // Position varies by version
    uint16_t unknown3;
    uint16_t unknown4;
    uint32_t unknown5;      // Added in v17.2+
    uint32_t flags;
    uint32_t rotation;      // In millidegrees
    int32_t  coords[2];
    uint32_t instanceRefAlt; // Position varies by version
    uint32_t annotationPtr;
    uint32_t firstPadPtr;
    uint32_t ptr3;
    uint32_t ptr4[3];
    uint32_t groupAssignmentPtr;
};

// String graphic wrapper data
struct STRING_GRAPHIC_WRAPPER_DATA
{
    uint16_t   type;
    LAYER_INFO layer;
    uint32_t   key;
    uint32_t   next;
    uint32_t   unknown1;    // Added in v17.2+
    uint32_t   unknown2;    // Added in v17.2+
    uint32_t   font;        // Added in v17.2+
    uint32_t   ptr3;        // Added in v17.2+
    uint32_t   unknown3;    // Added in v17.4+
    uint32_t   stringGraphicPtr;
    uint32_t   unknown4;
    uint32_t   fontAlt;     // Removed in v17.2+
    uint32_t   ptr4;        // Added in v17.2+
    int32_t    coords[2];
    uint32_t   unknown5;
    uint32_t   rotation;    // In millidegrees
    uint32_t   ptr3Alt;     // Removed in v17.2+
};

// String graphic data
struct STRING_GRAPHIC_DATA
{
    uint16_t      type;
    STRING_LAYER  layer;
    uint32_t      key;
    uint32_t      wrapperPtr;
    int32_t       coords[2];
    uint16_t      unknown;
    uint16_t      length;
    uint32_t      unknown2;  // Added in v17.4+
    // Followed by string data
};

// Text properties
struct TEXT_PROPERTIES
{
    uint8_t        fontKey;
    uint8_t        bitmask;
    TEXT_ALIGNMENT alignment;
    TEXT_REVERSAL  reversal;
};

// Placed pad data
struct PLACED_PAD_DATA
{
    uint16_t   type;
    LAYER_INFO layer;
    uint32_t   key;
    uint32_t   unknown1;
    uint32_t   netPtr;
    uint32_t   flags;
    uint32_t   prev;        // Added in v17.2+
    uint32_t   next;
    uint32_t   ptr3;
    uint32_t   ptr4;
    uint32_t   padPtr;      // Points to PAD_DATA
    uint32_t   ptr6;
    uint32_t   ptr7;
    uint32_t   ptr8;
    uint32_t   previous;
    uint32_t   unknown2;    // Added in v17.2+
    uint32_t   ptr10;
    uint32_t   ptr11;
    int32_t    coords[4];
};

// Via data
struct VIA_DATA
{
    uint16_t   type;
    LAYER_INFO layer;
    uint32_t   key;
    uint32_t   unknown1;
    uint32_t   netPtr;
    uint32_t   flags;
    uint32_t   unknown2;    // Added in v17.2+
    uint32_t   ptr2;
    uint32_t   ptr7;        // Added in v17.2+
    int32_t    coords[2];
    uint32_t   ptr3;
    uint32_t   ptr4;
    uint32_t   ptr5;
    uint32_t   ptr6;
    uint32_t   unknown3[2];
    int32_t    boundingBox[4];
};

// Rectangle data
struct RECTANGLE_DATA
{
    uint16_t   type;
    LAYER_INFO layer;
    uint32_t   key;
    uint32_t   next;
    uint32_t   ptr1;
    uint32_t   unknown1;
    uint32_t   unknown2;    // Added in v17.2+
    int32_t    coords[4];
    uint32_t   ptr2;
    uint32_t   unknown3[3];
};

// Rule region data
struct RULE_REGION_DATA
{
    uint16_t   type;
    LAYER_INFO layer;
    uint32_t   key;
    uint32_t   next;
    uint32_t   ptr1;
    uint32_t   unknown1;    // Added in v17.2+
    uint32_t   flags;
    uint32_t   firstSegmentPtr;
    uint32_t   ptr3;
    uint32_t   unknown2;
};

// Font dimension data
struct FONT_DIMENSION_DATA
{
    uint32_t a;
    uint32_t b;
    uint32_t charHeight;
    uint32_t charWidth;
    uint32_t unknown1;      // Added in v17.4+
    uint32_t xs[4];
    uint32_t ys[8];         // Added in v17.2+
};

// Font data container
struct FONT_DATA_CONTAINER
{
    uint16_t type;
    uint16_t subtype;
    uint32_t key;
    uint32_t next;
    uint32_t unknown1;      // Added in v17.2+
    uint32_t size;
    uint32_t count;
    uint32_t lastIndex;
    uint32_t unknown2;
    uint32_t unknown3;      // Added in v17.4+
};

} // namespace ALLEGRO

#endif // ALLEGRO_STRUCTS_H_