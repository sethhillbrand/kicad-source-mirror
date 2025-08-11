#ifndef _SCH_DIFFPAIR_H_
#define _SCH_DIFFPAIR_H_

#include <sch_item.h>
#include <sch_line.h>
#include <sch_text.h>

class SCH_DIFFPAIR : public SCH_ITEM
{
public:
    SCH_DIFFPAIR( const VECTOR2I& pos = VECTOR2I( 0, 0 ) );
    ~SCH_DIFFPAIR() { }

    static inline bool ClassOf( const EDA_ITEM* aItem )
    {
        return aItem && aItem->Type() == SCH_DIFFPAIR_T;
    }

    wxString GetClass() const override { return wxT( "SCH_DIFFPAIR" ); }

    EDA_ITEM* Clone() const override { return new SCH_DIFFPAIR( *this ); }

    VECTOR2I GetPosition() const override { return m_pos; }
    void SetPosition( const VECTOR2I& aPos ) override { m_pos = aPos; }

    const BOX2I GetBoundingBox() const override;

    std::vector<int> ViewGetLayers() const override;

    void Move( const VECTOR2I& aMoveVector ) override;
    void MirrorHorizontally( int aCenter ) override;
    void MirrorVertically( int aCenter ) override;
    void Rotate( const VECTOR2I& aCenter, bool aRotateCCW ) override;

    bool HitTest( const VECTOR2I& aPosition, int aAccuracy = 0 ) const override;
    bool HitTest( const BOX2I& aRect, bool aContained, int aAccuracy = 0 ) const override;

    void Plot( PLOTTER* aPlotter, bool aBackground, const SCH_PLOT_OPTS& aPlotOpts,
               int aUnit, int aBodyStyle, const VECTOR2I& aOffset, bool aDimmed ) override;

    void GetEndPoints( std::vector<DANGLING_END_ITEM>& aItemList ) override;

    std::vector<VECTOR2I> GetConnectionPoints() const override;

    bool IsConnectable() const override { return true; }
    bool CanConnect( const SCH_ITEM* aItem ) const override { return false; }

    wxString GetItemDescription( UNITS_PROVIDER* aUnitsProvider, bool aFull ) const override;

    SCH_LINE& LineP() { return m_lineP; }
    SCH_LINE& LineN() { return m_lineN; }
    SCH_TEXT& Text() { return m_text; }

protected:
    void swapData( SCH_ITEM* aItem ) override;

private:
    bool doIsConnected( const VECTOR2I& aPosition ) const override;

    VECTOR2I m_pos;
    SCH_LINE m_lineP;
    SCH_LINE m_lineN;
    SCH_TEXT m_text;
};

#endif
