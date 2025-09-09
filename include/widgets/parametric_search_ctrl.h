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

#ifndef PARAMETRIC_SEARCH_CTRL_H
#define PARAMETRIC_SEARCH_CTRL_H

#include <kicommon.h>
#include <wx/srchctrl.h>
#include <wx/stc/stc.h>
#include <vector>

/**
 * A search control that supports parameterized search terms.
 *
 * The control maintains a list of parameters and guides the user through
 * constructing parameter / comparison / value triplets.  It uses the
 * Scintilla autocompletion built into wxStyledTextCtrl to display
 * suggestions for parameters and comparison operators.
 */
class KICOMMON_API PARAMETRIC_SEARCH_CTRL : public wxSearchCtrl
{
public:
    enum class COMPARISON
    {
        IS,
        IS_NOT,
        CONTAINS,
        DOES_NOT_CONTAIN
    };

    struct TERM
    {
        wxString    parameter;
        COMPARISON  compare = COMPARISON::IS;
        wxString    value;
    };

    PARAMETRIC_SEARCH_CTRL( wxWindow* aParent, wxWindowID aId = wxID_ANY );
    ~PARAMETRIC_SEARCH_CTRL();

    /**
     * Set the list of known parameters that can be used when composing
     * searches.
     */
    void SetParameters( const std::vector<wxString>& aParameters );

    /**
     * Retrieve the current search string and parametric terms.
     */
    void GetParametricTerms( wxString& aSearch, std::vector<TERM>& aTerms ) const;

    void SetValue( const wxString& aValue ) override;
    void ChangeValue( const wxString& aValue ) override;
    wxString GetValue() const override;

    void SetFocus() override;

private:
    enum class STAGE
    {
        STAGE1,    ///< entering parameter or free text
        STAGE2,    ///< selecting comparison operator
        STAGE3     ///< entering value
    };

    void OnChar( wxKeyEvent& aEvent );
    void OnText( wxStyledTextEvent& aEvent );
    void OnAutoComplete( wxStyledTextEvent& aEvent );
    void ResetCurrent();
    void ApplyTermIndicators();
    void NotifyTextChange();
    void RepositionStyledText();

    struct TERM_RANGE
    {
        int start;
        int length;
    };

    wxStyledTextCtrl*          m_stc = nullptr;
    wxWindow*                  m_baseText = nullptr;    // internal text area of wxSearchCtrl
    std::vector<wxString>      m_parameters;
    std::vector<TERM>          m_terms;
    STAGE                      m_stage = STAGE::STAGE1;
    TERM                       m_current;
    bool                       m_inQuotes = false;
    std::vector<TERM_RANGE>    m_termRanges;
};

#endif    // PARAMETRIC_SEARCH_CTRL_H

