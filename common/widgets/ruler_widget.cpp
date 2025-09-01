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

#include <widgets/ruler_widget.h>

#include <cmath>

#include <wx/dcclient.h>
#include <wx/dcbuffer.h>
#include <wx/settings.h>
#include <wx/graphics.h>
#include <ui_events.h>
#include <eda_draw_frame.h>
#include <class_draw_panel_gal.h>
#include <gal/graphics_abstraction_layer.h>
#include <view/view.h>
#include <math/vector2d.h>

// clang-format off
wxBEGIN_EVENT_TABLE( RULER_WIDGET, wxPanel )
    EVT_PAINT( RULER_WIDGET::OnPaint )
    EVT_SIZE( RULER_WIDGET::OnSize )
wxEND_EVENT_TABLE()
// clang-format on

RULER_WIDGET::RULER_WIDGET( wxWindow* aParent, ORIENTATION aOrientation ) :
        wxPanel( aParent, wxID_ANY ),
        m_orientation( aOrientation ),
        m_units( EDA_UNITS::MM ),
        m_pixelsPerUnit( 10.0 ),
        m_cursorPos( -1 ),
        m_lastZoom( 0.0 ),
        m_viewportOrigin( 0, 0 ),
        m_lastViewportOrigin( 0, 0 )
{
    SetBackgroundStyle( wxBG_STYLE_PAINT );

    // Set fixed size to prevent user resizing
    if( m_orientation == ORIENTATION::HORIZONTAL )
    {
        SetMinSize( wxSize( -1, 30 ) );
        SetMaxSize( wxSize( -1, 30 ) );
    }
    else
    {
        SetMinSize( wxSize( 30, -1 ) );
        SetMaxSize( wxSize( 30, -1 ) );
    }

    bindToParentEvents();
}

RULER_WIDGET::~RULER_WIDGET()
{
    unbindFromParentEvents();
}

void RULER_WIDGET::SetUnits( EDA_UNITS aUnits )
{
    m_units = aUnits;
    Refresh();
}

void RULER_WIDGET::SetPixelsPerUnit( double aScale )
{
    m_pixelsPerUnit = aScale;
    Refresh();
}

void RULER_WIDGET::SetCursorPos( const wxPoint& aCursorPos )
{
    m_cursorPos = m_orientation == ORIENTATION::HORIZONTAL ? aCursorPos.x : aCursorPos.y;
    Refresh();
}

void RULER_WIDGET::OnPaint( wxPaintEvent& aEvent )
{
    wxAutoBufferedPaintDC dc( this );
    dc.SetBackground( wxBrush( wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOW ) ) );
    dc.Clear();

    wxSize size = GetClientSize();
    dc.SetPen( wxPen( wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOWTEXT ) ) );

    EDA_DRAW_FRAME* frame = dynamic_cast<EDA_DRAW_FRAME*>( GetParent() );
    if( !frame || !frame->GetCanvas() || !frame->GetCanvas()->GetView() )
        return;

    KIGFX::VIEW* view = frame->GetCanvas()->GetView();

    // Get current viewport in world coordinates
    VECTOR2D worldStart, worldEnd;

    int length = m_orientation == ORIENTATION::HORIZONTAL ? size.x : size.y;
    int base = m_orientation == ORIENTATION::HORIZONTAL ? size.y : size.x;

    if( m_orientation == ORIENTATION::HORIZONTAL )
    {
        // Convert screen coordinates to world coordinates
        worldStart = view->ToWorld( VECTOR2D( 0, 0 ) );
        worldEnd = view->ToWorld( VECTOR2D( length, 0 ) );
    }
    else
    {
        // Convert screen coordinates to world coordinates
        worldStart = view->ToWorld( VECTOR2D( 0, 0 ) );
        worldEnd = view->ToWorld( VECTOR2D( 0, length ) );
    }

    // Get grid origin and align ruler to it
    VECTOR2D gridOrigin( frame->GetGridOrigin() );
    double gridOriginCoord = m_orientation == ORIENTATION::HORIZONTAL ?
                            gridOrigin.x : gridOrigin.y;

    // Calculate step sizes in world coordinates (KiCad internal units = nanometers)
    double   stepMinor, stepMedium, stepMajor;

    // Base step sizes depend on units - convert to KiCad internal units (nanometers)
    if( m_units == EDA_UNITS::INCH )
    {
        stepMajor = 25400000.0;  // 1 inch = 25.4mm = 25.4e6 nm
        stepMedium = stepMajor / 2.0;  // 0.5 inch
        stepMinor = stepMajor / 8.0;   // 0.125 inch
    }
    else if( m_units == EDA_UNITS::MILS )
    {
        stepMinor = 25400.0;    // 1 mil = 0.001 inch = 25.4 micrometers = 25400 nm
        stepMedium = 5.0 * stepMinor;   // 5 mils
        stepMajor = 10.0 * stepMinor;   // 10 mils
    }
    else  // mm
    {
        stepMajor = 1000000.0;   // 1 mm = 1e6 nm
        stepMedium = stepMajor / 2.0;  // 0.5 mm
        stepMinor = stepMajor / 10.0;  // 0.1 mm
    }

    // Adjust scale based on zoom level to keep text readable
    double pixelsPerWorldUnit = view->GetScale();
    double stepMajorPixels = stepMajor * pixelsPerWorldUnit;

    // Check if text fits and adjust scale
    wxString testLabel = wxT( "999" );
    wxSize testSize = dc.GetTextExtent( testLabel );
    double requiredSpace = m_orientation == ORIENTATION::HORIZONTAL ? testSize.x : testSize.y;

    // Scale up if text doesn't fit
    while( stepMajorPixels < requiredSpace * 2.0 ) // Need 2x space for readability
    {
        stepMinor *= 10.0;
        stepMedium *= 10.0;
        stepMajor *= 10.0;
        stepMajorPixels = stepMajor * pixelsPerWorldUnit;
    }

    // Scale down if ticks are too sparse
    while( stepMajorPixels > length / 3.0 ) // At least 3 major ticks visible
    {
        stepMinor /= 10.0;
        stepMedium /= 10.0;
        stepMajor /= 10.0;
        stepMajorPixels = stepMajor * pixelsPerWorldUnit;
    }

    // Calculate step sizes in screen coordinates (pixels)
    double stepMinorPixels = stepMinor * pixelsPerWorldUnit;

    // Use major step for very zoomed out views to maintain performance
    double currentStepPixels = stepMinorPixels;
    double currentStepWorld = stepMinor;

    // Find the grid origin in screen coordinates
    VECTOR2D gridOriginScreen;

    if( m_orientation == ORIENTATION::HORIZONTAL )
        gridOriginScreen = view->ToScreen( VECTOR2D( gridOriginCoord, worldStart.y ) );
    else
        gridOriginScreen = view->ToScreen( VECTOR2D( worldStart.x, gridOriginCoord ) );

    double gridOriginScreenCoord = m_orientation == ORIENTATION::HORIZONTAL ?
                                  gridOriginScreen.x : gridOriginScreen.y;

    // Find first tick screen position aligned with grid origin
    double offsetFromGridOriginScreen = 0 - gridOriginScreenCoord;
    double ticksFromOriginScreen = std::floor( offsetFromGridOriginScreen / currentStepPixels );
    double firstTickScreen = gridOriginScreenCoord + ticksFromOriginScreen * currentStepPixels;

    // Make sure we start before the visible area
    if( firstTickScreen > 0 )
        firstTickScreen -= currentStepPixels;

    // Draw ticks and labels by iterating over screen positions
    for( double screenPos = firstTickScreen; screenPos <= length; screenPos += currentStepPixels )
    {
        if( screenPos < 0 )
            continue;

        int p = (int) std::round( screenPos );
        int tickLen = 5;

        // Convert screen position to world position to get the actual coordinate
        VECTOR2D worldPt;

        if( m_orientation == ORIENTATION::HORIZONTAL )
            worldPt = view->ToWorld( VECTOR2D( screenPos, 0 ) );
        else
            worldPt = view->ToWorld( VECTOR2D( 0, screenPos ) );

        double worldPos = m_orientation == ORIENTATION::HORIZONTAL ? worldPt.x : worldPt.y;

        // Calculate world distance from grid origin for tick classification
        double distFromOrigin = std::abs( worldPos - gridOriginCoord );
        bool isMajor, isMedium;


        // When using minor step, classify normally
        isMajor = std::fmod( distFromOrigin, stepMajor ) < stepMinor * 0.5;
        isMedium = std::fmod( distFromOrigin, stepMedium ) < stepMinor * 0.5;

        if( isMajor )
            tickLen = 15;
        else if( isMedium )
            tickLen = 10;

        // Draw tick mark
        if( m_orientation == ORIENTATION::HORIZONTAL )
            dc.DrawLine( p, base, p, base - tickLen );
        else
            dc.DrawLine( base, p, base - tickLen, p );

        // Draw labels for major ticks
        if( isMajor )
        {
            // Calculate value relative to grid origin in internal units
            double valueInInternalUnits = ( worldPos - gridOriginCoord );

            // Convert to display units
            double displayValue;

            if( m_units == EDA_UNITS::INCH )
                displayValue = valueInInternalUnits / 25400000.0;  // nm to inches
            else if( m_units == EDA_UNITS::MILS )
                displayValue = valueInInternalUnits / 25400.0;     // nm to mils
            else // mm
                displayValue = valueInInternalUnits / 1000000.0;   // nm to mm

            wxString label;
            if( std::abs( displayValue ) < 0.001 )
            {
                label = wxT( "0" );
            }
            else if( std::abs( displayValue ) >= 100 )
            {
                label = wxString::Format( wxT( "%.0f" ), displayValue );
            }
            else if( std::abs( displayValue ) >= 1 )
            {
                label = wxString::Format( wxT( "%.1f" ), displayValue );
            }
            else
            {
                label = wxString::Format( wxT( "%.2f" ), displayValue );
            }

            // Remove trailing zeros and decimal point if not needed
            if( label.Contains( "." ) )
            {
                while( label.EndsWith( "0" ) )
                    label.RemoveLast();
                if( label.EndsWith( "." ) )
                    label.RemoveLast();
            }

            wxSize tsize = dc.GetTextExtent( label );

            if( m_orientation == ORIENTATION::HORIZONTAL )
            {
                // Position text beside the tick mark (to the right)
                dc.DrawText( label, p + 3, base - tickLen - tsize.y + 5 );
            }
            else
            {
                // For vertical ruler, rotate text 90 degrees and position beside tick
                dc.SetGraphicsContext( wxGraphicsContext::Create( dc ) );
                if( dc.GetGraphicsContext() )
                {
                    wxGraphicsContext* gc = dc.GetGraphicsContext();
                    gc->PushState();

                    // Move to position and rotate 90 degrees
                    gc->Translate( base - tickLen - tsize.y - 3 + 5, p );
                    gc->Rotate( -1.5708 ); // Rotate -90 degrees (-π/2)

                    // Draw rotated text
                    gc->SetFont( dc.GetFont(), dc.GetTextForeground() );
                    gc->DrawText( label, 0, 0 );

                    gc->PopState();
                }
                else
                {
                    // Fallback if graphics context is not available
                    dc.DrawText( label, base - tickLen - tsize.x - 3, p - tsize.y / 2 );
                }
            }
        }
    }

    if( m_cursorPos >= 0 )
    {
        wxPen greenPen( wxColour( 0, 200, 0 ) );
        dc.SetPen( greenPen );

        if( m_orientation == ORIENTATION::HORIZONTAL )
            dc.DrawLine( m_cursorPos, 0, m_cursorPos, base );
        else
            dc.DrawLine( 0, m_cursorPos, base, m_cursorPos );
    }
}

void RULER_WIDGET::OnSize( wxSizeEvent& aEvent )
{
    // Prevent user resizing by restoring fixed size
    if( m_orientation == ORIENTATION::HORIZONTAL )
    {
        wxSize currentSize = GetSize();
        if( currentSize.y != 30 )
        {
            SetSize( wxSize( currentSize.x, 30 ) );
        }
    }
    else
    {
        wxSize currentSize = GetSize();
        if( currentSize.x != 30 )
        {
            SetSize( wxSize( 30, currentSize.y ) );
        }
    }

    aEvent.Skip();
}

void RULER_WIDGET::OnUnitsChanged( wxCommandEvent& aEvent )
{
    // Get the new units from the parent frame
    EDA_DRAW_FRAME* frame = dynamic_cast<EDA_DRAW_FRAME*>( GetParent() );
    if( frame )
    {
        SetUnits( frame->GetUserUnits() );
    }

    aEvent.Skip();
}

void RULER_WIDGET::updateViewport()
{
    EDA_DRAW_FRAME* frame = dynamic_cast<EDA_DRAW_FRAME*>( GetParent() );
    if( frame && frame->GetCanvas() && frame->GetCanvas()->GetView() )
    {
        KIGFX::VIEW* view = frame->GetCanvas()->GetView();
        BOX2D viewport = view->GetViewport();
        m_viewportOrigin = VECTOR2D( viewport.GetOrigin() );
    }
}

void RULER_WIDGET::bindToParentEvents()
{
    wxWindow* parent = GetParent();
    if( parent )
    {
        // Bind to units changed event
        parent->Bind( EDA_EVT_UNITS_CHANGED, &RULER_WIDGET::OnUnitsChanged, this );

        // For zoom and pan changes, we'll monitor through a timer
        m_zoomTimer.SetOwner( this );
        m_zoomTimer.Start( 50 ); // Check every 50ms for responsive updates

        Bind( wxEVT_TIMER, [this]( wxTimerEvent& )
        {
            EDA_DRAW_FRAME* frame = dynamic_cast<EDA_DRAW_FRAME*>( GetParent() );
            if( frame && frame->GetCanvas() )
            {
                double currentZoom = frame->GetCanvas()->GetGAL()->GetZoomFactor();

                // Get current viewport
                KIGFX::VIEW* view = frame->GetCanvas()->GetView();
                BOX2D viewport = view->GetViewport();
                VECTOR2D currentViewportOrigin = viewport.GetOrigin();

                // Check if zoom or viewport changed
                bool zoomChanged = std::abs( currentZoom - m_lastZoom ) > 1e-6;
                bool viewportChanged = ( currentViewportOrigin - m_lastViewportOrigin ).EuclideanNorm() > 1.0;

                if( zoomChanged )
                {
                    SetPixelsPerUnit( currentZoom );
                    m_lastZoom = currentZoom;
                }

                if( viewportChanged )
                {
                    m_lastViewportOrigin = currentViewportOrigin;
                    Refresh(); // Redraw ruler with new viewport
                }
            }
        } );

        // Initialize viewport tracking
        updateViewport();
        m_lastViewportOrigin = m_viewportOrigin;
    }
}

void RULER_WIDGET::unbindFromParentEvents()
{
    wxWindow* parent = GetParent();
    if( parent )
    {
        parent->Unbind( EDA_EVT_UNITS_CHANGED, &RULER_WIDGET::OnUnitsChanged, this );
        m_zoomTimer.Stop();
        // Note: Timer events are automatically unbound when the timer stops
    }
}
