/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArCfgSuperCell.cc
 * @created     : martedì lug 19, 2022 10:59:27 CEST
 */

#include <iostream>
#include <string>
#include <fstream>

#include "config/SLArCfgSuperCell.hh"

ClassImp(SLArCfgSuperCell)

SLArCfgSuperCell::SLArCfgSuperCell() : SLArCfgBaseModule()
{}

SLArCfgSuperCell::SLArCfgSuperCell(int id) : SLArCfgBaseModule(id)
{}

SLArCfgSuperCell::SLArCfgSuperCell(int id, float xc, float yc, float zc, 
             float phi, float theta, float psi) 
  : SLArCfgBaseModule(id, xc, yc, zc, phi, theta, psi)
{}

SLArCfgSuperCell::~SLArCfgSuperCell() 
{
  //if (fGShape) {delete fGShape; fGShape = nullptr;}
}

TGraph SLArCfgSuperCell::BuildGShape() const
{
  TGraph g(5);
  TVector3 pos(fPhysX, fPhysY, fPhysZ); 
  TVector3 size_tmp = fSize; 
  TRotation rot; 
  rot.SetXEulerAngles( fPhi, fTheta, fPsi ); 
  TRotation rot_inv = rot.Inverse(); 
  size_tmp.Transform( rot_inv ); 
  
  g.SetPoint(0, fAxis0.Dot(pos-0.5*size_tmp), fAxis1.Dot(pos-0.5*size_tmp));
  g.SetPoint(1, fAxis0.Dot(pos-0.5*size_tmp), fAxis1.Dot(pos+0.5*size_tmp));
  g.SetPoint(2, fAxis0.Dot(pos+0.5*size_tmp), fAxis1.Dot(pos+0.5*size_tmp));
  g.SetPoint(3, fAxis0.Dot(pos+0.5*size_tmp), fAxis1.Dot(pos-0.5*size_tmp));
  g.SetPoint(4, fAxis0.Dot(pos-0.5*size_tmp), fAxis1.Dot(pos-0.5*size_tmp));

  //printf("φ: %g, θ: %g, ψ: %g\n", fPhi, fTheta, fPsi);
  //printf("size:     [%.2f, %.2f, %.2f]\n", fSize.x(), fSize.y(), fSize.z());
  //printf("size_tmp: [%.2f, %.2f, %.2f] - axis0: [%.2f, %.2f, %.2f], axis1: [%.2f, %.2f, %.2f]\n", 
      //size_tmp.x(), size_tmp.y(), size_tmp.z(), 
      //fAxis0.x(), fAxis0.y(), fAxis0.z(), 
      //fAxis1.x(), fAxis1.y(), fAxis1.z());
  //printf("gbin:"); 
  //for (int i=0; i<5; i++) {
    //printf(" - [%g, %g]", g.GetX()[i], g.GetY()[i]); 
  //}

  g.SetName(Form("gShape%i", fIdx)); 
  return g; 
}

void SLArCfgSuperCell::DumpInfo() const
{
  printf("SuperCell[%i] id: %i at (%.2f, %.2f) mm, \n", 
      fIdx, fID, fX, fY);
}


