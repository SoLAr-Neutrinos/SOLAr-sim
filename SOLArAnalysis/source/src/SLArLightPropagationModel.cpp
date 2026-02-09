#include "SLArLightPropagationModel.hh"
#include "SLArLightPropagationPars.hpp"

#include <iostream>
#include <fstream>
#include <cmath>

#include "TRandom.h"
#include "TSystem.h"
#include "TMath.h"
#include "TFormula.h"
#include "Math/SpecFuncMathMore.h"

#include "G4UIcommand.hh"

using namespace std;
bool debug_2 = false;

namespace slarAna {
  
  TString DetectorFaceName[6] = {"Top", "Bottom", "Downstream", 
                                 "Upstream", "North", "South"}; 

  // constructor
  SLArLightPropagationModel::SLArLightPropagationModel() {

    // load mathmore library
    gSystem->Load("libMathMore.so");
    if(gSystem->Load("libMathMore.so") < 0) {
      throw(std::runtime_error("Unable to load MathMore library"));
    }
    _mathmore_loaded_ = true;

    std::cout << "Light simulation for DUNE Single Phase detector." << std::endl;
    std::cout << std::endl;
  }

  void SLArLightPropagationModel::SetDetectorClass(
      EDetectorFace kFace, EDetectorClass kClass) {
    if (fFaceClass.count(kFace) == 0) {
      fFaceClass.insert(std::make_pair(kFace, kClass));
    } else {
      fFaceClass[kFace] = kClass; 
    }
  }

  double SLArLightPropagationModel::VisibilityOpDetTile(
      SLArCfgBaseModule* cfgTile,
      const TVector3 &ScintPoint) 
  {

    TVector3 OpDetPoint(
        cfgTile->GetPhysX()/G4UIcommand::ValueOf("cm"), 
        cfgTile->GetPhysY()/G4UIcommand::ValueOf("cm"), 
        cfgTile->GetPhysZ()/G4UIcommand::ValueOf("cm"));

    TVector3 ScintPoint_rel = ScintPoint - OpDetPoint; 


    double costheta   = 0.; 
    double theta      = 0.; 
    double distance   = (ScintPoint_rel).Mag();

    double r_distance = -1; 
    double solid_angle= -1; 

    TVector3 OpDetNorm(-1, 0, 0);
    OpDetNorm = cfgTile->GetNormal(); 

    EDetectorFace kFace = kDownstrm; 
    if (OpDetNorm        == TVector3(+1, 0, 0)) {
      kFace = kSouth; 
    } else if (OpDetNorm == TVector3(-1, 0, 0)) {
      kFace = kNorth; 
    } else if (OpDetNorm == TVector3(0, +1, 0)) {
      kFace = kBottom; 
    } else if (OpDetNorm == TVector3(0, -1, 0)) {
      kFace = kTop; 
    } else if (OpDetNorm == TVector3(0, 0, +1)) {
      kFace = kUpstrm; 
    } else if (OpDetNorm == TVector3(0, 0, -1)) {
      kFace = kDownstrm;
    } else {
      printf("WARNING: Optical module %s[%i] has normal [%.2f, %.2f, %.2f]\n", 
          cfgTile->GetName(), cfgTile->GetIdx(), OpDetNorm.x(), OpDetNorm.y(), OpDetNorm.z());
      printf("which is not among the expected directions.\n\n"); 
      getchar();
    }

    EDetectorClass kClass; 
    if (fFaceClass.count(kFace) == 0) {
      printf("%s norm [%.1f, %.1f, %.1f], pos [%g, %g, %g] cm\n", 
          cfgTile->GetName(), OpDetNorm.x(), OpDetNorm.y(), OpDetNorm.z(), 
          OpDetPoint.x(), OpDetPoint.y(), OpDetPoint.z());

      printf("WARNING: Unknown detector type for %s face of the TPC\n", 
          DetectorFaceName[kFace].Data());
      return 0.; 
    }

    kClass = fFaceClass[kFace]; 

    costheta   = (ScintPoint_rel).Unit().Dot(OpDetNorm); 
    theta = acos(costheta)*TMath::RadToDeg();
    TVector3 r_ =  (
        ScintPoint - 
        OpDetNorm*(OpDetNorm.Dot(ScintPoint - TVector3(0, 0, 0)))
        );
    r_distance = r_.Mag();
    printf("r_ : (%g, %g, %g) - |r_| = %g\n", 
        r_.x(), r_.y(), r_.z(), r_distance);


/*
 *    printf("----------------------------------------\n");
 *    printf("Optical detector: %s - face: %s - class: %s\n", 
 *        cfgTile->GetName(), DetectorFaceName[kFace].Data(), 
 *        (kClass == kReadoutTile) ? "ReadoutTile" :
 *        (kClass == kSuperCell)   ? "SuperCell"   : "Unknown");
 *    printf("scint pos: (%g, %g, %g) cm - tile pos: (%g, %g, %g):\n\td = %g cm - cosθ = %g - θ = %g deg\n", 
 *        ScintPoint.x(), ScintPoint.y(), ScintPoint.z(), 
 *        OpDetPoint.x(), OpDetPoint.y(), OpDetPoint.z(), 
 *        distance, costheta, theta);
 */

    if (costheta < 0.001)
      solid_angle = 0;
    else {
      if (kClass == kReadoutTile) { 
        solid_angle= solid((SLArCfgReadoutTile*)cfgTile, ScintPoint_rel, kFace);
        //double solid_angle_test = solid_old((SLArCfgReadoutTile*)cfgTile, ScintPoint_rel); 
        //double delta_solid = solid_angle - solid_angle_test; 
        //if (delta_solid > 1e-8) {
          //printf("Ω = %g - Ω old = %g [δ = %g]\n", solid_angle, solid_angle_test, 
              //delta_solid);
        //}
      }
      else if (kClass == kSuperCell) 
        solid_angle = solid((SLArCfgSuperCell*)cfgTile, ScintPoint_rel, kFace);
    }

    // calculate solid angle
    if(solid_angle < 0){
      std::cout << "Error: solid angle is negative" << std::endl;
      exit(1);
    }
    if(r_distance < 0){
      std::cout << "Error: r_distance is negative" << std::endl;
      exit(1);
    }
    

    double vis_geo = exp(-1.*distance/L_abs) * (solid_angle / (4*pi));


    // determine Gaisser-Hillas correction for Rayleigh scattering, 
    // distance and angular dependence, accounting for border effects
    // offset angle bin
    if(theta>89.0){
      return vis_geo;
    }
    int j = (theta/delta_angle);

    // identify GH parameters and border corrections by optical detector type 
    // and scintillation type
    double pars_ini[4] = {0,0,0,0};
    double s1, s2, s3;
    int scintillation_type = 0;  // TODO: Fix for argon
    if (scintillation_type == 0) { // argon
      if (j >= 8){
        pars_ini[0] = fGHVUVPars_flat_argon[0][j];
        pars_ini[1] = fGHVUVPars_flat_argon[1][j];
        pars_ini[2] = fGHVUVPars_flat_argon[2][j];
        pars_ini[3] = fGHVUVPars_flat_argon[3][j];
      }
      else{
        double temp1 = fGHVUVPars_flat_argon[0][j];
        double temp2 = fGHVUVPars_flat_argon[0][j+1];
        pars_ini[0] = temp1 + (temp2-temp1)*(theta-j*delta_angle)/delta_angle;

        temp1 = fGHVUVPars_flat_argon[1][j];
        temp2 = fGHVUVPars_flat_argon[1][j+1];
        pars_ini[1] = temp1 + (temp2-temp1)*(theta-j*delta_angle)/delta_angle;

        temp1 = fGHVUVPars_flat_argon[2][j];
        temp2 = fGHVUVPars_flat_argon[2][j+1];
        pars_ini[2] = temp1 + (temp2-temp1)*(theta-j*delta_angle)/delta_angle;

        temp1 = fGHVUVPars_flat_argon[3][j];
        temp2 = fGHVUVPars_flat_argon[3][j+1];
        pars_ini[3] = temp1 + (temp2-temp1)*(theta-j*delta_angle)/delta_angle;
      }

      s1 = interpolate( angulo, slopes1_flat_argon, theta, true);
      s2 = interpolate( angulo, slopes2_flat_argon, theta, true);
      s3 = interpolate( angulo, slopes3_flat_argon, theta, true);
    }
    else if (scintillation_type == 1) { // xenon
      pars_ini[0] = fGHVUVPars_flat_xenon[0][j];
      pars_ini[1] = fGHVUVPars_flat_xenon[1][j];
      pars_ini[2] = fGHVUVPars_flat_xenon[2][j];
      pars_ini[3] = fGHVUVPars_flat_xenon[3][j];
      s1 = interpolate( angulo, slopes1_flat_xenon, theta, true);
      s2 = interpolate( angulo, slopes2_flat_xenon, theta, true);
      s3 = interpolate( angulo, slopes3_flat_xenon, theta, true);
    }
    else {
      std::cout << "Error: Invalid scintillation type configuration." << endl;
      exit(1);
    }
    // add border correction
    // FIXME: set r_distance to zero for testing purposes
    r_distance = 0;
    pars_ini[0] = pars_ini[0] + s1 * r_distance;
    pars_ini[1] = pars_ini[1] + s2 * r_distance;
    pars_ini[2] = pars_ini[2] + s3 * r_distance;
    pars_ini[3] = pars_ini[3];

    // calculate correction factor
    double GH_correction = GaisserHillas(distance, pars_ini);

    // apply correction
    double vis_vuv = 0 ;
    vis_vuv = GH_correction*vis_geo/costheta;

    //printf("\tGH_correction = %g\n", GH_correction);
    //printf("\tvis_vuv = %g\n", vis_vuv);
    //getchar(); 

    return vis_vuv;
  }


  double SLArLightPropagationModel::VisibilityOpDetTile(
      SLArCfgBaseModule* cfgTile,
      const TVector3 &ScintPoint, 
      const TVector3 &OpDetTranslation,
      const TVector3 &scintTranslation) 
  {

    TVector3 OpDetPoint(
        cfgTile->GetPhysX()/G4UIcommand::ValueOf("cm"), 
        cfgTile->GetPhysY()/G4UIcommand::ValueOf("cm"), 
        cfgTile->GetPhysZ()/G4UIcommand::ValueOf("cm"));

    OpDetPoint = OpDetPoint - OpDetTranslation;

    TVector3 ScintPoint_tpc = ScintPoint - scintTranslation;

    TVector3 ScintPoint_rel = ScintPoint_tpc - OpDetPoint; 


    double costheta   = 0.; 
    double theta      = 0.; 
    double distance   = (ScintPoint_rel).Mag();

    double r_distance = -1; 
    double solid_angle= -1; 

    TVector3 OpDetNorm(-1, 0, 0);
    OpDetNorm = cfgTile->GetNormal(); 
    TVector3 OpDetPlaneCenter(OpDetPoint); 

    EDetectorFace kFace = kDownstrm; 
    if (OpDetNorm        == TVector3(+1, 0, 0)) {
      kFace = kSouth; 
      OpDetPlaneCenter.SetZ(0.0); OpDetPlaneCenter.SetY(0.0);
    } else if (OpDetNorm == TVector3(-1, 0, 0)) {
      kFace = kNorth; 
      OpDetPlaneCenter.SetY(0.0); OpDetPlaneCenter.SetZ(0.0);
    } else if (OpDetNorm == TVector3(0, +1, 0)) {
      kFace = kBottom; 
      OpDetPlaneCenter.SetX(0.0); OpDetPlaneCenter.SetZ(0.0);
    } else if (OpDetNorm == TVector3(0, -1, 0)) {
      kFace = kTop; 
      OpDetPlaneCenter.SetX(0.0); OpDetPlaneCenter.SetZ(0.0);
    } else if (OpDetNorm == TVector3(0, 0, +1)) {
      kFace = kUpstrm; 
      OpDetPlaneCenter.SetX(0.0); OpDetPlaneCenter.SetY(0.0);
    } else if (OpDetNorm == TVector3(0, 0, -1)) {
      kFace = kDownstrm;
      OpDetPlaneCenter.SetX(0.0); OpDetPlaneCenter.SetY(0.0);
    } else {
      printf("WARNING: Optical module %s[%i] has normal [%.2f, %.2f, %.2f]\n", 
          cfgTile->GetName(), cfgTile->GetIdx(), OpDetNorm.x(), OpDetNorm.y(), OpDetNorm.z());
      printf("which is not among the expected directions.\n\n"); 
      getchar();
    }

    EDetectorClass kClass; 
    if (fFaceClass.count(kFace) == 0) {
      printf("%s norm [%.1f, %.1f, %.1f], pos [%g, %g, %g] cm\n", 
          cfgTile->GetName(), OpDetNorm.x(), OpDetNorm.y(), OpDetNorm.z(), 
          OpDetPoint.x(), OpDetPoint.y(), OpDetPoint.z());

      printf("WARNING: Unknown detector type for %s face of the TPC\n", 
          DetectorFaceName[kFace].Data());
      return 0.; 
    }

    kClass = fFaceClass[kFace]; 

    costheta   = (ScintPoint_rel).Unit().Dot(OpDetNorm); 
    theta = acos(costheta)*TMath::RadToDeg();
    TVector3 ScintPoint_rel_plane = ScintPoint_tpc - OpDetPlaneCenter;
    TVector3 r1 =  (
        ScintPoint_rel_plane -
        OpDetNorm*(OpDetNorm.Dot(ScintPoint_rel_plane))
        ); // Projection of the scint point onto opdet plane
    r_distance = r1.Mag();

    /*
     *printf("----------------------------------------\n");
     *printf("Optical detector: %s - face: %s - class: %s\n", 
     *    cfgTile->GetName(), DetectorFaceName[kFace].Data(), 
     *    (kClass == kReadoutTile) ? "ReadoutTile" :
     *    (kClass == kSuperCell)   ? "SuperCell"   : "Unknown");
     *printf("scint pos: (%g, %g, %g) cm - tile pos: (%g, %g, %g):\n\td = %g cm - cosθ = %g - θ = %g deg\n", 
     *    ScintPoint_tpc.x(), ScintPoint_tpc.y(), ScintPoint_tpc.z(), 
     *    OpDetPoint.x(), OpDetPoint.y(), OpDetPoint.z(), 
     *    distance, costheta, theta);
     *printf("OpDetPlaneCenter : (%g, %g, %g)\n", 
     *    OpDetPlaneCenter.x(), OpDetPlaneCenter.y(), OpDetPlaneCenter.z());
     *printf("r1 : (%g, %g, %g) - |r_distance| = %g\n", 
     *    r1.x(), r1.y(), r1.z(), r_distance);
     */


    if (costheta < 0.001)
      solid_angle = 0;
    else {
      if (kClass == kReadoutTile) { 
        solid_angle= solid((SLArCfgReadoutTile*)cfgTile, ScintPoint_rel, kFace);
        //double solid_angle_test = solid_old((SLArCfgReadoutTile*)cfgTile, ScintPoint_rel); 
        //double delta_solid = solid_angle - solid_angle_test; 
        //if (delta_solid > 1e-8) {
          //printf("Ω = %g - Ω old = %g [δ = %g]\n", solid_angle, solid_angle_test, 
              //delta_solid);
        //}
      }
      else if (kClass == kSuperCell) 
        solid_angle = solid((SLArCfgSuperCell*)cfgTile, ScintPoint_rel, kFace);
    }

    // calculate solid angle
    if(solid_angle < 0){
      std::cout << "Error: solid angle is negative" << std::endl;
      exit(1);
    }
    if(r_distance < 0){
      std::cout << "Error: r_distance is negative" << std::endl;
      exit(1);
    }
    

    double vis_geo = exp(-1.*distance/L_abs) * (solid_angle / (4*pi));


    // determine Gaisser-Hillas correction for Rayleigh scattering, 
    // distance and angular dependence, accounting for border effects
    // offset angle bin
    if(theta>89.0){
      return vis_geo;
    }
    int j = (theta/delta_angle);

    // identify GH parameters and border corrections by optical detector type 
    // and scintillation type
    double pars_ini[4] = {0,0,0,0};
    double s1, s2, s3;
    int scintillation_type = 0;  // TODO: Fix for argon
    const double (*GH_VUV_PARS)[9] = nullptr;
    const auto& slopes1_ = ( kFace == kBottom || kFace == kTop) ? 
      slopes1_flat_argon : slopes1_flat_argon;
    const auto& slopes2_ = ( kFace == kBottom || kFace == kTop) ?
      slopes2_flat_argon : slopes2_flat_argon;
    const auto& slopes3_ = ( kFace == kBottom || kFace == kTop) ?
      slopes3_flat_argon : slopes3_flat_argon;

    if ( kFace == kBottom || kFace == kTop) {
      GH_VUV_PARS = fGHVUVPars_flat_argon;
    }
    else {
      GH_VUV_PARS = fGHVUVPars_flat_argon; 
    }

    if (scintillation_type == 0) { // argon
      if (j >= 8){
        pars_ini[0] = GH_VUV_PARS[0][j];
        pars_ini[1] = GH_VUV_PARS[1][j];
        pars_ini[2] = GH_VUV_PARS[2][j];
        pars_ini[3] = GH_VUV_PARS[3][j];
      }
      else{
        double temp1 = GH_VUV_PARS[0][j];
        double temp2 = GH_VUV_PARS[0][j+1];
        pars_ini[0] = temp1 + (temp2-temp1)*(theta-j*delta_angle)/delta_angle;

        temp1 = GH_VUV_PARS[1][j];
        temp2 = GH_VUV_PARS[1][j+1];
        pars_ini[1] = temp1 + (temp2-temp1)*(theta-j*delta_angle)/delta_angle;

        temp1 = GH_VUV_PARS[2][j];
        temp2 = GH_VUV_PARS[2][j+1];
        pars_ini[2] = temp1 + (temp2-temp1)*(theta-j*delta_angle)/delta_angle;

        temp1 = GH_VUV_PARS[3][j];
        temp2 = GH_VUV_PARS[3][j+1];
        pars_ini[3] = temp1 + (temp2-temp1)*(theta-j*delta_angle)/delta_angle;
      }

      s1 = interpolate( angulo, slopes1_, theta, true);
      s2 = interpolate( angulo, slopes2_, theta, true);
      s3 = interpolate( angulo, slopes3_, theta, true);
    }
    else if (scintillation_type == 1) { // xenon
      pars_ini[0] = fGHVUVPars_flat_xenon[0][j];
      pars_ini[1] = fGHVUVPars_flat_xenon[1][j];
      pars_ini[2] = fGHVUVPars_flat_xenon[2][j];
      pars_ini[3] = fGHVUVPars_flat_xenon[3][j];
      s1 = interpolate( angulo, slopes1_, theta, true);
      s2 = interpolate( angulo, slopes2_, theta, true);
      s3 = interpolate( angulo, slopes3_, theta, true);
    }
    else {
      std::cout << "Error: Invalid scintillation type configuration." << endl;
      exit(1);
    }
    // add border correction
    pars_ini[0] = pars_ini[0] + s1 * r_distance;
    pars_ini[1] = pars_ini[1] + s2 * r_distance;
    pars_ini[2] = pars_ini[2] + s3 * r_distance;
    pars_ini[3] = pars_ini[3];

    // calculate correction factor
    double GH_correction = GaisserHillas(distance, pars_ini);

    // apply correction
    double vis_vuv = 0 ;
    vis_vuv = GH_correction*vis_geo/costheta;

    //printf("\tGH_correction = %g\n", GH_correction);
    //printf("\tvis_vuv = %g\n", vis_vuv);
    //getchar(); 

    return vis_vuv;
  }



  // gaisser-hillas function definition
  Double_t SLArLightPropagationModel::GaisserHillas(double x,double *par) {
    //This is the Gaisser-Hillas function
    Double_t X_mu_0=par[3];
    Double_t Normalization=par[0];
    Double_t Diff=par[1]-X_mu_0;
    Double_t Term=pow((x-X_mu_0)/Diff,Diff/par[2]);
    Double_t Exponential=TMath::Exp((par[1]-x)/par[2]);

    return ( Normalization*Term*Exponential);
  }


  // solid angle of rectanglular aperture calculation functions

  double SLArLightPropagationModel::omega(const double &a, const double &b, const double &d) const{

    double aa = 0.5*a/d;
    double bb = 0.5*b/d;
    double aux = (1+aa*aa+bb*bb)/((1.+aa*aa)*(1.+bb*bb));
    return 4*std::acos(std::sqrt(aux));

  }

  double SLArLightPropagationModel::solid(
      SLArCfgReadoutTile* cfgTile, 
      TVector3 &v, 
      EDetectorFace kFace) {

    TVector3 OpDetNorm = cfgTile->GetNormal();     

    // TODO: Fix to make it independent of detector plane
    // detector size
    TVector3 detSize(0, 0, 0); 
    TVector3 XPlane[2]; 

    if (kFace == kNorth || kFace == kSouth) {
      detSize.SetZ((cfgTile->GetAxis0().Dot(cfgTile->GetSize()))/G4UIcommand::ValueOf("cm")); 
      detSize.SetY((cfgTile->GetAxis1().Dot(cfgTile->GetSize()))/G4UIcommand::ValueOf("cm"));
      XPlane[0] = TVector3(0, 0, 1); 
      XPlane[1] = TVector3(0, 1, 0); 
    } else if (kFace == kTop || kFace == kBottom) {
      detSize.SetZ((cfgTile->GetAxis0().Dot(cfgTile->GetSize()))/G4UIcommand::ValueOf("cm")); 
      detSize.SetX((cfgTile->GetAxis1().Dot(cfgTile->GetSize()))/G4UIcommand::ValueOf("cm")); 
      XPlane[0] = TVector3(0, 0, 1); 
      XPlane[1] = TVector3(1, 0, 0); 
    } else if (kFace == kUpstrm || kFace == kDownstrm) {
      detSize.SetX((cfgTile->GetAxis0().Dot(cfgTile->GetSize()))/G4UIcommand::ValueOf("cm")); 
      detSize.SetY((cfgTile->GetAxis1().Dot(cfgTile->GetSize()))/G4UIcommand::ValueOf("cm"));  
      XPlane[0] = TVector3(1, 0, 0); 
      XPlane[1] = TVector3(0, 1, 0); 
    }

    // project scintillation point onto the photon detector plane
    TVector3 vv = v - OpDetNorm.Dot(v)*OpDetNorm; 

    // The hit is directly above the SiPM
    if( (vv - detSize).Mag2() == 0) {
      return omega(detSize.Dot(XPlane[0]),detSize.Dot(XPlane[1]),v.Mag());
    }

    bool isOut[2] = {false, false};
    double  Dx[2] = {0., 0.}; 
    double  dx[2] = {0., 0.}; 
    double  d     = std::fabs(v.Dot(OpDetNorm));

    for (int j=0; j<2; j++) {
      isOut[j] = std::fabs(vv.Dot(XPlane[j])) > 0.5*detSize.Dot(XPlane[j]); 
    }

    //printf("vv[0] = %g, vv[1] = %g, vv[2] = %g: %i - %i; d = %g\n", 
        //vv.x(), vv.y(), vv.z(), isOut[0], isOut[1], d); 

    if (isOut[0] == false && isOut[1] == false) {
      Dx[0] = 0.5*detSize.Dot(XPlane[0]) - std::fabs(vv.Dot(XPlane[0])); 
      Dx[1] = 0.5*detSize.Dot(XPlane[1]) - std::fabs(vv.Dot(XPlane[1])); 

      dx[0] = detSize.Dot(XPlane[0]); 
      dx[1] = detSize.Dot(XPlane[1]); 
      double to_return = (
           +omega(2*(dx[0]-Dx[0]), 2*(dx[1]-Dx[1]), d)
           +omega(2*Dx[0]        , 2*(dx[1]-Dx[1]), d)
           +omega(2*(dx[0]-Dx[0]), 2*Dx[1]        , d)
           +omega(2*Dx[0]        , 2*Dx[1]        , d))*0.25;
      return to_return;
    } else if (isOut[0] == true && isOut[1] == false) {
      Dx[0] = std::fabs(vv.Dot(XPlane[0])) - 0.5*detSize.Dot(XPlane[0]); 
      Dx[1] = 0.5*detSize.Dot(XPlane[1]) - std::fabs(vv.Dot(XPlane[1])); 

      dx[0] = detSize.Dot(XPlane[0]); 
      dx[1] = detSize.Dot(XPlane[1]); 

      double to_return = (
          +omega(2*(Dx[0]+dx[0]), 2*(dx[1]-Dx[1]), d)
          -omega(2*Dx[0]        , 2*(dx[1]-Dx[1]), d)
          +omega(2*(Dx[0]+dx[0]), 2*Dx[1]        , d)
          -omega(2*Dx[0]        , 2*Dx[1]        , d))*0.25;
      return to_return;
    } else if (isOut[0] == false && isOut[1] == true) {
      Dx[0] = 0.5*detSize.Dot(XPlane[0]) - std::fabs(vv.Dot(XPlane[0])); 
      Dx[1] = std::fabs(vv.Dot(XPlane[1])) - 0.5*detSize.Dot(XPlane[1]); 

      dx[0] = detSize.Dot(XPlane[0]); 
      dx[1] = detSize.Dot(XPlane[1]); 

      double to_return = (
          +omega(2*(dx[0]-Dx[0]), 2*(dx[1]+Dx[1]), d)
          -omega(2*(dx[0]-Dx[0]), 2*(Dx[1])      , d)
          +omega(2*(Dx[0])      , 2*(Dx[1]+dx[1]), d)
          -omega(2*Dx[0]        , 2*Dx[1]        , d))*0.25;
      return to_return;
    } else if (isOut[0] == true && isOut[1] == true) {
       Dx[0] = std::fabs(vv.Dot(XPlane[0])) - 0.5*detSize.Dot(XPlane[0]); 
       Dx[1] = std::fabs(vv.Dot(XPlane[1])) - 0.5*detSize.Dot(XPlane[1]); 

       dx[0] = detSize.Dot(XPlane[0]); 
       dx[1] = detSize.Dot(XPlane[1]); 

       double to_return = (
          +omega(2*(Dx[0]+dx[0]), 2*(Dx[1]+dx[1]), d)
          -omega(2*Dx[0]        , 2*(Dx[1]+dx[1]), d)
          -omega(2*(Dx[0]+dx[0]), 2*Dx[1]        , d) 
          +omega(2*Dx[0]        , 2*Dx[1]        , d))*0.25; 

       return to_return; 
    }
    
    // error message if none of these cases, i.e. something has gone wrong!
    std::cout << "Warning: invalid solid angle call." << std::endl;
    return 0.0;
  }

  double SLArLightPropagationModel::solid(
      SLArCfgSuperCell* cfgTile, 
      TVector3 &v, 
      EDetectorFace kFace) {

    TVector3 OpDetNorm = cfgTile->GetNormal();     

    // TODO: Fix to make it independent of detector plane
    // detector size
    TVector3 detSize(0, 0, 0); 
    TVector3 XPlane[2]; 

    if (kFace == kNorth || kFace == kSouth) {
      detSize.SetZ((cfgTile->GetAxis0().Dot(cfgTile->GetSize()))/G4UIcommand::ValueOf("cm")); 
      detSize.SetY((cfgTile->GetAxis1().Dot(cfgTile->GetSize()))/G4UIcommand::ValueOf("cm"));
      XPlane[0] = TVector3(0, 0, 1); 
      XPlane[1] = TVector3(0, 1, 0); 
    } else if (kFace == kTop || kFace == kBottom) {
      detSize.SetZ((cfgTile->GetAxis0().Dot(cfgTile->GetSize()))/G4UIcommand::ValueOf("cm")); 
      detSize.SetX((cfgTile->GetAxis1().Dot(cfgTile->GetSize()))/G4UIcommand::ValueOf("cm")); 
      XPlane[0] = TVector3(0, 0, 1); 
      XPlane[1] = TVector3(1, 0, 0); 
    } else if (kFace == kUpstrm || kFace == kDownstrm) {
      detSize.SetX((cfgTile->GetAxis0().Dot(cfgTile->GetSize()))/G4UIcommand::ValueOf("cm")); 
      detSize.SetY((cfgTile->GetAxis1().Dot(cfgTile->GetSize()))/G4UIcommand::ValueOf("cm"));  
      XPlane[0] = TVector3(1, 0, 0); 
      XPlane[1] = TVector3(0, 1, 0); 
    }

    // project scintillation point onto the photon detector plane
    TVector3 vv = v - OpDetNorm.Dot(v)*OpDetNorm; 

    // The hit is directly above the SiPM
    if( (vv - detSize).Mag2() == 0) {
      return omega(detSize.Dot(XPlane[0]),detSize.Dot(XPlane[1]),v.Mag());
    }

    bool isOut[2] = {false, false};
    double  Dx[2] = {0., 0.}; 
    double  dx[2] = {0., 0.}; 
    double  d     = std::fabs(v.Dot(OpDetNorm));

    for (int j=0; j<2; j++) {
      isOut[j] = std::fabs(vv.Dot(XPlane[j])) > 0.5*(detSize.Dot(XPlane[j])); 
    }
    //printf("vv[0] = %g, vv[1] = %g, vv[2] = %g: %i - %i\n", 
        //vv.x(), vv.y(), vv.z(), isOut[0], isOut[1]); 

    if (isOut[0] == false && isOut[1] == false) {
      Dx[0] = 0.5*detSize.Dot(XPlane[0]) - std::fabs(vv.Dot(XPlane[0])); 
      Dx[1] = 0.5*detSize.Dot(XPlane[1]) - std::fabs(vv.Dot(XPlane[1])); 

      dx[0] = detSize.Dot(XPlane[0]); 
      dx[1] = detSize.Dot(XPlane[1]); 

      double to_return = (
           +omega(2*(dx[0]-Dx[0]),2*(dx[1]-Dx[1]), d)
           +omega(2*Dx[0]        ,2*(dx[1]-Dx[1]), d)
           +omega(2*(dx[0]-Dx[0]),2*Dx[1]        , d)
           +omega(2*Dx[0]        ,2*Dx[1]        , d))*0.25;
      return to_return;
    } else if (isOut[0] == true && isOut[1] == false) {
      Dx[0] = std::fabs(vv.Dot(XPlane[0])) - 0.5*detSize.Dot(XPlane[0]); 
      Dx[1] = 0.5*detSize.Dot(XPlane[1]) - std::fabs(vv.Dot(XPlane[1])); 

      dx[0] = detSize.Dot(XPlane[0]); 
      dx[1] = detSize.Dot(XPlane[1]); 

      double to_return = (
          +omega(2*(Dx[0]+dx[0]), 2*(dx[1]-Dx[1]), d)
          -omega(2*Dx[0]        , 2*(dx[1]-Dx[1]), d)
          +omega(2*(Dx[0]+dx[0]), 2*Dx[1]        , d)
          -omega(2*Dx[0]        , 2*Dx[1]        , d))*0.25;
      return to_return;
    } else if (isOut[0] == false && isOut[1] == true) {
      Dx[0] = 0.5*detSize.Dot(XPlane[0]) - std::fabs(vv.Dot(XPlane[0])); 
      Dx[1] = std::fabs(vv.Dot(XPlane[1])) - 0.5*detSize.Dot(XPlane[1]); 

      dx[0] = detSize.Dot(XPlane[0]); 
      dx[1] = detSize.Dot(XPlane[1]); 

      double to_return = (
          +omega(2*(dx[0]-Dx[0]), 2*(dx[1]+Dx[1]), d)
          -omega(2*(dx[0]-Dx[0]), 2*(Dx[1])      , d)
          +omega(2*(Dx[0])      , 2*(Dx[1]+dx[1]), d)
          -omega(2*Dx[0]        , 2*Dx[1]        , d))*0.25;
      return to_return;
    } else if (isOut[0] == true && isOut[1] == true) {
       Dx[0] = std::fabs(vv.Dot(XPlane[0])) - 0.5*detSize.Dot(XPlane[0]); 
       Dx[1] = std::fabs(vv.Dot(XPlane[1])) - 0.5*detSize.Dot(XPlane[1]); 

       dx[0] = detSize.Dot(XPlane[0]); 
       dx[1] = detSize.Dot(XPlane[1]); 

       d     = std::fabs(v.Dot(OpDetNorm)); 
       
       double to_return = (
          +omega(2*(Dx[0]+dx[0]), 2*(Dx[1]+dx[1]), d)
          -omega(2*Dx[0]        , 2*(Dx[1]+dx[1]), d)
          -omega(2*(Dx[0]+dx[0]), 2*Dx[1]        , d) 
          +omega(2*Dx[0]        , 2*Dx[1]        , d))*0.25; 
       return to_return; 
    }
    
    // error message if none of these cases, i.e. something has gone wrong!
    std::cout << "Warning: invalid solid angle call." << std::endl;
    return 0.0;
  }



  // solid angle of circular aperture
  double SLArLightPropagationModel::Disk_SolidAngle(double *x, double *p) {
    const double d = x[0];
    const double h = x[1];
    const double b = p[0];
    if(b <= 0. || d < 0. || h <= 0.) return 0.;
    const double aa = TMath::Sqrt(h*h/(h*h+(b+d)*(b+d)));
    if(d == 0) {
      return 2.*TMath::Pi()*(1.-aa);
    }
    const double bb = TMath::Sqrt(4*b*d/(h*h+(b+d)*(b+d)));
    const double cc = 4*b*d/((b+d)*(b+d));

    if(!_mathmore_loaded_) {
      if(gSystem->Load("libMathMore.so") < 0) {
        throw(std::runtime_error("Unable to load MathMore library"));
      }
      _mathmore_loaded_ = true;
    }
    if(TMath::Abs(ROOT::Math::comp_ellint_1(bb) - bb) < 1e-10 && TMath::Abs(ROOT::Math::comp_ellint_3(cc,bb) - cc) <1e-10) {
      throw(std::runtime_error("please do gSystem->Load(\"libMathMore.so\") before running Disk_SolidAngle for the first time!"));
    }
    if(d < b) {
      return 2.*TMath::Pi() - 2.*aa*(ROOT::Math::comp_ellint_1(bb) + TMath::Sqrt(1.-cc)*ROOT::Math::comp_ellint_3(cc,bb));
    }
    if(d == b) {
      return TMath::Pi() - 2.*aa*ROOT::Math::comp_ellint_1(bb);
    }
    if(d > b) {
      return 2.*aa*(TMath::Sqrt(1.-cc)*ROOT::Math::comp_ellint_3(cc,bb) - ROOT::Math::comp_ellint_1(bb));
    }

    return 0.;
  }

  double SLArLightPropagationModel::Disk_SolidAngle(double d, double h, double b) {
    double x[2] = { d, h };
    double p[1] = { b };
    if(!_mathmore_loaded_) {
      if(gSystem->Load("libMathMore.so") < 0) {
        throw(std::runtime_error("Unable to load MathMore library"));
      }
      _mathmore_loaded_ = true;
    }
    return Disk_SolidAngle(x,p);
  }

  double SLArLightPropagationModel::Omega_Dome_Model(const double distance, const double theta) const {
    // this function calculates the solid angle of a semi-sphere of radius b,
    // as a correction to the analytic formula of the on-axix solid angle,
    // as we move off-axis an angle theta. We have used 9-angular bins
    // with delta_theta width.

    // par0 = Radius correction close
    // par1 = Radius correction far
    // par2 = breaking distance betwween "close" and "far"

    double par0[9] = {0., 0., 0., 0., 0., 0.597542, 1.00872, 1.46993, 2.04221};
    double par1[9] = {0, 0, 0.19569, 0.300449, 0.555598, 0.854939, 1.39166, 2.19141, 2.57732};
    const double delta_theta = 10.;
    int j = int(theta/delta_theta);
    // 8" PMT radius
    const double b = 8*2.54/2.;
    // distance form which the model parameters break (empirical value)
    const double d_break = 5*b;//par2

    if(distance >= d_break) {
      double R_apparent_far = b - par1[j];
      return  (2*3.1416 * (1 - sqrt(1 - pow(R_apparent_far/distance,2))));

    }
    else {
      double R_apparent_close = b - par0[j];
      return (2*3.1416 * (1 - sqrt(1 - pow(R_apparent_close/distance,2))));
    }
  }

  double SLArLightPropagationModel::interpolate( 
      const std::vector<double> &xData, 
      const std::vector<double> &yData, 
      double x, bool extrapolate ) 
  {
    int size = xData.size();
    int i = 0;                  // find left end of interval for interpolation
    if ( x >= xData[size - 2] ) // special case: beyond right end
    {
      i = size - 2;
    }
    else
    {
      while ( x > xData[i+1] ) i++;
    }
    double xL = xData[i];
    double yL = yData[i];
    double xR = xData[i+1]; 
    double yR = yData[i+1]; // points on either side (unless beyond ends)
    if ( !extrapolate )     // if beyond ends of array and not extrapolating
    {
      if ( x < xL ) yR = yL;
      if ( x > xR ) yL = yR;
    }
    double dydx = ( yR - yL ) / ( xR - xL );            // gradient
    return yL + dydx * ( x - xL );                      // linear interpolation
  }


  double SLArLightPropagationModel::solid_old(SLArCfgReadoutTile* cfgTile, TVector3 &v) {
    //TODO: add normal to photon detector in cfg 
    TVector3 OpDetNorm(-1, 0, 0);     

    // TODO: Fix to make it independent of detector plane
    // detector size
    double Dx_ = cfgTile->Get2DSize_X()/G4UIcommand::ValueOf("cm"); 
    double Dy_ = cfgTile->Get2DSize_Y()/G4UIcommand::ValueOf("cm");

    // project scintillation point onto the photon detector plane
    TVector3 vv = v - OpDetNorm.Dot(v)*OpDetNorm; 

    // The hit is directly above the SiPM
    if( vv.Y()==0.0 && vv.Z()==0.0){
      return omega(Dx_,Dy_,v.Mag());
    }

    if( (std::fabs(v.Y()) > Dx_*0.5) && (std::fabs(v.Z()) > Dy_*0.5)){
      double A, B, a, b, d;
      A = std::fabs(v.Y())-Dx_*0.5;
      B = std::fabs(v.Z())-Dy_*0.5;
      a = Dx_;
      b = Dy_;
      d = fabs(v.X());
      double to_return = (
           omega(2*(A+a),2*(B+b),d) 
          -omega(2*A,2*(B+b),d)
          -omega(2*(A+a),2*B,d)
          +omega(2*A,2*B,d))*0.25;
      return to_return;
    }

    if( (std::fabs(v.Y()) <= Dx_*0.5) && (std::fabs(v.Z()) <= Dy_*0.5)){
      double A, B, a, b, d;
      A = -std::abs(v.Y())+Dx_*0.5;
      B = -std::abs(v.Z())+Dy_*0.5;
      a = Dx_;
      b = Dy_;
      d = fabs(v.X());
      double to_return = (
            omega(2*(a-A),2*(b-B),d)
           +omega(2*A,2*(b-B),d)
           +omega(2*(a-A),2*B,d)
           +omega(2*A,2*B,d))*0.25;
      return to_return;
    }

    if( (std::fabs(v.Y()) > Dx_*0.5) && (std::fabs(v.Z()) <= Dy_*0.5)){
      double A, B, a, b, d;
      A =  std::fabs(v.Y())-Dx_*0.5;
      B = -std::fabs(v.Z())+Dy_*0.5;
      a = Dx_;
      b = Dy_;
      d = fabs(v.X());
      double to_return = (
          omega(2*(A+a),2*(b-B),d)
          -omega(2*A,2*(b-B),d)
          +omega(2*(A+a),2*B,d)
          -omega(2*A,2*B,d))*0.25;
      return to_return;
    }

    if( (std::fabs(v.Y()) <= 0.5*Dx_) && (std::fabs(v.Z()) > 0.5*Dy_)){
      double A, B, a, b, d;
      A = -std::fabs(v.Y())+0.5*Dx_;
      B =  std::fabs(v.Z())-0.5*Dy_;
      a = Dx_;
      b = Dy_;
      d = fabs(v.X());
      double to_return = (
          omega(2*(a-A),2*(B+b),d)
          -omega(2*(a-A),2*B,d)
          +omega(2*A,2*(B+b),d)
          -omega(2*A,2*B,d))*0.25;
      return to_return;
    }
    // error message if none of these cases, i.e. something has gone wrong!
    std::cout << "Warning: invalid solid angle call." << std::endl;
    return 0.0;
  }


}
