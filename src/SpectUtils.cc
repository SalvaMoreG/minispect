
#include "SpectUtils.h"
#include "CUserParameters.h"

#include "CVIP3Matrix.h"

#include <iostream>
#include <cmath>

using namespace SpectUtils;
using namespace std;


// Rotating the hitposition;
//
void 
SpectUtils::RotatePosition(const C3Vector& in_hitposition, C3Vector& out_rotatedposition, 
                    const double& in_angleDeg, AXIS in_axis )
{
    bool debug(false);
    double angleRad = (in_angleDeg*kPI/180.0);
        // kPI is defined in CVIPUtils.h (VIPCommonLib/include)
    if (debug)
        cout << "Rotation angles: " << in_angleDeg << " (degrees) " << angleRad << " (radians)" << endl;
    
    C3Matrix rotationMatrix( in_axis, angleRad);
    if (debug)
        cout << "matrix: " << rotationMatrix << endl;
    out_rotatedposition = rotationMatrix * in_hitposition;
}

double 
SpectUtils::GetRotationAngle(const unsigned long long int& in_eventNr)
{
    CUserParameters* parameters = CUserParameters::Instance();
    double angle = parameters->GetRotationAngleDeg();
    if ( parameters->GetRotationAngleDeltaN() > 0 )
    {
        int set = (int) (in_eventNr/parameters->GetRotationAngleDeltaN());
        angle += set * parameters->GetRotationAngleDeltaDeg();
    }
    return angle;
}

void
SpectUtils::GetPinholePosition(const unsigned long long int& in_eventNr, C3Vector& out_pinholePosition)
{
    CUserParameters* parameters = CUserParameters::Instance();
    out_pinholePosition.Set(parameters->GetPinHoleXPosition(),
                            parameters->GetPinHoleYPosition(),
                            parameters->GetPinHoleZPosition());
    if (   fabs(parameters->GetRotationAngleDeg()) > 0.001
        || fabs(parameters->GetRotationAngleDeltaDeg()) > 0.001 )
    {
        double angle = GetRotationAngle(in_eventNr);
        C3Vector tmpPos(out_pinholePosition);
        RotatePosition(tmpPos, out_pinholePosition, angle, parameters->GetRotationAngleAxis());
    }

    if ( fabs(parameters->GetOrbitAngleDeg()) > 0.001 )
    {
        C3Vector tmpPos(out_pinholePosition);
        RotatePosition(tmpPos, out_pinholePosition, parameters->GetOrbitAngleDeg()
                                                  , parameters->GetOrbitAngleAxis());
    }
}

