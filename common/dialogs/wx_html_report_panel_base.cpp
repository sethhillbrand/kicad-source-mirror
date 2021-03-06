///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Dec 30 2017)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "wx_html_report_panel_base.h"

///////////////////////////////////////////////////////////////////////////

WX_HTML_REPORT_PANEL_BASE::WX_HTML_REPORT_PANEL_BASE( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	m_box = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("Output Messages") ), wxVERTICAL );
	
	m_box->SetMinSize( wxSize( -1,130 ) ); 
	m_fgSizer = new wxFlexGridSizer( 2, 1, 0, 0 );
	m_fgSizer->AddGrowableCol( 0 );
	m_fgSizer->AddGrowableRow( 0 );
	m_fgSizer->SetFlexibleDirection( wxBOTH );
	m_fgSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_htmlView = new wxHtmlWindow( m_box->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHW_SCROLLBAR_AUTO );
	m_htmlView->SetFont( wxFont( 10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString ) );
	
	m_fgSizer->Add( m_htmlView, 1, wxBOTTOM|wxEXPAND|wxLEFT|wxRIGHT, 2 );
	
	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* bMargins;
	bMargins = new wxBoxSizer( wxVERTICAL );
	
	m_staticText3 = new wxStaticText( m_box->GetStaticBox(), wxID_ANY, _("Show:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText3->Wrap( -1 );
	bMargins->Add( m_staticText3, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	
	bSizer1->Add( bMargins, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_checkBoxShowAll = new wxCheckBox( m_box->GetStaticBox(), wxID_ANY, _("All"), wxDefaultPosition, wxDefaultSize, 0 );
	m_checkBoxShowAll->SetValue(true); 
	bSizer1->Add( m_checkBoxShowAll, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	
	bSizer1->Add( 0, 0, 0, wxEXPAND|wxRIGHT|wxLEFT, 10 );
	
	m_checkBoxShowErrors = new wxCheckBox( m_box->GetStaticBox(), wxID_ANY, _("Errors"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer1->Add( m_checkBoxShowErrors, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM|wxLEFT, 5 );
	
	m_errorsBadge = new wxStaticBitmap( m_box->GetStaticBox(), wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, 0 );
	m_errorsBadge->SetMinSize( wxSize( 20,20 ) );
	
	bSizer1->Add( m_errorsBadge, 0, wxBOTTOM|wxRIGHT|wxTOP, 4 );
	
	m_checkBoxShowWarnings = new wxCheckBox( m_box->GetStaticBox(), wxID_ANY, _("Warnings"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer1->Add( m_checkBoxShowWarnings, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM|wxLEFT, 5 );
	
	m_warningsBadge = new wxStaticBitmap( m_box->GetStaticBox(), wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, 0 );
	m_warningsBadge->SetMinSize( wxSize( 20,20 ) );
	
	bSizer1->Add( m_warningsBadge, 0, wxBOTTOM|wxRIGHT|wxTOP, 4 );
	
	m_checkBoxShowActions = new wxCheckBox( m_box->GetStaticBox(), wxID_ANY, _("Actions"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer1->Add( m_checkBoxShowActions, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	
	bSizer1->Add( 0, 0, 0, wxEXPAND|wxRIGHT|wxLEFT, 10 );
	
	m_checkBoxShowInfos = new wxCheckBox( m_box->GetStaticBox(), wxID_ANY, _("Infos"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer1->Add( m_checkBoxShowInfos, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	
	bSizer1->Add( 0, 0, 1, wxEXPAND|wxRIGHT|wxLEFT, 10 );
	
	m_btnSaveReportToFile = new wxButton( m_box->GetStaticBox(), wxID_ANY, _("Save..."), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer1->Add( m_btnSaveReportToFile, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	
	m_fgSizer->Add( bSizer1, 1, wxEXPAND, 5 );
	
	
	m_box->Add( m_fgSizer, 1, wxEXPAND, 5 );
	
	
	this->SetSizer( m_box );
	this->Layout();
	m_box->Fit( this );
	
	// Connect Events
	m_htmlView->Connect( wxEVT_RIGHT_UP, wxMouseEventHandler( WX_HTML_REPORT_PANEL_BASE::onRightClick ), NULL, this );
	m_checkBoxShowAll->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( WX_HTML_REPORT_PANEL_BASE::onCheckBoxShowAll ), NULL, this );
	m_checkBoxShowErrors->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( WX_HTML_REPORT_PANEL_BASE::onCheckBoxShowErrors ), NULL, this );
	m_checkBoxShowWarnings->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( WX_HTML_REPORT_PANEL_BASE::onCheckBoxShowWarnings ), NULL, this );
	m_checkBoxShowActions->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( WX_HTML_REPORT_PANEL_BASE::onCheckBoxShowActions ), NULL, this );
	m_checkBoxShowInfos->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( WX_HTML_REPORT_PANEL_BASE::onCheckBoxShowInfos ), NULL, this );
	m_btnSaveReportToFile->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( WX_HTML_REPORT_PANEL_BASE::onBtnSaveToFile ), NULL, this );
}

WX_HTML_REPORT_PANEL_BASE::~WX_HTML_REPORT_PANEL_BASE()
{
	// Disconnect Events
	m_htmlView->Disconnect( wxEVT_RIGHT_UP, wxMouseEventHandler( WX_HTML_REPORT_PANEL_BASE::onRightClick ), NULL, this );
	m_checkBoxShowAll->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( WX_HTML_REPORT_PANEL_BASE::onCheckBoxShowAll ), NULL, this );
	m_checkBoxShowErrors->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( WX_HTML_REPORT_PANEL_BASE::onCheckBoxShowErrors ), NULL, this );
	m_checkBoxShowWarnings->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( WX_HTML_REPORT_PANEL_BASE::onCheckBoxShowWarnings ), NULL, this );
	m_checkBoxShowActions->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( WX_HTML_REPORT_PANEL_BASE::onCheckBoxShowActions ), NULL, this );
	m_checkBoxShowInfos->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( WX_HTML_REPORT_PANEL_BASE::onCheckBoxShowInfos ), NULL, this );
	m_btnSaveReportToFile->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( WX_HTML_REPORT_PANEL_BASE::onBtnSaveToFile ), NULL, this );
	
}
