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
/// \file SLAr/src/SLArStackingAction.cc
/// \brief Implementation of the SLArStackingAction class
//
//
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#include "SLArStackingAction.hh"
#include "SLArEventAction.hh"
#include "SLArAnalysisManager.hh"
#include "physics/SLArPhysicsList.hh"
#include "SLArUserTrackInformation.hh"
#include "SLArRunAction.hh"

#include "G4VProcess.hh"
#include "G4RunManager.hh"

#include "G4ParticleDefinition.hh"
#include "G4ParticleTypes.hh"
#include "G4Track.hh"
#include "G4ios.hh"
#include <vector>

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

SLArStackingAction::SLArStackingAction(SLArEventAction* ea)
  : G4UserStackingAction(), fEventAction(ea)
{}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

SLArStackingAction::~SLArStackingAction()
{}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4ClassificationOfNewTrack
SLArStackingAction::ClassifyNewTrack(const G4Track * aTrack)
{
  G4ClassificationOfNewTrack kClassification = fUrgent; 

  if(aTrack->GetDefinition() != G4OpticalPhoton::OpticalPhotonDefinition()) {
    // check it the track already owns a user info 
    if (aTrack->GetUserInformation()) {
      //printf("Track ID %i already has User Information\n", aTrack->GetTrackID());
      return kClassification;
    }
    else {
      //printf("Track ID %i is a new one!\n", aTrack->GetTrackID());
      auto SLArAnaMgr = SLArAnalysisManager::Instance(); 
      G4int parentID = 0; 
      if (aTrack->GetParentID() == 0) { // this is a primary
        fEventAction->RegisterNewTrackPID(aTrack->GetTrackID(), aTrack->GetTrackID()); 
        parentID = aTrack->GetTrackID(); 
        //printf("Track %i is a candidate primary with pdg id %i\n", 
            //aTrack->GetTrackID(), aTrack->GetParticleDefinition()->GetPDGEncoding());

        // fix track ID in primary output object
        auto& primaries = SLArAnaMgr->GetMCTruth().GetPrimaries();
        for (auto &primaryInfo : primaries) {
          if (primaryInfo.GetTrackID() != -1) continue; // already assigned
          G4double match = PositivePrimaryIdentification(aTrack, primaryInfo);
          if (match) break;
        }
      } else {
        //printf("Not a primary, recording parent id\n");
        fEventAction->RegisterNewTrackPID(aTrack->GetTrackID(), aTrack->GetParentID()); 
        parentID = aTrack->GetParentID();
      }

      const char* particleName = aTrack->GetParticleDefinition()->GetParticleName().data(); 
      const char* creatorProc  = "PrimaryGenerator"; 
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
        if ( fEventAction->GetProcessExtraInfo().count(trkIdHelp) ) {
          creatorProc = fEventAction->GetProcessExtraInfo()[trkIdHelp];
        }
      }
      
      //printf("creating trajectory...\n");
      SLArEventTrajectory trajectory;
      trajectory.SetTrackID( aTrack->GetTrackID() ); 
      trajectory.SetParentID(aTrack->GetParentID()); 
      trajectory.SetParticleName( particleName );
      trajectory.SetPDGID( aTrack->GetDynamicParticle()->GetPDGcode() ); 
      trajectory.SetCreatorProcess( creatorProc ); 
      trajectory.SetTime( aTrack->GetGlobalTime() ); 
      trajectory.SetWeight(aTrack->GetWeight()); 
      trajectory.SetStoreTrajectoryPts( SLArAnaMgr->StoreTrajectoryFull() ); 
      //trajectory.SetOriginVolCopyNo(aTrack->GetVolume()->GetCopyNo()); 
      trajectory.SetInitKineticEne( aTrack->GetKineticEnergy() ); 
      auto& vertex_momentum = aTrack->GetMomentumDirection();
      trajectory.SetInitMomentum( vertex_momentum.x(), vertex_momentum.y(), vertex_momentum.z() );
      G4int ancestor_id = fEventAction->FindAncestorID( parentID ); 

      SLArMCPrimaryInfo* ancestor = nullptr; 
      auto& primaries = SLArAnaMgr->GetMCTruth().GetPrimaries();
      for (auto &p : primaries) {
        if (p.GetTrackID() == ancestor_id) {
          ancestor = &p; 
          break;
        }
      }
      if (!ancestor) printf("Unable to find corresponding primary particle\n");
#ifdef SLAR_DEBUG
      if (!ancestor) printf("Unable to find corresponding primary particle\n");
#endif

      ancestor->RegisterTrajectory( std::move(trajectory) ); 

      auto trkInfo = new SLArUserTrackInformation( ancestor->GetTrajectories().back() ); 

      trkInfo->SetStoreTrajectory(true); 

      aTrack->SetUserInformation( trkInfo ); 
    }
  }
  else 
  { // particle is optical photon
    if(aTrack->GetParentID() > 0)
    { // particle is secondary
      SLArAnalysisManager* anaMngr = SLArAnalysisManager::Instance(); 
      SLArMCPrimaryInfo* primary = nullptr; 
      //SLArMCPrimaryInfoPtr* primary = nullptr; 
      auto& primaries = anaMngr->GetMCTruth().GetPrimaries();

      int primary_parent_id = fEventAction->FindAncestorID(aTrack->GetParentID()); 
      
      const G4String creator_process = aTrack->GetCreatorProcess()->GetProcessName(); 

      if (creator_process != "OpWLS") {
        //#ifdef SLAR_DEBUG
        //printf("Creator process: %s, Primary parent ID %i\n", primary_parent_id, creator_process.data());
        //#endif
        for (auto &p : primaries) {
          if (p.GetTrackID() == primary_parent_id) {
            primary = &p; 
            //#ifdef SLAR_DEBUG
            //printf("primary parent found\n");
            //#endif
            break; 
          }
        }

#ifdef SLAR_DEBUG
        if (!primary) printf("Unable to find corresponding primary particle\n");
#endif

        if(creator_process == "Scintillation") {
          fEventAction->IncPhotonCount_Scnt();
          if (primary) primary->IncrementScintPhotons(); 
        }
        else if(creator_process == "Cerenkov") {
          fEventAction->IncPhotonCount_Cher();
          if (primary) primary->IncrementCherPhotons();
        }
      }
      else if(creator_process == "OpWLS") {
        fEventAction->IncPhotonCount_WLS();
      }
#ifdef SLAR_DEBUG
      else 
        printf("SLArStackingAction::ClassifyNewTrack unknown photon creation process %s\n", 
            aTrack->GetCreatorProcess()->GetProcessName().c_str());
#endif


      auto physicsList = 
        dynamic_cast<const SLArPhysicsList*>(G4RunManager::GetRunManager()->GetUserPhysicsList());  
      if (!physicsList) {
        G4Exception("SLArStackingAction::ClassifyNewTrack",
            "InvalidCast", FatalException,
            "SLArStackingAction: invalid cast to SLArPhysicsList");
      }
      if (physicsList->DoTraceOptPhotons() == false) {
        kClassification = G4ClassificationOfNewTrack::fKill;
      }
    }
  }


  return kClassification;
}

bool SLArStackingAction::PositivePrimaryIdentification(const G4Track* aTrack, SLArMCPrimaryInfo& aPrimary) const
{
  if (aTrack->GetDynamicParticle()->GetPDGcode() == aPrimary.GetCode()) {

    const G4ThreeVector pVertex(aPrimary.GetVertex().at(0), aPrimary.GetVertex().at(1), aPrimary.GetVertex().at(2));
    const G4ThreeVector pMomentum(aPrimary.GetMomentum().at(0), aPrimary.GetMomentum().at(1), aPrimary.GetMomentum().at(2));

    const HepGeom::Point3D<double>& track_pos_world = aTrack->GetPosition();
    const SLArRunAction* run_action = (SLArRunAction*)G4RunManager::GetRunManager()->GetUserRunAction();
    const G4Transform3D& world2LArVolume = run_action->GetTransformWorld2Det();

    const G4ThreeVector& track_pos = world2LArVolume * track_pos_world;
    const G4ThreeVector& track_mom = aTrack->GetMomentum();

    const G4ThreeVector diffPos = track_pos - pVertex;
    const G4ThreeVector diffMom = track_mom - pMomentum;

#ifdef SLAR_DEBUG
    printf("Comparing primary candidate track ID %i and PDG ID %i with primary info\n",
        aTrack->GetTrackID(), aPrimary.GetCode());
    printf("  Track momentum: [%g, %g, %g]\n",
        track_mom.x(), track_mom.y(), track_mom.z());
    printf("  Position diff: [%g, %g, %g] - mag %g\n",
        diffPos.x(), diffPos.y(), diffPos.z(), diffPos.mag());
    printf("  Momentum diff: [%g, %g, %g] - mag %g\n",
        diffMom.x(), diffMom.y(), diffMom.z(), diffMom.mag());
#endif

    G4double tolerance = 1e-3;
    if (aPrimary.GetCode() > 10000) {
      //printf("possible canidate %i - [%g, %g, %g] vs [%g, %g, %g]\n", 
      //aPrimary.GetCode(), 
      //aTrack->GetMomentum().x(), aTrack->GetMomentum().y(), aTrack->GetMomentum().z(), 
      //aPrimary.GetMomentum()[0], aPrimary.GetMomentum()[1], aPrimary.GetMomentum()[2]); 
      tolerance = 5e-2;
    }

    if ( diffPos.mag() < tolerance && diffMom.mag() < tolerance ) {
      //printf("This is a primary: Corrsponding primary info found\n");
      aPrimary.SetTrackID(aTrack->GetTrackID()); 
      return true;
    }
  }

  return false;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void SLArStackingAction::NewStage()
{}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void SLArStackingAction::PrepareNewEvent()
{}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
