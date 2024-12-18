/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2024 KiCad Developers, see AUTHORS.txt for contributors.
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

#ifndef DRC_RE_BASE_DATA_H_
#define DRC_RE_BASE_DATA_H_

class RuleEditorBaseData
{
public:
    RuleEditorBaseData() = default;

    explicit RuleEditorBaseData( unsigned int aId, unsigned int aParentId,wxString aRuleName ) :
            m_id( aId ), m_ParentId( aParentId ), m_ruleName( aRuleName )
    {
    }

    virtual ~RuleEditorBaseData() = default;

    unsigned int GetId() { return m_id; }
    void         SetId( unsigned int aId ) { m_id = aId; }

    unsigned int GetParentId() { return m_ParentId.value_or( -1 ); }
    void         SetParentId( unsigned int aParentId ) { m_ParentId = aParentId; }

    wxString GetRuleName() { return m_ruleName; }
    void     SetRuleName( wxString aRuleName ) { m_ruleName = aRuleName; }

    wxString GetComment() { return m_comment; }
    void     SetComment( wxString aComment ) { m_comment = aComment; }

protected:
    unsigned int m_id;
    std::optional<unsigned int> m_ParentId;
    wxString m_ruleName;
    wxString m_comment;
};


#endif // DRC_RE_BASE_DATA_H_