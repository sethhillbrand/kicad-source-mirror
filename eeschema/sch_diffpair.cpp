#include <sch_diffpair.h>
#include <sch_plot_opts.h>
#include <plotter.h>
#include <trigo.h>
#include <wx/intl.h>

SCH_DIFFPAIR::SCH_DIFFPAIR( const VECTOR2I& pos ) :
    SCH_ITEM( nullptr, SCH_DIFFPAIR_T ),
    m_pos( pos ),
    m_lineP( pos, LAYER_WIRE ),
    m_lineN( pos, LAYER_WIRE ),
    m_text( pos )
{
}

const BOX2I SCH_DIFFPAIR::GetBoundingBox() const
{
    BOX2I box = m_lineP.GetBoundingBox();
    box.Merge( m_lineN.GetBoundingBox() );
    box.Merge( m_text.GetBoundingBox() );
    return box;
}

std::vector<int> SCH_DIFFPAIR::ViewGetLayers() const
{
    return { m_lineP.GetLayer(), m_lineN.GetLayer(), m_text.GetLayer() };
}

void SCH_DIFFPAIR::Move( const VECTOR2I& aMoveVector )
{
    m_pos += aMoveVector;
    m_lineP.Move( aMoveVector );
    m_lineN.Move( aMoveVector );
    m_text.Move( aMoveVector );
}

void SCH_DIFFPAIR::MirrorHorizontally( int aCenter )
{
    m_lineP.MirrorHorizontally( aCenter );
    m_lineN.MirrorHorizontally( aCenter );
    m_text.MirrorHorizontally( aCenter );
    m_pos.x = 2 * aCenter - m_pos.x;
}

void SCH_DIFFPAIR::MirrorVertically( int aCenter )
{
    m_lineP.MirrorVertically( aCenter );
    m_lineN.MirrorVertically( aCenter );
    m_text.MirrorVertically( aCenter );
    m_pos.y = 2 * aCenter - m_pos.y;
}

void SCH_DIFFPAIR::Rotate( const VECTOR2I& aCenter, bool aRotateCCW )
{
    m_lineP.Rotate( aCenter, aRotateCCW );
    m_lineN.Rotate( aCenter, aRotateCCW );
    m_text.Rotate( aCenter, aRotateCCW );
    RotatePoint( &m_pos, aCenter, aRotateCCW );
}

bool SCH_DIFFPAIR::HitTest( const VECTOR2I& aPosition, int aAccuracy ) const
{
    return m_lineP.HitTest( aPosition, aAccuracy ) ||
           m_lineN.HitTest( aPosition, aAccuracy ) ||
           m_text.HitTest( aPosition, aAccuracy );
}

bool SCH_DIFFPAIR::HitTest( const BOX2I& aRect, bool aContained, int aAccuracy ) const
{
    return m_lineP.HitTest( aRect, aContained, aAccuracy ) ||
           m_lineN.HitTest( aRect, aContained, aAccuracy ) ||
           m_text.HitTest( aRect, aContained, aAccuracy );
}

void SCH_DIFFPAIR::Plot( PLOTTER* aPlotter, bool aBackground, const SCH_PLOT_OPTS& aPlotOpts,
                         int aUnit, int aBodyStyle, const VECTOR2I& aOffset, bool aDimmed )
{
    m_lineP.Plot( aPlotter, aBackground, aPlotOpts, aUnit, aBodyStyle, aOffset, aDimmed );
    m_lineN.Plot( aPlotter, aBackground, aPlotOpts, aUnit, aBodyStyle, aOffset, aDimmed );
    m_text.Plot( aPlotter, aBackground, aPlotOpts, aUnit, aBodyStyle, aOffset, aDimmed );
}

void SCH_DIFFPAIR::GetEndPoints( std::vector<DANGLING_END_ITEM>& aItemList )
{
    m_lineP.GetEndPoints( aItemList );
    m_lineN.GetEndPoints( aItemList );
}

std::vector<VECTOR2I> SCH_DIFFPAIR::GetConnectionPoints() const
{
    auto pts = m_lineP.GetConnectionPoints();
    auto pts2 = m_lineN.GetConnectionPoints();
    pts.insert( pts.end(), pts2.begin(), pts2.end() );
    return pts;
}

wxString SCH_DIFFPAIR::GetItemDescription( UNITS_PROVIDER*, bool ) const
{
    return _( "Differential Pair" );
}

void SCH_DIFFPAIR::swapData( SCH_ITEM* aItem )
{
    SCH_DIFFPAIR* other = dynamic_cast<SCH_DIFFPAIR*>( aItem );
    if( !other )
        return;

    std::swap( m_pos, other->m_pos );
    std::swap( m_lineP, other->m_lineP );
    std::swap( m_lineN, other->m_lineN );
    std::swap( m_text, other->m_text );
}

bool SCH_DIFFPAIR::doIsConnected( const VECTOR2I& aPosition ) const
{
    return m_lineP.IsEndPoint( aPosition ) || m_lineN.IsEndPoint( aPosition );
}
