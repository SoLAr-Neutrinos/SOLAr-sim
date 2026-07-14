/**
 * @author      : Daniele Guffanti (University and INFN Milano-Bicocca)
 * @file        : SLArCfgMegaTile.hh
 * @created     : martedì lug 19, 2022 10:16:31 CEST
 */

#ifndef SLARCFGMEGATILE_HH

#define SLARCFGMEGATILE_HH

#include "config/SLArCfgReadoutTile.hh"
#include "config/SLArCfgAssembly.hh"

class SLArCfgMegaTile : public SLArCfgAssembly<SLArCfgReadoutTile> {
  public: 
    SLArCfgMegaTile(); 
    SLArCfgMegaTile(TString name, int serie = 0); 
    SLArCfgMegaTile(const SLArCfgMegaTile& cfg); 
    ~SLArCfgMegaTile(); 

    //TODO: Reimplement in CfgAnode
    //SLArCfgReadoutTile* FindPixel(double, double); 

    void DumpMap() const override; 

  private: 

  public:
    ClassDefOverride(SLArCfgMegaTile, 1)
};


#endif /* end of include guard SLARCFGMEGATILE_HH */

