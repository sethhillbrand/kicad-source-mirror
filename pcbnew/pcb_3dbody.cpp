#include "pcb_3dbody.h"

#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakePolygon.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepPrimAPI_MakePrism.hxx>
#include <BRepPrimAPI_MakeSphere.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Wire.hxx>
#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>
#include <STEPControl_Writer.hxx>
#include <IFSelect_ReturnStatus.hxx>
#include <Standard_Stream.hxx>

PCB_3DBODY::PCB_3DBODY( BOARD_ITEM* aParent ) :
        BOARD_ITEM( aParent, PCB_3DBODY_T, F_Cu ),
        m_type( PCB_3DBODY_TYPE::BOX ),
        m_sourceLayer( 0 ),
        m_height( 0.0 ),
        m_standoff( 0.0 ),
        m_radius( 0.0 ),
        m_length( 0.0 ),
        m_width( 0.0 )
{
}

double PCB_3DBODY::Similarity( const BOARD_ITEM& aItem ) const
{
    if( aItem.Type() != PCB_3DBODY_T )
        return 0.0;

    const PCB_3DBODY& other = static_cast<const PCB_3DBODY&>( aItem );

    return ( m_type == other.m_type &&
             m_sourceLayer == other.m_sourceLayer &&
             m_height == other.m_height &&
             m_standoff == other.m_standoff &&
             m_radius == other.m_radius &&
             m_length == other.m_length &&
             m_width == other.m_width ) ? 1.0 : 0.0;
}

bool PCB_3DBODY::operator==( const BOARD_ITEM& aItem ) const
{
    if( aItem.Type() != PCB_3DBODY_T )
        return false;

    const PCB_3DBODY& other = static_cast<const PCB_3DBODY&>( aItem );
    return m_type == other.m_type && m_sourceLayer == other.m_sourceLayer &&
           m_height == other.m_height && m_standoff == other.m_standoff &&
           m_radius == other.m_radius && m_length == other.m_length &&
           m_width == other.m_width;
}

bool PCB_3DBODY::GenerateStep()
{
    TopoDS_Shape shape;

    switch( m_type )
    {
    case PCB_3DBODY_TYPE::SPHERE:
        shape = BRepPrimAPI_MakeSphere( m_radius ).Shape();
        break;
    case PCB_3DBODY_TYPE::CYLINDER:
        shape = BRepPrimAPI_MakeCylinder( m_radius, m_height ).Shape();
        break;
    case PCB_3DBODY_TYPE::BOX:
        shape = BRepPrimAPI_MakeBox( m_length, m_width, m_height ).Shape();
        break;
    case PCB_3DBODY_TYPE::EXTRUSION:
    {
        BRepBuilderAPI_MakePolygon poly;
        poly.Add( gp_Pnt( 0, 0, 0 ) );
        poly.Add( gp_Pnt( 1, 0, 0 ) );
        poly.Add( gp_Pnt( 1, 1, 0 ) );
        poly.Add( gp_Pnt( 0, 1, 0 ) );
        poly.Close();
        TopoDS_Wire wire = poly.Wire();
        TopoDS_Face face = BRepBuilderAPI_MakeFace( wire );
        shape = BRepPrimAPI_MakePrism( face, gp_Vec( 0, 0, m_height ) );
        break;
    }
    }

    STEPControl_Writer writer;
    writer.Transfer( shape, STEPControl_AsIs );

    Standard_SStream stream;
    if( writer.WriteStream( stream ) != IFSelect_RetDone )
        return false;

    std::string data = stream.str();
    m_stepData.assign( data.begin(), data.end() );
    return true;
}
