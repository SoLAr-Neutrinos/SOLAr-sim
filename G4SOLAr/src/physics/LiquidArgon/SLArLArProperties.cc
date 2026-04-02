/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArLArProperties
 * @created     : Thursday May 16, 2024 12:40:07 CEST
 */

#include "G4SystemOfUnits.hh"
#include "LiquidArgon/SLArLArProperties.hh"

SLArLArProperties::SLArLArProperties() :
  fElectricField(0.5), fLArTemperature(87.7), fMuElectron(1.), 
  fDiffCoefficientL(0.), fDiffCoefficientT(0.), 
  fvDrift(1.0), fvDriftInverse(1.0), fElectronLifetime(1e7), fElectronLifetimeInverse(1e-7)
{}

SLArLArProperties::SLArLArProperties(const SLArLArProperties& p) :
  fElectricField(p.fElectricField), fLArTemperature(p.fLArTemperature), fMuElectron(p.fMuElectron), 
  fDiffCoefficientL(p.fDiffCoefficientL), fDiffCoefficientT(p.fDiffCoefficientT), 
  fvDrift(p.fvDrift), fvDriftInverse(p.fvDriftInverse), 
  fElectronLifetime(p.fElectronLifetime), fElectronLifetimeInverse(p.fElectronLifetimeInverse)
{}

SLArLArProperties& SLArLArProperties::operator=(const SLArLArProperties& p) {
  if (this != &p) {
    fElectricField = p.fElectricField;
    fLArTemperature = p.fLArTemperature;
    fMuElectron = p.fMuElectron;
    fDiffCoefficientL = p.fDiffCoefficientL;
    fDiffCoefficientT = p.fDiffCoefficientT;
    fvDrift = p.fvDrift;
    fvDriftInverse = p.fvDriftInverse;
    fElectronLifetime = p.fElectronLifetime;
    fElectronLifetimeInverse = p.fElectronLifetimeInverse;
  }

  return *this;
}

void SLArLArProperties::ComputeProperties() {
  printf("SLArLArProperties::ComputeProperties() ");
  printf("Setup electron transport properties in LAr\n");

  fMuElectron = ComputeMobility(fElectricField, fLArTemperature);
  fvDrift     = ComputeDriftVelocity(fElectricField); 
  fvDriftInverse = 1.0/fvDrift;
  auto diff   = ComputeDiffusion(fElectricField, fLArTemperature); 
  fDiffCoefficientL = diff.at(0); 
  fDiffCoefficientT = diff.at(1); 
}

double SLArLArProperties::ComputeMobility(std::array<double, 2> par) {
  double mu = ComputeMobility(par[0], par[1]); 
  return mu;
}

double SLArLArProperties::ComputeMobility(double E, double larT) { 
  G4double num = a0 + a1*E + a2*pow(E, 1.5) + a3*pow(E, 2.5); 
  G4double den = 1 + (a1/a0)*E + a4*E*E + a5*E*E*E;

  G4double mu=  num/den*pow(larT/larT0, -1.5); 
  
  return mu;
}

double SLArLArProperties::ComputeDriftVelocity(double E) {
  G4double v = E*fMuElectron*1e3/*kV/cm->V/cm*/*1E-8/*cm/s->mm/ns*/;
  return v;
}

std::array<double,2> SLArLArProperties::ComputeDiffusion(double E, double larT) {
  G4double num = b0 + b1*E + b2*E*E; 
  G4double den = 1 + (b1/b0)*E + b3*E*E; 

  G4double epsL = (num/den)*(larT/larT1); 

  G4double diffL = epsL * fMuElectron * 1e-7; // [mm2/ns]

  std::array<double, 2> par = {E, larT}; 

  double dMudE = FastMuDerivative(par, 0.005*E, 0); 
  
  G4double diffT = diffL / (1+dMudE*E/fMuElectron); 

  return std::array<double,2>({diffL, diffT});
}

double SLArLArProperties::FastMuDerivative(
    std::array<double, 2> par, double step, int ipar) {
  
  G4double num = 0; G4double den = 12*step; 
  std::array<double, 2> par_ = par; 
  par_[ipar] = par[ipar] + 2*step; num -=   ComputeMobility(par_); 
  par_[ipar] = par[ipar] + 1*step; num += 8*ComputeMobility(par_); 
  par_[ipar] = par[ipar] - 1*step; num -= 8*ComputeMobility(par_); 
  par_[ipar] = par[ipar] - 2*step; num +=   ComputeMobility(par_); 

  return num/den;
}

void SLArLArProperties::PrintProperties() const {
  printf("**************************************************\n");
  printf("* SLArLArProperties Electron Transport Properties \n");
  printf("* - - - - - - - - - - - - - - - - - - - - - - - - \n");
  printf("* electron lifetime: %g ms\n", fElectronLifetime / CLHEP::ms); 
  printf("* electron mobility: %g cm²/s/V\n", fMuElectron); 
  printf("* drift velocity: %g cm/μs\n", fvDrift * 1e+2);
  printf("* diff coeff L: %g cm²/s\n", fDiffCoefficientL*1e+7);
  printf("* diff coeff T: %g cm²/s\n", fDiffCoefficientT*1e+7);
  printf("**************************************************\n");
  return;
}
