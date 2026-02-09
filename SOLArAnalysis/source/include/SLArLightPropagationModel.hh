/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArLightPropagationModel.hh
 * @created     : lunedì ott 17, 2022 18:00:41 CEST
 *
 * @brief       : class containing functions for calculating number 
 *                of hits on each optical channel via the solid angle method
 *                using gaisser-hillas corrections for Rayleigh scattering
 * 
 * Reimplemented from CLSim (Till Dieminger), DUNE simulation framework,
 * based on 
 *
 * D. Garcia-Gamez, P. Green, A.M. Szelc, 
 * "Predicting Transport Effects of Scintillation Light Signals in 
 * Large-Scale Liquid Argon Detectors", 
 * Published in: Eur.Phys.J.C 81 (2021) 4, 349 • e-Print: 2010.00324 [physics.ins-det]
 */

#ifndef SLARLIGHTPROPAGATIONMODEL_HH

#define SLARLIGHTPROPAGATIONMODEL_HH



#include <vector>
#include <string>

#include "TF1.h"
#include "TVector3.h"


#include "config/SLArCfgBaseSystem.hh"
#include "config/SLArCfgMegaTile.hh"
#include "config/SLArCfgReadoutTile.hh"
#include "config/SLArCfgSuperCell.hh"

namespace slarAna {
  /*! \enum EDetectorFace
   *
   *  Detector face: top, bottom, downstream (positive z), upstream (negative z)
   *  north, south (respect to the beam direction)
   */
  enum EDetectorFace {kTop = 0, kBottom = 1, kDownstrm = 2, 
                      kUpstrm = 3, kNorth = 4, kSouth = 5};
  enum EDetectorClass {kSuperCell, kReadoutTile}; 

  extern TString DetectorFaceName[6];

  class SLArLightPropagationModel {

    private:
      // useful constants
      const double pi = 3.141592653589793;

      // Parameters for fCorr function - fit to plot from LArQL Paper
      std::vector<double> fcorr_param = {0.032, 0.008258}; 

      // DUNE-SP Gaisser-Hillas angle bins
      std::vector<double> angulo = {5, 15, 25, 35, 45, 55, 65, 75, 85};
      const double delta_angle = 10.;

      bool _mathmore_loaded_ = false;

      std::map<EDetectorFace, EDetectorClass> fFaceClass;

    public:
      // constructor
      SLArLightPropagationModel();

      // destructor
      ~SLArLightPropagationModel(){};

      double VisibilityOpDetTile(
          SLArCfgBaseModule* cfgTile, 
          const TVector3 &ScintPoint);

      double VisibilityOpDetTile(
          SLArCfgBaseModule* cfgTile, 
          const TVector3 &ScintPoint,
          const TVector3 &opDetTranslation = TVector3(0,0,0),
          const TVector3 &scintTranslation = TVector3(0,0,0)
          );
      
      // gaisser-hillas function
      static Double_t GaisserHillas(double x, double *par);

      // solid angle of rectangular aperture calculation functions
      double omega(const double &a, const double &b, const double &d) const;
      double solid(SLArCfgReadoutTile* cfgTile, TVector3 &v, EDetectorFace kFace, bool verbose = false); 
      inline double solid(SLArCfgReadoutTile* cfgTile, TVector3 &detrframe, TVector3 &v, EDetectorFace kFace, bool verbose = false)
      {
        TVector3 vv = v + detrframe;
        return solid((SLArCfgReadoutTile*)cfgTile, vv, kFace, verbose);
      }
      double solid(SLArCfgSuperCell* cfgTile, TVector3 &v, EDetectorFace kFace, bool verbose = false); 
      inline double solid(SLArCfgSuperCell* cfgTile, TVector3& detrframe, TVector3 &v, EDetectorFace kFace, bool verbose = false)
      {
        TVector3 vv = v + detrframe;
        return solid((SLArCfgSuperCell*)cfgTile, vv, kFace, verbose);
      }
      double solid_old(SLArCfgReadoutTile* cfgTile, TVector3 &v); 

      // solid angle of circular aperture calculation functions
      double Disk_SolidAngle(double *x, double *p);
      double Disk_SolidAngle(double d, double h, double b);

      // solid angle of dome (PMTs)
      double Omega_Dome_Model(const double distance, const double theta) const;

      // linear interpolation function
      double interpolate( const std::vector<double> &xData, const std::vector<double> &yData, double x, bool extrapolate );

      // Set detector class for given TPC face
      void SetDetectorClass(EDetectorFace, EDetectorClass); 
  };
}
#endif /* end of include guard SLARLIGHTPROPAGATIONMODEL_HH */
