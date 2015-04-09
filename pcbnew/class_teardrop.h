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

/**
 * @file class_teardrop.h
 * @brief Definitions for teardrops.
 */

#ifndef CLASS_TEARDROP_H
#define CLASS_TEARDROP_H

#include "class_track.h"

class TEARDROP
{
public:
    TEARDROP();

    /**
     * @brief Defines the type of a teardrop.
     */
    typedef enum {
        TEARDROP_NONE,		///< The type is undefined
        TEARDROP_STRAIGHT,	///< The teardrop is created by two straight segments
        TEARDROP_CURVED		///< The teardrop is created by several segments approximating a curve
    } TEARDROP_TYPE;

    /**
     * @brief GetType returns the type of the teardrop.
     * @return TEARDROP_TYPE
     */
    TEARDROP_TYPE GetType() const {return m_type;}

    /**
     * @brief Function Create creates a teardrop(s) for a given track
     * @param aTrack
     * @return \a true in case the teardrops were successfully built and \a false otherwise
     */
    bool Create(TRACK &aTrack, ENDPOINT_T endPoint, TEARDROP_TYPE type);

    void GetCoordinates(std::vector<VECTOR2I> &points);

private:
    ///> Contains the type of teardrop
    TEARDROP_TYPE m_type;
    ///> \a m_upperSegment and \a m_lowerSegment contain coordinates of segments composing a teardrop
    std::vector<VECTOR2I> m_upperSegment;
    std::vector<VECTOR2I> m_lowerSegment;

    /**
     * @brief Function \a CurvedSegments computes several points on deltoid curve and moves
     * these points along the vector defined by \a aTrack.
     *
     * This function computes the coordinates of points only and does not build actual track segments.
     * See deltiod description and its parametric equations on [wiki page](http://en.wikipedia.org/wiki/Deltoid_curve).
     * @param [in] aTrack defines a vector along which the curved segments should be built
     * @param [in] aVia used as the center of coordinates
     * @param [out] upperSegment vector contains the coordinates of computed segments
     * @param [out] lowerSegment vector contains the coordinates of mirrored segments
     * @return \a true in case the segments were successfully built and \a false otherwise
     */
    bool CurvedSegments(TRACK &aTrack, const VIA &aVia, std::vector<VECTOR2I> &upperSegment, std::vector<VECTOR2I> &lowerSegment);

    /**
     * @brief Function \a StraightSegments builds two tangent lines for a circle from a givent point.
     *
     * This function computes the coordinates of points only and does not build actual track segments.
     * @param [in] aTrack defines a vector along which the segments should be built
     * @param [in] aVia represents a circle to which the segments should be built
     * @param [in] distance is distance ratio (in percent) from circle center in respect to its diameter
     * @param [out] upperSegment vector contains the coordinates of computed segments
     * @param [out] lowerSegment vector contains the coordinates of mirrored segments
     * @return \a true in case the segments were successfully built and \a false otherwise
     */
    bool StraightSegments(TRACK &aTrack, const VIA &aVia, std::vector<VECTOR2I> &upperSegment, std::vector<VECTOR2I> &lowerSegment, int distance);

    /**
     * @brief Function SetVector creates a vector from \a aTrack directed into \a aVia
     * @param aTrack is used to create a vector
     * @param startPoint is start point of resulting vector
     * @param endPoint is end point of resulting vector
     * @return \a true in case the vector is created successfully and \a false otherwise
     */
    bool SetVector(TRACK &aTrack, const VIA &aVia, VECTOR2I &startPoint, VECTOR2I &endPoint);

    VIA* GetViaOnEnd(TRACK &aTrack, ENDPOINT_T endPoint);
    bool BuildTracks(TRACK &aTrack, const std::vector<VECTOR2I> points, std::vector<TRACK *> tracks);
};

#endif // CLASS_TEARDROP_H
