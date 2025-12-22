/**
 * @author      Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        SLArUserTrackInformation.hh
 * @created     Thur Mar 30, 2023 16:38:56 CEST
 */

#ifndef SLARUSERTRACKINFORMATION_HH

#define SLARUSERTRACKINFORMATION_HH

#include "G4VUserTrackInformation.hh"
#include "event/SLArEventTrajectory.hh"

class SLArUserTrackInformation : public G4VUserTrackInformation {
  public: 
    inline SLArUserTrackInformation() : G4VUserTrackInformation(), fTrajectory(nullptr), 
      fStoreTrajectory(false), fAncestor(-1), fNphTemp(0), fNelTemp(0) {};
    SLArUserTrackInformation(SLArEventTrajectory* trj); 
    SLArUserTrackInformation(SLArEventTrajectory* trj, const G4String& infoType); 
    SLArUserTrackInformation(const SLArUserTrackInformation& info); 
    inline virtual ~SLArUserTrackInformation() {}; 

    inline G4bool CheckStoreTrajectory() const {return fStoreTrajectory;}
    inline SLArEventTrajectory* GimmeEvTrajectory() {return fTrajectory;}
    inline const SLArEventTrajectory* GimmeConstEvTrajectory() const {return fTrajectory;}
    inline void MakeTrajectory(); 

    inline void SetStoreTrajectory(const G4bool doStore) {fStoreTrajectory = doStore;}
    inline void SetTrajectory(SLArEventTrajectory& trajectory) {fTrajectory = &trajectory;} 
    inline void SetTrajectory(SLArEventTrajectory* trajectory) {fTrajectory = trajectory;}

    inline void SetTrackAncestor(const G4int ancestor) {fAncestor = ancestor;}
    inline G4int GetTrackAncestor() const {return fAncestor;}

  private:
    SLArEventTrajectory* fTrajectory;
    G4bool fStoreTrajectory; 
    G4int fAncestor;
    G4int fNphTemp; 
    G4int fNelTemp; 

};


#endif /* end of include guard SLARUSERTRACKINFORMATION_HH */

