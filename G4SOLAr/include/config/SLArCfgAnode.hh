/**
 * @author      : Daniele Guffanti (University and INFN Milano-Bicocca)
 * @file        : SLArCfgAnode.hh
 * @created     : Thursday Nov 10, 2022 16:23:20 CET
 */

#ifndef SLARCFGSYSTEMPIX_HH

#define SLARCFGSYSTEMPIX_HH

#include <vector>
#include "TH2Poly.h"
#include "config/SLArCfgAssembly.hh"
#include "config/SLArCfgMegaTile.hh"

class SLArCfgAnode : public SLArCfgAssembly<SLArCfgMegaTile> {
  public: 
    typedef std::array<int, 3> SLArPixIdx; 
    SLArCfgAnode(); 
    SLArCfgAnode(const SLArCfgAssembly<SLArCfgMegaTile>& cfg); 
    SLArCfgAnode(TString name); 
    SLArCfgAnode(const SLArCfgAnode& ref);
    ~SLArCfgAnode(); 

    TH2Poly* ConstructPixHistMap(const int depth, const std::vector<int>); 
    SLArPixIdx GetPixelBinIndex(const double& x, const double& y); 
    SLArPixIdx GetPixelIndex(const double& x, const double& y); 
    void RegisterMap(size_t ilevel, TH2Poly* hmap); 
    inline TH2Poly* GetAnodeMap(size_t ilevel) {return fAnodeLevelsMap.at(ilevel).get();}
    inline int GetTPCID() const {return fTPCID;}
    inline void SetTPCID(int tpcID) {fTPCID = tpcID;}

  protected:
    std::vector<std::unique_ptr<TH2Poly>> fAnodeLevelsMap; 
    int fTPCID; 

  public:
    ClassDefOverride(SLArCfgAnode, 2); 
};





#endif /* end of include guard SLARCFGSYSTEMPIX_HH */

