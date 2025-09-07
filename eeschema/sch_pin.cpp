/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2016 Jean-Pierre Charras, jp.charras at wanadoo.fr
 * Copyright (C) 2015 Wayne Stambaugh <stambaughw@gmail.com>
 * Copyright (C) 2018 CERN
 * Copyright The KiCad Developers, see AUTHORS.txt for contributors.
 * @author Jon Evans <jon@craftyjon.com>
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

#include "sch_pin.h"

#include <base_units.h>
#include <pgm_base.h>
#include <pin_layout_cache.h>
#include <plotters/plotter.h>
#include <sch_draw_panel.h>
#include <sch_edit_frame.h>
#include <symbol_edit_frame.h>
#include <settings/settings_manager.h>
#include <symbol_editor/symbol_editor_settings.h>
#include <trigo.h>
#include <string_utils.h>


// small margin in internal units between the pin text and the pin line
#define PIN_TEXT_MARGIN 4

/// Utility for getting the size of the 'internal' pin decorators (as a radius)
/// i.e. the clock symbols (falling clock is actually external but is of
/// the same kind)
static int internalPinDecoSize( const RENDER_SETTINGS* aSettings, const SCH_PIN &aPin )
{
    const SCH_RENDER_SETTINGS* settings = static_cast<const SCH_RENDER_SETTINGS*>( aSettings );

    if( settings && settings->m_PinSymbolSize )
        return settings->m_PinSymbolSize;

    return aPin.GetNameTextSize() != 0 ? aPin.GetNameTextSize() / 2 : aPin.GetNumberTextSize() / 2;
}


/// Utility for getting the size of the 'external' pin decorators (as a radius)
/// i.e. the negation circle, the polarity 'slopes' and the nonlogic
/// marker
static int externalPinDecoSize( const RENDER_SETTINGS* aSettings, const SCH_PIN &aPin )
{
    const SCH_RENDER_SETTINGS* settings = static_cast<const SCH_RENDER_SETTINGS*>( aSettings );

    if( settings && settings->m_PinSymbolSize )
        return settings->m_PinSymbolSize;

    return aPin.GetNumberTextSize() / 2;
}


SCH_PIN::SCH_PIN( LIB_SYMBOL* aParentSymbol ) :
        SCH_ITEM( aParentSymbol, SCH_PIN_T, 0, 0 ),
        m_libPin( nullptr ),
        m_position( { 0, 0 } ),
        m_length( schIUScale.MilsToIU( DEFAULT_PIN_LENGTH ) ),
        m_orientation( PIN_ORIENTATION::PIN_RIGHT ),
        m_shape( GRAPHIC_PINSHAPE::LINE ),
        m_type( ELECTRICAL_PINTYPE::PT_UNSPECIFIED ),
        m_hidden( false ),
        m_numTextSize( schIUScale.MilsToIU( DEFAULT_PINNUM_SIZE ) ),
        m_nameTextSize( schIUScale.MilsToIU( DEFAULT_PINNAME_SIZE ) ),
        m_isDangling( true ),
        m_layoutCache( std::make_unique<PIN_LAYOUT_CACHE>( *this ) )
{
    if( SYMBOL_EDITOR_SETTINGS* cfg = GetAppSettings<SYMBOL_EDITOR_SETTINGS>( "symbol_editor" ) )
    {
        m_length       = schIUScale.MilsToIU( cfg->m_Defaults.pin_length );
        m_numTextSize  = schIUScale.MilsToIU( cfg->m_Defaults.pin_num_size );
        m_nameTextSize = schIUScale.MilsToIU( cfg->m_Defaults.pin_name_size );
    }

    m_layer = LAYER_DEVICE;
}


SCH_PIN::SCH_PIN( LIB_SYMBOL* aParentSymbol, const wxString& aName, const wxString& aNumber,
                  PIN_ORIENTATION aOrientation, ELECTRICAL_PINTYPE aPinType, int aLength,
                  int aNameTextSize, int aNumTextSize, int aBodyStyle, const VECTOR2I& aPos,
                  int aUnit ) :
        SCH_ITEM( aParentSymbol, SCH_PIN_T, aUnit, aBodyStyle ),
        m_libPin( nullptr ),
        m_position( aPos ),
        m_length( aLength ),
        m_orientation( aOrientation ),
        m_shape( GRAPHIC_PINSHAPE::LINE ),
        m_type( aPinType ),
        m_hidden( false ),
        m_numTextSize( aNumTextSize ),
        m_nameTextSize( aNameTextSize ),
        m_isDangling( true ),
        m_layoutCache( std::make_unique<PIN_LAYOUT_CACHE>( *this ) )
{
    SetName( aName );
    SetNumber( aNumber );

    m_layer = LAYER_DEVICE;
}


SCH_PIN::SCH_PIN( SCH_SYMBOL* aParentSymbol, SCH_PIN* aLibPin ) :
        SCH_ITEM( aParentSymbol, SCH_PIN_T, 0, 0 ),
        m_libPin( aLibPin ),
        m_orientation( PIN_ORIENTATION::INHERIT ),
        m_shape( GRAPHIC_PINSHAPE::INHERIT ),
        m_type( ELECTRICAL_PINTYPE::PT_INHERIT ),
        m_isDangling( true ),
        m_layoutCache( std::make_unique<PIN_LAYOUT_CACHE>( *this ) )
{
    wxASSERT( aParentSymbol );

    SetName( m_libPin->GetName() );
    SetNumber( m_libPin->GetNumber() );
    m_position = m_libPin->GetPosition();

    m_layer = LAYER_PIN;
}


SCH_PIN::SCH_PIN( SCH_SYMBOL* aParentSymbol, const wxString& aNumber, const wxString& aAlt,
                  const KIID& aUuid ) :
        SCH_ITEM( aParentSymbol, SCH_PIN_T ),
        m_libPin( nullptr ),
        m_orientation( PIN_ORIENTATION::INHERIT ),
        m_shape( GRAPHIC_PINSHAPE::INHERIT ),
        m_type( ELECTRICAL_PINTYPE::PT_INHERIT ),
        m_number( aNumber ),
        m_alt( aAlt ),
        m_isDangling( true ),
        m_layoutCache( std::make_unique<PIN_LAYOUT_CACHE>( *this ) )
{
    wxASSERT( aParentSymbol );

    const_cast<KIID&>( m_Uuid ) = aUuid;
    m_layer = LAYER_PIN;
}


SCH_PIN::SCH_PIN( const SCH_PIN& aPin ) :
        SCH_ITEM( aPin ),
        m_libPin( aPin.m_libPin ),
        m_alternates( aPin.m_alternates ),
        m_position( aPin.m_position ),
        m_length( aPin.m_length ),
        m_orientation( aPin.m_orientation ),
        m_shape( aPin.m_shape ),
        m_type( aPin.m_type ),
        m_hidden( aPin.m_hidden ),
        m_numTextSize( aPin.m_numTextSize ),
        m_nameTextSize( aPin.m_nameTextSize ),
        m_alt( aPin.m_alt ),
        m_isDangling( aPin.m_isDangling ),
        m_layoutCache( std::make_unique<PIN_LAYOUT_CACHE>( *this ) )
{
    SetName( aPin.m_name );
    SetNumber( aPin.m_number );

    m_layer = aPin.m_layer;
}


SCH_PIN::~SCH_PIN()
{
}


SCH_PIN& SCH_PIN::operator=( const SCH_PIN& aPin )
{
    SCH_ITEM::operator=( aPin );

    m_libPin = aPin.m_libPin;
    m_alternates = aPin.m_alternates;
    m_alt = aPin.m_alt;
    m_name = aPin.m_name;
    m_number = aPin.m_number;
    m_position = aPin.m_position;
    m_length = aPin.m_length;
    m_orientation = aPin.m_orientation;
    m_shape = aPin.m_shape;
    m_type = aPin.m_type;
    m_hidden = aPin.m_hidden;
    m_numTextSize = aPin.m_numTextSize;
    m_nameTextSize = aPin.m_nameTextSize;
    m_isDangling = aPin.m_isDangling;

    return *this;
}


VECTOR2I SCH_PIN::GetPosition() const
{
    if( const SCH_SYMBOL* symbol = dynamic_cast<const SCH_SYMBOL*>( GetParentSymbol() ) )
        return symbol->GetTransform().TransformCoordinate( m_position ) + symbol->GetPosition();
    else
        return m_position;
}

PIN_ORIENTATION SCH_PIN::GetOrientation() const
{
    if( m_orientation == PIN_ORIENTATION::INHERIT )
    {
        if( !m_libPin )
            return PIN_ORIENTATION::PIN_RIGHT;

        return m_libPin->GetOrientation();
    }

    return m_orientation;
}


GRAPHIC_PINSHAPE SCH_PIN::GetShape() const
{
    if( !m_alt.IsEmpty() )
    {
        if( !m_libPin )
            return GRAPHIC_PINSHAPE::LINE;

        return m_libPin->GetAlt( m_alt ).m_Shape;
    }
    else if( m_shape == GRAPHIC_PINSHAPE::INHERIT )
    {
        if( !m_libPin )
            return GRAPHIC_PINSHAPE::LINE;

        return m_libPin->GetShape();
    }

    return m_shape;
}


int SCH_PIN::GetLength() const
{
    if( !m_length.has_value() )
    {
        if( !m_libPin )
            return 0;

        return m_libPin->GetLength();
    }

    return m_length.value();
}


ELECTRICAL_PINTYPE SCH_PIN::GetType() const
{
    if( !m_alt.IsEmpty() )
    {
        if( !m_libPin )
            return ELECTRICAL_PINTYPE::PT_UNSPECIFIED;

        return m_libPin->GetAlt( m_alt ).m_Type;
    }
    else if( m_type == ELECTRICAL_PINTYPE::PT_INHERIT )
    {
        if( !m_libPin )
            return ELECTRICAL_PINTYPE::PT_UNSPECIFIED;

        return m_libPin->GetType();
    }

    return m_type;
}

void SCH_PIN::SetType( ELECTRICAL_PINTYPE aType )
{
    if( aType == m_type )
        return;

    m_type = aType;
    m_layoutCache->MarkDirty( PIN_LAYOUT_CACHE::DIRTY_FLAGS::ELEC_TYPE );
}


wxString SCH_PIN::GetCanonicalElectricalTypeName() const
{
    if( m_type != ELECTRICAL_PINTYPE::PT_INHERIT )
        return ::GetCanonicalElectricalTypeName( m_type );

    if( !m_libPin )
        return ::GetCanonicalElectricalTypeName( ELECTRICAL_PINTYPE::PT_UNSPECIFIED );

    return m_libPin->GetCanonicalElectricalTypeName();
}


wxString SCH_PIN::GetElectricalTypeName() const
{
    if( m_type != ELECTRICAL_PINTYPE::PT_INHERIT )
        return ElectricalPinTypeGetText( m_type );

    if( !m_libPin )
        return ElectricalPinTypeGetText( ELECTRICAL_PINTYPE::PT_UNSPECIFIED );

    return m_libPin->GetElectricalTypeName();
}


bool SCH_PIN::IsGlobalPower() const
{
    return GetType() == ELECTRICAL_PINTYPE::PT_POWER_IN
           && ( !IsVisible() || GetParentSymbol()->IsGlobalPower() );
}


bool SCH_PIN::IsLocalPower() const
{
    return GetType() == ELECTRICAL_PINTYPE::PT_POWER_IN
           && GetParentSymbol()->IsLocalPower();
}


bool SCH_PIN::IsPower() const
{
    return IsLocalPower() || IsGlobalPower();
}


bool SCH_PIN::IsVisible() const
{
    if( !m_hidden.has_value() )
    {
        if( !m_libPin )
            return true;

        return m_libPin->IsVisible();
    }

    return !m_hidden.value();
}


const wxString& SCH_PIN::GetName() const
{
    if( !m_alt.IsEmpty() )
        return m_alt;

    return GetBaseName();
}


const wxString& SCH_PIN::GetBaseName() const
{
    if( m_libPin )
        return m_libPin->GetBaseName();

    return m_name;
}


void SCH_PIN::SetName( const wxString& aName )
{
    if( m_name == aName )
        return;

    m_name = aName;

    // pin name string does not support spaces
    m_name.Replace( wxT( " " ), wxT( "_" ) );

    m_layoutCache->MarkDirty( PIN_LAYOUT_CACHE::DIRTY_FLAGS::NAME );
}


void SCH_PIN::SetAlt( const wxString& aAlt )
{
    // Do not set the alternate pin definition to the default pin name.  This breaks the library
    // symbol comparison for the ERC and the library diff tool.  It also incorrectly causes the
    // schematic symbol pin alternate to be set.
    if( aAlt.IsEmpty() || aAlt == GetBaseName() )
    {
        m_alt = wxEmptyString;
        return;
    }

    if( !m_libPin )
    {
        wxFAIL_MSG( wxString::Format( wxS( "Pin '%s' has no corresponding lib_pin" ), m_number ) );
        m_alt = wxEmptyString;
        return;
    }

    if( !m_libPin->GetAlternates().contains( aAlt ) )
    {
        wxFAIL_MSG( wxString::Format( wxS( "Pin '%s' has no alterate '%s'" ), m_number, aAlt ) );
        m_alt = wxEmptyString;
        return;
    }

    m_alt = aAlt;
}

bool SCH_PIN::IsDangling() const
{
    if( GetType() == ELECTRICAL_PINTYPE::PT_NC || GetType() == ELECTRICAL_PINTYPE::PT_NIC )
        return false;

    return m_isDangling;
}


void SCH_PIN::SetIsDangling( bool aIsDangling )
{
    m_isDangling = aIsDangling;
}


bool SCH_PIN::IsStacked( const SCH_PIN* aPin ) const
{
    bool isConnectableType_a = GetType() == ELECTRICAL_PINTYPE::PT_PASSIVE
                            || GetType() == ELECTRICAL_PINTYPE::PT_NIC;
    bool isConnectableType_b = aPin->GetType() == ELECTRICAL_PINTYPE::PT_PASSIVE
                            || aPin->GetType() == ELECTRICAL_PINTYPE::PT_NIC;

    bool sameParent = m_parent == aPin->GetParent();
    bool samePos = GetPosition() == aPin->GetPosition();
    bool sameName = GetName() == aPin->GetName();
    bool typeCompat = ( GetType() == aPin->GetType() || isConnectableType_a || isConnectableType_b );

    wxLogTrace( "KICAD_STACKED_PINS",
                wxString::Format( "IsStacked: this='%s/%s' other='%s/%s' sameParent=%d samePos=%d sameName=%d typeCompat=%d",
                                  GetName(), GetNumber(), aPin->GetName(), aPin->GetNumber(), sameParent,
                                  samePos, sameName, typeCompat ) );

    return sameParent
           && GetPosition() == aPin->GetPosition()
           && GetName() == aPin->GetName()
           && ( GetType() == aPin->GetType() || isConnectableType_a || isConnectableType_b );
}


bool SCH_PIN::Matches( const EDA_SEARCH_DATA& aSearchData, void* aAuxData ) const
{
    const SCH_SEARCH_DATA& schSearchData =
            dynamic_cast<const SCH_SEARCH_DATA&>( aSearchData );

    if( schSearchData.searchAllPins
        && ( EDA_ITEM::Matches( GetName(), aSearchData )
             || EDA_ITEM::Matches( GetNumber(), aSearchData ) ) )
    {
        return true;
    }

    SCH_CONNECTION* connection = nullptr;
    SCH_SHEET_PATH* sheetPath = reinterpret_cast<SCH_SHEET_PATH*>( aAuxData );

    if( schSearchData.searchNetNames && sheetPath && ( connection = Connection( sheetPath ) ) )
    {
        wxString netName = connection->GetNetName();

        if( EDA_ITEM::Matches( netName, aSearchData ) )
            return true;
    }

    return false;
}


bool SCH_PIN::Replace( const EDA_SEARCH_DATA& aSearchData, void* aAuxData )
{
    bool isReplaced = false;

    if( dynamic_cast<LIB_SYMBOL*>( GetParentSymbol() ) )
    {
        isReplaced |= EDA_ITEM::Replace( aSearchData, m_name );
        isReplaced |= EDA_ITEM::Replace( aSearchData, m_number );
    }
    else
    {
        /* TODO: waiting on a way to override pins in the schematic...
        isReplaced |= EDA_ITEM::Replace( aSearchData, m_name );
        isReplaced |= EDA_ITEM::Replace( aSearchData, m_number );
         */
    }

    return isReplaced;
}


bool SCH_PIN::HitTest( const VECTOR2I& aPosition, int aAccuracy ) const
{
    // When looking for an "exact" hit aAccuracy will be 0 which works poorly if the pin has
    // no pin number or name.  Give it a floor.
    if( Schematic() )
        aAccuracy = std::max( aAccuracy, Schematic()->Settings().m_PinSymbolSize / 4 );

    BOX2I rect = GetBoundingBox( false, true, m_flags & SHOW_ELEC_TYPE );

    return rect.Inflate( aAccuracy ).Contains( aPosition );
}


bool SCH_PIN::HitTest( const BOX2I& aRect, bool aContained, int aAccuracy ) const
{
    if( m_flags & (STRUCT_DELETED | SKIP_STRUCT ) )
        return false;

    BOX2I sel = aRect;

    if ( aAccuracy )
        sel.Inflate( aAccuracy );

    if( aContained )
        return sel.Contains( GetBoundingBox( false, false, false ) );

    return sel.Intersects( GetBoundingBox( false, true, m_flags & SHOW_ELEC_TYPE ) );
}


wxString SCH_PIN::GetShownName() const
{
    if( !m_alt.IsEmpty() )
        return m_alt;
    else if( m_libPin )
        return m_libPin->GetShownName();

    return m_name;
}


wxString SCH_PIN::GetShownNumber() const
{
    return m_number;
}


std::vector<wxString> SCH_PIN::GetStackedPinNumbers( bool* aValid ) const
{
    wxString shown = GetShownNumber();
    wxLogTrace( "KICAD_STACKED_PINS",
                wxString::Format( "GetStackedPinNumbers: shown='%s'", shown ) );

    std::vector<wxString> numbers = ExpandStackedPinNotation( shown, aValid );

    // Log the expansion for debugging
    wxLogTrace( "KICAD_STACKED_PINS",
                wxString::Format( "Expanded '%s' to %zu pins", shown, numbers.size() ) );
    for( const wxString& num : numbers )
    {
        wxLogTrace( "KICAD_STACKED_PINS", wxString::Format( " -> '%s'", num ) );
    }

    return numbers;
}


void SCH_PIN::SetNumber( const wxString& aNumber )
{
    if( m_number == aNumber )
        return;

    m_number = aNumber;
    // pin number string does not support spaces
    m_number.Replace( wxT( " " ), wxT( "_" ) );

    m_layoutCache->MarkDirty( PIN_LAYOUT_CACHE::DIRTY_FLAGS::NUMBER );
}


int SCH_PIN::GetNameTextSize() const
{
    if( !m_nameTextSize.has_value() )
    {
        if( !m_libPin )
            return schIUScale.MilsToIU( DEFAULT_PINNAME_SIZE );

        return m_libPin->GetNameTextSize();
    }

    return m_nameTextSize.value();
}


void SCH_PIN::SetNameTextSize( int aSize )
{
    if( aSize == m_nameTextSize )
        return;

    m_nameTextSize = aSize;
    m_layoutCache->MarkDirty( PIN_LAYOUT_CACHE::DIRTY_FLAGS::NAME );
}


int SCH_PIN::GetNumberTextSize() const
{
    if( !m_numTextSize.has_value() )
    {
        if( !m_libPin )
            return schIUScale.MilsToIU( DEFAULT_PINNUM_SIZE );

        return m_libPin->GetNumberTextSize();
    }

    return m_numTextSize.value();
}


void SCH_PIN::SetNumberTextSize( int aSize )
{
    if( aSize == m_numTextSize )
        return;

    m_numTextSize = aSize;
    m_layoutCache->MarkDirty( PIN_LAYOUT_CACHE::DIRTY_FLAGS::NUMBER );
}


VECTOR2I SCH_PIN::GetPinRoot() const
{
    if( const SCH_SYMBOL* symbol = dynamic_cast<const SCH_SYMBOL*>( GetParentSymbol() ) )
    {
        const TRANSFORM& t = symbol->GetTransform();

        if( !m_libPin )
            return GetPosition();

        return t.TransformCoordinate( m_libPin->GetPinRoot() ) + symbol->GetPosition();
    }

    switch( GetOrientation() )
    {
    default:
    case PIN_ORIENTATION::PIN_RIGHT: return m_position + VECTOR2I(  GetLength(), 0 );
    case PIN_ORIENTATION::PIN_LEFT:  return m_position + VECTOR2I( -GetLength(), 0 );
    case PIN_ORIENTATION::PIN_UP:    return m_position + VECTOR2I( 0, -GetLength() );
    case PIN_ORIENTATION::PIN_DOWN:  return m_position + VECTOR2I( 0,  GetLength() );
    }
}


void SCH_PIN::PlotPinType( PLOTTER *aPlotter, const VECTOR2I &aPosition,
                           PIN_ORIENTATION aOrientation, bool aDimmed ) const
{
    int                  MapX1, MapY1, x1, y1;
    SCH_RENDER_SETTINGS* renderSettings = getRenderSettings( aPlotter );
    COLOR4D              color = renderSettings->GetLayerColor( LAYER_PIN );
    COLOR4D              bg = renderSettings->GetBackgroundColor();
    int                  penWidth = GetEffectivePenWidth( renderSettings );
    int                  pinLength = GetLength();

    if( bg == COLOR4D::UNSPECIFIED || !aPlotter->GetColorMode() )
        bg = COLOR4D::WHITE;

    if( aDimmed )
    {
        color.Desaturate( );
        color = color.Mix( bg, 0.5f );
    }

    aPlotter->SetColor( color );
    aPlotter->SetCurrentLineWidth( penWidth );

    MapX1 = MapY1 = 0;
    x1 = aPosition.x; y1 = aPosition.y;

    switch( aOrientation )
    {
    case PIN_ORIENTATION::PIN_UP:     y1 = aPosition.y - pinLength;  MapY1 = 1;              break;
    case PIN_ORIENTATION::PIN_DOWN:   y1 = aPosition.y + pinLength;  MapY1 = -1;             break;
    case PIN_ORIENTATION::PIN_LEFT:   x1 = aPosition.x - pinLength;  MapX1 = 1;              break;
    case PIN_ORIENTATION::PIN_RIGHT:  x1 = aPosition.x + pinLength;  MapX1 = -1;             break;
    case PIN_ORIENTATION::INHERIT:    wxFAIL_MSG( wxS( "aOrientation must be resolved!" ) ); break;
    }

    if( m_shape == GRAPHIC_PINSHAPE::INVERTED || m_shape == GRAPHIC_PINSHAPE::INVERTED_CLOCK )
    {
        const int radius = externalPinDecoSize( aPlotter->RenderSettings(), *this );
        aPlotter->Circle( VECTOR2I( MapX1 * radius + x1, MapY1 * radius + y1 ), radius * 2,
                          FILL_T::NO_FILL, penWidth );

        aPlotter->MoveTo( VECTOR2I( MapX1 * radius * 2 + x1, MapY1 * radius * 2 + y1 ) );
        aPlotter->FinishTo( aPosition );
    }
    else if( m_shape == GRAPHIC_PINSHAPE::FALLING_EDGE_CLOCK )
    {
        const int deco_size = internalPinDecoSize( aPlotter->RenderSettings(), *this );
        if( MapY1 == 0 ) /* MapX1 = +- 1 */
        {
            aPlotter->MoveTo( VECTOR2I( x1, y1 + deco_size ) );
            aPlotter->LineTo( VECTOR2I( x1 + MapX1 * deco_size * 2, y1 ) );
            aPlotter->FinishTo( VECTOR2I( x1, y1 - deco_size ) );
        }
        else    /* MapX1 = 0 */
        {
            aPlotter->MoveTo( VECTOR2I( x1 + deco_size, y1 ) );
            aPlotter->LineTo( VECTOR2I( x1, y1 + MapY1 * deco_size * 2 ) );
            aPlotter->FinishTo( VECTOR2I( x1 - deco_size, y1 ) );
        }

        aPlotter->MoveTo( VECTOR2I( MapX1 * deco_size * 2 + x1, MapY1 * deco_size * 2 + y1 ) );
        aPlotter->FinishTo( aPosition );
    }
    else
    {
        aPlotter->MoveTo( VECTOR2I( x1, y1 ) );
        aPlotter->FinishTo( aPosition );
    }

    if( m_shape == GRAPHIC_PINSHAPE::CLOCK
            || m_shape == GRAPHIC_PINSHAPE::INVERTED_CLOCK
            || m_shape == GRAPHIC_PINSHAPE::CLOCK_LOW )
    {
        const int deco_size = internalPinDecoSize( aPlotter->RenderSettings(), *this );
        if( MapY1 == 0 ) /* MapX1 = +- 1 */
        {
            aPlotter->MoveTo( VECTOR2I( x1, y1 + deco_size ) );
            aPlotter->LineTo( VECTOR2I( x1 - MapX1 * deco_size * 2, y1 ) );
            aPlotter->FinishTo( VECTOR2I( x1, y1 - deco_size ) );
        }
        else    /* MapX1 = 0 */
        {
            aPlotter->MoveTo( VECTOR2I( x1 + deco_size, y1 ) );
            aPlotter->LineTo( VECTOR2I( x1, y1 - MapY1 * deco_size * 2 ) );
            aPlotter->FinishTo( VECTOR2I( x1 - deco_size, y1 ) );
        }
    }

    if( m_shape == GRAPHIC_PINSHAPE::INPUT_LOW
            || m_shape == GRAPHIC_PINSHAPE::CLOCK_LOW ) /* IEEE symbol "Active Low Input" */
    {
        const int deco_size = externalPinDecoSize( aPlotter->RenderSettings(), *this );

        if( MapY1 == 0 )        /* MapX1 = +- 1 */
        {
            aPlotter->MoveTo( VECTOR2I( x1 + MapX1 * deco_size * 2, y1 ) );
            aPlotter->LineTo( VECTOR2I( x1 + MapX1 * deco_size * 2, y1 - deco_size * 2 ) );
            aPlotter->FinishTo( VECTOR2I( x1, y1 ) );
        }
        else    /* MapX1 = 0 */
        {
            aPlotter->MoveTo( VECTOR2I( x1, y1 + MapY1 * deco_size * 2 ) );
            aPlotter->LineTo( VECTOR2I( x1 - deco_size * 2, y1 + MapY1 * deco_size * 2 ) );
            aPlotter->FinishTo( VECTOR2I( x1, y1 ) );
        }
    }

    if( m_shape == GRAPHIC_PINSHAPE::OUTPUT_LOW ) /* IEEE symbol "Active Low Output" */
    {
        const int symbol_size = externalPinDecoSize( aPlotter->RenderSettings(), *this );

        if( MapY1 == 0 )        /* MapX1 = +- 1 */
        {
            aPlotter->MoveTo( VECTOR2I( x1, y1 - symbol_size * 2 ) );
            aPlotter->FinishTo( VECTOR2I( x1 + MapX1 * symbol_size * 2, y1 ) );
        }
        else    /* MapX1 = 0 */
        {
            aPlotter->MoveTo( VECTOR2I( x1 - symbol_size * 2, y1 ) );
            aPlotter->FinishTo( VECTOR2I( x1, y1 + MapY1 * symbol_size * 2 ) );
        }
    }
    else if( m_shape == GRAPHIC_PINSHAPE::NONLOGIC ) /* NonLogic pin symbol */
    {
        const int deco_size = externalPinDecoSize( aPlotter->RenderSettings(), *this );
        aPlotter->MoveTo( VECTOR2I( x1 - ( MapX1 + MapY1 ) * deco_size,
                                    y1 - ( MapY1 - MapX1 ) * deco_size ) );
        aPlotter->FinishTo( VECTOR2I( x1 + ( MapX1 + MapY1 ) * deco_size,
                                      y1 + ( MapY1 - MapX1 ) * deco_size ) );
        aPlotter->MoveTo( VECTOR2I( x1 - ( MapX1 - MapY1 ) * deco_size,
                                    y1 - ( MapY1 + MapX1 ) * deco_size ) );
        aPlotter->FinishTo( VECTOR2I( x1 + ( MapX1 - MapY1 ) * deco_size,
                                      y1 + ( MapY1 + MapX1 ) * deco_size ) );
    }

    if( m_type == ELECTRICAL_PINTYPE::PT_NC ) // Draw a N.C. symbol
    {
        const int deco_size = TARGET_PIN_RADIUS;
        const int ex1 = aPosition.x;
        const int ey1 = aPosition.y;
        aPlotter->MoveTo( VECTOR2I( ex1 - deco_size, ey1 - deco_size ) );
        aPlotter->FinishTo( VECTOR2I( ex1 + deco_size, ey1 + deco_size ) );
        aPlotter->MoveTo( VECTOR2I( ex1 + deco_size, ey1 - deco_size ) );
        aPlotter->FinishTo( VECTOR2I( ex1 - deco_size, ey1 + deco_size ) );
    }
}


void SCH_PIN::PlotPinTexts( PLOTTER *aPlotter, const VECTOR2I &aPinPos, PIN_ORIENTATION aPinOrient,
                            int aTextInside, bool aDrawPinNum, bool aDrawPinName,
                            bool aDimmed ) const
{
    RENDER_SETTINGS* settings = aPlotter->RenderSettings();
    KIFONT::FONT*    font = KIFONT::FONT::GetFont( settings->GetDefaultFont(), false, false );
    wxString         name = GetShownName();
    wxString         number = GetShownNumber();

    if( name.IsEmpty() || m_nameTextSize == 0 )
        aDrawPinName = false;

    if( number.IsEmpty() || m_numTextSize == 0 )
        aDrawPinNum = false;

    if( !aDrawPinNum && !aDrawPinName )
        return;

    int namePenWidth = settings->GetDefaultPenWidth();
    int numPenWidth  = settings->GetDefaultPenWidth();
    int name_offset = schIUScale.MilsToIU( PIN_TEXT_MARGIN ) + namePenWidth;
    int num_offset  = schIUScale.MilsToIU( PIN_TEXT_MARGIN ) + numPenWidth;

    /* Get the num and name colors */
    COLOR4D nameColor = settings->GetLayerColor( LAYER_PINNAM );
    COLOR4D numColor  = settings->GetLayerColor( LAYER_PINNUM );
    COLOR4D bg = settings->GetBackgroundColor();

    if( bg == COLOR4D::UNSPECIFIED || !aPlotter->GetColorMode() )
        bg = COLOR4D::WHITE;

    if( aDimmed )
    {
        nameColor.Desaturate( );
        numColor.Desaturate( );
        nameColor = nameColor.Mix( bg, 0.5f );
        numColor = numColor.Mix( bg, 0.5f );
    }

    int x1 = aPinPos.x;
    int y1 = aPinPos.y;

    switch( aPinOrient )
    {
    case PIN_ORIENTATION::PIN_UP:    y1 -= GetLength();                                   break;
    case PIN_ORIENTATION::PIN_DOWN:  y1 += GetLength();                                   break;
    case PIN_ORIENTATION::PIN_LEFT:  x1 -= GetLength();                                   break;
    case PIN_ORIENTATION::PIN_RIGHT: x1 += GetLength();                                   break;
    case PIN_ORIENTATION::INHERIT:   wxFAIL_MSG( wxS( "aPinOrient must be resolved!" ) ); break;
    }

    auto plotName =
            [&]( int x, int y, const EDA_ANGLE& angle, GR_TEXT_H_ALIGN_T hJustify,
                 GR_TEXT_V_ALIGN_T vJustify )
            {
                TEXT_ATTRIBUTES attrs;
                attrs.m_StrokeWidth = namePenWidth;
                attrs.m_Angle = angle;
                attrs.m_Size = VECTOR2I( GetNameTextSize(), GetNameTextSize() );
                attrs.m_Halign = hJustify;
                attrs.m_Valign = vJustify;
                attrs.m_Multiline = false;

                aPlotter->PlotText( VECTOR2I( x, y ), nameColor, name, attrs, font,
                                    GetFontMetrics() );
            };

    auto plotNum =
            [&]( int x, int y, const EDA_ANGLE& angle, GR_TEXT_H_ALIGN_T hJustify,
                 GR_TEXT_V_ALIGN_T vJustify )
            {
                TEXT_ATTRIBUTES attrs;
                attrs.m_StrokeWidth = numPenWidth;
                attrs.m_Angle = angle;
                attrs.m_Size = VECTOR2I( GetNumberTextSize(), GetNumberTextSize() );
                attrs.m_Halign = hJustify;
                attrs.m_Valign = vJustify;
                attrs.m_Multiline = false;

                aPlotter->PlotText( VECTOR2I( x, y ), numColor, number, attrs, font,
                                    GetFontMetrics() );
            };

    // Draw the text inside, but the pin numbers outside.
    if( aTextInside )
    {
        if( ( aPinOrient == PIN_ORIENTATION::PIN_LEFT )
          || ( aPinOrient == PIN_ORIENTATION::PIN_RIGHT ) ) // It's a horizontal line.
        {
            if( aDrawPinName )
            {
                if( aPinOrient == PIN_ORIENTATION::PIN_RIGHT )
                {
                    plotName( x1 + aTextInside, y1, ANGLE_HORIZONTAL,
                              GR_TEXT_H_ALIGN_LEFT, GR_TEXT_V_ALIGN_CENTER );
                }
                else    // orient == PIN_LEFT
                {
                    plotName( x1 - aTextInside, y1, ANGLE_HORIZONTAL,
                              GR_TEXT_H_ALIGN_RIGHT, GR_TEXT_V_ALIGN_CENTER );
                }
            }

            if( aDrawPinNum )
            {
                plotNum( ( x1 + aPinPos.x) / 2, y1 - num_offset, ANGLE_HORIZONTAL,
                         GR_TEXT_H_ALIGN_CENTER, GR_TEXT_V_ALIGN_BOTTOM );
            }
        }
        else         // It's a vertical line.
        {
            if( aPinOrient == PIN_ORIENTATION::PIN_DOWN )
            {
                if( aDrawPinName )
                {
                    plotName( x1, y1 + aTextInside, ANGLE_VERTICAL,
                              GR_TEXT_H_ALIGN_RIGHT, GR_TEXT_V_ALIGN_CENTER );
                }

                if( aDrawPinNum )
                {
                    plotNum( x1 - num_offset, ( y1 + aPinPos.y) / 2, ANGLE_VERTICAL,
                             GR_TEXT_H_ALIGN_CENTER, GR_TEXT_V_ALIGN_BOTTOM );
                }
            }
            else        /* PIN_UP */
            {
                if( aDrawPinName )
                {
                    plotName( x1, y1 - aTextInside, ANGLE_VERTICAL,
                              GR_TEXT_H_ALIGN_LEFT, GR_TEXT_V_ALIGN_CENTER );
                }

                if( aDrawPinNum )
                {
                    plotNum( x1 - num_offset, ( y1 + aPinPos.y) / 2, ANGLE_VERTICAL,
                             GR_TEXT_H_ALIGN_CENTER, GR_TEXT_V_ALIGN_BOTTOM );
                }
            }
        }
    }
    else     // Draw num & text pin outside.
    {
        if( ( aPinOrient == PIN_ORIENTATION::PIN_LEFT )
            || ( aPinOrient == PIN_ORIENTATION::PIN_RIGHT ) )
        {
            // It's an horizontal line.
            if( aDrawPinName && aDrawPinNum )
            {
                plotName( ( x1 + aPinPos.x) / 2, y1 - name_offset, ANGLE_HORIZONTAL,
                          GR_TEXT_H_ALIGN_CENTER, GR_TEXT_V_ALIGN_BOTTOM );

                plotNum( ( x1 + aPinPos.x) / 2, y1 + num_offset, ANGLE_HORIZONTAL,
                          GR_TEXT_H_ALIGN_CENTER, GR_TEXT_V_ALIGN_TOP );
            }
            else if( aDrawPinName )
            {
                plotName( ( x1 + aPinPos.x) / 2, y1 - name_offset, ANGLE_HORIZONTAL,
                          GR_TEXT_H_ALIGN_CENTER, GR_TEXT_V_ALIGN_BOTTOM );
            }
            else if( aDrawPinNum )
            {
                plotNum( ( x1 + aPinPos.x) / 2, y1 - name_offset, ANGLE_HORIZONTAL,
                          GR_TEXT_H_ALIGN_CENTER, GR_TEXT_V_ALIGN_BOTTOM );
            }
        }
        else
        {
            // Its a vertical line.
            if( aDrawPinName && aDrawPinNum )
            {
                plotName( x1 - name_offset, ( y1 + aPinPos.y ) / 2, ANGLE_VERTICAL,
                          GR_TEXT_H_ALIGN_CENTER, GR_TEXT_V_ALIGN_BOTTOM );

                plotNum( x1 + num_offset, ( y1 + aPinPos.y ) / 2, ANGLE_VERTICAL,
                          GR_TEXT_H_ALIGN_CENTER, GR_TEXT_V_ALIGN_TOP );
            }
            else if( aDrawPinName )
            {
                plotName( x1 - name_offset, ( y1 + aPinPos.y ) / 2, ANGLE_VERTICAL,
                          GR_TEXT_H_ALIGN_CENTER, GR_TEXT_V_ALIGN_BOTTOM );
            }
            else if( aDrawPinNum )
            {
                plotNum( x1 - num_offset, ( y1 + aPinPos.y ) / 2, ANGLE_VERTICAL,
                          GR_TEXT_H_ALIGN_CENTER, GR_TEXT_V_ALIGN_BOTTOM );
            }
        }
    }
}


PIN_ORIENTATION SCH_PIN::PinDrawOrient( const TRANSFORM& aTransform ) const
{
    PIN_ORIENTATION orient;
    VECTOR2I end; // position of pin end starting at 0,0 according to its orientation, length = 1

    switch( GetOrientation() )
    {
    default:
    case PIN_ORIENTATION::PIN_RIGHT:  end.x = 1;   break;
    case PIN_ORIENTATION::PIN_UP:     end.y = -1;  break;
    case PIN_ORIENTATION::PIN_DOWN:   end.y = 1;   break;
    case PIN_ORIENTATION::PIN_LEFT:   end.x = -1;  break;
    }

    // = pos of end point, according to the symbol orientation.
    end    = aTransform.TransformCoordinate( end );
    orient = PIN_ORIENTATION::PIN_UP;

    if( end.x == 0 )
    {
        if( end.y > 0 )
            orient = PIN_ORIENTATION::PIN_DOWN;
    }
    else
    {
        orient = PIN_ORIENTATION::PIN_RIGHT;

        if( end.x < 0 )
            orient = PIN_ORIENTATION::PIN_LEFT;
    }

    return orient;
}


EDA_ITEM* SCH_PIN::Clone() const
{
    //return new SCH_PIN( *this );
    SCH_ITEM* newPin = new SCH_PIN( *this );
    wxASSERT( newPin->GetUnit() == m_unit && newPin->GetBodyStyle() == m_bodyStyle );
    return newPin;
}


void SCH_PIN::ChangeLength( int aLength )
{
    int lengthChange = GetLength() - aLength;
    int offsetX = 0;
    int offsetY = 0;

    switch( m_orientation )
    {
    default:
    case PIN_ORIENTATION::PIN_RIGHT:
        offsetX = lengthChange;
        break;
    case PIN_ORIENTATION::PIN_LEFT:
        offsetX = -1 * lengthChange;
        break;
    case PIN_ORIENTATION::PIN_UP:
        offsetY = -1 * lengthChange;
        break;
    case PIN_ORIENTATION::PIN_DOWN:
        offsetY = lengthChange;
        break;
    }

    m_position += VECTOR2I( offsetX, offsetY );
    m_length = aLength;
}


void SCH_PIN::Move( const VECTOR2I& aOffset )
{
    m_position += aOffset;
}


void SCH_PIN::MirrorHorizontallyPin( int aCenter )
{
    m_position.x -= aCenter;
    m_position.x *= -1;
    m_position.x += aCenter;

    if( m_orientation == PIN_ORIENTATION::PIN_RIGHT )
        m_orientation = PIN_ORIENTATION::PIN_LEFT;
    else if( m_orientation == PIN_ORIENTATION::PIN_LEFT )
        m_orientation = PIN_ORIENTATION::PIN_RIGHT;
}


void SCH_PIN::MirrorHorizontally( int aCenter )
{
    if( dynamic_cast<LIB_SYMBOL*>( GetParentSymbol() ) )
        MirrorHorizontallyPin( aCenter );
}


void SCH_PIN::MirrorVerticallyPin( int aCenter )
{
    m_position.y -= aCenter;
    m_position.y *= -1;
    m_position.y += aCenter;

    if( m_orientation == PIN_ORIENTATION::PIN_UP )
        m_orientation = PIN_ORIENTATION::PIN_DOWN;
    else if( m_orientation == PIN_ORIENTATION::PIN_DOWN )
        m_orientation = PIN_ORIENTATION::PIN_UP;
}


void SCH_PIN::MirrorVertically( int aCenter )
{
    if( dynamic_cast<LIB_SYMBOL*>( GetParentSymbol() ) )
        MirrorVerticallyPin( aCenter );
}


void SCH_PIN::RotatePin( const VECTOR2I& aCenter, bool aRotateCCW )
{
    if( aRotateCCW )
    {
        RotatePoint( m_position, aCenter, ANGLE_90 );

        switch( GetOrientation() )
        {
        default:
        case PIN_ORIENTATION::PIN_RIGHT: m_orientation = PIN_ORIENTATION::PIN_UP;    break;
        case PIN_ORIENTATION::PIN_UP:    m_orientation = PIN_ORIENTATION::PIN_LEFT;  break;
        case PIN_ORIENTATION::PIN_LEFT:  m_orientation = PIN_ORIENTATION::PIN_DOWN;  break;
        case PIN_ORIENTATION::PIN_DOWN:  m_orientation = PIN_ORIENTATION::PIN_RIGHT; break;
        }
    }
    else
    {
        RotatePoint( m_position, aCenter, -ANGLE_90 );

        switch( GetOrientation() )
        {
        default:
        case PIN_ORIENTATION::PIN_RIGHT: m_orientation = PIN_ORIENTATION::PIN_DOWN;  break;
        case PIN_ORIENTATION::PIN_UP:    m_orientation = PIN_ORIENTATION::PIN_RIGHT; break;
        case PIN_ORIENTATION::PIN_LEFT:  m_orientation = PIN_ORIENTATION::PIN_UP;    break;
        case PIN_ORIENTATION::PIN_DOWN:  m_orientation = PIN_ORIENTATION::PIN_LEFT;  break;
        }
    }
}


void SCH_PIN::Rotate( const VECTOR2I& aCenter, bool aRotateCCW )
{
    if( dynamic_cast<LIB_SYMBOL*>( GetParentSymbol() ) )
        RotatePin( aCenter, aRotateCCW );
}


void SCH_PIN::Plot( PLOTTER* aPlotter, bool aBackground, const SCH_PLOT_OPTS& aPlotOpts,
                    int aUnit, int aBodyStyle, const VECTOR2I& aOffset, bool aDimmed )
{
    if( aBackground )
        return;

    SCH_RENDER_SETTINGS* renderSettings = getRenderSettings( aPlotter );

    if( !IsVisible() && !renderSettings->m_ShowHiddenPins )
        return;

    const SYMBOL*   part = GetParentSymbol();
    PIN_ORIENTATION orient = PinDrawOrient( renderSettings->m_Transform );
    VECTOR2I        pos = renderSettings->TransformCoordinate( m_position ) + aOffset;

    PlotPinType( aPlotter, pos, orient, aDimmed );
    PlotPinTexts( aPlotter, pos, orient, part->GetPinNameOffset(), part->GetShowPinNumbers(),
                  part->GetShowPinNames(), aDimmed );
}


void SCH_PIN::GetMsgPanelInfo( EDA_DRAW_FRAME* aFrame, std::vector<MSG_PANEL_ITEM>& aList )
{
    wxString msg;
    SYMBOL*  symbol = GetParentSymbol();

    aList.emplace_back( _( "Type" ), _( "Pin" ) );

    SCH_ITEM::GetMsgPanelInfo( aFrame, aList );

    aList.emplace_back( _( "Name" ), UnescapeString( GetShownName() ) );
    aList.emplace_back( _( "Number" ), GetShownNumber() );
    aList.emplace_back( _( "Type" ), ElectricalPinTypeGetText( GetType() ) );
    aList.emplace_back( _( "Style" ), PinShapeGetText( GetShape() ) );

    aList.emplace_back( _( "Visible" ), IsVisible() ? _( "Yes" ) : _( "No" ) );

    // Display pin length
    aList.emplace_back( _( "Length" ), aFrame->MessageTextFromValue( GetLength(), true ) );

    aList.emplace_back( _( "Orientation" ), PinOrientationName( GetOrientation() ) );

    if( dynamic_cast<LIB_SYMBOL*>( symbol ) )
    {
        aList.emplace_back( _( "Pos X" ), aFrame->MessageTextFromValue( GetPosition().x, true ) );
        aList.emplace_back( _( "Pos Y" ), aFrame->MessageTextFromValue( GetPosition().y, true ) );
    }
    else if( SCH_SYMBOL* schsymbol = dynamic_cast<SCH_SYMBOL*>( symbol ) )
    {
        SCH_EDIT_FRAME* schframe = dynamic_cast<SCH_EDIT_FRAME*>( aFrame );
        SCH_SHEET_PATH* currentSheet = schframe ? &schframe->GetCurrentSheet() : nullptr;

        // Don't use GetShownText(); we want to see the variable references here
        aList.emplace_back( symbol->GetRef( currentSheet ),
                            UnescapeString( schsymbol->GetField( FIELD_T::VALUE )->GetText() ) );
    }

#if defined(DEBUG)
    if( !IsConnectivityDirty() && dynamic_cast<SCH_EDIT_FRAME*>( aFrame ) )
    {
        SCH_CONNECTION* conn = Connection();

        if( conn )
            conn->AppendInfoToMsgPanel( aList );
    }
#endif
}


void SCH_PIN::ClearDefaultNetName( const SCH_SHEET_PATH* aPath )
{
    std::lock_guard<std::recursive_mutex> lock( m_netmap_mutex );

    if( aPath )
        m_net_name_map.erase( *aPath );
    else
        m_net_name_map.clear();
}


wxString SCH_PIN::GetDefaultNetName( const SCH_SHEET_PATH& aPath, bool aForceNoConnect )
{
    const SCH_SYMBOL* symbol = static_cast<const SCH_SYMBOL*>( GetParentSymbol() );

    // Need to check for parent as power symbol to make sure we aren't dealing
    // with legacy global power pins on non-power symbols
    if( IsGlobalPower() || IsLocalPower() )
    {
        SYMBOL* parent = GetLibPin()->GetParentSymbol();

        if( parent->IsGlobalPower() || parent->IsLocalPower() )
        {
            return EscapeString( symbol->GetValue( true, &aPath, false ), CTX_NETNAME );
        }
        else
        {
            wxString tmp = m_libPin ? m_libPin->GetName() : wxString( "??" );

            return EscapeString( tmp, CTX_NETNAME );
        }
    }

    std::lock_guard<std::recursive_mutex> lock( m_netmap_mutex );

    auto it = m_net_name_map.find( aPath );

    if( it != m_net_name_map.end() )
    {
        if( it->second.second == aForceNoConnect )
            return it->second.first;
    }

    wxString name = "Net-(";
    bool unconnected = false;

    if( aForceNoConnect || GetType() == ELECTRICAL_PINTYPE::PT_NC )
    {
        unconnected = true;
        name = ( "unconnected-(" );
    }

    bool annotated = true;

    std::vector<SCH_PIN*> pins = symbol->GetPins( &aPath );
    bool has_multiple = false;

    for( SCH_PIN* pin : pins )
    {
        if( pin->GetShownName() == GetShownName()
                && pin->GetShownNumber() != GetShownNumber()
                && unconnected == ( pin->GetType() == ELECTRICAL_PINTYPE::PT_NC ) )
        {
            has_multiple = true;
            break;
        }
    }

    wxString libPinShownName = m_libPin ? m_libPin->GetShownName() : wxString( "??" );
    wxString libPinShownNumber = m_libPin ? m_libPin->GetShownNumber() : wxString( "??" );

    // If this is a stacked pin (valid bracketed/list notation), choose the smallest
    // logical pin number for use in the generated net name so that the net naming
    // is deterministic and uses the lowest-value logical pin.  (Requirement: when
    // naming nets for a stacked pin, use the smallest logical pin name.)
    wxString effectivePadNumber = libPinShownNumber;

    if( m_libPin )
    {
        bool stackedValid = false;
        std::vector<wxString> logicalNums = m_libPin->GetStackedPinNumbers( &stackedValid );

        if( stackedValid && !logicalNums.empty() )
        {
            // Numbers are generated in ascending order, so front() is the smallest.
            effectivePadNumber = logicalNums.front();
            wxLogTrace( "KICAD_STACKED_PINS",
                        wxString::Format( "GetDefaultNetName: stacked pin shown='%s' -> using smallest logical='%s'",
                                          libPinShownNumber, effectivePadNumber ) );
        }
    }

    // Use timestamp for unannotated symbols
    if( symbol->GetRef( &aPath, false ).Last() == '?' )
    {
        name << GetParentSymbol()->m_Uuid.AsString();

        wxString libPinNumber = m_libPin ? m_libPin->GetNumber() : wxString( "??" );
        // Apply same smallest-logical substitution for unannotated symbols
        if( effectivePadNumber != libPinShownNumber && !effectivePadNumber.IsEmpty() )
            libPinNumber = effectivePadNumber;

        name << "-Pad" << libPinNumber << ")";
        annotated = false;
    }
    else if( !libPinShownName.IsEmpty() && ( libPinShownName != libPinShownNumber ) )
    {
        // Pin names might not be unique between different units so we must have the
        // unit token in the reference designator
        name << symbol->GetRef( &aPath, true );
        name << "-" << EscapeString( libPinShownName, CTX_NETNAME );

        if( unconnected || has_multiple )
        {
            // Use effective (possibly de-stacked) pad number in net name
            name << "-Pad" << EscapeString( effectivePadNumber, CTX_NETNAME );
        }

        name << ")";
    }
    else
    {
        // Pin numbers are unique, so we skip the unit token
        name << symbol->GetRef( &aPath, false );
        name << "-Pad" << EscapeString( effectivePadNumber, CTX_NETNAME ) << ")";
    }

    if( annotated )
        m_net_name_map[ aPath ] = std::make_pair( name, aForceNoConnect );

    return name;
}


const BOX2I SCH_PIN::ViewBBox() const
{
    return GetBoundingBox( false, true, m_flags & SHOW_ELEC_TYPE );
}


std::vector<int> SCH_PIN::ViewGetLayers() const
{
    return { LAYER_DANGLING,    LAYER_DEVICE, LAYER_SELECTION_SHADOWS,
             LAYER_OP_CURRENTS, LAYER_PINNAM, LAYER_PINNUM };
}


void SCH_PIN::validateExtentsCache( KIFONT::FONT* aFont, int aSize, const wxString& aText,
                                    EXTENTS_CACHE* aCache ) const
{
    if( aCache->m_Font == aFont
        && aCache->m_FontSize == aSize
        && aCache->m_Extents != VECTOR2I() )
    {
        return;
    }

    aCache->m_Font = aFont;
    aCache->m_FontSize = aSize;

    VECTOR2D fontSize( aSize, aSize );
    int      penWidth = GetPenSizeForNormal( aSize );

    aCache->m_Extents = aFont->StringBoundaryLimits( aText, fontSize, penWidth, false, false,
                                                       GetFontMetrics() );
}


BOX2I SCH_PIN::GetBoundingBox( bool aIncludeLabelsOnInvisiblePins, bool aIncludeNameAndNumber,
                               bool aIncludeElectricalType ) const
{
    // Just defer to the cache
    return m_layoutCache->GetPinBoundingBox( aIncludeLabelsOnInvisiblePins, aIncludeNameAndNumber,
                                             aIncludeElectricalType );
}


bool SCH_PIN::HasConnectivityChanges( const SCH_ITEM* aItem,
                                      const SCH_SHEET_PATH* aInstance ) const
{
    // Do not compare to ourself.
    if( aItem == this )
        return false;

    const SCH_PIN* pin = dynamic_cast<const SCH_PIN*>( aItem );

    // Don't compare against a different SCH_ITEM.
    wxCHECK( pin, false );

    if( GetPosition() != pin->GetPosition() )
        return true;

    if( GetNumber() != pin->GetNumber() )
        return true;

    return GetName() != pin->GetName();
}


bool SCH_PIN::ConnectionPropagatesTo( const EDA_ITEM* aItem ) const
{
    if( !m_libPin )
        return false;

    // Reciprocal checking is done in CONNECTION_GRAPH anyway
    return m_libPin->GetType() != ELECTRICAL_PINTYPE::PT_NC;
}


BITMAPS SCH_PIN::GetMenuImage() const
{
    if( m_libPin )
        return m_libPin->GetMenuImage();

    return ElectricalPinTypeGetBitmap( m_type );
}


wxString SCH_PIN::GetItemDescription( UNITS_PROVIDER* aUnitsProvider, ALT* aAlt ) const
{
    return getItemDescription( aAlt );
}


wxString SCH_PIN::GetItemDescription( UNITS_PROVIDER* aUnitsProvider, bool aFull ) const
{
    if( m_libPin )
    {
        SCH_PIN::ALT  localStorage;
        SCH_PIN::ALT* alt = nullptr;

        if( !m_alt.IsEmpty() )
        {
            localStorage = m_libPin->GetAlt( m_alt );
            alt = &localStorage;
        }

        wxString itemDesc = m_libPin ? m_libPin->GetItemDescription( aUnitsProvider, alt )
                                     : wxString( wxS( "Undefined library pin." ) );

        const SCH_SYMBOL* symbol = static_cast<const SCH_SYMBOL*>( GetParentSymbol() );

        return wxString::Format( "Symbol %s %s",
                                 UnescapeString( symbol->GetField( FIELD_T::REFERENCE )->GetText() ),
                                 itemDesc );
    }

    return getItemDescription( nullptr );
}


wxString SCH_PIN::getItemDescription( ALT* aAlt ) const
{
    wxString name = UnescapeString( aAlt ? aAlt->m_Name : GetShownName() );
    wxString electricalTypeName = ElectricalPinTypeGetText( aAlt ? aAlt->m_Type : m_type );
    wxString pinShapeName = PinShapeGetText( aAlt ? aAlt->m_Shape : m_shape );

    if( IsVisible() )
    {
        if ( !name.IsEmpty() )
        {
            return wxString::Format( _( "Pin %s [%s, %s, %s]" ),
                                     GetShownNumber(),
                                     name,
                                     electricalTypeName,
                                     pinShapeName );
        }
        else
        {
            return wxString::Format( _( "Pin %s [%s, %s]" ),
                                     GetShownNumber(),
                                     electricalTypeName,
                                     pinShapeName );
        }
    }
    else
    {
        if( !name.IsEmpty() )
        {
            return wxString::Format( _( "Hidden pin %s [%s, %s, %s]" ),
                                     GetShownNumber(),
                                     name,
                                     electricalTypeName,
                                     pinShapeName );
        }
        else
        {
            return wxString::Format( _( "Hidden pin %s [%s, %s]" ),
                                     GetShownNumber(),
                                     electricalTypeName,
                                     pinShapeName );
        }
    }
}


int SCH_PIN::compare( const SCH_ITEM& aOther, int aCompareFlags ) const
{
    // Ignore the UUID here
    // And the position, which we'll do after the number.
    int retv = SCH_ITEM::compare( aOther, aCompareFlags | SCH_ITEM::COMPARE_FLAGS::EQUALITY
                                                  | SCH_ITEM::COMPARE_FLAGS::SKIP_TST_POS );

    if( retv )
        return retv;

    const SCH_PIN* tmp = static_cast<const SCH_PIN*>( &aOther );

    wxCHECK( tmp, -1 );

    if( m_number != tmp->m_number )
    {
        // StrNumCmp: sort the same as the pads in the footprint file
        return StrNumCmp( m_number, tmp->m_number ) < 0;
    }

    if( m_position.x != tmp->m_position.x )
        return m_position.x - tmp->m_position.x;

    if( m_position.y != tmp->m_position.y )
        return m_position.y - tmp->m_position.y;

    if( dynamic_cast<const SCH_SYMBOL*>( GetParentSymbol() ) )
    {
        if( ( m_libPin == nullptr ) || ( tmp->m_libPin == nullptr ) )
            return -1;

        retv = m_libPin->compare( *tmp->m_libPin );

        if( retv )
            return retv;

        retv = m_alt.Cmp( tmp->m_alt );

        if( retv )
            return retv;
    }

    if( dynamic_cast<const LIB_SYMBOL*>( GetParentSymbol() ) )
    {
        if( m_length != tmp->m_length )
            return m_length.value_or( 0 ) - tmp->m_length.value_or( 0 );

        if( m_orientation != tmp->m_orientation )
            return static_cast<int>( m_orientation ) - static_cast<int>( tmp->m_orientation );

        if( m_shape != tmp->m_shape )
            return static_cast<int>( m_shape ) - static_cast<int>( tmp->m_shape );

        if( m_type != tmp->m_type )
            return static_cast<int>( m_type ) - static_cast<int>( tmp->m_type );

        if( m_hidden != tmp->m_hidden )
            return m_hidden.value_or( false ) - tmp->m_hidden.value_or( false );

        if( m_numTextSize != tmp->m_numTextSize )
            return m_numTextSize.value_or( 0 ) - tmp->m_numTextSize.value_or( 0 );

        if( m_nameTextSize != tmp->m_nameTextSize )
            return m_nameTextSize.value_or( 0 ) - tmp->m_nameTextSize.value_or( 0 );

        if( m_alternates.size() != tmp->m_alternates.size() )
            return static_cast<int>( m_alternates.size() - tmp->m_alternates.size() );

        auto lhsItem = m_alternates.begin();
        auto rhsItem = tmp->m_alternates.begin();

        while( lhsItem != m_alternates.end() )
        {
            const ALT& lhsAlt = lhsItem->second;
            const ALT& rhsAlt = rhsItem->second;

            retv = lhsAlt.m_Name.Cmp( rhsAlt.m_Name );

            if( retv )
                return retv;

            if( lhsAlt.m_Type != rhsAlt.m_Type )
                return static_cast<int>( lhsAlt.m_Type ) - static_cast<int>( rhsAlt.m_Type );

            if( lhsAlt.m_Shape != rhsAlt.m_Shape )
                return static_cast<int>( lhsAlt.m_Shape ) - static_cast<int>( rhsAlt.m_Shape );

            ++lhsItem;
            ++rhsItem;
        }
    }

    return 0;
}


double SCH_PIN::Similarity( const SCH_ITEM& aOther ) const
{
    if( aOther.m_Uuid == m_Uuid )
        return 1.0;

    if( aOther.Type() != SCH_PIN_T )
        return 0.0;

    const SCH_PIN* other = static_cast<const SCH_PIN*>( &aOther );

    if( m_libPin )
    {
        if( m_number != other->m_number )
            return 0.0;

        if( m_position != other->m_position )
            return 0.0;

        return m_libPin->Similarity( *other->m_libPin );
    }

    double similarity = SimilarityBase( aOther );

    if( m_name != other->m_name )
        similarity *= 0.9;

    if( m_number != other->m_number )
        similarity *= 0.9;

    if( m_position != other->m_position )
        similarity *= 0.9;

    if( m_length != other->m_length )
        similarity *= 0.9;

    if( m_orientation != other->m_orientation )
        similarity *= 0.9;

    if( m_shape != other->m_shape )
        similarity *= 0.9;

    if( m_type != other->m_type )
        similarity *= 0.9;

    if( m_hidden != other->m_hidden )
        similarity *= 0.9;

    if( m_numTextSize != other->m_numTextSize )
        similarity *= 0.9;

    if( m_nameTextSize != other->m_nameTextSize )
        similarity *= 0.9;

    if( m_alternates.size() != other->m_alternates.size() )
        similarity *= 0.9;

    return similarity;
}


std::ostream& SCH_PIN::operator<<( std::ostream& aStream )
{
    aStream << "SCH_PIN:" << std::endl
            << "  Name: \"" << m_name << "\"" << std::endl
            << "  Number: \"" << m_number << "\"" << std::endl
            << "  Position: " << m_position << std::endl
            << "  Length: " << GetLength() << std::endl
            << "  Orientation: " << PinOrientationName( m_orientation ) << std::endl
            << "  Shape: " << PinShapeGetText( m_shape ) << std::endl
            << "  Type: " << ElectricalPinTypeGetText( m_type ) << std::endl
            << "  Name Text Size: " << GetNameTextSize() << std::endl
            << "  Number Text Size: " << GetNumberTextSize() << std::endl;

    return aStream;
}


#if defined(DEBUG)

void SCH_PIN::Show( int nestLevel, std::ostream& os ) const
{
    NestedSpace( nestLevel, os ) << '<' << GetClass().Lower().mb_str()
                                 << " num=\"" << m_number.mb_str()
                                 << '"' << "/>\n";
}

#endif


void SCH_PIN::CalcEdit( const VECTOR2I& aPosition )
{
    if( IsMoving() )
        SetPosition( aPosition );
}


static struct SCH_PIN_DESC
{
    SCH_PIN_DESC()
    {
        auto& pinTypeEnum = ENUM_MAP<ELECTRICAL_PINTYPE>::Instance();

        if( pinTypeEnum.Choices().GetCount() == 0 )
        {
            pinTypeEnum.Map( ELECTRICAL_PINTYPE::PT_INPUT,         _HKI( "Input" ) )
                       .Map( ELECTRICAL_PINTYPE::PT_OUTPUT,        _HKI( "Output" ) )
                       .Map( ELECTRICAL_PINTYPE::PT_BIDI,          _HKI( "Bidirectional" ) )
                       .Map( ELECTRICAL_PINTYPE::PT_TRISTATE,      _HKI( "Tri-state" ) )
                       .Map( ELECTRICAL_PINTYPE::PT_PASSIVE,       _HKI( "Passive" ) )
                       .Map( ELECTRICAL_PINTYPE::PT_NIC,           _HKI( "Free" ) )
                       .Map( ELECTRICAL_PINTYPE::PT_UNSPECIFIED,   _HKI( "Unspecified" ) )
                       .Map( ELECTRICAL_PINTYPE::PT_POWER_IN,      _HKI( "Power input" ) )
                       .Map( ELECTRICAL_PINTYPE::PT_POWER_OUT,     _HKI( "Power output" ) )
                       .Map( ELECTRICAL_PINTYPE::PT_OPENCOLLECTOR, _HKI( "Open collector" ) )
                       .Map( ELECTRICAL_PINTYPE::PT_OPENEMITTER,   _HKI( "Open emitter" ) )
                       .Map( ELECTRICAL_PINTYPE::PT_NC,            _HKI( "Unconnected" ) );
        }

        auto& pinShapeEnum = ENUM_MAP<GRAPHIC_PINSHAPE>::Instance();

        if( pinShapeEnum.Choices().GetCount() == 0 )
        {
            pinShapeEnum.Map( GRAPHIC_PINSHAPE::LINE,               _HKI( "Line" ) )
                        .Map( GRAPHIC_PINSHAPE::INVERTED,           _HKI( "Inverted" ) )
                        .Map( GRAPHIC_PINSHAPE::CLOCK,              _HKI( "Clock" ) )
                        .Map( GRAPHIC_PINSHAPE::INVERTED_CLOCK,     _HKI( "Inverted clock" ) )
                        .Map( GRAPHIC_PINSHAPE::INPUT_LOW,          _HKI( "Input low" ) )
                        .Map( GRAPHIC_PINSHAPE::CLOCK_LOW,          _HKI( "Clock low" ) )
                        .Map( GRAPHIC_PINSHAPE::OUTPUT_LOW,         _HKI( "Output low" ) )
                        .Map( GRAPHIC_PINSHAPE::FALLING_EDGE_CLOCK, _HKI( "Falling edge clock" ) )
                        .Map( GRAPHIC_PINSHAPE::NONLOGIC,           _HKI( "NonLogic" ) );
        }

        auto& orientationEnum = ENUM_MAP<PIN_ORIENTATION>::Instance();

        if( orientationEnum.Choices().GetCount() == 0 )
        {
            orientationEnum.Map( PIN_ORIENTATION::PIN_RIGHT, _( "Right" ) )
                           .Map( PIN_ORIENTATION::PIN_LEFT,  _( "Left" ) )
                           .Map( PIN_ORIENTATION::PIN_UP,    _( "Up" ) )
                           .Map( PIN_ORIENTATION::PIN_DOWN,  _( "Down" ) );
        }

        auto isSymbolEditor =
                []( INSPECTABLE* aItem ) -> bool
                {
                    if( SCH_PIN* pin = dynamic_cast<SCH_PIN*>( aItem ) )
                        return dynamic_cast<LIB_SYMBOL*>( pin->GetParentSymbol() ) != nullptr;

                    return false;
                };

        PROPERTY_MANAGER& propMgr = PROPERTY_MANAGER::Instance();
        REGISTER_TYPE( SCH_PIN );
        propMgr.AddTypeCast( new TYPE_CAST<SCH_PIN, SCH_ITEM> );
        propMgr.InheritsAfter( TYPE_HASH( SCH_PIN ), TYPE_HASH( SCH_ITEM ) );

        propMgr.AddProperty( new PROPERTY<SCH_PIN, wxString>( _HKI( "Pin Name" ),
                    &SCH_PIN::SetName, &SCH_PIN::GetName ) )
                .SetWriteableFunc( isSymbolEditor );

        propMgr.AddProperty( new PROPERTY<SCH_PIN, wxString>( _HKI( "Pin Number" ),
                    &SCH_PIN::SetNumber, &SCH_PIN::GetNumber ) )
                .SetWriteableFunc( isSymbolEditor );

        propMgr.AddProperty( new PROPERTY_ENUM<SCH_PIN, ELECTRICAL_PINTYPE>( _HKI( "Electrical Type" ),
                    &SCH_PIN::SetType, &SCH_PIN::GetType ) )
                .SetWriteableFunc( isSymbolEditor );

        propMgr.AddProperty( new PROPERTY_ENUM<SCH_PIN, GRAPHIC_PINSHAPE>( _HKI( "Graphic Style" ),
                    &SCH_PIN::SetShape, &SCH_PIN::GetShape ) )
                .SetWriteableFunc( isSymbolEditor );

        propMgr.AddProperty( new PROPERTY<SCH_PIN, int>( _HKI( "Position X" ),
                    &SCH_PIN::SetX, &SCH_PIN::GetX, PROPERTY_DISPLAY::PT_COORD ) )
                .SetAvailableFunc( isSymbolEditor );

        propMgr.AddProperty( new PROPERTY<SCH_PIN, int>( _HKI( "Position Y" ),
                    &SCH_PIN::SetY, &SCH_PIN::GetY, PROPERTY_DISPLAY::PT_COORD ) )
                .SetAvailableFunc( isSymbolEditor );

        propMgr.AddProperty( new PROPERTY_ENUM<SCH_PIN, PIN_ORIENTATION>( _HKI( "Orientation" ),
                    &SCH_PIN::SetOrientation, &SCH_PIN::GetOrientation ) )
                .SetWriteableFunc( isSymbolEditor );

        propMgr.AddProperty( new PROPERTY<SCH_PIN, int>( _HKI( "Length" ),
                    &SCH_PIN::ChangeLength, &SCH_PIN::GetLength,
                    PROPERTY_DISPLAY::PT_SIZE ) )
                .SetWriteableFunc( isSymbolEditor );

        propMgr.AddProperty( new PROPERTY<SCH_PIN, int>( _HKI( "Name Text Size" ),
                    &SCH_PIN::SetNameTextSize, &SCH_PIN::GetNameTextSize,
                    PROPERTY_DISPLAY::PT_SIZE ) )
                .SetAvailableFunc( isSymbolEditor );

        propMgr.AddProperty( new PROPERTY<SCH_PIN, int>( _HKI( "Number Text Size" ),
                    &SCH_PIN::SetNumberTextSize, &SCH_PIN::GetNumberTextSize,
                    PROPERTY_DISPLAY::PT_SIZE ) )
                .SetAvailableFunc( isSymbolEditor );

        propMgr.AddProperty( new PROPERTY<SCH_PIN, bool>( _HKI( "Visible" ),
                    &SCH_PIN::SetVisible, &SCH_PIN::IsVisible ) )
                .SetAvailableFunc( isSymbolEditor );

    }
} _SCH_PIN_DESC;


ENUM_TO_WXANY( PIN_ORIENTATION )
ENUM_TO_WXANY( GRAPHIC_PINSHAPE )
ENUM_TO_WXANY( ELECTRICAL_PINTYPE )
