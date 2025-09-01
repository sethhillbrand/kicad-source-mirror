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

#ifndef RULER_WIDGET_H
#define RULER_WIDGET_H

#include <eda_units.h>
#include <wx/panel.h>
#include <wx/timer.h>

class RULER_WIDGET : public wxPanel
{
public:
    enum class ORIENTATION
    {
        HORIZONTAL,
        VERTICAL
    };

    RULER_WIDGET( wxWindow* aParent, ORIENTATION aOrientation );
    ~RULER_WIDGET();

    void SetUnits( EDA_UNITS aUnits );
    void SetPixelsPerUnit( double aScale );
    void SetCursorPos( const wxPoint& aCursorPos );

private:
    void OnPaint( wxPaintEvent& event );
    void OnSize( wxSizeEvent& event );
    void OnUnitsChanged( wxCommandEvent& event );

    void bindToParentEvents();
    void unbindFromParentEvents();
    void updateViewport();

    ORIENTATION m_orientation;
    EDA_UNITS   m_units;
    double      m_pixelsPerUnit;
    int         m_cursorPos;
    wxTimer     m_zoomTimer;
    double      m_lastZoom;
    VECTOR2D    m_viewportOrigin;  // World coordinates of top-left corner of viewport
    VECTOR2D    m_lastViewportOrigin;

    wxDECLARE_EVENT_TABLE();
};

#endif
