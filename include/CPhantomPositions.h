
#ifndef __miniSPECT_PhantomPositions_H___
#define __miniSPECT_PhantomPositions_H___

#include "CVIP3Vector.h"

#include "CMyTH2D.h"
#include <vector>
#include <numeric>


class CPhantomPositions
{
public:
					CPhantomPositions() {}
	virtual			~CPhantomPositions() {}

	// abstract interface method
    virtual void    GetTruePositions( std::vector<C3Vector>& out_positions, bool debug = false ) const {};
    void            GetResolution( MyTH2D* in_hXY, std::vector<C3Vector>& in_positions, double& x_resolution, double& y_resolution ) const;
    void            GetTruePositionsInFOV( MyTH2D* in_hXY, std::vector<C3Vector>& out_positions, bool threshold ) const;
    void            GetRealPositionsInFOV( MyTH2D* in_hXY,  std::vector<C3Vector>& in_positions, std::vector<C3Vector>& out_positions, double& deviation_mu, double& deviation_sigma ) const;
    void            GetIfGaussian( MyTH2D* in_hXY,  std::vector<C3Vector>& in_positions, bool& gaussian) const;
    void            GetPeakHeightDistribution( MyTH2D* in_hXY,  std::vector<C3Vector>& in_positions, double& height_mu, double& height_sigma ) const;
    virtual void    FindPositionsIn2DHisto( MyTH2D* in_hXY,
                        std::vector<int>& out_positions, bool debug ) const;
                    // Due to a design flaw in TH2D, it cannot be passed as const
                    // (Some calls to TH2D are not const calls...)
private:
    void            GetBinLimits( MyTH2D* in_hXY, C3Vector in_position, std::vector<int>& out_bin_positions, bool& x_limit, bool& y_limit ) const;
};

//
// This is mainly for the extremely rare case of a Nx * Ny rectangle phantom
//
class CSimplePhantomPositions : public CPhantomPositions
{
public:
					CSimplePhantomPositions() {}
	virtual			~CSimplePhantomPositions() {}

	void            GetTruePositions( std::vector<C3Vector>& out_positions, bool debug = false ) const override;

    void            FindPeakToValleys(MyTH2D* in_hXY,
            std::vector<int>& in_peak_position_bins,
            std::vector<double>& out_found_p2vs, double& out_average_p2vs, double& out_sigma_p2vs,
            std::vector<int>& out_lp_ybins, bool debug ) const;

private:
};

#endif
