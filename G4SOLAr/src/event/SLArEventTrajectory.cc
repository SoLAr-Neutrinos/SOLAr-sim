/**
 * @author      Daniele Guffanti (University and INFN Milano-Bicocca)
 * @file        SLArEvTrajectory.cc
 * @created     Mon Aug 31, 2020 17:39:33 CEST
 */

#include "event/SLArEventTrajectory.hh"
#include <cstdio>

ClassImp(SLArEventTrajectory)

SLArEventTrajectory::SLArEventTrajectory() : 
  fParticleName("noName"), 
  fCreatorProcess("noCreator"), 
  fEndProcess("noDestroyer"),
  fPDGID(0), fTrackID(-1), fParentID(-1), 
  fInitKineticEnergy(0.), fOriginVolCopyNo(0), fTrackLength(0.), fTime(0.), fWeight(1.),
  fInitMomentum(TVector3(0,0,0)), 
  fTotalEdep(0.), fTotalNph(0.), fTotalNel(0.)
{
  fTrjPoints.reserve(500);
}

SLArEventTrajectory::SLArEventTrajectory(const SLArEventTrajectory& trj) 
  : TObject(trj)
{
  //printf("Creating new SLArEventTrajectory with copy costructor\n");
  //printf("trk ID %i, PDG ID %i - trj size %lu\n", 
      //trj.GetTrackID(), 
      //trj.GetPDGID(), 
      //trj.fTrjPoints.size());

  fParticleName = trj.fParticleName; 
  fCreatorProcess = trj.fCreatorProcess; 
  fEndProcess = trj.fEndProcess; 
  fPDGID = trj.fPDGID; 
  fTrackID = trj.fTrackID; 
  fParentID = trj.fParentID; 
  fInitKineticEnergy = trj.fInitKineticEnergy; 
  fTrackLength = trj.fTrackLength; 
  fTime = trj.fTime; 
  fOriginVolCopyNo = trj.fOriginVolCopyNo;
  fWeight = trj.fWeight;
  fInitMomentum = trj.fInitMomentum; 
  fTotalEdep = trj.fTotalEdep; 
  fTotalNph = trj.fTotalNph; 
  fTotalNel = trj.fTotalNel; 

  for (const trj_point& pt : trj.fTrjPoints) {
    fTrjPoints.push_back( pt );
  }
  
}

SLArEventTrajectory::~SLArEventTrajectory()
{
  fParticleName = "noName";
  fTrackID      = -1;
  fParentID     = -1;
  fInitKineticEnergy = 0.;
  fTrackLength  = 0;
  fInitMomentum = TVector3(0,0,0);
  fTrjPoints.clear();
}

void SLArEventTrajectory::RegisterPoint(const trj_point& point) {
  fTrjPoints.push_back( point ); 
  return; 
}

void SLArEventTrajectory::RegisterPoint(double x, double y, double z, double energy, double edep, int n_ph, int n_el, int copy)
{
  fTrjPoints.push_back( trj_point(x, y, z, energy, edep, n_ph, n_el, copy) );
  return;
}

ClassImp(SLArEventTrajectoryLite)

SLArEventTrajectoryLite::SLArEventTrajectoryLite() 
  : TObject(), 
    fEvNumber(0), fPDGCode(0), fTrkID(-1), fParentID(-1), fOriginVol(0), 
    fOriginEnergy(0), fEnergy(0.0), fTime(0.0), fWeight(0.0), 
    fOriginVertex(0., 0., 0.), fScorerVertex(0., 0., 0.), fCreator("")
{}

SLArEventTrajectoryLite::SLArEventTrajectoryLite(const SLArEventTrajectoryLite& tright) 
  : TObject(tright) 
{
  fEvNumber = tright.fEvNumber;
  fPDGCode = tright.fPDGCode;
  fTrkID = tright.fTrkID;
  fParentID = tright.fParentID;
  fOriginVol = tright.fOriginVol;
  fOriginEnergy = tright.fOriginEnergy;
  fEnergy = tright.fEnergy;
  fTime = tright.fTime;
  fWeight = tright.fWeight;
  fCreator = tright.fCreator;
  for (int i=0; i<3; i++) {
    fOriginVertex[i] = tright.fOriginVertex[i]; 
    fScorerVertex[i] = tright.fScorerVertex[i]; 
  }
}

SLArEventTrajectoryLite::~SLArEventTrajectoryLite() {}

void SLArEventTrajectoryLite::SetValues(const SLArEventTrajectory& trajectory) 
{
  fPDGCode = trajectory.fPDGID;
  fTrkID = trajectory.fTrackID;
  fParentID = trajectory.fParentID;
  fWeight = trajectory.fWeight;
  fTime = trajectory.fTime;
  fOriginEnergy = trajectory.fInitKineticEnergy;
  fOriginVol = trajectory.fOriginVolCopyNo;
  fCreator = trajectory.fCreatorProcess;
}

void SLArEventTrajectoryLite::Reset() {
  fEvNumber = 0;
  fPDGCode = 0; 
  fTrkID = -1;
  fParentID = -1;
  fOriginVol = 0;
  fOriginEnergy = 0.0;
  fEnergy = 0.0;
  fTime = 0.0; 
  fWeight = 0.0; 
  fCreator = "";
  for (int i=0; i<3; i++) {
    fOriginVertex[i] = 0.;
    fScorerVertex[i] = 0.;
  }
 
  return;
}
