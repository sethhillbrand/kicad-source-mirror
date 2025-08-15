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

#ifndef DRC_RE_NUMERIC_INPUT_PANEL_H
#define DRC_RE_NUMERIC_INPUT_PANEL_H

#include "drc_re_numeric_input_panel_base.h"
#include "drc_re_content_panel_base.h"
#include "drc_re_numeric_input_constraint_data.h"
#include "drc_rule_editor_enums.h"
#include "drc_re_constraint_panel_params.h"


class DRC_RE_NUMERIC_INPUT_PANEL : public DRC_RE_NUMERIC_INPUT_PANEL_BASE, public DrcRuleEditorContentPanelBase
{
public:
    DRC_RE_NUMERIC_INPUT_PANEL( wxWindow* aParent, const DrcReNumericInputConstraintPanelParams& aConstraintPanelParams );

    ~DRC_RE_NUMERIC_INPUT_PANEL() override;

    bool TransferDataToWindow() override;

    bool TransferDataFromWindow() override;

    void StoreCatpuredValues() override;

    void BindStoredValues() override;

    double GetNumericInputValue() { return m_numericInputValue; }

private:
    double m_numericInputValue;
    std::shared_ptr<DrcReNumericInputConstraintData> m_constraintData;
};

#endif // DRC_RE_NUMERIC_INPUT_PANEL_H
