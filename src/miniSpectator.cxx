
#include <iostream>
#include <vector>
#include <cassert>
#include <string>
#include <fstream>
#include <iomanip>

#include "CUserParameters.h"
#include "CPhantomPositions.h"
#include "CSpectImage.h"

#include "CVIP3Vector.h"

#include "TCanvas.h"
#include "TF1.h"
#include "TH2D.h"
#include "TStyle.h"
#include "TPaveStats.h"
#include "TFile.h"

using namespace std;

struct TableData {
    double x_Res, y_Res, height_mu, height_sigma, dev_mu, dev_sigma, hitsNotLost;
    int peaksFound, peaksTotal;
    bool gaussian;
};

void parseInputFlags(int argc, char *argv[]);
void assignTableData(TableData* tableData, MyTH2D* in_hXY);
void getOptimalGaussValues(TableData* tableData, CSpectImage* spectImage, int& size_optimal, double& sigma_optimal);
inline const char * const BoolToString(bool b);

double GetDistance(const C3Vector& in_v1, const C3Vector& in_v2);

// ==================================

int main( int argc, char *argv[] )
{
	gStyle->SetOptFit();

    // Get the user parameters
	CUserParameters* parameters = CUserParameters::Instance();
    
    // Get the user flags
	if (argc > 1)
	{
	    parseInputFlags(argc, argv);
	}

    std::vector<C3Vector> known_peaks_positions;

    // Not sure how useful this
    // In principle, we can calculate the true positions for each of the phantom sources...
    CSimplePhantomPositions phantomPositions;

    // Only in the case of a simple rectangle phantom
    if (parameters->HasRectanglePhantom())
    {
        phantomPositions.GetTruePositions( known_peaks_positions, true );
    }

	TCanvas* m_XY = new TCanvas("m_XY", "XY", 1200, 600);
	TCanvas* m_lp = new TCanvas("m_lp", "line profiles", 800, 800);

    // Create the SPECT image
    CSpectImage spectImage;
    bool doECut(true);
    int totalEventsECut = spectImage.ReadLMAndConvert(doECut);
    // MyTH2D* my_hXY = spectImage.Get2DImage();
    std::vector<MyTH2D*> my2Dhistoos = spectImage.Get2DImages();
    
    // Save the 3D spect image
    spectImage.Save3D();

    // Find the peaks in the 2D XY histogram
    //
    std::vector<int> found_peaks_bins;
    std::map<int, std::vector<int> > found_peaks_bins_map;
    int hsize = my2Dhistoos.size();
    for (int icapa = 0; icapa < hsize; icapa++)
    {
        MyTH2D* hXY = my2Dhistoos[icapa];
        phantomPositions.FindPositionsIn2DHisto( hXY, found_peaks_bins, true );
        found_peaks_bins_map[icapa] = found_peaks_bins;
    }


    // ===================================

	if (parameters->HasRectanglePhantom() && found_peaks_bins.size() != known_peaks_positions.size() )
	{
		cout << "WARNING WARNING WARNING !!!" << endl;
		cout << "We found " << found_peaks_bins.size() << " peaks, "
			 << "instead of: " << known_peaks_positions.size() << endl;
	}

	// Print the histograms
    //
    TCanvas* m_E = new TCanvas("m_1", "E (keV)", 600, 600);

    TH1D* hE = spectImage.GetEnergySpectrum();
	double Etarget = parameters->GetSourceGammaEnergy();
	double Emargen = doECut ? 3.0 * parameters->GetEnergyResolution() : -1;
    
	double Emin(Etarget - 10.0);
	double Emax(Etarget + 10.0);
	
    
	m_E->cd();
	hE->GetXaxis()->SetTitle("Energy [keV]");
	hE->GetXaxis()->SetTitleSize(0.04);
	hE->GetXaxis()->SetLabelSize(0.04);
	hE->GetYaxis()->SetLabelSize(0.04);
	hE->Draw();	

	// 2025-04-28: The next lines I commented out
	// 	Apparantly, doing a fit will make it later impossible to 
	//	open the miniHistograms.root file and do hE->Draw()
	// 	(It will cause a segmentation fault)
	// 	In any case, it's better to do the fit when opening the root file
	// 	because it will give more control on the fit limits etc...
	/*
	TF1* fitGauss = ((TF1 *)(gROOT->GetFunction("gaus")));
	double initPars[3] = {11, Etarget, 0.51};	
	fitGauss->SetParameters(initPars); 		// to get rid of stupid ROOT warning, make this call
    int count = hE->GetEntries();
	fitGauss->SetParLimits(0, 10, count);
	fitGauss->SetParLimits(1, Emin, Emax);
	fitGauss->SetParLimits(2, 0.5, 10);
	hE->Fit("gaus","BQ","", Emin, Emax);
	*/
    m_E->Print("image_E.png", "png");

    // --------------

	m_XY->cd();
    m_XY->Divide(my2Dhistoos.size(), 1);
    for (int icapa = 0; icapa < my2Dhistoos.size(); icapa++)
    {
        m_XY->cd(icapa+1);
        MyTH2D* hXY = my2Dhistoos[icapa];
        hXY->GetXaxis()->SetTitle("x [mm]");
        hXY->GetXaxis()->SetTitleSize(0.04);
        hXY->GetYaxis()->SetTitle("y [mm]");
        hXY->GetYaxis()->SetTitleSize(0.04);
        hXY->GetXaxis()->SetLabelSize(0.04);
        hXY->GetYaxis()->SetLabelSize(0.04);
        gPad->SetLeftMargin(0.15);
        gPad->SetRightMargin(0.20);
        hXY->Draw("COLZ");
    }
    m_XY->Print("image_XY.png", "png");
    
    // Project capas onto one
    //
    spectImage.Project2Dcapas();
    MyTH2D* summedXY = spectImage.GetSummed2DHist();
    TCanvas* m_XYsum = new TCanvas("m_XYsum", "  ", 700, 700);
    summedXY->Draw("COLZ");
    m_XYsum->Print("image_XYsummed.png", "png");
    
    MyTH2D* objectXY = spectImage.GetObject2DHist();
    TCanvas* m_XYobj = new TCanvas("m_XYobj", "  ", 700, 700);
    objectXY->Draw("COLZ");
    m_XYobj->Print("image_XYobj.png", "png");
    
    int sampling = parameters->GetSampling();

    std::vector<C3Vector> peakPositions;
    std::vector<C3Vector> RealpeakPositions;
    
    std::map<std::string, TableData*> TableMap;   
    TableData* objectData = new TableData;
    TableMap["OD"] = objectData;
    
    cout << "Assigning object data" << endl;
    if ( parameters->HasRectanglePhantom() )
    {
        objectData->hitsNotLost = 100;
        assignTableData(objectData, objectXY);
    }
    
    MyTH2D* smoothXY(0);
    if ( parameters->HasGaussSmoothing() )
    {
        TableData* objectSmoothedData = new TableData;
        TableMap["OSD"] = objectSmoothedData;
        
        int template_size = parameters->GetGaussSmoothingSize();
        double sigma = parameters->GetGaussSmoothingSigma();
        int template_size_optimal(0);
        double sigma_optimal(0.);

        TableData* tableDataGauss = new TableData;
        getOptimalGaussValues(tableDataGauss, &spectImage, template_size_optimal, sigma_optimal);
        
        cout << "Optimal Values: ( " << template_size_optimal << " , " << sigma_optimal << " ) " << endl;
        spectImage.GaussianSmoothing(template_size_optimal, sigma_optimal, objectSmoothedData->hitsNotLost);

        smoothXY = spectImage.GetSmooth2DHist();
        TCanvas* m_XYsmooth = new TCanvas("m_XYsmooth", "  ", 700, 700);
        smoothXY->Draw("COLZ");
        m_XYsmooth->Print("image_XYsmooth.png", "png");
        
        if ( parameters->HasRectanglePhantom() )
        {
            assignTableData(objectSmoothedData, smoothXY);
        }
    }
    
    if ( parameters->HasRectanglePhantom() )
    {
        cout << "Creating table" << endl; 

        std::string map_string;
        ofstream outTableFile("Table.dat");
        using std::setw;
        outTableFile << std::left;
        
        outTableFile << setw(8) << "DT" << setw(16) << "HitsNotLost (%)" << setw(8) <<  "#Peaks" << setw(30) << "Res. (x, y)" << setw(30) << "Height (mu, sig)" << setw(30) << "Dev. (mu, sig)" << setw(8) << "Gaussian?" << endl ;
        
        for (std::map<std::string, TableData*>::iterator it = TableMap.begin(); it != TableMap.end(); it++)
        {
            outTableFile    << setw(8) << it->first 
                            << setw(16) << it->second->hitsNotLost << setw(8)
                            << std::to_string(it->second->peaksFound) + "/" + std::to_string(it->second->peaksTotal) << setw(30)
                            << std::to_string(it->second->x_Res) + " , " + std::to_string(it->second->y_Res) << setw(30)
                            << std::to_string(it->second->height_mu) + " , " + std::to_string(it->second->height_sigma) << setw(30)
                            << std::to_string(it->second->dev_mu) + " , " + std::to_string(it->second->dev_sigma)
                            << setw(8) << BoolToString(it->second->gaussian) << endl;
        }
        
        if (parameters->GetSampling() == 0)
        {
            int totalEvents = 50000000;
            double sensitivity = (objectXY->GetEntries()/(2*totalEvents))*100;
            outTableFile << "Sensitivity (%): " << sensitivity << "\t";
            for (int icapa = 0; icapa < hsize; icapa++)
            {
                outTableFile << "Layer_" + std::to_string(icapa) + ": " << (my2Dhistoos[icapa]->GetEntries()/(2*totalEvents))*100/sensitivity << "\t";
            }
            outTableFile << "Total events: " << totalEventsECut  <<endl;
        }
        
        cout << "Table Created" << endl;         
    }  
    
	/*
	std::unique_ptr<TH1D> h1b 
		= make_unique<TH1D>("h1b", "E", 200, 0.0, 0.0); 
 	for (int icol = 0; icol < 10; icol++) 
 		h1b->Fill(10*icol+0.5, (double)(100-icol));
	TH1D* h1bptr = h1b.get();
	h1bptr->Fit("gaus");	
		// testing that Fitting screws up h1b->Draw() in miniHistograms.root_NEW
	*/
    if (objectXY || smoothXY)
    {
        TFile outRootFile("miniHistograms.root_NEW", "RECREATE");
        // outRootFile.cd();
        if (objectXY) objectXY->Write();
        if (smoothXY) smoothXY->Write();
        if (hE) hE->Write();
 		//	if (h1bptr) h1bptr->Write();
		outRootFile.Close();
    }

    return 0;
}

// =========================================================

void parseInputFlags(int argc, char *argv[])
{
    std::string tmpStr;
    for (int i=1; i<argc; i++)
	{
		tmpStr = argv[i];
		if (tmpStr == "-writeLORs" )
		{
            CUserParameters::Instance()->SetDoWriteLORs();
		}
		else 
		{
            if (tmpStr != "-help" )
                cout << "Error! Unknown flag: " << tmpStr << endl;
			cout << endl;
			cout << argv[0] << " -writeLORs" << endl;
			cout << endl;
			exit(1);
		}
	}
	cout << endl;
}


// =====================================

double GetDistance(const C3Vector& in_v1, const C3Vector& in_v2)
{
	double dx = in_v2.GetX() - in_v1.GetX();
	double dy = in_v2.GetY() - in_v1.GetY();
	double dz = in_v2.GetZ() - in_v1.GetZ();
	return sqrt(dx*dx + dy*dy + dz*dz);
}

// =====================================

// OLD STUFF (maybe put somewhere else?)
/*

	// Loop over found peaks bins
	// 
	for (int ip = 0; ip < found_peaks_bins.size(); ip++)
	{
		int bin = found_peaks_bins[ip			
		myTH2D.GetBins(hXY, bin, binX, binY, binZ);

		posX = hXY->GetXaxis()->GetBinCenter(binX);
		posY = hXY->GetYaxis()->GetBinCenter(binY);
		posZ = hXY->GetZaxis()->GetBinCenter(binZ);

		cout << "Peak at position: " << posX << " " << posY << " " << posZ 
		     << " with value: " << hXY->GetBinContent(binX, binY) << " ";

		// distance to true known position
		//
		if (found_peaks_bins.size() == known_peaks_positions.size() )
		{
			C3Vector foundVec(posX, posY, posZ);
			C3Vector knownVec = known_peaks_positions[ip];
				// TODO: this assumes that the known positions are ordered in the same way as the found positions
				// this is true now, but we shouldn't assume it for the general case.
				// TODO: find some clever way to order both (i.e. "myidx = myidx_y*nS_x + myidx_x")
			cout << " Distance to true: " << GetDistance(knownVec, foundVec) << " ";
		}
		cout << endl;
	}
*/

// =====================================

void assignTableData(TableData* tableData, MyTH2D* in_hXY)
{
    std::vector<C3Vector> peakPositions;
    std::vector<C3Vector> RealpeakPositions;
    CSimplePhantomPositions phantomPositions;

    phantomPositions.GetTruePositionsInFOV(in_hXY, peakPositions, true);
    
    phantomPositions.GetRealPositionsInFOV(in_hXY, peakPositions, RealpeakPositions, tableData->dev_mu, tableData->dev_sigma);
    
    phantomPositions.GetIfGaussian(in_hXY, RealpeakPositions, tableData->gaussian);

    phantomPositions.GetPeakHeightDistribution( in_hXY, RealpeakPositions, tableData->height_mu, tableData->height_sigma );
    
    phantomPositions.GetResolution(in_hXY, RealpeakPositions, tableData->x_Res , tableData->y_Res);
    
    phantomPositions.GetTruePositionsInFOV(in_hXY, peakPositions, false);
    
    tableData->peaksTotal = peakPositions.size();
    tableData->peaksFound = RealpeakPositions.size();
}

void getOptimalGaussValues(TableData* tableData, CSpectImage* spectImage, int& size_optimal, double& sigma_optimal)
{
    cout << "Creating Gauss Table" << endl; 

    std::vector<int> sizes{3, 5, 7};
    std::vector<double> sigmas{0.5, 0.75, 1., 1.25, 1.5, 1.75, 2.};
    std::vector<int>::iterator size_it;
    std::vector<double>::iterator sigma_it;
    
    ofstream outTableFile("TableGauss.dat");
    using std::setw;
    outTableFile << std::left;
    
    outTableFile << setw(8) << "Size" << setw(8) << "Sigma" << setw(16) << "HitsNotLost (%)" << setw(8) << "#Peaks" << setw(30) << "Res. (x, y)" << setw(30) << "Height (mu, sig)" << setw(30) << "Dev. (mu, sig)" << setw(8)<< "Gaussian?" << endl;
    
    bool gauss_optimal = true;
    for (size_it = sizes.begin(); size_it != sizes.end(); size_it++)
    {
        for (sigma_it = sigmas.begin(); sigma_it != sigmas.end(); sigma_it++)
        {
            spectImage->GaussianSmoothing(*size_it, *sigma_it, tableData->hitsNotLost);
            assignTableData(tableData, spectImage->GetSmooth2DHist());
            if ( gauss_optimal && tableData->gaussian )
            {
                size_optimal = *size_it;
                sigma_optimal = *sigma_it;
                gauss_optimal = false;
            }
            
            outTableFile    << setw(8) << *size_it   
                            << setw(8) << *sigma_it
                            << setw(16) << tableData->hitsNotLost << setw(8)
                            << std::to_string(tableData->peaksFound) + "/" + std::to_string(tableData->peaksTotal) << setw(30)
                            << std::to_string(tableData->x_Res) + " , " + std::to_string(tableData->y_Res) << setw(30)
                            << std::to_string(tableData->height_mu) + " , " + std::to_string(tableData->height_sigma) << setw(30)
                            << std::to_string(tableData->dev_mu) + " , " + std::to_string(tableData->dev_sigma)
                            << setw(8) << BoolToString(tableData->gaussian) << endl;             
        }
    }
}

inline const char * const BoolToString(bool b)
{
  return b ? "true" : "false";
}



