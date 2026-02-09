/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArFastLightSim.cc
 * @created     : Friday Feb 06, 2026 09:29:16 CET
 */

#include "G4ParticleDefinition.hh"
#include "G4ParticleTypes.hh"
#include "G4RandomTools.hh"
#include "G4RunManager.hh"

#include "SLArFastLightSim.hh"
#include "SLArScintillation.hh"

void SLArFastLightSim::GetScintillationYieldByParticleType(
  const G4ParticleDefinition* pDef, G4double& yield1,
  G4double& yield2, G4double& yield3) const
{
  // new in 10.7, allow multiple time constants with ScintByParticleType
  // Get the G4MaterialPropertyVector containing the scintillation
  // yield as a function of the energy deposited and particle type

  G4MaterialPropertyVector* yieldVector = nullptr;
  G4MaterialPropertiesTable* MPT = fMaterial->GetMaterialPropertiesTable();

  // Protons
  if(pDef == G4Proton::ProtonDefinition())
  {
    yield1      = MPT->ConstPropertyExists(kPROTONSCINTILLATIONYIELD1)
               ? MPT->GetConstProperty(kPROTONSCINTILLATIONYIELD1)
               : 1.;
    yield2 = MPT->ConstPropertyExists(kPROTONSCINTILLATIONYIELD2)
               ? MPT->GetConstProperty(kPROTONSCINTILLATIONYIELD2)
               : 0.;
    yield3 = MPT->ConstPropertyExists(kPROTONSCINTILLATIONYIELD3)
               ? MPT->GetConstProperty(kPROTONSCINTILLATIONYIELD3)
               : 0.;
  }

  // Deuterons
  else if(pDef == G4Deuteron::DeuteronDefinition())
  {
    yield1      = MPT->ConstPropertyExists(kDEUTERONSCINTILLATIONYIELD1)
               ? MPT->GetConstProperty(kDEUTERONSCINTILLATIONYIELD1)
               : 1.;
    yield2 = MPT->ConstPropertyExists(kDEUTERONSCINTILLATIONYIELD2)
               ? MPT->GetConstProperty(kDEUTERONSCINTILLATIONYIELD2)
               : 0.;
    yield3 = MPT->ConstPropertyExists(kDEUTERONSCINTILLATIONYIELD3)
               ? MPT->GetConstProperty(kDEUTERONSCINTILLATIONYIELD3)
               : 0.;
  }

  // Tritons
  else if(pDef == G4Triton::TritonDefinition())
  {
    yield1      = MPT->ConstPropertyExists(kTRITONSCINTILLATIONYIELD1)
               ? MPT->GetConstProperty(kTRITONSCINTILLATIONYIELD1)
               : 1.;
    yield2 = MPT->ConstPropertyExists(kTRITONSCINTILLATIONYIELD2)
               ? MPT->GetConstProperty(kTRITONSCINTILLATIONYIELD2)
               : 0.;
    yield3 = MPT->ConstPropertyExists(kTRITONSCINTILLATIONYIELD3)
               ? MPT->GetConstProperty(kTRITONSCINTILLATIONYIELD3)
               : 0.;
  }

  // Alphas
  else if(pDef == G4Alpha::AlphaDefinition())
  {
    yield1      = MPT->ConstPropertyExists(kALPHASCINTILLATIONYIELD1)
               ? MPT->GetConstProperty(kALPHASCINTILLATIONYIELD1)
               : 1.;
    yield2 = MPT->ConstPropertyExists(kALPHASCINTILLATIONYIELD2)
               ? MPT->GetConstProperty(kALPHASCINTILLATIONYIELD2)
               : 0.;
    yield3 = MPT->ConstPropertyExists(kALPHASCINTILLATIONYIELD3)
               ? MPT->GetConstProperty(kALPHASCINTILLATIONYIELD3)
               : 0.;
  }

  // Ions (particles derived from G4VIon and G4Ions) and recoil ions
  // below the production cut from neutrons after hElastic
  else if(pDef->GetParticleType() == "nucleus" ||
          pDef == G4Neutron::NeutronDefinition())
  {
    yield1      = MPT->ConstPropertyExists(kIONSCINTILLATIONYIELD1)
               ? MPT->GetConstProperty(kIONSCINTILLATIONYIELD1)
               : 1.;
    yield2 = MPT->ConstPropertyExists(kIONSCINTILLATIONYIELD2)
               ? MPT->GetConstProperty(kIONSCINTILLATIONYIELD2)
               : 0.;
    yield3 = MPT->ConstPropertyExists(kIONSCINTILLATIONYIELD3)
               ? MPT->GetConstProperty(kIONSCINTILLATIONYIELD3)
               : 0.;
  }

  // Electrons (must also account for shell-binding energy
  // attributed to gamma from standard photoelectric effect)
  // and, default for particles not enumerated/listed above
  else
  {
    yield1      = MPT->ConstPropertyExists(kELECTRONSCINTILLATIONYIELD1)
               ? MPT->GetConstProperty(kELECTRONSCINTILLATIONYIELD1)
               : 1.;
    yield2 = MPT->ConstPropertyExists(kELECTRONSCINTILLATIONYIELD2)
               ? MPT->GetConstProperty(kELECTRONSCINTILLATIONYIELD2)
               : 0.;
    yield3 = MPT->ConstPropertyExists(kELECTRONSCINTILLATIONYIELD3)
               ? MPT->GetConstProperty(kELECTRONSCINTILLATIONYIELD3)
               : 0.;
  }

  return;
}

G4double SLArFastLightSim::SamplePhotonEmissionTime(const G4ParticleDefinition* pDef) const 
{
  G4MaterialPropertiesTable* MPT = fMaterial->GetMaterialPropertiesTable();
  if(!MPT) return 0.0;

  G4double scintCompYield[3] = {0.0, 0.0, 0.0};;
  G4double scintTimeConstants[3] = {0.0, 0.0, 0.0};
  G4double riseTime[3] = {0.0, 0.0, 0.0};

  G4int N_timeconstants = 1;

  if(MPT->GetProperty(kSCINTILLATIONCOMPONENT3))
    N_timeconstants = 3;
  else if(MPT->GetProperty(kSCINTILLATIONCOMPONENT2))
    N_timeconstants = 2;
  else if(!(MPT->GetProperty(kSCINTILLATIONCOMPONENT1)))
  {
    return 0.0;
  }

  G4double sum_yields = 0.;
  scintTimeConstants[0] = MPT->GetConstProperty(kSCINTILLATIONTIMECONSTANT1);
  if (MPT->ConstPropertyExists(kSCINTILLATIONRISETIME1)) {
    riseTime[0] = MPT->GetConstProperty(kSCINTILLATIONRISETIME1);
  }
  if (N_timeconstants >= 2) {
    scintTimeConstants[1] = MPT->GetConstProperty(kSCINTILLATIONTIMECONSTANT2);
    if (MPT->ConstPropertyExists(kSCINTILLATIONRISETIME2)) {
      riseTime[1] = MPT->GetConstProperty(kSCINTILLATIONRISETIME2);
    }
  }
  if (N_timeconstants == 3) {
    scintTimeConstants[2] = MPT->GetConstProperty(kSCINTILLATIONTIMECONSTANT3);
    if (MPT->ConstPropertyExists(kSCINTILLATIONRISETIME3)) {
      riseTime[2] = MPT->GetConstProperty(kSCINTILLATIONRISETIME3);
    }
  }

  GetScintillationYieldByParticleType( pDef, scintCompYield[0], scintCompYield[1], scintCompYield[2]);
  sum_yields = scintCompYield[0] + scintCompYield[1] + scintCompYield[2];
   
  // Sample time constant based on yield fractions
  G4double rand = G4UniformRand() * sum_yields;
  if (rand < scintCompYield[0]) {
    return sample_time(riseTime[0], scintTimeConstants[0]);
  } else if (rand < (scintCompYield[0] + scintCompYield[1])) {
    return sample_time(riseTime[1], scintTimeConstants[1]);
  } else {
    return sample_time(riseTime[2], scintTimeConstants[2]);
  }
}


G4double SLArFastLightSim::sample_time(G4double riseTime, G4double fallTime) const
{
  // Loop checking, 07-Aug-2015, Vladimir Ivanchenko
  while(true)
  {
    G4double ran1 = G4UniformRand();
    G4double ran2 = G4UniformRand();

    // exponential distribution as envelope function: very efficient
    G4double d = (riseTime + fallTime) / fallTime;
    // make sure the envelope function is
    // always larger than the bi-exponential
    G4double t  = -1.0 * fallTime * std::log(1. - ran1);
    G4double gg = d * single_exp(t, fallTime);
    if(ran2 <= bi_exp(t, riseTime, fallTime) / gg)
      return t;
  }
  return -1.0;
}
