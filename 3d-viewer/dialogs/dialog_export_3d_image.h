/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright The KiCad Developers, see AUTHORS.txt for contributors.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you may find one here:
 * http://www.gnu.org/licenses/gpl-3.0.html
 * or you may search the http://www.gnu.org website for the version 3 license,
 * or you may write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#pragma once

#include "dialog_shim.h"
#include <3d_viewer/eda_3d_viewer_frame.h> // for EDA_3D_VIEWER_EXPORT_FORMAT
#include <wx/spinctrl.h>
#include <wx/choice.h>

class DIALOG_EXPORT_3D_IMAGE : public DIALOG_SHIM
{
public:
    DIALOG_EXPORT_3D_IMAGE( wxWindow* aParent, EDA_3D_VIEWER_EXPORT_FORMAT aFormat,
                            const wxSize& aSize );

    EDA_3D_VIEWER_EXPORT_FORMAT GetFormat() const { return m_format; }
    wxSize GetSize() const { return wxSize( m_width, m_height ); }

private:
    bool TransferDataFromWindow() override;

    EDA_3D_VIEWER_EXPORT_FORMAT m_format;
    int m_width;
    int m_height;

    wxSpinCtrl* m_spinWidth;
    wxSpinCtrl* m_spinHeight;
    wxChoice*   m_choiceFormat;
};

