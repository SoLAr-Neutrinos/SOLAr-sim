//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
//
/// \file src/SLArPhysicsList.cc
/// \brief Implementation of the SLArPhysicsList class
///
/// Reimplemented from wls G4 example

#include "physics/SLArPhysicsList.hh"
#include "physics/SLArPhysicsListMessenger.hh"

#include "physics/SLArExtraPhysics.hh"
#include "physics/SLArOpticalPhysics.hh"

#include "G4LossTableManager.hh"

#include "G4ProcessManager.hh"
#include "G4ParticleTypes.hh"
#include "G4ParticleTable.hh"

#include "G4PhysListFactory.hh"
#include "G4EmLivermorePhysics.hh"

#include "G4Gamma.hh"
#include "G4Electron.hh"
#include "G4Positron.hh"

#include "physics/SLArStepMax.hh"

#include "G4ProcessTable.hh"

#include "G4PionDecayMakeSpin.hh"
#include "G4DecayWithSpin.hh"

#include "G4DecayTable.hh"
#include "G4MuonDecayChannelWithSpin.hh"
#include "G4MuonRadiativeDecayChannelWithSpin.hh"

#include "G4RadioactiveDecayPhysics.hh"

#include "G4UnitsTable.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

SLArPhysicsList::SLArPhysicsList(G4String physName, G4bool do_cerenkov) : 
  G4VModularPhysicsList()
{
  G4LossTableManager::Instance();

  defaultCutValue  = 0.1*CLHEP::mm;
  // set larger cuts for gamma, electrons and positrons 
  fCutForGamma     = 0.1*CLHEP::mm;
  fCutForElectron  = 0.1*CLHEP::mm;
  fCutForPositron  = 0.1*CLHEP::mm;

  G4VModularPhysicsList* phys = nullptr;
  G4PhysListFactory factory;
  if (factory.IsReferencePhysList(physName)) {
    phys = factory.GetReferencePhysList(physName);
    if(!phys)G4Exception("SLArPhysicsList::SLArPhysicsList","InvalidSetup",
        FatalException,"PhysicsList does not exist");
    fMessenger = new SLArPhysicsListMessenger(this);
  }
  else {
    G4cout << "Physics list " << physName << " is not defined." << G4endl;
    G4cout << "Available Physics Lists are:" << G4endl;
    factory.AvailablePhysLists();
    G4Exception("SLArPhysicsList::SLArPhysicsList","InvalidSetup",
        FatalException,"PhysicsList does not exist");
  }

  for (G4int i = 0; ; ++i) {
    G4VPhysicsConstructor* elem =
      const_cast<G4VPhysicsConstructor*> (phys->GetPhysics(i));
    if (elem == NULL) break;
    G4cout << "RegisterPhysics: " << elem->GetPhysicsName() << G4endl;
    RegisterPhysics(elem);
  }

  fAbsorptionOn = true;
  fCerenkovOn = do_cerenkov;
  fOpticalPhysics = new SLArOpticalPhysics(fAbsorptionOn, fCerenkovOn); 

  RegisterPhysics(new SLArExtraPhysics());
  RegisterPhysics(fOpticalPhysics);
  ReplacePhysics(new G4EmLivermorePhysics());

  fStepMaxProcess = new SLArStepMax();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

SLArPhysicsList::~SLArPhysicsList()
{
  delete fMessenger;

  delete fStepMaxProcess;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void SLArPhysicsList::ClearPhysics()
{
  for (G4PhysConstVector::iterator p  = fPhysicsVector->begin();
      p != fPhysicsVector->end(); ++p) {
    delete (*p);
  }
  fPhysicsVector->clear();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void SLArPhysicsList::ConstructParticle()
{
  G4VModularPhysicsList::ConstructParticle();

  G4DecayTable* MuonPlusDecayTable = new G4DecayTable();
  MuonPlusDecayTable -> Insert(new
      G4MuonDecayChannelWithSpin("mu+",0.986));
  MuonPlusDecayTable -> Insert(new
      G4MuonRadiativeDecayChannelWithSpin("mu+",0.014));
  G4MuonPlus::MuonPlusDefinition() -> SetDecayTable(MuonPlusDecayTable);

  G4DecayTable* MuonMinusDecayTable = new G4DecayTable();
  MuonMinusDecayTable -> Insert(new
      G4MuonDecayChannelWithSpin("mu-",0.986));
  MuonMinusDecayTable -> Insert(new
      G4MuonRadiativeDecayChannelWithSpin("mu-",0.014));
  G4MuonMinus::MuonMinusDefinition() -> SetDecayTable(MuonMinusDecayTable);

}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void SLArPhysicsList::ConstructProcess()
{
  G4VModularPhysicsList::ConstructProcess();

  SetVerbose(0);

  G4DecayWithSpin* decayWithSpin = new G4DecayWithSpin();

  G4ProcessTable* processTable = G4ProcessTable::GetProcessTable();

  G4VProcess* decay;
  decay = processTable->FindProcess("Decay",G4MuonPlus::MuonPlus());

  G4ProcessManager* pManager;
  pManager = G4MuonPlus::MuonPlus()->GetProcessManager();

  if (pManager) {
    if (decay) pManager->RemoveProcess(decay);
    pManager->AddProcess(decayWithSpin);
    // set ordering for PostStepDoIt and AtRestDoIt
    pManager ->SetProcessOrdering(decayWithSpin, idxPostStep);
    pManager ->SetProcessOrdering(decayWithSpin, idxAtRest);
  }

  decay = processTable->FindProcess("Decay",G4MuonMinus::MuonMinus());

  pManager = G4MuonMinus::MuonMinus()->GetProcessManager();

  if (pManager) {
    if (decay) pManager->RemoveProcess(decay);
    pManager->AddProcess(decayWithSpin);
    // set ordering for PostStepDoIt and AtRestDoIt
    pManager ->SetProcessOrdering(decayWithSpin, idxPostStep);
    pManager ->SetProcessOrdering(decayWithSpin, idxAtRest);
  }

  G4PionDecayMakeSpin* poldecay = new G4PionDecayMakeSpin();

  decay = processTable->FindProcess("Decay",G4PionPlus::PionPlus());

  pManager = G4PionPlus::PionPlus()->GetProcessManager();

  if (pManager) {
    if (decay) pManager->RemoveProcess(decay);
    pManager->AddProcess(poldecay);
    // set ordering for PostStepDoIt and AtRestDoIt
    pManager ->SetProcessOrdering(poldecay, idxPostStep);
    pManager ->SetProcessOrdering(poldecay, idxAtRest);
  }

  decay = processTable->FindProcess("Decay",G4PionMinus::PionMinus());

  pManager = G4PionMinus::PionMinus()->GetProcessManager();

  if (pManager) {
    if (decay) pManager->RemoveProcess(decay);
    pManager->AddProcess(poldecay);
    // set ordering for PostStepDoIt and AtRestDoIt
    pManager ->SetProcessOrdering(poldecay, idxPostStep);
    pManager ->SetProcessOrdering(poldecay, idxAtRest);
  }

  AddStepMax();

}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void SLArPhysicsList::RemoveFromPhysicsList(const G4String& name)
{
  RemovePhysics(name); 
  //G4bool success = false;
  //if (fPhysicsVector) {
    //printf("Physics vector has size %lu\n", fPhysicsVector->size());
  //} else {
    //printf("fPhysicsVector is NULL!\n");
  //}
  //for (G4PhysConstVector::iterator p  = fPhysicsVector->begin();
      //p != fPhysicsVector->end(); ++p) {
    //G4VPhysicsConstructor* e = (*p);
    //printf("physics name: %s\n", e->GetPhysicsName().c_str());
    //if (e->GetPhysicsName() == name) {
      //fPhysicsVector->erase(p);
      //success = true;
      //break;
    //}
  //}
  //if (!success) {
    //G4ExceptionDescription message;
    //message << "PhysicsList::RemoveFromEMPhysicsList "<< name << "not found";
    //G4Exception("example SLArPhysicsList::RemoveFromPhysicsList()",
        //"ExamSLArPhysicsList01",FatalException,message);
  //}
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void SLArPhysicsList::SetAbsorption(G4bool toggle)
{
  fAbsorptionOn = toggle;
  RemoveFromPhysicsList("Optical");
  fPhysicsVector->
    push_back(fOpticalPhysics = new SLArOpticalPhysics(toggle));
  fOpticalPhysics->ConstructProcess();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void SLArPhysicsList::SetCuts()
{
  if (verboseLevel >0) {
    G4cout << "SLArPhysicsList::SetCuts:";
    G4cout << "CutLength : " << G4BestUnit(defaultCutValue,"Length")
      << G4endl;
  }

  // set cut values for gamma at first and for e- second and next for e+,
  // because some processes for e+/e- need cut values for gamma
  SetCutValue(fCutForGamma, "gamma");
  SetCutValue(fCutForElectron, "e-");
  SetCutValue(fCutForPositron, "e+");

  if (verboseLevel>0) DumpCutValuesTable();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void SLArPhysicsList::SetCutForGamma(G4double cut)
{
  fCutForGamma = cut;
  SetParticleCuts(fCutForGamma, G4Gamma::Gamma());
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void SLArPhysicsList::SetCutForElectron(G4double cut)
{
  fCutForElectron = cut;
  SetParticleCuts(fCutForElectron, G4Electron::Electron());
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void SLArPhysicsList::SetCutForPositron(G4double cut)
{
  fCutForPositron = cut;
  SetParticleCuts(fCutForPositron, G4Positron::Positron());
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void SLArPhysicsList::SetStepMax(G4double step)
{
  fStepMaxProcess->SetStepMax(step);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

SLArStepMax* SLArPhysicsList::GetStepMaxProcess()
{
  return fStepMaxProcess;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void SLArPhysicsList::AddStepMax()
{
  // Step limitation seen as a process

  auto particleIterator=GetParticleIterator();
  particleIterator->reset();
  while ((*particleIterator)()){
    G4ParticleDefinition* particle = particleIterator->value();
    G4ProcessManager* pmanager = particle->GetProcessManager();

    if (fStepMaxProcess->IsApplicable(*particle) && !particle->IsShortLived())
    {
      if (pmanager) pmanager ->AddDiscreteProcess(fStepMaxProcess);
    }
  }
}

void SLArPhysicsList::EnforceEMCuts(G4bool to_enforce)
{
  G4EmParameters* param = G4EmParameters::Instance();
  param->SetApplyCuts(to_enforce);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void SLArPhysicsList::SetNbOfPhotonsCerenkov(G4int maxNumber)
{
  fOpticalPhysics->SetNbOfPhotonsCerenkov(maxNumber);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void SLArPhysicsList::SetVerbose(G4int verbose)
{
  if (fOpticalPhysics->GetCerenkovProcess()) {
    fOpticalPhysics->GetCerenkovProcess()->SetVerboseLevel(verbose);
  }
  fOpticalPhysics->GetScintillationProcess()->SetVerboseLevel(verbose);
  fOpticalPhysics->GetAbsorptionProcess()->SetVerboseLevel(verbose);
  fOpticalPhysics->GetRayleighScatteringProcess()->SetVerboseLevel(verbose);
  fOpticalPhysics->GetMieHGScatteringProcess()->SetVerboseLevel(verbose);
  fOpticalPhysics->GetBoundaryProcess()->SetVerboseLevel(verbose);
}
