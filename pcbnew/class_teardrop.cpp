#include "class_teardrop.h"
#include "class_board.h"
#include "class_board_item.h"
#include "class_undoredo_container.h"
#include "geometry/seg.h"
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
        result = StraightSegments(aTrack, *aVia, m_upperSegment, m_lowerSegment, 100);
    }
    else if (type == TEARDROP_CURVED) {
        result = CurvedSegments(aTrack, *aVia, m_upperSegment, m_lowerSegment);
    }

    return result;
}

void TEARDROP::GetCoordinates(std::vector<VECTOR2I> &points)
{
//    points.insert(points.end(), m_upperSegment.begin(), m_upperSegment.end());
//    points.insert(points.end(), m_lowerSegment.begin(), m_lowerSegment.end());
    points = vect;
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

bool TEARDROP::CurvedSegments(TRACK &aTrack, const VIA &aVia, std::vector<VECTOR2I> &upperSegment, std::vector<VECTOR2I> &lowerSegment)
{
    VECTOR2I startPoint(0, 0);
    VECTOR2I endPoint(0, 0);

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

    upperSegment.push_back(linePoint);
    upperSegment.push_back(upperPoint);

    // Calculate the number of segments needed to fill the area inside the teardrop
    std::vector<VECTOR2I> splitPoints;
    int vertexNum = 1;
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
        DrawDebugPoint(intersectionPoint, 5);

        // Second, calculate the distance between the track given and the intersection point
        SEG trackSegment(aTrack.GetStart().x, aTrack.GetStart().y, aTrack.GetEnd().x, aTrack.GetEnd().y);
        int dist = trackSegment.LineDistance(intersectionPoint);
        int numSegments = 2 * dist / aTrack.GetWidth();

        // Third, subdivide radius of the via and build additional segments
        SEG segRadius = SEG(upperPoint, lowerPoint);
//        SEG segRadius = SEG(upperPoint, viaCenter);
        SplitSegment(segRadius, numSegments, splitPoints);
    }

    std::list<VECTOR2I> pts;
    pts.push_back(upperPoint);
    for (size_t i = 0; i < splitPoints.size(); i++) {
        pts.push_back(splitPoints[i]);
    }
    pts.push_back(lowerPoint);

    vertexNum = 0;
    std::list<VECTOR2I>::iterator iter = pts.begin();
    while ( iter != pts.end() ) {
        switch (vertexNum) {
        case 0:
            vect.push_back(linePoint);
            vertexNum++;
            break;
        case 1:
            vect.push_back(*iter);
            vertexNum++;
            iter++;
            break;
        case 2:
            vect.push_back(*iter);
            vertexNum = 0;
            iter++;
            break;
        default:break;
        }
    }
    if (vertexNum == 0) {
        vect.push_back(linePoint);
    }
    else if (vertexNum == 2) {
        vect.push_back(vect[vect.size() - 3]);
        vect.push_back(linePoint);
    }

    for (size_t i = 0; i < vect.size(); i++) {
        printf("%d, %d\n", vect[i].x, vect[i].y);
    }
//    lowerSegment.push_back(lowerPoint);
//    lowerSegment.push_back(linePoint);

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
