/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2020 Thomas Pointhuber <thomas.pointhuber@gmx.at>
 * Copyright (C) 2020 KiCad Developers, see AUTHORS.txt for contributors.
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

#include <core/type_helpers.h>
#include <bitmaps.h>
#include <gr_basic.h>
#include <macros.h>
#include <pcb_edit_frame.h>
#include <richio.h>
#include <trigo.h>

#include <base_units.h>
#include <pcb_barcode.h>
#include <board.h>
#include <geometry/shape_poly_set.h>
#include <pcb_text.h>
#include <math/util.h> // for KiROUND
#include <convert_basic_shapes_to_polygon.h>
#include <wx/log.h>
#include <pgm_base.h>
#include <settings/color_settings.h>
#include <settings/settings_manager.h>
#include <scoped_set_reset.h>
#include <stdexcept>

#include <backend/zint.h>

PCB_BARCODE::PCB_BARCODE( BOARD_ITEM* aParent ) :
        BOARD_ITEM( aParent, PCB_BARCODE_T ),
        m_width( pcbIUScale.mmToIU( 40 ) ),
        m_height( pcbIUScale.mmToIU( 40 ) ),
        m_pos( 0, 0 ),
        m_text( this ),
        m_kind( BARCODE_T::QR_CODE ),
        m_angle( 0 )
{
    m_layer = Dwgs_User;
}


PCB_BARCODE::~PCB_BARCODE()
{
}


void PCB_BARCODE::SetPosition( const VECTOR2I& aPos )
{
    VECTOR2I delta = aPos - m_pos;
    m_text.Offset( delta );
    m_pos = aPos;
}


VECTOR2I PCB_BARCODE::GetPosition() const
{
    return m_pos;
}


void PCB_BARCODE::SetText( const wxString& aNewText )
{
    m_text.SetText( aNewText );
}


wxString PCB_BARCODE::GetText() const
{
    return m_text.GetText();
}


wxString PCB_BARCODE::GetShownText() const
{
    return m_text.GetShownText( true );
}


void PCB_BARCODE::SetLayer( PCB_LAYER_ID aLayer )
{
    m_layer = aLayer;
    m_text.SetLayer( aLayer );
}


void PCB_BARCODE::Move( const VECTOR2I& offset )
{
    m_pos += offset;
    m_text.Move( offset );
}


void PCB_BARCODE::Rotate( const VECTOR2I& aRotCentre, const EDA_ANGLE& aAngle )
{
    m_poly.Rotate( aAngle, m_poly.BBox().GetCenter() );
    m_text.Rotate( aRotCentre, aAngle );
    RotatePoint( m_pos, aRotCentre, aAngle );
    BOX2I bbox = m_poly.BBox();
    m_width = bbox.GetWidth();
    m_height = bbox.GetHeight();

    m_angle += aAngle;
}


void PCB_BARCODE::Flip( const VECTOR2I& aCentre, FLIP_DIRECTION aFlipLeftRight )
{
    BOARD* board = GetBoard();
    SetLayer( FlipLayer( GetLayer(), board ? board->GetCopperLayerCount() : 0 ) );
}


void PCB_BARCODE::ComputeBarcode()
{
    m_poly.RemoveAllContours();

    std::unique_ptr<zint_symbol, decltype( &ZBarcode_Delete )> symbol( ZBarcode_Create(), &ZBarcode_Delete );

    if( !symbol )
    {
        wxLogError( wxT( "Zint: failed to allocate symbol" ) );
        return;
    }

    symbol->input_mode = UNICODE_MODE;
    symbol->show_hrt = 0; // do not show HRT
    switch( m_kind )
    {
    case BARCODE_T::CODE_39: symbol->symbology = BARCODE_CODE39; break;
    case BARCODE_T::CODE_128: symbol->symbology = BARCODE_CODE128; break;
    case BARCODE_T::QR_CODE:
        symbol->symbology = BARCODE_QRCODE;
        symbol->option_1 = to_underlying( m_errorCorrection );
        break;
    case BARCODE_T::MICRO_QR_CODE:
        symbol->symbology = BARCODE_MICROQR;
        symbol->option_1 = to_underlying( m_errorCorrection );
        break;
    case BARCODE_T::DATA_MATRIX: symbol->symbology = BARCODE_DATAMATRIX; break;
    default: wxLogError( wxT( "Zint: invalid barcode type" ) ); return;
    }


    wxString text = GetText();

    if( text.empty() )
        return;

    if( ZBarcode_Encode( symbol.get(), text.c_str(), text.length() ) )
    {
        wxLogDebug( wxT( "Zint encode error: %s" ), wxString::FromUTF8( symbol->errtxt ) );
        return;
    }

    if( ZBarcode_Buffer_Vector( symbol.get(), 0 ) ) // 0 means success
    {
        wxLogDebug( wxT( "Zint render error: %s" ), wxString::FromUTF8( symbol->errtxt ) );
        return;
    }

    for( zint_vector_rect* rect = symbol->vector->rectangles; rect != nullptr; rect = rect->next )
    {
        int x1 = KiROUND( rect->x );
        int x2 = x1 + KiROUND( rect->width );
        int y1 = KiROUND( rect->y );
        int y2 = y1 - KiROUND( rect->height );

        SHAPE_LINE_CHAIN shapeline;
        shapeline.Append( x1, y1 );
        shapeline.Append( x2, y1 );
        shapeline.Append( x2, y2 );
        shapeline.Append( x1, y2 );
        shapeline.SetClosed( true );

        m_poly.AddOutline( shapeline );
    }

    for( zint_vector_hexagon* hex = symbol->vector->hexagons; hex != nullptr; hex = hex->next )
    {
        int radius = KiROUND( hex->diameter / 2.0f );
        int cx = KiROUND( hex->x );
        int cy = KiROUND( hex->y );

        SHAPE_LINE_CHAIN poly;
        poly.Append( cx, cy + radius );
        poly.Append( cx + KiROUND( 0.86f * radius ), cy + KiROUND( 0.5f * radius ) );
        poly.Append( cx + KiROUND( 0.86f * radius ), cy - KiROUND( 0.5f * radius ) );
        poly.Append( cx, cy - radius );
        poly.Append( cx - KiROUND( 0.86f * radius ), cy - KiROUND( 0.5f * radius ) );
        poly.Append( cx - KiROUND( 0.86f * radius ), cy + KiROUND( 0.5f * radius ) );
        poly.SetClosed( true );

        m_poly.AddOutline( poly );
    }

    if( m_isKnockout )
    {
        // Compute rectangle anchored at the barcode top-left/bottom-right.
        // m_pos is the centre; use GetTopLeft/GetBotRight to get correct corners.
        VECTOR2I topLeft = GetTopLeft();
        VECTOR2I botRight = GetBotRight();

        topLeft -= m_margin;            // move top-left further up/left
        botRight += m_margin;          // move bottom-right further down/right

        // Offset the buffer by the margin so the boolean subtraction aligns as intended.
        if( m_margin.x || m_margin.y )
            m_poly.Move( m_margin );

        SHAPE_LINE_CHAIN rect;
        rect.Append( topLeft );
        rect.Append( VECTOR2I( botRight.x, topLeft.y ) );
        rect.Append( botRight );
        rect.Append( VECTOR2I( topLeft.x, botRight.y ) );
        rect.SetClosed( true );

        SHAPE_POLY_SET rectPoly;
        rectPoly.AddOutline( rect );
        rectPoly.BooleanSubtract( m_poly );
        m_poly = rectPoly;
    }

    // Set the position of the barcode to the center of the polygon
    if( m_poly.OutlineCount() > 0 )
    {
        VECTOR2I pos = m_poly.BBox().GetCenter();
        m_poly.Move( -pos );
    }

    m_poly.CacheTriangulation();

    SetRect( m_pos - VECTOR2I( ( m_width + m_margin.x ) / 2, ( m_height + m_margin.y ) / 2 ),
             m_pos + VECTOR2I( ( m_width + m_margin.x ) / 2, ( m_height + m_margin.y ) / 2 ) );
}


// see class_cotation.h
void PCB_BARCODE::GetMsgPanelInfo( EDA_DRAW_FRAME* aFrame, std::vector<MSG_PANEL_ITEM>& aList )
{
    // for now, display only the text within the BARCODE using class TEXTE_PCB.
    m_text.GetMsgPanelInfo( aFrame, aList );
}


bool PCB_BARCODE::HitTest( const VECTOR2I& aPosition, int aAccuracy ) const
{
    if( m_text.TextHitTest( aPosition ) )
        return true;

    return GetBoundingBox().Contains( aPosition ); // TODO: simple hit test
}


bool PCB_BARCODE::HitTest( const BOX2I& aRect, bool aContained, int aAccuracy ) const
{
    BOX2I arect = aRect;
    arect.Inflate( aAccuracy );

    BOX2I rect = GetBoundingBox();

    if( aAccuracy )
        rect.Inflate( aAccuracy );

    if( aContained )
        return arect.Contains( rect );

    return arect.Intersects( rect );
}


VECTOR2I PCB_BARCODE::GetTopLeft() const
{
    return GetPosition() - VECTOR2I( m_width / 2, m_height / 2 );
}


VECTOR2I PCB_BARCODE::GetBotRight() const
{
    return GetPosition() + VECTOR2I( m_width / 2, m_height / 2 );
}


void PCB_BARCODE::SetRect( const VECTOR2I& aTopLeft, const VECTOR2I& aBotRight )
{
    BOX2I bbox = m_poly.BBox();
    int oldW = bbox.GetWidth();
    int oldH = bbox.GetHeight();

    SetPosition( ( aTopLeft + aBotRight ) / 2 );
    int newW = aBotRight.x - aTopLeft.x;
    int newH = aBotRight.y - aTopLeft.y;

    double scaleX = oldW ? static_cast<double>( newW ) / oldW : 1.0;
    double scaleY = oldH ? static_cast<double>( newH ) / oldH : 1.0;

    for( int polyIdx = 0; polyIdx < m_poly.OutlineCount(); ++polyIdx )
    {
        SHAPE_POLY_SET::POLYGON& poly = m_poly.Polygon( polyIdx );

        for( SHAPE_LINE_CHAIN& chain : poly )
        {
            for( int ii = 0; ii < chain.PointCount(); ++ii )
            {
                VECTOR2I pt = chain.CPoint( ii );
                pt.x = KiROUND( pt.x * scaleX );
                pt.y = KiROUND( pt.y * scaleY );
                chain.SetPoint( ii, pt );
            }
        }
    }

    m_width = newW;
    m_height = newH;

    m_poly.CacheTriangulation( false );
}


const BOX2I PCB_BARCODE::GetBoundingBox() const
{
    BOX2I bBox = m_text.GetBoundingBox();
    bBox.Merge( BOX2I::ByCenter( GetPosition(), VECTOR2I( m_width, m_height ) ) );

    return bBox;
}


wxString PCB_BARCODE::GetItemDescription( UNITS_PROVIDER* aUnitsProvider, bool aFull ) const
{
    return wxString::Format( _( "BARCODE \"%s\" on %s" ), GetText(), GetLayerName() );
}


BITMAPS PCB_BARCODE::GetMenuImage() const
{
    return BITMAPS::add_barcode;
}


const BOX2I PCB_BARCODE::ViewBBox() const
{
    BOX2I dimBBox = GetBoundingBox();
    dimBBox.Merge( m_text.ViewBBox() );

    return dimBBox;
}


void PCB_BARCODE::TransformShapeToPolygon( SHAPE_POLY_SET& aBuffer, PCB_LAYER_ID aLayer, int aClearance, int aMaxError,
                                           ERROR_LOC aErrorLoc, bool ignoreLineWidth ) const
{
    if( aLayer != m_layer )
        return;

    if( aClearance == 0 )
    {
        aBuffer.Append( m_poly );
    }
    else
    {
        SHAPE_POLY_SET poly = m_poly;
        poly.Inflate( aClearance, CORNER_STRATEGY::CHAMFER_ACUTE_CORNERS, aMaxError, aErrorLoc );
        aBuffer.Append( poly );
    }

}

void PCB_BARCODE::SetErrorCorrection( BARCODE_ECC_T aErrorCorrection )
{
    // Micro QR codes do not support High (H) error correction level
    if( m_kind == BARCODE_T::MICRO_QR_CODE && aErrorCorrection == BARCODE_ECC_T::H )
        m_errorCorrection = BARCODE_ECC_T::Q;
    else
        m_errorCorrection = aErrorCorrection;
    // Don't auto-compute here as it may be called during loading
}


void PCB_BARCODE::SetKind( BARCODE_T aKind )
{
    m_kind = aKind;

    // When switching to Micro QR, validate and adjust ECC if needed
    if( m_kind == BARCODE_T::MICRO_QR_CODE && m_errorCorrection == BARCODE_ECC_T::H )
        m_errorCorrection = BARCODE_ECC_T::Q;

    // Don't auto-compute here as it may be called during loading
}


void PCB_BARCODE::SetBarcodeErrorCorrection( BARCODE_ECC_T aErrorCorrection )
{
    SetErrorCorrection( aErrorCorrection );
    ComputeBarcode();
}


void PCB_BARCODE::SetBarcodeKind( BARCODE_T aKind )
{
    SetKind( aKind );
    ComputeBarcode();
}


EDA_ITEM* PCB_BARCODE::Clone() const
{
    PCB_BARCODE* item = new PCB_BARCODE( *this );
    item->CopyFrom( this );
    item->m_text.SetParent( item );
    return item;
}

double PCB_BARCODE::Similarity( const BOARD_ITEM& aItem ) const
{
    if( !ClassOf( &aItem ) )
        return 0.0;

    const PCB_BARCODE* other = static_cast<const PCB_BARCODE*>( &aItem );

    // Compare text, width, height, position, and kind
    double similarity = 0.0;
    if( GetText() == other->GetText() )
        similarity += 0.2;
    if( m_width == other->m_width )
        similarity += 0.2;
    if( m_height == other->m_height )
        similarity += 0.2;
    if( GetPosition() == other->GetPosition() )
        similarity += 0.2;
    if( m_kind == other->m_kind )
        similarity += 0.2;

    return similarity;
}

bool PCB_BARCODE::operator==( const BOARD_ITEM& aItem ) const
{
    if( !ClassOf( &aItem ) )
        return false;

    const PCB_BARCODE* other = static_cast<const PCB_BARCODE*>( &aItem );

    // Compare text, width, height, position, and kind
    return ( GetText() == other->GetText() ) && ( m_width == other->m_width ) && ( m_height == other->m_height )
           && ( GetPosition() == other->GetPosition() ) && ( m_kind == other->m_kind );
}

// ---- Property registration ----
static struct PCB_BARCODE_DESC
{
    PCB_BARCODE_DESC()
    {
        PROPERTY_MANAGER& propMgr = PROPERTY_MANAGER::Instance();
        REGISTER_TYPE( PCB_BARCODE );
        propMgr.InheritsAfter( TYPE_HASH( PCB_BARCODE ), TYPE_HASH( BOARD_ITEM ) );

        const wxString groupBarcode = _HKI( "Barcode Properties" );

        ENUM_MAP<BARCODE_T>& kindMap = ENUM_MAP<BARCODE_T>::Instance();
        if( kindMap.Choices().GetCount() == 0 )
        {
            kindMap.Undefined( BARCODE_T::QR_CODE );
            kindMap.Map( BARCODE_T::CODE_39,       _HKI( "CODE_39" ) )
                   .Map( BARCODE_T::CODE_128,      _HKI( "CODE_128" ) )
                   .Map( BARCODE_T::DATA_MATRIX,   _HKI( "DATA_MATRIX" ) )
                   .Map( BARCODE_T::QR_CODE,       _HKI( "QR_CODE" ) )
                   .Map( BARCODE_T::MICRO_QR_CODE, _HKI( "MICRO_QR_CODE" ) );
        }

        ENUM_MAP<BARCODE_ECC_T>& eccMap = ENUM_MAP<BARCODE_ECC_T>::Instance();
        if( eccMap.Choices().GetCount() == 0 )
        {
            eccMap.Undefined( BARCODE_ECC_T::L );
            eccMap.Map( BARCODE_ECC_T::L, _HKI( "L (Low)" ) )
                 .Map( BARCODE_ECC_T::M, _HKI( "M (Medium)" ) )
                 .Map( BARCODE_ECC_T::Q, _HKI( "Q (Quartile)" ) )
                 .Map( BARCODE_ECC_T::H, _HKI( "H (High)" ) );
        }

        auto hasKnockout = []( INSPECTABLE* aItem ) -> bool
        {
            if( PCB_BARCODE* bc = dynamic_cast<PCB_BARCODE*>( aItem ) )
                return bc->IsKnockout();
            return false;
        };

        propMgr.AddProperty( new PROPERTY<PCB_BARCODE, wxString>( _HKI( "Text" ),
                                    &PCB_BARCODE::SetBarcodeText, &PCB_BARCODE::GetText ), groupBarcode );

        propMgr.AddProperty( new PROPERTY<PCB_BARCODE, bool>( _HKI( "Show Text" ),
                                    &PCB_BARCODE::SetShowText, &PCB_BARCODE::GetShowText ), groupBarcode );

        propMgr.AddProperty( new PROPERTY<PCB_BARCODE, int>( _HKI( "Width" ),
                                    &PCB_BARCODE::SetBarcodeWidth, &PCB_BARCODE::GetWidth,
                                    PROPERTY_DISPLAY::PT_COORD ), groupBarcode );

        propMgr.AddProperty( new PROPERTY<PCB_BARCODE, int>( _HKI( "Height" ),
                                    &PCB_BARCODE::SetBarcodeHeight, &PCB_BARCODE::GetHeight,
                                    PROPERTY_DISPLAY::PT_COORD ), groupBarcode );

        propMgr.AddProperty( new PROPERTY<PCB_BARCODE, double>( _HKI( "Orientation" ),
                                    &PCB_BARCODE::SetOrientation, &PCB_BARCODE::GetOrientation ), groupBarcode );

        propMgr.AddProperty( new PROPERTY_ENUM<PCB_BARCODE, BARCODE_T>( _HKI( "Kind" ),
                                    &PCB_BARCODE::SetBarcodeKind, &PCB_BARCODE::GetKind ), groupBarcode );

        auto isQRCode = []( INSPECTABLE* aItem ) -> bool
        {
            if( PCB_BARCODE* bc = dynamic_cast<PCB_BARCODE*>( aItem ) )
                return bc->GetKind() == BARCODE_T::QR_CODE;
            return false;
        };

        auto isMicroQR = []( INSPECTABLE* aItem ) -> bool
        {
            if( PCB_BARCODE* bc = dynamic_cast<PCB_BARCODE*>( aItem ) )
                return bc->GetKind() == BARCODE_T::MICRO_QR_CODE;
            return false;
        };

        // QR Code Error Correction (all levels including High)
        auto qrEccProp = new PROPERTY_ENUM<PCB_BARCODE, BARCODE_ECC_T>( _HKI( "Error Correction" ),
                                    &PCB_BARCODE::SetBarcodeErrorCorrection, &PCB_BARCODE::GetErrorCorrection );
        qrEccProp->SetAvailableFunc( isQRCode );
        propMgr.AddProperty( qrEccProp, groupBarcode );

        // Micro QR Code Error Correction (limited levels - no High)
        // We need a unique name for the properties panel so that we can conditionally display the dropdown
        // I've been unable to figure out how to conditionally limit which drop down choices are available
        // So I'll just create a separate property for Micro QR
        auto microQrEccProp = new PROPERTY_ENUM<PCB_BARCODE, BARCODE_ECC_T>( _HKI( "MicroQR Error Correction" ),
                                    &PCB_BARCODE::SetBarcodeErrorCorrection, &PCB_BARCODE::GetErrorCorrection );
        microQrEccProp->SetAvailableFunc( isMicroQR );

        // Create custom choices for Micro QR (excluding High)
        wxPGChoices microQrChoices;
        microQrChoices.Add( _( "L (Low)" ), static_cast<int>( BARCODE_ECC_T::L ) );
        microQrChoices.Add( _( "M (Medium)" ), static_cast<int>( BARCODE_ECC_T::M ) );
        microQrChoices.Add( _( "Q (Quartile)" ), static_cast<int>( BARCODE_ECC_T::Q ) );
        microQrEccProp->SetChoices( microQrChoices );

        propMgr.AddProperty( microQrEccProp, groupBarcode );

        propMgr.AddProperty( new PROPERTY<PCB_BARCODE, bool>( _HKI( "Knockout" ),
                                    &PCB_BARCODE::SetIsKnockout, &PCB_BARCODE::IsKnockout ), groupBarcode );

        propMgr.AddProperty( new PROPERTY<PCB_BARCODE, int>( _HKI( "Margin X" ),
                                    &PCB_BARCODE::SetMarginX, &PCB_BARCODE::GetMarginX,
                                    PROPERTY_DISPLAY::PT_COORD ), groupBarcode ).SetAvailableFunc( hasKnockout );

        propMgr.AddProperty( new PROPERTY<PCB_BARCODE, int>( _HKI( "Margin Y" ),
                                    &PCB_BARCODE::SetMarginY, &PCB_BARCODE::GetMarginY,
                                    PROPERTY_DISPLAY::PT_COORD ), groupBarcode ).SetAvailableFunc( hasKnockout );
    }
} _PCB_BARCODE_DESC;

// wxAny conversion implementations for enum properties (declarations in header)
IMPLEMENT_ENUM_TO_WXANY( BARCODE_T );
IMPLEMENT_ENUM_TO_WXANY( BARCODE_ECC_T );
