/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArCfgBaseSystem.cc
 * @created     : martedì lug 19, 2022 13:05:36 CEST
 */

#include <stdexcept>
#include <map>
#include "TObjString.h"
#include "TObjArray.h"
#include "TPRegexp.h"
#include "TRegexp.h"
#include "TH2Poly.h"
#include "TList.h"

#include "config/SLArCfgSuperCellArray.hh"
#include "config/SLArCfgMegaTile.hh"
#include "config/SLArCfgBaseSystem.hh"

templateClassImp(SLArCfgBaseSystem)


template<class TAssemblyModule>
SLArCfgBaseSystem<TAssemblyModule>::SLArCfgBaseSystem() 
  : SLArCfgBaseModule()//, fH2Bins(nullptr)
{
  fName = "aMapHasNoName"; 
}

template<class TAssemblyModule>
SLArCfgBaseSystem<TAssemblyModule>::SLArCfgBaseSystem(TString name) 
  : SLArCfgBaseModule()//, fH2Bins(nullptr)
{
  fName = name; 
}

template<class TAssemblyModule>
SLArCfgBaseSystem<TAssemblyModule>::SLArCfgBaseSystem(const SLArCfgBaseSystem<TAssemblyModule>& cfg) 
  : SLArCfgBaseModule(cfg) 
{
  fNElements = cfg.fNElements; 
  for (const auto &mod : cfg.fElementsMap) {
    fElementsMap.insert(
        std::make_pair(mod.first, TAssemblyModule(mod.second))); 
  }

  //fH2Bins = nullptr; 
  //if (cfg.fH2Bins) {
    //fH2Bins = new TH2Poly(); 

    //TList* lbin = cfg.fH2Bins->GetBins(); 
    //for (const auto& bbin : *lbin) {
      //TH2PolyBin* bin = (TH2PolyBin*)bbin; 
      //fH2Bins->AddBin(bin->GetPolygon()->Clone()); 
    //}
  //}
  
  return; 
}

template<class TAssemblyModule>
SLArCfgBaseSystem<TAssemblyModule>::~SLArCfgBaseSystem() {
  for (auto &mod : fElementsMap) {
    //if (mod.second) {delete mod.second; mod.second = 0;}
  }
  fElementsMap.clear(); 
  //if (fH2Bins) {delete fH2Bins;}
}

template<class TAssemblyModule>
TAssemblyModule& SLArCfgBaseSystem<TAssemblyModule>::GetBaseElement(const char* name)
{
  for (auto& mod : fElementsMap) {
    if (strcmp(mod.second.GetName(),name) == 0) {
      return mod.second;
    }
  }
  
  char err_msg[100]; 
  sprintf(err_msg, 
      "SLArCfgBaseSystem::GetBaseElement() ERROR: Element '%s' not found in register\n\n", 
      name);
  throw std::runtime_error(err_msg); 
}

template<class TAssemblyModule>
TAssemblyModule& SLArCfgBaseSystem<TAssemblyModule>::GetBaseElement(int idx)
{
  return fElementsMap.find(idx)->second;
}


template<class TAssemblyModule>
void SLArCfgBaseSystem<TAssemblyModule>::RegisterElement(TAssemblyModule& mod) 
{
  int idx = mod.GetIdx();
  if (fElementsMap.count(idx)) 
  {
    std::cout<<"Base element "<<idx<<" already registered. Skip"<<std::endl;
    return;
  }

  printf("SLArCfgBaseSystem::RegisterElement(%s)...\n", mod.GetName());
  fElementsMap.insert( std::make_pair(idx, std::move(mod)) );
  printf("DONE\n");
  fNElements++; 
}
/*
 *
 *template<class TAssemblyModule>
 *TH2Poly* SLArCfgBaseSystem<TAssemblyModule>::BuildPolyBinHist()
 *{
 *  TH2Poly* h2Bins = new TH2Poly();
 *  h2Bins->SetName(fName+"_bins");
 *
 *  h2Bins->SetFloat();
 *
 *  int iBin = 1;
 *  for (auto &mod : fElementsMap) 
 *  {
 *    //if (!mod.second->GetGraphShape()) {
 *      //mod.second->BuildGShape(); 
 *    //}
 *    auto g = mod.second.BuildGShape(); 
 *    TString gBinName = Form("gBin%i", iBin);
 *    g.SetName( gBinName ); 
 *    printf("SLArCfgBaseSystem::BuildPolyBinHist: Adding bin %i\n", iBin);
 *    int bin_idx = h2Bins->AddBin( std::move(new TGraph(g)) );
 *    mod.second.SetBinIdx(bin_idx);
 *    iBin ++;
 *  }
 *
 *  return h2Bins;
 *}
 */

template<class TAssemblyModule>
void SLArCfgBaseSystem<TAssemblyModule>::DumpMap() const
{
  std::printf("SLArCfgSystem %s has %lu entries\n", 
      fName.Data(), fElementsMap.size());
  for (auto &itr : fElementsMap)
    itr.second.DumpInfo();
}

template<class TAssemblyModule>
void SLArCfgBaseSystem<TAssemblyModule>::DumpInfo() const 
{
  DumpMap(); 
}


template<class TAssemblyModule>
TAssemblyModule& SLArCfgBaseSystem<TAssemblyModule>::FindBaseElementInMap(int ibin) 
{
  for (auto& mod : fElementsMap) {
    if (mod.second.GetBinIdx() == ibin) {
      return mod.second; 
    }
  }

  char err_msg[100]; 
  sprintf(err_msg, "SLArCfgBaseSystem::FindBaseElementInMap() ERROR: Element with bin index %i not found in register\n\n", ibin);
  throw std::runtime_error(err_msg); 
}

//template<class TAssemblyModule>
//void SLArCfgBaseSystem<TAssemblyModule>::ResetH2Hits() {
  //for (auto &mod : fElementsMap) {
    //if (mod.second) mod.second->ResetH2Hits(); 
  //}
//}

template<class TAssemblyModule> 
TGraph SLArCfgBaseSystem<TAssemblyModule>::BuildGShape() const {
  double x_min =  1e10;
  double x_max = -1e10; 
  double y_min =  1e10; 
  double y_max = -1e10; 

  for (const auto &el : fElementsMap) {
    TGraph gbin = el.second.BuildGShape(); 
    double* x = gbin.GetX(); 
    double* y = gbin.GetY(); 
    int n = gbin.GetN(); 
    double x_min_bin = *std::min_element(x, x+n);
    double x_max_bin = *std::max_element(x, x+n); 
    double y_min_bin = *std::min_element(y, y+n); 
    double y_max_bin = *std::max_element(y, y+n); 

    if (x_min_bin < x_min) x_min = x_min_bin; 
    if (x_max_bin > x_max) x_max = x_max_bin; 
    if (y_min_bin < y_min) y_min = y_min_bin; 
    if (y_max_bin > y_max) y_max = y_max_bin;
  }

  TGraph g(5); 
  g.SetPoint(0, x_min, y_min); 
  g.SetPoint(1, x_min, y_max); 
  g.SetPoint(2, x_max, y_max); 
  g.SetPoint(3, x_max, y_min); 
  g.SetPoint(4, x_min, y_min); 

  g.SetName(Form("g%s", fName.Data())); 
  return g;
}


template class SLArCfgBaseSystem<SLArCfgSuperCellArray>;

