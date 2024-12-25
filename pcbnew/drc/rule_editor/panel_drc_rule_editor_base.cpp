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

	bContentSizer = new wxBoxSizer( wxVERTICAL );

	wxBoxSizer* bBasicDetailSizer;
	bBasicDetailSizer = new wxBoxSizer( wxVERTICAL );

	wxFlexGridSizer* fgSizer2;
	fgSizer2 = new wxFlexGridSizer( 1, 5, 5, 0 );
	fgSizer2->SetFlexibleDirection( wxBOTH );
	fgSizer2->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	m_nameLabel = new wxStaticText( this, wxID_ANY, _("Name"), wxDefaultPosition, wxDefaultSize, 0 );
	m_nameLabel->Wrap( -1 );
	fgSizer2->Add( m_nameLabel, 0, wxALL, 5 );

	m_nameCtrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 300,-1 ), 0 );
	fgSizer2->Add( m_nameCtrl, 0, wxALL, 5 );

	m_commentLabel = new wxStaticText( this, wxID_ANY, _("Comment"), wxDefaultPosition, wxDefaultSize, 0 );
	m_commentLabel->Wrap( -1 );
	fgSizer2->Add( m_commentLabel, 0, wxALL, 5 );

	m_commentCtrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 300,-1 ), 0 );
	fgSizer2->Add( m_commentCtrl, 0, wxALL, 5 );

	m_validateBtn = new wxButton( this, wxID_ANY, _("Validate"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer2->Add( m_validateBtn, 0, wxALL, 5 );


	bBasicDetailSizer->Add( fgSizer2, 1, wxEXPAND|wxLEFT|wxTOP, 5 );


	bContentSizer->Add( bBasicDetailSizer, 0, 0, 20 );

	m_constraintSizer = new wxBoxSizer( wxVERTICAL );

	m_constraintHeaderTitle = new wxStaticText( this, wxID_ANY, _("Constraint"), wxDefaultPosition, wxDefaultSize, 0 );
	m_constraintHeaderTitle->Wrap( -1 );
	m_constraintSizer->Add( m_constraintHeaderTitle, 0, wxALL, 5 );

	m_staticline3 = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	m_constraintSizer->Add( m_staticline3, 0, wxEXPAND | wxALL, 5 );

	m_constraintContentSizer = new wxBoxSizer( wxVERTICAL );


	m_constraintSizer->Add( m_constraintContentSizer, 0, wxEXPAND, 5 );


	bContentSizer->Add( m_constraintSizer, 0, wxEXPAND|wxTOP, 15 );

	wxBoxSizer* bConditionSizer;
	bConditionSizer = new wxBoxSizer( wxVERTICAL );

	m_staticText71 = new wxStaticText( this, wxID_ANY, _("Conditions"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText71->Wrap( -1 );
	bConditionSizer->Add( m_staticText71, 0, wxEXPAND|wxLEFT|wxRIGHT|wxTOP, 5 );

	m_staticline11 = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	bConditionSizer->Add( m_staticline11, 0, wxEXPAND | wxALL, 5 );

	m_textConditionCtrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( -1,80 ), 0 );
	bConditionSizer->Add( m_textConditionCtrl, 0, wxALL|wxEXPAND, 5 );

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


	bConditionSizer->Add( bSizer17, 1, wxEXPAND, 5 );


	bContentSizer->Add( bConditionSizer, 0, wxEXPAND|wxTOP, 15 );

	wxBoxSizer* bLayerSizer;
	bLayerSizer = new wxBoxSizer( wxVERTICAL );

	m_staticText711 = new wxStaticText( this, wxID_ANY, _("Layer"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText711->Wrap( -1 );
	bLayerSizer->Add( m_staticText711, 0, wxALL|wxEXPAND|wxLEFT|wxRIGHT|wxTOP, 5 );

	m_staticline111 = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	bLayerSizer->Add( m_staticline111, 0, wxEXPAND | wxALL, 5 );

	m_LayersComboBoxSizer = new wxBoxSizer( wxVERTICAL );

	m_LayersComboBoxSizer->SetMinSize( wxSize( -1,70 ) );

	bLayerSizer->Add( m_LayersComboBoxSizer, 1, wxEXPAND, 5 );


	bContentSizer->Add( bLayerSizer, 0, wxEXPAND|wxTOP, 15 );

	m_staticline4 = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	bContentSizer->Add( m_staticline4, 0, wxEXPAND | wxALL, 5 );


	mainSizer->Add( bContentSizer, 1, 0, 5 );


	this->SetSizer( mainSizer );
	this->Layout();
	mainSizer->Fit( this );
}

PANEL_DRC_RULE_EDITOR_BASE::~PANEL_DRC_RULE_EDITOR_BASE()
{
}
