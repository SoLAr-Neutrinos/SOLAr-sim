/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArGeoUtils
 * @created     : Monday Feb 12, 2024 12:27:48 CET
 */

#include "detector/SLArGeoUtils.hh"
#include "G4Box.hh"

namespace slargeo {
  G4double get_bounding_volume_surface(const G4VSolid* solid) {
    if (dynamic_cast<const G4Box*>(solid)) {
      const auto box = (G4Box*)solid;
      return box->GetSurfaceArea(); 
    }
    else {
      printf("slargeo::get_bounding_volume_surface: WARNING"); 
      printf("get_bounding_volume_surface is only implemented for G4Box solids. "); 
      printf("Feel free to work on your solid's implementation and let me know!\n");
      printf("Using a box approximation.\n");

      G4ThreeVector lo; 
      G4ThreeVector hi;
      G4ThreeVector dim; 
      solid->BoundingLimits(lo, hi);

      for (int i=0; i<3; i++) dim[i] = fabs(hi[i] - lo[i]); 

      G4double half_area = dim[0]*dim[1] + dim[0]*dim[2] + dim[1]*dim[2];

      return 2*half_area; 
    }
  }
}

