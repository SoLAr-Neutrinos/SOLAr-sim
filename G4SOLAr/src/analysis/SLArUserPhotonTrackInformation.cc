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
