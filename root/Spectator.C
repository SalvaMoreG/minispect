
class C3Vector
{
public:
	C3Vector(double in_x, double in_y, double in_z) 
		: m_x(in_x), m_y(in_y), m_z(in_z) {}
	double m_x;
	double m_y; 
	double m_z;
};

int GetBin(const TH2D* in_h2, int in_binX, int in_binY, int in_binZ);
void GetBins(const TH2D* in_h2, int in_bin, int& out_binX, int& out_binY, int& out_binZ);
double GetDistance(const C3Vector& in_v1, const C3Vector& in_v2);


// ==================================

int Main()
{
	gStyle->SetOptFit();

	string fname;
	cout << "give list mode (LM) hits filename" << endl;
	cin >> fname;

	ifstream ffile( fname.c_str() );
	if ( !ffile.is_open() )
	{
		cout << "file does not exist, " << fname << endl;
		exit(1);
	}

	// const double Emargen( 5.1 ); // keV; 5.1 = 3 * 1.7, sigma = 1.7
	cout << "Give the target energy (e.g. 140 keV) and the sigma of the energy resolution" << endl;
	cout << "Esigma == -1, means: no energy cut" << endl;
	double Etarget, Esigma;
	cin >> Etarget >> Esigma;
	double Emargen = 3.0 * Esigma;

	// Geometry constants for known src positions
	// TODO: read from geom file (or config file) <<<<<
	double distance_pS_pS(5.0); 	// mm		// centre to centre
	int nS_x(5), nS_y(4);
	int nPeaks = nS_x * nS_y;
	double pos_src_x = -1.0 * (((double) nS_x - 1)/2) * distance_pS_pS;
	double pos_src_y = -1.0 * (((double) nS_y - 1)/2) * distance_pS_pS;
	double pos_src_z(0.0);
	// TODO: read from geom file (or config file) >>>>>
	// Fill known peaks
	std::vector<C3Vector> known_peaks_positions;
	for (int ipy = 0; ipy < nS_y; ipy++)
	{
		cout << "Known src positions at y = " << pos_src_y << ". ";
		cout << "x = ";
		double pos_src_x_tmp = pos_src_x;
		for (int ipx = 0; ipx < nS_x; ipx++)
		{
			C3Vector posVec(pos_src_x_tmp, pos_src_y, pos_src_z);	
			cout << pos_src_x_tmp << " , ";
			known_peaks_positions.push_back(posVec);
			pos_src_x_tmp += distance_pS_pS;
		}	
		cout << endl;
		pos_src_y += distance_pS_pS;
	}

	TCanvas* m_E = new TCanvas("m_1", "E (keV)", 600, 600);
	TCanvas* m_XY = new TCanvas("m_XY", "XY", 1200, 600);
	TCanvas* m_z = new TCanvas("m_z", "Z", 300, 300);
	TCanvas* m_lp = new TCanvas("m_lp", "line profiles", 800, 800);

	TH1D* hE = new TH1D("hE", "E", 200, 0.0, 200);
	TH1D* hz = new TH1D("hz", "Z", 50, 0., 50.);
	TH2D* hXY = new TH2D("hXY",  "hitsXY", 100, -50.0, 50.0, 50, -25.0, 25.0);

	string dummy;
	double E, x, y, z, radius;
	unsigned long long int time_ps;
	unsigned int count(0);
	
	while ( !ffile.eof() ) 
	{
		ffile >> dummy >> dummy >> time_ps >> E >> x >> y >> z;
		ffile.ignore(1024, '\n');
		if ( !ffile.eof() )
		{
			hE->Fill(E*1000.0);	// MeV to keV
	
			// if no E cut OR if E(keV) - Etarget < Emargen, fill XY map and line-profiles
			if ( Emargen < 0 || fabs(E*1000.0 - Etarget) < Emargen)
			{
				hXY->Fill(x, y);
			}
			
			hz->Fill(z);
			count++;
		}
	}
	// Get the bin and contents of the 2D histogram with maximum valueto get the peak threshold
	int binX(0), binY(0), binZ(0);
	double posX(0), posY(0), posZ(0);
	int bin = hXY->GetMaximumBin(binX, binY, binZ);	
		// ROOT is nice enough to have a function that returns the bin with the maximum contents
	double maxValue = hXY->GetBinContent(bin);
	double peakThreshold = 0.20 * maxValue;
	cout << "Max value in XY map: " << maxValue << ". ";
	cout << "Peak finding algorithm, uses threshold: " << peakThreshold << endl;

	// just for checking:
	/*
	{
		cout << "XY map, bin: " << bin << " with max value: " << maxValue << endl;
		cout << "@: " << binX << " " << binY << " " << binZ << endl;
		posX = hXY->GetXaxis()->GetBinCenter(binX);
		posY = hXY->GetYaxis()->GetBinCenter(binY);
		posZ = hXY->GetZaxis()->GetBinCenter(binZ);
		cout << "@: " << posX << " " << posY << " " << posZ << endl;

		int checkBin, checkBinX, checkBinY, checkBinZ; 
		checkBin = GetBin(hXY, binX, binY, binZ);				// check this function
		GetBins(hXY, bin, checkBinX, checkBinY, checkBinZ);		// check this function

		if (bin != checkBin)
			cout << "checking single bin: " << checkBin << endl;
		if (binX != checkBinX || binY != checkBinY || binZ != checkBinZ)
			cout << "checking xyz checkBins: " << checkBinX << " " << checkBinY << " " << checkBinZ << endl;
		assert(bin == checkBin);
		assert(binX == checkBinX && binY == checkBinY && binZ == checkBinZ);
	}
	*/

	// Ok, let's loop around 2D histogram to do all kinds of analysis stuff
	//
	std::vector<int> found_peaks_bins;
	std::vector<int> found_p2vs;
	double average_p2vs(0.0);
	for (int iY = 2; iY < hXY->GetNbinsY()-1; iY++)	 // skip the edges
	{
		int countX(0);
		for (int iX = 2; iX < hXY->GetNbinsX()-1; iX++)	 // skip the edges
		{
			double value = hXY->GetBinContent(iX,iY);
			// TODO: for next-door neighbours, we use ">=" because it might be (with low statistics especially)
			// that two neighbouring pixels have exactly the same number of counts...
			// TODO: we should somehow put the peak in between both pixels... (argh!!!)
			if (   value > peakThreshold 
				&& (value == hXY->GetBinContent(iX+1, iY) || value == hXY->GetBinContent(iX, iY+1)) )
			{
				value += 1;	// artificially make sure that this pixel has preference over the next neighbour
			}
			if (   value > peakThreshold 
				&& value > hXY->GetBinContent(iX-1, iY) && value > hXY->GetBinContent(iX+1, iY) 
                && value > hXY->GetBinContent(iX, iY-1) && value > hXY->GetBinContent(iX, iY+1) 
                && value > hXY->GetBinContent(iX-1, iY-1) && value > hXY->GetBinContent(iX+1, iY-1) 
                && value > hXY->GetBinContent(iX-1, iY+1) && value > hXY->GetBinContent(iX+1, iY+1) )
			{
				// yeah! a peak!
				bin = GetBin(hXY, iX, iY, 1);
				found_peaks_bins.push_back(bin);
				countX++;
				if (countX > 1 && countX <= nS_x)
				{
					int prev_idx = found_peaks_bins.size() - 2;
					int prev_bin = found_peaks_bins[prev_idx];
					GetBins(hXY, prev_bin, binX, binY, binZ);
					assert(binX < iX && binY == iY);
					int mid_binX = 0.5 * (binX + iX);	// binX + (iX - binX)/2 = (binX + iX)/2
					double valley = hXY->GetBinContent(mid_binX,iY);
					double prev_value = hXY->GetBinContent(binX, binY);
					if (valley < 1) valley = 1;
					double p2v = ((value + prev_value)/2.0)/valley;
					found_p2vs.push_back(p2v);
					average_p2vs += p2v;
				}
			}
		}
	}
	average_p2vs = average_p2vs/found_p2vs.size();

	cout << "Found " << found_peaks_bins.size() << " peaks" 
	     << " and " << found_p2vs.size() << " peak-to-valleys" 
		 << endl;

	if (found_peaks_bins.size() != known_peaks_positions.size() )
	{
		cout << "WARNING WARNING WARNING !!!" << endl;
		cout << "We found " << found_peaks_bins.size() << " peaks, "
			 << "instead of: " << known_peaks_positions.size() << endl;
	}

	// Loop over found peaks bins
	// 
	vector<int> lp_ybins;
	int ip2v(0);
	double sigma_p2vs(0.0);
	for (int ip = 0; ip < found_peaks_bins.size(); ip++)
	{
		int bin = found_peaks_bins[ip];
		GetBins(hXY, bin, binX, binY, binZ);

		posX = hXY->GetXaxis()->GetBinCenter(binX);
		posY = hXY->GetYaxis()->GetBinCenter(binY);
		posZ = hXY->GetZaxis()->GetBinCenter(binZ);

		cout << "Peak at position: " << posX << " " << posY << " " << posZ 
		     << " with value: " << hXY->GetBinContent(binX, binY) << " ";

		if (found_p2vs.size() > 0 && (ip+1) % nS_x != 0)
		{
			cout << "Peak to valley: " << found_p2vs[ip2v] << " ";
			double delta = found_p2vs[ip2v] - average_p2vs;
            sigma_p2vs += delta * delta;
			ip2v++;
		}

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

		if (lp_ybins.size() == 0 || binY > lp_ybins[lp_ybins.size() - 1]+1 )
			lp_ybins.push_back(binY);

	}
	cout << "# lineprofiles: " << lp_ybins.size() << endl;	
	assert( lp_ybins.size() == nS_y);

	sigma_p2vs = sqrt(sigma_p2vs / found_p2vs.size());
	cout << " <p2v>: " << average_p2vs << " +- " << sigma_p2vs << endl;

	//
	// Print the histograms

	double Emin(Etarget - 10.0);
	double Emax(Etarget + 10.0);
	if (Emargen > 0)
	{
		Emin = 140 - Emargen;
		Emax = 140 + Emargen;
	}
	
	m_E->cd();
	hE->GetXaxis()->SetTitle("Energy [keV]");
	hE->GetXaxis()->SetTitleSize(0.04);
	hE->GetXaxis()->SetLabelSize(0.04);
	hE->GetYaxis()->SetLabelSize(0.04);
	hE->Draw();	
	TF1* fitGauss = ((TF1 *)(gROOT->GetFunction("gaus")));
	double initPars[3] = {11, Etarget, 0.51};	
	fitGauss->SetParameters(initPars); 		// to get rid of stupid ROOT warning, make this call
	fitGauss->SetParLimits(0, 10, count);
	fitGauss->SetParLimits(1, Emin, Emax);
	fitGauss->SetParLimits(2, 0.5, 10);
	hE->Fit("gaus","BQ","", Emin, Emax);

	m_XY->cd();
	hXY->GetXaxis()->SetTitle("x [mm]");
	hXY->GetXaxis()->SetTitleSize(0.04);
	hXY->GetYaxis()->SetTitle("y [mm]");
	hXY->GetYaxis()->SetTitleSize(0.04);
	hXY->GetXaxis()->SetLabelSize(0.04);
	hXY->GetYaxis()->SetLabelSize(0.04);
	gPad->SetLeftMargin(0.15);
	gPad->SetRightMargin(0.20);
	hXY->Draw("COLZ");	
	cout << "#events in XY map: " << hXY->GetEntries() << endl;

	m_z->cd();
	hz->Draw();	

	// Lineprofiles using ProjectionX
	//
	m_lp->cd();
	m_lp->Divide(2, 2);

	TString lpss[4] = {"lp_X_1", "lp_X_2", "lp_X_3", "lp_X_4"};
	for (int ilp = 0; ilp < lp_ybins.size(); ilp++)
	{
		int binY = lp_ybins[ilp];
		TH1D* h_tmp = hXY->ProjectionX(lpss[ilp], binY, binY);
		h_tmp->GetXaxis()->SetRangeUser(-20, 20);
		posY = hXY->GetYaxis()->GetBinCenter(binY);
		TString posYstr("line_profile @ y = ");
		posYstr += posY;
		// cout << posY << " " << posYstr << endl;
		h_tmp->SetTitle(posYstr);

		m_lp->cd(ilp+1);
		h_tmp->Draw("HIST C");
	}

	return 0;
}

// =====================================

int GetBin(const TH2D* in_h2, int in_binX, int in_binY, int in_binZ)
{
	int dimension = in_h2->GetDimension();
	int bin(0);
	if (dimension == 2)
	{
		int nX = in_h2->GetNbinsX()+2;		
			// add 2 because ROOT has an extra initial bin and final bin (under- and overflow) in each dimension
		bin = in_binY * nX + in_binX;		
	}
	else
		cout << "WARNING, dimension not implemented: " << dimension << endl;
	
	return bin;
}

// =====================================

void GetBins(const TH2D* in_h2, int in_bin, int& out_binX, int& out_binY, int& out_binZ)
{
	int dimension = in_h2->GetDimension();
	out_binX = 0;
	out_binY = 0;
	out_binZ = 0;

	if (dimension == 2)
	{
		int nX = in_h2->GetNbinsX()+2;		
			// add 2 because ROOT has an extra initial bin and final bin (under- and overflow) in each dimension
		out_binX = in_bin % nX;
		out_binY = (in_bin - out_binX)/nX;
		out_binZ = 1;
	}
	else
		cout << "WARNING, dimension not implemented: " << dimension << endl;
	
}

// =====================================

double GetDistance(const C3Vector& in_v1, const C3Vector& in_v2)
{
	double dx = in_v2.m_x - in_v1.m_x;
	double dy = in_v2.m_y - in_v1.m_y;
	double dz = in_v2.m_z - in_v1.m_z;
	return sqrt(dx*dx + dy*dy + dz*dz);
}






