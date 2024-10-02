/**
 * @author      Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        SLArRandomExtra
 * @created     Thursday Sep 21, 2023 18:15:34 CEST
 */

#include "SLArRandomExtra.hh"

G4ThreeVector SampleRandomDirection() {
  double cosTheta = 2*G4UniformRand() - 1.;
  double phi = CLHEP::twopi*G4UniformRand();
  double sinTheta = std::sqrt(1. - cosTheta*cosTheta);
  double ux = sinTheta*std::cos(phi),
         uy = sinTheta*std::sin(phi),
         uz = cosTheta;

  G4ThreeVector dir(ux, uy, uz);
  
  return dir; 
}

