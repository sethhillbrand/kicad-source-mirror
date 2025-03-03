#include "wx/wx.h"
#include <wx/app.h>
#include <wx/dialog.h>
#include <wx/frame.h>
#include <wx/window.h>

#include <wx/sysopt.h>
#include <assistant_interface.h>


class MyDialog : public wxFrame
{
public:
    MyDialog( const wxString& title );
    virtual ~MyDialog();
};

class MyApp : public wxApp
{
public:
    virtual bool OnInit() wxOVERRIDE;
};

wxIMPLEMENT_APP_CONSOLE( MyApp );

bool MyApp::OnInit()
{
    if( !wxApp::OnInit() )
        return false;

    // Create the main window
    auto gs_dialog = new MyDialog( "Standalone Copilot" );

    gs_dialog->Show();

    return true;
}


MyDialog::MyDialog( const wxString& title ) : wxFrame( NULL, wxID_ANY, title )
{
    wxSizer* const sizerTop = new wxBoxSizer( wxVERTICAL );
    sizerTop->Add( ASSISTANT_INTERFACE::get_instance().create_assistant_panel( this ),
                   wxSizerFlags( 1 ).Expand().Border() );
    SetSizerAndFit( sizerTop );
    SetSize( wxSize( 800, 600 ) );
}

MyDialog::~MyDialog()
{
}
