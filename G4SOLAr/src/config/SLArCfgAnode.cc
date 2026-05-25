/**
 * @author      Daniele Guffanti (University and INFN Milano-Bicocca)
 * @file        SLArCfgAnode.cc
 * @created     Thursday Nov 10, 2022 16:24:26 CET
 */

#include "config/SLArCfgAnode.hh"
#include "TList.h"


ClassImp(SLArCfgAnode)

SLArCfgAnode::SLArCfgAnode() 
  : SLArCfgAssembly<SLArCfgMegaTile>(), 
  fTPCID(0), fAnodeLevelsMap(3)
{} 

SLArCfgAnode::SLArCfgAnode(const SLArCfgAssembly<SLArCfgMegaTile>& cfg) 
 : SLArCfgAssembly<SLArCfgMegaTile>(cfg), fTPCID(0), fAnodeLevelsMap(3)
{}

SLArCfgAnode::SLArCfgAnode(TString name) 
  : SLArCfgAssembly<SLArCfgMegaTile>(name), fTPCID(0), fAnodeLevelsMap(3)
{}

SLArCfgAnode::SLArCfgAnode(const SLArCfgAnode& ref) 
  : SLArCfgAssembly<SLArCfgMegaTile>(ref), fTPCID(ref.fTPCID), fAnodeLevelsMap(3)
{
  
  for (int i=0; i<3; i++) {
    if (ref.fAnodeLevelsMap.at(i) != nullptr) {
      fAnodeLevelsMap.at(i) = 
        std::unique_ptr<TH2Poly>((TH2Poly*)ref.fAnodeLevelsMap.at(i)->Clone());
    } 
  }
}

SLArCfgAnode::~SLArCfgAnode() {
  //for (auto &hmap : fAnodeLevelsMap) {delete hmap.second; hmap.second = 0;}
  fAnodeLevelsMap.clear(); 
}

SLArCfgAnode::SLArPixIdx SLArCfgAnode::GetPixelBinIndex(const double& x0, const double& x1) {
  SLArCfgAnode::SLArPixIdx pidx = {-9, -9, -9}; 

  int ibin = fAnodeLevelsMap.at(0)->FindBin(x0, x1); 

  if (ibin < 0) return pidx;
  SLArCfgMegaTile& megatile = GetBaseElement(ibin-1);
  TVector3 mt_pos = TVector3(megatile.GetPhysX(), megatile.GetPhysY(), megatile.GetPhysZ()); 
  //printf("megatile_pos: [%g, %g, %g] mm\n", 
  //mt_pos[0], mt_pos[1], mt_pos[2]); 

  pidx[0] = megatile.GetBinIdx();  
  Double_t mt_x0 = mt_pos.Dot(fAxis0); 
  Double_t mt_x1 = mt_pos.Dot(fAxis1); 
  //printf("correct for MT %s coordinates: %g, %g mm -> %g, %g \n", 
  //megatile.GetName(), mt_x0, mt_x1, x0-mt_x0, x1-mt_x1);
  ibin = fAnodeLevelsMap.at(1)->FindBin(x0-mt_x0, x1-mt_x1);
  if (ibin <= 0) return pidx;
  //SLArCfgReadoutTile& tile = megatile.GetBaseElementByBin(ibin); 
  SLArCfgReadoutTile& tile = megatile.GetBaseElement(ibin-1); 
  //if (tile) {
  pidx[1] = tile.GetBinIdx(); 
  TVector3 tile_pos(tile.GetPhysX(), tile.GetPhysY(), tile.GetPhysZ()); 
  //printf("tile_pos: [%g, %g, %g]\n", tile_pos[0], tile_pos[1], tile_pos[2]);
  Double_t t_x0 = tile_pos.Dot(fAxis0);
  Double_t t_x1 = tile_pos.Dot(fAxis1); 
  pidx[2] = fAnodeLevelsMap.at(2)->FindBin(x0-t_x0, x1-t_x1); 
  //printf("pix id %i\n", pidx[2]);
  //} 
//#ifdef SLAR_DEBUG
  //else {
    //printf("Cannot find tile with bin index %i\n", ibin);
  //}
//#endif

  //} 
//#ifdef SLAR_DEBUG
  //else {
    //printf("SLArCfgAnode::FindPixel(%g, %g)\n", x0, x1);
    //printf("Coordinates outside of anode scope\n"); 
  //}
//#endif

  return pidx; 
}

SLArCfgAnode::SLArPixIdx SLArCfgAnode::GetPixelIndex(const double& x0, const double& x1) {
  SLArCfgAnode::SLArPixIdx pidx = {-9}; 

  //printf("original coordinates: %g, %g\n", x0, x1);
  int ibin = fAnodeLevelsMap.at(0)->FindBin(x0, x1); 
  if (ibin <= 0) return pidx;

  //SLArCfgMegaTile& megatile = GetBaseElementByBin(ibin);
  SLArCfgMegaTile& megatile = GetBaseElement(ibin-1);
  //if (megatile) {
  TVector3 mt_pos = TVector3(megatile.GetPhysX(), megatile.GetPhysY(), megatile.GetPhysZ()); 
  //printf("megatile_pos: [%g, %g, %g] mm\n", mt_pos[0], mt_pos[1], mt_pos[2]); 

  pidx[0] = megatile.GetIdx();  
  Double_t mt_x0 = mt_pos.Dot(fAxis0); 
  Double_t mt_x1 = mt_pos.Dot(fAxis1); 
  Double_t local_x0 = x0 - mt_x0; 
  Double_t local_x1 = x1 - mt_x1;
  //printf("correct for MT %s coordinates: %g, %g mm -> %g, %g \n", 
  //megatile.GetName(), mt_x0, mt_x1, local_x0, local_x1);
  ibin = fAnodeLevelsMap.at(1)->FindBin(x0-mt_x0, x1-mt_x1);
  if (ibin <= 0) return pidx; 
  //SLArCfgReadoutTile& tile = megatile.GetBaseElementByBin(ibin); 
  SLArCfgReadoutTile& tile = megatile.GetBaseElement(ibin-1); 
  //if (tile) {
  pidx[1] = tile.GetIdx(); 
  TVector3 tile_pos(tile.GetPhysX(), tile.GetPhysY(), tile.GetPhysZ()); 
  //printf("tile_pos: [%g, %g, %g]\n", tile_pos[0], tile_pos[1], tile_pos[2]);
  Double_t t_x0 = tile_pos.Dot(fAxis0);
  Double_t t_x1 = tile_pos.Dot(fAxis1); 
  local_x0 = x0 - t_x0;
  local_x1 = x1 - t_x1;
  //printf("looking for bin at coordinates: %g, %g\n", local_x0, local_x1); 
  pidx[2] = fAnodeLevelsMap.at(2)->FindBin(local_x0, local_x1); 
  //} 
//#ifdef SLAR_DEBUG
  //else {
    //printf("Cannot find tile with bin index %i\n", ibin);
  //}
//#endif

  //} 
//#ifdef SLAR_DEBUG
  //else {
    //printf("SLArCfgAnode::FindPixel(%g, %g)\n", x0, x1);
    //printf("Coordinates outside of anode scope\n"); 
  //}
//#endif

  return pidx; 
}

void SLArCfgAnode::RegisterMap(size_t ilevel, TH2Poly* hmap) {
  fAnodeLevelsMap.at(ilevel) = std::unique_ptr<TH2Poly>(hmap); 
  return;
}

TH2Poly* SLArCfgAnode::ConstructPixHistMap(const int depth, 
    const std::vector<int> idx)
{
  switch (depth) {
    // Returns the map of the MegaTiles
    case 0:
      {
        return fAnodeLevelsMap.at(0).get(); 
      }
      break;
    // Returns the map at tile-level for a specfied MTile
    case 1:
      {
        SLArCfgMegaTile& cfgMegaTile  = GetBaseElement(idx[0]); 
        //if (!cfgMegaTile) {
          //return nullptr;
        //}
        return cfgMegaTile.BuildPolyBinHist(); 
      }
      break;

    // return the map at pixel level for a specified tile
    case 2:
      {
        SLArCfgMegaTile& cfgMegaTile  = GetBaseElement(idx[0]); 
        //if (!cfgMegaTile) {
          //printf("SLArCfgAnode::ConstructPixHistMap ERROR. Cannot find MT %i in configuration\n", 
              //idx[0]);
          //return nullptr; 
        //}
        SLArCfgReadoutTile& cfgTile = cfgMegaTile.GetBaseElement(idx[1]);
        TVector3 tile_pos( cfgTile.GetPhysX(), cfgTile.GetPhysY(), cfgTile.GetPhysZ() ); 
        //double tile_xpos = cfgTile->GetPhysZ(); 
        //double tile_ypos = cfgTile->GetPhysY(); 

        TString name_ = Form("hqpix_mt%i_t%i", idx[0], idx[1]); 
        TH2Poly* h2 = new TH2Poly(name_.Data(),
            Form("%s;#it{z} [mm];#it{y} [mm]", name_.Data()),
            (tile_pos-0.5*cfgTile.GetSize()).Dot(fAxis0), 
            (tile_pos+0.5*cfgTile.GetSize()).Dot(fAxis0),
            (tile_pos-0.5*cfgTile.GetSize()).Dot(fAxis1), 
            (tile_pos+0.5*cfgTile.GetSize()).Dot(fAxis1)
            ); 
        h2->SetFloat(); 

        TH2Poly* h2_template = fAnodeLevelsMap.at(2).get();
        for (const auto& bbin : *(h2_template->GetBins())) {
          TH2PolyBin* bin = (TH2PolyBin*)bbin;
          TGraph* g_tpl = (TGraph*)bin->GetPolygon();
          TGraph* g = (TGraph*)g_tpl->Clone(); 
          for (int i=0; i<g->GetN(); i++) {
            g->GetX()[i] += tile_pos.Dot(fAxis0); 
            g->GetY()[i] += tile_pos.Dot(fAxis1); 
          }
          h2->AddBin(g); 
        }
        return h2;
      }
      break;
  }

  char err_msg[100]; 
  snprintf(err_msg, sizeof(err_msg),
	  "SLArCfgAnode::ConstructPixHistMap(): ERROR while building anode map with depth level %i\n", depth);
  throw std::runtime_error(err_msg); 
}
