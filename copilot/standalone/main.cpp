#include "wx/wx.h"

#include <wx/sysopt.h>
#include "standalone.h"
#include <assistant/chat/chat_panel.h>
#include "dylib.hpp"


static const auto do_init = []() {
    wxSystemOptions::SetOption("msw.no-manifest-check",1);
    return 0;
}();

// ----------------------------------------------------------------------------
// global variables
// ----------------------------------------------------------------------------

static MyDialog* gs_dialog = NULL;

// ============================================================================
// implementation
// ============================================================================

// ----------------------------------------------------------------------------
// MyApp
// ----------------------------------------------------------------------------

wxIMPLEMENT_APP( MyApp );

bool MyApp::OnInit()
{

    if( !wxApp::OnInit() )
        return false;

    // Create the main window
    gs_dialog = new MyDialog( "wxTaskBarIcon Test Dialog" );

    gs_dialog->Show( true );

    return true;
}


// ----------------------------------------------------------------------------
// MyDialog implementation
// ----------------------------------------------------------------------------

wxBEGIN_EVENT_TABLE( MyDialog, wxDialog ) EVT_BUTTON( wxID_ABOUT, MyDialog::OnAbout )
        EVT_BUTTON( wxID_OK, MyDialog::OnOK ) EVT_BUTTON( wxID_EXIT, MyDialog::OnExit )
                EVT_CLOSE( MyDialog::OnCloseWindow ) wxEND_EVENT_TABLE()


                        MyDialog::MyDialog( const wxString& title ) :
        wxDialog( NULL, wxID_ANY, title )
{
    wxSizer* const sizerTop = new wxBoxSizer( wxVERTICAL );

    wxSizerFlags flags;
    flags.Border( wxALL, 10 );

    sizerTop->Add( new wxStaticText( this, wxID_ANY,
                                     "Press 'Hide me' to hide this window, Exit to quit." ),
                   flags );

    sizerTop->Add( new wxStaticText( this, wxID_ANY,
                                     "Double-click on the taskbar icon to show me again." ),
                   flags );

    sizerTop->AddStretchSpacer()->SetMinSize( 200, 50 );

    wxSizer* const sizerBtns = new wxBoxSizer( wxHORIZONTAL );
    sizerBtns->Add( new wxButton( this, wxID_ABOUT, "&About" ), flags );
    sizerBtns->Add( new wxButton( this, wxID_OK, "&Hide" ), flags );
    sizerBtns->Add( new wxButton( this, wxID_EXIT, "E&xit" ), flags );

    sizerTop->Add( sizerBtns, flags.Align( wxALIGN_CENTER_HORIZONTAL ) );
    SetSizerAndFit( sizerTop );
    Centre();
}

MyDialog::~MyDialog()
{
}

void MyDialog::OnAbout( wxCommandEvent& WXUNUSED( event ) )
{
    static const char* const title = "About wxWidgets Taskbar Sample";
    static const char* const message = "wxWidgets sample showing wxTaskBarIcon class\n"
                                       "\n"
                                       "(C) 1997 Julian Smart\n"
                                       "(C) 2007 Vadim Zeitlin";
    wxMessageBox( message, title, wxICON_INFORMATION | wxOK, this );
}

void MyDialog::OnOK( wxCommandEvent& WXUNUSED( event ) )
{
    Show( false );
}

void MyDialog::OnExit( wxCommandEvent& WXUNUSED( event ) )
{
    Close( true );
}

void MyDialog::OnCloseWindow( wxCloseEvent& WXUNUSED( event ) )
{
    Destroy();
}


// ----------------------------------------------------------------------------
// MyTaskBarIcon implementation
// ----------------------------------------------------------------------------

enum
{
    PU_RESTORE = 10001,
    PU_NEW_ICON,
    PU_EXIT,
    PU_CHECKMARK,
    PU_SUB1,
    PU_SUB2,
    PU_SUBMAIN
};
