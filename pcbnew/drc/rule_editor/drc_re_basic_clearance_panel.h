/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2024 KiCad Developers, see AUTHORS.txt for contributors.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DRC_RE_BASIC_CLEARANCE_PANEL_H
#define DRC_RE_BASIC_CLEARANCE_PANEL_H

#include "drc_re_basic_clearance_panel_base.h"
#include "drc_re_content_panel_base.h"
#include "drc_re_base_clearance_constraint_data.h"


class DRC_RE_BASIC_CLEARANCE_PANEL : public DRC_RE_BASIC_CLEARANCE_PANEL_BASE, public DrcRuleEditorContentPanelBase
{
public:
    DRC_RE_BASIC_CLEARANCE_PANEL( wxWindow* aParent, wxString* aConstraintTitle , std::shared_ptr<DrcReBaseClearanceConstraintData> aConstraintData );

    ~DRC_RE_BASIC_CLEARANCE_PANEL() override;

    bool TransferDataToWindow() override;

    bool TransferDataFromWindow() override;

    void StoreCatpuredValues() override;

    void BindStoredValues() override;

    double GetClearance() { return m_clearanceValue; }

private:
    double m_clearanceValue;
    std::shared_ptr<DrcReBaseClearanceConstraintData> m_constraintData;
};

#endif // DRC_RE_BASIC_CLEARANCE_PANEL_H
