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

#ifndef DRC_RE_PARALLEL_LIMIT_PANEL_H
#define DRC_RE_PARALLEL_LIMIT_PANEL_H

#include "drc_re_parallel_limit_panel_base.h"
#include "drc_re_content_panel_base.h"
#include "drc_re_parallel_limit_constraint_data.h"
#include "drc_rule_editor_utils.h"


class DRC_RE_PARALLEL_LIMIT_PANEL : public DRC_RE_PARALLEL_LIMIT_PANEL_BASE, public DrcRuleEditorContentPanelBase
{
public:
    DRC_RE_PARALLEL_LIMIT_PANEL( wxWindow* aParent, wxString* aConstraintTitle , std::shared_ptr<DrcReParallelLimitConstraintData> aConstraintData );

    ~DRC_RE_PARALLEL_LIMIT_PANEL() override;

    bool TransferDataToWindow() override;

    bool TransferDataFromWindow() override;

    bool ValidateInputs( int* aErrorCount, std::string* aValidationMessage ) override;

private:    
    std::shared_ptr<DrcReParallelLimitConstraintData> m_constraintData;
};

#endif // DRC_RE_PARALLEL_LIMIT_PANEL_H
