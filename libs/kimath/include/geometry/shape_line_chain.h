/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2013 CERN
 * @author Tomasz Wlostowski <tomasz.wlostowski@cern.ch>
 * Copyright The KiCad Developers, see AUTHORS.txt for contributors.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#ifndef __SHAPE_LINE_CHAIN
#define __SHAPE_LINE_CHAIN

#include <vector>
#include <optional>
#include <clipper2/clipper.h>
#include <geometry/seg.h>
#include <geometry/shape_line_chain_base.h>
#include <geometry/chain_segment.h>  // unified segment type
#include <geometry/corner_strategy.h>
#include <math/vector2d.h>

// Forward declarations
class SHAPE_POLY_SET;
class SHAPE_ARC;

/**
 * Holds information on each point of a SHAPE_LINE_CHAIN that is retrievable
 * after an operation with Clipper2Lib
 */
struct CLIPPER_Z_VALUE
{
    CLIPPER_Z_VALUE()
    {
        m_FirstArcIdx = -1;
        m_SecondArcIdx = -1;
    }

    CLIPPER_Z_VALUE( const std::pair<ssize_t, ssize_t> aShapeIndices, ssize_t aOffset = 0 )
    {
        m_FirstArcIdx = aShapeIndices.first;
        m_SecondArcIdx = aShapeIndices.second;

        auto offsetVal = [&]( ssize_t& aVal )
                         {
                             if( aVal >= 0 )
                                 aVal += aOffset;
                         };

        offsetVal( m_FirstArcIdx );
        offsetVal( m_SecondArcIdx );
    }

    ssize_t m_FirstArcIdx;
    ssize_t m_SecondArcIdx;
};

/**
 * Represent a polyline containing arcs as well as line segments: A chain of connected line and/or
 * arc segments.
 *
 * This refactored version uses a unified SEGMENT type instead of separate containers for
 * points, shape indices, and arcs, greatly simplifying the implementation while maintaining
 * full compatibility with the existing API.
 */
class SHAPE_LINE_CHAIN : public SHAPE_LINE_CHAIN_BASE
{
public:
    typedef std::vector<CHAIN_SEGMENT>::iterator segment_iter;
    typedef std::vector<CHAIN_SEGMENT>::const_iterator segment_citer;

    /**
     * Represent an intersection between two line segments
     */
    struct INTERSECTION
    {
        VECTOR2I p;              ///< Point of intersection
        int index_our;           ///< Index of intersecting segment in this chain
        int index_their;         ///< Index of intersecting segment in other chain
        bool is_corner_our;      ///< True if corner of our chain lies on their chain
        bool is_corner_their;    ///< True if corner of their chain lies on our chain
        bool valid;              ///< Validity flag for intersection refinement

        INTERSECTION() :
            index_our(-1), index_their(-1),
            is_corner_our(false), is_corner_their(false), valid(false) {}
    };

    typedef std::vector<INTERSECTION> INTERSECTIONS;

    // Constructors
    SHAPE_LINE_CHAIN() :
        SHAPE_LINE_CHAIN_BASE(SH_LINE_CHAIN), m_hasFirstPoint(false), m_closed(false), m_width(0) {}

    SHAPE_LINE_CHAIN(const SHAPE_LINE_CHAIN& aShape) = default;
    SHAPE_LINE_CHAIN(SHAPE_LINE_CHAIN&& aShape) noexcept = default;

    SHAPE_LINE_CHAIN(const std::vector<int>& aV);
    SHAPE_LINE_CHAIN(const std::vector<VECTOR2I>& aV, bool aClosed = false);
    SHAPE_LINE_CHAIN(const SHAPE_ARC& aArc, bool aClosed = false, std::optional<int> aMaxError = {});

    // Legacy Clipper2 constructor for compatibility
    SHAPE_LINE_CHAIN(const Clipper2Lib::Path64& aPath,
                      const std::vector<CLIPPER_Z_VALUE>& aZValueBuffer,
                      const std::vector<SHAPE_ARC>& aArcBuffer);

    virtual ~SHAPE_LINE_CHAIN() {}

    // Assignment operators
    SHAPE_LINE_CHAIN& operator=(const SHAPE_LINE_CHAIN&) = default;
    SHAPE_LINE_CHAIN& operator=(SHAPE_LINE_CHAIN&&) noexcept = default;

    SHAPE* Clone() const override;

    // Basic properties
    void Clear();
    void SetClosed(bool aClosed);
    bool IsClosed() const override { return m_closed; }
    void SetWidth(int aWidth) { m_width = aWidth; }
    int Width() const { return m_width; }

    // Size and indexing
    int SegmentCount() const { return static_cast<int>(m_segments.size()); }
    int PointCount() const;  // Computed from segments: N+1 for open, N for closed
    int ShapeCount() const { return SegmentCount(); }  // Same as SegmentCount in new model

    // SHAPE_LINE_CHAIN_BASE interface implementation
    size_t GetPointCount() const override { return static_cast<size_t>(PointCount()); }
    size_t GetSegmentCount() const override { return static_cast<size_t>(SegmentCount()); }
    VECTOR2I GetPointAt(size_t aIndex) const override;
    SEG GetSegmentAt(size_t aIndex) const override;

    // Segment access (refactored interface)
    const CHAIN_SEGMENT& GetSegment(int aIndex) const;
    CHAIN_SEGMENT& GetSegment(int aIndex);
    const SEG CSegment(int aIndex) const;  // Legacy compatibility - converts to SEG

    // Point access (computed from segments)
    VECTOR2I GetPoint(int aIndex) const;
    const VECTOR2I CPoint(int aIndex) const { return GetPoint(aIndex); }
    const VECTOR2I CLastPoint() const { return GetPoint(-1); }
    const std::vector<VECTOR2I> CPoints() const;  // Computed on demand

    // Segment iteration
    segment_citer begin() const { return m_segments.begin(); }
    segment_citer end() const { return m_segments.end(); }
    segment_iter begin() { return m_segments.begin(); }
    segment_iter end() { return m_segments.end(); }

    // Geometry queries
    const BOX2I BBox(int aClearance = 0) const override;
    void GenerateBBoxCache() const;
    BOX2I* GetCachedBBox() const override { return &m_bbox; }
    long long int Length() const;
    double Area(bool aAbsolute = true) const;

    // Collision detection (overrides base class for arc support)
    bool Collide(const VECTOR2I& aP, int aClearance = 0, int* aActual = nullptr,
                 VECTOR2I* aLocation = nullptr) const override;
    bool Collide(const SEG& aSeg, int aClearance = 0, int* aActual = nullptr,
                 VECTOR2I* aLocation = nullptr) const override;

    // Point and distance queries
    const VECTOR2I NearestPoint(const VECTOR2I& aP, bool aAllowInternalShapePoints = true) const;
    const VECTOR2I NearestPoint(const SEG& aSeg, int& dist) const;
    int NearestSegment(const VECTOR2I& aP) const;
    bool CheckClearance(const VECTOR2I& aP, const int aDist) const;
    const VECTOR2I PointAlong(int aPathLength) const;
    int PathLength(const VECTOR2I& aP, int aIndex = -1) const;

    // Search operations
    int Find(const VECTOR2I& aP, int aThreshold = 0) const;
    int FindSegment(const VECTOR2I& aP, int aThreshold = 1) const;

    // Modification operations
    void Append(int aX, int aY, bool aAllowDuplication = false);
    void Append(const VECTOR2I& aP, bool aAllowDuplication = false);
    void Append(const SHAPE_LINE_CHAIN& aOtherLine);
    void Append(const SHAPE_ARC& aArc);
    void Append(const SHAPE_ARC& aArc, int aMaxError);

    void Insert(size_t aVertex, const VECTOR2I& aP);
    void Insert(size_t aVertex, const SHAPE_ARC& aArc);
    void Insert(size_t aVertex, const SHAPE_ARC& aArc, int aMaxError);

    void SetPoint(int aIndex, const VECTOR2I& aPos);
    void Replace(int aStartIndex, int aEndIndex, const VECTOR2I& aP);
    void Replace(int aStartIndex, int aEndIndex, const SHAPE_LINE_CHAIN& aLine);

    void Remove(int aIndex);
    void Remove(int aStartIndex, int aEndIndex);
    void RemoveShape(int aPointIndex);  // Remove entire segment

    int Split(const VECTOR2I& aP, bool aExact = false);

    // Simplification
    void RemoveDuplicatePoints();
    void Simplify(int aTolerance = 0);

    // Slicing and manipulation
    const SHAPE_LINE_CHAIN Slice(int aStartIndex, int aEndIndex) const;
    const SHAPE_LINE_CHAIN Reverse() const;

    void Split(const VECTOR2I& aStart, const VECTOR2I& aEnd, SHAPE_LINE_CHAIN& aPre,
               SHAPE_LINE_CHAIN& aMid, SHAPE_LINE_CHAIN& aPost) const;

    // Intersections
    bool Intersects(const SEG& aSeg) const;
    bool Intersects(const SHAPE_LINE_CHAIN& aChain) const;
    int Intersect(const SEG& aSeg, INTERSECTIONS& aIp) const;
    int Intersect(const SHAPE_LINE_CHAIN& aChain, INTERSECTIONS& aIp,
                  bool aExcludeColinearAndTouching = false,
                  BOX2I* aChainBBox = nullptr) const;

    // Self-intersection detection
    const std::optional<INTERSECTION> SelfIntersecting() const;

    // Offsetting
    bool OffsetLine(int aAmount, CORNER_STRATEGY aCornerStrategy, int aMaxError,
                    SHAPE_LINE_CHAIN& aLeft, SHAPE_LINE_CHAIN& aRight,
                    bool aSimplify = false) const;

    // Transformations
    void Move(const VECTOR2I& aVector) override;
    void Rotate(const EDA_ANGLE& aAngle, const VECTOR2I& aCenter = {0, 0}) override;
    void Mirror(const VECTOR2I& aRef, FLIP_DIRECTION aFlipDirection);
    void Mirror(const SEG& axis);

    // Utility
    bool IsSolid() const override { return false; }
    void TransformToPolygon(SHAPE_POLY_SET& aBuffer, int aError, ERROR_LOC aErrorLoc) const override;

    // Closest point operations
    bool ClosestPoints(const SHAPE_LINE_CHAIN& aOther, VECTOR2I& aPt0, VECTOR2I& aPt1) const;

    // Serialization
    const std::string Format(bool aCplusPlus = true) const override;
    bool Parse(std::stringstream& aStream) override;

    // Comparison
    bool operator!=(const SHAPE_LINE_CHAIN& aRhs) const;
    bool CompareGeometry(const SHAPE_LINE_CHAIN& aOther) const;

    // Arc operations
    size_t ArcCount() const;
    const SHAPE_ARC& Arc(size_t aArc) const;  // Returns arc from segment
    bool IsArcSegment(size_t aSegment) const { return aSegment < m_segments.size() && m_segments[aSegment].IsArc(); }
    bool HasArcs() const;

    // Deprecated methods (for legacy compatibility)
    int NextShape( int aIndex) const;

    /**
     * Remove all arc references in the line chain, converting them to piecewise linear approximations.
     *
     * This replaces any arc segments with multiple line segments that approximate the arc shape.
     * The approximation accuracy can be controlled via the error parameter.
     *
     * @param aMaxError Maximum allowed deviation between arc and line approximation (in internal units)
     *                  Smaller values give more accurate approximations with more line segments
     */
    void ClearArcs(int aMaxError = 10);

    /**
     * Convert a specific arc segment to line segments.
     *
     * @param aSegmentIndex Index of the arc segment to convert
     * @param aMaxError Maximum allowed deviation for the approximation
     * @return Number of line segments created, or -1 if conversion failed
     */
    int ConvertArcToLines(int aSegmentIndex, int aMaxError = 10);

    // Conversion helpers for legacy compatibility
    Clipper2Lib::Path64 ConvertToClipper2(bool aRequiredOrientation,
                                          std::vector<CLIPPER_Z_VALUE>& aZValueBuffer,
                                          std::vector<SHAPE_ARC>& aArcBuffer) const;

protected:
    // Internal helpers
    void RecalculateBBox() const;
    void UpdateBBoxWithSegment(const CHAIN_SEGMENT& aSegment) const;

    /**
     * Split an arc segment at the given point.
     *
     * @param aSegmentIndex Index of the arc segment to split
     * @param aSplitPoint Point where the arc should be split
     * @param aCoincident If true, both resulting arcs share the split point.
     *                   If false, first arc ends before split point, second starts at split point.
     * @return Index of the second arc segment created, or -1 if split failed
     */
    int SplitArcSegment(int aSegmentIndex, const VECTOR2I& aSplitPoint, bool aCoincident = true);

    /**
     * Split an arc segment at a parametric position along the arc.
     *
     * @param aSegmentIndex Index of the arc segment to split
     * @param aT Parametric position (0.0 = start, 1.0 = end)
     * @param aCoincident If true, both resulting arcs share the split point
     * @return Index of the second arc segment created, or -1 if split failed
     */
    int SplitArcSegmentAtT(int aSegmentIndex, double aT, bool aCoincident = true);

    /**
     * Ensure segment continuity after modifications.
     * Updates segment start/end points to maintain chain connectivity.
     *
     * @param aSegmentIndex Index of segment to check/fix continuity
     */
    void UpdateSegmentContinuity(int aSegmentIndex);

    /**
     * Helper method to convert arc to line segments with proper error handling.
     *
     * @param aArc The arc to convert
     * @param aMaxError Maximum allowable deviation
     * @return Vector of line segments approximating the arc
     */
    std::vector<CHAIN_SEGMENT> ConvertArcToLineSegments(const SHAPE_ARC& aArc, int aMaxError);

private:
    std::vector<CHAIN_SEGMENT> m_segments;    ///< The unified segment container
    VECTOR2I m_firstPoint;              ///< Stored when chain has no segments yet
    bool m_hasFirstPoint;               ///< True if m_firstPoint is valid
    bool m_closed;                      ///< True if chain forms a closed loop
    int m_width;                        ///< Width for collision/rendering
    mutable BOX2I m_bbox;              ///< Cached bounding box
};

#endif // __SHAPE_LINE_CHAIN