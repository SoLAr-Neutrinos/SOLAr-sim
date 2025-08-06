/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArTrackingAction.cc
 * @created     : Tue Aug 09, 2022 22:04:56 CEST
 */

#include "SLArAnalysisManager.hh"
#include "SLArEventAction.hh"

#include "SLArTrajectory.hh"
#include "SLArTrackingAction.hh"
#include "SLArUserPhotonTrackInformation.hh"
#include "SLArUserTrackInformation.hh"
#include "SLArAnalysisManager.hh"

#include "G4TrackingManager.hh"
#include "G4RunManager.hh"
#include "G4Track.hh"
#include "G4ParticleTypes.hh"
#include "G4EventManager.hh"
#include "G4Event.hh"
#include <cstdio>

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

SLArTrackingAction::SLArTrackingAction() {
  fTrackingExtraMessenger = new SLArTrackingActionMessenger( this ); 
}

SLArTrackingAction::~SLArTrackingAction() {
  if (fTrackingExtraMessenger) delete fTrackingExtraMessenger;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void SLArTrackingAction::PreUserTrackingAction(const G4Track* aTrack)
{
  bool debug = false; 

  if (aTrack->GetParticleDefinition() != G4OpticalPhoton::OpticalPhotonDefinition())
  {
    SLArAnalysisManager* SLArAnaMgr = SLArAnalysisManager::Instance();
    fpTrackingManager->SetStoreTrajectory( _store_particle_trajectory_ );
    auto trkInfo = (SLArUserTrackInformation*)aTrack->GetUserInformation();

    if (trkInfo) {
      if (debug) {
        printf("check info: track already has associated information\n");
      }
      if (aTrack->GetParentID() == 0) {
        SLArEventTrajectory t = CreateNewTrajectory(aTrack); 
        SLArMCPrimaryInfo* ancestor = nullptr; 
        auto& primaries = SLArAnaMgr->GetMCTruth().GetPrimaries();
        for (auto &p : primaries) {
          if (p.GetTrackID() == trkInfo->GetTrackAncestor()) {
            ancestor = &p; 
            break;
          }
        }
        if (!ancestor) {
          printf("SLArTrackingAction::PostUserTrackingAction() WARNING"); 
          printf(" Unable to find corresponding ancestor for primary particle %i\n", 
              aTrack->GetTrackID());
        }
        ancestor->RegisterTrajectory( std::move(t) ); 
        trkInfo->SetStoreTrajectory(true); 
        trkInfo->SetTrajectory( ancestor->GetTrajectories().back() ); 

        if (trkInfo->GimmeEvTrajectory()->GetConstPoints().empty()) {
          fpTrackingManager->SetTrajectory(new SLArTrajectory(aTrack));
        }
      }
      else {
        auto event = G4EventManager::GetEventManager()->GetNonconstCurrentEvent();
        auto trj_container = event->GetTrajectoryContainer();

        if (trj_container) {
          auto trj_vector = trj_container->GetVector();
          for (auto &tt : *trj_vector) {
            auto t = (SLArTrajectory*)tt;
            if (t->GetTrackID() == aTrack->GetTrackID()) {
              fpTrackingManager->SetTrajectory( t );
              break;
            }
          }
        }
      }
    }
  }
  else {
    auto photonInfo = (SLArUserPhotonTrackInformation*)aTrack->GetUserInformation();
    if (photonInfo) {
      const auto touchable_handle = aTrack->GetTouchableHandle();
      if (!touchable_handle) {
        G4Exception("SLArStackingAction::ClassifyNewTrackOpticalPhoton",
            "InvalidTouchable", FatalException,
            "SLArStackingAction: track has no touchable handle");
      }
      std::vector<int>& origin_vol = photonInfo->GetOriginVolume();
      if (origin_vol.empty()) {
        const auto touchableHandle = aTrack->GetTouchableHandle(); 
        if (touchableHandle) {
          G4int depth = touchableHandle->GetHistoryDepth();
          origin_vol.reserve(depth + 1);
          for (G4int i = 0; i <= depth; ++i) {
            origin_vol.push_back(touchableHandle->GetCopyNumber(i));
          }
        }
      }
    }
    if (_store_photon_trajectory_) {
      fpTrackingManager->SetStoreTrajectory( _store_photon_trajectory_ );
      if (fpTrackingManager->GetStoreTrajectory()) {
        fpTrackingManager->SetTrajectory(new SLArTrajectory(aTrack));
      }
    }
  }

  if (debug) {
    auto trkInfo = dynamic_cast<SLArUserTrackInformation*>(aTrack->GetUserInformation());
    printf("check info: ancestor %i - trajectory %p\n", 
        trkInfo->GetTrackAncestor(), trkInfo->GimmeEvTrajectory());
  }
//#ifdef SLAR_DEBUG
  //printf("SLArTrackingAction::PreUserTrackingAction() DONE\n");
//#endif // DEBUG
       //Use custom trajectory class
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void SLArTrackingAction::PostUserTrackingAction(const G4Track* aTrack){
  SLArTrajectory* trajectory =
    (SLArTrajectory*)fpTrackingManager->GimmeTrajectory();

  const G4TrackStatus status = aTrack->GetTrackStatus();

  bool debug = false;
  if (debug) {
    printf("SLArTrackingAction::PostUserTrackingAction() Track ID %i, "
        "Particle %s, Parent ID %i\n", 
        aTrack->GetTrackID(), 
        aTrack->GetParticleDefinition()->GetParticleName().data(),
        aTrack->GetParentID());
  }

  if (aTrack->GetParticleDefinition() != G4OpticalPhoton::OpticalPhotonDefinition()) {
    if (fpTrackingManager->GetStoreTrajectory() == true) {
      trajectory->SetDrawTrajectory(true);
    }

    SetupSecondaries(aTrack, debug);

    auto trackInformation = (SLArUserTrackInformation*)aTrack->GetUserInformation();
    if (trackInformation && status != fSuspend) {
      delete trackInformation; //delete the user info
      aTrack->SetUserInformation(nullptr); //remove it from the track
    }
  }
  else {
    SLArUserPhotonTrackInformation*
      trackInformation=(SLArUserPhotonTrackInformation*)aTrack->GetUserInformation();

    if (trackInformation) {
      if (debug) {
        printf("SLArTrackingAction::PostUserTrackingAction() Optical photon "
            "Track ID %i, creator process %i\n", 
            aTrack->GetTrackID(), trackInformation->GetCreator());
      }

      SetupSecondariesFromOpticalPhoton(aTrack, debug);

      if(trackInformation->GetForceDrawTrajectory()) {
        trajectory->SetDrawTrajectory(true);
      }

      if (status != fSuspend) {
        delete trackInformation; //delete the user info
        aTrack->SetUserInformation(nullptr); //remove it from the track
      } 
    }
  }

  if (debug) {
    getchar();
  }
}

void SLArTrackingAction::SetupSecondaries(const G4Track* aTrack, const bool debug) 
{
  const std::vector<G4Track*>* secondaries = aTrack->GetStep()->GetSecondary();
  SLArAnalysisManager* SLArAnaMgr = SLArAnalysisManager::Instance();
  const auto trackInformation = 
    dynamic_cast<SLArUserTrackInformation*>(aTrack->GetUserInformation());

  for (G4Track* sec : *secondaries) {
    if (sec->GetParticleDefinition() == G4OpticalPhoton::OpticalPhotonDefinition()) {
      SLArUserPhotonTrackInformation* photonInfo = 
        dynamic_cast<SLArUserPhotonTrackInformation*>(sec->GetUserInformation());
      if (photonInfo) {
        photonInfo->SetForceDrawTrajectory( _store_photon_trajectory_ ); 
        continue;
      }

      photonInfo = new SLArUserPhotonTrackInformation(); 
      photonInfo->SetAncestorID( trackInformation->GetTrackAncestor() ); 
      photonInfo->SetForceDrawTrajectory( _store_photon_trajectory_ ); 
      sec->SetUserInformation(photonInfo);
      continue;
    }

    if (sec->GetParentID() ==0 ) {
      continue;
    }

    SLArUserTrackInformation* secInfo = new SLArUserTrackInformation();
    if (sec->GetParentID() == 0) {
      printf("SLArTrackingAction::PostUserTrackingAction() WARNING: "
          "Primary particle %i in secondaries loop???\n", 
          aTrack->GetTrackID());
      secInfo->SetTrackAncestor( aTrack->GetTrackID() );
    } else {
      secInfo->SetTrackAncestor( trackInformation->GetTrackAncestor() );
    }

    SLArEventTrajectory t = CreateNewTrajectory(sec); 
    SLArMCPrimaryInfo* ancestor = nullptr; 
    auto& primaries = SLArAnaMgr->GetMCTruth().GetPrimaries();
    for (auto &p : primaries) {
      if (p.GetTrackID() == secInfo->GetTrackAncestor()) {
        ancestor = &p; 
        break;
      }
    }
    if (!ancestor) {
      printf("SLArTrackingAction::PostUserTrackingAction() WARNING"); 
      printf(" Unable to find corresponding ancestor for secondary particle %i\n", 
          sec->GetTrackID());
    }
    ancestor->RegisterTrajectory( std::move(t) ); 
    secInfo->SetStoreTrajectory(true); 
    secInfo->SetTrajectory( ancestor->GetTrajectories().back() ); 
    if (debug) {
      printf("SLArTrackingAction::SetupSecondaries() "
          "Secondary particle PDG %i - %g MeV, ancestor %i - trajectory %p\n", 
          sec->GetParticleDefinition()->GetPDGEncoding(), 
          sec->GetKineticEnergy(),
          secInfo->GetTrackAncestor(), 
          secInfo->GimmeEvTrajectory());
    }
    sec->SetUserInformation(secInfo);
  }

  return;
}

void SLArTrackingAction::SetupSecondariesFromOpticalPhoton(const G4Track* aTrack, const bool debug) 
{
  const std::vector<G4Track*>* secondaries = aTrack->GetStep()->GetSecondary();
  SLArAnalysisManager* SLArAnaMgr = SLArAnalysisManager::Instance();
  const auto trackInformation = 
    dynamic_cast<SLArUserPhotonTrackInformation*>(aTrack->GetUserInformation());

  for (G4Track* sec : *secondaries) {
    if (sec->GetParticleDefinition() == G4OpticalPhoton::OpticalPhotonDefinition()) {
      SLArUserPhotonTrackInformation* photonInfo = 
        dynamic_cast<SLArUserPhotonTrackInformation*>(sec->GetUserInformation());
      if (photonInfo) {
        photonInfo->SetForceDrawTrajectory( _store_photon_trajectory_ ); 
        continue;
      }

      photonInfo = new SLArUserPhotonTrackInformation(); 
      photonInfo->SetAncestorID( trackInformation->GetAncestorID() ); 
      photonInfo->SetForceDrawTrajectory( _store_photon_trajectory_ ); 
      sec->SetUserInformation(photonInfo);
      continue;
    }

    if (sec->GetParentID() ==0 ) {
      continue;
    }

    SLArUserTrackInformation* secInfo = new SLArUserTrackInformation();
    if (sec->GetParentID() == 0) {
      printf("SLArTrackingAction::PostUserTrackingAction() WARNING: "
          "Primary particle %i in secondaries loop???\n", 
          aTrack->GetTrackID());
      secInfo->SetTrackAncestor( aTrack->GetTrackID() );
    } else {
      secInfo->SetTrackAncestor( trackInformation->GetAncestorID() );
    }

    SLArEventTrajectory t = CreateNewTrajectory(sec); 
    SLArMCPrimaryInfo* ancestor = nullptr; 
    auto& primaries = SLArAnaMgr->GetMCTruth().GetPrimaries();
    for (auto &p : primaries) {
      if (p.GetTrackID() == secInfo->GetTrackAncestor()) {
        ancestor = &p; 
        break;
      }
    }
    if (!ancestor) {
      printf("SLArTrackingAction::PostUserTrackingAction() WARNING"); 
      printf(" Unable to find corresponding ancestor for secondary particle %i\n", 
          sec->GetTrackID());
    }
    ancestor->RegisterTrajectory( std::move(t) ); 
    secInfo->SetStoreTrajectory(true); 
    secInfo->SetTrajectory( ancestor->GetTrajectories().back() ); 
    sec->SetUserInformation(secInfo);
  }

  return;
}

G4String SLArTrackingAction::GetProcessName(const G4Track* aTrack) const
{
  auto fEventAction = static_cast<const SLArEventAction*>
    (G4RunManager::GetRunManager()->GetUserEventAction());
  G4String creatorProc  = "PrimaryGenerator"; 
  if (aTrack->GetCreatorProcess()) {
    creatorProc = aTrack->GetCreatorProcess()->GetProcessName(); 
    G4double momentum_4[4] = {0}; 
    for (size_t i = 0; i < 3; i++) {
      momentum_4[i] = aTrack->GetMomentum()[i];
    }
    momentum_4[3] = aTrack->GetKineticEnergy(); 
    auto trkIdHelp = SLArEventAction::TrackIdHelpInfo_t(
        aTrack->GetParentID(), 
        aTrack->GetDynamicParticle()->GetPDGcode(),
        momentum_4); 
    //std::printf("trkID: %i, ParentID: %i, pdg code: %i\n", 
    //aTrack->GetTrackID(), aTrack->GetParentID(), trkIdHelp.pdg); 
    if ( fEventAction->GetProcessExtraInfo().find(trkIdHelp) 
        != fEventAction->GetProcessExtraInfo().end() ) {
      creatorProc = fEventAction->GetProcessExtraInfo().at(trkIdHelp);
    }
  }
  return creatorProc;
}

SLArEventTrajectory SLArTrackingAction::CreateNewTrajectory(const G4Track* aTrack) 
{
  SLArAnalysisManager* SLArAnaMgr = SLArAnalysisManager::Instance();
  SLArEventTrajectory trajectory;

  G4String creatorProc = GetProcessName(aTrack);

  trajectory.SetTrackID( aTrack->GetTrackID() ); 
  trajectory.SetParentID(aTrack->GetParentID()); 
  trajectory.SetParticleName( aTrack->GetParticleDefinition()->GetParticleName() );
  trajectory.SetPDGID( aTrack->GetDynamicParticle()->GetPDGcode() ); 
  trajectory.SetCreatorProcess( creatorProc ); 
  trajectory.SetTime( aTrack->GetGlobalTime() ); 
  trajectory.SetWeight(aTrack->GetWeight()); 
  trajectory.SetStoreTrajectoryPts( SLArAnaMgr->StoreTrajectoryFull() ); 
  //trajectory.SetOriginVolCopyNo(aTrack->GetVolume()->GetCopyNo()); 
  trajectory.SetInitKineticEne( aTrack->GetKineticEnergy() ); 
  auto& vertex_momentum = aTrack->GetMomentumDirection();
  trajectory.SetInitMomentum( vertex_momentum.x(), vertex_momentum.y(), vertex_momentum.z() );

  return trajectory;
}

#include <G4UIcmdWithABool.hh>

SLArTrackingActionMessenger::SLArTrackingActionMessenger(SLArTrackingAction* trkAction)
  : G4UImessenger(), fTrkAction(trkAction), 
  fCmdStoreParticleTrajectory(nullptr), fCmdStorePhotonTrajectory(nullptr)
{
  fCmdStoreParticleTrajectory = 
    new G4UIcmdWithABool("/tracking/storeParticleTrajectory", this); 
  fCmdStoreParticleTrajectory->SetGuidance("store (or not) particle trajectory"); 
  fCmdStoreParticleTrajectory->SetDefaultValue(true);

  fCmdStorePhotonTrajectory = 
    new G4UIcmdWithABool("/tracking/storePhotonTrajectory", this); 
  fCmdStorePhotonTrajectory->SetGuidance("store (or not) optical photon trajectory"); 
  fCmdStorePhotonTrajectory->SetDefaultValue(true);

  return;
}

SLArTrackingActionMessenger::~SLArTrackingActionMessenger() {
  if (fCmdStoreParticleTrajectory) delete fCmdStoreParticleTrajectory;
  if (fCmdStorePhotonTrajectory) delete fCmdStorePhotonTrajectory;
}

void SLArTrackingActionMessenger::SetNewValue(G4UIcommand* command, G4String newValue) {
  if (command == fCmdStoreParticleTrajectory) {
    G4bool do_store = fCmdStoreParticleTrajectory->GetNewBoolValue(newValue); 
    fTrkAction->_store_particle_trajectory_ = do_store;
  }
  else if (command == fCmdStorePhotonTrajectory) {
    G4bool do_store = fCmdStorePhotonTrajectory->GetNewBoolValue(newValue); 
    fTrkAction->_store_photon_trajectory_ = do_store;
  }

  return;
}

