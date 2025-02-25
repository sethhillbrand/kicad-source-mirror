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

	m_richText1 = new wxRichTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0|wxHSCROLL|wxVSCROLL );
	bSizer1->Add( m_richText1, 1, wxEXPAND | wxALL, 5 );

	wxBoxSizer* bSizer2;
	bSizer2 = new wxBoxSizer( wxHORIZONTAL );

	m_usr_input = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	bSizer2->Add( m_usr_input, 1, wxEXPAND, 0 );

	m_btn_send = new wxButton( this, wxID_ANY, _("<"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer2->Add( m_btn_send, 0, wxFIXED_MINSIZE, 0 );


	bSizer1->Add( bSizer2, 0, wxEXPAND, 0 );


	this->SetSizer( bSizer1 );
	this->Layout();
	bSizer1->Fit( this );

	// Connect Events
	m_usr_input->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( CHAT_PANEL_BASE::m_usr_inputOnText ), NULL, this );
	m_usr_input->Connect( wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler( CHAT_PANEL_BASE::m_usr_inputOnTextEnter ), NULL, this );
	m_usr_input->Connect( wxEVT_COMMAND_TEXT_MAXLEN, wxCommandEventHandler( CHAT_PANEL_BASE::m_usr_inputOnTextMaxLen ), NULL, this );
	m_usr_input->Connect( wxEVT_COMMAND_TEXT_URL, wxTextUrlEventHandler( CHAT_PANEL_BASE::m_usr_inputOnTextURL ), NULL, this );
	m_btn_send->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( CHAT_PANEL_BASE::m_btn_sendOnButtonClick ), NULL, this );
}

CHAT_PANEL_BASE::~CHAT_PANEL_BASE()
{
}
