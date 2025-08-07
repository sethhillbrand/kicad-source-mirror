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

#ifndef PCB_STACKUP_TABLE_H
#define PCB_STACKUP_TABLE_H

#include <pcb_table.h>
#include <board.h>
#include <vector>

class PCB_TEXT;

enum class STACKUP_UNIT_MODE
{
    AUTO,
    MM,
    INCH
};

enum class STACKUP_TABLE_COLUMN_ID
{
    LAYER_NAME,
    TYPE,
    MATERIAL,
    THICKNESS,
    COLOR,
    EPSILON_R,
    LOSS_TANGENT
};

struct STACKUP_COLUMN_DEF
{
    STACKUP_TABLE_COLUMN_ID id;
    wxString                header;
    bool                    visible;

    STACKUP_COLUMN_DEF( STACKUP_TABLE_COLUMN_ID aId, const wxString& aHeader, bool aVisible = true ) :
            id( aId ),
            header( aHeader ),
            visible( aVisible )
    {
    }
};

class PCB_STACKUP_TABLE : public PCB_TABLE, public BOARD_LISTENER
{
public:
    PCB_STACKUP_TABLE( BOARD_ITEM* aParent = nullptr, PCB_LAYER_ID aLayer = Dwgs_User );
    PCB_STACKUP_TABLE( const PCB_STACKUP_TABLE& aOther );
    ~PCB_STACKUP_TABLE() override;

    bool Update( BOARD* aBoard, BOARD_COMMIT* aCommit );

    EDA_ITEM* Clone() const override;
    void      SetParent( EDA_ITEM* aParent ) override;

    void Move( const VECTOR2I& aMoveVector ) override;
    void Rotate( const VECTOR2I& aRotCentre, const EDA_ANGLE& aAngle ) override;
    void Flip( const VECTOR2I& aCentre, FLIP_DIRECTION aFlipDirection ) override;
    void Mirror( const VECTOR2I& aCentre, FLIP_DIRECTION aFlipDirection ) override;

    void OnBoardCompositeUpdate( BOARD& aBoard, std::vector<BOARD_ITEM*>& aAdded, std::vector<BOARD_ITEM*>& aRemoved,
                                 std::vector<BOARD_ITEM*>& aChanged ) override;
    void OnBoardNetSettingsChanged( BOARD& aBoard ) override;

    wxString GetClass() const override { return wxT( "PCB_STACKUP_TABLE" ); }

    void              SetUnitsMode( STACKUP_UNIT_MODE aMode ) { m_unitsMode = aMode; }
    STACKUP_UNIT_MODE GetUnitsMode() const { return m_unitsMode; }

    void SetNumericPrecision( int aPrecision ) { m_precision = aPrecision; }
    int  GetNumericPrecision() const { return m_precision; }

    void SetColumnVisible( STACKUP_TABLE_COLUMN_ID aColumn, bool aVisible );
    bool IsColumnVisible( STACKUP_TABLE_COLUMN_ID aColumn ) const;
    void SetColumnHeader( STACKUP_TABLE_COLUMN_ID aColumn, const wxString& aHeader );

    STACKUP_TABLE_COLUMN_ID GetVisibleColumnId( int aVisibleIndex ) const;
    void                    ShowAllColumns();
    bool                    HasTitle() const { return !m_title.IsEmpty(); }

    void SetHeaderTextStyle( const VECTOR2I& aSize, int aThickness );
    void SetBodyTextStyle( const VECTOR2I& aSize, int aThickness );

    void            SetTitle( const wxString& aTitle ) { m_title = aTitle; }
    const wxString& GetTitle() const { return m_title; }

    void SetVerticalMargin( int aMargin ) { m_verticalMargin = aMargin; }
    int  GetVerticalMargin() const { return m_verticalMargin; }

private:
    bool isOurItem( BOARD_ITEM* aItem ) const;
    bool m_updating = false;

    STACKUP_UNIT_MODE               m_unitsMode;
    int                             m_precision;
    std::vector<STACKUP_COLUMN_DEF> m_columns;
    VECTOR2I                        m_headerTextSize;
    int                             m_headerTextThickness;
    VECTOR2I                        m_bodyTextSize;
    int                             m_bodyTextThickness;
    wxString                        m_title;
    int                             m_verticalMargin;
    PCB_TEXT*                       m_titleItem;
};

#endif
