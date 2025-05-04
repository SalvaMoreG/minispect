
#ifndef __miniSPECT_SpectImage_H___
#define __miniSPECT_SpectImage_H___

#include "CMyTH2D.h"
#include <math.h>
#include <cmath>
#include <string>

#include <vector>
#include <map>
#include <memory>
#include "TH1D.h"
#include "CVIPImage.h"
#include "CVIPFieldOfView.h"

class CSpectImage
{
public:
					CSpectImage();
	virtual			~CSpectImage();

	// 
    int             ReadLMAndConvert(bool doECut);
    void            Save3D() const;
    void            Project2Dcapas();
    void            Project2Dobject(C3Vector in_vector, std::map<int, std::vector<double>>& in_positions); 
    void            GaussianSmoothing(int templateSize, double sigma, double& numberOfHitsNotLost);
    
    // Get and set
    // MyTH2D*         Get2DImage() const { return m_hXY; }
    const std::vector<MyTH2D*>&         Get2DImages() const { return m_2Dhistoos; }
    
    MyTH2D*         GetSummed2DHist() const { return m_summed2D; }
    MyTH2D*         GetObject2DHist() const { return m_object2D; }
    MyTH2D*         GetSmooth2DHist() const { return m_smoothed2D; }

    TH1D*           GetEnergySpectrum() const { return m_hE.get(); }
        // unique_ptr 
        // To pass a std::unique_ptr as a (const) pointer, you can use the get() member function of std::unique_ptr,      
        //      which returns the raw pointer managed by the unique pointer. 
        // This allows you to pass it as a (const) pointer.
        // (https://www.quora.com/How-do-I-pass-in-a-unique_ptr-variable-as-a-const-pointer)
        // Note that "m_hE->" directly connects to the raw pointer managed by the unique pointer

private:
    
    // Private methods...
    void            CreateLORFromSinglePinhole(const C3Vector& in_hitPos, 
                         const double& in_E_MeV, const unsigned long long int& in_eventNr );
    void            CreateLORFromUSPECT(const C3Vector& in_hitPos, const std::string& pixelType, const double& in_E_MeV );
    void            WriteOutLOR(const C3Vector& in_hitPos, const C3Vector& in_2ndHitPos, 
                         const double& in_E_MeV);
    
    void            CalculateHoles(std::map<int, std::vector<double>>& out_positions);
    void            DecideHole(C3Vector in_vector, int& out_hole);
    std::vector<std::vector<double>>    CreateTemplate(int templateSize, double sigma);
    void            SmoothBin(int binX, int binY, std::vector<std::vector<double>> in_template, double& out_value);
    
    std::vector<MyTH2D*>   m_2Dhistoos;
    MyTH2D*         m_summed2D;
    MyTH2D*         m_object2D;
    MyTH2D*         m_smoothed2D;

    std::unique_ptr<TH1D>      m_hE;           // Energy spectrum (TODO: strange place to keep this)
    
    std::unique_ptr<CVipImage> m_spectImage;   // 3D image of the hits in the scanner
        // Note: when we do tomography, using CC_OSEM, we will also have a 3D image of 
        // the backprojected image (i.e. of the source space).
        // Note(2): Using a unique_ptr or shared_ptr (since C++11), we don't have to worry about memory leaks anymore
        // Reserved space gets deleted automatically when the ptr is not used anywhere in the code anymore
        
    CVIPFieldOfView* m_spectDimensions;
    std::ofstream    m_LORfile;
};


#endif
