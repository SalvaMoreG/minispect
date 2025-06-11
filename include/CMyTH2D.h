
#ifndef __miniSPECT_CMyTH2D_H___
#define __miniSPECT_CMyTH2D_H___

#include "TH2D.h"
#include "CVIP3Vector.h"

class MyTH2D : public TH2D
{
public:
    MyTH2D() {}
    MyTH2D(const char* name, const char* title,
            Int_t nbinsx, Double_t xlow, Double_t xup,
            Int_t nbinsy, Double_t ylow, Double_t yup)
        : TH2D(name, title, nbinsx, xlow, xup, nbinsy, ylow, yup) {}
    ~MyTH2D() {}
    
    int GetBin(int in_binX, int in_binY) const;
    void GetBins(int in_bin, int& out_binX, int& out_binY) const;

    void GetPosition(int in_binX, int in_binY, C3Vector& out_vector) const;
    void GetBins(const C3Vector& in_vector, int& out_binX, int& out_binY ) const;    
    
private:
};

#endif
