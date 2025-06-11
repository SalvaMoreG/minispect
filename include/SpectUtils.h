
#ifndef __miniSPECT_Utils_H___
#define __miniSPECT_Utils_H___

#include "CVIP3Vector.h"
#include "CVIPUtils.h"


namespace SpectUtils
{
    // Rotating the hitposition;
    void RotatePosition(const C3Vector& io_hitposition, C3Vector& out_rotatedposition, 
                        const double& in_angleDeg, AXIS in_axis );
    
    double GetRotationAngle(const unsigned long long int& in_eventNr);

    void GetPinholePosition(const unsigned long long int& in_eventNr, C3Vector& out_pinholePosition);
};

#endif

