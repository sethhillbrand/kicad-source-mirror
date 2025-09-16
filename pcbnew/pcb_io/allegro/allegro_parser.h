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

#ifndef ALLEGRO_PARSER_H_
#define ALLEGRO_PARSER_H_

#include <map>
#include <unordered_map>
#include <vector>
#include <memory>
#include <optional>
#include <string>

#include <wx/string.h>

#include "allegro_file.h"
#include "allegro_structs.h"

#include <geometry/shape_line_chain.h>
#include <math/vector2d.h>
#include <geometry/eda_angle.h>
#include <board.h>
#include <footprint.h>
#include <pad.h>
#include <netinfo.h>
#include <board_item_container.h>
namespace ALLEGRO
{
struct ARC_DATA;
struct SEGMENT_DATA;
struct SHAPE_DATA;
struct FOOTPRINT_DATA;
struct PLACED_FOOTPRINT_DATA;
struct PAD_DATA;
struct PLACED_PAD_DATA;
struct PAD_STACK_DATA;
struct PAD_STACK_COMPONENT;
struct NET_DATA;
struct NET_ASSIGNMENT_DATA;
struct TRACK_DATA;
struct VIA_DATA;
struct RECTANGLE_DATA;
struct STRING_GRAPHIC_WRAPPER_DATA;

#define ALLEGRO_DBG wxT( "KICAD_ALLEGRO" )

/**
 * Parser for Cadence Allegro binary board files (.brd)
 *
 * Supports Allegro versions 16.0 through 17.5
 */
class ALLEGRO_PARSER
{
public:
    /**
     * Constructor
     * @param aBoard Board to populate with parsed data
     * @param aAllegroBrdFile Memory-mapped Allegro board file
     */
    ALLEGRO_PARSER( BOARD* aBoard, const ALLEGRO_FILE& aAllegroBrdFile );

    /**
     * Parse the Allegro board file and populate the KiCad board
     * @throw IO_ERROR on parse failure
     */
    void Parse();

private:
    // File format detection and initialization
    bool DetectFileVersion();
    void InitializeParser();
    void ParseHeader();
    void ParseStringTable();

    // Main parsing stages
    void ParseObjects();
    void BuildBoard();

    // Object parsers
    void ParseArc( const void* aData );
    void ParseSegment( const void* aData );
    void ParseNetAssignment( const void* aData );
    void ParseTrack( const void* aData );
    void ParsePad( const void* aData );
    void ParseInstance( const void* aData );
    void ParseNet( const void* aData );
    void ParsePadStack( const void* aData );
    void ParseShape( const void* aData );
    void ParseFootprint( const void* aData );
    void ParsePlacedFootprint( const void* aData );
    void ParseStringGraphic( const void* aData );
    void ParsePlacedPad( const void* aData );
    void ParseVia( const void* aData );
    void ParseRectangle( const void* aData );
    void ParseRuleRegion( const void* aData );
    void ParseFontData( const void* aData );
    void ParseLayerInfo( const void* aData );
    void ParseModelInfo( const void* aData );
    void ParseMetadata( const void* aData );

    // Board construction helpers
    void ProcessNets();
    void ProcessFootprints();
    void ProcessShapes();
    void ProcessAnnotations();
    void ProcessZones();
    void UpdateLayerConfiguration();

    // Footprint creation
    void CreateFootprint( const ALLEGRO::PLACED_FOOTPRINT_DATA& aPlacedFootprint );
    void AddFootprintPad( FOOTPRINT* aFootprint, const ALLEGRO::PLACED_PAD_DATA& aPlacedPad );
    void AddFootprintGraphics( FOOTPRINT* aFootprint, uint32_t aGraphicsPtr );
    void AddFootprintText( FOOTPRINT* aFootprint, const ALLEGRO::STRING_GRAPHIC_WRAPPER_DATA& aTextWrapper );
    void AddFootprintZone( FOOTPRINT* aFootprint, const ALLEGRO::SHAPE_DATA& aShape );

    // Track and via creation
    void CreateTrack( const ALLEGRO::NET_DATA& aNet, const ALLEGRO::TRACK_DATA& aTrack );
    void CreateVia( const ALLEGRO::VIA_DATA& aVia );

    // Shape and zone creation
    void CreateZone( BOARD_ITEM_CONTAINER& aContainer, const ALLEGRO::SHAPE_DATA& aShape,
                     const wxString& aNetName = wxEmptyString );
    void CreateAnnotation( BOARD_ITEM_CONTAINER& aContainer, uint32_t aAnnotationPtr );
    void CreateRectangle( BOARD_ITEM_CONTAINER& aContainer, const ALLEGRO::RECTANGLE_DATA& aRectangle );

    // Pad helpers
    void SetupPadShape( PAD& aPad, const ALLEGRO::PAD_STACK_COMPONENT& aComponent );
    ALLEGRO::PAD_STACK_COMPONENT* GetPadComponent( const ALLEGRO::PAD_STACK_DATA& aPadStack,
                                                   size_t aIndex );

    // Geometry parsing
    SHAPE_LINE_CHAIN ParseContour( uint32_t aFirstSegmentPtr );
    std::pair<VECTOR2I, VECTOR2I> ParseSegmentCoordinates( const ALLEGRO::SEGMENT_DATA& aSegment );
    std::pair<VECTOR2I, VECTOR2I> ParseArcCoordinates( const ALLEGRO::ARC_DATA& aArc );
    VECTOR2I CalculateArcMidpoint( const VECTOR2I& aStart, const VECTOR2I& aEnd,
                                   const VECTOR2I& aCenter );

    // Data access helpers
    void* GetObjectPointer( uint32_t aKey );
    bool IsObjectType( uint32_t aKey, ALLEGRO::OBJECT_TYPE aType );
    wxString GetString( uint32_t aKey );
    std::optional<wxString> GetOptionalString( uint32_t aKey );
    NETINFO_ITEM* GetOrCreateNet( const wxString& aNetName );
    NETINFO_ITEM* GetNetFromAssignment( const ALLEGRO::NET_ASSIGNMENT_DATA& aAssignment );
    PCB_LAYER_ID ConvertLayer( const ALLEGRO::LAYER_INFO& aLayer );
    wxString GetLayerName( const ALLEGRO::LAYER_INFO& aLayer );
    const ALLEGRO::FONT_DIMENSION_DATA* GetFontData( uint8_t aFontKey );

    // Coordinate and unit conversion
    int ScaleSize( int aAllegroValue );
    double ScaleSize( double aAllegroValue );
    VECTOR2I ScalePosition( int aX, int aY );
    VECTOR2I ScalePosition( const int aCoords[2] );
    EDA_ANGLE ScaleAngle( uint32_t aMillidegrees );
    double ConvertCadenceFloat( const ALLEGRO::CADENCE_FLOAT& aFloat );

    // Utility functions
    uint32_t RoundToWord( uint32_t aLength );
    uint32_t ByteSwap32( uint32_t aValue );
    void AdvanceReadPointer( size_t aBytes );
    void ValidateReadPointer( size_t aBytes );

    // Logging
    void LogDebug( const wxString& aMessage );
    void LogWarning( const wxString& aMessage );
    void LogError( const wxString& aMessage );

private:
    BOARD*              m_board;
    const ALLEGRO_FILE& m_allegroFile;

    // File parsing state
    const void*         m_baseAddress;
    const void*         m_currentAddress;
    size_t              m_fileSize;
    ALLEGRO::FILE_VERSION m_fileVersion;

    // Parsed data structures
    const ALLEGRO::FILE_HEADER* m_header;
    std::unordered_map<uint32_t, void*> m_objectPointers;
    std::unordered_map<uint32_t, std::string> m_stringTable;
    std::map<uint32_t, std::vector<uint8_t>> m_layerSets;
    std::map<uint32_t, const ALLEGRO::FONT_DIMENSION_DATA*> m_fontData;
    std::map<wxString, std::unique_ptr<FOOTPRINT>> m_footprintLibrary;

    // Conversion factors
    double m_scaleFactor;
    int    m_copperLayerCount;

    // Parser configuration based on file version
    bool m_hasVersion172Extensions;
    bool m_hasVersion174Extensions;
    bool m_hasVersion165Extensions;
};

} // namespace ALLEGRO

#endif // ALLEGRO_PARSER_H_