///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version 4.2.1-0-g80c4cb6a-dirty)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "panel_drc_group_header_base.h"

///////////////////////////////////////////////////////////////////////////

PANEL_DRC_GROUP_HEADER_BASE::PANEL_DRC_GROUP_HEADER_BASE( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name ) : wxPanel( parent, id, pos, size, style, name )
{
	wxBoxSizer* mainSizer;
	mainSizer = new wxBoxSizer( wxHORIZONTAL );

	m_dataGrid = new wxGrid( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );

	// Grid
	m_dataGrid->CreateGrid( 5, 5 );
	m_dataGrid->EnableEditing( true );
	m_dataGrid->EnableGridLines( true );
	m_dataGrid->EnableDragGridSize( false );
	m_dataGrid->SetMargins( 0, 0 );

	// Columns
	m_dataGrid->EnableDragColMove( false );
	m_dataGrid->EnableDragColSize( true );
	m_dataGrid->SetColLabelAlignment( wxALIGN_CENTER, wxALIGN_CENTER );

	// Rows
	m_dataGrid->EnableDragRowSize( true );
	m_dataGrid->SetRowLabelAlignment( wxALIGN_CENTER, wxALIGN_CENTER );

	// Label Appearance

	// Cell Defaults
	m_dataGrid->SetDefaultCellAlignment( wxALIGN_LEFT, wxALIGN_TOP );
	mainSizer->Add( m_dataGrid, 0, wxALL, 5 );


	this->SetSizer( mainSizer );
	this->Layout();
	mainSizer->Fit( this );
}

PANEL_DRC_GROUP_HEADER_BASE::~PANEL_DRC_GROUP_HEADER_BASE()
{
}
