/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArCfgAssembly.hh
 * @created     : Tuesday Jul 19, 2022 11:12:12 CEST
 */

#ifndef SLARCFGASSEMBLY_HH

#define SLARCFGASSEMBLY_HH

#include <iostream>
#include <fstream>
#include <map>
#include <vector>

#include "TH2Poly.h"
#include <config/SLArCfgBaseModule.hh>

template<class TBaseModule>
class SLArCfgAssembly : public SLArCfgBaseModule {
  public: 
    enum ESubModuleReferenceFrame {kRelative = 0, kWorld = 1}; 
    SLArCfgAssembly(); 
    SLArCfgAssembly(TString name, int serie = 0); 
    SLArCfgAssembly(const SLArCfgAssembly& cfg); 
    virtual ~SLArCfgAssembly(); 

    virtual void DumpMap() const;
    virtual void DumpInfo() const override; 
    inline TBaseModule& GetBaseElementByBin(const int ibin) {
      auto itr = fBinToIdxMap.find(ibin); 
      if (itr == fBinToIdxMap.end()) {
        fprintf(stderr, "SLArCfgAssembly::GetBaseElementByBin ERROR: Module with Bin Index %i not found in records\n", ibin);
        exit(EXIT_FAILURE);
      }
      int module_idx = itr->second;
      return fElementsMap.at(module_idx); 
    }
    inline const TBaseModule& GettBaseElementByBin(const int ibin) const {
      auto itr = fBinToIdxMap.find(ibin); 
      if (itr == fBinToIdxMap.end()) {
        fprintf(stderr, "SLArCfgAssembly::GetBaseElementByBin ERROR: Module with Bin Index %i not found in records\n", ibin);
        exit(EXIT_FAILURE);
      }
      int module_idx = itr->second;
      return fElementsMap.at(module_idx); 
    }; 
    inline TBaseModule& GetBaseElementByID(int const id) {
      auto itr = fIDtoIdxMap.find(id); 
      if (itr == fIDtoIdxMap.end()) {
        fprintf(stderr, "SLArCfgAssembly::GetBaseElementByID ERROR: Module with ID %i not found in records\n", id);
        exit(EXIT_FAILURE);
      }
      int module_idx = itr->second;
      return fElementsMap.at(module_idx); 
    }
    inline const TBaseModule& GetBaseElementByID(const int id) const {
      auto itr = fIDtoIdxMap.find(id);
      if (itr == fIDtoIdxMap.end()) {
        fprintf(stderr, "SLArCfgAssembly::GetBaseElementByID ERROR: Module with ID %i not found in records\n", id);
        exit(EXIT_FAILURE);
      }
      const int module_idx = itr->second;
      return fElementsMap.at(module_idx); 
    }; 
    inline TBaseModule& GetBaseElement(int const idx) {
      return fElementsMap.at(idx); 
    }
    inline const TBaseModule& GetBaseElement(const int idx) const {
      return fElementsMap.at(idx); 
    };
    inline std::vector<TBaseModule>& GetMap() {return fElementsMap;}
    inline const std::vector<TBaseModule>& GetConstMap() const {return fElementsMap;}
    void RegisterElement(TBaseModule& element);
    virtual TH2Poly* BuildPolyBinHist(const ESubModuleReferenceFrame kFrame = kWorld, const bool set_bin_idx = false, const int n = 25, const int m = 25);
    TGraph BuildGShape() const override; 

  protected: 
    int fNElements; 
    std::vector<TBaseModule> fElementsMap;
    std::map<int, int> fBinToIdxMap;
    std::map<int, int> fIDtoIdxMap; 

  public:
    ClassDefOverride(SLArCfgAssembly,3);
}; 


#endif /* end of include guard SLARCFGASSEMBLY_HH */

