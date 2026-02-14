/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArExtScorerSD
 * @created     : Friday Feb 09, 2024 23:08:46 CET
 */

#include "G4SDManager.hh"
#include "G4RunManager.hh"
#include "G4Run.hh"

#include "SLArUserTrackInformation.hh"
#include "SLArEventTrajectory.hh"
#include "SLArRunAction.hh"
#include <cstdio>
#include "SensitiveDetectors/SLArExtScorerSD.hh"

SLArExtScorerSD::SLArExtScorerSD(G4String name) 
  : G4VSensitiveDetector(name), fHitsCollection(nullptr), fHCID(-10)
{
  collectionName.insert(SensitiveDetectorName+"Coll");
}

SLArExtScorerSD::~SLArExtScorerSD()
{}

void SLArExtScorerSD::Initialize(G4HCofThisEvent* hce) {
  fHitsCollection = new SLArExtHitsCollection(SensitiveDetectorName, collectionName[0]); 
  if (fHCID < 0) {
    fHCID = G4SDManager::GetSDMpointer()->GetCollectionID(fHitsCollection); 
    //if (verboseLevel) {
      printf("Registering SLArExtHitsCollection with ID %i\n", fHCID); 
      //getchar();
    //}
  }

  hce->AddHitsCollection(fHCID,fHitsCollection);
}

G4bool SLArExtScorerSD::ProcessHits(G4Step* step, G4TouchableHistory* ) {
  auto track = step->GetTrack(); 
  auto thePrePoint = step->GetPreStepPoint(); 
  auto thePostPoint = step->GetPostStepPoint(); 
  auto trkInfo = (SLArUserTrackInformation*)track->GetUserInformation(); 

  // terminate track after having entered the scorer volume
  track->SetTrackStatus( fStopAndKill ); 
  if (trkInfo == nullptr) {
    printf("WARNING: %s track with id %i does not own its SLArUserTrkInformation\n", 
        track->GetDynamicParticle()->GetParticleDefinition()->GetParticleName().data(),
        track->GetTrackID()); 
    return false; 
  }
  const SLArEventTrajectory* trajectory = trkInfo->GimmeEvTrajectory();


  if ( fabs(trajectory->GetPDGID()) == 12 || 
       fabs(trajectory->GetPDGID()) == 14 ||
       fabs(trajectory->GetPDGID()) == 16 ) {
    return true;
  }

  auto scorer_hit = new SLArExtHit(); 
  auto iev = G4RunManager::GetRunManager()->GetCurrentRun()->GetNumberOfEvent();
  scorer_hit->fEvNumber = iev; 
  scorer_hit->fOriginEnergy = trajectory->GetInitKineticEne(); 
  scorer_hit->fOriginVol = trajectory->GetOriginVolumeCopyNo(); 
  scorer_hit->fWeight = trajectory->GetWeight(); 
  scorer_hit->fTrkID = trajectory->GetTrackID(); 
  scorer_hit->fParentID = trajectory->GetParentID(); 
  scorer_hit->fTime = trajectory->GetTime(); 
  scorer_hit->fPDGCode = trajectory->GetPDGID(); 
  scorer_hit->fCreator = trajectory->GetCreatorProcess(); 
  scorer_hit->fEnergy = thePostPoint->GetKineticEnergy(); 

  scorer_hit->fOriginVertex[0] = trajectory->GetConstPoints().front().fX; 
  scorer_hit->fOriginVertex[1] = trajectory->GetConstPoints().front().fY; 
  scorer_hit->fOriginVertex[2] = trajectory->GetConstPoints().front().fZ; 

  scorer_hit->fVertex[0] = thePostPoint->GetPosition().x();
  scorer_hit->fVertex[1] = thePostPoint->GetPosition().y();
  scorer_hit->fVertex[2] = thePostPoint->GetPosition().z();

  fHitsCollection->insert( std::move(scorer_hit) ); 

  if (verboseLevel > 1) {
    printf("SLArExtScorerSD::ProcessHits()\n");
    scorer_hit->Print(); 
    //getchar(); 
  }

  return true;
}
