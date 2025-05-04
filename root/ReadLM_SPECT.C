

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

	TCanvas* m_E = new TCanvas("m_1", "E (keV)", 600, 600);
	TCanvas* m_XY = new TCanvas("m_XY", "XY", 1200, 600);
	TCanvas* m_XY_Ecut = new TCanvas("m_XY_Ecut", "XY Ecut", 1200, 600);
	TCanvas* m_z = new TCanvas("m_z", "Z", 300, 300);
	TCanvas* m_lp = new TCanvas("m_lp", "line profiles", 800, 800);

	TH1D* hE = new TH1D("hE", "E", 200, 0.0, 200);
	TH1D* hz = new TH1D("hz", "Z", 50, 0., 50.);
	TH2D* hXY = new TH2D("hXY",  "hitsXY", 100, -50.0, 50.0, 50, -25.0, 25.0);
	TH2D* hXY_Ecut = new TH2D("hXY_Ecut",  "hitsXY_Ecut", 100, -50.0, 50.0, 50, -25.0, 25.0);

	TH1D* lp_X_1 = new TH1D("lp_X_1", "line profile @ y = 7.5 (Ecut)", 40, -20.0, 20.0);
	TH1D* lp_X_2 = new TH1D("lp_X_2", "line profile @ y = 2.5 (Ecut)", 40, -20.0, 20.0);
	TH1D* lp_X_3 = new TH1D("lp_X_3", "line profile @ y = -2.5 (Ecut)", 40, -20.0, 20.0);
	TH1D* lp_X_4 = new TH1D("lp_X_4", "line profile @ y = -7.5 (Ecut)", 40, -20.0, 20.0);

	string dummy;
	double E, x, y, z, radius;
	unsigned long long int time_ps;
	unsigned int count(0);
	const double Emargen( 5.1 ); // keV; 5.1 = 3 * 1.7, sigma = 1.7
	while ( !ffile.eof() ) // && count < 100000 )
	{
		ffile >> dummy >> dummy >> time_ps >> E >> x >> y >> z;
		ffile.ignore(1024, '\n');
		if ( !ffile.eof() )
		{
			if (count == 0) cout << count << ", First time: " << time_ps << endl;
			hE->Fill(E*1000.0);	// MeV to keV
			hXY->Fill(x, y);
	
			if ( fabs(E*1000.0 - 140) < Emargen)
			{
				hXY_Ecut->Fill(x, y);
		
				if (y > 7 && y < 8) lp_X_1->Fill(x);
				else if (y >  2 && y <  3) lp_X_2->Fill(x);
				else if (y < -2 && y > -3) lp_X_3->Fill(x);
				else if (y < -7 && y > -8) lp_X_4->Fill(x);
			}
			
			hz->Fill(z);
			count++;
		}
	}
	cout << count << ", Last time: " << time_ps << endl;

	const double Emin(140 - Emargen);
	const double Emax(140 + Emargen);
	
	m_E->cd();
	hE->GetXaxis()->SetTitle("Energy [keV]");
	hE->GetXaxis()->SetTitleSize(0.04);
	hE->GetXaxis()->SetLabelSize(0.04);
	hE->GetYaxis()->SetLabelSize(0.04);
	hE->Draw();	
	TF1* fitGauss = ((TF1 *)(gROOT->GetFunction("gaus")));
	fitGauss->SetParLimits(0, 100, count);
	fitGauss->SetParLimits(1, Emin, Emax);
	fitGauss->SetParLimits(2, 0.5, 10);
	hE->Fit("gaus","B","", Emin, Emax);

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

	m_XY_Ecut->cd();
	hXY_Ecut->GetXaxis()->SetTitle("x [mm]");
	hXY_Ecut->GetXaxis()->SetTitleSize(0.04);
	hXY_Ecut->GetYaxis()->SetTitle("y [mm]");
	hXY_Ecut->GetYaxis()->SetTitleSize(0.04);
	hXY_Ecut->GetXaxis()->SetLabelSize(0.04);
	hXY_Ecut->GetYaxis()->SetLabelSize(0.04);
	gPad->SetLeftMargin(0.15);
	gPad->SetRightMargin(0.20);
	hXY_Ecut->Draw("COLZ");	

	m_z->cd();
	hz->Draw();	

	m_lp->cd();
	m_lp->Divide(2, 2);
	m_lp->cd(1);
	lp_X_1->Draw("HIST C");
	m_lp->cd(2);
	lp_X_2->Draw("HIST C");
	m_lp->cd(3);
	lp_X_3->Draw("HIST C");
	m_lp->cd(4);
	lp_X_4->Draw("HIST C");

	return 0;
}

