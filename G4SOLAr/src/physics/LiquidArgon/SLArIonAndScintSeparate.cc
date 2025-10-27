/**
 * @author      Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        SLArIonAndScintSeparate.cc
 * @created     mercoledì mar 01, 2023 10:55:35 CET
 */

#include <math.h>
#include "TString.h"
#include "SLArDebugManager.hh"
#include "G4MaterialPropertiesTable.hh"

#include "LiquidArgon/SLArIonAndScintSeparate.h"

SLArIonAndScintSeparate::SLArIonAndScintSeparate() : 
  SLArIonAndScintModel(), 
  fModBoxA(0.930), fModBoxB(0.212), fUseModBoxRecomb(true), fLightYield(0) 
{}

SLArIonAndScintSeparate::SLArIonAndScintSeparate(const G4MaterialPropertiesTable* mpt) :
  SLArIonAndScintModel(mpt), 
  fModBoxA( mpt->GetConstProperty("SPRT_MODBOXA") ), 
  fModBoxB( mpt->GetConstProperty("SPRT_MODBOXB") ), 
  fUseModBoxRecomb( static_cast<bool>( mpt->GetConstProperty("SPRT_USEMODBOX") ) )
{}

Ion_and_Scint_t SLArIonAndScintSeparate::ComputeIonAndScintYield(double& dEdx, const double& electric_field) const {
  double recomb = 0.;

  // Guard against spurious values of dE/dx. Note: assumes density of LAr
  if (dEdx < 1.) { dEdx = 1.; }

  if (fUseModBoxRecomb) {
      double const scaled_modboxb = fModBoxB / fLArDensity;
      double const Xi = scaled_modboxb * dEdx / electric_field;
      recomb = log(fModBoxA + Xi) / Xi;
  }
  else {
    double const scaled_recombk = fBirksK / fLArDensity;
    recomb = fBirksA / (1. + dEdx * scaled_recombk / electric_field);
  }

  // 1.e-3 converts fEnergyDeposit to GeV
  auto const numIonElectrons =  recomb / fWion;

  DEBUG_MSG_FUNC(SLArDebugManager::LARPHYSICS, 
      TString::Format(
        "SLArIonAndScintSeparate: Electrons produced for %g MeV/mm deposited with %g recombination: %g\n",
        dEdx, recomb, numIonElectrons));

  return Ion_and_Scint_t(numIonElectrons, fLightYield);
}

Ion_and_Scint_t SLArIonAndScintSeparate::ComputeIonAndScintYield(const double& energy_deposit, const double& step_length, const double& electric_field) const {
  float ds = step_length;

  double recomb = 0.;
  double dEdx = (ds <= 0.0) ? 0.0 : energy_deposit / ds;

  // Guard against spurious values of dE/dx. Note: assumes density of LAr
  if (dEdx < 1.) { dEdx = 1.; }

  if (fUseModBoxRecomb) {
    if (ds > 0) {
      double const scaled_modboxb = fModBoxB / fLArDensity;
      double const Xi = scaled_modboxb * dEdx / electric_field;
      recomb = log(fModBoxA + Xi) / Xi;
    }
    else {
      recomb = 0;
    }
  }
  else {
    double const scaled_recombk = fBirksK / fLArDensity;
    recomb = fBirksA / (1. + dEdx * scaled_recombk / electric_field);
  }

  // 1.e-3 converts fEnergyDeposit to GeV
  auto const numIonElectrons =  recomb / fWion;

  DEBUG_MSG_FUNC(SLArDebugManager::LARPHYSICS, 
      TString::Format(
        "SLArIonAndScintSeparate: Electrons produced for %g MeV deposited with %g recombination: %g\n",
        energy_deposit, recomb, numIonElectrons)); 

  return Ion_and_Scint_t(numIonElectrons, fLightYield);
}

Ion_and_Scint_t SLArIonAndScintSeparate::ComputeIonAndScint(const double& energy_deposit, const double& step_length, const double& electric_field) const {
  Ion_and_Scint_t yield = ComputeIonAndScintYield(energy_deposit, step_length, electric_field); 
  yield.ion *= energy_deposit;
  yield.scint *= energy_deposit;
  return yield; 
}

