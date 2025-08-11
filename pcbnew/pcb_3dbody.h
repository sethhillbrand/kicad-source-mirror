#ifndef PCB_3DBODY_H
#define PCB_3DBODY_H

#include <board_item.h>
#include <wx/string.h>
#include <vector>
#include <cstdint>

// Simple placeholder for parametrically generated 3D bodies attached to footprints.
// This is a minimal skeleton and does not implement full functionality.

enum class PCB_3DBODY_TYPE
{
    SPHERE,
    CYLINDER,
    BOX,
    EXTRUSION
};

class PCB_3DBODY : public BOARD_ITEM
{
public:
    PCB_3DBODY( BOARD_ITEM* aParent = nullptr );

    wxString GetClass() const override
    {
        return wxT( "PCB_3DBODY" );
    }

    PCB_3DBODY_TYPE GetType() const { return m_type; }
    void            SetType( PCB_3DBODY_TYPE aType ) { m_type = aType; }

    int  GetSourceLayer() const { return m_sourceLayer; }
    void SetSourceLayer( int aLayer ) { m_sourceLayer = aLayer; }

    double GetHeight() const { return m_height; }
    void   SetHeight( double aHeight ) { m_height = aHeight; }

    double GetStandoff() const { return m_standoff; }
    void   SetStandoff( double aStandoff ) { m_standoff = aStandoff; }

    // Additional parameters for primitive shapes
    double GetRadius() const { return m_radius; }
    void   SetRadius( double aRadius ) { m_radius = aRadius; }

    double GetLength() const { return m_length; }
    void   SetLength( double aLength ) { m_length = aLength; }

    double GetWidth() const { return m_width; }
    void   SetWidth( double aWidth ) { m_width = aWidth; }

    const std::vector<uint8_t>& GetStepData() const { return m_stepData; }
    bool                        GenerateStep();

    double Similarity( const BOARD_ITEM& aItem ) const override;
    bool   operator==( const BOARD_ITEM& aItem ) const override;

private:
    PCB_3DBODY_TYPE       m_type;
    int                   m_sourceLayer;
    double                m_height;
    double                m_standoff;
    double                m_radius;
    double                m_length;
    double                m_width;
    std::vector<uint8_t>  m_stepData;
};

#endif // PCB_3DBODY_H
