/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2015 Elphel, Inc.
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

#ifndef CLASS_TEARDROP_H
#define CLASS_TEARDROP_H

#include "class_track.h"
#include "geometry/seg.h"

/**
 * @brief The TEARDROP class
 * is base definition of a teardrop. It is intended for calculation and holding of points which
 * compose a teardrop. This class does not contain any methods which create actual tracks.
 */
class TEARDROP
{
public:
    TEARDROP();

    /**
     * @brief The TEARDROP_TYPE defines the type of a teardrop.
     */
    typedef enum
    {
        TEARDROP_NONE,      ///< The type is undefined
        TEARDROP_STRAIGHT,  ///< The teardrop is created by two straight segments
        TEARDROP_CURVED     ///< The teardrop is created by several segments approximating a curve
    } TEARDROP_TYPE;

    /**
     * @brief Function \a GetType
     * returns the type of the teardrop.
     * @return TEARDROP_TYPE - the type of the teardrop
     */
    TEARDROP_TYPE GetType() const { return m_type; }

    /**
     * @brief Function \a Create
     * creates a teardrop(s) for a given track.
     * @param [in] aTrack is a track at which teardrop(s) should be created
     * @param [in] aEndPoint is an end point at which a teardrop should be created
     * @param [in] aType defines the type of a teardrop
     * @return bool - \a true in case the teardrops were successfully built and \a false otherwise
     */
    bool Create( TRACK& aTrack, ENDPOINT_T aEndPoint, TEARDROP_TYPE aType );

    /**
     * @brief Function \a GetCoordinates
     * returns the coordinates of created teardrop.
     * @param [out] aPoints is a container for coordinates
     */
    void GetCoordinates( std::vector<VECTOR2I>& aPoints ) const { aPoints = m_coordinates; }

private:
    /// Contains the type of teardrop
    TEARDROP_TYPE m_type;
    /// Contains the actual coordinates of teardrop
    std::vector<VECTOR2I> m_coordinates;

    /**
     * @brief Function \a curvedSegments
     * computes several points on deltoid curve and moves these points along the vector
     * defined by \a aTrack.
     *
     * This function computes the coordinates of points only and does not build actual track segments.
     * See deltiod description and its parametric equations on [wiki page](http://en.wikipedia.org/wiki/Deltoid_curve).
     * @param [in] aTrack defines a vector along which the curved segments should be built
     * @param [in] aVia used as the center of coordinates
     * @return bool - \a true in case the segments were successfully built and \a false otherwise
     */
    bool curvedSegments( TRACK& aTrack, const VIA& aVia );

    /**
     * @brief Function \a straightSegments
     * builds two tangent lines to a circle from a givent point.
     *
     * This function computes the coordinates of points only and does not build actual track segments.
     * @param [in] aTrack defines a vector along which the segments should be built
     * @param [in] aVia represents a circle to which the segments should be built
     * @param [in] aDistance is distance ratio (in percent) from circle center in respect to its diameter
     * @return bool - \a true in case the segments were successfully built and \a false otherwise
     */
    bool straightSegments( TRACK& aTrack, const VIA& aVia, int aDistance );

    /**
     * @brief Function \a setVector
     * creates a vector from \a aTrack directed into \a aVia.
     * @param [in] aTrack is used to create a vector
     * @param [in] aVia is an object to which the vector should be pointed to
     * @param [out] aStartPoint is start point of resulting vector
     * @param [out] aEndPoint is end point of resulting vector
     * @return bool - \a true in case the vector is created successfully and \a false otherwise
     */
    bool setVector( TRACK& aTrack, const VIA& aVia, VECTOR2I& aStartPoint, VECTOR2I& aEndPoint );

    /**
     * @brief Function \a getObjectOnEnd
     * returns an object (via or pad) at the given end of a track.
     * @param [in] aTrack is a reference track
     * @param [in] aEndPoint defines the end in question
     * @return BOARD_CONNECTED_ITEM - the object found or NULL otherwise
     */
    BOARD_CONNECTED_ITEM* getObjectOnEnd( TRACK& aTrack, ENDPOINT_T aEndPoint );

    /**
     * @brief Function \a splitSegment
     * splits a segment into given number of subsegments.
     * @param [in] aSegment is a segment to be split
     * @param [i] aSplits is a number of splits
     * @param [out] aPoints is a container for split points
     */
    void splitSegment( const SEG& aSegment, int aSplits, std::vector<VECTOR2I>& aPoints );

    /**
     * @brief Function \a pointOnCurve
     * calculates a single point on a deltoid curve.
     * @param [in] aAngle is an angle at which the point should be calculated
     * @param [in] aRadius is the radius of a rolling circle
     * @param [out] aPoint is a container for calculated point
     */
    inline void pointOnCurve( int aAngle, double aRadius, VECTOR2I& aPoint )
    {
        double coeff = M_PI / 180.0;

        aPoint.x = 2 * aRadius * cos( coeff * aAngle ) + aRadius * cos( 2 * coeff * aAngle );
        aPoint.y = 2 * aRadius * sin( coeff * aAngle ) - aRadius * sin( 2 * coeff * aAngle );
    }
};

#endif    // CLASS_TEARDROP_H
