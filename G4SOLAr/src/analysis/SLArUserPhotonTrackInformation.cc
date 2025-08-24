/**
 * @author      : guff (guff@guff-gssi)
 * @file        : SLArUserPhotonTrackInformation
 * @created     : lunedì ago 31, 2020 18:39:43 CEST
 */

#include "SLArUserPhotonTrackInformation.hh"

SLArUserPhotonTrackInformation::SLArUserPhotonTrackInformation()
  : fStatus(active),fAncestorID(-1),fCreator(optical::kUnknown),fReflections(0),fForcedraw(false) {}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

SLArUserPhotonTrackInformation::~SLArUserPhotonTrackInformation() {}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void SLArUserPhotonTrackInformation::AddTrackStatusFlag(int s)
{
  if(s&active) //track is now active
    fStatus&=~inactive; //remove any flags indicating it is inactive
  else if(s&inactive) //track is now inactive
    fStatus&=~active; //remove any flags indicating it is active
  fStatus|=s; //add new flags
}

int SLArUserPhotonTrackInformation::GetOriginVolumID() const {
  G4int originVolID = 0;
  if ( fCreator == optical::kWLS ) {
    if (fOriginVolume.size() > 4) {
      originVolID  = fOriginVolume.at(3)*1e6;  // optical module wall
      originVolID += fOriginVolume.at(2)*1e3; // optical module row
      originVolID += fOriginVolume.at(1);     // optical module column
    }
  }
  else {
    originVolID = fOriginVolume[0];
  }
  return originVolID;
}
