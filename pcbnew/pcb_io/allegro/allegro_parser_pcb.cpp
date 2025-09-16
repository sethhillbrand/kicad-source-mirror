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

#include <wx/log.h>
#include <wx/translation.h>

#include <board.h>
#include <board_design_settings.h>
#include <footprint.h>
#include <pad.h>
#include <pcb_track.h>
#include <pcb_shape.h>
#include <pcb_text.h>
#include <pcb_group.h>
#include <zone.h>
#include <netinfo.h>
#include <ki_exception.h>
#include <geometry/seg.h>
#include <geometry/shape_arc.h>
#include <geometry/shape_line_chain.h>
#include <geometry/shape_poly_set.h>
#include <trigo.h>
#include <convert_basic_shapes_to_polygon.h>
#include <fix_board_shape.h>
#include <base_units.h>
#include <pcb_field.h>
#include <lib_id.h>

#include <cstring>
#include <string>
#include <algorithm>

#include "allegro_parser.h"
#include "allegro_file.h"
#include "allegro_structs.h"

// Constants for coordinate conversion
constexpr double ALLEGRO_IMPERIAL_SCALE = 25400.0;
constexpr double ALLEGRO_METRIC_SCALE = 1000000.0;
constexpr int ALLEGRO_ANGLE_SCALE = 1000;  // Millidegrees to degrees

// Default trace width for zones (8 mils)
constexpr int DEFAULT_ZONE_WIDTH = pcbIUScale.MilsToIU( 8 );

// Shape join distance for board outline healing
constexpr int SHAPE_JOIN_DISTANCE = pcbIUScale.mmToIU( 1.5 );

// Field names for Allegro-specific data
constexpr const char* ALLEGRO_ID_FIELD = "ALLEGRO_ID";

using namespace ALLEGRO;

// Layer mapping from Allegro to KiCad
static const std::map<LAYER_INFO, PCB_LAYER_ID> LAYER_MAP = {
    { { LAYER_FAMILY::SILK_SCREEN, 0xF1 }, F_SilkS },
    { { LAYER_FAMILY::SILK_SCREEN, 0xF3 }, B_Paste },
    { { LAYER_FAMILY::SILK_SCREEN, 0xF4 }, F_Paste },
    { { LAYER_FAMILY::SILK_SCREEN, 0xF6 }, B_SilkS },
    { { LAYER_FAMILY::SILK_SCREEN, 0xF7 }, F_SilkS },
    { { LAYER_FAMILY::SILK_SCREEN, 0xFA }, B_CrtYd },
    { { LAYER_FAMILY::SILK_SCREEN, 0xFB }, F_CrtYd },
    { { LAYER_FAMILY::SILK_SCREEN, 0xFD }, F_SilkS },
    { { LAYER_FAMILY::SILK_SCREEN, 0xEC }, B_Mask },
    { { LAYER_FAMILY::SILK_SCREEN, 0xED }, F_Mask },
    { { LAYER_FAMILY::SILK_SCREEN, 0xEE }, User_7 },
    { { LAYER_FAMILY::SILK_SCREEN, 0xEF }, User_8 },
    { { LAYER_FAMILY::SILK_SCREEN, 0x02 }, Eco1_User },
    { { LAYER_FAMILY::SILK_SCREEN, 0x00 }, Eco2_User },
    { { LAYER_FAMILY::BOARD_GEOMETRY, 0xFD }, Edge_Cuts },
};

// Implementation of CADENCE_FLOAT conversion
double CADENCE_FLOAT::ToDouble() const
{
    // Cadence stores doubles with swapped 32-bit words
    uint64_t value = (static_cast<uint64_t>( high ) << 32) | low;
    double result;
    std::memcpy( &result, &value, sizeof( double ) );
    return result;
}

ALLEGRO_PARSER::ALLEGRO_PARSER( BOARD* aBoard, const ALLEGRO_FILE& aAllegroBrdFile ) :
    m_board( aBoard ),
    m_allegroFile( aAllegroBrdFile ),
    m_baseAddress( nullptr ),
    m_currentAddress( nullptr ),
    m_fileSize( 0 ),
    m_fileVersion( FILE_VERSION::V16_0 ),
    m_header( nullptr ),
    m_scaleFactor( 1.0 ),
    m_copperLayerCount( 2 ),
    m_hasVersion172Extensions( false ),
    m_hasVersion174Extensions( false ),
    m_hasVersion165Extensions( false )
{
    wxLogTrace( ALLEGRO_DBG, wxT( "Initializing Allegro parser" ) );
}

void ALLEGRO_PARSER::Parse()
{
    wxLogTrace( ALLEGRO_DBG, wxT( "Starting Allegro file parse" ) );

    // Initialize parsing state
    m_baseAddress = m_allegroFile.region.get_address();
    m_fileSize = m_allegroFile.region.get_size();
    m_currentAddress = m_baseAddress;

    if( !DetectFileVersion() )
        THROW_IO_ERROR( _( "Unsupported Allegro file version" ) );

    InitializeParser();
    ParseHeader();
    ParseStringTable();
    ParseObjects();
    BuildBoard();

    wxLogTrace( ALLEGRO_DBG, wxT( "Allegro file parse complete" ) );
}

bool ALLEGRO_PARSER::DetectFileVersion()
{
    if( m_fileSize < sizeof( uint32_t ) )
        return false;

    uint32_t magic = *static_cast<const uint32_t*>( m_baseAddress );

    wxLogTrace( ALLEGRO_DBG, wxT( "File magic: 0x%08X" ), magic );

    // Map magic number to version enum
    switch( magic )
    {
    case 0x00130000:
    case 0x00130200:
        m_fileVersion = FILE_VERSION::V16_0;
        break;
    case 0x00130402:
        m_fileVersion = FILE_VERSION::V16_2;
        break;
    case 0x00130C03:
        m_fileVersion = FILE_VERSION::V16_4;
        break;
    case 0x00131003:
        m_fileVersion = FILE_VERSION::V16_5;
        m_hasVersion165Extensions = true;
        break;
    case 0x00131503:
    case 0x00131504:
        m_fileVersion = FILE_VERSION::V16_6;
        m_hasVersion165Extensions = true;
        break;
    case 0x00140400:
    case 0x00140500:
    case 0x00140501:
    case 0x00140502:
    case 0x00140600:
    case 0x00140700:
        m_fileVersion = FILE_VERSION::V17_2;
        m_hasVersion165Extensions = true;
        m_hasVersion172Extensions = true;
        break;
    case 0x00140900:
    case 0x00140901:
    case 0x00140902:
    case 0x00140E00:
        m_fileVersion = FILE_VERSION::V17_4;
        m_hasVersion165Extensions = true;
        m_hasVersion172Extensions = true;
        m_hasVersion174Extensions = true;
        break;
    case 0x00141500:
    case 0x00141501:
    case 0x00141502:
        m_fileVersion = FILE_VERSION::V17_5;
        m_hasVersion165Extensions = true;
        m_hasVersion172Extensions = true;
        m_hasVersion174Extensions = true;
        break;
    default:
        wxLogTrace( ALLEGRO_DBG, wxT( "Unknown file version: 0x%08X" ), magic );
        return false;
    }

    wxLogTrace( ALLEGRO_DBG, wxT( "Detected Allegro version %d.%d" ),
                ( magic >> 16 ) & 0xFF, ( magic >> 8 ) & 0xFF );

    return true;
}

void ALLEGRO_PARSER::InitializeParser()
{
    m_objectPointers.clear();
    m_stringTable.clear();
    m_layerSets.clear();
    m_fontData.clear();
    m_footprintLibrary.clear();
}

void ALLEGRO_PARSER::ParseHeader()
{
    wxLogTrace( ALLEGRO_DBG, wxT( "Parsing file header" ) );

    m_header = static_cast<const ALLEGRO::FILE_HEADER*>( m_currentAddress );
    AdvanceReadPointer( sizeof( ALLEGRO::FILE_HEADER ) );

    // Cache object count for validation
    m_objectPointers.reserve( m_header->objectCount );

    // Calculate scale factor based on units
    switch( static_cast<ALLEGRO::BOARD_UNITS>( m_header->units ) )
    {
    case ALLEGRO::BOARD_UNITS::IMPERIAL:
        m_scaleFactor = ALLEGRO_IMPERIAL_SCALE;
        wxLogTrace( ALLEGRO_DBG, wxT( "Using imperial units, scale factor: %s" ),
                   wxString::FromUTF8( std::to_string( m_scaleFactor ) ) );
        break;
    case ALLEGRO::BOARD_UNITS::METRIC:
        m_scaleFactor = ALLEGRO_METRIC_SCALE;
        wxLogTrace( ALLEGRO_DBG, wxT( "Using metric units, scale factor: %s" ),
                   wxString::FromUTF8( std::to_string( m_scaleFactor ) ) );
        break;
    default:
        THROW_IO_ERROR( wxString::Format( _( "Unknown board units: 0x%02X" ),
                                         static_cast<int>( m_header->units ) ) );
    }
#ifndef HEADER_STRINGS_OFFSET
#define HEADER_STRINGS_OFFSET 0x100 // TODO: Set correct offset if known
#endif

struct STRING_GRAPHIC_DATA { uint8_t dummy[32]; }; // TODO: Replace with real definition
struct OBJECT_BASE { uint8_t dummy[32]; }; // TODO: Replace with real definition

    wxLogTrace( ALLEGRO_DBG, wxT( "Allegro version: %s" ), m_header->allegroVersion );
    wxLogTrace( ALLEGRO_DBG, wxT( "Object count: %u" ), m_header->objectCount );
    wxLogTrace( ALLEGRO_DBG, wxT( "String count: %u" ), m_header->stringCount );
}

void ALLEGRO_PARSER::ParseStringTable()
{
    wxLogTrace( ALLEGRO_DBG, wxT( "Parsing string table with %u entries" ),
                m_header->stringCount );

    // String table starts at fixed offset
    m_currentAddress = static_cast<const char*>( m_baseAddress ) + HEADER_STRINGS_OFFSET;

    for( uint32_t i = 0; i < m_header->stringCount; i++ )
    {
        uint32_t id = *static_cast<const uint32_t*>( m_currentAddress );
        AdvanceReadPointer( sizeof( uint32_t ) );

        const char* str = static_cast<const char*>( m_currentAddress );
        m_stringTable[id] = str;

        // Strings are word-aligned, including null terminator
        uint32_t length = strlen( str ) + 1;
        AdvanceReadPointer( RoundToWord( length ) );

        wxLogTrace( ALLEGRO_DBG, wxT( "String 0x%08X: %s" ), id, str );
    }
}

void ALLEGRO_PARSER::ParseObjects()
{
    wxLogTrace( ALLEGRO_DBG, wxT( "Parsing objects" ) );

    // Objects follow the string table
    while( m_currentAddress < static_cast<const char*>( m_baseAddress ) + m_fileSize )
    {
        const uint8_t* typePtr = static_cast<const uint8_t*>( m_currentAddress );

        if( *typePtr == 0x00 )
            break;  // End of objects

        OBJECT_TYPE type = static_cast<OBJECT_TYPE>( *typePtr );

        wxLogTrace( ALLEGRO_DBG, wxT( "Parsing object type 0x%02X at offset 0x%08lX" ),
                   *typePtr,
                   static_cast<const char*>( m_currentAddress ) -
                   static_cast<const char*>( m_baseAddress ) );

        switch( type )
        {
        case OBJECT_TYPE::ARC:
            ParseArc( m_currentAddress );
            break;

        case OBJECT_TYPE::NET_ASSIGNMENT:
            ParseNetAssignment( m_currentAddress );
            break;

        case OBJECT_TYPE::TRACK:
            ParseTrack( m_currentAddress );
            break;

        case OBJECT_TYPE::PAD:
            ParsePad( m_currentAddress );
            break;

        case OBJECT_TYPE::INSTANCE:
            ParseInstance( m_currentAddress );
            break;

        case OBJECT_TYPE::NET:
            ParseNet( m_currentAddress );
            break;

        case OBJECT_TYPE::PAD_STACK:
            ParsePadStack( m_currentAddress );
            break;

        case OBJECT_TYPE::SHAPE:
            ParseShape( m_currentAddress );
            break;

        case OBJECT_TYPE::FOOTPRINT:
            ParseFootprint( m_currentAddress );
            break;

        case OBJECT_TYPE::PLACED_FOOTPRINT:
            ParsePlacedFootprint( m_currentAddress );
            break;

        case OBJECT_TYPE::STRING_GRAPHIC_WRAPPER:
            ParseStringGraphic( m_currentAddress );
            break;

        case OBJECT_TYPE::STRING_GRAPHIC:
            {
                // STRING_GRAPHIC is handled with its wrapper
                const STRING_GRAPHIC_DATA* data = static_cast<const STRING_GRAPHIC_DATA*>( m_currentAddress );
                m_objectPointers[ByteSwap32( data->key )] = const_cast<void*>( m_currentAddress );

                size_t objectSize = sizeof( STRING_GRAPHIC_DATA );
                if( m_hasVersion174Extensions )
                    objectSize += sizeof( uint32_t );
                objectSize += RoundToWord( data->length );

                AdvanceReadPointer( objectSize );
            }
            break;

        case OBJECT_TYPE::PLACED_PAD:
            ParsePlacedPad( m_currentAddress );
            break;

        case OBJECT_TYPE::VIA:
            ParseVia( m_currentAddress );
            break;

        case OBJECT_TYPE::RECTANGLE:
            ParseRectangle( m_currentAddress );
            break;

        case OBJECT_TYPE::RULE_REGION:
            ParseRuleRegion( m_currentAddress );
            break;

        case OBJECT_TYPE::FONT_DATA:
            ParseFontData( m_currentAddress );
            break;

        case OBJECT_TYPE::LAYER_INFO:
            ParseLayerInfo( m_currentAddress );
            break;

        case OBJECT_TYPE::MODEL_INFO:
            ParseModelInfo( m_currentAddress );
            break;

        case OBJECT_TYPE::METADATA:
            ParseMetadata( m_currentAddress );
            break;

        case OBJECT_TYPE::SEGMENT_15:
        case OBJECT_TYPE::SEGMENT_16:
        case OBJECT_TYPE::SEGMENT_17:
            ParseSegment( m_currentAddress );
            break;

        case OBJECT_TYPE::UNKNOWN_03:
        case OBJECT_TYPE::UNKNOWN_0E:
        case OBJECT_TYPE::UNKNOWN_27:
            // Skip these known but unused types
            {
                const OBJECT_BASE* base = static_cast<const OBJECT_BASE*>( m_currentAddress );
                m_objectPointers[ByteSwap32( base->key )] = const_cast<void*>( m_currentAddress );
                AdvanceReadPointer( sizeof( OBJECT_BASE ) );
            }
            break;

        default:
            // Skip unknown object types
            wxLogTrace( ALLEGRO_DBG, wxT( "Skipping unknown object type 0x%02X" ), *typePtr );

            // Try to determine size and skip
            const OBJECT_BASE* base = static_cast<const OBJECT_BASE*>( m_currentAddress );

            if( base->next != 0 && m_objectPointers.count( ByteSwap32( base->next ) ) > 0 )
            {
                // Skip to next object in chain
                m_currentAddress = m_objectPointers[ByteSwap32( base->next )];
            }
            else
            {
                // Skip minimum object size
                AdvanceReadPointer( sizeof( OBJECT_BASE ) );
            }
            break;
        }
    }

    wxLogTrace( ALLEGRO_DBG, wxT( "Finished parsing %zu objects" ), m_objectPointers.size() );
}

void ALLEGRO_PARSER::ParseArc( const void* aData )
{
    const ARC_DATA* arc = static_cast<const ARC_DATA*>( aData );
    uint32_t key = ByteSwap32( arc->key );

    m_objectPointers[key] = const_cast<void*>( aData );

    size_t objectSize = sizeof( ARC_DATA );
    if( !m_hasVersion172Extensions )
        objectSize -= sizeof( uint32_t );  // No unknown2 field

    AdvanceReadPointer( objectSize );

    wxLogTrace( ALLEGRO_DBG, wxT( "Parsed arc with key 0x%08X" ), key );
}

void ALLEGRO_PARSER::ParseSegment( const void* aData )
{
    const SEGMENT_DATA* segment = static_cast<const SEGMENT_DATA*>( aData );
    uint32_t key = ByteSwap32( segment->key );

    m_objectPointers[key] = const_cast<void*>( aData );

    size_t objectSize = sizeof( SEGMENT_DATA );
    if( !m_hasVersion172Extensions )
        objectSize -= sizeof( uint32_t );  // No unknown field

    AdvanceReadPointer( objectSize );

    wxLogTrace( ALLEGRO_DBG, wxT( "Parsed segment with key 0x%08X" ), key );
}

void ALLEGRO_PARSER::ParseNetAssignment( const void* aData )
{
    const NET_ASSIGNMENT_DATA* assignment = static_cast<const NET_ASSIGNMENT_DATA*>( aData );
    uint32_t key = ByteSwap32( assignment->key );

    m_objectPointers[key] = const_cast<void*>( aData );

    size_t objectSize = sizeof( NET_ASSIGNMENT_DATA );
    if( !m_hasVersion174Extensions )
        objectSize -= sizeof( uint32_t );  // No unknown field

    AdvanceReadPointer( objectSize );

    wxLogTrace( ALLEGRO_DBG, wxT( "Parsed net assignment with key 0x%08X" ), key );
}

void ALLEGRO_PARSER::ParseTrack( const void* aData )
{
    const TRACK_DATA* track = static_cast<const TRACK_DATA*>( aData );
    uint32_t key = ByteSwap32( track->key );

    m_objectPointers[key] = const_cast<void*>( aData );

    size_t objectSize = sizeof( TRACK_DATA );
    if( !m_hasVersion172Extensions )
        objectSize -= sizeof( uint32_t ) * 3;  // No unknown3 array

    AdvanceReadPointer( objectSize );

    wxLogTrace( ALLEGRO_DBG, wxT( "Parsed track with key 0x%08X, layer %d:%d" ),
                key, static_cast<int>( track->layer.family ), track->layer.ordinal );
}

void ALLEGRO_PARSER::ParsePad( const void* aData )
{
    const PAD_DATA* pad = static_cast<const PAD_DATA*>( aData );
    uint32_t key = ByteSwap32( pad->key );

    m_objectPointers[key] = const_cast<void*>( aData );

    size_t objectSize = sizeof( PAD_DATA );
    if( !m_hasVersion174Extensions )
        objectSize -= sizeof( uint32_t );  // No unknown1
    if( !m_hasVersion172Extensions )
        objectSize -= sizeof( uint32_t );  // No unknown3

    AdvanceReadPointer( objectSize );

    wxLogTrace( ALLEGRO_DBG, wxT( "Parsed pad with key 0x%08X" ), key );
}

void ALLEGRO_PARSER::ParseInstance( const void* aData )
{
    const INSTANCE_DATA* instance = static_cast<const INSTANCE_DATA*>( aData );
    uint32_t key = ByteSwap32( instance->key );

    m_objectPointers[key] = const_cast<void*>( aData );

    size_t objectSize = sizeof( INSTANCE_DATA );
    if( m_hasVersion172Extensions )
    {
        objectSize += sizeof( uint32_t ) * 2;  // Add ptr0, unknown2, unknown3
        objectSize -= sizeof( uint32_t );      // Remove unknown4
    }

    AdvanceReadPointer( objectSize );

    wxLogTrace( ALLEGRO_DBG, wxT( "Parsed instance with key 0x%08X" ), key );
}

void ALLEGRO_PARSER::ParseNet( const void* aData )
{
    const NET_DATA* net = static_cast<const NET_DATA*>( aData );
    uint32_t key = ByteSwap32( net->key );

    m_objectPointers[key] = const_cast<void*>( aData );

    size_t objectSize = sizeof( NET_DATA );
    if( !m_hasVersion172Extensions )
        objectSize -= sizeof( uint32_t );  // No unknown2

    AdvanceReadPointer( objectSize );

    wxLogTrace( ALLEGRO_DBG, wxT( "Parsed net with key 0x%08X, name: %s" ),
                key, GetString( ByteSwap32( net->netNameStringPtr ) ).c_str() );
}

void ALLEGRO_PARSER::ParsePadStack( const void* aData )
{
    const PAD_STACK_DATA* padStack = static_cast<const PAD_STACK_DATA*>( aData );
    uint32_t key = ByteSwap32( padStack->key );

    m_objectPointers[key] = const_cast<void*>( aData );

    // Calculate base size
    size_t objectSize = sizeof( PAD_STACK_DATA );

    // Adjust for version differences
    if( m_hasVersion172Extensions )
    {
        objectSize -= sizeof( uint32_t ) * 4;  // Remove unknown4
        objectSize += sizeof( uint32_t ) * 3;  // Add unknown5
        objectSize -= sizeof( uint16_t );      // Remove unknown6
        objectSize += sizeof( uint16_t );      // Add unknown7
        objectSize += sizeof( uint32_t ) * 28; // Add unknown9
    }

    if( m_hasVersion165Extensions )
    {
        objectSize += sizeof( uint32_t ) * 8;  // Add unknown10
    }

    // Add size for pad stack components
    uint16_t componentCount = 10 + padStack->layerCount * 3;
    if( m_hasVersion172Extensions )
        componentCount = 21 + padStack->layerCount * 4;

    size_t componentSize = sizeof( PAD_STACK_COMPONENT );
    if( m_hasVersion172Extensions )
    {
        componentSize += sizeof( uint32_t ) * 3;  // Add unknown4, unknown5, unknown6
        componentSize -= sizeof( uint32_t );      // Remove unknown7
    }

    objectSize += componentCount * componentSize;

    // Add trailing data
    if( m_hasVersion172Extensions )
        objectSize += padStack->n * 40;
    else
        objectSize += padStack->n * 32 - 4;

    AdvanceReadPointer( objectSize );

    wxLogTrace( ALLEGRO_DBG, wxT( "Parsed pad stack with key 0x%08X, %d layers" ),
                key, padStack->layerCount );
}

void ALLEGRO_PARSER::ParseShape( const void* aData )
{
    const SHAPE_DATA* shape = static_cast<const SHAPE_DATA*>( aData );
    uint32_t key = ByteSwap32( shape->key );

    m_objectPointers[key] = const_cast<void*>( aData );

    size_t objectSize = sizeof( SHAPE_DATA );
    if( !m_hasVersion172Extensions )
    {
        objectSize -= sizeof( uint32_t ) * 2;  // No unknown2
        objectSize -= sizeof( uint32_t );      // No ptr7
        objectSize += sizeof( uint32_t );      // Has ptr7Alt
    }

    AdvanceReadPointer( objectSize );

    wxLogTrace( ALLEGRO_DBG, wxT( "Parsed shape with key 0x%08X, layer %d:%d" ),
                key, static_cast<int>( shape->layer.family ), shape->layer.ordinal );
}

void ALLEGRO_PARSER::ParseFootprint( const void* aData )
{
    const FOOTPRINT_DATA* footprint = static_cast<const FOOTPRINT_DATA*>( aData );
    uint32_t key = ByteSwap32( footprint->key );

    m_objectPointers[key] = const_cast<void*>( aData );

    size_t objectSize = sizeof( FOOTPRINT_DATA );
    if( !m_hasVersion172Extensions )
        objectSize -= sizeof( uint32_t );  // No unknown3
    if( static_cast<uint32_t>( m_fileVersion ) < 0x00130C00 )  // Before v16.4
        objectSize -= sizeof( uint32_t );  // No unknown2

    AdvanceReadPointer( objectSize );

    wxLogTrace( ALLEGRO_DBG, wxT( "Parsed footprint with key 0x%08X" ), key );
}

void ALLEGRO_PARSER::ParsePlacedFootprint( const void* aData )
{
    const PLACED_FOOTPRINT_DATA* placed = static_cast<const PLACED_FOOTPRINT_DATA*>( aData );
    uint32_t key = ByteSwap32( placed->key );

    m_objectPointers[key] = const_cast<void*>( aData );

    size_t objectSize = sizeof( PLACED_FOOTPRINT_DATA );

    // Complex size calculation based on version
    if( m_hasVersion172Extensions )
    {
        objectSize += sizeof( uint32_t ) * 2;  // Add unknown2, unknown5
    }

    AdvanceReadPointer( objectSize );

    wxLogTrace( ALLEGRO_DBG, wxT( "Parsed placed footprint with key 0x%08X" ), key );
}

void ALLEGRO_PARSER::ParseStringGraphic( const void* aData )
{
    const STRING_GRAPHIC_WRAPPER_DATA* wrapper = static_cast<const STRING_GRAPHIC_WRAPPER_DATA*>( aData );
    uint32_t key = ByteSwap32( wrapper->key );

    m_objectPointers[key] = const_cast<void*>( aData );

    size_t objectSize = sizeof( STRING_GRAPHIC_WRAPPER_DATA );

    if( m_hasVersion172Extensions )
    {
        objectSize += sizeof( uint32_t ) * 4;  // Add unknown1, unknown2, font, ptr3, ptr4
        objectSize -= sizeof( uint32_t ) * 2;  // Remove fontAlt, ptr3Alt
    }

    if( m_hasVersion174Extensions )
        objectSize += sizeof( uint32_t );  // Add unknown3

    AdvanceReadPointer( objectSize );

    wxLogTrace( ALLEGRO_DBG, wxT( "Parsed string graphic wrapper with key 0x%08X" ), key );
}

void ALLEGRO_PARSER::ParsePlacedPad( const void* aData )
{
    const PLACED_PAD_DATA* placed = static_cast<const PLACED_PAD_DATA*>( aData );
    uint32_t key = ByteSwap32( placed->key );

    m_objectPointers[key] = const_cast<void*>( aData );

    size_t objectSize = sizeof( PLACED_PAD_DATA );
    if( m_hasVersion172Extensions )
        objectSize += sizeof( uint32_t ) * 2;  // Add prev, unknown2

    AdvanceReadPointer( objectSize );

    wxLogTrace( ALLEGRO_DBG, wxT( "Parsed placed pad with key 0x%08X" ), key );
}

void ALLEGRO_PARSER::ParseVia( const void* aData )
{
    const VIA_DATA* via = static_cast<const VIA_DATA*>( aData );
    uint32_t key = ByteSwap32( via->key );

    m_objectPointers[key] = const_cast<void*>( aData );

    size_t objectSize = sizeof( VIA_DATA );
    if( m_hasVersion172Extensions )
        objectSize += sizeof( uint32_t ) * 2;  // Add unknown2, ptr7

    AdvanceReadPointer( objectSize );

    wxLogTrace( ALLEGRO_DBG, wxT( "Parsed via with key 0x%08X" ), key );
}

void ALLEGRO_PARSER::ParseRectangle( const void* aData )
{
    const RECTANGLE_DATA* rectangle = static_cast<const RECTANGLE_DATA*>( aData );
    uint32_t key = ByteSwap32( rectangle->key );

    m_objectPointers[key] = const_cast<void*>( aData );

    size_t objectSize = sizeof( RECTANGLE_DATA );
    if( m_hasVersion172Extensions )
        objectSize += sizeof( uint32_t );  // Add unknown2

    AdvanceReadPointer( objectSize );

    wxLogTrace( ALLEGRO_DBG, wxT( "Parsed rectangle with key 0x%08X" ), key );
}

void ALLEGRO_PARSER::ParseRuleRegion( const void* aData )
{
    const RULE_REGION_DATA* region = static_cast<const RULE_REGION_DATA*>( aData );
    uint32_t key = ByteSwap32( region->key );

    m_objectPointers[key] = const_cast<void*>( aData );

    size_t objectSize = sizeof( RULE_REGION_DATA );
    if( m_hasVersion172Extensions )
        objectSize += sizeof( uint32_t );  // Add unknown1

    AdvanceReadPointer( objectSize );

    wxLogTrace( ALLEGRO_DBG, wxT( "Parsed rule region with key 0x%08X" ), key );
}

void ALLEGRO_PARSER::ParseFontData( const void* aData )
{
    const FONT_DATA_CONTAINER* container = static_cast<const FONT_DATA_CONTAINER*>( aData );
    uint32_t key = ByteSwap32( container->key );

    m_objectPointers[key] = const_cast<void*>( aData );

    size_t objectSize = sizeof( FONT_DATA_CONTAINER );
    if( m_hasVersion172Extensions )
        objectSize += sizeof( uint32_t );  // Add unknown1
    if( m_hasVersion174Extensions )
        objectSize += sizeof( uint32_t );  // Add unknown3

    // Parse font dimensions if this is a font subtype (0x08)
    if( container->subtype == 0x08 )
    {
        const void* fontData = static_cast<const char*>( aData ) + objectSize;

        for( uint32_t i = 0; i < container->size; i++ )
        {
            const FONT_DIMENSION_DATA* font = static_cast<const FONT_DIMENSION_DATA*>( fontData );

            // Store font data by index
            m_fontData[i] = font;

            size_t fontSize = sizeof( FONT_DIMENSION_DATA );
            if( !m_hasVersion174Extensions )
                fontSize -= sizeof( uint32_t );  // No unknown1
            if( !m_hasVersion172Extensions )
                fontSize -= sizeof( uint32_t ) * 8;  // No ys array

            fontData = static_cast<const char*>( fontData ) + fontSize;
        }

        objectSize += container->size * sizeof( FONT_DIMENSION_DATA );
    }
    else
    {
        // Skip unknown font data types
        objectSize += container->size * 4;  // Minimum size guess
    }

    AdvanceReadPointer( objectSize );

    wxLogTrace( ALLEGRO_DBG, wxT( "Parsed font data with key 0x%08X, subtype 0x%02X" ),
                key, container->subtype );
}

void ALLEGRO_PARSER::ParseLayerInfo( const void* aData )
{
    const LAYER_SET_HEADER* header = static_cast<const LAYER_SET_HEADER*>( aData );

    size_t objectSize = sizeof( LAYER_SET_HEADER );

    if( m_hasVersion174Extensions )
        objectSize += sizeof( uint32_t );  // Extra field

    AdvanceReadPointer( objectSize );

    // Parse layer entries
    std::vector<uint8_t> layerData;

    for( uint16_t i = 0; i < header->count; i++ )
    {
        if( static_cast<uint32_t>( m_fileVersion ) <= 0x00130C00 )  // v16.4 and earlier
        {
            const LOCAL_LAYER_PROPERTIES* props =
                static_cast<const LOCAL_LAYER_PROPERTIES*>( m_currentAddress );

            layerData.insert( layerData.end(),
                            reinterpret_cast<const uint8_t*>( props ),
                            reinterpret_cast<const uint8_t*>( props ) + sizeof( *props ) );

            AdvanceReadPointer( sizeof( LOCAL_LAYER_PROPERTIES ) );
        }
        else
        {
            const REFERENCE_LAYER_PROPERTIES* props =
                static_cast<const REFERENCE_LAYER_PROPERTIES*>( m_currentAddress );

            layerData.insert( layerData.end(),
                            reinterpret_cast<const uint8_t*>( props ),
                            reinterpret_cast<const uint8_t*>( props ) + sizeof( *props ) );

            AdvanceReadPointer( sizeof( REFERENCE_LAYER_PROPERTIES ) );
        }
    }

    // Get layer set key
    uint32_t layerSetKey = *static_cast<const uint32_t*>( m_currentAddress );
    AdvanceReadPointer( sizeof( uint32_t ) );

    m_layerSets[ByteSwap32( layerSetKey )] = layerData;

    wxLogTrace( ALLEGRO_DBG, wxT( "Parsed layer set with key 0x%08X, %d layers" ),
                ByteSwap32( layerSetKey ), header->count );
}

void ALLEGRO_PARSER::ParseModelInfo( const void* aData )
{
    const MODEL_INFO_HEADER* header = static_cast<const MODEL_INFO_HEADER*>( aData );
    uint32_t key = ByteSwap32( header->key );

    m_objectPointers[key] = const_cast<void*>( aData );

    size_t objectSize = sizeof( MODEL_INFO_HEADER );

    // Skip model data
    objectSize += RoundToWord( header->size );

    if( m_hasVersion172Extensions )
        objectSize += sizeof( uint32_t );

    AdvanceReadPointer( objectSize );

    wxLogTrace( ALLEGRO_DBG, wxT( "Parsed model info with key 0x%08X, size %u" ),
                key, header->size );
}

void ALLEGRO_PARSER::ParseMetadata( const void* aData )
{
    // Metadata has variable structure, skip for now
    const uint16_t* type = static_cast<const uint16_t*>( aData );
    const uint16_t* subtype = reinterpret_cast<const uint16_t*>(
        static_cast<const char*>( aData ) + sizeof( uint16_t ) );
    const uint32_t* size = reinterpret_cast<const uint32_t*>(
        static_cast<const char*>( aData ) + sizeof( uint16_t ) * 2 );

    size_t objectSize = sizeof( uint16_t ) * 2 + sizeof( uint32_t ) * 2 + *size;

    AdvanceReadPointer( objectSize );

    wxLogTrace( ALLEGRO_DBG, wxT( "Parsed metadata type 0x%04X, subtype 0x%04X, size %u" ),
                *type, *subtype, *size );
}

void ALLEGRO_PARSER::BuildBoard()
{
    wxLogTrace( ALLEGRO_DBG, wxT( "Building board from parsed data" ) );

    UpdateLayerConfiguration();
    ProcessNets();
    ProcessFootprints();
    ProcessShapes();
    ProcessAnnotations();
    ProcessZones();

    // Heal board outlines
    std::vector<PCB_SHAPE*> shapes;

    for( BOARD_ITEM* item : m_board->Drawings() )
    {
        if( item->IsOnLayer( Edge_Cuts ) && item->Type() == PCB_SHAPE_T )
            shapes.push_back( static_cast<PCB_SHAPE*>( item ) );
    }

    ConnectBoardShapes( shapes, SHAPE_JOIN_DISTANCE );

    for( auto& shape : shapes )
        m_board->Add( shape, ADD_MODE::APPEND );

    wxLogTrace( ALLEGRO_DBG, wxT( "Board construction complete" ) );
}

void ALLEGRO_PARSER::UpdateLayerConfiguration()
{
    wxLogTrace( ALLEGRO_DBG, wxT( "Updating layer configuration" ) );

    // Get copper layer set from header
    LAYER_MAP_ENTRY copperEntry = m_header->layerSets[static_cast<uint8_t>( LAYER_FAMILY::COPPER )];
    uint32_t layerSetPtr = ByteSwap32( copperEntry.layerSetPtr );

    if( m_layerSets.count( layerSetPtr ) == 0 )
    {
        wxLogTrace( ALLEGRO_DBG, wxT( "No copper layer set found" ) );
        return;
    }

    const std::vector<uint8_t>& layerData = m_layerSets[layerSetPtr];

    if( static_cast<uint32_t>( m_fileVersion ) <= 0x00130C00 )  // v16.4 and earlier
    {
        size_t entrySize = sizeof( LOCAL_LAYER_PROPERTIES );
        m_copperLayerCount = layerData.size() / entrySize;

        const LOCAL_LAYER_PROPERTIES* props =
            reinterpret_cast<const LOCAL_LAYER_PROPERTIES*>( layerData.data() );

        for( int i = 0; i < m_copperLayerCount; i++ )
        {
            PCB_LAYER_ID layer = ConvertLayer( { LAYER_FAMILY::COPPER, static_cast<uint8_t>( i ) } );
            m_board->SetLayerName( layer, wxString( props[i].layerName ) );

            wxLogTrace( ALLEGRO_DBG, wxT( "Layer %d: %s" ), i, props[i].layerName );
        }
    }
    else
    {
        size_t entrySize = sizeof( REFERENCE_LAYER_PROPERTIES );
        m_copperLayerCount = layerData.size() / entrySize;

        const REFERENCE_LAYER_PROPERTIES* props =
            reinterpret_cast<const REFERENCE_LAYER_PROPERTIES*>( layerData.data() );

        for( int i = 0; i < m_copperLayerCount; i++ )
        {
            PCB_LAYER_ID layer = ConvertLayer( { LAYER_FAMILY::COPPER, static_cast<uint8_t>( i ) } );
            wxString layerName = GetString( ByteSwap32( props[i].layerNameStringPtr ) );
            m_board->SetLayerName( layer, layerName );

            wxLogTrace( ALLEGRO_DBG, wxT( "Layer %d: %s" ), i, layerName );
        }
    }

    m_board->SetCopperLayerCount( m_copperLayerCount );
}

void ALLEGRO_PARSER::ProcessNets()
{
    wxLogTrace( ALLEGRO_DBG, wxT( "Processing nets" ) );

    // Iterate through net linked list
    uint32_t netPtr = ByteSwap32( m_header->nets.head );

    while( netPtr != 0 && netPtr != ByteSwap32( m_header->nets.tail ) )
    {
        if( !IsObjectType( netPtr, OBJECT_TYPE::NET ) )
            break;

        const NET_DATA* net = static_cast<const NET_DATA*>( GetObjectPointer( netPtr ) );

        wxString netName = GetString( ByteSwap32( net->netNameStringPtr ) );

        if( !netName.IsEmpty() )
        {
            NETINFO_ITEM* netInfo = new NETINFO_ITEM( m_board, netName,
                                                      m_board->GetNetCount() + 1 );
            m_board->Add( netInfo, ADD_MODE::APPEND );

            wxLogTrace( ALLEGRO_DBG, wxT( "Added net: %s" ), netName );
        }

        // Process tracks and vias for this net
        uint32_t assignmentPtr = ByteSwap32( net->netAssignmentPtr );

        while( assignmentPtr != 0 && assignmentPtr != netPtr )
        {
            if( !IsObjectType( assignmentPtr, OBJECT_TYPE::NET_ASSIGNMENT ) )
                break;

            const NET_ASSIGNMENT_DATA* assignment =
                static_cast<const NET_ASSIGNMENT_DATA*>( GetObjectPointer( assignmentPtr ) );

            uint32_t shapePtr = ByteSwap32( assignment->shapePtr );

            // Process connected objects (tracks, vias, shapes)
            while( shapePtr != 0 )
            {
                void* obj = GetObjectPointer( shapePtr );
                if( !obj )
                    break;

                if( IsObjectType( shapePtr, OBJECT_TYPE::TRACK ) )
                {
                    const TRACK_DATA* track = static_cast<const TRACK_DATA*>( obj );
                    CreateTrack( *net, *track );
                    shapePtr = ByteSwap32( track->ptr0 );
                }
                else if( IsObjectType( shapePtr, OBJECT_TYPE::VIA ) )
                {
                    const VIA_DATA* via = static_cast<const VIA_DATA*>( obj );
                    CreateVia( *via );
                    shapePtr = ByteSwap32( via->unknown1 );
                }
                else if( IsObjectType( shapePtr, OBJECT_TYPE::SHAPE ) )
                {
                    const SHAPE_DATA* shape = static_cast<const SHAPE_DATA*>( obj );
                    CreateZone( *m_board, *shape, netName );
                    shapePtr = ByteSwap32( shape->next );
                }
                else
                {
                    break;
                }
            }

            assignmentPtr = ByteSwap32( assignment->next );
        }

        netPtr = ByteSwap32( net->next );
    }
}

void ALLEGRO_PARSER::ProcessFootprints()
{
    wxLogTrace( ALLEGRO_DBG, wxT( "Processing footprints" ) );

    // Iterate through footprint linked list
    uint32_t footprintPtr = ByteSwap32( m_header->footprints.head );

    while( footprintPtr != 0 && footprintPtr != ByteSwap32( m_header->footprints.tail ) )
    {
        if( !IsObjectType( footprintPtr, OBJECT_TYPE::FOOTPRINT ) )
            break;

        const FOOTPRINT_DATA* footprintData =
            static_cast<const FOOTPRINT_DATA*>( GetObjectPointer( footprintPtr ) );

        wxString footprintName = GetString( ByteSwap32( footprintData->footprintStringPtr ) );

        // Process placed instances of this footprint
        uint32_t placedPtr = ByteSwap32( footprintData->placedSymbolPtr );

        while( placedPtr != 0 )
        {
            if( !IsObjectType( placedPtr, OBJECT_TYPE::PLACED_FOOTPRINT ) )
                break;

            const PLACED_FOOTPRINT_DATA* placed =
                static_cast<const PLACED_FOOTPRINT_DATA*>( GetObjectPointer( placedPtr ) );

            CreateFootprint( *placed );

            placedPtr = ByteSwap32( placed->next );
        }

        footprintPtr = ByteSwap32( footprintData->next );
    }
}

void ALLEGRO_PARSER::ProcessShapes()
{
    wxLogTrace( ALLEGRO_DBG, wxT( "Processing shapes" ) );

    // Process standalone shapes (not associated with nets)
    uint32_t shapePtr = ByteSwap32( m_header->shapes.head );

    while( shapePtr != 0 && shapePtr != ByteSwap32( m_header->shapes.tail ) )
    {
        void* obj = GetObjectPointer( shapePtr );
        if( !obj )
            break;

        if( IsObjectType( shapePtr, OBJECT_TYPE::SHAPE ) )
        {
            const SHAPE_DATA* shape = static_cast<const SHAPE_DATA*>( obj );
            CreateZone( *m_board, *shape );
            shapePtr = ByteSwap32( shape->next );
        }
        else if( IsObjectType( shapePtr, OBJECT_TYPE::UNKNOWN_0E ) )
        {
            // Skip unknown type
            const OBJECT_BASE* base = static_cast<const OBJECT_BASE*>( obj );
            shapePtr = ByteSwap32( base->next );
        }
        else
        {
            break;
        }
    }

    // Process rectangles
    uint32_t rectPtr = ByteSwap32( m_header->rectangleShapes.head );

    while( rectPtr != 0 && rectPtr != ByteSwap32( m_header->rectangleShapes.tail ) )
    {
        void* obj = GetObjectPointer( rectPtr );
        if( !obj )
            break;

        if( IsObjectType( rectPtr, OBJECT_TYPE::RECTANGLE ) )
        {
            const RECTANGLE_DATA* rectangle = static_cast<const RECTANGLE_DATA*>( obj );
            CreateRectangle( *m_board, *rectangle );
            rectPtr = ByteSwap32( rectangle->next );
        }
        else if( IsObjectType( rectPtr, OBJECT_TYPE::SHAPE ) )
        {
            const SHAPE_DATA* shape = static_cast<const SHAPE_DATA*>( obj );
            CreateZone( *m_board, *shape );
            rectPtr = ByteSwap32( shape->next );
        }
        else
        {
            break;
        }
    }
}

void ALLEGRO_PARSER::ProcessAnnotations()
{
    wxLogTrace( ALLEGRO_DBG, wxT( "Processing annotations" ) );

    // Process annotation linked list
    uint32_t annotationPtr = ByteSwap32( m_header->annotations.head );

    while( annotationPtr != 0 && annotationPtr != ByteSwap32( m_header->annotations.tail ) )
    {
        if( !IsObjectType( annotationPtr, OBJECT_TYPE::ANNOTATION ) )
            break;

        CreateAnnotation( *m_board, annotationPtr );

        const OBJECT_BASE* base = static_cast<const OBJECT_BASE*>( GetObjectPointer( annotationPtr ) );
        annotationPtr = ByteSwap32( base->next );
    }

    // Process text strings
    uint32_t textPtr = ByteSwap32( m_header->stringsAndUnknown.head );

    while( textPtr != 0 && textPtr != ByteSwap32( m_header->stringsAndUnknown.tail ) )
    {
        void* obj = GetObjectPointer( textPtr );
        if( !obj )
            break;

        if( IsObjectType( textPtr, OBJECT_TYPE::STRING_GRAPHIC_WRAPPER ) )
        {
            const STRING_GRAPHIC_WRAPPER_DATA* wrapper =
                static_cast<const STRING_GRAPHIC_WRAPPER_DATA*>( obj );
            // TODO: This needs to be converted to add board text, not footprint text
            // AddFootprintText( m_board, *wrapper );
            textPtr = ByteSwap32( wrapper->next );
        }
        else if( IsObjectType( textPtr, OBJECT_TYPE::UNKNOWN_03 ) )
        {
            const OBJECT_BASE* base = static_cast<const OBJECT_BASE*>( obj );
            textPtr = ByteSwap32( base->next );
        }
        else
        {
            break;
        }
    }
}

void ALLEGRO_PARSER::ProcessZones()
{
    // Zones are processed as part of shapes and net processing
    wxLogTrace( ALLEGRO_DBG, wxT( "Zone processing integrated with shape processing" ) );
}

void ALLEGRO_PARSER::CreateFootprint( const PLACED_FOOTPRINT_DATA& aPlacedFootprint )
{
    // Get footprint definition
    uint32_t footprintKey = ByteSwap32( aPlacedFootprint.next );

    if( !IsObjectType( footprintKey, OBJECT_TYPE::FOOTPRINT ) )
    {
        wxLogTrace( ALLEGRO_DBG, wxT( "Invalid footprint reference" ) );
        return;
    }

    const FOOTPRINT_DATA* footprintData =
        static_cast<const FOOTPRINT_DATA*>( GetObjectPointer( footprintKey ) );

    wxString footprintName = GetString( ByteSwap32( footprintData->footprintStringPtr ) );

    std::unique_ptr<FOOTPRINT> footprint = std::make_unique<FOOTPRINT>( m_board );

    // Set footprint properties
    LIB_ID libId( wxT( "allegro_lib" ), footprintName );
    footprint->SetFPID( libId );

    // Get reference designator
    uint32_t instanceRef = ByteSwap32( aPlacedFootprint.instanceRef );
    if( !m_hasVersion172Extensions )
        instanceRef = ByteSwap32( aPlacedFootprint.instanceRefAlt );

    if( instanceRef != 0 && IsObjectType( instanceRef, OBJECT_TYPE::INSTANCE ) )
    {
        const INSTANCE_DATA* instance =
            static_cast<const INSTANCE_DATA*>( GetObjectPointer( instanceRef ) );

        wxString refdes = GetString( ByteSwap32( instance->refdesStringPtr ) );
        footprint->SetReference( refdes );
    }
    else
    {
        footprint->SetReference( wxT( "REF**" ) );
    }

    footprint->Reference().SetVisible( false );
    footprint->Value().SetText( footprintName );
    footprint->Value().SetVisible( false );

    // Set position and orientation
    VECTOR2I position = ScalePosition( aPlacedFootprint.coords );
    footprint->SetPosition( position );

    EDA_ANGLE orientation = ScaleAngle( ByteSwap32( aPlacedFootprint.rotation ) );
    footprint->SetOrientationDegrees( orientation.AsDegrees() );

    // Set layer
    if( aPlacedFootprint.layer == 0 )
        footprint->SetLayer( F_Cu );
    else
        footprint->SetLayer( B_Cu );

    // Add pads
    uint32_t padPtr = ByteSwap32( aPlacedFootprint.firstPadPtr );

    while( padPtr != 0 && IsObjectType( padPtr, OBJECT_TYPE::PLACED_PAD ) )
    {
        const PLACED_PAD_DATA* placedPad =
            static_cast<const PLACED_PAD_DATA*>( GetObjectPointer( padPtr ) );

        AddFootprintPad( footprint.get(), *placedPad );

        padPtr = ByteSwap32( placedPad->next );
    }

    // Add graphics
    uint32_t graphicsPtr = ByteSwap32( aPlacedFootprint.annotationPtr );

    while( graphicsPtr != 0 && IsObjectType( graphicsPtr, OBJECT_TYPE::ANNOTATION ) )
    {
        AddFootprintGraphics( footprint.get(), graphicsPtr );

        const OBJECT_BASE* base = static_cast<const OBJECT_BASE*>( GetObjectPointer( graphicsPtr ) );
        graphicsPtr = ByteSwap32( base->next );
    }

    // Add text
    uint32_t textPtr = ByteSwap32( aPlacedFootprint.ptr3 );

    while( textPtr != 0 )
    {
        void* obj = GetObjectPointer( textPtr );
        if( !obj )
            break;

        if( IsObjectType( textPtr, OBJECT_TYPE::STRING_GRAPHIC_WRAPPER ) )
        {
            const STRING_GRAPHIC_WRAPPER_DATA* wrapper =
                static_cast<const STRING_GRAPHIC_WRAPPER_DATA*>( obj );
            AddFootprintText( footprint.get(), *wrapper );
            textPtr = ByteSwap32( wrapper->next );
        }
        else if( IsObjectType( textPtr, OBJECT_TYPE::UNKNOWN_03 ) )
        {
            const OBJECT_BASE* base = static_cast<const OBJECT_BASE*>( obj );
            textPtr = ByteSwap32( base->next );
        }
        else
        {
            break;
        }
    }

    // Add zones
    for( int i = 0; i < 3; i++ )
    {
        uint32_t zonePtr = ByteSwap32( aPlacedFootprint.ptr4[i] );

        while( zonePtr != 0 )
        {
            void* obj = GetObjectPointer( zonePtr );
            if( !obj )
                break;

            if( IsObjectType( zonePtr, OBJECT_TYPE::SHAPE ) )
            {
                const SHAPE_DATA* shape = static_cast<const SHAPE_DATA*>( obj );
                AddFootprintZone( footprint.get(), *shape );
                zonePtr = ByteSwap32( shape->next );
            }
            else if( IsObjectType( zonePtr, OBJECT_TYPE::UNKNOWN_0E ) )
            {
                const OBJECT_BASE* base = static_cast<const OBJECT_BASE*>( obj );
                zonePtr = ByteSwap32( base->next );
            }
            else
            {
                break;
            }
        }
    }

    // Store Allegro ID as a field
    PCB_FIELD* idField = footprint->GetField( FIELD_T::USER );
    idField->SetName( ALLEGRO_ID_FIELD );
    idField->SetText( wxString::Format( wxT( "0x%08X" ), ByteSwap32( aPlacedFootprint.key ) ) );
    idField->SetVisible( false );

    m_board->Add( footprint.release(), ADD_MODE::APPEND );

    wxLogTrace( ALLEGRO_DBG, wxT( "Created footprint %s at (%d, %d)" ),
                footprintName, position.x, position.y );
}

void ALLEGRO_PARSER::AddFootprintPad( FOOTPRINT* aFootprint, const PLACED_PAD_DATA& aPlacedPad )
{
    // Get pad definition
    uint32_t padKey = ByteSwap32( aPlacedPad.padPtr );

    if( !IsObjectType( padKey, OBJECT_TYPE::PAD ) )
    {
        wxLogTrace( ALLEGRO_DBG, wxT( "Invalid pad reference" ) );
        return;
    }

    const PAD_DATA* padData = static_cast<const PAD_DATA*>( GetObjectPointer( padKey ) );

    std::unique_ptr<PAD> pad = std::make_unique<PAD>( aFootprint );

    // Set pad number
    wxString padNumber = GetString( ByteSwap32( padData->stringPtr ) );
    pad->SetNumber( padNumber );

    // Set position relative to footprint
    VECTOR2I position = ScalePosition( padData->coords );
    pad->SetPosition( position );

    // Set orientation
    EDA_ANGLE orientation = ScaleAngle( ByteSwap32( padData->rotation ) );
    pad->SetOrientationDegrees( orientation.AsDegrees() );

    // Get pad stack for shape information
    uint32_t padStackKey = ByteSwap32( padData->padStackPtr );

    if( !IsObjectType( padStackKey, OBJECT_TYPE::PAD_STACK ) )
    {
        wxLogTrace( ALLEGRO_DBG, wxT( "Invalid pad stack reference" ) );
        aFootprint->Add( pad.release(), ADD_MODE::APPEND );
        return;
    }

    const PAD_STACK_DATA* padStack =
        static_cast<const PAD_STACK_DATA*>( GetObjectPointer( padStackKey ) );

    // Set pad type based on pad stack info
    switch( padStack->padInfo.type )
    {
    case PAD_TYPE::SMD_PIN:
    case PAD_TYPE::SMD_PIN_ALT:
        pad->SetAttribute( PAD_ATTRIB::SMD );
        pad->SetLayerSet( PAD::SMDMask() );
        break;

    case PAD_TYPE::THROUGH_HOLE:
    case PAD_TYPE::VIA:
    case PAD_TYPE::SLOT:
        pad->SetAttribute( PAD_ATTRIB::PTH );
        pad->SetLayerSet( PAD::PTHMask() );
        break;

    case PAD_TYPE::NON_PLATED_HOLE:
        pad->SetAttribute( PAD_ATTRIB::NPTH );
        pad->SetLayerSet( PAD::UnplatedHoleMask() );
        break;

    default:
        wxLogTrace( ALLEGRO_DBG, wxT( "Unknown pad type %d" ),
                   static_cast<int>( padStack->padInfo.type ) );
        break;
    }

    // Set pad shape from pad stack component
    // The pad shape is determined by layer-specific components in the pad stack
    uint8_t componentOffset = 23;  // Default top layer offset
    if( !m_hasVersion172Extensions )
        componentOffset = 12;

    PAD_STACK_COMPONENT* component = GetPadComponent( *padStack, componentOffset );

    if( component && component->shapeType != 0 )
    {
        SetupPadShape( *pad, *component );
    }

    // Set net
    uint32_t netPtr = ByteSwap32( aPlacedPad.netPtr );

    if( netPtr != 0 && IsObjectType( netPtr, OBJECT_TYPE::NET_ASSIGNMENT ) )
    {
        const NET_ASSIGNMENT_DATA* assignment =
            static_cast<const NET_ASSIGNMENT_DATA*>( GetObjectPointer( netPtr ) );

        NETINFO_ITEM* net = GetNetFromAssignment( *assignment );
        pad->SetNet( net );
    }

    aFootprint->Add( pad.release(), ADD_MODE::APPEND );
}

void ALLEGRO_PARSER::AddFootprintGraphics( FOOTPRINT* aFootprint, uint32_t aGraphicsPtr )
{
    CreateAnnotation( *aFootprint, aGraphicsPtr );
}

void ALLEGRO_PARSER::AddFootprintText( FOOTPRINT* aFootprint, const STRING_GRAPHIC_WRAPPER_DATA& aTextWrapper )
{
    uint32_t graphicKey = ByteSwap32( aTextWrapper.stringGraphicPtr );

    if( !IsObjectType( graphicKey, OBJECT_TYPE::STRING_GRAPHIC ) )
    {
        wxLogTrace( ALLEGRO_DBG, wxT( "Invalid string graphic reference" ) );
        return;
    }

    const STRING_GRAPHIC_DATA* graphic =
        static_cast<const STRING_GRAPHIC_DATA*>( GetObjectPointer( graphicKey ) );

    // Get text string
    const char* text = reinterpret_cast<const char*>( graphic ) + sizeof( STRING_GRAPHIC_DATA );
    if( m_hasVersion174Extensions )
        text += sizeof( uint32_t );

    if( strlen( text ) == 0 )
        return;

    std::unique_ptr<PCB_TEXT> pcbText = std::make_unique<PCB_TEXT>( aFootprint );

    pcbText->SetText( wxString( text ) );

    // Set layer
    PCB_LAYER_ID layer = User_2;

    switch( graphic->layer )
    {
    case STRING_LAYER::TOP_TEXT:
    case STRING_LAYER::TOP_PIN:
    case STRING_LAYER::TOP_REFDES:
        layer = F_SilkS;
        break;

    case STRING_LAYER::BOT_TEXT:
    case STRING_LAYER::BOT_PIN:
    case STRING_LAYER::BOT_REFDES:
        layer = B_SilkS;
        break;

    case STRING_LAYER::TOP_PIN_LABEL:
        layer = User_5;
        break;

    default:
        break;
    }

    pcbText->SetLayer( layer );

    // Set position
    VECTOR2I position = ScalePosition( aTextWrapper.coords );
    pcbText->SetPosition( position );

    // Set orientation
    EDA_ANGLE angle = ScaleAngle( ByteSwap32( aTextWrapper.rotation ) );
    pcbText->SetTextAngleDegrees( angle.AsDegrees() );

    // Set text properties
    TEXT_PROPERTIES textProps;

    if( m_hasVersion172Extensions )
    {
        textProps = *reinterpret_cast<const TEXT_PROPERTIES*>( &aTextWrapper.font );
    }
    else
    {
        textProps = *reinterpret_cast<const TEXT_PROPERTIES*>( &aTextWrapper.fontAlt );
    }

    // Set alignment
    switch( textProps.alignment )
    {
    case TEXT_ALIGNMENT::TOP_LEFT:
        pcbText->SetVertJustify( GR_TEXT_V_ALIGN_TOP );
        pcbText->SetHorizJustify( GR_TEXT_H_ALIGN_LEFT );
        break;
    case TEXT_ALIGNMENT::CENTER_LEFT:
        pcbText->SetVertJustify( GR_TEXT_V_ALIGN_CENTER );
        pcbText->SetHorizJustify( GR_TEXT_H_ALIGN_LEFT );
        break;
    case TEXT_ALIGNMENT::BOTTOM_LEFT:
        pcbText->SetVertJustify( GR_TEXT_V_ALIGN_BOTTOM );
        pcbText->SetHorizJustify( GR_TEXT_H_ALIGN_LEFT );
        break;
    case TEXT_ALIGNMENT::TOP_CENTER:
        pcbText->SetVertJustify( GR_TEXT_V_ALIGN_TOP );
        pcbText->SetHorizJustify( GR_TEXT_H_ALIGN_CENTER );
        break;
    case TEXT_ALIGNMENT::CENTER:
        pcbText->SetVertJustify( GR_TEXT_V_ALIGN_CENTER );
        pcbText->SetHorizJustify( GR_TEXT_H_ALIGN_CENTER );
        break;
    case TEXT_ALIGNMENT::BOTTOM_CENTER:
        pcbText->SetVertJustify( GR_TEXT_V_ALIGN_BOTTOM );
        pcbText->SetHorizJustify( GR_TEXT_H_ALIGN_CENTER );
        break;
    case TEXT_ALIGNMENT::TOP_RIGHT:
        pcbText->SetVertJustify( GR_TEXT_V_ALIGN_TOP );
        pcbText->SetHorizJustify( GR_TEXT_H_ALIGN_RIGHT );
        break;
    case TEXT_ALIGNMENT::CENTER_RIGHT:
        pcbText->SetVertJustify( GR_TEXT_V_ALIGN_CENTER );
        pcbText->SetHorizJustify( GR_TEXT_H_ALIGN_RIGHT );
        break;
    case TEXT_ALIGNMENT::BOTTOM_RIGHT:
        pcbText->SetVertJustify( GR_TEXT_V_ALIGN_BOTTOM );
        pcbText->SetHorizJustify( GR_TEXT_H_ALIGN_RIGHT );
        break;
    }

    // Set mirrored if reversed
    if( textProps.reversal == TEXT_REVERSAL::REVERSED )
        pcbText->SetMirrored( true );

    // Set font size from font data
    const FONT_DIMENSION_DATA* fontData = GetFontData( textProps.fontKey );

    if( fontData )
    {
        VECTOR2I textSize( ScaleSize( static_cast<int>( fontData->charWidth ) ),
                          ScaleSize( static_cast<int>( fontData->charHeight ) ) );
        pcbText->SetTextSize( textSize );
    }

    aFootprint->Add( pcbText.release(), ADD_MODE::APPEND );
}

void ALLEGRO_PARSER::AddFootprintZone( FOOTPRINT* aFootprint, const SHAPE_DATA& aShape )
{
    CreateZone( *aFootprint, aShape );
}

void ALLEGRO_PARSER::CreateTrack( const NET_DATA& aNet, const TRACK_DATA& aTrack )
{
    wxString netName = GetString( ByteSwap32( aNet.netNameStringPtr ) );
    NETINFO_ITEM* netInfo = GetOrCreateNet( netName );
    PCB_LAYER_ID layer = ConvertLayer( aTrack.layer );

    // Process track segments
    uint32_t segmentPtr = ByteSwap32( aTrack.firstSegmentPtr );

    while( segmentPtr != 0 )
    {
        void* obj = GetObjectPointer( segmentPtr );
        if( !obj )
            break;

        if( IsObjectType( segmentPtr, OBJECT_TYPE::ARC ) )
        {
            const ARC_DATA* arc = static_cast<const ARC_DATA*>( obj );

            auto [start, end] = ParseArcCoordinates( *arc );

            VECTOR2D center( ScaleSize( ConvertCadenceFloat( arc->centerX ) ),
                           ScaleSize( -ConvertCadenceFloat( arc->centerY ) ) );

            VECTOR2I mid = CalculateArcMidpoint( start, end, center );

            SHAPE_ARC shapeArc;
            shapeArc.ConstructFromStartEndCenter( start, end, center,
                                                 arc->subtype == 0x00, 0 );

            std::unique_ptr<PCB_ARC> pcbArc = std::make_unique<PCB_ARC>( m_board, &shapeArc );

            pcbArc->SetLayer( layer );
            pcbArc->SetWidth( ScaleSize( static_cast<int>( ByteSwap32( arc->width ) ) ) );
            pcbArc->SetNet( netInfo );

            m_board->Add( pcbArc.release(), ADD_MODE::APPEND );

            segmentPtr = ByteSwap32( arc->next );
        }
        else if( IsObjectType( segmentPtr, OBJECT_TYPE::SEGMENT_15 ) ||
                IsObjectType( segmentPtr, OBJECT_TYPE::SEGMENT_16 ) ||
                IsObjectType( segmentPtr, OBJECT_TYPE::SEGMENT_17 ) )
        {
            const SEGMENT_DATA* segment = static_cast<const SEGMENT_DATA*>( obj );

            auto [start, end] = ParseSegmentCoordinates( *segment );

            std::unique_ptr<PCB_TRACK> track = std::make_unique<PCB_TRACK>( m_board );

            track->SetLayer( layer );
            track->SetWidth( ScaleSize( static_cast<int>( ByteSwap32( segment->width ) ) ) );
            track->SetStart( start );
            track->SetEnd( end );
            track->SetNet( netInfo );

            m_board->Add( track.release(), ADD_MODE::APPEND );

            segmentPtr = ByteSwap32( segment->next );
        }
        else
        {
            break;
        }
    }
}

void ALLEGRO_PARSER::CreateVia( const VIA_DATA& aVia )
{
    std::unique_ptr<PCB_VIA> via = std::make_unique<PCB_VIA>( m_board );

    VECTOR2I position = ScalePosition( aVia.coords );
    via->SetPosition( position );

    // Get net
    uint32_t netPtr = ByteSwap32( aVia.netPtr );

    if( netPtr != 0 && IsObjectType( netPtr, OBJECT_TYPE::NET_ASSIGNMENT ) )
    {
        const NET_ASSIGNMENT_DATA* assignment =
            static_cast<const NET_ASSIGNMENT_DATA*>( GetObjectPointer( netPtr ) );

        NETINFO_ITEM* net = GetNetFromAssignment( *assignment );
        via->SetNet( net );
    }

    // TODO: Get via dimensions from pad stack or other source
    via->SetDrill( pcbIUScale.mmToIU( 0.3 ) );  // Default drill
    via->SetWidth( PADSTACK::ALL_LAYERS, pcbIUScale.mmToIU( 0.6 ) );  // Default diameter

    m_board->Add( via.release(), ADD_MODE::APPEND );

    wxLogTrace( ALLEGRO_DBG, wxT( "Created via at (%d, %d)" ), position.x, position.y );
}

void ALLEGRO_PARSER::CreateZone( BOARD_ITEM_CONTAINER& aContainer, const SHAPE_DATA& aShape,
                                 const wxString& aNetName )
{
    // Parse zone outline
    SHAPE_LINE_CHAIN outline = ParseContour( ByteSwap32( aShape.firstSegmentPtr ) );

    if( outline.PointCount() < 3 )
    {
        wxLogTrace( ALLEGRO_DBG, wxT( "Zone outline has insufficient points" ) );
        return;
    }

    PCB_LAYER_ID layer = ConvertLayer( aShape.layer );

    // Handle different shape types
    if( aShape.layer.family == LAYER_FAMILY::COPPER )
    {
        std::unique_ptr<ZONE> zone = std::make_unique<ZONE>( &aContainer );

        zone->SetZoneName( wxString::Format( wxT( "Zone_0x%08X" ), ByteSwap32( aShape.key ) ) );
        zone->SetLocalClearance( 0 );
        zone->SetMinThickness( DEFAULT_ZONE_WIDTH );

        outline.SetClosed( true );
        zone->Outline()->Append( outline );

        zone->SetPadConnection( ZONE_CONNECTION::INHERITED );
        zone->SetLayer( layer );

        if( !aNetName.IsEmpty() )
        {
            NETINFO_ITEM* net = GetOrCreateNet( aNetName );
            zone->SetNet( net );
        }

        // Parse cutouts
        uint32_t cutoutPtr = ByteSwap32( aShape.cutoutsPtr );

        while( cutoutPtr != 0 && IsObjectType( cutoutPtr, OBJECT_TYPE::RULE_REGION ) )
        {
            const RULE_REGION_DATA* region =
                static_cast<const RULE_REGION_DATA*>( GetObjectPointer( cutoutPtr ) );

            SHAPE_LINE_CHAIN cutout = ParseContour( ByteSwap32( region->firstSegmentPtr ) );

            if( cutout.PointCount() >= 3 )
            {
                cutout.SetClosed( true );
                zone->Outline()->AddHole( cutout );
            }

            cutoutPtr = ByteSwap32( region->next );
        }

        aContainer.Add( zone.release(), ADD_MODE::APPEND );
    }
    else if( aShape.layer.family == LAYER_FAMILY::BOARD_GEOMETRY &&
            aShape.layer.ordinal == 0xFD )
    {
        // Board outline
        std::unique_ptr<PCB_SHAPE> shape = std::make_unique<PCB_SHAPE>( &aContainer, SHAPE_T::POLY );

        SHAPE_POLY_SET polySet;
        outline.SetClosed( true );
        polySet.AddOutline( outline );

        shape->SetPolyShape( polySet );
        shape->SetFilled( false );
        shape->SetLayer( Edge_Cuts );

        aContainer.Add( shape.release(), ADD_MODE::APPEND );
    }
    else if( aShape.layer.family == LAYER_FAMILY::SILK_SCREEN )
    {
        // Silk screen shape
        std::unique_ptr<PCB_SHAPE> shape = std::make_unique<PCB_SHAPE>( &aContainer, SHAPE_T::POLY );

        SHAPE_POLY_SET polySet;
        outline.SetClosed( true );
        polySet.AddOutline( outline );

        shape->SetPolyShape( polySet );
        shape->SetFilled( true );
        shape->SetLayer( layer );

        aContainer.Add( shape.release(), ADD_MODE::APPEND );
    }
}

void ALLEGRO_PARSER::CreateAnnotation( BOARD_ITEM_CONTAINER& aContainer, uint32_t aAnnotationPtr )
{
    // Annotation objects contain graphics (lines, arcs, etc.)
    void* obj = GetObjectPointer( aAnnotationPtr );
    if( !obj )
        return;

    // Get layer and process graphics chain
    const OBJECT_BASE* base = static_cast<const OBJECT_BASE*>( obj );
    uint32_t graphicsPtr = ByteSwap32( base->next );

    // TODO: Determine layer from annotation object
    PCB_LAYER_ID layer = User_1;

    while( graphicsPtr != 0 )
    {
        void* graphicsObj = GetObjectPointer( graphicsPtr );
        if( !graphicsObj )
            break;

        if( IsObjectType( graphicsPtr, OBJECT_TYPE::ARC ) )
        {
            const ARC_DATA* arc = static_cast<const ARC_DATA*>( graphicsObj );

            auto [start, end] = ParseArcCoordinates( *arc );

            VECTOR2D center( ScaleSize( ConvertCadenceFloat( arc->centerX ) ),
                           ScaleSize( -ConvertCadenceFloat( arc->centerY ) ) );

            VECTOR2I mid = CalculateArcMidpoint( start, end, center );

            std::unique_ptr<PCB_SHAPE> shape =
                std::make_unique<PCB_SHAPE>( &aContainer, SHAPE_T::ARC );

            shape->SetLayer( layer );
            shape->SetWidth( ScaleSize( static_cast<int>( ByteSwap32( arc->width ) ) ) );
            shape->SetArcGeometry( start, mid, end );

            aContainer.Add( shape.release(), ADD_MODE::APPEND );

            graphicsPtr = ByteSwap32( arc->next );
        }
        else if( IsObjectType( graphicsPtr, OBJECT_TYPE::SEGMENT_15 ) ||
                IsObjectType( graphicsPtr, OBJECT_TYPE::SEGMENT_16 ) ||
                IsObjectType( graphicsPtr, OBJECT_TYPE::SEGMENT_17 ) )
        {
            const SEGMENT_DATA* segment = static_cast<const SEGMENT_DATA*>( graphicsObj );

            auto [start, end] = ParseSegmentCoordinates( *segment );

            std::unique_ptr<PCB_SHAPE> shape =
                std::make_unique<PCB_SHAPE>( &aContainer, SHAPE_T::SEGMENT );

            shape->SetLayer( layer );
            shape->SetWidth( ScaleSize( static_cast<int>( ByteSwap32( segment->width ) ) ) );
            shape->SetStart( start );
            shape->SetEnd( end );

            aContainer.Add( shape.release(), ADD_MODE::APPEND );

            graphicsPtr = ByteSwap32( segment->next );
        }
        else
        {
            break;
        }
    }
}

void ALLEGRO_PARSER::CreateRectangle( BOARD_ITEM_CONTAINER& aContainer, const RECTANGLE_DATA& aRectangle )
{
    PCB_LAYER_ID layer = ConvertLayer( aRectangle.layer );

    VECTOR2I topLeft = ScalePosition( aRectangle.coords[0], aRectangle.coords[1] );
    VECTOR2I bottomRight = ScalePosition( aRectangle.coords[2], aRectangle.coords[3] );

    std::unique_ptr<PCB_SHAPE> shape =
        std::make_unique<PCB_SHAPE>( &aContainer, SHAPE_T::RECTANGLE );

    shape->SetStart( topLeft );
    shape->SetEnd( bottomRight );
    shape->SetFilled( false );
    shape->SetLayer( layer );

    aContainer.Add( shape.release(), ADD_MODE::APPEND );
}

void ALLEGRO_PARSER::SetupPadShape( PAD& aPad, const PAD_STACK_COMPONENT& aComponent )
{
    // Determine pad shape from component type
    switch( aComponent.shapeType )
    {
    case 0x02:  // Circle
        aPad.SetShape( PADSTACK::ALL_LAYERS, PAD_SHAPE::CIRCLE );
        break;

    case 0x05:  // Rectangle
    case 0x06:
        aPad.SetShape( PADSTACK::ALL_LAYERS, PAD_SHAPE::RECTANGLE );
        break;

    case 0x0B:  // Rounded rectangle
    case 0x1B:
    case 0x0C:
        aPad.SetShape( PADSTACK::ALL_LAYERS, PAD_SHAPE::ROUNDRECT );
        aPad.SetRoundRectRadiusRatio( PADSTACK::ALL_LAYERS, 0.25 );
        break;

    case 0x16:  // Custom shape
        aPad.SetShape( PADSTACK::ALL_LAYERS, PAD_SHAPE::CUSTOM );

        // Parse custom shape from shape pointer
        if( aComponent.shapePtr != 0 && IsObjectType( ByteSwap32( aComponent.shapePtr ),
                                                      OBJECT_TYPE::SHAPE ) )
        {
            const SHAPE_DATA* shape =
                static_cast<const SHAPE_DATA*>( GetObjectPointer( ByteSwap32( aComponent.shapePtr ) ) );

            SHAPE_LINE_CHAIN outline = ParseContour( ByteSwap32( shape->firstSegmentPtr ) );

            aPad.DeletePrimitivesList();
            aPad.AddPrimitivePoly( PADSTACK::ALL_LAYERS, outline, 0, true );

            // Set a minimal anchor pad size
            aPad.SetSize( PADSTACK::ALL_LAYERS, VECTOR2I( pcbIUScale.MilsToIU( 1 ),
                                                          pcbIUScale.MilsToIU( 1 ) ) );
        }
        break;

    default:
        wxLogTrace( ALLEGRO_DBG, wxT( "Unknown pad shape type 0x%02X" ), aComponent.shapeType );
        break;
    }

    // Set pad size
    if( aComponent.shapeType != 0x16 )  // Not custom
    {
        VECTOR2I size( ScaleSize( aComponent.width ), ScaleSize( aComponent.height ) );
        aPad.SetSize( PADSTACK::ALL_LAYERS, size );
    }

    // Apply offset
    VECTOR2I offset( ScaleSize( aComponent.offsetX ), ScaleSize( -aComponent.offsetY ) );
    aPad.Move( offset );
}

PAD_STACK_COMPONENT* ALLEGRO_PARSER::GetPadComponent( const PAD_STACK_DATA& aPadStack, size_t aIndex )
{
    const char* basePtr = reinterpret_cast<const char*>( &aPadStack );

    size_t headerSize = sizeof( PAD_STACK_DATA );

    // Adjust for version differences
    if( m_hasVersion172Extensions )
    {
        headerSize -= sizeof( uint32_t ) * 4;  // Remove unknown4
        headerSize += sizeof( uint32_t ) * 3;  // Add unknown5
        headerSize -= sizeof( uint16_t );      // Remove unknown6
        headerSize += sizeof( uint16_t );      // Add unknown7
        headerSize += sizeof( uint32_t ) * 28; // Add unknown9
    }

    if( m_hasVersion165Extensions )
    {
        headerSize += sizeof( uint32_t ) * 8;  // Add unknown10
    }

    size_t componentSize = sizeof( PAD_STACK_COMPONENT );

    if( m_hasVersion172Extensions )
    {
        componentSize += sizeof( uint32_t ) * 3;  // Add unknown4, unknown5, unknown6
        componentSize -= sizeof( uint32_t );      // Remove unknown7
    }

    return reinterpret_cast<PAD_STACK_COMPONENT*>(
        const_cast<char*>( basePtr + headerSize + aIndex * componentSize ) );
}

SHAPE_LINE_CHAIN ALLEGRO_PARSER::ParseContour( uint32_t aFirstSegmentPtr )
{
    SHAPE_LINE_CHAIN chain;

    uint32_t segmentPtr = aFirstSegmentPtr;
    bool first = true;

    while( segmentPtr != 0 )
    {
        void* obj = GetObjectPointer( segmentPtr );
        if( !obj )
            break;

        if( IsObjectType( segmentPtr, OBJECT_TYPE::ARC ) )
        {
            const ARC_DATA* arc = static_cast<const ARC_DATA*>( obj );

            auto [start, end] = ParseArcCoordinates( *arc );

            VECTOR2D center( ScaleSize( ConvertCadenceFloat( arc->centerX ) ),
                           ScaleSize( -ConvertCadenceFloat( arc->centerY ) ) );

            SHAPE_ARC shapeArc;
            shapeArc.ConstructFromStartEndCenter( start, end, center,
                                                 arc->subtype == 0x00, 0 );

            if( first )
                chain.Append( start );

            chain.Append( shapeArc );

            segmentPtr = ByteSwap32( arc->next );
        }
        else if( IsObjectType( segmentPtr, OBJECT_TYPE::SEGMENT_15 ) ||
                IsObjectType( segmentPtr, OBJECT_TYPE::SEGMENT_16 ) ||
                IsObjectType( segmentPtr, OBJECT_TYPE::SEGMENT_17 ) )
        {
            const SEGMENT_DATA* segment = static_cast<const SEGMENT_DATA*>( obj );

            auto [start, end] = ParseSegmentCoordinates( *segment );

            if( first )
                chain.Append( start );

            chain.Append( end );

            segmentPtr = ByteSwap32( segment->next );
        }
        else
        {
            break;
        }

        first = false;
    }

    chain.SetClosed( true );
    return chain;
}

std::pair<VECTOR2I, VECTOR2I> ALLEGRO_PARSER::ParseSegmentCoordinates( const SEGMENT_DATA& aSegment )
{
    VECTOR2I start = ScalePosition( aSegment.coords[0], aSegment.coords[1] );
    VECTOR2I end = ScalePosition( aSegment.coords[2], aSegment.coords[3] );

    return { start, end };
}

std::pair<VECTOR2I, VECTOR2I> ALLEGRO_PARSER::ParseArcCoordinates( const ARC_DATA& aArc )
{
    VECTOR2I start = ScalePosition( aArc.coords[0], aArc.coords[1] );
    VECTOR2I end = ScalePosition( aArc.coords[2], aArc.coords[3] );

    return { start, end };
}

VECTOR2I ALLEGRO_PARSER::CalculateArcMidpoint( const VECTOR2I& aStart, const VECTOR2I& aEnd,
                                              const VECTOR2I& aCenter )
{
    // Calculate midpoint of arc for KiCad's arc representation
    double startAngle = atan2( aStart.y - aCenter.y, aStart.x - aCenter.x );
    double endAngle = atan2( aEnd.y - aCenter.y, aEnd.x - aCenter.x );

    double midAngle = ( startAngle + endAngle ) / 2.0;

    // Handle arc direction
    double angleDiff = endAngle - startAngle;

    if( angleDiff > M_PI )
        angleDiff -= 2 * M_PI;
    else if( angleDiff < -M_PI )
        angleDiff += 2 * M_PI;

    if( angleDiff < 0 )
        midAngle += M_PI;

    double radius = ( aStart - aCenter ).EuclideanNorm();

    VECTOR2I mid;
    mid.x = aCenter.x + radius * cos( midAngle );
    mid.y = aCenter.y + radius * sin( midAngle );

    return mid;
}

void* ALLEGRO_PARSER::GetObjectPointer( uint32_t aKey )
{
    auto it = m_objectPointers.find( aKey );

    if( it != m_objectPointers.end() )
        return it->second;

    return nullptr;
}

bool ALLEGRO_PARSER::IsObjectType( uint32_t aKey, OBJECT_TYPE aType )
{
    void* obj = GetObjectPointer( aKey );

    if( !obj )
        return false;

    const uint8_t* typePtr = static_cast<const uint8_t*>( obj );

    return *typePtr == static_cast<uint8_t>( aType );
}

wxString ALLEGRO_PARSER::GetString( uint32_t aKey )
{
    auto it = m_stringTable.find( aKey );

    if( it != m_stringTable.end() )
        return wxString( it->second );

    return wxEmptyString;
}

std::optional<wxString> ALLEGRO_PARSER::GetOptionalString( uint32_t aKey )
{
    auto it = m_stringTable.find( aKey );

    if( it != m_stringTable.end() )
        return wxString( it->second );

    return std::nullopt;
}

NETINFO_ITEM* ALLEGRO_PARSER::GetOrCreateNet( const wxString& aNetName )
{
    NETINFO_ITEM* net = m_board->FindNet( aNetName );

    if( !net )
    {
        net = new NETINFO_ITEM( m_board, aNetName, m_board->GetNetCount() + 1 );
        m_board->Add( net, ADD_MODE::APPEND );
    }

    return net;
}

NETINFO_ITEM* ALLEGRO_PARSER::GetNetFromAssignment( const NET_ASSIGNMENT_DATA& aAssignment )
{
    uint32_t netPtr = ByteSwap32( aAssignment.netPtr );

    if( !IsObjectType( netPtr, OBJECT_TYPE::NET ) )
        return nullptr;

    const NET_DATA* net = static_cast<const NET_DATA*>( GetObjectPointer( netPtr ) );

    wxString netName = GetString( ByteSwap32( net->netNameStringPtr ) );

    return GetOrCreateNet( netName );
}

PCB_LAYER_ID ALLEGRO_PARSER::ConvertLayer( const LAYER_INFO& aLayer )
{
    // Check predefined layer mappings
    auto it = LAYER_MAP.find( aLayer );

    if( it != LAYER_MAP.end() )
        return it->second;

    // Handle copper layers
    if( aLayer.family == LAYER_FAMILY::COPPER )
    {
        if( aLayer.ordinal == 0 )
            return F_Cu;
        else if( aLayer.ordinal == m_copperLayerCount - 1 )
            return B_Cu;
        else if( aLayer.ordinal < MAX_LAYER_COUNT )
            return static_cast<PCB_LAYER_ID>( B_Cu + ( aLayer.ordinal * 2 ) );
    }

    // Default to user layer
    return User_1;
}

wxString ALLEGRO_PARSER::GetLayerName( const LAYER_INFO& aLayer )
{
    // Get layer name from layer set data
    LAYER_MAP_ENTRY entry = m_header->layerSets[static_cast<uint8_t>( aLayer.family )];
    uint32_t layerSetPtr = ByteSwap32( entry.layerSetPtr );

    if( m_layerSets.count( layerSetPtr ) == 0 )
        return wxEmptyString;

    const std::vector<uint8_t>& layerData = m_layerSets[layerSetPtr];

    if( static_cast<uint32_t>( m_fileVersion ) <= 0x00130C00 )  // v16.4 and earlier
    {
        size_t entrySize = sizeof( LOCAL_LAYER_PROPERTIES );

        if( aLayer.ordinal * entrySize >= layerData.size() )
            return wxEmptyString;

        const LOCAL_LAYER_PROPERTIES* props =
            reinterpret_cast<const LOCAL_LAYER_PROPERTIES*>(
                layerData.data() + aLayer.ordinal * entrySize );

        return wxString( props->layerName );
    }
    else
    {
        size_t entrySize = sizeof( REFERENCE_LAYER_PROPERTIES );

        if( aLayer.ordinal * entrySize >= layerData.size() )
            return wxEmptyString;

        const REFERENCE_LAYER_PROPERTIES* props =
            reinterpret_cast<const REFERENCE_LAYER_PROPERTIES*>(
                layerData.data() + aLayer.ordinal * entrySize );

        return GetString( ByteSwap32( props->layerNameStringPtr ) );
    }
}

const FONT_DIMENSION_DATA* ALLEGRO_PARSER::GetFontData( uint8_t aFontKey )
{
    // Font key seems to have an offset
    constexpr int FONT_KEY_OFFSET = -1;

    auto it = m_fontData.find( aFontKey + FONT_KEY_OFFSET );

    if( it != m_fontData.end() )
        return it->second;

    wxLogTrace( ALLEGRO_DBG, wxT( "Font key %d not found" ), aFontKey );
    return nullptr;
}

int ALLEGRO_PARSER::ScaleSize( int aAllegroValue )
{
    return KiROUND( aAllegroValue * m_scaleFactor );
}

double ALLEGRO_PARSER::ScaleSize( double aAllegroValue )
{
    return aAllegroValue * m_scaleFactor;
}

VECTOR2I ALLEGRO_PARSER::ScalePosition( int aX, int aY )
{
    return VECTOR2I( ScaleSize( aX ), ScaleSize( -aY ) );
}

VECTOR2I ALLEGRO_PARSER::ScalePosition( const int aCoords[2] )
{
    return ScalePosition( aCoords[0], aCoords[1] );
}

EDA_ANGLE ALLEGRO_PARSER::ScaleAngle( uint32_t aMillidegrees )
{
    double degrees = static_cast<double>( aMillidegrees ) / ALLEGRO_ANGLE_SCALE;
    return EDA_ANGLE( degrees, DEGREES_T );
}

double ALLEGRO_PARSER::ConvertCadenceFloat( const CADENCE_FLOAT& aFloat )
{
    return aFloat.ToDouble();
}

uint32_t ALLEGRO_PARSER::RoundToWord( uint32_t aLength )
{
    return ( aLength + WORD_ALIGNMENT - 1 ) / WORD_ALIGNMENT * WORD_ALIGNMENT;
}

uint32_t ALLEGRO_PARSER::ByteSwap32( uint32_t aValue )
{
    return ( ( aValue & 0x000000FF ) << 24 ) |
           ( ( aValue & 0x0000FF00 ) << 8 ) |
           ( ( aValue & 0x00FF0000 ) >> 8 ) |
           ( ( aValue & 0xFF000000 ) >> 24 );
}

void ALLEGRO_PARSER::AdvanceReadPointer( size_t aBytes )
{
    ValidateReadPointer( aBytes );
    m_currentAddress = static_cast<const char*>( m_currentAddress ) + aBytes;
}

void ALLEGRO_PARSER::ValidateReadPointer( size_t aBytes )
{
    const char* newAddr = static_cast<const char*>( m_currentAddress ) + aBytes;
    const char* endAddr = static_cast<const char*>( m_baseAddress ) + m_fileSize;

    if( newAddr > endAddr )
    {
        THROW_IO_ERROR( wxString::Format(
            _( "Attempted to read beyond end of file (offset 0x%08lX + 0x%08lX)" ),
            static_cast<const char*>( m_currentAddress ) - static_cast<const char*>( m_baseAddress ),
            aBytes ) );
    }
}

void ALLEGRO_PARSER::LogDebug( const wxString& aMessage )
{
    wxLogTrace( ALLEGRO_DBG, aMessage );
}

void ALLEGRO_PARSER::LogWarning( const wxString& aMessage )
{
    wxLogTrace( ALLEGRO_DBG, wxT( "WARNING: %s" ), aMessage );
    wxLogWarning( aMessage );
}

void ALLEGRO_PARSER::LogError( const wxString& aMessage )
{
    wxLogTrace( ALLEGRO_DBG, wxT( "ERROR: %s" ), aMessage );
    wxLogError( aMessage );
}

// Factory function for creating parser with version detection
std::unique_ptr<ALLEGRO_PARSER> CreateAllegroParser( BOARD* aBoard, const ALLEGRO_FILE& aFile )
{
    return std::make_unique<ALLEGRO_PARSER>( aBoard, aFile );
}

// Helper functions for external use

const std::map<uint32_t, std::function<void( BOARD*, const ALLEGRO_FILE& )>> PARSERS = {
    { 0x00130000, []( BOARD* aBoard, const ALLEGRO_FILE& aFile )
        { ALLEGRO_PARSER( aBoard, aFile ).Parse(); } },
    { 0x00130200, []( BOARD* aBoard, const ALLEGRO_FILE& aFile )
        { ALLEGRO_PARSER( aBoard, aFile ).Parse(); } },
    { 0x00130402, []( BOARD* aBoard, const ALLEGRO_FILE& aFile )
        { ALLEGRO_PARSER( aBoard, aFile ).Parse(); } },
    { 0x00130C03, []( BOARD* aBoard, const ALLEGRO_FILE& aFile )
        { ALLEGRO_PARSER( aBoard, aFile ).Parse(); } },
    { 0x00131003, []( BOARD* aBoard, const ALLEGRO_FILE& aFile )
        { ALLEGRO_PARSER( aBoard, aFile ).Parse(); } },
    { 0x00131503, []( BOARD* aBoard, const ALLEGRO_FILE& aFile )
        { ALLEGRO_PARSER( aBoard, aFile ).Parse(); } },
    { 0x00131504, []( BOARD* aBoard, const ALLEGRO_FILE& aFile )
        { ALLEGRO_PARSER( aBoard, aFile ).Parse(); } },
    { 0x00140400, []( BOARD* aBoard, const ALLEGRO_FILE& aFile )
        { ALLEGRO_PARSER( aBoard, aFile ).Parse(); } },
    { 0x00140500, []( BOARD* aBoard, const ALLEGRO_FILE& aFile )
        { ALLEGRO_PARSER( aBoard, aFile ).Parse(); } },
    { 0x00140501, []( BOARD* aBoard, const ALLEGRO_FILE& aFile )
        { ALLEGRO_PARSER( aBoard, aFile ).Parse(); } },
    { 0x00140502, []( BOARD* aBoard, const ALLEGRO_FILE& aFile )
        { ALLEGRO_PARSER( aBoard, aFile ).Parse(); } },
    { 0x00140600, []( BOARD* aBoard, const ALLEGRO_FILE& aFile )
        { ALLEGRO_PARSER( aBoard, aFile ).Parse(); } },
    { 0x00140700, []( BOARD* aBoard, const ALLEGRO_FILE& aFile )
        { ALLEGRO_PARSER( aBoard, aFile ).Parse(); } },
    { 0x00140900, []( BOARD* aBoard, const ALLEGRO_FILE& aFile )
        { ALLEGRO_PARSER( aBoard, aFile ).Parse(); } },
    { 0x00140901, []( BOARD* aBoard, const ALLEGRO_FILE& aFile )
        { ALLEGRO_PARSER( aBoard, aFile ).Parse(); } },
    { 0x00140902, []( BOARD* aBoard, const ALLEGRO_FILE& aFile )
        { ALLEGRO_PARSER( aBoard, aFile ).Parse(); } },
    { 0x00140E00, []( BOARD* aBoard, const ALLEGRO_FILE& aFile )
        { ALLEGRO_PARSER( aBoard, aFile ).Parse(); } },
    { 0x00141500, []( BOARD* aBoard, const ALLEGRO_FILE& aFile )
        { ALLEGRO_PARSER( aBoard, aFile ).Parse(); } },
    { 0x00141501, []( BOARD* aBoard, const ALLEGRO_FILE& aFile )
        { ALLEGRO_PARSER( aBoard, aFile ).Parse(); } },
    { 0x00141502, []( BOARD* aBoard, const ALLEGRO_FILE& aFile )
        { ALLEGRO_PARSER( aBoard, aFile ).Parse(); } },
};