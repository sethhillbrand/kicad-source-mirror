#include "panel_pcb_3dbody_generator.h"

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/filename.h>
#include <wx/filefn.h>
#include <fstream>

#include <pcb_base_frame.h>
#include <pcb_base_edit_frame.h>
#include <footprint.h>

PANEL_PCB_3DBODY_GENERATOR::PANEL_PCB_3DBODY_GENERATOR( PCB_BASE_EDIT_FRAME* aFrame, FOOTPRINT* aFootprint,
                                                        wxWindow* aParent, wxWindowID id )
    : wxPanel( aParent, id ), m_preview( nullptr )
{
    wxBoxSizer* topSizer = new wxBoxSizer( wxVERTICAL );

    wxBoxSizer* paramSizer = new wxBoxSizer( wxHORIZONTAL );

    m_shapeChoice = new wxChoice( this, wxID_ANY );
    m_shapeChoice->Append( _( "Sphere" ) );
    m_shapeChoice->Append( _( "Cylinder" ) );
    m_shapeChoice->Append( _( "Box" ) );
    m_shapeChoice->SetSelection( 0 );
    paramSizer->Add( m_shapeChoice, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5 );

    auto makeParam = [&]( const wxString& label, wxTextCtrl** txt, wxSpinButton** spin )
    {
        wxBoxSizer* s = new wxBoxSizer( wxHORIZONTAL );
        s->Add( new wxStaticText( this, wxID_ANY, label ), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5 );
        *txt = new wxTextCtrl( this, wxID_ANY, wxT( "1" ), wxDefaultPosition, wxSize( 60, -1 ) );
        *spin = new wxSpinButton( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_VERTICAL );
        ( *spin )->SetRange( 1, 1000 );
        s->Add( *txt, 0, wxRIGHT, 5 );
        s->Add( *spin, 0 );
        paramSizer->Add( s, 0, wxALL, 5 );
    };

    makeParam( _( "Radius" ), &m_radiusCtrl, &m_spinRadius );
    makeParam( _( "Length" ), &m_lengthCtrl, &m_spinLength );
    makeParam( _( "Width" ), &m_widthCtrl, &m_spinWidth );
    makeParam( _( "Height" ), &m_heightCtrl, &m_spinHeight );

    topSizer->Add( paramSizer, 0, wxEXPAND );

    m_models.emplace_back();
    m_preview = new PANEL_PREVIEW_3D_MODEL( this, aFrame, aFootprint, &m_models );
    topSizer->Add( m_preview, 1, wxEXPAND | wxALL, 5 );

    SetSizer( topSizer );

    m_shapeChoice->Bind( wxEVT_CHOICE, &PANEL_PCB_3DBODY_GENERATOR::onShapeChanged, this );
    m_radiusCtrl->Bind( wxEVT_TEXT, &PANEL_PCB_3DBODY_GENERATOR::onParamText, this );
    m_lengthCtrl->Bind( wxEVT_TEXT, &PANEL_PCB_3DBODY_GENERATOR::onParamText, this );
    m_widthCtrl->Bind( wxEVT_TEXT, &PANEL_PCB_3DBODY_GENERATOR::onParamText, this );
    m_heightCtrl->Bind( wxEVT_TEXT, &PANEL_PCB_3DBODY_GENERATOR::onParamText, this );

    m_spinRadius->Bind( wxEVT_SPIN_UP, &PANEL_PCB_3DBODY_GENERATOR::onRadiusSpinUp, this );
    m_spinRadius->Bind( wxEVT_SPIN_DOWN, &PANEL_PCB_3DBODY_GENERATOR::onRadiusSpinDown, this );
    m_spinLength->Bind( wxEVT_SPIN_UP, &PANEL_PCB_3DBODY_GENERATOR::onLengthSpinUp, this );
    m_spinLength->Bind( wxEVT_SPIN_DOWN, &PANEL_PCB_3DBODY_GENERATOR::onLengthSpinDown, this );
    m_spinWidth->Bind( wxEVT_SPIN_UP, &PANEL_PCB_3DBODY_GENERATOR::onWidthSpinUp, this );
    m_spinWidth->Bind( wxEVT_SPIN_DOWN, &PANEL_PCB_3DBODY_GENERATOR::onWidthSpinDown, this );
    m_spinHeight->Bind( wxEVT_SPIN_UP, &PANEL_PCB_3DBODY_GENERATOR::onHeightSpinUp, this );
    m_spinHeight->Bind( wxEVT_SPIN_DOWN, &PANEL_PCB_3DBODY_GENERATOR::onHeightSpinDown, this );

    updateVisibility();
    updateModel();
}

void PANEL_PCB_3DBODY_GENERATOR::onShapeChanged( wxCommandEvent& )
{
    updateVisibility();
    updateModel();
}

void PANEL_PCB_3DBODY_GENERATOR::onParamText( wxCommandEvent& )
{
    updateModel();
}

static void adjustCtrl( wxTextCtrl* ctrl, double step )
{
    double v = 0.0;
    ctrl->GetValue().ToDouble( &v );
    v += step;
    ctrl->SetValue( wxString::Format( wxT( "%.2f" ), v ) );
}

void PANEL_PCB_3DBODY_GENERATOR::onRadiusSpinUp( wxSpinEvent& )
{
    adjustCtrl( m_radiusCtrl, 1.0 );
    updateModel();
}

void PANEL_PCB_3DBODY_GENERATOR::onRadiusSpinDown( wxSpinEvent& )
{
    adjustCtrl( m_radiusCtrl, -1.0 );
    updateModel();
}

void PANEL_PCB_3DBODY_GENERATOR::onLengthSpinUp( wxSpinEvent& )
{
    adjustCtrl( m_lengthCtrl, 1.0 );
    updateModel();
}

void PANEL_PCB_3DBODY_GENERATOR::onLengthSpinDown( wxSpinEvent& )
{
    adjustCtrl( m_lengthCtrl, -1.0 );
    updateModel();
}

void PANEL_PCB_3DBODY_GENERATOR::onWidthSpinUp( wxSpinEvent& )
{
    adjustCtrl( m_widthCtrl, 1.0 );
    updateModel();
}

void PANEL_PCB_3DBODY_GENERATOR::onWidthSpinDown( wxSpinEvent& )
{
    adjustCtrl( m_widthCtrl, -1.0 );
    updateModel();
}

void PANEL_PCB_3DBODY_GENERATOR::onHeightSpinUp( wxSpinEvent& )
{
    adjustCtrl( m_heightCtrl, 1.0 );
    updateModel();
}

void PANEL_PCB_3DBODY_GENERATOR::onHeightSpinDown( wxSpinEvent& )
{
    adjustCtrl( m_heightCtrl, -1.0 );
    updateModel();
}

void PANEL_PCB_3DBODY_GENERATOR::updateVisibility()
{
    int sel = m_shapeChoice->GetSelection();
    bool showRadius = sel == 0 || sel == 1;      // sphere or cylinder
    bool showLengthWidth = sel == 2;             // box
    bool showHeight = sel == 1 || sel == 2;      // cylinder or box

    m_radiusCtrl->Show( showRadius );
    m_spinRadius->Show( showRadius );
    m_lengthCtrl->Show( showLengthWidth );
    m_spinLength->Show( showLengthWidth );
    m_widthCtrl->Show( showLengthWidth );
    m_spinWidth->Show( showLengthWidth );
    m_heightCtrl->Show( showHeight );
    m_spinHeight->Show( showHeight );

    Layout();
}

void PANEL_PCB_3DBODY_GENERATOR::updateModel()
{
    double radius = 0.0, length = 0.0, width = 0.0, height = 0.0;
    m_radiusCtrl->GetValue().ToDouble( &radius );
    m_lengthCtrl->GetValue().ToDouble( &length );
    m_widthCtrl->GetValue().ToDouble( &width );
    m_heightCtrl->GetValue().ToDouble( &height );

    radius = pcbIUScale.mmToIU( radius );
    length = pcbIUScale.mmToIU( length );
    width = pcbIUScale.mmToIU( width );
    height = pcbIUScale.mmToIU( height );

    m_body.SetRadius( radius );
    m_body.SetLength( length );
    m_body.SetWidth( width );
    m_body.SetHeight( height );

    switch( m_shapeChoice->GetSelection() )
    {
    case 0: m_body.SetType( PCB_3DBODY_TYPE::SPHERE ); break;
    case 1: m_body.SetType( PCB_3DBODY_TYPE::CYLINDER ); break;
    case 2: m_body.SetType( PCB_3DBODY_TYPE::BOX ); break;
    default: break;
    }

    if( !m_body.GenerateStep() )
        return;

    const std::vector<uint8_t>& data = m_body.GetStepData();
    wxFileName tmp = wxFileName::CreateTempFileName( wxT( "3dbody" ) );
    tmp.SetExt( wxT( "step" ) );
    std::ofstream out( tmp.GetFullPath().ToStdString(), std::ios::binary );
    out.write( reinterpret_cast<const char*>( data.data() ), data.size() );
    out.close();
    m_tempFile = tmp.GetFullPath();

    m_models[0].m_Filename = m_tempFile;
    m_models[0].m_Show = true;
    m_models[0].m_IsParametric = true;

    m_preview->UpdateDummyFootprint( true );
}
