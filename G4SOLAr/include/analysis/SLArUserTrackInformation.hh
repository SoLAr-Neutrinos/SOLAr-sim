/**
 * @author      Daniele Guffanti (University and INFN Milano-Bicocca)
 * @file        SLArUserTrackInformation.hh
 * @created     Thur Mar 30, 2023 16:38:56 CEST
 */

#ifndef SLARUSERTRACKINFORMATION_HH

#define SLARUSERTRACKINFORMATION_HH

#include "G4VUserTrackInformation.hh"
#include "event/SLArEventTrajectory.hh"

class SLArUserTrackInformation : public G4VUserTrackInformation {
  public: 
    SLArUserTrackInformation(SLArEventTrajectory* trj); 
    SLArUserTrackInformation(SLArEventTrajectory* trj, const G4String& infoType); 
    SLArUserTrackInformation(const SLArUserTrackInformation& info); 
    inline virtual ~SLArUserTrackInformation() {}; 

    inline G4bool CheckStoreTrajectory() {return fStoreTrajectory;}
    inline SLArEventTrajectory* GimmeEvTrajectory() {return fTrajectory;}
    inline const SLArEventTrajectory* GimmeConstEvTrajectory() {return fTrajectory;}
    inline void MakeTrajectory(); 

    inline void SetStoreTrajectory(const G4bool doStore) {fStoreTrajectory = doStore;}
    inline void SetTrajectory(SLArEventTrajectory& trajectory) {fTrajectory = &trajectory;} 

  private:
    SLArEventTrajectory* fTrajectory;
    G4bool fStoreTrajectory; 
    G4int fNphTemp; 
    G4int fNelTemp; 

};


#endif /* end of include guard SLARUSERTRACKINFORMATION_HH */

