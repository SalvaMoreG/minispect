
#include <iostream>

#include "TCanvas.h"
#include "CMyTH2D.h"
#include "TPaveStats.h"

using namespace std;

int main()
{
	
	MyTH2D* myTH2D = new MyTH2D("myTH2D", " ", 100, 0., 100., 100, 0., 100.);
	for (int i = 0; i < 100; i++)
	{
        myTH2D->Fill(i, 100-i, 1234);
	}
	TCanvas* m1 = new TCanvas("m1", " ", 500, 500);
	myTH2D->Draw("COLZ");

    // TPaveStats* stat = dynamic_cast<TPaveStats*>(myTH2D->FindObject("stats"));
    TPaveStats* stat = (TPaveStats*)myTH2D->FindObject("stats");
    if (stat) {
        stat->SetX1NDC(0.1); stat->SetX2NDC(0.3);
        stat->Draw();
    } else {
        cout << "ERROR! No stats box found!" << endl;
    }

	m1->Print("test.png", "png");

	return 0;
}

