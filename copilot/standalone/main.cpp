#include "wx/wx.h"

#include <wx/sysopt.h>
#include "standalone.h"
#include <assistant_interface.h>

static const auto do_init = []()
{
    wxSystemOptions::SetOption( "msw.no-manifest-check", 1 );
    return 0;
}();

// ----------------------------------------------------------------------------
// global variables
// ----------------------------------------------------------------------------


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
    auto gs_dialog = new MyDialog( "wxTaskBarIcon Test Dialog" );

    gs_dialog->Show(  );

    return true;
}


// ----------------------------------------------------------------------------
// MyDialog implementation
// ----------------------------------------------------------------------------


MyDialog::MyDialog( const wxString& title ) : wxFrame( NULL, wxID_ANY, title )
{
    SetSize( wxSize( 800, 600 ) );
    wxSizer* const sizerTop = new wxBoxSizer( wxVERTICAL );
    sizerTop->Add( ASSISTANT_INTERFACE::get_instance().create_chat_panel( this ), wxSizerFlags(1).Expand().Border() );
    SetSizerAndFit( sizerTop );
}

MyDialog::~MyDialog()
{
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
