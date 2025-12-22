/**
 * @author      Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        SLArUserTrackInformation.hh
 * @created     Thur Mar 30, 2023 16:48:30 CEST
 */

#include "SLArUserTrackInformation.hh"

SLArUserTrackInformation::SLArUserTrackInformation(SLArEventTrajectory* trj)
  : G4VUserTrackInformation(), fTrajectory(trj), fStoreTrajectory(true), 
  fAncestor(-1), fNphTemp(0), fNelTemp(0)
{}

SLArUserTrackInformation::SLArUserTrackInformation(SLArEventTrajectory* trj, const G4String& infoType) 
  : G4VUserTrackInformation(infoType), fTrajectory(trj), fStoreTrajectory(true), fAncestor(-1), fNphTemp(0), fNelTemp(0)
{}

SLArUserTrackInformation::SLArUserTrackInformation(const SLArUserTrackInformation& info)
  : G4VUserTrackInformation(info), fTrajectory(info.fTrajectory), fStoreTrajectory(info.fStoreTrajectory),
  fAncestor(info.fAncestor), fNphTemp(info.fNphTemp), fNelTemp(info.fNelTemp)
{}



