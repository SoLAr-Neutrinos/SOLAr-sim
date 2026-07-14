/**
 * @author      : Daniele Guffanti (University and INFN Milano-Bicocca)
 * @file        : SLArTrajectory.cc
 * @created     : Mon Aug 31, 2020 12:59:32 CEST
 */

#include "SLArAnalysisManager.hh"
#include "SLArTrajectory.hh"
#include "G4TrajectoryPoint.hh"
#include "G4Trajectory.hh"
#include "G4ParticleTable.hh"
#include "G4ParticleTypes.hh"
#include "G4ThreeVector.hh"
#include "G4Polyline.hh"
#include "G4Circle.hh"
#include "G4Colour.hh"
#include "G4VisAttributes.hh"
#include "G4VVisManager.hh"
#include "G4Polymarker.hh"
#include "G4VProcess.hh"

G4ThreadLocal G4Allocator<SLArTrajectory>* SLArTrajectoryAllocator = nullptr;

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

SLArTrajectory::SLArTrajectory()
  :G4Trajectory(),fWls(false),fDrawit(false),
   fForceNoDraw(false),fForceDraw(false), fTime(0.)
{
  fParticleDefinition = nullptr;
  fEdepContainer.reserve(500);
  fNphtContainer.reserve(500); 
  fCopyNumberContainer.reserve(500);
  fNionElectronsContainer.reserve(500); 
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

SLArTrajectory::SLArTrajectory(const G4Track* aTrack)
  :G4Trajectory(aTrack),fWls(false),fDrawit(false), fTime(0.)
{
#ifdef SLAR_DEBUG
  if (aTrack->GetDynamicParticle()->GetParticleDefinition() != 
      G4OpticalPhoton::Definition()) {
    //printf("SLArTrajectory: Create new SLArTrajectory for trk %i\n", 
        //aTrack->GetTrackID());
    //getchar();
  }
#endif

  fEdepContainer.reserve(500);
  fNphtContainer.reserve(500); 
  fCopyNumberContainer.reserve(500);
  fNionElectronsContainer.reserve(500); 
  fParticleDefinition=aTrack->GetDefinition();
  
  if (aTrack->GetParentID() > 0) {
    fCreatorProcess    =aTrack->GetCreatorProcess()->GetProcessName();
  }
  else {
    fCreatorProcess = "PrimaryGenerator";
    // this is a primary. Save the track ID in the corresponding SLArMCPrimaryInfo
    //SLArAnalysisManager* SLArAnaMgr = SLArAnalysisManager::Instance();
    //for (auto &primaryInfo : SLArAnaMgr->GetEvent()->GetPrimaries()) {
      //if (fabs(aTrack->GetMomentum().x() - primaryInfo.GetMomentum()[0]) < 1e-6 &&
          //fabs(aTrack->GetMomentum().y() - primaryInfo.GetMomentum()[1]) < 1e-6 &&
          //fabs(aTrack->GetMomentum().z() - primaryInfo.GetMomentum()[2]) < 1e-6) {
        //primaryInfo.SetTrackID(aTrack->GetTrackID()); 
        //break;
      //}
    //}
  }
  fTime = aTrack->GetGlobalTime(); 
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

SLArTrajectory::SLArTrajectory(SLArTrajectory &right)
  :G4Trajectory(right),fWls(right.fWls),fDrawit(right.fDrawit)
{
  fParticleDefinition=right.fParticleDefinition;
  fEdepContainer.resize(right.fEdepContainer.size() ); 
  fEdepContainer.assign(right.fEdepContainer.begin(), right.fEdepContainer.end());

  fNphtContainer.resize(right.fNphtContainer.size() ); 
  fNphtContainer.assign(right.fNphtContainer.begin(), right.fNphtContainer.end());

  fNionElectronsContainer.resize(right.fNionElectronsContainer.size() ); 
  fNionElectronsContainer.assign(right.fNionElectronsContainer.begin(), right.fNionElectronsContainer.end());

  fCopyNumberContainer.resize(right.fCopyNumberContainer.size() ); 
  fCopyNumberContainer.assign(right.fCopyNumberContainer.begin(), right.fCopyNumberContainer.end());
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

SLArTrajectory::~SLArTrajectory() {
  fEdepContainer.clear();
  fNphtContainer.clear(); 
  fNionElectronsContainer.clear(); 
  fCopyNumberContainer.clear(); 
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void SLArTrajectory::MergeTrajectory(SLArTrajectory* secondTrajectory) {
#ifdef SLAR_DEBUG
  printf("calling SLArTrajectory::MergeTrajectory\n");
#endif

  G4int ent = secondTrajectory->GetPointEntries();
  for(G4int i=1; i<ent; ++i) // initial pt of 2nd trajectory shouldn't be merged
  {
    AddEdep(secondTrajectory->GetEdep().at(i)); 
    AddOpticalPhotons(secondTrajectory->GetNphotons().at(i)); 
    AddIonizationElectrons(secondTrajectory->GetIonElectrons().at(i)); 
    AddVolCopyNumber(secondTrajectory->GetVolCopyNumumber().at(i));  
  }
  secondTrajectory->GetEdep().clear(); 
  secondTrajectory->GetNphotons().clear(); 
  secondTrajectory->GetIonElectrons().clear(); 
  secondTrajectory->GetVolCopyNumumber().clear(); 

  G4Trajectory::MergeTrajectory(secondTrajectory); 
  return; 
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void SLArTrajectory::DrawTrajectory() const
{
  // i_mode is no longer available as an argument of G4VTrajectory.
  // In this exampple it was always called with an argument of 50.
  const G4int i_mode = 50;
  // Consider using commands /vis/modeling/trajectories.
  
  //Taken from G4VTrajectory and modified to select colours based on particle
  //type and to selectively eliminate drawing of certain trajectories.

  if(!fForceDraw && (!fDrawit || fForceNoDraw))
    return;

  // If i_mode>=0, draws a trajectory as a polyline and, if i_mode!=0,
  // adds markers - yellow circles for step points and magenta squares
  // for auxiliary points, if any - whose screen size in pixels is
  // given by std::abs(i_mode)/1000.  E.g: i_mode = 5000 gives easily
  // visible markers.
 
  G4VVisManager* pVVisManager = G4VVisManager::GetConcreteInstance();
  if (!pVVisManager) return;
 
  const G4double markerSize = std::abs(i_mode)/1000;
  G4bool lineRequired (i_mode >= 0);
  G4bool markersRequired (markerSize > 0.);
 
  G4Polyline trajectoryLine;
  G4Polymarker stepPoints;
  G4Polymarker auxiliaryPoints;
 
  for (G4int i = 0; i < GetPointEntries() ; i++) {
    G4VTrajectoryPoint* aTrajectoryPoint = GetPoint(i);
    const std::vector<G4ThreeVector>* auxiliaries
      = aTrajectoryPoint->GetAuxiliaryPoints();
    if (auxiliaries) {
      for (size_t iAux = 0; iAux < auxiliaries->size(); ++iAux) {
        const G4ThreeVector pos((*auxiliaries)[iAux]);
        if (lineRequired) {
          trajectoryLine.push_back(pos);
        }
        if (markersRequired) {
          auxiliaryPoints.push_back(pos);
        }
      }
    }
    const G4ThreeVector pos(aTrajectoryPoint->GetPosition());
    if (lineRequired) {
      trajectoryLine.push_back(pos);
    }
    if (markersRequired) {
      stepPoints.push_back(pos);
    }
  }
 
  if (lineRequired) {
    G4Colour colour;
 
    if(fParticleDefinition==G4OpticalPhoton::OpticalPhotonDefinition()){
      colour = G4Colour(0.,0.5,0.5);
    }
    else if (fParticleDefinition == G4Electron::ElectronDefinition()) {
      colour = G4Colour(1., 1., 0.); 
    }
    else if (fParticleDefinition == G4Gamma::GammaDefinition()) {
      colour = G4Colour(0., 1., 0.); 
    } 
    else if (fParticleDefinition == G4Neutron::NeutronDefinition()) {
      colour = G4Colour(0., 0., 1.); 
    } 
    else if (fParticleDefinition == G4Positron::PositronDefinition()) {
      colour = G4Colour(1., 0., 1.); 
    }
    else {
      //All other particles are red
      colour = G4Colour(1.,0.,0.);
    }
 
    G4VisAttributes trajectoryLineAttribs(colour);
    trajectoryLine.SetVisAttributes(&trajectoryLineAttribs);
    pVVisManager->Draw(trajectoryLine);
  }
  if (markersRequired) {
    auxiliaryPoints.SetMarkerType(G4Polymarker::squares);
    auxiliaryPoints.SetScreenSize(markerSize);
    auxiliaryPoints.SetFillStyle(G4VMarker::filled);
    G4VisAttributes auxiliaryPointsAttribs(G4Colour(0.,1.,1.));  // Magenta
    auxiliaryPoints.SetVisAttributes(&auxiliaryPointsAttribs);
    pVVisManager->Draw(auxiliaryPoints);

    stepPoints.SetMarkerType(G4Polymarker::circles);
    stepPoints.SetScreenSize(markerSize);
    stepPoints.SetFillStyle(G4VMarker::filled);
    G4VisAttributes stepPointsAttribs(G4Colour(1.,1.,0.));  // Yellow
    stepPoints.SetVisAttributes(&stepPointsAttribs);
    pVVisManager->Draw(stepPoints);
  }
}
