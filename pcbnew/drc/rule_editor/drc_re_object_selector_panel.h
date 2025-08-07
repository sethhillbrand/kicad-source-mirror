#pragma once

#include <wx/panel.h>

class BOARD;
class NET_SELECTOR;
class NETCLASS_SELECTOR;
class AREA_SELECTOR;

class wxStaticText;
class wxChoice;
class wxBoxSizer;
class wxStyledTextCtrl;

/**
 * Panel used in the rule editor conditions to select object filters.
 */
class DRC_RE_OBJECT_SELECTOR_PANEL : public wxPanel
{
public:
    DRC_RE_OBJECT_SELECTOR_PANEL( wxWindow* parent, BOARD* board, const wxString& label );

    void SetLabelText( const wxString& text );
    void SetCustomQueryCtrl( wxStyledTextCtrl* ctrl );

private:
    void onChoice( wxCommandEvent& event );

    wxStaticText*      m_label;
    wxChoice*          m_choice;
    NET_SELECTOR*      m_netSelector;
    NETCLASS_SELECTOR* m_netclassSelector;
    AREA_SELECTOR*     m_areaSelector;
    wxStyledTextCtrl*  m_customQueryCtrl;
    wxBoxSizer*        m_rowSizer;
};

