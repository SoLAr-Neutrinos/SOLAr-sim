/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArTrackingAction.cc
 * @created     : Tue Aug 09, 2022 22:04:56 CEST
 */

#include "SLArAnalysisManager.hh"

#include "SLArTrajectory.hh"
#include "SLArTrackingAction.hh"
#include "SLArUserPhotonTrackInformation.hh"
#include "SLArUserTrackInformation.hh"
#include "SLArDetectorConstruction.hh"

#include "G4TrackingManager.hh"
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
//#ifdef SLAR_DEBUG
  //printf("SLArTrackingAction::PreUserTrackingAction\n");
//#endif // DEBUG

  //Let this be up to the user via vis.mac
  if (aTrack->GetParticleDefinition() != G4OpticalPhoton::OpticalPhotonDefinition())
  {
    fpTrackingManager->SetStoreTrajectory( _store_particle_trajectory_ );
    auto trkInfo = (SLArUserTrackInformation*)aTrack->GetUserInformation();

    if (trkInfo) {
      if (trkInfo->GimmeEvTrajectory()->GetConstPoints().empty()) {
        fpTrackingManager->SetTrajectory(new SLArTrajectory(aTrack));
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
    if (fpTrackingManager->GetStoreTrajectory()) {
      fpTrackingManager->SetStoreTrajectory( _store_photon_trajectory_ );
      //This user track information is only relevant to the photons
      fpTrackingManager->SetUserTrackInformation(
          new SLArUserPhotonTrackInformation);
      if (fpTrackingManager->GetStoreTrajectory()) {
        fpTrackingManager->SetTrajectory(new SLArTrajectory(aTrack));
      }
    }
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

  //Lets choose to draw only the photons that hit the sphere and a pmt
  if (fpTrackingManager->GetStoreTrajectory()) {
    if(aTrack->GetDefinition()==G4OpticalPhoton::OpticalPhotonDefinition()){
      SLArUserPhotonTrackInformation*
        trackInformation=(SLArUserPhotonTrackInformation*)aTrack->GetUserInformation();

      //const G4VProcess* creator=aTrack->GetCreatorProcess();
      //if(creator && creator->GetProcessName()=="OpWLS"){
      // trajectory->WLS();
      // trajectory->SetDrawTrajectory(false);
      //trajectory->SetDrawTrajectory(true);
      //}

      //if((trackInformation->GetTrackStatus()&hitPMT)|| 
      //(trackInformation->GetTrackStatus()&absorbed) ||
      //(trackInformation->GetTrackStatus()&boundaryAbsorbed) )
      trajectory->SetDrawTrajectory(true);

      if (trackInformation) {
        if(trackInformation->GetForceDrawTrajectory())
          trajectory->SetDrawTrajectory(true);
      }
    }
    else //draw all other trajectories and store them in SLArMCPrimaryInfo
    {
      trajectory->SetDrawTrajectory(true);
    }  
  }

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

