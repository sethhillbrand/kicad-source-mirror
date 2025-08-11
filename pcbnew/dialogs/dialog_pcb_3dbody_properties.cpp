#include "dialog_pcb_3dbody_properties.h"

#include <wx/sizer.h>
#include <wx/stattext.h>

DIALOG_PCB_3DBODY_PROPERTIES::DIALOG_PCB_3DBODY_PROPERTIES( wxWindow* aParent ) :
        DIALOG_SHIM( aParent, wxID_ANY, _( "3D Body Properties" ) )
{
    wxBoxSizer* sizer = new wxBoxSizer( wxVERTICAL );
    sizer->Add( new wxStaticText( this, wxID_ANY, _( "Parametric 3D body properties editing not implemented." ) ), 0,
                wxALL, 5 );
    SetSizerAndFit( sizer );
}
