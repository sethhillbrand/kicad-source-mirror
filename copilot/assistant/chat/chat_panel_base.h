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
#include <wx/richtext/richtextctrl.h>
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
#include <wx/panel.h>

///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
/// Class CHAT_PANEL_BASE
///////////////////////////////////////////////////////////////////////////////
class CHAT_PANEL_BASE : public wxPanel
{
	private:

	protected:
		wxRichTextCtrl* m_chat_ctrl;
		wxTextCtrl* m_usr_input;
		wxButton* m_btn_send;

		// Virtual event handlers, override them in your derived class
		virtual void m_chat_ctrlOnTextMaxLen( wxCommandEvent& event ) { event.Skip(); }
		virtual void m_chat_ctrlOnTextURL( wxTextUrlEvent& event ) { event.Skip(); }
		virtual void m_btn_sendOnButtonClick( wxCommandEvent& event ) { event.Skip(); }


	public:

		CHAT_PANEL_BASE( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxTAB_TRAVERSAL, const wxString& name = wxEmptyString );

		~CHAT_PANEL_BASE();

};

