/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArLArSD.cc
 * @created     : giovedì nov 03, 2022 12:44:17 CET
 */


#include "SLArAnalysisManager.hh"
#include "SLArUserTrackInformation.hh"
#include "SLArTrackingAction.hh"
#include "SLArRunAction.hh"
#include "SLArTrajectory.hh"
#include "SLArScintillation.h"
#include "SLArPrimaryGeneratorAction.hh"
#include "SLArEventAction.hh"
#include "detector/TPC/SLArLArSD.hh"
#include "detector/TPC/SLArLArHit.hh"
#include "physics/SLArElectronDrift.hh"

#include "G4EventManager.hh"
#include "G4RunManager.hh"
#include "G4TrackingManager.hh"
#include "G4HCofThisEvent.hh"
#include "G4TouchableHistory.hh"
#include "G4VPhysicalVolume.hh"
#include "G4VProcess.hh"
#include "G4Track.hh"
#include "G4Step.hh"
#include "G4SDManager.hh"
#include "G4ios.hh"
#include "G4PhysicalConstants.hh"
#include "G4OpticalPhoton.hh"
#include "G4UnitsTable.hh"
#include <cstdio>

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

SLArLArSD::SLArLArSD(G4String name, G4int tpcID)
  : G4VSensitiveDetector(name), fHitsCollection(0), 
    fHCID(-4), fTPCID(tpcID)
{
  collectionName.insert("TPC"+std::to_string(tpcID)+"Coll");
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

SLArLArSD::~SLArLArSD()
{}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void SLArLArSD::Initialize(G4HCofThisEvent* hce)
{
  fHitsCollection 
    = new SLArLArHitsCollection(SensitiveDetectorName,collectionName[0]);
  if (fHCID<0)
  { fHCID = G4SDManager::GetSDMpointer()
    ->GetCollectionID(fHitsCollection); }

  hce->AddHitsCollection(fHCID,fHitsCollection);

  // Add single hit for the entire target volume
  fHitsCollection->insert(new SLArLArHit());
 
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4bool SLArLArSD::ProcessHits(G4Step* step, G4TouchableHistory*)
{

  G4StepPoint* preStepPoint  = step->GetPreStepPoint();
  G4StepPoint* postStepPoint = step->GetPostStepPoint();

  G4double     edep          = step->GetTotalEnergyDeposit();

  G4TouchableHistory* touchable
    = (G4TouchableHistory*)(step->GetPreStepPoint()->GetTouchable());
#ifdef SLAR_DEBUG
  printf("SLArLArSD::ProcessHits(trk %i)\n", step->GetTrack()->GetTrackID());
#endif

  if (step->GetTrack()->GetDynamicParticle()
      ->GetDefinition() != G4OpticalPhoton::OpticalPhotonDefinition()) {

#ifdef SLAR_DEBUG
    printf("SLArLArSD::ProcessHits(): processing %s [%i] TPC hit\n", 
        step->GetTrack()->GetParticleDefinition()->GetParticleName().data(), 
        step->GetTrack()->GetTrackID());
    //getchar(); 
#endif

    SLArRunAction* runAction = 
      (SLArRunAction*)G4RunManager::GetRunManager()->GetUserRunAction(); 

    SLArAnalysisManager* anaMngr = SLArAnalysisManager::Instance(); 

    // Get hit from collection
    SLArLArHit* hit = (*fHitsCollection)[0];
    hit->Add(edep);

    int n_ph = 0; 
    int n_el = 0; 
    auto trackingAction = 
      (SLArTrackingAction*)G4EventManager::GetEventManager()->GetUserTrackingAction(); 

    auto stepMngr = trackingAction->GetTrackingManager()->GetSteppingManager(); 
    if (stepMngr->GetfStepStatus() != fAtRestDoItProc) {
      G4ProcessVector* process_vector = stepMngr->GetfPostStepDoItVector(); 
      for (size_t iproc = 0; iproc < stepMngr->GetMAXofPostStepLoops(); iproc++) {
        G4VProcess* proc = (*process_vector)[iproc]; 

        if (proc->GetProcessName() == "Scintillation")  {
          SLArScintillation* scint_process = (SLArScintillation*)proc; 
          n_ph = scint_process->GetNumPhotons(); 
          n_el = scint_process->GetNumIonElectrons(); 

          break;
        } 
      }
    }

    try {
      auto& anodeCfg = anaMngr->GetAnodeCfgByTPC(fTPCID);  


#ifdef SLAR_DEBUG
      printf("SLArLArSD::ProcessHits(): processing %s [%i] TPC hit: %i electrons to drift\n", 
          step->GetTrack()->GetParticleDefinition()->GetParticleName().data(), 
          step->GetTrack()->GetTrackID(), 
          n_el);
#endif

      auto generatorAction = (gen::SLArPrimaryGeneratorAction*)
        G4RunManager::GetRunManager()->GetUserPrimaryGeneratorAction(); 
      const auto eventAction = (SLArEventAction*)
        G4RunManager::GetRunManager()->GetUserEventAction(); 
      auto ancestor_id = eventAction->FindAncestorID(step->GetTrack()->GetTrackID()); 
      // Add edep in LAr to the primary 
      SLArMCPrimaryInfo* ancestor = nullptr;
      auto& primaries = anaMngr->GetEvent().GetPrimaries();
      for (auto &p : primaries) {
        if (p.GetTrackID() == ancestor_id) {
          ancestor = &p;
          break;
        }
      }

      if (ancestor) ancestor->IncrementLArEdep(edep); 

      if (generatorAction->DoDriftElectrons()) {
        runAction->GetElectronDrift()->Drift(n_el, 
            step->GetTrack()->GetTrackID(), ancestor_id,
            0.5*(postStepPoint->GetPosition()+preStepPoint->GetPosition()),
            postStepPoint->GetGlobalTime(), 
            &anodeCfg, 
            &anaMngr->GetEvent().GetEventAnodeByTPCID(fTPCID)); 
      } 
      //else {
        //printf("SLArLArSD::ProcessHits WARNING: Sensitive Detector TPC ID %i does not match with any TPC in the geometry\n", fTPCID);
        //getchar(); 
      //}
    }
    catch (const std::exception& e) {
      G4cerr << "SLArLArSD::ProcessHits() ERROR: No anode config found for TPC "
        << fTPCID << G4endl; 
      G4cerr << e.what() << G4endl; 
      return false;
    }

  }     

  return true;
}
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
