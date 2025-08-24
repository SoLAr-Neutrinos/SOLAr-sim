/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArUserPhotonTrackInformation
 * @created     : Monday Aug 31, 2020 18:39:16 CEST
 *
 * Reimplemented from optical/LXe/include/LXeUserTrackInformation.hh
 */

#ifndef SLArUSERPHOTONTRACKINFORMATION_HH

#define SLArUSERPHOTONTRACKINFORMATION_HH

#include "G4VUserTrackInformation.hh"
#include "globals.hh"


enum SLArTrackStatus { active=1, hitPMT=2, absorbed=4, boundaryAbsorbed=8,
                    inactive=14};

/*SLArTrackStatus:
  active: still being tracked
  hitPMT: stopped by being detected in a PMT
  absorbed: stopped by being absorbed with G4OpAbsorbtion
  boundaryAbsorbed: stopped by being aborbed with G4OpAbsorbtion
  inactive: track is stopped for some reason
   -This is the sum of all stopped flags so can be used to remove stopped flags
 
*/

namespace optical {
  enum EPhotonCreator {kUnknown = 0, kCherenkov = 1, kScintillation = 2, kWLS= 3, kOther = 4};
  inline G4String EPhProcName[5] = {"Unknown", "Cherenkov", "Scintillation", "WLS", "Other"};
}

class SLArUserPhotonTrackInformation : public G4VUserTrackInformation
{
  public:
    SLArUserPhotonTrackInformation();
    virtual ~SLArUserPhotonTrackInformation();

    //Sets the track status to s (does not check validity of flags)
    void SetTrackStatusFlags(int status){fStatus=status;}
    //Does a smart add of track status flags (disabling old flags that conflict)
    //If s conflicts with itself it will not be detected
    void AddTrackStatusFlag(int s);
 
    int GetTrackStatus()const {return fStatus;}
 
    void IncReflections(){fReflections++;}
    G4int GetReflectionCount()const {return fReflections;}

    void SetForceDrawTrajectory(G4bool b){fForcedraw=b;}
    G4bool GetForceDrawTrajectory(){return fForcedraw;}

    inline void SetAncestorID(const G4int& ancestorID) {fAncestorID = ancestorID;}
    inline G4int GetAncestorID() const {return fAncestorID;}

    void SetCreator(optical::EPhotonCreator o){fCreator=o;}
    optical::EPhotonCreator GetCreator() const {return fCreator;}

    void SetOriginVolume(G4int v){fOriginVolume.clear(); fOriginVolume.push_back(v);}
    void SetOriginVolume(const std::vector<G4int>& v){fOriginVolume = v;}
    const std::vector<G4int>& GetOriginVolume() const {return fOriginVolume;}
    std::vector<G4int>& GetOriginVolume() {return fOriginVolume;}
    int GetOriginVolumID() const;

    inline virtual void Print() const{};

  private:
    G4int fStatus;
    G4int fReflections;
    G4int fAncestorID; // ID of the ancestor track, if any
    optical::EPhotonCreator fCreator;
    std::vector<G4int> fOriginVolume;
    G4bool fForcedraw;
};




#endif /* end of include guard SLArUSERPHOTONTRACKINFORMATION_HH */

