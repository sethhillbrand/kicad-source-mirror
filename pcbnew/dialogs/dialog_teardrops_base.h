///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Jan 29 2014)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#ifndef __dialog_teardrops_base__
#define __dialog_teardrops_base__

class DIALOG_SHIM;

#include "dialog_shim.h"
#include <wx/string.h>
#include <wx/radiobut.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/checkbox.h>
#include <wx/stattext.h>
#include <wx/choice.h>
#include <wx/statline.h>
#include <wx/button.h>
#include <wx/dialog.h>

///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
/// Class DIALOG_TEARDROPS_BASE
///////////////////////////////////////////////////////////////////////////////
class DIALOG_TEARDROPS_BASE : public DIALOG_SHIM
{
	private:
	
	protected:
		wxRadioButton* m_modeAdd;
		wxRadioButton* m_modeRemove;
		wxRadioButton* m_tracksAll;
		wxRadioButton* m_tracksSelected;
		wxCheckBox* m_checkIgnore;
		wxCheckBox* m_checkClear;
		wxStaticText* m_staticStyle;
		wxChoice* m_choiceStyle;
		wxCheckBox* m_scopeVias;
		wxCheckBox* m_scopePads;
		wxCheckBox* m_scopeTracks;
		wxStaticLine* m_staticline1;
		wxStdDialogButtonSizer* m_sdbSizer;
		wxButton* m_sdbSizerOK;
		wxButton* m_sdbSizerCancel;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnModeAdd( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnModeRemove( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnTracksAll( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnTracksSelected( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnIgnoreDrc( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnClearSelection( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnStyleChanged( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnScopeVias( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnScopePads( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		
		DIALOG_TEARDROPS_BASE( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("Teardrops"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 615,329 ), long style = wxDEFAULT_DIALOG_STYLE );
		virtual ~DIALOG_TEARDROPS_BASE();
	
};

#endif //__dialog_teardrops_base__
