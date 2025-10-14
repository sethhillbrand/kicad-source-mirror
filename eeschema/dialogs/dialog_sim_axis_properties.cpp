#include <dialog_sim_axis_properties.h>

#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/intl.h>
#include <wx/msgdlg.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>

DIALOG_SIM_AXIS_PROPERTIES::DIALOG_SIM_AXIS_PROPERTIES( wxWindow* aParent,
                                                        const SIM_AXIS_DIALOG_SETTINGS& aSettings ) :
        wxDialog( aParent, wxID_ANY, _( "Format Axis" ), wxDefaultPosition, wxDefaultSize,
                  wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER )
{
    m_results.minValue      = aSettings.minValue;
    m_results.maxValue      = aSettings.maxValue;
    m_results.boundsLocked  = aSettings.boundsLocked;
    m_results.isLogarithmic = aSettings.isLogarithmic;
    m_results.showUnits     = aSettings.showUnits;
    m_results.alignment     = aSettings.currentAlignment;

    wxBoxSizer* topSizer = new wxBoxSizer( wxVERTICAL );

    if( !aSettings.axisName.IsEmpty() )
    {
        wxStaticText* label = new wxStaticText( this, wxID_ANY, aSettings.axisName );
        label->SetFont( label->GetFont().Bold() );
        topSizer->Add( label, 0, wxALL, 10 );
    }

    wxStaticBoxSizer* boundsSizer = new wxStaticBoxSizer( wxVERTICAL, this, _( "Bounds" ) );

    m_autoBounds = new wxCheckBox( this, wxID_ANY, _( "Automatic bounds" ) );
    m_autoBounds->SetValue( !aSettings.boundsLocked );
    boundsSizer->Add( m_autoBounds, 0, wxBOTTOM, 5 );

    wxFlexGridSizer* grid = new wxFlexGridSizer( 2, 2, 5, 5 );
    grid->AddGrowableCol( 1 );

    grid->Add( new wxStaticText( this, wxID_ANY, _( "Minimum:" ) ), 0, wxALIGN_CENTER_VERTICAL );
    m_minValue = new wxTextCtrl( this, wxID_ANY );
    grid->Add( m_minValue, 1, wxEXPAND );

    grid->Add( new wxStaticText( this, wxID_ANY, _( "Maximum:" ) ), 0, wxALIGN_CENTER_VERTICAL );
    m_maxValue = new wxTextCtrl( this, wxID_ANY );
    grid->Add( m_maxValue, 1, wxEXPAND );

    boundsSizer->Add( grid, 1, wxEXPAND );
    topSizer->Add( boundsSizer, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10 );

    wxStaticBoxSizer* optionsSizer = new wxStaticBoxSizer( wxVERTICAL, this, _( "Display" ) );

    m_logScale = new wxCheckBox( this, wxID_ANY, _( "Logarithmic scale" ) );
    m_logScale->SetValue( aSettings.isLogarithmic );
    m_logScale->Enable( aSettings.logSupported );
    optionsSizer->Add( m_logScale, 0, wxBOTTOM, 5 );

    m_showUnits = new wxCheckBox( this, wxID_ANY, _( "Show units" ) );
    m_showUnits->SetValue( aSettings.showUnits );
    optionsSizer->Add( m_showUnits, 0 );

    topSizer->Add( optionsSizer, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10 );

    wxStaticBoxSizer* positionSizer = new wxStaticBoxSizer( wxVERTICAL, this, _( "Position" ) );

    positionSizer->Add( new wxStaticText( this, wxID_ANY, _( "Labels and ticks:" ) ), 0, wxBOTTOM, 5 );

    m_alignment = new wxChoice( this, wxID_ANY );

    int selection = 0;

    for( size_t ii = 0; ii < aSettings.alignments.size(); ++ii )
    {
        m_alignment->Append( aSettings.alignments[ii].second );
        m_alignmentValues.push_back( aSettings.alignments[ii].first );

        if( aSettings.alignments[ii].first == aSettings.currentAlignment )
            selection = static_cast<int>( ii );
    }

    if( !m_alignmentValues.empty() )
        m_alignment->SetSelection( selection );

    positionSizer->Add( m_alignment, 0, wxEXPAND );
    topSizer->Add( positionSizer, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10 );

    wxStdDialogButtonSizer* buttonSizer = new wxStdDialogButtonSizer();
    buttonSizer->AddButton( new wxButton( this, wxID_OK ) );
    buttonSizer->AddButton( new wxButton( this, wxID_CANCEL ) );
    buttonSizer->Realize();

    topSizer->Add( buttonSizer, 0, wxEXPAND | wxALL, 10 );

    wxString minText = wxString::Format( wxS( "%g" ), aSettings.minValue );
    wxString maxText = wxString::Format( wxS( "%g" ), aSettings.maxValue );

    m_minValue->SetValue( minText );
    m_maxValue->SetValue( maxText );

    m_autoBounds->Bind( wxEVT_CHECKBOX, &DIALOG_SIM_AXIS_PROPERTIES::onAutoBounds, this );

    wxCommandEvent dummyEvent;
    onAutoBounds( dummyEvent );

    SetSizerAndFit( topSizer );
    CentreOnParent();
}


void DIALOG_SIM_AXIS_PROPERTIES::GetResults( SIM_AXIS_DIALOG_RESULTS& aResults ) const
{
    aResults = m_results;
}


 void DIALOG_SIM_AXIS_PROPERTIES::onAutoBounds( wxCommandEvent& WXUNUSED( aEvent ) )
 {
     bool enable = !m_autoBounds->GetValue();

     m_minValue->Enable( enable );
     m_maxValue->Enable( enable );
 }


 bool DIALOG_SIM_AXIS_PROPERTIES::TransferDataFromWindow()
 {
     if( !wxDialog::TransferDataFromWindow() )
         return false;

     m_results.boundsLocked = !m_autoBounds->GetValue();

     double minValue = m_results.minValue;
     double maxValue = m_results.maxValue;

     if( m_results.boundsLocked )
     {
         if( !m_minValue->GetValue().ToDouble( &minValue ) )
         {
             wxMessageBox( _( "Please enter a valid minimum value." ), _( "Invalid value" ),
                           wxOK | wxICON_WARNING, this );
             m_minValue->SetFocus();
             m_minValue->SelectAll();
             return false;
         }

         if( !m_maxValue->GetValue().ToDouble( &maxValue ) )
         {
             wxMessageBox( _( "Please enter a valid maximum value." ), _( "Invalid value" ),
                           wxOK | wxICON_WARNING, this );
             m_maxValue->SetFocus();
             m_maxValue->SelectAll();
             return false;
         }

         if( minValue >= maxValue )
         {
             wxMessageBox( _( "The minimum value must be less than the maximum value." ),
                           _( "Invalid range" ), wxOK | wxICON_WARNING, this );
             m_minValue->SetFocus();
             m_minValue->SelectAll();
             return false;
         }
     }

     m_results.minValue      = minValue;
     m_results.maxValue      = maxValue;
     m_results.isLogarithmic = m_logScale->GetValue();
     m_results.showUnits     = m_showUnits->GetValue();

     int selection = m_alignment->GetSelection();

     if( selection == wxNOT_FOUND && !m_alignmentValues.empty() )
         selection = 0;

     if( selection != wxNOT_FOUND )
         m_results.alignment = m_alignmentValues[selection];

     return true;
 }

