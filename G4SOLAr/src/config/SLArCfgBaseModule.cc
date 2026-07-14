/**
 * @author      : Daniele Guffanti (University and INFN Milano-Bicocca)
 * @file        : SLArCfgBaseModule.cc
 * @created     : martedì lug 19, 2022 10:54:11 CEST
 */

#include "config/SLArCfgBaseModule.hh"

ClassImp(SLArCfgBaseModule)

SLArCfgBaseModule::SLArCfgBaseModule() : TNamed(), 
  fID(-1), fIdx(-1), fBin(0), fX(0.), fY(0.), fZ(0.), fPhi(0.), fTheta(0.), fPsi(0.), 
  fPhysX(0.), fPhysY(0.), fPhysZ(0.), 
  fNormal(0, 0, 1), fAxis0(0, 0, 0), fAxis1(0, 0, 0), fSize(0, 0, 0),
  fVecAxis0{0, 0, 0}, fVecAxis1{0, 0, 0}, fVecNormal{0, 0, 0}
{}

SLArCfgBaseModule::SLArCfgBaseModule(int id) : TNamed(),
  fID(id), fIdx(-1), fBin(0), fX(0.), fY(0.), fZ(0.), fPhi(0.), fTheta(0.), fPsi(0.), 
  fPhysX(0.), fPhysY(0.), fPhysZ(0.), 
  fNormal(0, 0, 1), fAxis0(0, 0, 0), fAxis1(0, 0, 0), fSize(0, 0, 0),
  fVecAxis0{0, 0, 0}, fVecAxis1{0, 0, 0}, fVecNormal{0, 0, 0}
{}

SLArCfgBaseModule::SLArCfgBaseModule(int id, float xc, float yc, float zc, 
             float phi, float theta, float psi) : TNamed(), 
  fID(id), fIdx(-1), fBin(0), fX(xc), fY(yc), fZ(zc), fPhi(phi), fTheta(theta), fPsi(psi), 
  fPhysX(0.), fPhysY(0.), fPhysZ(0.), 
  fNormal(0, 0, 1), fAxis0(0, 0, 0), fAxis1(0, 0, 0), fSize(0, 0, 0),
  fVecAxis0{0, 0, 0}, fVecAxis1{0, 0, 0}, fVecNormal{0, 0, 0}
{}

SLArCfgBaseModule::SLArCfgBaseModule(const SLArCfgBaseModule& base) : 
  TNamed(base)
{
  fID = base.fID;
  fIdx = base.fIdx;
  fBin = base.fBin;
  fX = base.fX;
  fY = base.fY;
  fZ = base.fZ;
  fPhi = base.fPhi;
  fTheta = base.fTheta;
  fPsi = base.fPsi;
  fPhysX = base.fPhysX;
  fPhysY = base.fPhysY;
  fPhysZ = base.fPhysZ;
  fNormal = base.fNormal;
  fAxis0 = base.fAxis0; 
  fAxis1 = base.fAxis1; 
  fSize = base.fSize;

  fVecAxis0 = base.fVecAxis0;
  fVecAxis1 = base.fVecAxis1; 
  fVecNormal= base.fVecNormal;
  //if (base.fGShape) {
  //fGShape = (TGraph*)base.fGShape->Clone();
  //}
}

SLArCfgBaseModule::~SLArCfgBaseModule()
{
  //if (fGShape) delete fGShape;
}

void SLArCfgBaseModule::SetupAxes() {
  if (fNormal.Mag2() < 1e-6) {
    printf("SLArCfgBaseModule::SetupAxes: ERROR: normal vector is null\n");
    exit(EXIT_FAILURE);
  }

  if (fNormal == TVector3(1, 0, 0) || fNormal == TVector3(-1, 0, 0)) {
    fAxis0.SetXYZ(0, 0, 1);  
  } 
  else if (fNormal == TVector3(0, 1, 0) || fNormal == TVector3(0, -1, 0)) {
    fAxis0.SetXYZ(0, 0, 1);  
  } 
  else if (fNormal == TVector3(0, 0, 1) || fNormal == TVector3(0, 0, -1)) {
    fAxis0.SetXYZ(1, 0, 0);  
  }
  fAxis1 = fAxis0.Cross( fNormal ); 
  for (int i=0; i<3; i++) fAxis1[i] = std::fabs( fAxis1[i] ); 

  for (int k=0; k<3; k++) {
    fVecNormal[k] = fNormal[k]; 
    fVecAxis0 [k] = fAxis0 [k];
    fVecAxis1 [k] = fAxis1 [k]; 
  }
 
  return;
}

