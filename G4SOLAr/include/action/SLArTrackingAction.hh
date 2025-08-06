/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArTrackingAction.hh
 * @created     : Monday Aug 31, 2020 18:35:29 CEST
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

