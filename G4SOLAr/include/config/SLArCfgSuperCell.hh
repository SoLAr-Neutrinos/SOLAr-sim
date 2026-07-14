/**
 * @author      : Daniele Guffanti (University and INFN Milano-Bicocca)
 * @file        : SLArCfgSuperCell.hh
 * @created     : Mon Feb 10, 2020 18:11:46 CET
 *
 * @brief SuperCell information container 
 */

#ifndef SLARCFGSUPERCELL_HH

#define SLARCFGSUPERCELL_HH

#include "config/SLArCfgBaseModule.hh"

class SLArCfgSuperCell : public SLArCfgBaseModule
{
  public:
    SLArCfgSuperCell();
    SLArCfgSuperCell(int idx);
    SLArCfgSuperCell(int idx, float xc, float yc, float zc, 
          float phi, float theta, float psi);
    ~SLArCfgSuperCell();

    void   DumpInfo() const override;
    TGraph BuildGShape() const override;

  protected:

  public:
    ClassDefOverride(SLArCfgSuperCell, 1);
};


#endif /* end of include guard BCPMT_HH */

