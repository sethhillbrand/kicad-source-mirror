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

#include <bitmaps.h>
#include <fctsys.h>
#include <gr_basic.h>
#include <kicad_string.h>
#include <macros.h>
#include <pcb_edit_frame.h>
#include <richio.h>
#include <trigo.h>

#include <base_units.h>
#include <class_barcode.h>
#include <class_board.h>
#include <class_pcb_text.h>
#include <math/util.h> // for KiROUND
#include <pgm_base.h>
#include <settings/color_settings.h>
#include <settings/settings_manager.h>


BARCODE::BARCODE( BOARD_ITEM* aParent )
        : BOARD_ITEM( aParent, PCB_BARCODE_T ),
          m_Width( Millimeter2iu( 40 ) ),
          m_Height( Millimeter2iu( 40 ) ),
          m_Text( this ),
          m_Kind( BARCODE_T::CODE_39 )
{
    m_Layer = Dwgs_User;
}


BARCODE::~BARCODE()
{
}


void BARCODE::SetPosition( const wxPoint& aPos )
{
    m_Text.SetTextPos( aPos );
}


const wxPoint BARCODE::GetPosition() const
{
    return m_Text.GetTextPos();
}


void BARCODE::SetText( const wxString& aNewText )
{
    m_Text.SetText( aNewText );
}


const wxString BARCODE::GetText() const
{
    return m_Text.GetText();
}


void BARCODE::SetLayer( PCB_LAYER_ID aLayer )
{
    m_Layer = aLayer;
    m_Text.SetLayer( aLayer );
}


void BARCODE::Move( const wxPoint& offset )
{
    m_Text.Offset( offset );

    // TODO
}


void BARCODE::Rotate( const wxPoint& aRotCentre, double aAngle )
{
    wxPoint tmp = m_Text.GetTextPos();
    RotatePoint( &tmp, aRotCentre, aAngle );
    m_Text.SetTextPos( tmp );

    double newAngle = m_Text.GetTextAngle() + aAngle;

    if( newAngle >= 3600 )
        newAngle -= 3600;

    if( newAngle > 900 && newAngle < 2700 )
        newAngle -= 1800;

    m_Text.SetTextAngle( newAngle );

    // TODO
}


void BARCODE::Flip( const wxPoint& aCentre, bool aFlipLeftRight )
{

    // BARCODE items are not usually on copper layers, so
    // copper layers count is not taken in accoun in Flip transform
    SetLayer( FlipLayer( GetLayer() ) );
}


void BARCODE::ComputeBarcode()
{
    // TODO
}


// see class_cotation.h
void BARCODE::GetMsgPanelInfo( EDA_DRAW_FRAME* aFrame, std::vector<MSG_PANEL_ITEM>& aList )
{
    // for now, display only the text within the BARCODE using class TEXTE_PCB.
    m_Text.GetMsgPanelInfo( aFrame, aList );
}


bool BARCODE::HitTest( const wxPoint& aPosition, int aAccuracy ) const
{
    if( m_Text.TextHitTest( aPosition ) )
        return true;

    return GetBoundingBox().Contains( aPosition ); // TODO: simple hit test
}


bool BARCODE::HitTest( const EDA_RECT& aRect, bool aContained, int aAccuracy ) const
{
    EDA_RECT arect = aRect;
    arect.Inflate( aAccuracy );

    EDA_RECT rect = GetBoundingBox();
    if( aAccuracy )
        rect.Inflate( aAccuracy );

    if( aContained )
        return arect.Contains( rect );

    return arect.Intersects( rect );
}


const EDA_RECT BARCODE::GetBoundingBox() const
{
    EDA_RECT bBox;
    int      xmin, xmax, ymin, ymax;

    bBox = m_Text.GetTextBox();
    xmin = bBox.GetX();
    xmax = bBox.GetRight();
    ymin = bBox.GetY();
    ymax = bBox.GetBottom();

    xmin = std::min( xmin, GetPosition().x - m_Width / 2 );
    xmax = std::min( xmax, GetPosition().x + m_Width / 2 );
    ymin = std::min( ymin, GetPosition().y - m_Height / 2 );
    ymax = std::min( ymax, GetPosition().y + m_Height / 2 );

    bBox.SetX( xmin );
    bBox.SetY( ymin );
    bBox.SetWidth( xmax - xmin + 1 );
    bBox.SetHeight( ymax - ymin + 1 );

    bBox.Normalize();

    return bBox;
}


wxString BARCODE::GetSelectMenuText( EDA_UNITS aUnits ) const
{
    return wxString::Format( _( "BARCODE \"%s\" on %s" ), GetText(), GetLayerName() );
}


BITMAP_DEF BARCODE::GetMenuImage() const
{
    return add_dimension_xpm; // TOOD
}


const BOX2I BARCODE::ViewBBox() const
{
    BOX2I dimBBox = BOX2I(
            VECTOR2I( GetBoundingBox().GetPosition() ), VECTOR2I( GetBoundingBox().GetSize() ) );
    dimBBox.Merge( m_Text.ViewBBox() );

    return dimBBox;
}


EDA_ITEM* BARCODE::Clone() const
{
    return new BARCODE( *this );
}

void BARCODE::SwapData( BOARD_ITEM* aImage )
{
    assert( aImage->Type() == PCB_BARCODE_T );

    std::swap( *( (BARCODE*) this ), *( (BARCODE*) aImage ) );
}
