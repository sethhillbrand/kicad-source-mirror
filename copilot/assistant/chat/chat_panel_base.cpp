///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version 4.2.1-0-g80c4cb6)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "chat_panel_base.h"

///////////////////////////////////////////////////////////////////////////

CHAT_PANEL_BASE::CHAT_PANEL_BASE( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name ) : wxPanel( parent, id, pos, size, style, name )
{
	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer( wxVERTICAL );

	m_chat_ctrl = new wxRichTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_AUTO_URL|wxTE_READONLY|wxHSCROLL|wxVSCROLL );
	bSizer1->Add( m_chat_ctrl, 1, wxEXPAND | wxALL, 0 );

	wxBoxSizer* bSizer3;
	bSizer3 = new wxBoxSizer( wxHORIZONTAL );

	m_staticText1 = new wxStaticText( this, wxID_ANY, _("@"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText1->Wrap( -1 );
	bSizer3->Add( m_staticText1, 0, wxALL, 5 );

	m_cb_netlist = new wxCheckBox( this, wxID_ANY, _("Netlist"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer3->Add( m_cb_netlist, 0, wxALL, 5 );

	m_cb_bom = new wxCheckBox( this, wxID_ANY, _("BOM"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer3->Add( m_cb_bom, 0, wxALL, 5 );


	bSizer3->Add( 0, 0, 1, wxEXPAND, 5 );


	bSizer1->Add( bSizer3, 0, wxEXPAND, 0 );

	wxBoxSizer* bSizer2;
	bSizer2 = new wxBoxSizer( wxHORIZONTAL );

	m_usr_input = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER );
	bSizer2->Add( m_usr_input, 1, wxEXPAND, 0 );

	m_btn_send = new wxButton( this, wxID_ANY, _("<"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer2->Add( m_btn_send, 0, wxFIXED_MINSIZE, 0 );


	bSizer1->Add( bSizer2, 0, wxEXPAND, 0 );


	this->SetSizer( bSizer1 );
	this->Layout();
	bSizer1->Fit( this );

	// Connect Events
	m_chat_ctrl->Connect( wxEVT_COMMAND_TEXT_MAXLEN, wxCommandEventHandler( CHAT_PANEL_BASE::m_chat_ctrlOnTextMaxLen ), NULL, this );
	m_chat_ctrl->Connect( wxEVT_COMMAND_TEXT_URL, wxTextUrlEventHandler( CHAT_PANEL_BASE::m_chat_ctrlOnTextURL ), NULL, this );
	m_cb_netlist->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( CHAT_PANEL_BASE::m_cb_netlistOnCheckBox ), NULL, this );
	m_cb_bom->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( CHAT_PANEL_BASE::m_cb_bomOnCheckBox ), NULL, this );
	m_btn_send->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( CHAT_PANEL_BASE::m_btn_sendOnButtonClick ), NULL, this );
}

CHAT_PANEL_BASE::~CHAT_PANEL_BASE()
{
}
