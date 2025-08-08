/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2015 CERN
 * Copyright The KiCad Developers, see AUTHORS.txt for contributors.
 * @author Tomasz Wlostowski <tomasz.wlostowski@cern.ch>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include <geometry/shape_line_chain.h>
#include <geometry/shape_line_chain_base.h>
#include <geometry/shape_poly_set.h>
#include <geometry/circle.h>
#include <convert_basic_shapes_to_polygon.h>
#include <trigo.h>
#include <math/util.h>
#include <limits>
#include <sstream>
#include <algorithm>

// Constructors
SHAPE_LINE_CHAIN::SHAPE_LINE_CHAIN(const std::vector<int>& aV) :
    SHAPE_LINE_CHAIN_BASE(SH_LINE_CHAIN), m_hasFirstPoint(false), m_closed(false), m_width(0)
{
    for (size_t i = 0; i < aV.size(); i += 2)
    {
        Append(aV[i], aV[i + 1]);
    }
}

SHAPE_LINE_CHAIN::SHAPE_LINE_CHAIN(const std::vector<VECTOR2I>& aV, bool aClosed) :
    SHAPE_LINE_CHAIN_BASE(SH_LINE_CHAIN), m_hasFirstPoint(false), m_closed(false), m_width(0)
{
    for (const auto& pt : aV)
    {
        Append(pt);
    }
    SetClosed(aClosed);
}

SHAPE_LINE_CHAIN::SHAPE_LINE_CHAIN(const SHAPE_ARC& aArc, bool aClosed, std::optional<int> aMaxError) :
    SHAPE_LINE_CHAIN_BASE(SH_LINE_CHAIN), m_hasFirstPoint(false), m_closed(false), m_width(aArc.GetWidth())
{
    if (aMaxError.has_value())
        Append(aArc, aMaxError.value());
    else
        Append(aArc);
    SetClosed(aClosed);
}

// Legacy Clipper2 constructor - converts old format to new
SHAPE_LINE_CHAIN::SHAPE_LINE_CHAIN(const Clipper2Lib::Path64& aPath,
                                    const std::vector<CLIPPER_Z_VALUE>& aZValueBuffer,
                                    const std::vector<SHAPE_ARC>& aArcBuffer) :
    SHAPE_LINE_CHAIN_BASE(SH_LINE_CHAIN), m_closed(true), m_width(0)
{
    // Convert from old complex format to new simple format
    // This is a compatibility layer during transition
    for (size_t i = 0; i < aPath.size(); ++i)
    {
        VECTOR2I pt(aPath[i].x, aPath[i].y);

        // Check if this point is associated with an arc
        int zIdx = aPath[i].z;
        bool isArcPoint = false;

        if (zIdx >= 0 && zIdx < static_cast<int>(aZValueBuffer.size()))
        {
            const CLIPPER_Z_VALUE& zValue = aZValueBuffer[zIdx];
            if (zValue.m_FirstArcIdx >= 0 && zValue.m_FirstArcIdx < static_cast<ssize_t>(aArcBuffer.size()))
            {
                // This point is part of an arc - we'll handle arcs separately
                isArcPoint = true;
            }
        }

        if (!isArcPoint)
        {
            Append(pt);
        }
    }

    // Add arcs where appropriate (simplified for compatibility)
    // In practice, a full migration would convert the old representation completely
}

SHAPE* SHAPE_LINE_CHAIN::Clone() const
{
    return new SHAPE_LINE_CHAIN(*this);
}

void SHAPE_LINE_CHAIN::Clear()
{
    m_segments.clear();
    m_hasFirstPoint = false;
    m_closed = false;
    m_bbox = BOX2I();
}

void SHAPE_LINE_CHAIN::SetClosed(bool aClosed)
{
    if (m_closed == aClosed)
        return;

    m_closed = aClosed;

    if (m_closed && m_segments.size() > 0)
    {
        // If closing, check if we need to add a segment from last to first point
        VECTOR2I firstPt = m_segments.front().GetStart();
        VECTOR2I lastPt = m_segments.back().GetEnd();

        if (firstPt != lastPt)
        {
            m_segments.emplace_back(lastPt, firstPt);
        }
    }
    else if (!m_closed && m_segments.size() > 1)
    {
        // If opening, check if last segment connects back to first point
        VECTOR2I firstPt = m_segments.front().GetStart();
        VECTOR2I lastEnd = m_segments.back().GetEnd();

        if (firstPt == lastEnd)
        {
            m_segments.pop_back();
        }
    }
}


const CHAIN_SEGMENT& SHAPE_LINE_CHAIN::GetSegment(int aIndex) const
{
    if (aIndex < 0)
        aIndex += SegmentCount();

    if (aIndex < 0 || aIndex >= SegmentCount())
        throw std::out_of_range("Segment index out of range");

    return m_segments[aIndex];
}

CHAIN_SEGMENT& SHAPE_LINE_CHAIN::GetSegment(int aIndex)
{
    if (aIndex < 0)
        aIndex += SegmentCount();

    if (aIndex < 0 || aIndex >= SegmentCount())
        throw std::out_of_range("Segment index out of range");

    return m_segments[aIndex];
}

const SEG SHAPE_LINE_CHAIN::CSegment(int aIndex) const
{
    const CHAIN_SEGMENT& segment = GetSegment(aIndex);
    if (segment.IsLine())
        return segment.AsLine();
    else
    {
        // For arcs, return a line from start to end (approximation)
        return SEG(segment.GetStart(), segment.GetEnd());
    }
}


const std::vector<VECTOR2I> SHAPE_LINE_CHAIN::CPoints() const
{
    std::vector<VECTOR2I> points;
    points.reserve(PointCount());

    for (int i = 0; i < PointCount(); ++i)
    {
        points.push_back(GetPoint(i));
    }

    return points;
}

const BOX2I SHAPE_LINE_CHAIN::BBox(int aClearance) const
{
    if (m_segments.empty())
        return BOX2I();

    BOX2I bbox = m_segments[0].BBox();

    for (size_t i = 1; i < m_segments.size(); ++i)
    {
        bbox.Merge(m_segments[i].BBox());
    }

    if (aClearance != 0 || m_width != 0)
        bbox.Inflate(aClearance + m_width / 2);

    return bbox;
}

void SHAPE_LINE_CHAIN::GenerateBBoxCache() const
{
    m_bbox = BBox(0);
}

long long int SHAPE_LINE_CHAIN::Length() const
{
    long long int total = 0;

    for (const auto& segment : m_segments)
    {
        total += static_cast<long long int>(segment.GetLength());
    }

    return total;
}

bool SHAPE_LINE_CHAIN::Collide(const VECTOR2I& aP, int aClearance, int* aActual, VECTOR2I* aLocation) const
{
    if (IsClosed() && PointInside(aP, aClearance))
    {
        if (aLocation)
            *aLocation = aP;
        if (aActual)
            *aActual = 0;
        return true;
    }

    int minDist = std::numeric_limits<int>::max();
    VECTOR2I nearestLocation;
    bool collision = false;

    for (const auto& segment : m_segments)
    {
        int dist;
        VECTOR2I location;

        if (segment.Collide(aP, aClearance, &dist, &location))
        {
            collision = true;
            if (dist < minDist)
            {
                minDist = dist;
                nearestLocation = location;
            }
            if (dist == 0)
                break;
        }
    }

    if (collision)
    {
        if (aLocation)
            *aLocation = nearestLocation;
        if (aActual)
            *aActual = minDist;
    }

    return collision;
}

bool SHAPE_LINE_CHAIN::Collide(const SEG& aSeg, int aClearance, int* aActual, VECTOR2I* aLocation) const
{
    if (IsClosed() && PointInside(aSeg.A))
    {
        if (aLocation)
            *aLocation = aSeg.A;
        if (aActual)
            *aActual = 0;
        return true;
    }

    int minDist = std::numeric_limits<int>::max();
    VECTOR2I nearestLocation;
    bool collision = false;

    for (const auto& segment : m_segments)
    {
        int dist;
        VECTOR2I location;

        if (segment.Collide(aSeg, aClearance, &dist, &location))
        {
            collision = true;
            if (dist < minDist)
            {
                minDist = dist;
                nearestLocation = location;
            }
            if (dist == 0)
                break;
        }
    }

    if (collision)
    {
        if (aLocation)
            *aLocation = nearestLocation;
        if (aActual)
            *aActual = minDist;
    }

    return collision;
}

void SHAPE_LINE_CHAIN::Append(int aX, int aY, bool aAllowDuplication)
{
    Append(VECTOR2I(aX, aY), aAllowDuplication);
}

void SHAPE_LINE_CHAIN::Append(const VECTOR2I& aP, bool aAllowDuplication)
{
    if (!m_hasFirstPoint)
    {
        // First point - store it but don't create a segment yet
        m_firstPoint = aP;
        m_hasFirstPoint = true;
        m_bbox = BOX2I(aP, VECTOR2I(0, 0));
        return;
    }

    // Get the last point in the chain
    VECTOR2I lastPoint = m_segments.empty() ? m_firstPoint : m_segments.back().GetEnd();

    // Skip duplicate points unless explicitly allowed
    if (!aAllowDuplication && lastPoint == aP)
        return;

    // Create a new line segment
    CHAIN_SEGMENT newSegment(lastPoint, aP);
    m_segments.push_back(newSegment);
    UpdateBBoxWithSegment(newSegment);
}

void SHAPE_LINE_CHAIN::Append(const SHAPE_LINE_CHAIN& aOtherLine)
{
    if (aOtherLine.SegmentCount() == 0)
        return;

    // Handle case where we only have a first point
    if (!m_hasFirstPoint && m_segments.empty())
    {
        // Copy all segments from other chain
        for (const auto& segment : aOtherLine.m_segments)
        {
            m_segments.push_back(segment);
        }
        m_hasFirstPoint = true;
        m_firstPoint = aOtherLine.GetPoint(0);
        RecalculateBBox();
        return;
    }

    // Ensure continuity between chains
    VECTOR2I ourEnd = m_segments.empty() ? m_firstPoint : m_segments.back().GetEnd();
    VECTOR2I theirStart = aOtherLine.GetPoint(0);

    if (ourEnd != theirStart)
    {
        // Add connecting segment
        m_segments.emplace_back(ourEnd, theirStart);
    }

    // Copy all segments from other chain
    for (const auto& segment : aOtherLine.m_segments)
    {
        m_segments.push_back(segment);
    }

    RecalculateBBox();
}

void SHAPE_LINE_CHAIN::Append(const SHAPE_ARC& aArc)
{
    if (!m_hasFirstPoint && m_segments.empty())
    {
        // First shape is an arc - use arc start as first point
        m_firstPoint = aArc.GetP0();
        m_hasFirstPoint = true;
    }

    // Ensure continuity
    VECTOR2I ourEnd = m_segments.empty() ? m_firstPoint : m_segments.back().GetEnd();
    if (ourEnd != aArc.GetP0())
    {
        // Add connecting line segment
        m_segments.emplace_back(ourEnd, aArc.GetP0());
    }

    CHAIN_SEGMENT arcSegment(aArc);
    arcSegment.SetWidth(m_width);
    m_segments.push_back(arcSegment);
    UpdateBBoxWithSegment(arcSegment);
}

void SHAPE_LINE_CHAIN::Append(const SHAPE_ARC& aArc, int aMaxError)
{
    // For now, just use the arc directly
    // In a full implementation, you might want to handle maxError differently
    Append(aArc);
}

const VECTOR2I SHAPE_LINE_CHAIN::NearestPoint(const VECTOR2I& aP, bool aAllowInternalShapePoints) const
{
    if (m_segments.empty())
        return VECTOR2I(0, 0);

    int minDist = std::numeric_limits<int>::max();
    VECTOR2I nearest;

    for (const auto& segment : m_segments)
    {
        VECTOR2I candidate = segment.NearestPoint(aP);
        int dist = (candidate - aP).EuclideanNorm();

        if (dist < minDist)
        {
            minDist = dist;
            nearest = candidate;
        }
    }

    return nearest;
}

int SHAPE_LINE_CHAIN::NearestSegment(const VECTOR2I& aP) const
{
    if (m_segments.empty())
        return -1;

    int minDist = std::numeric_limits<int>::max();
    int nearestIndex = 0;

    for (size_t i = 0; i < m_segments.size(); ++i)
    {
        int dist = m_segments[i].Distance(aP);
        if (dist < minDist)
        {
            minDist = dist;
            nearestIndex = static_cast<int>(i);
        }
    }

    return nearestIndex;
}

int SHAPE_LINE_CHAIN::FindSegment(const VECTOR2I& aP, int aThreshold) const
{
    for (size_t i = 0; i < m_segments.size(); ++i)
    {
        if (m_segments[i].Distance(aP) <= aThreshold)
            return static_cast<int>(i);
    }
    return -1;
}

int SHAPE_LINE_CHAIN::Find(const VECTOR2I& aP, int aThreshold) const
{
    for (int i = 0; i < PointCount(); ++i)
    {
        VECTOR2I pt = GetPoint(i);
        if (aThreshold == 0)
        {
            if (pt == aP)
                return i;
        }
        else
        {
            if ((pt - aP).EuclideanNorm() <= aThreshold)
                return i;
        }
    }
    return -1;
}

const SHAPE_LINE_CHAIN SHAPE_LINE_CHAIN::Reverse() const
{
    SHAPE_LINE_CHAIN result;
    result.m_closed = m_closed;
    result.m_width = m_width;

    // Reverse segments and their directions
    for (auto it = m_segments.rbegin(); it != m_segments.rend(); ++it)
    {
        result.m_segments.push_back(it->Reversed());
    }

    result.RecalculateBBox();
    return result;
}

void SHAPE_LINE_CHAIN::Move(const VECTOR2I& aVector)
{
    for (auto& segment : m_segments)
    {
        segment.Move(aVector);
    }
    m_bbox.Move(aVector);
}

void SHAPE_LINE_CHAIN::Rotate(const EDA_ANGLE& aAngle, const VECTOR2I& aCenter)
{
    for (auto& segment : m_segments)
    {
        segment.Rotate(aAngle, aCenter);
    }
    RecalculateBBox();
}

void SHAPE_LINE_CHAIN::Mirror(const VECTOR2I& aRef, FLIP_DIRECTION aFlipDirection)
{
    for (auto& segment : m_segments)
    {
        segment.Mirror(aRef, aFlipDirection);
    }
    RecalculateBBox();
}

void SHAPE_LINE_CHAIN::Mirror(const SEG& axis)
{
    for (auto& segment : m_segments)
    {
        segment.Mirror(axis);
    }
    RecalculateBBox();
}


void SHAPE_LINE_CHAIN::ClearArcs(int aMaxError)
{
    if (aMaxError <= 0)
        aMaxError = 10;  // Default reasonable approximation

    // Process segments from back to front to avoid index shifts during insertion
    for (int i = static_cast<int>(m_segments.size()) - 1; i >= 0; --i)
    {
        if (m_segments[i].IsArc())
        {
            ConvertArcToLines(i, aMaxError);
        }
    }

    // Verify no arcs remain
    assert(!HasArcs());

    // Update bounding box after all conversions
    RecalculateBBox();
}

int SHAPE_LINE_CHAIN::ConvertArcToLines(int aSegmentIndex, int aMaxError)
{
    // Bounds checking
    if (aSegmentIndex < 0 || aSegmentIndex >= SegmentCount())
        return -1;

    CHAIN_SEGMENT& segment = m_segments[aSegmentIndex];

    // Only convert arc segments
    if (!segment.IsArc())
        return 0;  // Nothing to convert

    const SHAPE_ARC& arc = segment.AsArc();

    // Convert arc to line segments
    std::vector<CHAIN_SEGMENT> lineSegments = ConvertArcToLineSegments(arc, aMaxError);

    if (lineSegments.empty())
        return -1;  // Conversion failed

    // Replace the arc segment with line segments
    // First, replace the current segment with the first line segment
    m_segments[aSegmentIndex] = lineSegments[0];

    // Then insert the remaining line segments
    if (lineSegments.size() > 1)
    {
        m_segments.insert(m_segments.begin() + aSegmentIndex + 1,
                         lineSegments.begin() + 1, lineSegments.end());
    }

    // Ensure continuity is maintained
    UpdateSegmentContinuity(aSegmentIndex);
    if (aSegmentIndex + lineSegments.size() < static_cast<size_t>(SegmentCount()))
    {
        UpdateSegmentContinuity(aSegmentIndex + lineSegments.size());
    }

    return static_cast<int>(lineSegments.size());
}

std::vector<CHAIN_SEGMENT> SHAPE_LINE_CHAIN::ConvertArcToLineSegments(const SHAPE_ARC& aArc, int aMaxError)
{
    std::vector<CHAIN_SEGMENT> result;

    try
    {
        // Get the polyline approximation of the arc
        auto arcPoints = aArc.ConvertToPolyline(aMaxError);

        if (arcPoints.PointCount() < 2)
        {
            // Degenerate arc - create a single line segment
            result.emplace_back(aArc.GetP0(), aArc.GetP1());
            return result;
        }

        // Create line segments from consecutive points
        for (int i = 0; i < arcPoints.PointCount() - 1; ++i)
        {
            VECTOR2I start = arcPoints.CPoint(i);
            VECTOR2I end = arcPoints.CPoint(i + 1);

            // Skip zero-length segments
            if (start == end)
                continue;

            CHAIN_SEGMENT lineSegment(start, end);

            // Preserve width from original arc
            lineSegment.SetWidth(aArc.GetWidth());

            result.push_back(lineSegment);
        }

        // Ensure we have at least one segment
        if (result.empty())
        {
            result.emplace_back(aArc.GetP0(), aArc.GetP1());
            result.back().SetWidth(aArc.GetWidth());
        }
    }
    catch (...)
    {
        // Fallback: create a single line segment from arc endpoints
        result.emplace_back(aArc.GetP0(), aArc.GetP1());
        result.back().SetWidth(aArc.GetWidth());
    }

    return result;
}

size_t SHAPE_LINE_CHAIN::ArcCount() const
{
    return std::count_if(m_segments.begin(), m_segments.end(),
                        [](const CHAIN_SEGMENT& seg) { return seg.IsArc(); });
}

bool SHAPE_LINE_CHAIN::HasArcs() const
{
    return std::any_of(m_segments.begin(), m_segments.end(),
                      [](const CHAIN_SEGMENT& seg) { return seg.IsArc(); });
}

void SHAPE_LINE_CHAIN::TransformToPolygon(SHAPE_POLY_SET& aBuffer, int aError, ERROR_LOC aErrorLoc) const
{
    std::vector<VECTOR2I> points;

    if (m_segments.empty())
        return;

    // Convert all segments to points
    points.push_back(m_segments[0].GetStart());

    for (const auto& segment : m_segments)
    {
        if (segment.IsArc())
        {
            // Get arc approximation points (excluding start point)
            auto arcPoints = segment.GetApproximatePoints(aError);
            for (size_t i = 1; i < arcPoints.size(); ++i)
            {
                points.push_back(arcPoints[i]);
            }
        }
        else
        {
            points.push_back(segment.GetEnd());
        }
    }

    SHAPE_LINE_CHAIN chain(points, m_closed);
    aBuffer.AddOutline(chain);
}


const SHAPE_ARC& SHAPE_LINE_CHAIN::Arc(size_t aArc) const
{
    size_t arcsSeen = 0;
    for (const auto& segment : m_segments)
    {
        if (segment.IsArc())
        {
            if (arcsSeen == aArc)
                return segment.AsArc();
            arcsSeen++;
        }
    }
    throw std::out_of_range("Arc index out of range");
}

// Helper methods
void SHAPE_LINE_CHAIN::RecalculateBBox() const
{
    if (!m_hasFirstPoint && m_segments.empty())
    {
        m_bbox = BOX2I();
        return;
    }

    if (m_hasFirstPoint && m_segments.empty())
    {
        m_bbox = BOX2I(m_firstPoint, VECTOR2I(0, 0));
        return;
    }

    m_bbox = m_segments[0].BBox();
    for (size_t i = 1; i < m_segments.size(); ++i)
    {
        m_bbox.Merge(m_segments[i].BBox());
    }

    if (m_width != 0)
        m_bbox.Inflate(m_width / 2);
}

void SHAPE_LINE_CHAIN::UpdateBBoxWithSegment(const CHAIN_SEGMENT& aSegment) const
{
    BOX2I segBBox = aSegment.BBox();

    if (!m_hasFirstPoint && m_segments.size() <= 1)
        m_bbox = segBBox;
    else
        m_bbox.Merge(segBBox);

    if (m_width != 0)
        m_bbox.Inflate(m_width / 2);
}

bool SHAPE_LINE_CHAIN::operator!=(const SHAPE_LINE_CHAIN& aRhs) const
{
    if (PointCount() != aRhs.PointCount())
        return true;

    if (SegmentCount() != aRhs.SegmentCount())
        return true;

    if (m_closed != aRhs.m_closed)
        return true;

    for (int i = 0; i < SegmentCount(); ++i)
    {
        if (m_segments[i] != aRhs.m_segments[i])
            return true;
    }

    return false;
}

bool SHAPE_LINE_CHAIN::CompareGeometry(const SHAPE_LINE_CHAIN& aOther) const
{
    // Create copies and simplify for comparison
    SHAPE_LINE_CHAIN a = *this;
    SHAPE_LINE_CHAIN b = aOther;

    // For geometry comparison, we need to compare the actual point sequences
    auto aPoints = a.CPoints();
    auto bPoints = b.CPoints();

    if (aPoints.size() != bPoints.size())
        return false;

    for (size_t i = 0; i < aPoints.size(); ++i)
    {
        if (aPoints[i] != bPoints[i])
            return false;
    }

    return true;
}

bool SHAPE_LINE_CHAIN::ClosestPoints(const SHAPE_LINE_CHAIN& aOther, VECTOR2I& aPt0, VECTOR2I& aPt1) const
{
    if (m_segments.empty() || aOther.m_segments.empty())
        return false;

    int64_t minDistSq = std::numeric_limits<int64_t>::max();
    bool found = false;

    for (const auto& ourSeg : m_segments)
    {
        for (const auto& theirSeg : aOther.m_segments)
        {
            VECTOR2I pt1 = ourSeg.NearestPoint(theirSeg.GetStart());
            VECTOR2I pt2 = theirSeg.NearestPoint(pt1);

            int64_t distSq = (pt1 - pt2).SquaredEuclideanNorm();

            if (distSq < minDistSq)
            {
                minDistSq = distSq;
                aPt0 = pt1;
                aPt1 = pt2;
                found = true;
            }
        }
    }

    return found;
}

void SHAPE_LINE_CHAIN::Remove(int aIndex)
{
    Remove(aIndex, aIndex);
}

void SHAPE_LINE_CHAIN::Remove(int aStartIndex, int aEndIndex)
{
    if (aStartIndex < 0) aStartIndex += SegmentCount();
    if (aEndIndex < 0) aEndIndex += SegmentCount();

    if (aStartIndex > aEndIndex || aStartIndex < 0 || aEndIndex >= SegmentCount())
        return;

    m_segments.erase(m_segments.begin() + aStartIndex,
                    m_segments.begin() + aEndIndex + 1);

    RecalculateBBox();
}

void SHAPE_LINE_CHAIN::RemoveShape(int aPointIndex)
{
    // For the new design, removing a "shape" means removing a segment
    // Convert point index to segment index
    if (aPointIndex <= 0 || aPointIndex > SegmentCount())
        return;

    Remove(aPointIndex - 1);
}

void SHAPE_LINE_CHAIN::SetPoint(int aIndex, const VECTOR2I& aPos)
{
    if (aIndex < 0)
        aIndex += PointCount();

    if (aIndex < 0 || aIndex >= PointCount())
        return;

    if (aIndex == 0)
    {
        // Changing first point
        if (m_segments.empty())
        {
            m_firstPoint = aPos;
        }
        else
        {
            m_segments[0].SetStart(aPos);
            // Also update last segment if closed
            if (m_closed && m_segments.size() > 1)
            {
                m_segments.back().SetEnd(aPos);
            }
        }
    }
    else if (aIndex == PointCount() - 1 && !m_closed)
    {
        // Changing last point of open chain
        if (!m_segments.empty())
        {
            m_segments.back().SetEnd(aPos);
        }
    }
    else
    {
        // Changing intermediate point - affects two segments
        int segIndex = aIndex - 1;
        if (segIndex >= 0 && segIndex < SegmentCount())
        {
            m_segments[segIndex].SetEnd(aPos);
        }
        if (segIndex + 1 < SegmentCount())
        {
            m_segments[segIndex + 1].SetStart(aPos);
        }
    }

    RecalculateBBox();
}

const SHAPE_LINE_CHAIN SHAPE_LINE_CHAIN::Slice(int aStartIndex, int aEndIndex) const
{
    if (aStartIndex < 0) aStartIndex += SegmentCount();
    if (aEndIndex < 0) aEndIndex += SegmentCount();

    SHAPE_LINE_CHAIN result;
    result.m_width = m_width;
    result.m_closed = false;  // Sliced chains are typically not closed

    if (aStartIndex <= aEndIndex && aStartIndex >= 0 && aEndIndex < SegmentCount())
    {
        for (int i = aStartIndex; i <= aEndIndex; ++i)
        {
            result.m_segments.push_back(m_segments[i]);
        }

        if (!result.m_segments.empty())
        {
            result.m_hasFirstPoint = true;
            result.m_firstPoint = result.m_segments[0].GetStart();
        }

        result.RecalculateBBox();
    }

    return result;
}

int SHAPE_LINE_CHAIN::Split(const VECTOR2I& aP, bool aExact)
{
    int segIndex = FindSegment(aP, aExact ? 0 : 2);
    if (segIndex < 0)
        return -1;

    CHAIN_SEGMENT& segment = m_segments[segIndex];

    // Don't split if point is at segment endpoints
    if (segment.GetStart() == aP || segment.GetEnd() == aP)
        return segIndex;

    if (segment.IsLine())
    {
        SEG& line = segment.AsLine();

        // Create two new segments
        CHAIN_SEGMENT seg1(line.A, aP);
        CHAIN_SEGMENT seg2(aP, line.B);

        // Replace original with first new segment
        m_segments[segIndex] = seg1;

        // Insert second new segment
        m_segments.insert(m_segments.begin() + segIndex + 1, seg2);
    }
    else
    {
        // For arcs, splitting is more complex - simplified implementation
        const SHAPE_ARC& arc = segment.AsArc();

        // Convert to line segments at split point (simplified)
        CHAIN_SEGMENT seg1(arc.GetP0(), aP);
        CHAIN_SEGMENT seg2(aP, arc.GetP1());

        m_segments[segIndex] = seg1;
        m_segments.insert(m_segments.begin() + segIndex + 1, seg2);
    }

    RecalculateBBox();
    return segIndex + 1;
}

void SHAPE_LINE_CHAIN::Insert(size_t aVertex, const VECTOR2I& aP)
{
    if (aVertex > static_cast<size_t>(SegmentCount()))
        aVertex = static_cast<size_t>(SegmentCount());

    if (aVertex == 0 && m_segments.empty())
    {
        // Insert as first point
        Append(aP);
        return;
    }

    // Get connection points
    VECTOR2I prevPt = (aVertex == 0) ? GetPoint(0) : m_segments[aVertex - 1].GetEnd();
    VECTOR2I nextPt = (aVertex >= static_cast<size_t>(SegmentCount())) ?
                      GetPoint(-1) : m_segments[aVertex].GetStart();

    // Create new segments
    CHAIN_SEGMENT newSeg1(prevPt, aP);
    CHAIN_SEGMENT newSeg2(aP, nextPt);

    if (aVertex < static_cast<size_t>(SegmentCount()))
    {
        // Replace existing segment with two new ones
        m_segments[aVertex] = newSeg2;
        m_segments.insert(m_segments.begin() + aVertex, newSeg1);
    }
    else
    {
        // Append new segment
        m_segments.push_back(newSeg1);
    }

    RecalculateBBox();
}

void SHAPE_LINE_CHAIN::Simplify(int aTolerance)
{
    if (m_segments.size() < 2)
        return;

    std::vector<CHAIN_SEGMENT> simplified;
    simplified.reserve(m_segments.size());

    // Simple implementation - remove segments that are too short or nearly collinear
    for (size_t i = 0; i < m_segments.size(); ++i)
    {
        const CHAIN_SEGMENT& seg = m_segments[i];

        // Always keep arc segments
        if (seg.IsArc())
        {
            simplified.push_back(seg);
            continue;
        }

        // For line segments, check length and collinearity
        if (seg.GetLength() < aTolerance)
            continue;  // Skip very short segments

        // Check collinearity with next segment
        if (i + 1 < m_segments.size() && m_segments[i + 1].IsLine())
        {
            const SEG& line1 = seg.AsLine();
            const SEG& line2 = m_segments[i + 1].AsLine();

            if (line1.Collinear(line2))
            {
                // Merge the two segments
                CHAIN_SEGMENT merged(line1.A, line2.B);
                simplified.push_back(merged);
                i++;  // Skip next segment as it's been merged
                continue;
            }
        }

        simplified.push_back(seg);
    }

    m_segments = std::move(simplified);
    RecalculateBBox();
}

void SHAPE_LINE_CHAIN::RemoveDuplicatePoints()
{
    if (m_segments.empty())
        return;

    std::vector<CHAIN_SEGMENT> cleaned;
    cleaned.reserve(m_segments.size());

    for (const auto& segment : m_segments)
    {
        // Skip zero-length line segments
        if (segment.IsLine() && segment.GetStart() == segment.GetEnd())
            continue;

        cleaned.push_back(segment);
    }

    m_segments = std::move(cleaned);
    RecalculateBBox();
}

// Additional missing method implementations
double SHAPE_LINE_CHAIN::Area(bool aAbsolute) const
{
    if (!m_closed || m_segments.empty())
        return 0.0;

    // Use shoelace formula for polygon area
    // Convert segments to points first
    std::vector<VECTOR2I> points = CPoints();

    double area = 0.0;
    int size = static_cast<int>(points.size());

    for (int i = 0, j = size - 1; i < size; ++i)
    {
        area += (static_cast<double>(points[j].x) + points[i].x) *
                (static_cast<double>(points[j].y) - points[i].y);
        j = i;
    }

    if (aAbsolute)
        return std::abs(area * 0.5);
    else
        return -area * 0.5;
}

const std::string SHAPE_LINE_CHAIN::Format(bool aCplusPlus) const
{
    std::stringstream ss;

    if (aCplusPlus)
    {
        ss << "SHAPE_LINE_CHAIN( { ";

        for (int i = 0; i < PointCount(); i++)
        {
            VECTOR2I pt = GetPoint(i);
            ss << "VECTOR2I( " << pt.x << ", " << pt.y << " )";

            if (i != PointCount() - 1)
                ss << ", ";
        }

        ss << " }, " << (m_closed ? "true" : "false") << " );";
    }
    else
    {
        // Simple format for debugging
        ss << "CHAIN[" << SegmentCount() << " segments, " << PointCount() << " points";
        if (m_closed) ss << ", closed";
        ss << "]";
    }

    return ss.str();
}

bool SHAPE_LINE_CHAIN::Parse(std::stringstream& aStream)
{
    // Basic parsing implementation
    Clear();

    int numPoints;
    bool closed;

    aStream >> numPoints >> closed;

    if (numPoints < 0 || numPoints > 100000)  // Sanity check
        return false;

    for (int i = 0; i < numPoints; ++i)
    {
        int x, y;
        aStream >> x >> y;
        Append(VECTOR2I(x, y));
    }

    SetClosed(closed);
    return true;
}

bool SHAPE_LINE_CHAIN::Intersects(const SEG& aSeg) const
{
    for (const auto& segment : m_segments)
    {
        auto intersections = segment.Intersect(aSeg);
        if (!intersections.empty())
            return true;
    }
    return false;
}

bool SHAPE_LINE_CHAIN::Intersects(const SHAPE_LINE_CHAIN& aChain) const
{
    for (const auto& ourSeg : m_segments)
    {
        for (const auto& theirSeg : aChain.m_segments)
        {
            auto intersections = ourSeg.Intersect(theirSeg);
            if (!intersections.empty())
                return true;
        }
    }
    return false;
}

int SHAPE_LINE_CHAIN::Intersect(const SEG& aSeg, INTERSECTIONS& aIp) const
{
    aIp.clear();

    for (int i = 0; i < SegmentCount(); ++i)
    {
        auto intersections = m_segments[i].Intersect(aSeg);

        for (const auto& pt : intersections)
        {
            INTERSECTION is;
            is.p = pt;
            is.index_our = i;
            is.index_their = -1;
            is.valid = true;
            is.is_corner_our = (pt == m_segments[i].GetStart() || pt == m_segments[i].GetEnd());
            is.is_corner_their = (pt == aSeg.A || pt == aSeg.B);

            aIp.push_back(is);
        }
    }

    return static_cast<int>(aIp.size());
}

int SHAPE_LINE_CHAIN::Intersect(const SHAPE_LINE_CHAIN& aChain, INTERSECTIONS& aIp,
                                bool aExcludeColinearAndTouching, BOX2I* aChainBBox) const
{
    aIp.clear();

    BOX2I otherBBox = aChainBBox ? *aChainBBox : aChain.BBox();

    for (int i = 0; i < SegmentCount(); ++i)
    {
        const CHAIN_SEGMENT& ourSeg = m_segments[i];
        BOX2I ourBBox = ourSeg.BBox();

        if (!otherBBox.Intersects(ourBBox))
            continue;

        for (int j = 0; j < aChain.SegmentCount(); ++j)
        {
            const CHAIN_SEGMENT& theirSeg = aChain.m_segments[j];

            auto intersections = ourSeg.Intersect(theirSeg);

            for (const auto& pt : intersections)
            {
                INTERSECTION is;
                is.p = pt;
                is.index_our = i;
                is.index_their = j;
                is.valid = true;
                is.is_corner_our = (pt == ourSeg.GetStart() || pt == ourSeg.GetEnd());
                is.is_corner_their = (pt == theirSeg.GetStart() || pt == theirSeg.GetEnd());

                // Skip collinear/touching intersections if requested
                if (aExcludeColinearAndTouching && (is.is_corner_our || is.is_corner_their))
                    continue;

                aIp.push_back(is);
            }
        }
    }

    return static_cast<int>(aIp.size());
}

const std::optional<SHAPE_LINE_CHAIN::INTERSECTION> SHAPE_LINE_CHAIN::SelfIntersecting() const
{
    for (int i = 0; i < SegmentCount(); ++i)
    {
        for (int j = i + 2; j < SegmentCount(); ++j)
        {
            // Skip adjacent segments and for closed chains, skip first-last
            if (j == i + 1 || (m_closed && i == 0 && j == SegmentCount() - 1))
                continue;

            const CHAIN_SEGMENT& seg1 = m_segments[i];
            const CHAIN_SEGMENT& seg2 = m_segments[j];

            auto intersections = seg1.Intersect(seg2);

            if (!intersections.empty())
            {
                INTERSECTION is;
                is.p = intersections[0];  // Take first intersection
                is.index_our = i;
                is.index_their = j;
                is.valid = true;
                is.is_corner_our = false;
                is.is_corner_their = false;

                return is;
            }
        }
    }

    return std::nullopt;
}

bool SHAPE_LINE_CHAIN::CheckClearance(const VECTOR2I& aP, const int aDist) const
{
    for (const auto& segment : m_segments)
    {
        if (segment.Distance(aP) <= aDist)
            return true;
    }
    return false;
}

const VECTOR2I SHAPE_LINE_CHAIN::PointAlong(int aPathLength) const
{
    if (aPathLength <= 0 || m_segments.empty())
        return GetPoint(0);

    int currentLength = 0;

    for (const auto& segment : m_segments)
    {
        int segLength = static_cast<int>(segment.GetLength());

        if (currentLength + segLength >= aPathLength)
        {
            // Point is on this segment
            int remaining = aPathLength - currentLength;
            double ratio = static_cast<double>(remaining) / segLength;

            VECTOR2I start = segment.GetStart();
            VECTOR2I end = segment.GetEnd();

            return start + VECTOR2I(
                static_cast<int>((end.x - start.x) * ratio),
                static_cast<int>((end.y - start.y) * ratio)
            );
        }

        currentLength += segLength;
    }

    // Beyond end of chain
    return GetPoint(-1);
}

int SHAPE_LINE_CHAIN::PathLength(const VECTOR2I& aP, int aIndex) const
{
    int totalLength = 0;

    for (int i = 0; i < SegmentCount(); ++i)
    {
        const CHAIN_SEGMENT& segment = m_segments[i];

        if (aIndex >= 0 && i != aIndex)
        {
            totalLength += static_cast<int>(segment.GetLength());
            continue;
        }

        // Check if point lies on this segment
        if (segment.Contains(aP, 2))  // Small tolerance
        {
            VECTOR2I start = segment.GetStart();
            totalLength += (aP - start).EuclideanNorm();
            return totalLength;
        }

        if (aIndex < 0)
            totalLength += static_cast<int>(segment.GetLength());
    }

    return -1;  // Point not found
}


VECTOR2I SHAPE_LINE_CHAIN::GetPointAt(size_t aIndex) const
{
    return GetPoint(aIndex);
}


SEG SHAPE_LINE_CHAIN::GetSegmentAt(size_t aIndex) const
{
    return CSegment(aIndex);
}


VECTOR2I SHAPE_LINE_CHAIN::GetPoint(int aIndex) const
{
    if (m_segments.empty())
    {
        if (m_hasFirstPoint && aIndex == 0)
            return m_firstPoint;
        return VECTOR2I(0, 0);
    }

    int pointCount = PointCount();

    if (aIndex < 0)
        aIndex += pointCount;
    else if (aIndex >= pointCount)
        aIndex %= pointCount;

    if (aIndex == 0)
    {
        return m_segments[0].GetStart();
    }
    else if (m_closed || aIndex < pointCount - 1)
    {
        // For closed chains or non-last points, return end of previous segment
        int segIndex = aIndex - 1;

        if (segIndex >= static_cast<int>(m_segments.size()))
            segIndex = static_cast<int>(m_segments.size()) - 1;

        return m_segments[segIndex].GetEnd();
    }
    else
    {
        return m_segments.back().GetEnd();
    }
}


int SHAPE_LINE_CHAIN::PointCount() const
{
    if (m_segments.empty())
        return m_hasFirstPoint ? 1 : 0;

    if (m_closed)
        return static_cast<int>(m_segments.size());
    else
        return static_cast<int>(m_segments.size()) + 1;
}

int SHAPE_LINE_CHAIN::NextShape( int aIndex ) const
{
    if( aIndex < 0 || aIndex >= static_cast<int>( m_segments.size() ) )
        return -1;

    return aIndex + 1;
}
