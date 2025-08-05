/**
 * @author      Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        SLArRandomExtra.cc
 * @created     Thursday Sep 21, 2023 18:15:34 CEST
 */

#include "SLArRandomExtra.hh"
#include <G4ThreeVector.hh>

G4ThreeVector SLArRandom::SampleRandomDirection() {
  double cosTheta = 2*G4UniformRand() - 1.;
  double sinTheta = std::sqrt(1. - cosTheta*cosTheta);

  double phi = CLHEP::twopi*G4UniformRand();
  double ux = sinTheta*std::cos(phi),
         uy = sinTheta*std::sin(phi),
         uz = cosTheta;

  G4ThreeVector dir(ux, uy, uz);
  
  return dir; 
}

G4ThreeVector SLArRandom::SampleLinearPolarization(const G4ThreeVector& momentum) {
  G4ThreeVector ref(0, 0, 1); 
  if ( fabs( fabs(momentum.z()) - 1.0 ) < 0.01 ) {
    ref.set(1, 0, 0); 
  }
  
  G4ThreeVector p0 = momentum.cross(ref); 
  p0 = p0.unit(); 

  G4ThreeVector p1 = momentum.cross(p0); 
  p1 = p1.unit(); 

  double phi = CLHEP::twopi*G4UniformRand();
  double cosPhi = std::cos(phi); 
  double sinPhi = std::sin(phi); 

  G4ThreeVector polarization = cosPhi * p0 + sinPhi * p1;
  polarization = polarization.unit();

  return polarization;
}

std::array<G4ThreeVector,2> SLArRandom::SampleRandomDirectionAndPolarization() {
  double cosTheta = 2*G4UniformRand() - 1.;
  double sinTheta = std::sqrt(1. - cosTheta*cosTheta);

  double phi = CLHEP::twopi*G4UniformRand();
  double cosPhi = std::cos(phi); 
  double sinPhi = std::sin(phi); 

  double ux = sinTheta*cosPhi,
         uy = sinTheta*sinPhi,
         uz = cosTheta;

  G4ThreeVector p_dir(ux, uy, uz);
  
  double sx = cosTheta*cosPhi, 
         sy = cosTheta*sinPhi, 
         sz = -sinTheta; 

  G4ThreeVector polarization(sx, sy, sz); 
  G4ThreeVector perp = p_dir.cross(polarization); 

  double psi = CLHEP::twopi*G4UniformRand();
  double sinPsi = std::sin(psi);
  double cosPsi = std::cos(psi);

  polarization = cosPsi * polarization + sinPsi * perp;
  polarization = polarization.unit();

  return {p_dir, polarization}; 
}

