
#include "CMyTH2D.h"

#include <iostream>

using namespace std;

// ============

int MyTH2D::GetBin(int in_binX, int in_binY) const
{
	int dimension = GetDimension(); // TH2D function
	int bin(0);
	if (dimension == 2)
	{
		int nX = GetNbinsX()+2;     // TH2D function
			// add 2 because ROOT has an extra initial bin and final bin (under- and overflow) in each dimension
		bin = in_binY * nX + in_binX;		
	}
	else
		cout << "WARNING, dimension not implemented: " << dimension << endl;
	
	return bin;
}

// =====================================

void MyTH2D::GetBins(int in_bin, int& out_binX, int& out_binY) const
{
	int dimension = GetDimension();     // TH2D function
	out_binX = 0;
	out_binY = 0;

	if (dimension == 2)
	{
		int nX = GetNbinsX()+2;         // TH2D function
			// add 2 because ROOT has an extra initial bin and final bin (under- and overflow) in each dimension
		out_binX = in_bin % nX;
		out_binY = (in_bin - out_binX)/nX;
	}
	else
		cout << "WARNING, dimension not implemented: " << dimension << endl;
}

// =====================================

void MyTH2D::GetPosition(int in_binX, int in_binY, C3Vector& out_vector) const
{
    double posX = GetXaxis()->GetBinCenter(in_binX);
    double posY = GetYaxis()->GetBinCenter(in_binY);
    out_vector.Set (posX, posY, 0.0);   // Z position is not relevant for a 2D histogram
}

// =======================================================

void MyTH2D::GetBins(const C3Vector& in_vector, int& out_binX, int& out_binY ) const
{
    /*
    double dX = GetXaxis()->GetBinWidth(1);
    double lowX = GetXaxis()->GetBinLowEdge(1);
    out_binX = (in_vector.GetX()-lowX)/dX;
    
    cout << "lowX: " << lowX << " posX-lowX: " << in_vector.GetX()-lowX
        << " (posX-lowX)/dX: " << (in_vector.GetX()-lowX)/dX
        << " out_binX: " << out_binX << endl;
    
    double dY = GetYaxis()->GetBinWidth(1);
    double lowY = GetYaxis()->GetBinLowEdge(1);
    out_binY = (in_vector.GetY()-lowY)/dY;
    // DEBUG: 
    cout << "MyTH2D::GetBins(const C3Vector& in_vector, int& out_binX, int& out_binY )" << endl;
    cout << "Position: " << in_vector << endl;
    cout << "X position: " << in_vector.GetX() << " dX: " << dX
         << " binX: " << out_binX << endl;
    cout << "Y position: " << in_vector.GetY() << " dY: " << dY
         << " binY: " << out_binY << endl; 
    */
         
    // Other method:
    int globalBin = FindFixBin	( in_vector.GetX(), in_vector.GetY(), 0);
    GetBins(globalBin, out_binX, out_binY);
    //  cout << "Other method: " << endl;
    //  cout << " binX: " << out_binX << endl;
    //  cout << " binX: " << out_binY << endl;          
}

