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
#include <wx/log.h>
#include <algorithm>

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
    wxLogTrace( "parametric_search", "ctor: parent=%p id=%d", aParent, static_cast<int>( aId ) );
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
    wxLogTrace( "parametric_search", "ctor: baseText=%p", m_baseText );

    // Create STC as a sibling of this control to avoid GTK AddChildGTK assertion.
    wxWindow* stcParent = GetParent() ? GetParent() : aParent;
    m_stc = new wxStyledTextCtrl( stcParent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE );
    wxLogTrace( "parametric_search", "ctor: created STC=%p parent=%p", m_stc, stcParent );

    // Make STC visually match the native text control and behave like a single-line field.
    {
        wxColour bg = m_baseText ? m_baseText->GetBackgroundColour() : GetBackgroundColour();
        wxColour fg = m_baseText ? m_baseText->GetForegroundColour() : GetForegroundColour();
        wxFont   fn = m_baseText ? m_baseText->GetFont() : GetFont();

        m_stc->SetBackgroundColour( bg );
        m_stc->StyleSetBackground( wxSTC_STYLE_DEFAULT, bg );
        m_stc->StyleSetForeground( wxSTC_STYLE_DEFAULT, fg );
        m_stc->StyleSetFont( wxSTC_STYLE_DEFAULT, fn );
        m_stc->StyleClearAll(); // apply defaults to all styles

        // Remove editor chrome to resemble a plain text box
        for( int i = 0; i < 3; ++i )
            m_stc->SetMarginWidth( i, 0 );
        m_stc->SetUseHorizontalScrollBar( false );
        m_stc->SetUseVerticalScrollBar( false );
        m_stc->SetWrapMode( wxSTC_WRAP_NONE );
        m_stc->SetScrollWidthTracking( false );
        m_stc->SetScrollWidth( 1 );
        m_stc->SetEdgeMode( wxSTC_EDGE_NONE );
        m_stc->SetCaretForeground( fg );
        wxLogTrace( "parametric_search", "ctor: styled STC to match base text fg/bg and disabled scrollbars" );
    }

    // Initially position it over the text area and hide the native text.
    RepositionStyledText();
    if( m_baseText )
        m_baseText->Hide();
    wxLogTrace( "parametric_search", "ctor: initial reposition done; base hidden=%d", m_baseText ? 1 : 0 );

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
    m_stc->Bind( wxEVT_STC_AUTOCOMP_CANCELLED, []( wxStyledTextEvent& ){ wxLogTrace( "parametric_search", "AutoComp: cancelled" ); } );
    m_stc->Bind( wxEVT_SET_FOCUS, []( wxFocusEvent& ){ wxLogTrace( "parametric_search", "STC: got focus" ); } );
    m_stc->Bind( wxEVT_KILL_FOCUS, []( wxFocusEvent& ){ wxLogTrace( "parametric_search", "STC: lost focus" ); } );
    m_stc->AutoCompSetSeparator( '\n' );
    // Improve autocomplete behavior and visibility
    m_stc->AutoCompSetIgnoreCase( true );
    m_stc->AutoCompSetChooseSingle( false );
    m_stc->AutoCompSetAutoHide( false );
    m_stc->AutoCompSetMaxHeight( 12 );
    wxLogTrace( "parametric_search", "ctor: STC bindings done; separator set to \\n" );

    // Configure three rounded box indicators: parameter, operator, value
    struct IND { int idx; wxColour fg; int alpha; int oalpha; };
    IND inds[] = {
        { 0, wxColour( 105, 154, 204 ), 180, 255 },  // parameter: blue-ish
        { 1, wxColour( 125, 184, 125 ), 180, 255 },  // operator: green-ish
        { 2, wxColour( 204, 153, 102 ), 180, 255 }   // value: orange-ish
    };
    for( const auto& i : inds )
    {
        m_stc->IndicatorSetStyle( i.idx, wxSTC_INDIC_ROUNDBOX );
        m_stc->IndicatorSetUnder( i.idx, false );
        m_stc->IndicatorSetForeground( i.idx, i.fg );
        m_stc->IndicatorSetAlpha( i.idx, i.alpha );
        m_stc->IndicatorSetOutlineAlpha( i.idx, i.oalpha );
    }
    wxLogTrace( "parametric_search", "ctor: indicators configured" );
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
    // Merge default base fields with provided parameters, unique + stable
    static const wxString kBase[] = { "Value", "Datasheet", "Footprint", "Description", "Item" };
    m_parameters.clear();
    for( const auto& s : kBase ) m_parameters.push_back( s );
    for( const auto& s : aParameters ) m_parameters.push_back( s );
    std::sort( m_parameters.begin(), m_parameters.end(), []( const wxString& a, const wxString& b ){ return a.CmpNoCase( b ) < 0; } );
    m_parameters.erase( std::unique( m_parameters.begin(), m_parameters.end(), []( const wxString& a, const wxString& b ){ return a.IsSameAs( b, false ); } ), m_parameters.end() );
    wxLogTrace( "parametric_search", "SetParameters: count=%zu", m_parameters.size() );
    // Trace items to verify expected suggestions are present
    for( const auto& p : m_parameters )
        wxLogTrace( "parametric_search", "SetParameters: item='%s'", p );
}

void PARAMETRIC_SEARCH_CTRL::GetParametricTerms( wxString& aSearch, std::vector<TERM>& aTerms ) const
{
    aSearch = m_stc->GetText();
    aTerms  = m_terms;
    wxLogTrace( "parametric_search", "GetParametricTerms: searchLen=%d terms=%zu", (int) aSearch.Length(), aTerms.size() );
}

void PARAMETRIC_SEARCH_CTRL::SetValue( const wxString& aValue )
{
    wxLogTrace( "parametric_search", "SetValue: len=%d", (int) aValue.Length() );
    m_stc->SetText( aValue );
    m_terms.clear();
    m_segRanges.clear();
    m_stage = STAGE::STAGE1;
    ResetCurrent();
    NotifyTextChange();
}

void PARAMETRIC_SEARCH_CTRL::ChangeValue( const wxString& aValue )
{
    wxLogTrace( "parametric_search", "ChangeValue: len=%d", (int) aValue.Length() );
    m_stc->SetText( aValue );
    m_terms.clear();
    m_segRanges.clear();
    m_stage = STAGE::STAGE1;
    ResetCurrent();
    NotifyTextChange();
}

wxString PARAMETRIC_SEARCH_CTRL::GetValue() const
{
    wxLogTrace( "parametric_search", "GetValue: len=%d", (int) m_stc->GetTextLength() );
    return m_stc->GetText();
}

void PARAMETRIC_SEARCH_CTRL::OnChar( wxKeyEvent& aEvent )
{
    int key = aEvent.GetKeyCode();
    wxLogTrace( "parametric_search", "OnChar: key=%d stage=%d", key, (int) m_stage );

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
            wxLogTrace( "parametric_search", "OnChar: ESC in STAGE2 -> revert to STAGE1" );
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
            wxLogTrace( "parametric_search", "OnChar: ESC in STAGE3 -> revert to STAGE2" );
            return;
        }

        aEvent.Skip();
    this->CallAfter( &PARAMETRIC_SEARCH_CTRL::NotifyTextChange );
    wxLogTrace( "parametric_search", "OnChar: ESC passthrough" );
        return;
    }
    else if( key == WXK_BACK )
    {
        // First, if caret is adjacent to a styled chip, convert it to plain text (do not delete text)
        if( ConvertAdjacentChipToPlainText() )
        {
            ApplyTermIndicators();
            this->CallAfter( &PARAMETRIC_SEARCH_CTRL::NotifyTextChange );
            wxLogTrace( "parametric_search", "OnChar: BACK converted chip to plain text" );
            return;
        }

        if( m_stage == STAGE::STAGE3 )
        {
            if( m_current.value.IsEmpty() )
            {
                m_stage = STAGE::STAGE2;
                wxLogTrace( "parametric_search", "OnChar: BACK in STAGE3 -> STAGE2" );
            }
            else
            {
                m_current.value.RemoveLast();
                wxLogTrace( "parametric_search", "OnChar: BACK in STAGE3 value-1 len=%d", (int) m_current.value.Length() );
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
            wxLogTrace( "parametric_search", "OnChar: BACK in STAGE2 -> STAGE1" );
            aEvent.Skip();
            return;
        }
        else if( m_stage == STAGE::STAGE1 )
        {
            int pos = m_stc->GetCurrentPos();

            if( pos > 1 && m_stc->GetCharAt( pos - 1 ) == ' ' )
            {
                // If at boundary after a finalized term, first convert the adjacent segment (likely VALUE)
                // into plain text by clearing its rounded rectangle, without deleting any characters.
                int segIdx = FindSegmentAtPos( pos - 2 );
                if( segIdx >= 0 )
                {
                    if( m_segRanges[ segIdx ].styled )
                    {
                        m_stc->SetIndicatorCurrent( m_segRanges[ segIdx ].style );
                        m_stc->IndicatorClearRange( m_segRanges[ segIdx ].start, m_segRanges[ segIdx ].length );
                        m_segRanges[ segIdx ].styled = false;
                        ApplyTermIndicators();
                        NotifyTextChange();
                        wxLogTrace( "parametric_search", "OnChar: BACK converted adjacent chip to plain text at pos=%d", pos - 2 );
                        return;
                    }
                }
            }
        }
    }

    aEvent.Skip();
    this->CallAfter( &PARAMETRIC_SEARCH_CTRL::NotifyTextChange );
    wxLogTrace( "parametric_search", "OnChar: skipped, forwarded to default handler" );
}

void PARAMETRIC_SEARCH_CTRL::OnText( wxStyledTextEvent& aEvent )
{
    wxLogTrace( "parametric_search", "OnText: stage=%d pos=%d", (int) m_stage, m_stc->GetCurrentPos() );
    if( m_stage == STAGE::STAGE1 )
    {
        // Use STC word boundaries to determine the active token
        const int curPos = m_stc->GetCurrentPos();
        const int start  = m_stc->WordStartPosition( curPos, true );
        const wxString token = m_stc->GetTextRange( start, curPos );
        const wxString tokenLower = token.Lower();
        wxArrayString matches;

        for( const wxString& p : m_parameters )
        {
            // Case-insensitive contains so users can type lowercase and still match (e.g. 'dat' -> 'Datasheet')
            if( p.Lower().Contains( tokenLower ) )
                matches.push_back( p );
        }

        if( !matches.IsEmpty() )
        {
            wxString list;

            for( size_t i = 0; i < matches.GetCount(); ++i )
            {
                list += matches[i];
                if( i + 1 < matches.GetCount() )
                    list += "\n"; // separator is set to '\n'
            }

            m_stc->AutoCompShow( static_cast<int>( token.Length() ), list );
            wxLogTrace( "parametric_search", "OnText: showing autocomplete with %zu matches for token '%s' active=%d",
                        (size_t) matches.size(), token, m_stc->AutoCompActive() ? 1 : 0 );
        }
        else
        {
            if( m_stc->AutoCompActive() )
                m_stc->AutoCompCancel();
            wxLogTrace( "parametric_search", "OnText: no matches for token '%s'", token );
        }
    }
    else if( m_stage == STAGE::STAGE3 )
    {
    int pos = m_stc->GetCurrentPos();
        wxChar ch = m_stc->GetCharAt( pos - 1 );

        if( ch == '"' )
            m_inQuotes = !m_inQuotes;
        wxLogTrace( "parametric_search", "OnText: stage3 ch='%c' inQuotes=%d", (int) ch, m_inQuotes ? 1 : 0 );

        if( !m_inQuotes && ch == ' ' )
        {
            wxString comp = ComparisonToString( m_current.compare );
            int pos2 = m_stc->GetCurrentPos();
            int lenParam = static_cast<int>( m_current.parameter.Length() );
            int lenOp    = static_cast<int>( comp.Length() );
            int lenVal   = static_cast<int>( m_current.value.Length() );
            int totalLen = lenParam + 1 + lenOp + 1 + lenVal; // with spaces
            int start    = pos2 - totalLen - 1; // include trailing space typed now
            if( start < 0 ) start = 0; // clamp to avoid invalid ranges

            int termIndex = (int) m_terms.size();
            m_terms.push_back( m_current );
            // Record three segments
            m_segRanges.push_back( { SEGMENT::PARAM, start, lenParam, termIndex, true, 0 } );
            m_segRanges.push_back( { SEGMENT::OP,    start + lenParam + 1, lenOp, termIndex, true, 1 } );
            m_segRanges.push_back( { SEGMENT::VALUE, start + lenParam + 1 + lenOp + 1, lenVal, termIndex, true, 2 } );
            ApplyTermIndicators();
            ResetCurrent();
            m_stage = STAGE::STAGE1;
            wxLogTrace( "parametric_search", "OnText: finalized term, range=[%d,%d]", start, totalLen );
        }
        else
        {
            m_current.value += ch;
            wxLogTrace( "parametric_search", "OnText: value now len=%d", (int) m_current.value.Length() );
        }
    }

    NotifyTextChange();
}

void PARAMETRIC_SEARCH_CTRL::OnAutoComplete( wxStyledTextEvent& aEvent )
{
    wxString sel = aEvent.GetText();
    wxLogTrace( "parametric_search", "OnAutoComplete: sel='%s' stage=%d", sel, (int) m_stage );

    if( m_stage == STAGE::STAGE1 )
    {
        m_current.parameter = sel;
        m_stage = STAGE::STAGE2;
        // After accepting the parameter, schedule showing the comparison ops.
        // Do this asynchronously to avoid re-entrancy with the previous popup teardown.
        m_stc->CallAfter( [this]
        {
            if( !m_stc )
                return;
            // Ensure any previous popup is closed before opening the next
            if( m_stc->AutoCompActive() )
                m_stc->AutoCompCancel();
            m_stc->SetFocus();
            m_stc->AddText( " " );
            m_stc->AutoCompShow( 0, "is\nis not\ncontains\ndoes not contain" );
            wxLogTrace( "parametric_search", "OnAutoComplete: moved to STAGE2; showing ops list (async) active=%d", m_stc->AutoCompActive() ? 1 : 0 );
        } );
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
        wxLogTrace( "parametric_search", "OnAutoComplete: moved to STAGE3" );
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
    // Clear all indicators first
    for( int idx : { 0, 1, 2 } )
    {
        m_stc->SetIndicatorCurrent( idx );
        m_stc->IndicatorClearRange( 0, m_stc->GetTextLength() );
    }

    // Apply per-segment rounded boxes
    size_t count = 0;
    int docLen = m_stc->GetTextLength();
    for( SEG_RANGE r : m_segRanges )
    {
        if( !r.styled ) continue;
        // Clamp ranges to document bounds to avoid no-op fills
        if( r.start < 0 ) r.start = 0;
        if( r.start > docLen ) continue;
        if( r.length < 0 ) r.length = 0;
        if( r.start + r.length > docLen ) r.length = docLen - r.start;
        if( r.length <= 0 ) continue;
        m_stc->SetIndicatorCurrent( r.style );
        m_stc->IndicatorFillRange( r.start, r.length );
        ++count;
    }
    wxLogTrace( "parametric_search", "ApplyTermIndicators: count=%zu docLen=%d", count, docLen );
}

void PARAMETRIC_SEARCH_CTRL::NotifyTextChange()
{
    wxCommandEvent evt( wxEVT_TEXT, GetId() );
    evt.SetEventObject( this );
    evt.SetString( m_stc->GetText() );
    // Post asynchronously to avoid re-entrancy that could reset stage during autocomplete flows
    wxPostEvent( this, evt );
    wxLogTrace( "parametric_search", "NotifyTextChange: textLen=%d terms=%zu stage=%d (posted)", (int) m_stc->GetTextLength(), m_terms.size(), (int) m_stage );
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
    wxLogTrace( "parametric_search", "RepositionStyledText: using baseText geom at %d,%d size %dx%d", parentTL.x, parentTL.y, sz.x, sz.y );
    }
    else
    {
        wxPoint tl = ClientToScreen( r.GetTopLeft() );
        wxPoint parentTL = m_stc->GetParent()->ScreenToClient( tl );
        m_stc->SetSize( wxRect( parentTL, r.GetSize() ) );
    wxLogTrace( "parametric_search", "RepositionStyledText: using client rect at %d,%d size %dx%d", parentTL.x, parentTL.y, r.width, r.height );
    }

    m_stc->Raise();
}

int PARAMETRIC_SEARCH_CTRL::FindSegmentAtPos( int aPos ) const
{
    for( size_t i = 0; i < m_segRanges.size(); ++i )
    {
        const auto& r = m_segRanges[i];
        if( aPos >= r.start && aPos < r.start + r.length )
            return (int) i;
    }
    return -1;
}

bool PARAMETRIC_SEARCH_CTRL::ConvertAdjacentChipToPlainText()
{
    int pos = m_stc->GetCurrentPos();
    // Check left and right of caret for a segment boundary
    int segIdx = FindSegmentAtPos( pos - 1 );
    if( segIdx < 0 )
        segIdx = FindSegmentAtPos( pos );
    if( segIdx < 0 )
        return false;

    SEG_RANGE& seg = m_segRanges[ segIdx ];
    if( !seg.styled )
        return false; // already plain

    // Clear this chip's indicator only (do not modify text)
    m_stc->SetIndicatorCurrent( seg.style );
    m_stc->IndicatorClearRange( seg.start, seg.length );
    seg.styled = false;
    return true;
}

