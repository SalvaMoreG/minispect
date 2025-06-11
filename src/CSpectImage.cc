
#include "CSpectImage.h"
#include "CUserParameters.h"
#include "SpectUtils.h"

#include <iostream>
#include <fstream>
#include <string>
#include <cassert>
#include <cstdlib>
#include <time.h>

#include "TString.h"

using namespace std;

bool first(true);

// ===============00

CSpectImage::CSpectImage()
    : m_spectDimensions(NULL)
    , m_summed2D(NULL)
    , m_object2D(NULL)
    , m_smoothed2D(NULL)

{
    m_spectDimensions = new CVIPFieldOfView();
    m_spectDimensions->Initialize( "minispect_fov_parameters.conf" );
    
    //  m_hE = new TH1D("hE", "E", 200, 0.0, 200);  
    // Using unique_ptr (or shared_ptr) ("smart reference pointer").
    m_hE = make_unique<TH1D>("hE", "E", 200, 0.0, 0.0);  
		// open-ended limits (so root will decide what's the min and max)
    
    // Det size of 2D histo from parameters...
    m_2Dhistoos.clear();
    int Ncapas(2);
    if (m_spectDimensions->GetNumVoxelsZ() > 0)
        Ncapas = m_spectDimensions->GetNumVoxelsZ();
    MyTH2D* hXY(NULL);
    for (int icapa = 0; icapa < Ncapas; icapa++)
    {
        TString rootstr("hXY_");
        rootstr+= icapa;
        hXY = new MyTH2D(rootstr,  rootstr,
                m_spectDimensions->GetNumVoxelsX(), 
                     m_spectDimensions->GetLowerEdge().GetX(), 
                     m_spectDimensions->GetUpperEdge().GetX(),
                m_spectDimensions->GetNumVoxelsY(), 
                     m_spectDimensions->GetLowerEdge().GetY(), 
                     m_spectDimensions->GetUpperEdge().GetY());
        m_2Dhistoos.push_back(hXY);
    }
    m_summed2D = new MyTH2D("summedXY", "summedXY", 
                m_spectDimensions->GetNumVoxelsX(), 
                     m_spectDimensions->GetLowerEdge().GetX(), 
                     m_spectDimensions->GetUpperEdge().GetX(),
                m_spectDimensions->GetNumVoxelsY(), 
                     m_spectDimensions->GetLowerEdge().GetY(), 
                     m_spectDimensions->GetUpperEdge().GetY());   
    m_object2D = new MyTH2D("objectXY", "objectXY", 
                m_spectDimensions->GetNumVoxelsX(), 
                     m_spectDimensions->GetLowerEdge().GetX(), 
                     m_spectDimensions->GetUpperEdge().GetX(),
                m_spectDimensions->GetNumVoxelsY(), 
                     m_spectDimensions->GetLowerEdge().GetY(), 
                     m_spectDimensions->GetUpperEdge().GetY()); 
    m_smoothed2D = new MyTH2D("smoothedXY", "smoothedXY", 
                m_spectDimensions->GetNumVoxelsX(), 
                     m_spectDimensions->GetLowerEdge().GetX(), 
                     m_spectDimensions->GetUpperEdge().GetX(),
                m_spectDimensions->GetNumVoxelsY(), 
                     m_spectDimensions->GetLowerEdge().GetY(), 
                     m_spectDimensions->GetUpperEdge().GetY()); 
    
    // Get image size from parameters 
    //      m_spectImage = new CVipImage( m_spectDimensions->GetNumberOfVoxels() );
    // Using unique_ptr (or shared_ptr) ("smart reference pointer").
    m_spectImage = make_unique<CVipImage>( m_spectDimensions->GetNumberOfVoxels() );
    
    if ( CUserParameters::Instance()->GetDoWriteLORs() )
    {
        m_LORfile.open("spect_LORS.dat_NEW");
    }
}

CSpectImage::~CSpectImage()
{
    // deleting goes automatic with unique_ptr or shared_ptr
    //  delete m_hE;            // Not necessary anymore (and not possible on a unique_ptr or shared_ptr)
    //  delete m_spectImage;    // Not necessary anymore (and not possible on a unique_ptr or shared_ptr)
    delete m_spectDimensions;
    
    for (int icapa = 0; icapa < m_2Dhistoos.size(); icapa++)
    {
        MyTH2D* hXY = m_2Dhistoos[icapa];
        delete hXY;
    }
    delete m_summed2D;
    delete m_object2D;    
    delete m_smoothed2D;  
}

// =================================00==

int
CSpectImage::ReadLMAndConvert(bool doECut)
{
    bool debug(false);
	CUserParameters* parameters = CUserParameters::Instance();

    ifstream listfile;
    ifstream ffile;
    if ( parameters->GetListFilename().size() > 3 )
    {
        listfile.open( parameters->GetListFilename().c_str() );
        if ( !listfile.is_open() )
        {
            cout << "file does not exist: " << parameters->GetListFilename() << endl;
            exit(1);
        }
    }
    else
    {
        ffile.open( parameters->GetDataFilename().c_str() );
        if ( !ffile.is_open() )
        {
            cout << "file does not exist: " << parameters->GetDataFilename() << endl;
            exit(1);
        }
    }
    
	string pixelType;
	double E, x, y, z, radius;
	unsigned long long int time_ps;
    unsigned long long int eventNr;
	unsigned int count(0);
    
	double Etarget = parameters->GetSourceGammaEnergy();
	double Emargen = doECut ? 3.0 * parameters->GetEnergyResolution() : -1;
    
    double zmin = m_spectDimensions->GetLowerEdge().GetZ();
    double zmax = m_spectDimensions->GetUpperEdge().GetZ();    
    double zcapasize = (zmax - zmin)/m_2Dhistoos.size();
    
    std::map<int, std::vector<double>> hole_positions;
    CalculateHoles(hole_positions);
    
    bool sampled(false);
    int sampling = parameters->GetSampling();

    while ( (listfile.is_open() && !listfile.eof()) || !ffile.eof())
    {
        if (listfile.is_open())
        {
            if (ffile.is_open()) // previous loop
                ffile.close();

            string fname;
            listfile >> fname;
            if (fname.size() > 3)
            {
                ffile.open( fname.c_str() );
                if ( !ffile.is_open() )
                {
                    cout << "file does not exist: " << fname << endl;
                    exit(1);
                }
                cout << "Reading file: " << fname << endl;
            }
        }
        while ( !ffile.eof())
        {
            ffile >> eventNr >> pixelType >> time_ps >> E >> x >> y >> z;
            ffile.ignore(1024, '\n');
            if ( !ffile.eof() )
            {
                if (m_hE) m_hE->Fill(E*1000.0);	// MeV to keV

                // if no E cut OR if E(keV) - Etarget < Emargen, fill XY map and line-profiles
                if ( Emargen < 0 || fabs(E*1000.0 - Etarget) < Emargen)
                {
                    if ( sampling != 0 && count <  sampling)
                    {
                        sampled = true;
                    }
                    else
                    {
                        sampled = false;
                    }
                    count++;

                    if (sampled || sampling == 0)
                    {
                        // Save the original x, y, z coordinates for all 3D projections into object space
                        //
                        C3Vector hitPos(x, y, z);   // used for 3D projections...
                        if (debug)
                            cout << "hitPos: " << hitPos << endl;

                        //
                        C3Vector projectedScanner2DPos(x, y, z);
                        if ( fabs(parameters->GetOrbitAngleDeg()) > 0.001 )
                        {
                            // If the orbit is A degrees, we have to rotate back with -A degrees
                            //
                            SpectUtils::RotatePosition(hitPos, projectedScanner2DPos,
                                                -1.0*parameters->GetOrbitAngleDeg()
                                                , parameters->GetOrbitAngleAxis());
                            if (debug)
                                cout << "rotated back Pos: " << projectedScanner2DPos << endl;
                        }
                        if (    fabs(parameters->GetRotationAngleDeg()) > 0.001
                            || fabs(parameters->GetRotationAngleDeltaDeg()) > 0.001 )
                        {
                            // Angle = the initial rotation angle, plus the event dependent delta angle
                            double angle = SpectUtils::GetRotationAngle(eventNr);

                            // If in GAMOS we rotated with 30 degrees, we have to rotate back with -30 degrees
                            //
                            C3Vector tmpPos( projectedScanner2DPos );
                            SpectUtils::RotatePosition(tmpPos, projectedScanner2DPos,
                                                -1.0*angle, parameters->GetRotationAngleAxis());
                            if (debug)
                                cout << "rotated back Pos: " << projectedScanner2DPos << endl;
                        }
                        
                        if (!parameters->GetIfOnly3D())
                        {
                            // file the ROOT 2D histogram
                            // the first layer. TODO: get icapa from "z" w.r.t to zmin and zmax
                            double relz = (projectedScanner2DPos.GetZ() - zmin);
                            int icapa = (int) relz/zcapasize;
                            if (icapa < 0 || icapa >= m_2Dhistoos.size())
                            {
                                cout << "ERROR: z = " << z << " icapa: " << icapa << endl;
                            }
                            assert(icapa >= 0 && icapa < m_2Dhistoos.size());

                            // Fill the 2D histograms
                            // if the scanner is rotated, use the coordinates that are rotated back
                            // (i.e. If in GAMOS we rotated with 30 degrees, we rotated back with -30 degrees))

                            MyTH2D* hXY = m_2Dhistoos[icapa];
                            if (hXY) hXY->Fill(projectedScanner2DPos.GetX(), projectedScanner2DPos.GetY());
                            //
                            // Projection onto 3D object space
                            //
                            
                            if ( CUserParameters::Instance()->GetDoWriteLORs() &&
                                parameters->GetNumberOfPinholes() == 0 )
                            {
                                CreateLORFromUSPECT(hitPos, pixelType, E);
                            }
                        
                            if(!parameters->GetIfMultiplexing() && parameters->GetNumberOfPinholes()>0)
                            {
                                Project2Dobject(hitPos, hole_positions);

                                if ( parameters->GetDoWriteLORs() )
                                {
                                    CreateLORFromSinglePinhole(hitPos, E, eventNr);                                
                                }
                            }
                            
                            //
                            // fill the 2D image with the back-rotated Positions...
                            //
                            if (m_spectDimensions && m_spectImage)
                            {
                                int voxelIdx = m_spectDimensions->GetVoxelIndex( projectedScanner2DPos );
                                double value = m_spectImage->GetVoxelValue( voxelIdx ) + 1; // add a count
                                m_spectImage->SetVoxelValue( voxelIdx, value );
                            }
                        }
                        else
                        {
                            if ( parameters->GetDoWriteLORs() && parameters->GetNumberOfPinholes() == 0 )
                            {
                                CreateLORFromUSPECT(hitPos, pixelType, E);
                            }
                        }
                        
                    }
                }
            }
        }
	}
	return count;
}

// =================================00==

void
CSpectImage::Save3D() const
{
    string outfname("spect_image.img_NEW");
    m_spectImage->Write( outfname, *m_spectDimensions, FFORMAT_BINARY);
}

// =================================00==

void
CSpectImage::Project2Dcapas() 
{
    CUserParameters* parameters = CUserParameters::Instance();
    
    C3Vector myPosition;
    double dX, dY, dZ;
    double dXcapa, dYcapa, dZcapa;
    
    int Ncapas = m_spectDimensions->GetNumVoxelsZ();
    assert( m_2Dhistoos.size() == Ncapas );
    
    double spect_zmin = m_spectDimensions->GetLowerEdge().GetZ();
    double spect_zmax = m_spectDimensions->GetUpperEdge().GetZ();
    double capa_zsize = (spect_zmax - spect_zmin)/Ncapas;
    double capaZpos(0.0);
    double newX(0.0), newY(0.0);
    
    // Loop over remaining capas
    //
    for (int icapa = 0; icapa < m_2Dhistoos.size(); icapa++)
    {
        MyTH2D* hXY = m_2Dhistoos[icapa];
        for (int iY = 1; iY < hXY->GetNbinsY()-1; iY++)   // skip the edges
        {
            for (int iX = 1; iX < hXY->GetNbinsX()-1; iX++)   // skip the edges
            { 
//              if icapa == 0, use it as it is (no projection)
//              if (icapa > 0): 
//                     - Get the X,Y,Z position of this bin
//                     - look at the Z position (i.e. the radial position) of this layer
//                     - deltaZ = Zpos(icapa) - Zpos(pinhole)
//                     - deltaX = Xpos(bin) - Xpos(pinhole)
//                     - deltaY = Ypos(bin) - Ypos(pinhole)
//                     - deltaZcapa = Zpos(icapa) - Zpos(icapa=0)
//                     - deltaXcapa = DeltaX/deltaZ * deltaZcapa
//                     - deltaYcapa = DeltaY/deltaZ * deltaZcapa
//                     - newX = X - deltaXcapa
//                     - newY = Y - deltaYcapa
//                     - find bins in "summed2Dhisto" and add contents of current capa histo                
//
                int newcontent = hXY->GetBinContent(iX, iY);
                if (icapa == 0 && newcontent > 0)
                {
                    //  newcontent += m_summed2D->GetBinContent(iX, iY);
                    //  m_summed2D->SetBinContent(iX, iY, newcontent);
                    //
                    C3Vector newpos;
                    m_summed2D->GetPosition(iX, iY, newpos);
                    m_summed2D->Fill(newpos.GetX(), newpos.GetY(), newcontent);
                        // Fill automatically adds
                }
                else if ( newcontent > 0)
                {
                    capaZpos = spect_zmin + (icapa+0.5)*capa_zsize;
                    dZ = capaZpos - parameters->GetPinHoleZPosition();
                    hXY->GetPosition(iX, iY, myPosition);
                    dX = myPosition.GetX() - parameters->GetPinHoleXPosition();
                    dY = myPosition.GetY() - parameters->GetPinHoleYPosition();
                    dZcapa = icapa*capa_zsize;
                    dXcapa = (dX/dZ)*dZcapa;
                    dYcapa = (dY/dZ)*dZcapa;
                    
                    newX = myPosition.GetX() - dXcapa;
                    newY = myPosition.GetY() - dYcapa;
                    
                    /*
                    myPosition.Set(newX, newY, 1);
                    int newbinX, newbinY;
                    hXY->GetBins(myPosition, newbinX, newbinY );
                    
                    newcontent += m_summed2D->GetBinContent(newbinX, newbinY);
                    m_summed2D->SetBinContent(newbinX, newbinY, newcontent);
                    */

                    m_summed2D->Fill(newX, newY, newcontent);
                        // Fill automatically adds
                }
            }
        }
        
    }    
}

// =================================00==

void
CSpectImage::Project2Dobject(C3Vector in_vector, std::map<int, std::vector<double>>& in_positions) 
{
    CUserParameters* parameters = CUserParameters::Instance();
    double zobj = parameters->GetObjectCollimatorZDistance();
    double zdet = parameters->GetDetectorCollimatorZDistance();
    double zmin = m_spectDimensions->GetLowerEdge().GetZ();

    int holeKey = 0;
    DecideHole(in_vector, holeKey);
    double x = in_vector.GetX();
    double y = in_vector.GetY();
    double z = in_vector.GetZ();

    std::vector<double> holePos;
    holePos = in_positions[holeKey];
    double x_hole = holePos[0];
    double y_hole = holePos[1];
    double z_hole = zmin - zdet;
    
    double z_proj = zmin - zdet - zobj;
    double x_proj = (x_hole - x)*(z_proj - z)/(z_hole - z) + x;
    double y_proj = (y_hole - y)*(z_proj - z)/(z_hole - z) + y;
        
    m_object2D->Fill(x_proj, y_proj);
}

// =================================00==

void
CSpectImage::CalculateHoles(std::map<int, std::vector<double>>& out_positions) 
{
    CUserParameters* parameters = CUserParameters::Instance();
    int numberOfPinholes = parameters->GetNumberOfPinholes();
    double xmin = m_spectDimensions->GetLowerEdge().GetX();
    double xmax = m_spectDimensions->GetUpperEdge().GetX();  
    double projectionRadius = (xmax-xmin)/(2*numberOfPinholes);
    for(int i = 0; i<numberOfPinholes; i++)
    {
        for(int j = 0;j<numberOfPinholes; j++)
        {
            std::vector<double> v = {xmin+projectionRadius+2*projectionRadius*i, 
                                     xmin+projectionRadius+2*projectionRadius*j};
            out_positions[i*numberOfPinholes + j] = v;
        }
    }
}

// =================================00==

void
CSpectImage::DecideHole(C3Vector in_vector, int& out_hole) 
{
    CUserParameters* parameters = CUserParameters::Instance();
    int numberOfPinholes = parameters->GetNumberOfPinholes();
    double xmin = m_spectDimensions->GetLowerEdge().GetX();
    double xmax = m_spectDimensions->GetUpperEdge().GetX();  
    double projectionRadius = (xmax-xmin)/(2*numberOfPinholes);
    double x = in_vector.GetX();
    double y = in_vector.GetY();
    out_hole = 0;
    
    for(int i = 0; i<numberOfPinholes; i++)
    {
        for(int j = 0;j<numberOfPinholes; j++)
        {
            if (x > xmin+2*projectionRadius*i && x < xmin+2*projectionRadius*(i+1) &&
                y > xmin+2*projectionRadius*j && y < xmin+2*projectionRadius*(j+1))
            {
                out_hole = i*numberOfPinholes + j;
                break;
            }
        }
    }
}

// =================00===

void
CSpectImage::GaussianSmoothing(int templateSize, double sigma, double& numberOfHitsNotLost )
{
    CUserParameters* parameters = CUserParameters::Instance();
    std::vector<std::vector<double>> out_template = CreateTemplate(templateSize, sigma);
    double xmin = m_spectDimensions->GetLowerEdge().GetX();
    double xmax = m_spectDimensions->GetUpperEdge().GetX();  
    double value(0.), valueSampled(0.), x(0.), y(0.);
    C3Vector position;
    int edgeSkip = (templateSize-1)/2;
    double threshold = parameters->GetGaussSmoothingThreshold();
    double entries_filtered(0.);
    m_smoothed2D->Reset("ICESM");
    for (int iY = edgeSkip; iY < m_smoothed2D->GetNbinsY()-edgeSkip; iY++)   // skip the edges
    {
        for (int iX = edgeSkip; iX < m_smoothed2D->GetNbinsX()-edgeSkip; iX++)   // skip the edges
        {
            SmoothBin(iX, iY, out_template, value);
            m_smoothed2D->GetPosition(iX, iY, position);
            x = position.GetX();
            y = position.GetY();
            if (value > threshold)
            {
                m_smoothed2D->Fill(x, y, value);
                entries_filtered += value;
            }
            value = 0.;
        }
    }
    cout << "Template: " << endl;
    for(int x = 0; x < templateSize; x++)
    {
        for(int y = 0; y < templateSize; y++)
        {
            cout << out_template[x][y] << " ";
        }
        
        cout << endl;
    }
    
    numberOfHitsNotLost = entries_filtered/m_object2D->GetEntries()*100;
}


// =================00===
std::vector<std::vector<double>>
CSpectImage::CreateTemplate(int templateSize, double sigma)
{
    std::vector<std::vector<double>> out_template;
    double center = (templateSize-1)/2;
    double total(0.), x_pos(0.), y_pos(0.);
    double ssigma = 2*sigma*sigma;
    double value;
    for(int x = 0; x < templateSize; x++)
    {
        x_pos = x*m_spectDimensions->GetVoxelSize().GetX();
        std::vector<double> vectorY;

        for(int y = 0; y < templateSize; y++)
        {
            y_pos = y*m_spectDimensions->GetVoxelSize().GetY();
            value = (1/(ssigma*M_PI))*exp(-((x_pos-center)*(x_pos-center)+(y_pos-center)*(y_pos-center))/ssigma); 
            vectorY.push_back(value);
            total+=value;
        }
        out_template.push_back(vectorY);
    }
    
    for(int x = 0; x < templateSize; x++) 
    {
        for(int y = 0; y < templateSize; y++)
        {
            out_template[x][y] = out_template[x][y]/total;
        }
    }
    
    return out_template;
}


// =================00===

void
CSpectImage::SmoothBin(int binX, int binY, std::vector<std::vector<double>> in_template, double& out_value)
{
    double sumValue(0.), binValue(0.);
    int n = in_template.size();
    for ( int i = 0; i < n; i++ )
    {
        int x_skip = -(n-1)/2 + i;
        for ( int j = 0; j < n; j++ )
        {
            int y_skip = -(n-1)/2 + j;
            int bin = m_object2D->GetBin(binX+x_skip, binY+y_skip);
            binValue = m_object2D->GetBinContent(bin);
            out_value += binValue*in_template[i][j];
        }
    }
}


// =================00===

void
CSpectImage::CreateLORFromSinglePinhole(const C3Vector& in_hitPos, 
    const double& in_E_MeV, const unsigned long long int& in_eventNr )
{
    CUserParameters* parameters = CUserParameters::Instance();
    int numberOfPinholes = parameters->GetNumberOfPinholes();
    if (numberOfPinholes != 1)
    {
        cout << "WARNING! LORs only work for 1 single pinhole. Current #pinholes: " 
             << numberOfPinholes << endl;
        cout << "No LORs written out" << endl;
        return;
    }

    C3Vector pinholePos;
    SpectUtils::GetPinholePosition(in_eventNr, pinholePos); // Might be rotated...

    double dx = in_hitPos.GetX() - pinholePos.GetX();
    double dy = in_hitPos.GetY() - pinholePos.GetY();
    double dz = in_hitPos.GetZ() - pinholePos.GetZ();;

    double dxdz = dx/dz;
    double dydz = dy/dz;
    double dzdz = 1.0;

    double lambda = 10.0; // seems large enough
    dz = lambda*dz; // new distance over Z

    C3Vector fakeHit;
    fakeHit.Set(in_hitPos.GetX() - dxdz*dz,
                in_hitPos.GetY() - dydz*dz,
                in_hitPos.GetZ() - dzdz*dz);

    // Write out
    WriteOutLOR(in_hitPos, fakeHit, in_E_MeV);
}

// =================00===

void
CSpectImage::CreateLORFromUSPECT(const C3Vector& in_hitPos, const string& pixelType, 
    const double& in_E_MeV )
{  
    C3Vector fakeHit1, fakeHit2; 
    //Bottom pixel
    if (pixelType == "CdTe_pixel_b")
    {
        fakeHit1.Set ( in_hitPos.GetX(), in_hitPos.GetY(),
                in_hitPos.GetZ() ); //Hit position
        fakeHit2.Set( in_hitPos.GetX(), in_hitPos.GetY(),
                     in_hitPos.GetZ() + 100 ); //Vertical long enough fake position
    }
    else //Side pixel 
    {
        fakeHit1.Set( in_hitPos.GetX() - 100, in_hitPos.GetY(),
                     in_hitPos.GetZ() ); //Left long enough fake position 
        fakeHit2.Set( in_hitPos.GetX() + 100, in_hitPos.GetY(),
                     in_hitPos.GetZ() ); //Right long enough fake position
    }
    WriteOutLOR(fakeHit1, fakeHit2, in_E_MeV);
}

// =================00===

void
CSpectImage::WriteOutLOR(const C3Vector& in_hitPos, const C3Vector& in_2ndHitPos, 
                         const double& in_E_MeV) 
{
    // Format LOR coinc file (input for CC_OSEM): 
    // -19 -113.825 -76.5169 519.934     -31   135.844 18.4777 502.054
    //  z1   y1       x1      e1          z2    y2      x2      e2;
    // positions in mm, energies in keV
    // 
    m_LORfile << in_hitPos.GetZ() << " " << in_hitPos.GetY() << " " << in_hitPos.GetX() 
              << " " << in_E_MeV*1000.0
              << "    "
              << in_2ndHitPos.GetZ() << " " << in_2ndHitPos.GetY() << " " << in_2ndHitPos.GetX() 
              << " " << in_E_MeV*1000.0 << endl;    
}

