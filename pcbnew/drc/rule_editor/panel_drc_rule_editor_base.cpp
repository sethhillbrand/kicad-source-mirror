///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version 4.2.1-0-g80c4cb6)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "panel_drc_rule_editor_base.h"

///////////////////////////////////////////////////////////////////////////

PANEL_DRC_RULE_EDITOR_BASE::PANEL_DRC_RULE_EDITOR_BASE( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name ) : wxPanel( parent, id, pos, size, style, name )
{
	wxBoxSizer* mainSizer;
	mainSizer = new wxBoxSizer( wxHORIZONTAL );

	wxBoxSizer* bSizer3;
	bSizer3 = new wxBoxSizer( wxVERTICAL );

	wxBoxSizer* bSizer4;
	bSizer4 = new wxBoxSizer( wxVERTICAL );

	wxFlexGridSizer* fgSizer2;
	fgSizer2 = new wxFlexGridSizer( 1, 5, 5, 0 );
	fgSizer2->SetFlexibleDirection( wxBOTH );
	fgSizer2->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	m_staticText3 = new wxStaticText( this, wxID_ANY, _("Name"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText3->Wrap( -1 );
	fgSizer2->Add( m_staticText3, 0, wxALL, 5 );

	m_textNameCtrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 300,-1 ), 0 );
	fgSizer2->Add( m_textNameCtrl, 0, wxALL, 5 );

	m_staticText6 = new wxStaticText( this, wxID_ANY, _("Comment"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText6->Wrap( -1 );
	fgSizer2->Add( m_staticText6, 0, wxALL, 5 );

	m_textCommentCtrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 300,-1 ), 0 );
	fgSizer2->Add( m_textCommentCtrl, 0, wxALL, 5 );

	m_button1 = new wxButton( this, wxID_ANY, _("Validate"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer2->Add( m_button1, 0, wxALL, 5 );


	bSizer4->Add( fgSizer2, 1, wxEXPAND|wxLEFT|wxTOP, 5 );


	bSizer3->Add( bSizer4, 0, 0, 20 );

	m_constraintSizer = new wxBoxSizer( wxVERTICAL );


	bSizer3->Add( m_constraintSizer, 0, wxEXPAND|wxTOP, 15 );

	wxBoxSizer* bSizer71;
	bSizer71 = new wxBoxSizer( wxVERTICAL );

	m_staticText71 = new wxStaticText( this, wxID_ANY, _("Conditions"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText71->Wrap( -1 );
	bSizer71->Add( m_staticText71, 0, wxEXPAND|wxLEFT|wxRIGHT|wxTOP, 5 );

	m_staticline11 = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	bSizer71->Add( m_staticline11, 0, wxEXPAND | wxALL, 5 );

	m_textConditionCtrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( -1,80 ), 0 );
	bSizer71->Add( m_textConditionCtrl, 0, wxALL|wxEXPAND, 5 );

	wxBoxSizer* bSizer17;
	bSizer17 = new wxBoxSizer( wxHORIZONTAL );

	wxBoxSizer* bSizer18;
	bSizer18 = new wxBoxSizer( wxVERTICAL );

	m_button9 = new wxButton( this, wxID_ANY, _("Show Matches"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer18->Add( m_button9, 0, wxALL, 5 );


	bSizer17->Add( bSizer18, 1, wxEXPAND, 5 );

	wxBoxSizer* bSizer19;
	bSizer19 = new wxBoxSizer( wxVERTICAL );


	bSizer19->Add( 0, 0, 1, wxEXPAND, 5 );

	m_button10 = new wxButton( this, wxID_ANY, _("Check Syntax"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer19->Add( m_button10, 0, wxALIGN_RIGHT|wxALL, 5 );


	bSizer17->Add( bSizer19, 1, wxEXPAND, 5 );


	bSizer71->Add( bSizer17, 1, wxEXPAND, 5 );


	bSizer3->Add( bSizer71, 0, wxEXPAND|wxTOP, 15 );

	wxBoxSizer* bSizer711;
	bSizer711 = new wxBoxSizer( wxVERTICAL );

	m_staticText711 = new wxStaticText( this, wxID_ANY, _("Layer"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText711->Wrap( -1 );
	bSizer711->Add( m_staticText711, 0, wxALL|wxEXPAND|wxLEFT|wxRIGHT|wxTOP, 5 );

	m_staticline111 = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	bSizer711->Add( m_staticline111, 0, wxEXPAND | wxALL, 5 );

	m_LayersComboBoxSizer = new wxBoxSizer( wxVERTICAL );


	bSizer711->Add( m_LayersComboBoxSizer, 1, wxEXPAND, 5 );


	bSizer3->Add( bSizer711, 0, wxEXPAND|wxTOP, 15 );


	mainSizer->Add( bSizer3, 1, 0, 5 );


	this->SetSizer( mainSizer );
	this->Layout();
	mainSizer->Fit( this );
}

PANEL_DRC_RULE_EDITOR_BASE::~PANEL_DRC_RULE_EDITOR_BASE()
{
}
