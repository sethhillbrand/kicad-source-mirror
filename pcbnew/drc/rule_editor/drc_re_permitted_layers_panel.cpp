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

#include <pgm_base.h>
#include <settings/settings_manager.h>
#include <footprint_editor_settings.h>
#include <template_fieldnames.h>
#include <widgets/std_bitmap_button.h>
#include <grid_tricks.h>
#include <eda_text.h>
#include "drc_re_permitted_layers_panel.h"
#include <grid_layer_box_helpers.h>
#include <bitmaps.h>
#include <confirm.h>
#include <kidialog.h>
#include <wx/bitmap.h>
#include <wx/statbmp.h>

DRC_RE_PERMITTED_LAYERS_PANEL::DRC_RE_PERMITTED_LAYERS_PANEL( wxWindow* aParent, wxString* aConstraintTitle, 
                                                        std::shared_ptr<DrcRePermittedLayersConstraintData> aConstraintData ) :
        DRC_RE_PERMITTED_LAYERS_PANEL_BASE( aParent ),
        m_constraintData( aConstraintData ), 
        m_allowPermittedLayers( false )
{
    wxStaticBitmap* constraintBitmap = new wxStaticBitmap( this,  wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, 0 );
    constraintBitmap->SetBitmap( KiBitmapBundle( BITMAPS::constraint_permitted_layers ) );

    bConstraintImageSizer->Add( constraintBitmap, 0, wxALL | wxEXPAND, 10 );
}


DRC_RE_PERMITTED_LAYERS_PANEL::~DRC_RE_PERMITTED_LAYERS_PANEL()
{
}


bool DRC_RE_PERMITTED_LAYERS_PANEL::TransferDataToWindow()
{
    return true;
}


bool DRC_RE_PERMITTED_LAYERS_PANEL::TransferDataFromWindow()
{
    return false;
}