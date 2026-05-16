/**
 * @author      : guff (guff@guff-gssi)
 * @file        : SLArUserPhotonTrackInformation
 * @created     : lunedì ago 31, 2020 18:39:16 CEST
 *
 * Reimplemented from optical/LXe/include/LXeUserTrackInformation.hh
 */

#ifndef SLArUSERPHOTONTRACKINFORMATION_HH

#define SLArUSERPHOTONTRACKINFORMATION_HH

#include "G4VUserTrackInformation.hh"
#include "globals.hh"


enum SLArOpTrackStatus { active=1, hitOpDet=2, absorbed=4, boundaryAbsorbed=8,
                    inactive=14};

/*SLArTrackStatus:
  active: still being tracked
  hitOpDet: stopped by being detected in an Optical Detector
  absorbed: stopped by being absorbed with G4OpAbsorbtion
  boundaryAbsorbed: stopped by being aborbed with G4OpAbsorbtion
  inactive: track is stopped for some reason
   -This is the sum of all stopped flags so can be used to remove stopped flags
 
*/

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

    inline virtual void Print() const{};

  private:

    int fStatus;
    G4int fReflections;
    G4bool fForcedraw;
};




#endif /* end of include guard SLArUSERPHOTONTRACKINFORMATION_HH */

