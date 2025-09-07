/*
 * Shared helper for computing multi-line stacked pin number layout.
 * Extracted from sch_painter so tests can exercise identical logic.
 */

#pragma once

#include <wx/string.h>
#include <wx/arrstr.h>
#include <math/vector2d.h>
#include <font/text_attributes.h>

struct MULTILINE_PIN_TEXT_LAYOUT
{
    bool        m_IsMultiLine = false;     // true if brace-wrapped multi-line stacked list
    wxArrayString m_Lines;                 // individual numbered lines (trimmed)
    VECTOR2D    m_StartPos;                // position used for line index 0 after alignment shift
    int         m_LineSpacing = 0;         // inter-line spacing in IU (along secondary axis)
};

// Compute layout for a (possibly) multi-line stacked pin number string.  If not multi-line, the
// returned layout has m_IsMultiLine=false and no further adjustments are required.
MULTILINE_PIN_TEXT_LAYOUT ComputeMultiLinePinNumberLayout( const wxString& aText,
        const VECTOR2D& aAnchorPos, const TEXT_ATTRIBUTES& aAttrs );
