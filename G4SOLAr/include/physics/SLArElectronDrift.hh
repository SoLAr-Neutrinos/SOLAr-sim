/**
 * @author      : Daniele Guffanti (University and INFN Milano-Bicocca)
 * @file        : SLArElectronDrift.hh
 * @created     : giovedì nov 10, 2022 17:42:44 CET
 */

#ifndef SLARELECTRONDRIFT_HH

#define SLARELECTRONDRIFT_HH

#include <math.h>
#include <vector>
#include "physics/LiquidArgon/SLArLArProperties.hh"
#include "G4ThreeVector.hh"
#include "G4Transform3D.hh"
#include "G4SystemOfUnits.hh"

class SLArCfgAnode;
class SLArEventAnode;

class SLArElectronDrift {
  public:
    SLArElectronDrift(const SLArLArProperties& lar_properties); 
    ~SLArElectronDrift() {} 

    inline void SetLArTargetTransform(const G4Transform3D& world2target) { fWorld2TargetTransform = world2target; }

    void Drift(const int& n, const int& trkId, const int& ancestorId,
        const G4ThreeVector& prestep_pos,
        const G4ThreeVector& poststep_pos, 
        const double& prestep_time, 
        const double& poststep_time,
        SLArCfgAnode* anodeCfg, SLArEventAnode* anodeEv);

  private: 
    const SLArLArProperties& fLArProperties;
    G4Transform3D fWorld2TargetTransform;

    inline G4ThreeVector transform_position(const G4ThreeVector& world_pos) const {
      const HepGeom::Point3D<G4double> p(world_pos.x(), world_pos.y(), world_pos.z());
      const HepGeom::Point3D<G4double> tp = fWorld2TargetTransform * p;
      return G4ThreeVector(tp.x(), tp.y(), tp.z());
    }
};


#endif /* end of include guard SLARELECDRIFT_HH */

