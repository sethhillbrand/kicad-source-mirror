///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version 4.2.1-0-g80c4cb6)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "drc_re_smd_entry_panel_base.h"

///////////////////////////////////////////////////////////////////////////

DRC_RE_SMD_ENTRY_PANEL_BASE::DRC_RE_SMD_ENTRY_PANEL_BASE( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name ) : wxPanel( parent, id, pos, size, style, name )
{
	wxBoxSizer* mainSizer;
	mainSizer = new wxBoxSizer( wxVERTICAL );

	wxBoxSizer* bConstraintImageAndValueSizer;
	bConstraintImageAndValueSizer = new wxBoxSizer( wxHORIZONTAL );

	bConstraintImageSizer = new wxBoxSizer( wxVERTICAL );


	bConstraintImageAndValueSizer->Add( bConstraintImageSizer, 1, wxBOTTOM|wxEXPAND|wxLEFT, 5 );

	wxBoxSizer* bConstraintContentSizer;
	bConstraintContentSizer = new wxBoxSizer( wxVERTICAL );


	bConstraintContentSizer->Add( 0, 0, 1, wxEXPAND, 5 );

	wxFlexGridSizer* fgSizer3;
	fgSizer3 = new wxFlexGridSizer( 2, 1, 0, 0 );
	fgSizer3->SetFlexibleDirection( wxBOTH );
	fgSizer3->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	m_chkSide = new wxCheckBox( this, wxID_ANY, _("Side"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer3->Add( m_chkSide, 0, wxALL, 5 );

	m_chkCorner = new wxCheckBox( this, wxID_ANY, _("Corner"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer3->Add( m_chkCorner, 0, wxALL, 5 );

	m_chkAnyAngle = new wxCheckBox( this, wxID_ANY, _("Any Angle"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer3->Add( m_chkAnyAngle, 0, wxALL, 5 );


	bConstraintContentSizer->Add( fgSizer3, 0, wxEXPAND, 5 );


	bConstraintContentSizer->Add( 0, 0, 1, wxEXPAND, 5 );


	bConstraintImageAndValueSizer->Add( bConstraintContentSizer, 1, wxBOTTOM|wxEXPAND|wxLEFT|wxRIGHT, 5 );


	mainSizer->Add( bConstraintImageAndValueSizer, 0, wxEXPAND, 5 );


	this->SetSizer( mainSizer );
	this->Layout();
}

DRC_RE_SMD_ENTRY_PANEL_BASE::~DRC_RE_SMD_ENTRY_PANEL_BASE()
{
}
