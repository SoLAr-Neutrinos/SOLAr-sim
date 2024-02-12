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
// $Id: SLArSuperCellSD.cc 76474 2013-11-11 10:36:34Z gcosmo $
//
/// \file SLArSuperCellSD.cc
/// \brief Implementation of the SLArSuperCell class

#include "detector/SuperCell/SLArSuperCellSD.hh"
#include "detector/SuperCell/SLArSuperCellHit.hh"

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

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

SLArSuperCellSD::SLArSuperCellSD(G4String name)
: G4VSensitiveDetector(name), fHitsCollection(0), fHCID(-1)
{
    collectionName.insert("SuperCellColl");
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

SLArSuperCellSD::~SLArSuperCellSD()
{}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void SLArSuperCellSD::Initialize(G4HCofThisEvent* hce)
{
  fHitsCollection 
    = new SLArSuperCellHitsCollection(SensitiveDetectorName,collectionName[0]);
  if (fHCID<0) { 
    fHCID = G4SDManager::GetSDMpointer()->GetCollectionID(fHitsCollection); 
  }

  hce->AddHitsCollection(fHCID,fHitsCollection);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4bool SLArSuperCellSD::ProcessHits(G4Step* step, G4TouchableHistory*)
{
  //G4StepPoint* preStepPoint  = step->GetPreStepPoint();
  //G4StepPoint* postStepPoint = step->GetPostStepPoint();


  G4TouchableHistory* touchable
    = (G4TouchableHistory*)(step->GetPreStepPoint()->GetTouchable());


#ifdef SLAR_DEBUG
  printf("SLArSuperCellSD::ProcessHits(): processing %s [%i] TPC hit\n", 
      step->GetTrack()->GetParticleDefinition()->GetParticleName().data(), 
      step->GetTrack()->GetTrackID());
  //getchar(); 
#endif

  //if (step->GetTrack()->GetDynamicParticle()
      //->GetDefinition()->GetParticleName() != "opticalphoton") {

    //G4ThreeVector worldPos = preStepPoint->GetPosition();
    //G4ThreeVector localPos
      //= touchable->GetHistory()
        //->GetTopTransform().TransformPoint(worldPos);

    //SLArSuperCellHit* hit = new SLArSuperCellHit();
    //hit->SetWorldPos(worldPos);
    //hit->SetLocalPos(localPos);
    //hit->SetPhotonEnergy(preStepPoint->GetTotalEnergy());
    //hit->SetTime(preStepPoint->GetGlobalTime());
    //hit->SetSuperCellNo(
        //preStepPoint->GetTouchableHandle()->GetCopyNumber(1));

    //delete hit;
    ////fHitsCollection->insert(hit);

  //}     

  return true;
}

G4bool SLArSuperCellSD::ProcessHits_constStep(const G4Step* step,
                                       G4TouchableHistory* ){

  G4Track* track = step->GetTrack();
  //G4cout << "SLArSuperCellSD::ProcessHits_constStep" << G4endl;
  //need to know if this is an optical photon
  if(track->GetDefinition()
     != G4OpticalPhoton::OpticalPhotonDefinition()) return false;
#ifdef SLAR_DEBUG
  printf("SLArSuperCellSD::ProcessHits_constStep(): processing %s [%i] TPC hit\n", 
      step->GetTrack()->GetParticleDefinition()->GetParticleName().data(), 
      step->GetTrack()->GetTrackID());
  //getchar(); 
#endif


  G4double phEne = 0*CLHEP::eV;

  G4StepPoint* preStepPoint  = step->GetPreStepPoint();
  G4StepPoint* postStepPoint = step->GetPostStepPoint();
  
  G4TouchableHistory* touchable
    = (G4TouchableHistory*)(step->GetPostStepPoint()->GetTouchable());

  G4ThreeVector worldPos 
    = postStepPoint->GetPosition();
  G4ThreeVector localPos
    = touchable->GetHistory()
      ->GetTopTransform().TransformPoint(worldPos);
 
  SLArSuperCellHit* hit = nullptr;
  //Find the correct hit collection (in case of multple PMTs)

  if (track->GetParticleDefinition() == 
      G4OpticalPhoton::OpticalPhotonDefinition())
  {
    // Get the creation process of optical photon
    G4String procName = "";
    if (track->GetCreatorProcess()) // make sure consider only secondaries
    {
      procName = track->GetCreatorProcess()->GetProcessName();
    }
    phEne = track->GetTotalEnergy();

    hit = new SLArSuperCellHit(); //so create new hit
    hit->SetPhotonEnergy( phEne );
    hit->SetPhotonWavelength( CLHEP::h_Planck * CLHEP::c_light / phEne *1e6); 
    hit->SetWorldPos(worldPos);
    hit->SetLocalPos(localPos);
    hit->SetTime(preStepPoint->GetGlobalTime());
    if (verboseLevel > 2) {
      for (int i=0; i<5; i++) {
        printf("[%i] volume: %s - copyNo: %i\n", 
            i, touchable->GetVolume(i)->GetName().data(), touchable->GetCopyNumber(i));
      }
      //getchar(); 
    }
    //hit->SetSuperCellIdx(postStepPoint->
        //GetTouchableHandle()->GetCopyNumber(1));
    hit->SetSuperCellNo( touchable->GetCopyNumber(1) ); 
    hit->SetSuperCellRowNo( touchable->GetCopyNumber(2) ); 
    hit->SetSuperCellArrayNo( touchable->GetCopyNumber(3) ); 

    hit->SetPhotonProcess(procName);
    hit->SetProducerID( track->GetParentID() );
    
    fHitsCollection->insert(hit);
  }
  return true;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
