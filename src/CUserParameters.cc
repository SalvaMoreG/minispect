
#include "CUserParameters.h"

#include <fstream>
#include <iostream>
#include <cstdlib>

using namespace std;

CUserParameters* CUserParameters::sInstance = 0;	// initialize pointer

CUserParameters*
CUserParameters::Instance()
{
	if (sInstance == NULL)
	{
		sInstance = new CUserParameters();
	}
	return sInstance;
}

void
CUserParameters::Destroy()
{
	delete sInstance;
	sInstance = NULL;
}

CUserParameters::CUserParameters()
    : m_datafilename("")
    , m_listfilename("")
	, m_Esrcgamma(0.0)
    , m_Esigma(0.0)
    , m_hasRectanglePhantom(false)
    , m_rectanglePhantom_nX(0)
    , m_rectanglePhantom_nY(0)
    , m_rectanglePhantomStep(0.0)
    , m_pinholeXpos(0.0)
    , m_pinholeYpos(0.0)
    , m_pinholeZpos(0.0)
    , m_numberOfPinholes(1)
    , m_multiplexing(false)
    , m_zobj(0.0)
    , m_zdet(0.0)
    , m_gaussSmoothing(false)
    , m_gaussSmoothingSigma(1.)
    , m_gaussSmoothingSize(3.)
    , m_gaussSmoothingThreshold(0.)
    , m_sampling(0)
    , m_writeLORs(false)
    , m_rotationAngleDeg(0.0)
    , m_rotationAngleAxis(Axis_X)
    , m_rotationAngleDeltaN(0)
    , m_rotationAngleDeltaDeg(0.0)
    , m_orbitAngleDeg(0.0)
    , m_orbitAngleAxis(Axis_X)
    , m_only3D(false)
{
	ReadConfigurationParameters();
}

CUserParameters::~CUserParameters()
{
}

void
CUserParameters::ReadConfigurationParameters()
{
	std::string conffilename = "minispect_parameters.conf";
	ifstream conffile(conffilename.c_str(), ios::in);
	if (!conffile.is_open())
	{
		cout << "Configure file not open: " << conffilename << endl;
		exit(1);
	}

	std::string tmpStr, tmpStrVal;
	double tmpVal;

	while (!conffile.eof())
    {
		conffile >> tmpStr >> tmpVal >> tmpStrVal;
		if (conffile.fail() && !conffile.eof())
		{
			cout << "ERROR! FORMAT ERROR in parameters file" << endl;
			exit(1);
		}
		if (!conffile.eof())
		{
			if (tmpStr == "sourceEnergy")
			{
				m_Esrcgamma = tmpVal;
			}
			else if (tmpStr == "energyResolutionSigma")
			{
				m_Esigma = tmpVal;
			}

			else if (tmpStr == "dataFileName")
            {
                m_datafilename = tmpStrVal;
            }
            else if (tmpStr == "listFileName")
            {
                m_listfilename = tmpStrVal;
            }

            else if (tmpStr == "phantomRectangle")
            {
                m_hasRectanglePhantom = tmpVal;
            }
            else if (tmpStr == "phantomRectangle_nX")
            {
                m_rectanglePhantom_nX = (int) tmpVal;
            }
            else if (tmpStr == "phantomRectangle_nY")
            {
                m_rectanglePhantom_nY = (int) tmpVal;
            }
            else if (tmpStr == "phantomRectangleStep")
            {
                m_rectanglePhantomStep = tmpVal;
            }
            else if (tmpStr == "numberOfPinholes")
            {
                m_numberOfPinholes = (int) tmpVal;
            }
            
            else if (tmpStr == "pinholeXpos")
            {
                m_pinholeXpos = tmpVal;
            }
            else if (tmpStr == "pinholeYpos")
            {
                m_pinholeYpos = tmpVal;
            }
            else if (tmpStr == "pinholeZpos")
            {
                m_pinholeZpos = tmpVal;
            }  
            else if (tmpStr == "gaussSmoothing")
            {
                m_gaussSmoothing = tmpVal;
            }  
            else if (tmpStr == "gaussSmoothingSigma")
            {
                m_gaussSmoothingSigma = tmpVal;
            }  
            else if (tmpStr == "gaussSmoothingSize")
            {
                m_gaussSmoothingSize = tmpVal;
            }  
            else if (tmpStr == "gaussSmoothingThreshold")
            {
                m_gaussSmoothingThreshold = tmpVal;
            } 
            else if (tmpStr == "sampling")
            {
                m_sampling = tmpVal;
            } 
            else if (tmpStr == "only3D")
            {
                m_only3D = tmpVal;
            } 
            else if (tmpStr == "multiplexing")
            {
                m_multiplexing = tmpVal;
            }
            else if (tmpStr == "ZdistanceObjColl")
            {
                m_zobj = tmpVal;
            }
            else if (tmpStr == "ZdistanceDetColl")
            {
                m_zdet = tmpVal;
            }  
            
            else if (tmpStr == "rotationAngleDegrees")
            {
                m_rotationAngleDeg = tmpVal;

                if (tmpStrVal == "X") m_rotationAngleAxis = Axis_X;
                else if (tmpStrVal == "Y") m_rotationAngleAxis = Axis_Y;
                else if (tmpStrVal == "Z") m_rotationAngleAxis = Axis_Z;
                else
                {
                    cout << "ERROR! Wrong axis: " << tmpStr << " " << tmpStrVal << endl;
                    GetHelp();
                    exit(2);
                }
            }
            else if (tmpStr == "rotationAngleDelta")
            {
                m_rotationAngleDeltaN = (int) tmpVal;
                m_rotationAngleDeltaDeg = atof( tmpStrVal.c_str() );
                cout << "Delta N: " << m_rotationAngleDeltaN 
                     << " Delta angle: " << m_rotationAngleDeltaDeg << endl;
            }
            else if (tmpStr == "orbitAngleDegrees")
            {
                m_orbitAngleDeg = tmpVal;

                if (tmpStrVal == "X") m_orbitAngleAxis = Axis_X;
                else if (tmpStrVal == "Y") m_orbitAngleAxis = Axis_Y;
                else if (tmpStrVal == "Z") m_orbitAngleAxis = Axis_Z;
                else
                {
                    cout << "ERROR! Wrong axis: " << tmpStr << " " << tmpStrVal << endl;
                    GetHelp();
                    exit(2);
                }
            }

            else if (!conffile.eof())
            {
                cout << "ERROR! Unknown parameter: " << tmpStr << endl;
                GetHelp();
                exit(2);                
            }            
		}
	}
}

void
CUserParameters::GetHelp() const
{
	cout << "Possible parameters in 'osem_parameters.conf' " << endl;
	cout << "sourceEnergy			140			//" << endl;
    cout << "energyResolutionSigma   1.7        //" << endl;
    cout << "listFileName           0           ./listLM.txt" << endl;
	cout << "dataFileName			0			./LM.out" << endl;

    cout << "phantomRectangle        1          //" << endl;
    cout << "phantomRectangle_nX     5          //" << endl;
    cout << "phantomRectangle_nY     4          //" << endl;
    cout << "phantomRectangleStep    5.0        //" << endl;
    
    cout << "numberOfPinholes        1          //" << endl;
    cout << "multiplexing            0          //" << endl;
    cout << "pinholeXpos             0          //" << endl;
    cout << "pinholeYpos             0          //" << endl;
    cout << "pinholeZpos            15          //" << endl;
    
    cout << "ZdistanceObjColl        15          //" << endl;
    cout << "ZdistanceDetColl        15          //" << endl;
    
    cout << "gaussSmoothing           1          //" << endl;
    cout << "gaussSmoothingSigma      1.         //" << endl;
    cout << "gaussSmoothingSize       3          //" << endl;
    
    cout << "sampling              1000          //" << endl;
    cout << "only3D                0             //" << endl;

    cout << "rotationAngleDegrees    30          X" << endl;
    cout << "rotationAngleDeltaN    10000        //" << endl;
    cout << "orbitAngleDegrees       45          Y" << endl;
}






