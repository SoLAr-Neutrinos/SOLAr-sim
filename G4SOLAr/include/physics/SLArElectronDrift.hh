/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArElectronDrift.hh
 * @created     : giovedì nov 10, 2022 17:42:44 CET
 */

#ifndef SLARELECTRONDRIFT_HH

#define SLARELECTRONDRIFT_HH

#include <math.h>
#include <vector>
#include "physics/LiquidArgon/SLArLArProperties.hh"
#include "G4ThreeVector.hh"
#include "G4SystemOfUnits.hh"

class SLArCfgAnode;
class SLArEventAnode;

class SLArElectronDrift {
  public:
    SLArElectronDrift(const SLArLArProperties& lar_properties); 
    ~SLArElectronDrift() {} 

    void Drift(const int& n, const int& trkId, const int& ancestorId,
        const G4ThreeVector& prestep_pos,
        const G4ThreeVector& poststep_pos, 
        const double& prestep_time, 
        const double& poststep_time,
        SLArCfgAnode* anodeCfg, SLArEventAnode* anodeEv);

  private: 
    const SLArLArProperties& fLArProperties;
};


#endif /* end of include guard SLARELECDRIFT_HH */

