
#include "CPhantomPositions.h"
#include "CUserParameters.h"

#include <iostream>
#include <cmath>
#include <cassert>
#include <math.h>

using namespace std;

//
// General Peak Positions finder
//
void
CPhantomPositions::FindPositionsIn2DHisto( MyTH2D* in_hXY,
                    std::vector<int>& out_positions, bool debug ) const
{
    // Get the bin and contents of the 2D histogram with maximum value to get the peak threshold
    int binX(0), binY(0), binZ(0);
    double posX(0), posY(0);
    int bin = in_hXY->GetMaximumBin(binX, binY, binZ);
        // ROOT is nice enough to have a function that returns the bin with the maximum contents
    double maxValue = in_hXY->GetBinContent(bin);
    double peakThreshold = 0.20 * maxValue;
    if (debug)
    {
        cout << "Max value in XY map: " << maxValue << ". ";
        cout << "Peak finding algorithm, uses threshold: " << peakThreshold << endl;
    }

    for (int iY = 2; iY < in_hXY->GetNbinsY()-1; iY++)   // skip the edges
    {
        for (int iX = 2; iX < in_hXY->GetNbinsX()-1; iX++)   // skip the edges
        {
            double value = in_hXY->GetBinContent(iX,iY);
            // TODO: in case next-door neighbour has the same value (with low statistics especially)
            //  to avoid two neighbouring pixels with the same number of counts...
            // TODO: we should somehow put the peak-position in between both pixels... 
            //
            if (   value > peakThreshold
                && (value == in_hXY->GetBinContent(iX+1, iY) || value == in_hXY->GetBinContent(iX, iY+1)) )
            {
                value += 1; // artificially make sure that this pixel has preference over the next neighbour
            }
            if (   value > peakThreshold
                && value > in_hXY->GetBinContent(iX-1, iY) && value > in_hXY->GetBinContent(iX+1, iY)
                && value > in_hXY->GetBinContent(iX, iY-1) && value > in_hXY->GetBinContent(iX, iY+1)
                && value > in_hXY->GetBinContent(iX-1, iY-1) && value > in_hXY->GetBinContent(iX+1, iY-1)
                && value > in_hXY->GetBinContent(iX-1, iY+1) && value > in_hXY->GetBinContent(iX+1, iY+1) )
            {
                // yeah! a peak!
                bin = in_hXY->GetBin(iX, iY);
                out_positions.push_back(bin);
            }
        }
    }
}

// ====================================================================================

void
CPhantomPositions::GetTruePositionsInFOV( MyTH2D* in_hXY, std::vector<C3Vector>& out_positions, bool threshold ) const
{
	// Geometry constants for known src positions
	double distance_pS_pS = CUserParameters::Instance()->GetRectanglePhantomStep(); 	// mm		// centre to centre
    int nS_x = CUserParameters::Instance()->GetRectanglePhantom_nX();
    int nS_y = CUserParameters::Instance()->GetRectanglePhantom_nY();
    
    double x_min = in_hXY->GetXaxis()->GetXmin();
    double x_max = in_hXY->GetXaxis()->GetXmax();

    double y_min = in_hXY->GetYaxis()->GetXmin();
    double y_max = in_hXY->GetYaxis()->GetXmax();

	int nPeaks = nS_x * nS_y;
	double pos_src_x = -1.0 * (((double) nS_x - 1)/2) * distance_pS_pS;
	double pos_src_y = -1.0 * (((double) nS_y - 1)/2) * distance_pS_pS;
	double pos_src_z(0.0);
	
	out_positions.clear();
    int binX(0), binY(0), binZ(0);
    
    int bin = in_hXY->GetMaximumBin(binX, binY, binZ);
    double maxValue = in_hXY->GetBinContent(bin);
    double peakThreshold = 0.20 * maxValue;
    
	for (int ipy = 0; ipy < nS_y; ipy++)
	{
		double pos_src_x_tmp = pos_src_x;
        if (pos_src_y <= y_max && pos_src_y >= y_min)
        {
            for (int ipx = 0; ipx < nS_x; ipx++)
            {
                if (pos_src_x_tmp <= x_max && pos_src_x_tmp >= x_min)
                {
                    C3Vector posVec(pos_src_x_tmp, pos_src_y, pos_src_z);
                    in_hXY->GetBins(posVec, binX, binY);
                    if(in_hXY->GetBinContent(binX, binY) > peakThreshold || !threshold)
                    {
                        out_positions.push_back(posVec); //We have a theoretical peak iff its in histogram range and we have an over threshold number of hits
                        //If not threshold, get all the peaks in histogram range
                    }
                }
                pos_src_x_tmp += distance_pS_pS;
            }	
        }	
		
		pos_src_y += distance_pS_pS;
	}
}

// ====================================================================================

void
CPhantomPositions::GetRealPositionsInFOV( MyTH2D* in_hXY,  std::vector<C3Vector>& in_positions, std::vector<C3Vector>& out_positions, double& deviation_mu, double& deviation_sigma ) const
{   
    double maxValue(0.), binValue(0.);
    bool xLimit(false), yLimit(false);
    
    C3Vector RealpeakPos(0., 0., 0.);

    std::vector<int> bin_limits;
	out_positions.clear();
    
    for (int i = 0; i < in_positions.size(); i++)
    {
        bin_limits.clear();
        GetBinLimits(in_hXY, in_positions[i], bin_limits, xLimit, yLimit);
        if (!xLimit && !yLimit)
        {
            for (int iX = bin_limits[0]; iX < bin_limits[1]; iX++) 
            {
                for (int iY = bin_limits[2]; iY < bin_limits[3]; iY++) 
                {
                    binValue = in_hXY->GetBinContent(iX, iY);
                    if ( binValue > maxValue )
                    {
                        in_hXY->GetPosition(iX, iY, RealpeakPos);
                        maxValue = binValue;
                    }
                }
            }               
        }

        out_positions.push_back(RealpeakPos);
        maxValue = 0.;
    }
    if (in_positions.size() != out_positions.size() )
    {
        cout << "ERROR: not matching sizes of theo and real #peaks" << endl;
    }
    std::vector<double> deviations;
    for (int i = 0; i < in_positions.size(); i++)
    {
        C3Vector deviation = in_positions[i] - out_positions[i];
        deviations.push_back(deviation.GetLength());
    }
    double dif = std::accumulate(std::begin(deviations), std::end(deviations), 0.0);
    deviation_mu =  dif / out_positions.size();
    
    deviation_sigma = 0.;
    for (int i = 0; i < deviations.size(); i++)
    {
        deviation_sigma += (deviations[i] - deviation_mu) * (deviations[i] - deviation_mu);
    }
    deviation_sigma = sqrt(deviation_sigma/deviations.size());
}

// ====================================================================================

void
CPhantomPositions::GetIfGaussian( MyTH2D* in_hXY,  std::vector<C3Vector>& in_positions, bool& gaussian) const
{
    double prevbinValue(0.), binValue(0.);
    bool xLimit(false), yLimit(false);
    int binX(0), binY(0);
    gaussian = true;
    
    std::vector<int> bin_limits;
    
    for (int i = 0; i < in_positions.size(); i++)
    {
        C3Vector peakPos(in_positions[i]);
        in_hXY->GetBins(peakPos, binX, binY);
        
        bin_limits.clear();
        GetBinLimits(in_hXY, in_positions[i], bin_limits, xLimit, yLimit);
        
        if (!xLimit)
        {
            prevbinValue = in_hXY->GetBinContent(binX, binY);
            for (int iX = binX; iX > bin_limits[0]; iX--) 
            {
                binValue = in_hXY->GetBinContent(iX, binY);
                if ( binValue <= prevbinValue )
                {
                    prevbinValue = binValue;
                }
                else
                {
                    gaussian = false;
                    break;
                }
            }
            
            prevbinValue = in_hXY->GetBinContent(binX, binY);
            for (int iX = binX; iX < bin_limits[1]; iX++) 
            {
                binValue = in_hXY->GetBinContent(iX, binY);
                if ( binValue <= prevbinValue )
                {
                    prevbinValue = binValue;
                }
                else
                {
                    gaussian = false;
                    break;
                }
            }
        }
        
        if (!yLimit)
        {
            prevbinValue = in_hXY->GetBinContent(binX, binY);
            for (int iY = binY; iY > bin_limits[2]; iY--) 
            {
                binValue = in_hXY->GetBinContent(binX, iY);
                if ( binValue <= prevbinValue )
                {
                    prevbinValue = binValue;
                }
                else
                {
                    gaussian = false;
                    break;
                }
            }
            
            prevbinValue = in_hXY->GetBinContent(binX, binY);
            for (int iY = binY; iY < bin_limits[3]; iY++) 
            {
                binValue = in_hXY->GetBinContent(binX, iY);
                if ( binValue <= prevbinValue )
                {
                    prevbinValue = binValue;
                }
                else
                {
                    gaussian = false;
                    break;
                }
            }
        }
    }
}

// ====================================================================================

void
CPhantomPositions::GetPeakHeightDistribution( MyTH2D* in_hXY,  std::vector<C3Vector>& in_positions, double& height_mu, double& height_sigma ) const
{
    std::vector<double> heights;
    int binX(0), binY(0);
    for (int i = 0; i < in_positions.size(); i++)
    {
        in_hXY->GetBins(in_positions[i], binX, binY);
        heights.push_back(in_hXY->GetBinContent(binX, binY));
    }
    
    double sum = std::accumulate(std::begin(heights), std::end(heights), 0.0);
    height_mu =  sum / in_positions.size();
    
    height_sigma = 0.;
    for (int i = 0; i < heights.size(); i++)
    {
        height_sigma += (heights[i] - height_mu) * (heights[i] - height_mu);
    }
    height_sigma = sqrt(height_sigma/heights.size());
}

// ====================================================================================

void
CPhantomPositions::GetResolution( MyTH2D* in_hXY, std::vector<C3Vector>& in_positions, double& x_resolution, double& y_resolution ) const
{
	// Geometry constants for known src positions
	double distance_pS_pS = CUserParameters::Instance()->GetRectanglePhantomStep(); 	// mm		// centre to centre
    
    double x_min = in_hXY->GetXaxis()->GetXmin();
    double x_max = in_hXY->GetXaxis()->GetXmax();

    double y_min = in_hXY->GetYaxis()->GetXmin();
    double y_max = in_hXY->GetYaxis()->GetXmax();

    int binX(0), binY(0);
    double peakValue(0.), binValue(0.), prevbinValue(0.);
    double xRes(0.), yRes(0.);
    double binPosX(0.), binPosY(0.);
    bool xLimit(false), yLimit(false);
    bool xFound1(false), xFound2(false), yFound1(false), yFound2(false);
    std::vector<int> bin_limits;
    std::vector<double> xResolutionMeans;
    std::vector<double> yResolutionMeans;
    
    for (int i = 0; i < in_positions.size(); i++)
    {
        C3Vector peakPos(in_positions[i]);
        in_hXY->GetBins(peakPos, binX, binY);
        peakValue = in_hXY->GetBinContent(binX, binY);
        bin_limits.clear();
        xLimit = false;
        yLimit = false;
        
        GetBinLimits(in_hXY, peakPos, bin_limits, xLimit, yLimit);
        
        C3Vector xMinBinPos(0., 0., 0.);
        C3Vector xMaxBinPos(0., 0., 0.);
        C3Vector yMinBinPos(0., 0., 0.);
        C3Vector yMaxBinPos(0., 0., 0.);
        
        in_hXY->GetPosition(bin_limits[0], binY, xMinBinPos);
        in_hXY->GetPosition(bin_limits[1], binY, xMaxBinPos);
        in_hXY->GetPosition(binX, bin_limits[2], yMinBinPos);
        in_hXY->GetPosition(binX, bin_limits[3], yMaxBinPos);
        
        
        //cout << "Peak " << i << ", x : ( " << xMinBinPos.GetX() << " , " << xMaxBinPos.GetX() << " ) , y : ( " << yMinBinPos.GetY() << " , " << yMaxBinPos.GetY() << " ) " << endl;
        
        C3Vector halfMaxBinPos(0., 0., 0.);
        C3Vector prevhalfMaxBinPos(0., 0., 0.);
        double halfMaxPos(0.);

        if (!xLimit)
        {
            for (int iX = binX; iX > bin_limits[0]; iX--) 
            {
                binValue = in_hXY->GetBinContent(iX, binY);
                if ( binValue < peakValue/2. && binValue>1)
                {
                    prevbinValue = in_hXY->GetBinContent(iX+1, binY);
                    in_hXY->GetPosition(iX, binY, halfMaxBinPos);
                    in_hXY->GetPosition(iX+1, binY, prevhalfMaxBinPos);
                    halfMaxPos = halfMaxBinPos.GetX() + (peakValue/2. - binValue)/(prevbinValue-binValue)*(prevhalfMaxBinPos.GetX()-halfMaxBinPos.GetX());
                    xRes += peakPos.GetX() - halfMaxPos;
                    xFound1 = true;
                    
                    //cout << "halfMaxPos: " << halfMaxPos << " , prehalfMaxBinPos : " << prevhalfMaxBinPos.GetX() << " , halfMaxBinPos : " << halfMaxBinPos.GetX() << endl;
                    break;
                }
            }
                
            for (int iX = binX; iX < bin_limits[1]; iX++) 
            {
                binValue = in_hXY->GetBinContent(iX, binY);
                if ( binValue < peakValue/2. && binValue>1)
                {
                    prevbinValue = in_hXY->GetBinContent(iX-1, binY);
                    in_hXY->GetPosition(iX, binY, halfMaxBinPos);
                    in_hXY->GetPosition(iX-1, binY, prevhalfMaxBinPos);
                    halfMaxPos = halfMaxBinPos.GetX() + (peakValue/2. - binValue)/(prevbinValue-binValue)*(prevhalfMaxBinPos.GetX()-halfMaxBinPos.GetX());
                    xRes -= peakPos.GetX() - halfMaxPos;
                    xFound2 = true;
                    break;

                }
            }
            
            if(xFound1 && xFound2)
            {
                xResolutionMeans.push_back(xRes);
                xFound1 = false;
                xFound2 = false;
            }
            xRes = 0.;
        }
        
        if (!yLimit)
        {
            for (int iY = binY; iY > bin_limits[2]; iY--) 
            {
                binValue = in_hXY->GetBinContent(binX, iY);
                if ( binValue < peakValue/2. && binValue>1)
                {
                    prevbinValue = in_hXY->GetBinContent(binX, iY+1);
                    in_hXY->GetPosition(binX, iY, halfMaxBinPos);
                    in_hXY->GetPosition(binX, iY+1, prevhalfMaxBinPos);
                    halfMaxPos = halfMaxBinPos.GetY() + (peakValue/2. - binValue)/(prevbinValue-binValue)*(prevhalfMaxBinPos.GetY()-halfMaxBinPos.GetY());
                    yRes += peakPos.GetY() - halfMaxPos;
                    yFound1 = true;
                    
                    break;
                }
            }
            
            for (int iY = binY; iY < bin_limits[3]; iY++) 
            {
                binValue = in_hXY->GetBinContent(binX, iY);
                if ( binValue < peakValue/2. && binValue>1)
                {
                    prevbinValue = in_hXY->GetBinContent(binX, iY-1);
                    in_hXY->GetPosition(binX, iY, halfMaxBinPos);
                    in_hXY->GetPosition(binX, iY-1, prevhalfMaxBinPos);
                    halfMaxPos = halfMaxBinPos.GetY() + (peakValue/2. - binValue)/(prevbinValue-binValue)*(prevhalfMaxBinPos.GetY()-halfMaxBinPos.GetY());
                    yRes -= peakPos.GetY() - halfMaxPos;
                    yFound2 = true;
                    
                    //cout << "halfMaxPos: " << halfMaxPos << " , prehalfMaxBinPos : " << prevhalfMaxBinPos.GetY() << " , halfMaxBinPos : " << halfMaxBinPos.GetY() << endl;
                    break;
                }
            }
            

            if(yFound1 && yFound2)
            {
                yResolutionMeans.push_back(yRes);
                yFound1 = false;
                yFound2 = false;
            }
            yRes = 0.;
        }
    }    
    
    double sumX = std::accumulate(std::begin(xResolutionMeans), std::end(xResolutionMeans), 0.0);
    double meanX =  sumX / xResolutionMeans.size();

    double sumY = std::accumulate(std::begin(yResolutionMeans), std::end(yResolutionMeans), 0.0);
    double meanY =  sumY / yResolutionMeans.size();
    
	x_resolution = meanX;
    y_resolution = meanY;
    
    //cout << "Peaks x involved in resolution calculation: " << xResolutionMeans.size() << endl;
    //cout << "Peaks y involved in resolution calculation: " << xResolutionMeans.size() << endl;
}

void
CPhantomPositions::GetBinLimits( MyTH2D* in_hXY, C3Vector in_position, std::vector<int>& out_bin_positions, bool& x_limit, bool& y_limit ) const
{
    double distance_pS_pS = CUserParameters::Instance()->GetRectanglePhantomStep(); 	// mm		// centre to centre
    
    double x_min = in_hXY->GetXaxis()->GetXmin();
    double x_max = in_hXY->GetXaxis()->GetXmax();

    double y_min = in_hXY->GetYaxis()->GetXmin();
    double y_max = in_hXY->GetYaxis()->GetXmax();
    
    C3Vector distanceXPos(distance_pS_pS/2., 0., 0.);
    C3Vector distanceYPos(0., distance_pS_pS/2., 0.);
    C3Vector auxPos(0., 0., 0.);
    int binX(0), binY(0);

    auxPos = in_position - distanceXPos;
    
    if (auxPos.GetX()<x_min)
    {
        out_bin_positions.push_back(0);
        x_limit = true;
    }
    else
    {
        in_hXY -> GetBins(auxPos, binX, binY);
        out_bin_positions.push_back(binX);
    }
    
    auxPos = in_position + distanceXPos;
    
    if (auxPos.GetX()>x_max)
    {
        out_bin_positions.push_back(0);
        x_limit = true;
    }
    else
    {
        in_hXY -> GetBins(auxPos, binX, binY);
        out_bin_positions.push_back(binX);
    }
    
    auxPos = in_position - distanceYPos;
    
    if (auxPos.GetY()<y_min)
    {
        out_bin_positions.push_back(0);
        y_limit = true;
    }
    else
    {
        in_hXY -> GetBins(auxPos, binX, binY);
        out_bin_positions.push_back(binY);
    }
    
    auxPos = in_position + distanceYPos;
    
    if (auxPos.GetY()>y_max)
    {
        out_bin_positions.push_back(0);
        y_limit = true;
    }
    else
    {
        in_hXY -> GetBins(auxPos, binX, binY);
        out_bin_positions.push_back(binY);
    }
}

// ====================================================================================
// CSimplePhantomPositions is for the exceptional case that we have a Nx * Ny rectangular source phantom
//

void
CSimplePhantomPositions::GetTruePositions( std::vector<C3Vector>& out_positions, bool debug ) const
{
    // 
    //  if (debug)
    //      cout << __PRETTY_FUNCTION__ << endl;    // just a "funny" test
    
	// Geometry constants for known src positions
	double distance_pS_pS = CUserParameters::Instance()->GetRectanglePhantomStep(); 	// mm		// centre to centre
    int nS_x = CUserParameters::Instance()->GetRectanglePhantom_nX();
    int nS_y = CUserParameters::Instance()->GetRectanglePhantom_nY();

	int nPeaks = nS_x * nS_y;
	double pos_src_x = -1.0 * (((double) nS_x - 1)/2) * distance_pS_pS;
	double pos_src_y = -1.0 * (((double) nS_y - 1)/2) * distance_pS_pS;
	double pos_src_z(0.0);
	// TODO: read from geom file (or config file) >>>>>
	// Fill known peaks
	out_positions.clear();
	for (int ipy = 0; ipy < nS_y; ipy++)
	{
        if (debug)
        {
            cout << "Known src positions at y = " << pos_src_y << ". ";
            cout << "x = ";
        }
		double pos_src_x_tmp = pos_src_x;
		for (int ipx = 0; ipx < nS_x; ipx++)
		{
			C3Vector posVec(pos_src_x_tmp, pos_src_y, pos_src_z);	
            if (debug)
                cout << pos_src_x_tmp << " , ";
			out_positions.push_back(posVec);
			pos_src_x_tmp += distance_pS_pS;
		}	
		if (debug)
            cout << endl;
		pos_src_y += distance_pS_pS;
	}
}

// ============================

void
CSimplePhantomPositions::FindPeakToValleys(MyTH2D* in_hXY,
         std::vector<int>& in_peak_position_bins,
         std::vector<double>& out_found_p2vs, double& out_average_p2vs, double& out_sigma_p2vs,
         vector<int>& out_lp_ybins, bool debug ) const
{
    int nS_x = CUserParameters::Instance()->GetRectanglePhantom_nX();
    int nS_y = CUserParameters::Instance()->GetRectanglePhantom_nY();

    int binX(0), binY(0), binZ(0);
    double posX(0), posY(0);

    out_lp_ybins.clear();
    out_average_p2vs = 0.0;
    int countX(0);
    for (int ip = 0; ip < in_peak_position_bins.size(); ip++)
    {
        int bin = in_peak_position_bins[ip];
        in_hXY->GetBins(bin, binX, binY);

        if (out_lp_ybins.size() == 0 || binY > out_lp_ybins[out_lp_ybins.size() - 1]+1 )
            out_lp_ybins.push_back(binY);

        posX = in_hXY->GetXaxis()->GetBinCenter(binX);
        posY = in_hXY->GetYaxis()->GetBinCenter(binY);
        double value = in_hXY->GetBinContent(binX, binY);

        if (ip%nS_x > 0)
            // select valleys between peaks of the same row (same Y)
        {
            int prev_bin = in_peak_position_bins[ip-1];
            int prev_binX, prev_binY;
            // olé
            in_hXY->GetBins(prev_bin, prev_binX, prev_binY);

            
            if (prev_binX >= binX || prev_binY != binY)
            {
                cout << "countX: " << countX << endl;
                cout << "prev_binX and binX: " << prev_binX << " " << binX << endl;
                cout << "prev_binY and binY: " << prev_binY << " " << binY << endl;
            }
            
            //assert(prev_binX < binX && prev_binY == binY);

            int mid_binX = 0.5 * (prev_binX + binX);
                // prev_binX + (binX - prev_binX)/2
                // = (prev_binX + binX)/2
            double valley = in_hXY->GetBinContent(mid_binX, binY);
            double prev_value = in_hXY->GetBinContent(prev_binX, prev_binY);
            if (valley < 1) valley = 1;
            double p2v = ((value + prev_value)/2.0)/valley;
            out_found_p2vs.push_back(p2v);
            out_average_p2vs += p2v;
        }
        if (countX == nS_x) countX = 0;
    }
    out_average_p2vs = out_average_p2vs/out_found_p2vs.size();

    out_sigma_p2vs = 0.0;
    for (int ip2v = 0; ip2v < out_found_p2vs.size(); ip2v++)
    {
        double delta = out_found_p2vs[ip2v] - out_average_p2vs;
        out_sigma_p2vs += delta * delta;
    }
    out_sigma_p2vs = sqrt(out_sigma_p2vs / out_found_p2vs.size());

    //assert( out_lp_ybins.size() == nS_y);

}

