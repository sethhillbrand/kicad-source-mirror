///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version 4.2.1-0-g80c4cb6)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/intl.h>
#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/statline.h>
#include <wx/sizer.h>
#include <wx/textctrl.h>
#include <wx/panel.h>

///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
/// Class DRC_RE_BASIC_CLEARANCE_PANEL_BASE
///////////////////////////////////////////////////////////////////////////////
class DRC_RE_BASIC_CLEARANCE_PANEL_BASE : public wxPanel
{
	private:

	protected:
		wxStaticText* m_constraintHeaderTitle;
		wxStaticLine* m_staticline2;
		wxBoxSizer* bConstraintImageSizer;
		wxStaticText* m_staticText4;
		wxTextCtrl* m_textBasicClearance;
		wxStaticText* m_staticText5;

	public:

		DRC_RE_BASIC_CLEARANCE_PANEL_BASE( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 500,300 ), long style = wxTAB_TRAVERSAL, const wxString& name = wxEmptyString );

		~DRC_RE_BASIC_CLEARANCE_PANEL_BASE();

};

