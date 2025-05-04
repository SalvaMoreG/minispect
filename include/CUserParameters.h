#ifndef __miniSPECT_UserParameters_H___
#define __miniSPECT_UserParameters_H___

#pragma once

#include "CVIPUtils.h"

#include <string>

class CUserParameters
{
public:
	// singleton access
	static CUserParameters*		Instance();
	static void 				Destroy();

	inline std::string          GetDataFilename() const { return m_datafilename; }
    inline std::string          GetListFilename() const { return m_listfilename; }
	
	inline double				GetSourceGammaEnergy() const { return m_Esrcgamma; }
	inline double				GetEnergyResolution() const { return m_Esigma; }

	bool                        HasRectanglePhantom() const { return m_hasRectanglePhantom; }
	int                         GetRectanglePhantom_nX() const { return m_rectanglePhantom_nX; }
    int                         GetRectanglePhantom_nY() const { return m_rectanglePhantom_nY; }
    double                      GetRectanglePhantomStep() const { return m_rectanglePhantomStep; }
    
    int                         GetNumberOfPinholes() const { return m_numberOfPinholes; }
    double                      GetPinHoleXPosition() const { return m_pinholeXpos; }
    double                      GetPinHoleYPosition() const { return m_pinholeYpos; }
    double                      GetPinHoleZPosition() const { return m_pinholeZpos; }
    
    bool                        GetIfMultiplexing() const { return m_multiplexing; }
    double                      GetObjectCollimatorZDistance() const { return m_zobj; }
    double                      GetDetectorCollimatorZDistance() const { return m_zdet; }
    
    bool                        HasGaussSmoothing() const { return m_gaussSmoothing; }
    double                      GetGaussSmoothingSigma() const { return m_gaussSmoothingSigma; }
    int                         GetGaussSmoothingSize() const { return m_gaussSmoothingSize; }
    double                      GetGaussSmoothingThreshold() const { return m_gaussSmoothingThreshold; }

    int                         GetSampling() const { return m_sampling; }
    bool                        GetIfOnly3D() const { return m_only3D; }

    void                        SetDoWriteLORs() { m_writeLORs = true; }
    bool                        GetDoWriteLORs() const { return m_writeLORs; }

    // Rotation business
    void                    SetRotationAngleDeg(const double& in_angleDeg) { m_rotationAngleDeg = in_angleDeg; }
    double                  GetRotationAngleDeg() const { return m_rotationAngleDeg; }
    
    void                    SetRotationAngleAxis( AXIS in_axis ) { m_rotationAngleAxis = in_axis; }
    AXIS                    GetRotationAngleAxis( ) const { return m_rotationAngleAxis; }

    void                    SetRotationAngleDeltaN( int in_deltaN ) { m_rotationAngleDeltaN = in_deltaN; }
    int                     GetRotationAngleDeltaN() const { return m_rotationAngleDeltaN; }
    
    void                    SetRotationAngleDeltaDeg( double in_deg ) { m_rotationAngleDeltaDeg = in_deg; }
    double                  GetRotationAngleDeltaDeg() const { return m_rotationAngleDeltaDeg; }    

    void                    SetOrbitAngleDeg( const double& in_angle ) { m_orbitAngleDeg = in_angle; }
    double                  GetOrbitAngleDeg() const { return m_orbitAngleDeg; }

    void                    SetOrbitAngleAxis( AXIS in_axis ) { m_orbitAngleAxis = in_axis; }
    AXIS                    GetOrbitAngleAxis() const { return m_orbitAngleAxis; }

private:

	// constructor and destructor
								CUserParameters();
	virtual						~CUserParameters();

	// copy and assign constructors, not defined, to avoid using them without knowing
								CUserParameters(const CUserParameters& in_obj);
	CUserParameters& 			operator= (const CUserParameters& in_obj);
    
    // misc methods
    void                        ReadConfigurationParameters();
    void                        GetHelp() const;

	// static instance
	static CUserParameters*		sInstance;
    

	// Parameters
	double 						m_Esrcgamma;
    double 						m_Esigma;
    std::string                 m_listfilename;
    std::string                 m_datafilename;
    
    bool                        m_hasRectanglePhantom;
    int                         m_rectanglePhantom_nX;
    int                         m_rectanglePhantom_nY;
    double                      m_rectanglePhantomStep;
    
    int                         m_numberOfPinholes;
    double                      m_pinholeXpos;
    double                      m_pinholeYpos;
    double                      m_pinholeZpos;
    
    bool                        m_multiplexing;
    double                      m_zobj;
    double                      m_zdet;
    
    bool                        m_gaussSmoothing;
    double                      m_gaussSmoothingSigma;
    int                         m_gaussSmoothingSize;
    double                      m_gaussSmoothingThreshold;

    int                         m_sampling;
    bool                        m_only3D;

    bool                        m_writeLORs;

    double                      m_rotationAngleDeg;
    AXIS                        m_rotationAngleAxis;
    int                         m_rotationAngleDeltaN;
    double                      m_rotationAngleDeltaDeg;
    
    double                      m_orbitAngleDeg;
    AXIS                        m_orbitAngleAxis;
};

#endif




