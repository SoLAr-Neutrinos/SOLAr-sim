/**
 * @author      Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        SLArGeoUtils.hpp
 * @created     Wed Apr 19, 2023 08:08:00 CEST
 */

#ifndef SLARGEOUTILS_HH

#define SLARGEOUTILS_HH

#include <map>
#include <G4ThreeVector.hh>

class G4VSolid;

namespace slargeo {
  enum EBoxFace {kXplus=0, kXminus=1, kYplus=2, kYminus=3, kZplus=4, kZminus=5}; 
  static std::map<EBoxFace, G4ThreeVector> BoxFaceNormal  = {
    {kXplus, G4ThreeVector(-1, 0, 0)},
    {kXminus, G4ThreeVector(+1, 0, 0)},
    {kYplus, G4ThreeVector(0, -1, 0)},
    {kYminus, G4ThreeVector(0, +1, 0)},
    {kZplus, G4ThreeVector(0, 0, -1)},
    {kZminus, G4ThreeVector(0, 0, +1)}
  };

  G4double get_bounding_volume_surface(const G4VSolid* solid);
}



#endif /* end of include guard SLARGEOUTILS_HH */

