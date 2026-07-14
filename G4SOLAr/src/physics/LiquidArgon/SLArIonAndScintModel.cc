/**
 * @author      Daniele Guffanti (University and INFN Milano-Bicocca)
 * @file        SLArIonAndScintModel.cc
 * @created     Wed Mar 01, 2023 11:41:24 CET
 */

#include "LiquidArgon/SLArIonAndScintModel.h"
#include "G4MaterialPropertiesTable.hh"
#include "G4SystemOfUnits.hh"

SLArIonAndScintModel::SLArIonAndScintModel() :
  fWion(23.6e-6), fBirksK(0.0497), fBirksA(0.808), fLArDensity(1.39)
{
  fIonizationDensity = 1.0 / fWion;
}

SLArIonAndScintModel::SLArIonAndScintModel(const G4MaterialPropertiesTable* mpt) :
  fWion(mpt->GetConstProperty("WION")), 
  fBirksK(mpt->GetConstProperty("BIRKSRECOMBK")),
  fBirksA(mpt->GetConstProperty("BIRKSRECOMBA")), 
  fLArDensity(mpt->GetConstProperty("DENSITY")/( CLHEP::g / CLHEP::cm3 ))
{
  fIonizationDensity = 1.0 / fWion;   
}

Ion_and_Scint_t SLArIonAndScintModel::ComputeIonAndScint(const double& energy_deposit, const double& step_length, const double& electric_field) const {
  double dEdx = energy_deposit / step_length;
  Ion_and_Scint_t yield = ComputeIonAndScintYield(dEdx, electric_field); 
  yield.ion *= energy_deposit;
  yield.scint *= energy_deposit;
  return yield; 
}

