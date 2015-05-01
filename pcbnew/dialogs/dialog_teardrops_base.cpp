///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Jan 29 2014)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "dialog_teardrops_base.h"

///////////////////////////////////////////////////////////////////////////

DIALOG_TEARDROPS_BASE::DIALOG_TEARDROPS_BASE( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : DIALOG_SHIM( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxSize( -1,-1 ), wxDefaultSize );
	
	wxBoxSizer* bSizer7;
	bSizer7 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer13;
	bSizer13 = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* m_settingsSizer;
	m_settingsSizer = new wxBoxSizer( wxVERTICAL );
	
	wxStaticBoxSizer* m_modeSizer;
	m_modeSizer = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Mode") ), wxVERTICAL );
	
	m_modeAdd = new wxRadioButton( this, wxID_ANY, wxT("Add teardrops"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	m_modeSizer->Add( m_modeAdd, 0, 0, 5 );
	
	m_modeRemove = new wxRadioButton( this, wxID_ANY, wxT("Remove teardrops"), wxDefaultPosition, wxDefaultSize, 0 );
	m_modeSizer->Add( m_modeRemove, 0, 0, 5 );
	
	m_settingsSizer->Add( m_modeSizer, 1, wxEXPAND, 5 );
	
	wxStaticBoxSizer* m_tracksSizer;
	m_tracksSizer = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Tracks") ), wxVERTICAL );
	
	m_tracksAll = new wxRadioButton( this, wxID_ANY, wxT("All tracks"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	m_tracksSizer->Add( m_tracksAll, 0, 0, 5 );
	
	m_tracksSelected = new wxRadioButton( this, wxID_ANY, wxT("Selected tracks only"), wxDefaultPosition, wxDefaultSize, 0 );
	m_tracksSizer->Add( m_tracksSelected, 0, 0, 5 );
	
	m_settingsSizer->Add( m_tracksSizer, 1, wxEXPAND, 5 );
	
	wxStaticBoxSizer* m_optionsSizer;
	m_optionsSizer = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Options") ), wxVERTICAL );
	
	m_checkIgnore = new wxCheckBox( this, wxID_ANY, wxT("Ignore DRC"), wxDefaultPosition, wxDefaultSize, 0 );
	m_optionsSizer->Add( m_checkIgnore, 0, 0, 5 );
	
	m_checkClear = new wxCheckBox( this, wxID_ANY, wxT("Clear selection"), wxDefaultPosition, wxDefaultSize, 0 );
	m_checkClear->SetValue(true); 
	m_checkClear->Enable( false );
	
	m_optionsSizer->Add( m_checkClear, 0, 0, 5 );
	
	wxBoxSizer* m_styleSizer;
	m_styleSizer = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticStyle = new wxStaticText( this, wxID_ANY, wxT("Teardrop style"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticStyle->Wrap( -1 );
	m_styleSizer->Add( m_staticStyle, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT|wxALL, 5 );
	
	wxString m_choiceStyleChoices[] = { wxT("Straight"), wxT("Curved") };
	int m_choiceStyleNChoices = sizeof( m_choiceStyleChoices ) / sizeof( wxString );
	m_choiceStyle = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_choiceStyleNChoices, m_choiceStyleChoices, 0 );
	m_choiceStyle->SetSelection( 0 );
	m_styleSizer->Add( m_choiceStyle, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxTOP, 5 );
	
	m_optionsSizer->Add( m_styleSizer, 1, wxEXPAND, 5 );
	
	m_settingsSizer->Add( m_optionsSizer, 1, wxEXPAND, 5 );
	
	bSizer13->Add( m_settingsSizer, 1, wxEXPAND, 5 );
	
	wxBoxSizer* m_scopeSizer;
	m_scopeSizer = new wxBoxSizer( wxVERTICAL );
	
	wxStaticBoxSizer* sbSizer16;
	sbSizer16 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Scope") ), wxVERTICAL );
	
	m_scopeVias = new wxCheckBox( this, wxID_ANY, wxT("Vias"), wxDefaultPosition, wxDefaultSize, 0 );
	m_scopeVias->SetValue(true); 
	sbSizer16->Add( m_scopeVias, 0, 0, 5 );
	
	m_scopePads = new wxCheckBox( this, wxID_ANY, wxT("Circular pads"), wxDefaultPosition, wxDefaultSize, 0 );
	m_scopePads->SetValue(true); 
	sbSizer16->Add( m_scopePads, 0, 0, 5 );
	
	m_scopeTracks = new wxCheckBox( this, wxID_ANY, wxT("Tracks"), wxDefaultPosition, wxDefaultSize, 0 );
	m_scopeTracks->Enable( false );
	m_scopeTracks->Hide();
	
	sbSizer16->Add( m_scopeTracks, 0, 0, 5 );
	
	m_scopeSizer->Add( sbSizer16, 1, wxEXPAND, 5 );
	
	bSizer13->Add( m_scopeSizer, 1, wxEXPAND, 5 );
	
	bSizer7->Add( bSizer13, 1, wxEXPAND, 5 );
	
	m_staticline1 = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	bSizer7->Add( m_staticline1, 0, wxEXPAND | wxALL, 5 );
	
	m_sdbSizer = new wxStdDialogButtonSizer();
	m_sdbSizerOK = new wxButton( this, wxID_OK );
	m_sdbSizer->AddButton( m_sdbSizerOK );
	m_sdbSizerCancel = new wxButton( this, wxID_CANCEL );
	m_sdbSizer->AddButton( m_sdbSizerCancel );
	m_sdbSizer->Realize();
	bSizer7->Add( m_sdbSizer, 0, wxEXPAND, 5 );
	
	this->SetSizer( bSizer7 );
	this->Layout();
	
	this->Centre( wxBOTH );
	
	// Connect Events
	m_modeAdd->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( DIALOG_TEARDROPS_BASE::OnModeAdd ), NULL, this );
	m_modeRemove->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( DIALOG_TEARDROPS_BASE::OnModeRemove ), NULL, this );
	m_tracksAll->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( DIALOG_TEARDROPS_BASE::OnTracksAll ), NULL, this );
	m_tracksSelected->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( DIALOG_TEARDROPS_BASE::OnTracksSelected ), NULL, this );
	m_checkIgnore->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( DIALOG_TEARDROPS_BASE::OnIgnoreDrc ), NULL, this );
	m_checkClear->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( DIALOG_TEARDROPS_BASE::OnClearSelection ), NULL, this );
	m_choiceStyle->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( DIALOG_TEARDROPS_BASE::OnStyleChanged ), NULL, this );
	m_scopeVias->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( DIALOG_TEARDROPS_BASE::OnScopeVias ), NULL, this );
	m_scopePads->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( DIALOG_TEARDROPS_BASE::OnScopePads ), NULL, this );
}

DIALOG_TEARDROPS_BASE::~DIALOG_TEARDROPS_BASE()
{
	// Disconnect Events
	m_modeAdd->Disconnect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( DIALOG_TEARDROPS_BASE::OnModeAdd ), NULL, this );
	m_modeRemove->Disconnect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( DIALOG_TEARDROPS_BASE::OnModeRemove ), NULL, this );
	m_tracksAll->Disconnect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( DIALOG_TEARDROPS_BASE::OnTracksAll ), NULL, this );
	m_tracksSelected->Disconnect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( DIALOG_TEARDROPS_BASE::OnTracksSelected ), NULL, this );
	m_checkIgnore->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( DIALOG_TEARDROPS_BASE::OnIgnoreDrc ), NULL, this );
	m_checkClear->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( DIALOG_TEARDROPS_BASE::OnClearSelection ), NULL, this );
	m_choiceStyle->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( DIALOG_TEARDROPS_BASE::OnStyleChanged ), NULL, this );
	m_scopeVias->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( DIALOG_TEARDROPS_BASE::OnScopeVias ), NULL, this );
	m_scopePads->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( DIALOG_TEARDROPS_BASE::OnScopePads ), NULL, this );
}
