/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright The KiCad Developers, see AUTHORS.txt for contributors.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you may find one here:
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 * or you may search the http://www.gnu.org website for the version 2 license,
 * or you may write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#include <widgets/parametric_search_ctrl.h>
#include <wx/sizer.h>
#include <wx/colour.h>
#include <wx/srchctrl.h>
#include <wx/textctrl.h>
#include <wx/window.h>
#include <wx/event.h>
#include <wx/app.h>
#include <wx/stc/stc.h>

static wxString ComparisonToString( PARAMETRIC_SEARCH_CTRL::COMPARISON aComp )
{
    switch( aComp )
    {
    case PARAMETRIC_SEARCH_CTRL::COMPARISON::IS:
        return "is";
    case PARAMETRIC_SEARCH_CTRL::COMPARISON::IS_NOT:
        return "is not";
    case PARAMETRIC_SEARCH_CTRL::COMPARISON::CONTAINS:
        return "contains";
    default:
        return "does not contain";
    }
}

PARAMETRIC_SEARCH_CTRL::PARAMETRIC_SEARCH_CTRL( wxWindow* aParent, wxWindowID aId )
    : wxSearchCtrl( aParent, aId )
{
    // Find the internal text window of wxSearchCtrl so we can mirror its geometry.
    m_baseText = nullptr;
    for( wxWindowList::compatibility_iterator it = GetChildren().GetFirst(); it; it = it->GetNext() )
    {
        wxWindow* w = it->GetData();
        if( dynamic_cast<wxTextCtrl*>( w ) )
        {
            m_baseText = w;
            break;
        }
    }

    // Create STC as a sibling of this control to avoid GTK AddChildGTK assertion.
    wxWindow* stcParent = GetParent() ? GetParent() : aParent;
    m_stc = new wxStyledTextCtrl( stcParent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE );

    // Initially position it over the text area and hide the native text.
    RepositionStyledText();
    if( m_baseText )
        m_baseText->Hide();

    // Keep STC in sync with this control's geometry.
    Bind( wxEVT_SIZE, [this]( wxSizeEvent& ) { RepositionStyledText(); } );
    Bind( wxEVT_MOVE, [this]( wxMoveEvent& ) { RepositionStyledText(); } );
    Bind( wxEVT_SHOW, [this]( wxShowEvent& e ) {
        if( e.IsShown() && m_stc )
            m_stc->Show();
        RepositionStyledText();
        e.Skip();
    } );

    m_stc->Bind( wxEVT_STC_CHARADDED, &PARAMETRIC_SEARCH_CTRL::OnText, this );
    m_stc->Bind( wxEVT_CHAR, &PARAMETRIC_SEARCH_CTRL::OnChar, this );
    m_stc->Bind( wxEVT_STC_AUTOCOMP_SELECTION, &PARAMETRIC_SEARCH_CTRL::OnAutoComplete, this );
    m_stc->AutoCompSetSeparator( '\n' );

    m_stc->IndicatorSetStyle( 0, wxSTC_INDIC_ROUNDBOX );
    m_stc->IndicatorSetUnder( 0, true );
    m_stc->IndicatorSetForeground( 0, wxColour( 180, 180, 180 ) );
    m_stc->IndicatorSetAlpha( 0, 40 );
    m_stc->IndicatorSetOutlineAlpha( 0, 200 );
}

PARAMETRIC_SEARCH_CTRL::~PARAMETRIC_SEARCH_CTRL()
{
    if( m_baseText )
        m_baseText->Show();

    if( m_stc )
    {
        m_stc->Hide();
        m_stc->Destroy();
        m_stc = nullptr;
    }
}

void PARAMETRIC_SEARCH_CTRL::SetParameters( const std::vector<wxString>& aParameters )
{
    m_parameters = aParameters;
}

void PARAMETRIC_SEARCH_CTRL::GetParametricTerms( wxString& aSearch, std::vector<TERM>& aTerms ) const
{
    aSearch = m_stc->GetText();
    aTerms  = m_terms;
}

void PARAMETRIC_SEARCH_CTRL::SetValue( const wxString& aValue )
{
    m_stc->SetText( aValue );
    m_terms.clear();
    m_termRanges.clear();
    m_stage = STAGE::STAGE1;
    ResetCurrent();
    NotifyTextChange();
}

void PARAMETRIC_SEARCH_CTRL::ChangeValue( const wxString& aValue )
{
    m_stc->SetText( aValue );
    m_terms.clear();
    m_termRanges.clear();
    m_stage = STAGE::STAGE1;
    ResetCurrent();
    NotifyTextChange();
}

wxString PARAMETRIC_SEARCH_CTRL::GetValue() const
{
    return m_stc->GetText();
}

void PARAMETRIC_SEARCH_CTRL::OnChar( wxKeyEvent& aEvent )
{
    int key = aEvent.GetKeyCode();

    if( key == WXK_ESCAPE )
    {
        if( m_stage == STAGE::STAGE2 )
        {
            int  pos   = m_stc->GetCurrentPos();
            int  start = pos - static_cast<int>( m_current.parameter.Length() + 1 );
            m_stc->DeleteRange( start, static_cast<int>( m_current.parameter.Length() + 1 ) );
            ResetCurrent();
            m_stage = STAGE::STAGE1;
            this->CallAfter( &PARAMETRIC_SEARCH_CTRL::NotifyTextChange );
            return;
        }
        else if( m_stage == STAGE::STAGE3 )
        {
            int pos = m_stc->GetCurrentPos();
            m_stc->DeleteRange( pos - static_cast<int>( m_current.value.Length() ),
                                 static_cast<int>( m_current.value.Length() ) );
            m_current.value.clear();
            m_stage = STAGE::STAGE2;
            this->CallAfter( &PARAMETRIC_SEARCH_CTRL::NotifyTextChange );
            return;
        }

        aEvent.Skip();
    this->CallAfter( &PARAMETRIC_SEARCH_CTRL::NotifyTextChange );
        return;
    }
    else if( key == WXK_BACK )
    {
        if( m_stage == STAGE::STAGE3 )
        {
            if( m_current.value.IsEmpty() )
            {
                m_stage = STAGE::STAGE2;
            }
            else
            {
                m_current.value.RemoveLast();
            }

            this->CallAfter( &PARAMETRIC_SEARCH_CTRL::NotifyTextChange );
            aEvent.Skip();
            return;
        }
        else if( m_stage == STAGE::STAGE2 )
        {
            m_stage = STAGE::STAGE1;
            ResetCurrent();
            this->CallAfter( &PARAMETRIC_SEARCH_CTRL::NotifyTextChange );
            aEvent.Skip();
            return;
        }
        else if( m_stage == STAGE::STAGE1 )
        {
            int pos = m_stc->GetCurrentPos();

            if( pos > 1 && m_stc->GetCharAt( pos - 1 ) == ' '
                    && m_stc->IndicatorValueAt( 0, pos - 2 ) )
            {
                m_stc->DeleteRange( pos - 1, 1 );

                TERM term = m_terms.back();
                m_terms.pop_back();
                TERM_RANGE range = m_termRanges.back();
                m_termRanges.pop_back();

                m_stc->SetIndicatorCurrent( 0 );
                m_stc->IndicatorClearRange( range.start, range.length );
                m_current = term;
                m_stage = STAGE::STAGE3;
                m_inQuotes = false;
                m_stc->GotoPos( range.start + range.length );
                NotifyTextChange();
                return;
            }
        }
    }

    aEvent.Skip();
    this->CallAfter( &PARAMETRIC_SEARCH_CTRL::NotifyTextChange );
}

void PARAMETRIC_SEARCH_CTRL::OnText( wxStyledTextEvent& aEvent )
{
    if( m_stage == STAGE::STAGE1 )
    {
        wxString txt = m_stc->GetTextRange( 0, m_stc->GetCurrentPos() );
        wxString token = txt.AfterLast( ' ' );
        wxArrayString matches;

        for( const wxString& p : m_parameters )
        {
            if( p.Contains( token ) )
                matches.push_back( p );
        }

        if( !matches.IsEmpty() )
        {
            wxString list;

            for( size_t i = 0; i < matches.GetCount(); ++i )
            {
                list += matches[i];
                if( i + 1 < matches.GetCount() )
                    list += " ";
            }

            m_stc->AutoCompShow( static_cast<int>( token.Length() ), list );
        }
    }
    else if( m_stage == STAGE::STAGE3 )
    {
    int pos = m_stc->GetCurrentPos();
        wxChar ch = m_stc->GetCharAt( pos - 1 );

        if( ch == '"' )
            m_inQuotes = !m_inQuotes;

        if( !m_inQuotes && ch == ' ' )
        {
            wxString comp = ComparisonToString( m_current.compare );
            int pos2 = m_stc->GetCurrentPos();
            int len = static_cast<int>( m_current.parameter.Length() + 1 + comp.Length() + 1
                                        + m_current.value.Length() );
            int start = pos2 - len - 1;

            m_terms.push_back( m_current );
            m_termRanges.push_back( { start, len } );
            ApplyTermIndicators();
            ResetCurrent();
            m_stage = STAGE::STAGE1;
        }
        else
        {
            m_current.value += ch;
        }
    }

    NotifyTextChange();
}

void PARAMETRIC_SEARCH_CTRL::OnAutoComplete( wxStyledTextEvent& aEvent )
{
    wxString sel = aEvent.GetText();

    if( m_stage == STAGE::STAGE1 )
    {
        m_current.parameter = sel;
        m_stage = STAGE::STAGE2;

    m_stc->AddText( " " );
    m_stc->AutoCompShow( 0, "is\nis not\ncontains\ndoes not contain" );
    }
    else if( m_stage == STAGE::STAGE2 )
    {
        if( sel == "is" )
            m_current.compare = COMPARISON::IS;
        else if( sel == "is not" )
            m_current.compare = COMPARISON::IS_NOT;
        else if( sel == "contains" )
            m_current.compare = COMPARISON::CONTAINS;
        else
            m_current.compare = COMPARISON::DOES_NOT_CONTAIN;

        m_stage = STAGE::STAGE3;
        m_stc->AddText( " " );
    }

    NotifyTextChange();
}

void PARAMETRIC_SEARCH_CTRL::ResetCurrent()
{
    m_current.parameter.clear();
    m_current.value.clear();
    m_inQuotes = false;
}

void PARAMETRIC_SEARCH_CTRL::ApplyTermIndicators()
{
    m_stc->SetIndicatorCurrent( 0 );
    m_stc->IndicatorClearRange( 0, m_stc->GetTextLength() );

    for( const TERM_RANGE& r : m_termRanges )
        m_stc->IndicatorFillRange( r.start, r.length );
}

void PARAMETRIC_SEARCH_CTRL::NotifyTextChange()
{
    wxCommandEvent evt( wxEVT_TEXT, GetId() );
    evt.SetEventObject( this );
    evt.SetString( m_stc->GetText() );
    ProcessWindowEvent( evt );
}

void PARAMETRIC_SEARCH_CTRL::SetFocus()
{
    if( m_stc )
        m_stc->SetFocus();
    else
        wxSearchCtrl::SetFocus();
}

void PARAMETRIC_SEARCH_CTRL::RepositionStyledText()
{
    if( !m_stc )
        return;

    // Compute where the editable area of the search control lies in parent coords.
    wxRect r = GetClientRect();

    // Account for search/cancel buttons by using the internal text control rect if available.
    if( m_baseText )
    {
        wxPoint tl = m_baseText->GetScreenPosition();
        wxSize  sz = m_baseText->GetSize();
        wxPoint parentTL = m_stc->GetParent()->ScreenToClient( tl );
        m_stc->SetSize( wxRect( parentTL, sz ) );
    }
    else
    {
        wxPoint tl = ClientToScreen( r.GetTopLeft() );
        wxPoint parentTL = m_stc->GetParent()->ScreenToClient( tl );
        m_stc->SetSize( wxRect( parentTL, r.GetSize() ) );
    }

    m_stc->Raise();
}

