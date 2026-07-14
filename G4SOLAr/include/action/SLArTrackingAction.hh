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
 * @file        : SLArTrackingAction.hh
 * @created     : Mon Aug 31, 2020 18:35:29 CEST
 *
 * Reimplemented from optical/LXe/include/LXeTrackingAction.hh
 */

#ifndef SLArTRACKINGACTION_HH

#define SLArTRACKINGACTION_HH

#include "G4UserTrackingAction.hh"
#include "G4TrackingManager.hh"
#include "G4UImessenger.hh"
#include "globals.hh"

#include "event/SLArEventTrajectory.hh"

class SLArTrackingActionMessenger;

class SLArTrackingAction : public G4UserTrackingAction {

  public:

    SLArTrackingAction();
    virtual ~SLArTrackingAction();

    virtual void PreUserTrackingAction(const G4Track*);
    virtual void PostUserTrackingAction(const G4Track*);

    G4TrackingManager* GetTrackingManager() {return fpTrackingManager;}

  private:
    SLArTrackingActionMessenger* fTrackingExtraMessenger;
    G4bool _store_particle_trajectory_  = false;
    G4bool _store_photon_trajectory_ = false;
    SLArEventTrajectory CreateNewTrajectory(const G4Track*); 
    void SetupSecondaries(const G4Track* aTrack, const bool debug = false); 
    void SetupSecondariesFromOpticalPhoton(const G4Track* aTrack, const bool debug = false); 

    G4String GetProcessName(const G4Track* aTrack) const;

    friend class SLArTrackingActionMessenger;
};

class G4UIdirectory;
class G4UIcmdWithADoubleAndUnit;
class G4UIcmdWith3VectorAndUnit;
class G4UIcmdWith3Vector;
class G4UIcmdWithAString;
class G4UIcmdWithADouble; 
class G4UIcmdWithAnInteger;
class G4UIcmdWithABool; 
class G4UIcmdWithoutParameter;


class SLArTrackingActionMessenger : public G4UImessenger {
  public: 
    SLArTrackingActionMessenger( SLArTrackingAction* );
    virtual ~SLArTrackingActionMessenger(); 

    virtual void SetNewValue(G4UIcommand*, G4String); 
  
  private:
    SLArTrackingAction*     fTrkAction;
    G4UIcmdWithABool*       fCmdStoreParticleTrajectory;
    G4UIcmdWithABool*       fCmdStorePhotonTrajectory;
};



#endif /* end of include guard SLArTRACKINGACTION_HH */

