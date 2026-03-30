/**
 * @author      : guff (guff@guff-gssi)
 * @file        : BCCfgArrayPMT
 * @created     : giovedì giu 04, 2020 11:47:56 CEST
 */

#include "config/SLArCfgSuperCellArray.hh"

ClassImp(SLArCfgSuperCellArray)

SLArCfgSuperCellArray::SLArCfgSuperCellArray() 
  : SLArCfgAssembly<SLArCfgSuperCell>()
{
  SetName("aPMTArrayHasNoName");
}

SLArCfgSuperCellArray::SLArCfgSuperCellArray(TString name, int serie) 
  : SLArCfgAssembly<SLArCfgSuperCell>(name, serie) 
{
  SetName(name);
  printf("SLArCfgSuperCellArray created with name %s\n", fName.Data());
}


SLArCfgSuperCellArray::SLArCfgSuperCellArray(const SLArCfgSuperCellArray &cfg)
  : SLArCfgAssembly<SLArCfgSuperCell>(cfg), fTPCID(cfg.fTPCID)
{}

SLArCfgSuperCellArray::~SLArCfgSuperCellArray()
{
  //if (fH2Bins) {delete fH2Bins; fH2Bins = nullptr;}
  //for (auto &sc : fElementsMap)
    //if (sc.second) {delete sc.second; sc.second = 0;}
  fElementsMap.clear();
  fNElements = 0;
}

void SLArCfgSuperCellArray::DumpMap() const
{
  const size_t n_elements = fElementsMap.size();
  std::printf("SLArCfgSuperCellArray %s has %lu entries\n", 
      fName.Data(), n_elements);
  for (size_t i_element = 0; i_element < n_elements; i_element++) {
    const SLArCfgSuperCell& el = fElementsMap.at(i_element); 
    printf("SuperCell[%i]: id: %i, %s\n", el.GetIdx(), el.GetID(), el.GetName());
  }
}


