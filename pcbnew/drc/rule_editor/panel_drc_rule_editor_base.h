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
#include <wx/textctrl.h>
#include <wx/button.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/sizer.h>
#include <wx/statline.h>
#include <wx/panel.h>

///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
/// Class PANEL_DRC_RULE_EDITOR_BASE
///////////////////////////////////////////////////////////////////////////////
class PANEL_DRC_RULE_EDITOR_BASE : public wxPanel
{
	private:

	protected:
		wxStaticText* m_nameLabel;
		wxTextCtrl* m_nameCtrl;
		wxStaticText* m_commentLabel;
		wxTextCtrl* m_commentCtrl;
		wxButton* m_validateBtn;
		wxBoxSizer* m_constraintSizer;
		wxStaticText* m_constraintHeaderTitle;
		wxStaticLine* m_staticline3;
		wxBoxSizer* m_constraintContentSizer;
		wxStaticText* m_staticText71;
		wxStaticLine* m_staticline11;
		wxTextCtrl* m_textConditionCtrl;
		wxButton* m_button9;
		wxButton* m_button10;
		wxStaticText* m_staticText711;
		wxStaticLine* m_staticline111;
		wxBoxSizer* m_LayersComboBoxSizer;

	public:

		PANEL_DRC_RULE_EDITOR_BASE( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxTAB_TRAVERSAL, const wxString& name = wxEmptyString );

		~PANEL_DRC_RULE_EDITOR_BASE();

};

