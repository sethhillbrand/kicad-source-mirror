#include "drc_re_object_selector_panel.h"

#include <board.h>
#include <widgets/net_selector.h>
#include <widgets/netclass_selector.h>
#include <widgets/area_selector.h>

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/choice.h>
#include <wx/stc/stc.h>

DRC_RE_OBJECT_SELECTOR_PANEL::DRC_RE_OBJECT_SELECTOR_PANEL( wxWindow* parent, BOARD* board,
                                                            const wxString& label )
        : wxPanel( parent ), m_customQueryCtrl( nullptr )
{
    wxBoxSizer* mainSizer = new wxBoxSizer( wxVERTICAL );

    m_label = new wxStaticText( this, wxID_ANY, label );
    mainSizer->Add( m_label, 0, wxALL, 5 );

    m_rowSizer = new wxBoxSizer( wxHORIZONTAL );

    wxArrayString choices;
    choices.Add( _( "Any" ) );
    choices.Add( _( "Net" ) );
    choices.Add( _( "Netclass" ) );
    choices.Add( _( "Within Area" ) );
    choices.Add( _( "Custom Query" ) );

    m_choice = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, choices );
    m_choice->SetSelection( 0 );
    m_rowSizer->Add( m_choice, 0, wxALL, 5 );

    m_netSelector = new NET_SELECTOR( this, wxID_ANY );
    m_netSelector->SetNetInfo( &board->GetNetInfo() );
    m_rowSizer->Add( m_netSelector, 1, wxALL | wxEXPAND, 5 );
    m_netSelector->Hide();

    m_netclassSelector = new NETCLASS_SELECTOR( this, wxID_ANY );
    m_netclassSelector->SetBoard( board );
    m_rowSizer->Add( m_netclassSelector, 1, wxALL | wxEXPAND, 5 );
    m_netclassSelector->Hide();

    m_areaSelector = new AREA_SELECTOR( this, wxID_ANY );
    m_areaSelector->SetBoard( board );
    m_rowSizer->Add( m_areaSelector, 1, wxALL | wxEXPAND, 5 );
    m_areaSelector->Hide();

    mainSizer->Add( m_rowSizer, 0, wxEXPAND, 0 );

    SetSizer( mainSizer );

    m_choice->Bind( wxEVT_CHOICE, &DRC_RE_OBJECT_SELECTOR_PANEL::onChoice, this );
}

void DRC_RE_OBJECT_SELECTOR_PANEL::SetLabelText( const wxString& text )
{
    m_label->SetLabelText( text );
}

void DRC_RE_OBJECT_SELECTOR_PANEL::SetCustomQueryCtrl( wxStyledTextCtrl* ctrl )
{
    if( m_customQueryCtrl )
        m_customQueryCtrl->Reparent( this );

    m_customQueryCtrl = ctrl;

    if( m_customQueryCtrl )
    {
        m_customQueryCtrl->Reparent( this );
        GetSizer()->Add( m_customQueryCtrl, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 5 );
        m_customQueryCtrl->Hide();
    }
}

void DRC_RE_OBJECT_SELECTOR_PANEL::onChoice( wxCommandEvent& event )
{
    m_netSelector->Hide();
    m_netclassSelector->Hide();
    m_areaSelector->Hide();
    if( m_customQueryCtrl )
        m_customQueryCtrl->Hide();

    switch( m_choice->GetSelection() )
    {
    case 1: // Net
        m_netSelector->Show();
        break;
    case 2: // Netclass
        m_netclassSelector->Show();
        break;
    case 3: // Within Area
        m_areaSelector->Show();
        break;
    case 4: // Custom Query
        if( m_customQueryCtrl )
            m_customQueryCtrl->Show();
        break;
    default:
        break;
    }

    Layout();
}

