#include "class_teardrop.h"
#include "class_board.h"
#include "class_board_item.h"
#include "class_undoredo_container.h"

TEARDROP::TEARDROP(BOARD_ITEM *aParent) :
    TRACK(aParent, PCB_TRACE_T)
{
    m_type = TEARDROP_NONE;
}

bool TEARDROP::Create(TRACK &aTrack, ENDPOINT_T endPoint, TEARDROP_TYPE type = TEARDROP_STRAIGHT)
{
    std::vector<VECTOR2I> upperSegment;
    std::vector<VECTOR2I> lowerSegment;
    bool result = false;

    VIA *aVia = GetViaOnEnd(aTrack, endPoint);
    if (aVia == NULL) {
        return false;
    }

    if (type == TEARDROP_STRAIGHT) {
        result = StraightSegments(aTrack, *aVia, upperSegment, lowerSegment, 100);
    }
    else if (type == TEARDROP_CURVED) {
        result = CurvedSegments(aTrack, *aVia, upperSegment, lowerSegment);
    }
    if (result == true) {
        result = BuildTracks(aTrack, upperSegment, m_upperSegment);
        result = BuildTracks(aTrack, lowerSegment, m_lowerSegment);
    }

    return result;
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

// TODO: m_TracksConnected member is considered a temporary storage. Find another way to get an object
VIA *TEARDROP::GetViaOnEnd(TRACK &aTrack, ENDPOINT_T endPoint)
{
    wxPoint trackPoint;
    std::vector<TRACK *>::const_iterator iter;

    if (endPoint == ENDPOINT_START) {
        trackPoint = aTrack.GetStart();
    }
    else {
        trackPoint = aTrack.GetEnd();
    }

    for (iter = aTrack.m_TracksConnected.begin(); iter != aTrack.m_TracksConnected.end(); ++iter) {
        KICAD_T type = (*iter)->Type();
        bool hitTest = (*iter)->HitTest(trackPoint);
        if (type == PCB_VIA_T && hitTest == true) {
            return static_cast<VIA *>(*iter);
        }
    }
    return NULL;
}

bool TEARDROP::BuildTracks(TRACK &aTrack, std::vector<VECTOR2I> points, std::vector<TRACK *> tracks)
{
    BOARD *board = aTrack.GetBoard();
    wxPoint currentPoint(0, 0);
    wxPoint prevPoint(points[0].x, points[0].y);
    for (size_t i = 1; i < points.size(); i++) {
        TRACK *track = new TRACK(aTrack);
        track->SetWidth(aTrack.GetWidth());
        track->SetLayer(aTrack.GetLayer());
        track->SetNetCode(aTrack.GetNetCode());
        currentPoint.x = points[i].x;
        currentPoint.y = points[i].y;
        track->SetStart(prevPoint);
        track->SetEnd(currentPoint);
        board->Add(track);
        tracks.push_back(track);
        prevPoint = currentPoint;
    }

    return true;
}
