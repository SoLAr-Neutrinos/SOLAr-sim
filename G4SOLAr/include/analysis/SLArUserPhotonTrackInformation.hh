// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************


/**
 * @author      : Daniele Guffanti (University and INFN Milano-Bicocca)
 * @file        : SLArUserPhotonTrackInformation.hh
 * @created     : Mon Aug 31, 2020 18:39:16 CEST
 *
 * Reimplemented from optical/LXe/include/LXeUserTrackInformation.hh
 */

#ifndef SLArUSERPHOTONTRACKINFORMATION_HH

#define SLArUSERPHOTONTRACKINFORMATION_HH

#include <vector>
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

namespace optical {
  enum class EPhotonCreator {kUnknown = 0, kCherenkov = 1, kScintillation = 2, kWLS= 3, kPrimaryGen = 4, kOther = 5};
  inline G4String EPhProcName[6] = {"Unknown", "Cherenkov", "Scintillation", "WLS", "PriamryGen", "Other"};
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

