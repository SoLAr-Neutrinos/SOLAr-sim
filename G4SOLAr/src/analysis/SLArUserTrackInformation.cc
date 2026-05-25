/**
 * @author      Daniele Guffanti (University and INFN Milano-Bicocca)
 * @file        SLArUserTrackInformation.hh
 * @created     Thur Mar 30, 2023 16:48:30 CEST
 */

#include "SLArUserTrackInformation.hh"

SLArUserTrackInformation::SLArUserTrackInformation(SLArEventTrajectory* trj)
  : G4VUserTrackInformation(), fTrajectory(trj), fStoreTrajectory(0)
{}

SLArUserTrackInformation::SLArUserTrackInformation(SLArEventTrajectory* trj, const G4String& infoType) 
  : G4VUserTrackInformation(infoType), fTrajectory(trj), fStoreTrajectory(0)
{}

SLArUserTrackInformation::SLArUserTrackInformation(const SLArUserTrackInformation& info)
  : G4VUserTrackInformation(info), fTrajectory(info.fTrajectory)
{
  fStoreTrajectory = info.fStoreTrajectory; 
}



