/**
 * @author      : Daniele Guffanti (University and INFN Milano-Bicocca)
 * @file        : SLArCfgMegaTile.cc
 * @created     : Tuesday Jul 19, 2022 10:15:59 CEST
 */

#include "config/SLArCfgMegaTile.hh"

ClassImp(SLArCfgMegaTile)

SLArCfgMegaTile::SLArCfgMegaTile()
  : SLArCfgAssembly<SLArCfgReadoutTile>()
{
  SetName("aMegaTileHasNoName");    
}

SLArCfgMegaTile::SLArCfgMegaTile(TString name, int serie) 
  : SLArCfgAssembly<SLArCfgReadoutTile>(name, serie) 
{
#ifdef SLAR_DEBUG
  printf("SLArCfgMegaTile created with name %s\n", fName.Data());
#endif
}

SLArCfgMegaTile::SLArCfgMegaTile(const SLArCfgMegaTile &cfg)
  : SLArCfgAssembly<SLArCfgReadoutTile>(cfg)
{}

SLArCfgMegaTile::~SLArCfgMegaTile()
{
  //if (fH2Bins) {delete fH2Bins; fH2Bins = nullptr;}
  //for (auto &sc : fElementsMap)
    //if (sc.second) {delete sc.second; sc.second = 0;}
  fElementsMap.clear();
  fNElements = 0;
}

void SLArCfgMegaTile::DumpMap() const
{
  const size_t n_elements = fElementsMap.size(); 
  std::printf("SLArCfgMegaTile %s has %lu entries\n", 
      fName.Data(), n_elements);
  for (size_t i_element = 0; i_element < n_elements; i_element++) {
    fElementsMap.at(i_element).DumpInfo();
  }
}

/*
 *SLArCfgReadoutTile* SLArCfgMegaTile::FindPixel(double x, double y) {
 *  if (!fH2Bins) BuildPolyBinHist(); 
 *
 *  int ibin = fH2Bins->FindBin(x, y);
 *  SLArCfgReadoutTile* tile = FindBaseElementInMap(ibin); 
 *
 *  return tile; 
 *}
 */
