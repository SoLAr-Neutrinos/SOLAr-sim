/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArCfgReadoutTile.cc
 * @created     : martedì lug 19, 2022 13:40:08 CEST
 */

//#include <iostream>
//#include <string>
//#include <fstream>
//#include "TList.h"

#include <config/SLArCfgReadoutTile.hh>

ClassImp(SLArCfgReadoutTile)

SLArCfgReadoutTile::SLArCfgReadoutTile()
  : SLArCfgBaseModule(), f2DSize_X(0.), f2DSize_Y(0.)
{}

SLArCfgReadoutTile::SLArCfgReadoutTile(int id) 
  : SLArCfgBaseModule(id), f2DSize_X(0.), f2DSize_Y(0.)
{}

SLArCfgReadoutTile::SLArCfgReadoutTile(int id, float xc, float yc, float zc, 
             float phi, float theta, float psi) 
  : SLArCfgBaseModule(id, xc, yc, zc, phi, theta, psi), 
  f2DSize_X(0.), f2DSize_Y(0.)
{}

SLArCfgReadoutTile::SLArCfgReadoutTile(const SLArCfgReadoutTile& ref)
  : SLArCfgBaseModule(ref), f2DSize_X(ref.f2DSize_X), f2DSize_Y(ref.f2DSize_Y), 
  fNCellRows(ref.fNCellRows), fNCellCols(ref.fNCellCols)
{
  //fH2Pixels = nullptr; 
  //if (ref.fH2Pixels) {
    //fH2Pixels = new TH2Poly(); 
    //fH2Pixels->SetNameTitle(ref.fH2Pixels->GetName(), ref.fH2Pixels->GetTitle()); 

    //fH2Pixels->SetEntries(ref.fH2Pixels->GetEntries()); 
  //}

  //for (const auto& gb : ref.fPixelBins) {
    //int ibin = fH2Pixels->AddBin(gb.second->Clone()); 
    //fPixelBins.insert(std::make_pair(ibin, (TGraph*)gb.second->Clone())); 
  //}
}

SLArCfgReadoutTile::~SLArCfgReadoutTile()
{
  //if (fGShape) {delete fGShape; fGShape = nullptr;}
  //if (fH2Pixels) {delete fH2Pixels; fH2Pixels = nullptr;}
  //for (auto &gb : fPixelBins) {
    //if (gb.second) {delete gb.second;}
  //}
  //fPixelBins.clear(); 
}

/*
 *void SLArCfgReadoutTile::AddPixelToHistMap(TH2Poly* hmap, std::vector<xypoint> pp)
 *{
 *  // check that the first and last point defining the bin are the same 
 *  if (pp.front() != pp.back()) pp.push_back(pp.front()); 
 *
 *  TGraph g(pp.size()); 
 *  int ip = 0; 
 *  for (const auto &p : pp) {
 *    //printf("pix[0] = %g, pix[1] = %g\n", p[0], p[1]);
 *    g.SetPoint(ip, p[0], p[1]); 
 *    ip++; 
 *  }
 *
 *  if (hmap) {
 *    hmap = new TH2Poly(Form("Tile%i_pixmap", fIdx), 
 *        Form("Tile %i pixel map", fIdx), -7000, +7000, -3000, 3000);
 *    hmap->SetFloat(); 
 *    hmap->ChangePartition(50, 50); 
 *  }
 *
 *
 *  int nbin = hmap->GetNumberOfBins(); 
 *  int ibin = hmap->AddBin((TGraph*)g.Clone(Form("pix_%i", nbin))); 
 *
 *  fPixelBins.insert( std::make_pair(ibin, (TGraph*)g.Clone(Form("pix_%i", nbin))) ); 
 *}
 */

/*
 *int SLArCfgReadoutTile::FindPixel(double x, double y) {
 *  int ibin = fH2Pixels->FindBin(x, y); 
 *  return ibin; 
 *}
 */

TGraph SLArCfgReadoutTile::BuildGShape() const
{
  TGraph g(5);
  TVector3 pos(fPhysX, fPhysY, fPhysZ); 
  TVector3 size_tmp(fSize); 
  TRotation rot; 
  rot.SetXPhi( fPhi ); rot.SetXTheta( fTheta ); rot.SetXPsi( fPsi ); 
  TRotation rot_inv = rot.Inverse(); 
  size_tmp.Transform( rot_inv ); 
  
  g.SetPoint(0, fAxis0.Dot(pos-0.5*size_tmp), fAxis1.Dot(pos-0.5*size_tmp));
  g.SetPoint(1, fAxis0.Dot(pos-0.5*size_tmp), fAxis1.Dot(pos+0.5*size_tmp));
  g.SetPoint(2, fAxis0.Dot(pos+0.5*size_tmp), fAxis1.Dot(pos+0.5*size_tmp));
  g.SetPoint(3, fAxis0.Dot(pos+0.5*size_tmp), fAxis1.Dot(pos-0.5*size_tmp));
  g.SetPoint(4, fAxis0.Dot(pos-0.5*size_tmp), fAxis1.Dot(pos-0.5*size_tmp));

  g.SetName(Form("gShape%i", fIdx)); 

  return g;
}

void SLArCfgReadoutTile::DumpInfo() const
{
  printf("ReadoutTile[%i] id: %i at (%.2f, %.2f) mm, \n", 
      fIdx, fID, fX, fY);
}


