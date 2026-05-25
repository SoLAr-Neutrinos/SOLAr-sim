/**
 * @author      : Daniele Guffanti (University and INFN Milano-Bicocca)
 * @file        : SLArCfgSuperCellArray.hh
 * @created     : Thur Nov 10, 2022 12:53:23 CET
 */

#ifndef SLARCFGSUPERCELLARRAY_HH

#define SLARCFGSUPERCELLARRAY_HH

#include "config/SLArCfgAssembly.hh"
#include "config/SLArCfgSuperCell.hh"

class SLArCfgSuperCellArray : public SLArCfgAssembly<SLArCfgSuperCell> {
  public:
    SLArCfgSuperCellArray();
    SLArCfgSuperCellArray(TString name, int serie = 0);
    SLArCfgSuperCellArray(const SLArCfgSuperCellArray &cfg);
    ~SLArCfgSuperCellArray() override;

    void DumpMap(); 

    ClassDefOverride(SLArCfgSuperCellArray, 3);
};

#endif /* end of include guard SLARCFGSUPERCELLARRAY_HH */

