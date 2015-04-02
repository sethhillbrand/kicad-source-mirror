#include "class_teardrop.h"

TEARDROP::TEARDROP(BOARD_ITEM *aParent) :
    TRACK(aParent, PCB_TRACE_T)
{
    m_type = TEARDROP_NONE;
}

bool TEARDROP::Create(TRACK &aTrack)
{

}

bool TEARDROP::SetVector(TRACK &aTrack, const VIA & aVia, VECTOR2I &startPoint, VECTOR2I &endPoint)
{
    // Decide which end of the track is inside via and set this point as end of vector
    STATUS_FLAGS status = aTrack.IsPointOnEnds(wxPoint(aVia.GetPosition().x, aVia.GetPosition().y), aVia.GetWidth() / 2);
    if (status == STARTPOINT) {
        startPoint = aTrack.GetEnd();
        endPoint = aTrack.GetStart();
    }
    else if (status == ENDPOINT) {
        startPoint = aTrack.GetStart();
        endPoint = aTrack.GetEnd();
    }
    else {
        // The via is too far from any ends or the track is too short
        return false;
    }

    return true;
}

bool TEARDROP::CurvedSegments(TRACK &aTrack, const VIA &aVia, std::vector<VECTOR2I> &upperSegment, std::vector<VECTOR2I> &lowerSegment)
{
    VECTOR2I startPoint(0, 0);
    VECTOR2I endPoint(0, 0);

    if ( !SetVector(aTrack, aVia, startPoint, endPoint) ) {
        return false;
    }

    VECTOR2I point(0, 0);
    VECTOR2I viaCenter(aVia.GetPosition().x, aVia.GetPosition().y);
    double coeff = M_PI / 180.0;
    double radius = (aVia.GetWidth() / 2) - (aTrack.GetWidth() / 2);
    double rotationAngle = VECTOR2I(startPoint - endPoint).Angle();
    // Calculate segments of deltoid
    for ( int i = 10; i <= 60; i = i + 10 ) {
        point.x = 2 * radius * cos(coeff * i) + radius * cos(2 * coeff * i);
        point.y = 2 * radius * sin(coeff * i) - radius * sin(2 * coeff * i);
        point = point.Rotate(rotationAngle);
        point += viaCenter;
        upperSegment.push_back(point);
    }
    for ( int i = 300; i <= 350; i = i + 10 ) {
        point.x = 2 * radius * cos(coeff * i) + radius * cos(2 * coeff * i);
        point.y = 2 * radius * sin(coeff * i) - radius * sin(2 * coeff * i);
        point = point.Rotate(rotationAngle);
        point += viaCenter;
        lowerSegment.push_back(point);
    }

    return true;
}

bool TEARDROP::StraightSegments(TRACK &aTrack, const VIA &aVia, std::vector<VECTOR2I> &upperSegment, std::vector<VECTOR2I> &lowerSegment, int distance = 100)
{
    VECTOR2I startPoint(0, 0);
    VECTOR2I endPoint(0, 0);
    VECTOR2I viaCenter(aVia.GetPosition().x, aVia.GetPosition().y);

    if ( !SetVector(aTrack, aVia, startPoint, endPoint) ) {
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

    upperSegment.push_back(linePoint);
    upperSegment.push_back(upperPoint);
    lowerSegment.push_back(linePoint);
    lowerSegment.push_back(lowerPoint);

    return true;
}
