/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright The KiCad Developers, see AUTHORS.txt for contributors.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "pcb_stackup_table.h"
#include "stackup_predefined_prms.h"

#include <base_units.h>
#include <board_commit.h>
#include <board_design_settings.h>
#include <board_stackup_manager/board_stackup.h>
#include <pcb_tablecell.h>
#include <pcb_text.h>
#include <trigo.h>
#include <scoped_set_reset.h>
#include <string_utils.h>
#include <wx/translation.h>

PCB_STACKUP_TABLE::PCB_STACKUP_TABLE( BOARD_ITEM* aParent, PCB_LAYER_ID aLayer ) :
        PCB_TABLE( aParent, pcbIUScale.mmToIU( DEFAULT_LINE_WIDTH ) ),
        m_unitsMode( STACKUP_UNIT_MODE::AUTO ),
        m_precision( 3 ),
        m_headerTextSize( VECTOR2I( pcbIUScale.mmToIU( 2.0 ), pcbIUScale.mmToIU( 2.0 ) ) ),
        m_headerTextThickness( pcbIUScale.mmToIU( 0.4 ) ),
        m_bodyTextSize( VECTOR2I( pcbIUScale.mmToIU( 1.5 ), pcbIUScale.mmToIU( 1.5 ) ) ),
        m_bodyTextThickness( pcbIUScale.mmToIU( 0.2 ) ),
        m_title( _( "Board Stackup" ) ),
        m_verticalMargin( 0 ),
        m_titleItem( nullptr )
{
    SetLayer( aLayer );

    m_columns = { { STACKUP_TABLE_COLUMN_ID::LAYER_NAME, _( "Layer Name" ), true },
                  { STACKUP_TABLE_COLUMN_ID::TYPE, _( "Type" ), true },
                  { STACKUP_TABLE_COLUMN_ID::MATERIAL, _( "Material" ), true },
                  { STACKUP_TABLE_COLUMN_ID::THICKNESS, _( "Thickness" ), true },
                  { STACKUP_TABLE_COLUMN_ID::COLOR, _( "Color" ), true },
                  { STACKUP_TABLE_COLUMN_ID::EPSILON_R, _( "Epsilon R" ), true },
                  { STACKUP_TABLE_COLUMN_ID::LOSS_TANGENT, _( "Loss Tangent" ), true } };

    if( BOARD* brd = dynamic_cast<BOARD*>( aParent ) )
        brd->AddListener( this );
}

PCB_STACKUP_TABLE::PCB_STACKUP_TABLE( const PCB_STACKUP_TABLE& aOther ) :
        PCB_TABLE( aOther ),
        m_unitsMode( aOther.m_unitsMode ),
        m_precision( aOther.m_precision ),
        m_columns( aOther.m_columns ),
        m_headerTextSize( aOther.m_headerTextSize ),
        m_headerTextThickness( aOther.m_headerTextThickness ),
        m_bodyTextSize( aOther.m_bodyTextSize ),
        m_bodyTextThickness( aOther.m_bodyTextThickness ),
        m_title( aOther.m_title ),
        m_verticalMargin( aOther.m_verticalMargin ),
        m_titleItem( nullptr )
{
    m_updating = false;
    EDA_ITEM::SetParent( nullptr );

    if( aOther.m_titleItem )
    {
        m_titleItem = new PCB_TEXT( *aOther.m_titleItem );
        m_titleItem->SetParent( this );
    }
}

PCB_STACKUP_TABLE::~PCB_STACKUP_TABLE()
{
    if( BOARD* brd = GetBoard() )
        brd->RemoveListener( this );

    if( m_titleItem )
        delete m_titleItem;
}

void PCB_STACKUP_TABLE::Move( const VECTOR2I& aMoveVector )
{
    PCB_TABLE::Move( aMoveVector );

    if( m_titleItem )
        m_titleItem->Move( aMoveVector );
}

void PCB_STACKUP_TABLE::Rotate( const VECTOR2I& aRotCentre, const EDA_ANGLE& aAngle )
{
    PCB_TABLE::Rotate( aRotCentre, aAngle );

    if( m_titleItem )
        m_titleItem->Rotate( aRotCentre, aAngle );
}

void PCB_STACKUP_TABLE::Flip( const VECTOR2I& aCentre, FLIP_DIRECTION aFlipDirection )
{
    PCB_TABLE::Flip( aCentre, aFlipDirection );

    if( m_titleItem )
    {
        m_titleItem->Flip( aCentre, aFlipDirection );
        m_titleItem->SetLayer( GetLayer() );
    }
}

void PCB_STACKUP_TABLE::Mirror( const VECTOR2I& aCentre, FLIP_DIRECTION aFlipDirection )
{
    for( PCB_TABLECELL* cell : GetCells() )
        cell->Mirror( aCentre, aFlipDirection );

    if( m_titleItem )
        m_titleItem->Mirror( aCentre, aFlipDirection );

    Normalize();
}

void PCB_STACKUP_TABLE::SetColumnVisible( STACKUP_TABLE_COLUMN_ID aColumn, bool aVisible )
{
    for( STACKUP_COLUMN_DEF& col : m_columns )
    {
        if( col.id == aColumn )
        {
            col.visible = aVisible;
            break;
        }
    }
}

bool PCB_STACKUP_TABLE::IsColumnVisible( STACKUP_TABLE_COLUMN_ID aColumn ) const
{
    for( const STACKUP_COLUMN_DEF& col : m_columns )
    {
        if( col.id == aColumn )
            return col.visible;
    }

    return false;
}

void PCB_STACKUP_TABLE::SetColumnHeader( STACKUP_TABLE_COLUMN_ID aColumn, const wxString& aHeader )
{
    for( STACKUP_COLUMN_DEF& col : m_columns )
    {
        if( col.id == aColumn )
        {
            col.header = aHeader;
            break;
        }
    }
}

STACKUP_TABLE_COLUMN_ID PCB_STACKUP_TABLE::GetVisibleColumnId( int aVisibleIndex ) const
{
    int idx = 0;

    for( const STACKUP_COLUMN_DEF& col : m_columns )
    {
        if( col.visible )
        {
            if( idx == aVisibleIndex )
                return col.id;

            ++idx;
        }
    }

    return STACKUP_TABLE_COLUMN_ID::LAYER_NAME;
}

void PCB_STACKUP_TABLE::ShowAllColumns()
{
    for( STACKUP_COLUMN_DEF& col : m_columns )
        col.visible = true;
}

void PCB_STACKUP_TABLE::SetHeaderTextStyle( const VECTOR2I& aSize, int aThickness )
{
    m_headerTextSize = aSize;
    m_headerTextThickness = aThickness;
}

void PCB_STACKUP_TABLE::SetBodyTextStyle( const VECTOR2I& aSize, int aThickness )
{
    m_bodyTextSize = aSize;
    m_bodyTextThickness = aThickness;
}

bool PCB_STACKUP_TABLE::isOurItem( BOARD_ITEM* aItem ) const
{
    if( aItem == this )
        return true;

    if( m_titleItem && aItem == m_titleItem )
        return true;

    return false;
}

bool PCB_STACKUP_TABLE::Update( BOARD* aBoard, BOARD_COMMIT* aCommit )
{
    if( !aBoard )
        return false;

    if( m_updating )
        return false;

    SCOPED_SET_RESET updating( m_updating, true );

    VECTOR2I tablePos = GetCells().empty() ? VECTOR2I( 0, 0 ) : GetPosition();

    if( m_titleItem )
    {
        if( aCommit )
            aCommit->Remove( m_titleItem );
        else
            aBoard->Delete( m_titleItem );

        m_titleItem = nullptr;
    }

    ClearCells();
    m_colWidths.clear();
    m_rowHeights.clear();

    BOARD_DESIGN_SETTINGS& settings = aBoard->GetDesignSettings();
    BOARD_STACKUP&         stackup = settings.GetStackupDescriptor();
    stackup.SynchronizeWithBoard( &settings );
    std::vector<BOARD_STACKUP_ITEM*> layers = stackup.GetList();

    EDA_UNITS units;

    switch( m_unitsMode )
    {
    case STACKUP_UNIT_MODE::MM: units = EDA_UNITS::MM; break;
    case STACKUP_UNIT_MODE::INCH: units = EDA_UNITS::INCH; break;
    case STACKUP_UNIT_MODE::AUTO:
    default: units = aBoard->GetUserUnits(); break;
    }

    int visibleCols = 0;
    for( const STACKUP_COLUMN_DEF& col : m_columns )
        if( col.visible )
            ++visibleCols;
    SetColCount( visibleCols );

    auto addHeaderCell = [&]( const wxString& text )
    {
        PCB_TABLECELL* c = new PCB_TABLECELL( this );
        c->SetTextSize( m_headerTextSize );
        c->SetTextThickness( m_headerTextThickness );
        c->SetText( text );
        AddCell( c );
    };

    auto addDataCell = [&]( const wxString& text, const char align = 'L' )
    {
        PCB_TABLECELL* c = new PCB_TABLECELL( this );
        c->SetTextSize( m_bodyTextSize );
        c->SetTextThickness( m_bodyTextThickness );
        if( align == 'R' )
            c->SetHorizJustify( GR_TEXT_H_ALIGN_RIGHT );
        c->SetText( text );
        AddCell( c );
    };

    auto formatDouble = [&]( double v )
    {
        return wxString::Format( "%.*f", m_precision, v );
    };

    auto layerThicknessString = [&]( const BOARD_STACKUP_ITEM& aStackupItem, int aSublayerId )
    {
        const int layerThickness = aStackupItem.GetThickness( aSublayerId );

        if( !aStackupItem.IsThicknessEditable() )
            return NotSpecifiedPrm();

        double   userValue = EDA_UNIT_UTILS::UI::ToUserUnit( pcbIUScale, units, layerThickness );
        wxString val = wxString::Format( "%.*f", m_precision, userValue );
        if( units == EDA_UNITS::MM )
            val += wxT( " mm" );
        else if( units == EDA_UNITS::INCH )
            val += wxT( " in" );

        return val;
    };

    for( const STACKUP_COLUMN_DEF& col : m_columns )
    {
        if( col.visible )
            addHeaderCell( col.header );
    }

    for( int i = 0; i < stackup.GetCount(); ++i )
    {
        BOARD_STACKUP_ITEM* stackup_item = layers.at( i );

        for( int sub = 0; sub < stackup_item->GetSublayersCount(); ++sub )
        {
            wxString layerName;

            if( stackup_item->GetLayerName().IsEmpty() )
            {
                if( IsValidLayer( stackup_item->GetBrdLayerId() ) )
                    layerName = aBoard->GetLayerName( stackup_item->GetBrdLayerId() );

                if( layerName.IsEmpty() && stackup_item->GetType() == BS_ITEM_TYPE_DIELECTRIC )
                    layerName = _( "Dielectric" );
            }
            else
            {
                layerName = stackup_item->GetLayerName();
            }

            for( const STACKUP_COLUMN_DEF& col : m_columns )
            {
                if( !col.visible )
                    continue;

                switch( col.id )
                {
                case STACKUP_TABLE_COLUMN_ID::LAYER_NAME: addDataCell( layerName ); break;
                case STACKUP_TABLE_COLUMN_ID::TYPE: addDataCell( InitialCaps( stackup_item->GetTypeName() ) ); break;
                case STACKUP_TABLE_COLUMN_ID::MATERIAL: addDataCell( stackup_item->GetMaterial( sub ) ); break;
                case STACKUP_TABLE_COLUMN_ID::THICKNESS:
                    addDataCell( layerThicknessString( *stackup_item, sub ), 'R' );
                    break;
                case STACKUP_TABLE_COLUMN_ID::COLOR: addDataCell( stackup_item->GetColor( sub ) ); break;
                case STACKUP_TABLE_COLUMN_ID::EPSILON_R:
                    addDataCell( formatDouble( stackup_item->GetEpsilonR( sub ) ), 'R' );
                    break;
                case STACKUP_TABLE_COLUMN_ID::LOSS_TANGENT:
                    addDataCell( formatDouble( stackup_item->GetLossTangent( sub ) ), 'R' );
                    break;
                }
            }
        }
    }

    Autosize();
    SetPosition( tablePos );

    if( !m_title.IsEmpty() )
    {
        m_titleItem = new PCB_TEXT( this );
        m_titleItem->SetText( m_title );
        m_titleItem->SetTextSize( m_headerTextSize );
        m_titleItem->SetTextThickness( m_headerTextThickness );
        m_titleItem->SetLayer( GetLayer() );
        m_titleItem->SetPosition( tablePos - VECTOR2I( 0, m_verticalMargin ) );

        if( aCommit )
            aCommit->Add( m_titleItem );
        else
            aBoard->Add( m_titleItem );
    }

    return true;
}

void PCB_STACKUP_TABLE::OnBoardCompositeUpdate( BOARD& aBoard, std::vector<BOARD_ITEM*>& aAdded,
                                                std::vector<BOARD_ITEM*>& aRemoved, std::vector<BOARD_ITEM*>& aChanged )
{
    auto checkList = [&]( const std::vector<BOARD_ITEM*>& lst )
    {
        for( BOARD_ITEM* it : lst )
        {
            if( !isOurItem( it ) )
                return true;
        }
        return false;
    };

    if( checkList( aAdded ) || checkList( aRemoved ) || checkList( aChanged ) )
        Update( &aBoard, nullptr );
}

void PCB_STACKUP_TABLE::OnBoardNetSettingsChanged( BOARD& aBoard )
{
    Update( &aBoard, nullptr );
}

EDA_ITEM* PCB_STACKUP_TABLE::Clone() const
{
    return new PCB_STACKUP_TABLE( *this );
}

void PCB_STACKUP_TABLE::SetParent( EDA_ITEM* aParent )
{
    if( BOARD* brd = GetBoard() )
        brd->RemoveListener( this );

    EDA_ITEM::SetParent( aParent );

    if( m_titleItem )
        m_titleItem->SetParent( this );

    if( BOARD* brd = GetBoard() )
        brd->AddListener( this );
}
