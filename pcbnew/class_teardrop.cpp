#include "class_teardrop.h"
#include "class_board.h"
#include "class_board_item.h"
#include "class_undoredo_container.h"
#include "geometry/seg.h"
#include "geometry/shape_circle.h"
#include "router/pns_utils.h"

TEARDROP::TEARDROP()
{
    m_type = TEARDROP_NONE;
}

bool TEARDROP::Create(TRACK &aTrack, ENDPOINT_T endPoint, TEARDROP_TYPE type = TEARDROP_STRAIGHT)
{
    bool result = false;

    BOARD_CONNECTED_ITEM *anObject = GetObjectOnEnd(aTrack, endPoint);
    VIA *aVia = NULL;
    if (anObject == NULL) {
        return false;
    }
    else {
        switch (anObject->Type()) {
        case PCB_VIA_T:
            aVia = dynamic_cast<VIA *>(anObject);
            break;
        case PCB_PAD_T:
            aVia = new VIA(NULL);
            aVia->SetLayer(anObject->GetLayer());
            aVia->SetPosition(anObject->GetPosition());
            aVia->SetWidth(2 * dynamic_cast<D_PAD *>(anObject)->GetBoundingRadius());
            break;
        default:
            break;
        }
    }

    if (type == TEARDROP_STRAIGHT) {
        result = StraightSegments(aTrack, *aVia, 100);
    }
    else if (type == TEARDROP_CURVED) {
        result = CurvedSegments(aTrack, *aVia);
    }

    return result;
}

bool TEARDROP::SetVector(TRACK &aTrack, const VIA & aVia, VECTOR2I &startPoint, VECTOR2I &endPoint)
{
    // Decide which end of the track is inside via and set this point as end of vector
    STATUS_FLAGS status = aTrack.IsPointOnEnds(aVia.GetPosition(), aVia.GetWidth() / 2);
    if (status == STARTPOINT) {
        startPoint = aTrack.GetEnd();
        endPoint = aTrack.GetStart();
    }
    else if (status == ENDPOINT) {
        startPoint = aTrack.GetStart();
        endPoint = aTrack.GetEnd();
    }
    else {
        // The via is too far from any end or the track is too short
        return false;
    }

    return true;
}

bool TEARDROP::CurvedSegments(TRACK &aTrack, const VIA &aVia)
{
    VECTOR2I startPoint(0, 0);
    VECTOR2I endPoint(0, 0);
    std::vector<VECTOR2I> upperSegment;
    std::vector<VECTOR2I> lowerSegment;

    if ( !SetVector(aTrack, aVia, startPoint, endPoint) ) {
        return false;
    }

    // Check that the track is not too short
    double segOutsideVia = aTrack.GetLength() - (aVia.GetWidth() / 2);
    double minLength = (150 * aVia.GetWidth() / 2) / 100;
    if (segOutsideVia < minLength) {
        return false;
    }

    VECTOR2I point(0, 0);
    VECTOR2I viaCenter(aVia.GetPosition().x, aVia.GetPosition().y);
    VECTOR2I apertureUpper(0, 0);
    VECTOR2I apertureLower(0, 0);
    double radius = (aVia.GetWidth() / 2) - (aTrack.GetWidth() / 2);
    double rotationAngle = VECTOR2I(startPoint - endPoint).Angle();
    // Calculate the segments of deltoid composing the outline of a teardrop
    for ( int i = 10; i <= 60; i = i + 10 ) {
        PointOnCurve(i, radius, point);
        point = point.Rotate(rotationAngle);
        point += viaCenter;
        m_coordinates.push_back(point);
        if (i == 50) {
            apertureUpper = point;
        }
    }
    for ( int i = 300; i <= 350; i = i + 10 ) {
        PointOnCurve(i, radius, point);
        point = point.Rotate(rotationAngle);
        point += viaCenter;
        m_coordinates.push_back(point);
        if (i == 340) {
            apertureLower = point;
        }
    }

    // Calculate the number of segments needed to fill the area inside the teardrop
    if (aVia.GetWidth() / 2 > 2 * aTrack.GetWidth()) {
        // First, calculate the distance between two points on both sides of the track and
        // number of iterations required to fill the zone
        SEG aperture(apertureUpper, apertureLower);
        int numSegments = aperture.Length() / aTrack.GetWidth();

        int delta = radius / numSegments;
        for (int iteration = 0; iteration < numSegments; iteration++) {
            radius = radius - delta;
            for ( int i = 10; i <= 60; i = i + 10 ) {
                PointOnCurve(i, radius, point);
                point = point.Rotate(rotationAngle);
                point += viaCenter;
                if (i == 10) {
                    int distance = SEG(viaCenter, point).Length();
                    if (distance < aVia.GetWidth() / 2) {
                        break;
                    }
                }
                m_coordinates.push_back(point);
            }
            lowerSegment.clear();
            for ( int i = 350; i >= 300; i = i - 10 ) {
                PointOnCurve(i, radius, point);
                point = point.Rotate(rotationAngle);
                point += viaCenter;
                if (i == 350) {
                    int distance = SEG(viaCenter, point).Length();
                    if (distance < aVia.GetWidth() / 2) {
                        break;
                    }
                }
                lowerSegment.push_back(point);
            }
            // Revert coordinates order
            for (std::vector<VECTOR2I>::reverse_iterator iter = lowerSegment.rbegin(); iter != lowerSegment.rend(); ++iter) {
                m_coordinates.push_back(*iter);
            }
        }
    }

    return true;
}

bool TEARDROP::StraightSegments(TRACK &aTrack, const VIA &aVia, int distance = 100)
{
    VECTOR2I startPoint(0, 0);
    VECTOR2I endPoint(0, 0);
    VECTOR2I viaCenter(aVia.GetPosition().x, aVia.GetPosition().y);

    if ( !SetVector(aTrack, aVia, startPoint, endPoint) ) {
        return false;
    }

    // Check that the track is not too short
    double segOutsideVia = aTrack.GetLength() - (aVia.GetWidth() / 2);
    double minLength = (distance * aVia.GetWidth() / 2) / 100;
    if (segOutsideVia < minLength) {
        return false;
    }

    // Equation coefficients
    double r = (aVia.GetWidth() / 2) + ((distance * aVia.GetWidth()) / (2 *100));
    double a = pow((endPoint.x - startPoint.x), 2) + pow((endPoint.y - startPoint.y), 2);
    double b = 2 * (double)(endPoint.x - startPoint.x) * (double)(startPoint.x - viaCenter.x) + 2 * (double)(endPoint.y - startPoint.y) * (double)(startPoint.y - viaCenter.y);
    double c = pow((startPoint.x - viaCenter.x), 2) + pow((startPoint.y - viaCenter.y), 2) - pow(r, 2);
    double t = 2 * c / (-b + sqrt(b * b - 4 * a * c));

    double x = (endPoint.x - startPoint.x) * t + startPoint.x;
    double y = (endPoint.y - startPoint.y) * t + startPoint.y;
    VECTOR2I linePoint(x, y);

    int correctedRadius = (aVia.GetWidth() / 2) - (aTrack.GetWidth() / 2);
    c = pow((startPoint.x - viaCenter.x), 2) + pow((startPoint.y - viaCenter.y), 2) - pow(correctedRadius, 2);
    t = 2 * c / (-b + sqrt(b * b - 4 * a * c));
    x = (endPoint.x - startPoint.x) * t + startPoint.x;
    y = (endPoint.y - startPoint.y) * t + startPoint.y;
    VECTOR2I circlePoint(x, y);

    VECTOR2I upperPoint = circlePoint - viaCenter;
    VECTOR2I lowerPoint = upperPoint;
    upperPoint = upperPoint.Rotate(M_PI / 2);
    lowerPoint = lowerPoint.Rotate(-M_PI / 2);
    upperPoint += viaCenter;
    lowerPoint += viaCenter;

    // Calculate the number of segments needed to fill the area inside the teardrop
    std::vector<VECTOR2I> splitPoints;
    if (aVia.GetWidth() / 2 > 2 * aTrack.GetWidth()) {
        // First, calculate the intersection point of the circle and one hand of the teardrop
        r = aVia.GetWidth() / 2;
        a = pow((upperPoint.x - startPoint.x), 2) + pow((upperPoint.y - startPoint.y), 2);
        b = 2 * (double)(upperPoint.x - startPoint.x) * (double)(startPoint.x - viaCenter.x) + 2 * (double)(upperPoint.y - startPoint.y) * (double)(startPoint.y - viaCenter.y);
        c = pow((startPoint.x - viaCenter.x), 2) + pow((startPoint.y - viaCenter.y), 2) - pow(r, 2);
        t = 2 * c / (-b + sqrt(b * b - 4 * a * c));
        x = (upperPoint.x - startPoint.x) * t + startPoint.x;
        y = (upperPoint.y - startPoint.y) * t + startPoint.y;
        VECTOR2I intersectionPoint((int)x, (int)y);

        // Second, calculate the distance between the given track and the intersection point
        SEG trackSegment(aTrack.GetStart().x, aTrack.GetStart().y, aTrack.GetEnd().x, aTrack.GetEnd().y);
        int dist = trackSegment.LineDistance(intersectionPoint);
        int numSegments = 2 * dist / aTrack.GetWidth();

        // Third, subdivide the diameter of the via and build additional segments
        SEG segDiameter = SEG(upperPoint, lowerPoint);
        SplitSegment(segDiameter, numSegments, splitPoints);
    }

    std::list<VECTOR2I> outlinePoints;
    outlinePoints.push_back(upperPoint);
    for (size_t i = 0; i < splitPoints.size(); i++) {
        outlinePoints.push_back(splitPoints[i]);
    }
    outlinePoints.push_back(lowerPoint);

    // Biuld triangles filling the teardrop
    int vertexNum = 0;
    std::list<VECTOR2I>::iterator iter = outlinePoints.begin();
    while ( iter != outlinePoints.end() ) {
        switch (vertexNum) {
        case 0:
            m_coordinates.push_back(linePoint);
            vertexNum++;
            break;
        case 1:
            m_coordinates.push_back(*iter);
            vertexNum++;
            iter++;
            break;
        case 2:
            m_coordinates.push_back(*iter);
            vertexNum = 0;
            iter++;
            break;
        default:break;
        }
    }
    // Append additional vertexies in order to finish last triangle
    if (vertexNum == 0) {
        m_coordinates.push_back(linePoint);
    }
    else if (vertexNum == 2) {
        m_coordinates.push_back(m_coordinates[m_coordinates.size() - 3]);
        m_coordinates.push_back(linePoint);
    }

    return true;
}

// TODO: m_TracksConnected member is considered a temporary storage. Find another way to get an object
BOARD_CONNECTED_ITEM* TEARDROP::GetObjectOnEnd(TRACK &aTrack, ENDPOINT_T endPoint)
{
    wxPoint trackPoint;
    BOARD_CONNECTED_ITEM *item = NULL;
    std::vector<TRACK *>::const_iterator iter;

    if (endPoint == ENDPOINT_START) {
        trackPoint = aTrack.GetStart();
    }
    else {
        trackPoint = aTrack.GetEnd();
    }

    // Check for vias first
    for (iter = aTrack.m_TracksConnected.begin(); iter != aTrack.m_TracksConnected.end(); ++iter) {
        KICAD_T type = (*iter)->Type();
        bool hitTest = (*iter)->HitTest(trackPoint);
        if (type == PCB_VIA_T && hitTest == true) {
            item = *iter;
        }
    }
    // Check for pads if via was not found on this end of the track
    if (item == NULL) {
        for (std::vector<D_PAD *>::iterator iter = aTrack.m_PadsConnected.begin(); iter != aTrack.m_PadsConnected.end(); ++iter) {
            PAD_SHAPE_T shape = (*iter)->GetShape();
            bool hitTest = (*iter)->HitTest(trackPoint);
            if (shape == PAD_CIRCLE && hitTest == true) {
                item = *iter;
            }
        }
    }

    return item;
}

void TEARDROP::SplitSegment(const SEG &segment, int splits, std::vector<VECTOR2I> &points)
{
    int dX = abs((segment.A.x - segment.B.x) / splits);
    int dY = abs((segment.A.y - segment.B.y) / splits);
    if (segment.A.x > segment.B.x) {
        dX = -dX;
    }
    if (segment.A.y > segment.B.y) {
        dY = -dY;
    }
    VECTOR2I delta(dX, dY);
    points.push_back(segment.A + delta);
    // The last point is excluded as it will coinside with already built tracks
    for (int i = 1; i < splits - 1; i++) {
        points.push_back(points.back() + delta);
    }
}
